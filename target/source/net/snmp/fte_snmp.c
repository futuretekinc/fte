#include <stdlib.h>
#include <ctype.h>
#include "fte_target.h"
#include "snmpcfg.h"
#include "asn1.h"
#include "snmp.h"
#include "fte_target.h"
#include "fte_config.h" 
#include "fte_net.h"
#include "fte_object.h"
#include "fte_log.h" 
#include "fte_snmp.h" 
#include "fte_time.h" 
#include "fte_sys.h" 
#include "sys/fte_sys_timer.h" 
#include "nxjson.h"
#include "fte_json.h"

#undef  __MODULE__
#define __MODULE__  FTE_MODULE_NET_SNMP

#if FTE_SNMPD_SUPPORTED

#if ! RTCSCFG_ENABLE_IP4 
  #error This application requires RTCSCFG_ENABLE_IP4 defined non-zero in user_config.h. Please recompile BSP with this option.
#endif

#if ! RTCSCFG_ENABLE_SNMP
  #error This application requires RTCSCFG_ENABLE_SNMP defined non-zero in user_config.h. Please recompile BSP with this option.
#endif

#if ! MQX_HAS_TIME_SLICE
  #error This application requires MQX_HAS_TIME_SLICE defined non-zero in user_config.h. Please recompile BSP with this option.
#endif

/****************************************************************/

#define COUNTER_OVERFLOW    5
#define COUNTER_DELAY       5000

#ifndef FTE_NET_SNMP_TRAPS_COUNT
#define FTE_NET_SNMP_TRAPS_COUNT           5
#endif

typedef struct
{
    FTE_BOOL     bStatic;
    _ip_address xIP;
}   FTE_TRAP_SERVER, _PTR_ FTE_TRAP_SERVER_PTR;

typedef struct  
{
    struct queue_element_struct _PTR_   NEXT;
    struct queue_element_struct _PTR_   PREV;
    FTE_NET_SNMP_TRAP_TYPE              xType;
    union 
    {
        struct
        {
            FTE_UINT32     nOID;
            FTE_BOOL     bOccurred;
        }   xAlert;
        
        struct
        {
            _ip_address xHostIP;
        }   xDiscovery;
        
        struct
        {
            _ip_address xServerIP;
        }   xManagement;
    }   xParams;
    FTE_CHAR_PTR    pBuff;
}   FTE_TRAP_MSG, _PTR_ FTE_TRAP_MSG_PTR;

static const FTE_CHAR_PTR _unknown = "";

static FTE_CHAR     _buff[2048];
static FTE_LIST     _trapList;
static FTE_LIST     _trapServerList;
static FTE_UINT32   ulReqAlertCount = 0;
static FTE_UINT32   ulRespAlertCount = 0;
static FTE_UINT32   ulReqDiscoveryCount = 0;
static FTE_UINT32   ulRespDiscoveryCount = 0;

extern RTCSMIB_NODE MIBNODE_enterprises;
extern RTCSMIB_NODE MIBNODE_futuretek;
extern const RTCSMIB_NODE MIBNODE_msgDiscovery;
extern const RTCSMIB_NODE MIBNODE_msgAlert;

FTE_RET   FTE_SNMPD_init
(   
    FTE_SNMP_CFG_PTR pConfig
)
{
    FTE_INT32  i, ret;

#if FTE_NET_SNMP_MIB1213
    /* init RFC 1213 MIB */
    MIB1213_init();
#endif
    
#if FTE_NET_SNMP_MIBMQX
    /* init MQX MIB */
    MIBMQX_init();
#endif
    
    RTCSMIB_mib_add(&MIBNODE_futuretek);
    
    ret = SNMP_init(FTE_NET_SNMP_NAME, FTE_NET_SNMP_PRIO, FTE_NET_SNMP_STACK);
    if (ret != RTCS_OK)
    {
        return  FTE_RET_ERROR;
    }

    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id_from_name(FTE_NET_SNMP_NAME));
    
    FTE_LIST_init(&_trapList);
    FTE_LIST_init(&_trapServerList);
    
    for(i = 0 ;i < pConfig->xTrap.ulCount ; i++)
    {
        FTE_SNMPD_TRAP_add(pConfig->xTrap.pList[i], TRUE);
    }
    
    SNMP_trap_select_community(pConfig->xTrap.pCommunity);
    
#if FTE_NET_SNMP_TRAP_V1
    SNMP_trap_coldStart();
#elif FTE_NET_SNMP_TRAP_V2
    SNMPv2_trap_coldStart();
#endif

    return  FTE_RET_OK;
}

FTE_RET   FTE_SNMPD_TRAP_add
(
    _ip_address     target, 
    FTE_BOOL        bStatic
)
{
    FTE_LIST_ITERATOR   xIter;
    FTE_TRAP_SERVER_PTR pServer;
    
    FTE_LIST_ITER_init(&_trapServerList, &xIter);
    while ((pServer = FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        if (pServer->xIP == target)
        {
            return  FTE_RET_OK;
        }
    }    

    pServer = FTE_MEM_allocZero(sizeof(FTE_TRAP_SERVER));
    if (pServer == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pServer->xIP = target;
    pServer->bStatic = bStatic;    
    
    if (FTE_LIST_pushBack(&_trapServerList, pServer) != FTE_RET_OK)
    {
        FTE_MEM_free(pServer);
        return  FTE_RET_OK;
    }
    
    RTCS_trap_target_add(target);
    
    if (bStatic)
    {
        return  FTE_CFG_NET_TRAP_addIP(target);
    }
    
   return   FTE_RET_OK;
}

FTE_RET   FTE_SNMPD_TRAP_del
(
    _ip_address     target
)
{
    FTE_LIST_ITERATOR   xIter;
    FTE_TRAP_SERVER_PTR pServer;
    
    FTE_LIST_ITER_init(&_trapServerList, &xIter);
    while ((pServer = FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        if (pServer->xIP == target)
        {  
            RTCS_trap_target_remove(target);
            if (pServer->bStatic)
            {
                FTE_CFG_NET_TRAP_delIP(target);
            }
         
            FTE_LIST_remove(&_trapServerList, pServer);
            FTE_MEM_free(pServer);
            
            return  FTE_RET_OK;
        }
    }    
    
    return  FTE_RET_ERROR;
}

static FTE_TRAP_MSG_PTR pCurrentTrapMsg = NULL;

void FTE_SNMPD_TRAP_processing(void)
{
    if (IPCFG_STATE_UNBOUND == ipcfg_get_state(BSP_DEFAULT_ENET_DEVICE))
    {
        return;
    }
    
    if (FTE_LIST_count(&_trapList) != 0)
    {
        if (FTE_LIST_popFront(&_trapList, (FTE_VOID_PTR _PTR_)&pCurrentTrapMsg) != FTE_RET_OK)
        {
            ERROR("Trap list broken!\n");
            FTE_LIST_init(&_trapList);
            return;
        }
        
        switch(pCurrentTrapMsg->xType)
        {
        case    FTE_NET_SNMP_TRAP_TYPE_ALERT:
            {
#if  FTE_NET_SNMP_TRAP_V1
                SNMP_trap_userSpec((RTCSMIB_NODE *)&MIBNODE_msgAlert , 3, &MIBNODE_futuretek );
#elif FTE_NET_SNMP_TRAP_V2
                SNMPv2_trap_userSpec( (RTCSMIB_NODE *)&MIBNODE_msgAlert );
#endif
                ++ulRespAlertCount;
                TRACE("Send Alert trap[%d]\n", ulRespAlertCount);
            }
            break;
            
        case    FTE_NET_SNMP_TRAP_TYPE_DISCOVERY:
            {
                FTE_BOOL bNewServer = FALSE;
                if (!FTE_NET_SERVER_isExist(pCurrentTrapMsg->xParams.xDiscovery.xHostIP))
                {
                    TRACE("Add temporary trap server(%d.%d.%d.%d) for discovery\n", IPBYTES(pCurrentTrapMsg->xParams.xDiscovery.xHostIP));
                    FTE_UINT32 error = RTCS_trap_target_add(pCurrentTrapMsg->xParams.xDiscovery.xHostIP);
                    if (error) 
                    {
                        printf("\nFailed to add target trap, error = %X", error);
                    } 
                    bNewServer = TRUE;
                }

#if  FTE_NET_SNMP_TRAP_V1
                SNMP_trap_userSpec((RTCSMIB_NODE *)&MIBNODE_msgDiscovery , 3, &MIBNODE_futuretek );
#elif FTE_NET_SNMP_TRAP_V2
                SNMPv2_trap_userSpec( (RTCSMIB_NODE *)&MIBNODE_msgDiscovery );
#endif
                ++ulRespDiscoveryCount;
                TRACE("Send Discovery trap[%d]\n", ulRespDiscoveryCount);
                
                if (bNewServer)
                {
                    RTCS_trap_target_remove(pCurrentTrapMsg->xParams.xDiscovery.xHostIP);
                    TRACE("Remove temporary trap server(%d.%d.%d.%d) for discovery\n", IPBYTES(pCurrentTrapMsg->xParams.xDiscovery.xHostIP));
                }                
            }
            break;
            
        case    FTE_NET_SNMP_TRAP_TYPE_ADD:
            {
                if (!FTE_NET_SERVER_isExist(pCurrentTrapMsg->xParams.xManagement.xServerIP))
                {
                    
                    FTE_UINT32 error = FTE_SNMPD_TRAP_add(pCurrentTrapMsg->xParams.xManagement.xServerIP, TRUE);
                    if (error) 
                    {
                        TRACE("\nFailed to add target trap, error = %X", error);
                    } 
                }
            }
            break;
            
        case    FTE_NET_SNMP_TRAP_TYPE_DEL:
            {
                if (FTE_NET_SERVER_isExist(pCurrentTrapMsg->xParams.xManagement.xServerIP))
                {
                    FTE_UINT32 error = FTE_SNMPD_TRAP_del(pCurrentTrapMsg->xParams.xManagement.xServerIP);
                    if (error) 
                    {
                        TRACE("\nFailed to remove target trap, error = %X", error);
                    } 
                }
            }
            break;
        }
        
        if (pCurrentTrapMsg->pBuff != NULL)
        {
            FTE_MEM_free(pCurrentTrapMsg->pBuff);
            pCurrentTrapMsg->pBuff = NULL;
        }
        FTE_MEM_free(pCurrentTrapMsg);
        pCurrentTrapMsg = NULL;
        
        _time_delay(100);
    }
}

FTE_RET   FTE_SNMPD_TRAP_sendAlert
(
    FTE_UINT32  nOID, 
    FTE_BOOL    bOccurred
)
{
    FTE_TRAP_MSG_PTR    pMsg;
    
    if (FTE_LIST_count(&_trapList) > 100)
    {
        FTE_TRAP_MSG_PTR pTempTrapMsg = NULL;

        if (FTE_LIST_popFront(&_trapList, (FTE_VOID_PTR _PTR_)&pTempTrapMsg) != FTE_RET_OK)
        {
            return  FTE_RET_ERROR;
        }
        
        if (pTempTrapMsg->pBuff != NULL)
        {
            FTE_MEM_free(pTempTrapMsg->pBuff);
        }
        FTE_MEM_free(pTempTrapMsg);
    }

    
    ++ulReqAlertCount;
    
    pMsg = (FTE_TRAP_MSG_PTR)FTE_MEM_allocZero(sizeof(FTE_TRAP_MSG));
    if (pMsg == NULL)
    {
        return   FTE_RET_ERROR;
    }
    pMsg->xType                     = FTE_NET_SNMP_TRAP_TYPE_ALERT;
    pMsg->xParams.xAlert.nOID      = nOID;
    pMsg->xParams.xAlert.bOccurred = bOccurred;

    if (FTE_LIST_pushBack(&_trapList, pMsg) != FTE_RET_OK)
    {
        DEBUG("Not enough memory!\n");
        FTE_SYS_setUnstable();
        FTE_MEM_free(pMsg);
        return  FTE_RET_ERROR;
    }

   return   FTE_RET_OK;
}

FTE_RET   FTE_SNMPD_TRAP_discovery
(
    _ip_address     xHostIP
)
{
    FTE_TRAP_MSG_PTR    pMsg;
    
    ulReqDiscoveryCount ++;

#if 0
    FTE_LIST_ITERATOR   xIter;
    
    FTE_LIST_ITER_init(&_trapList, &xIter);
    while ((pMsg = FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        if ((pMsg->xType == FTE_NET_SNMP_TRAP_TYPE_DISCOVERY) && (pMsg->xParams.xDiscovery.xHostIP == xHostIP))
        {
            return  FTE_RET_OK;
        }
    }    
#endif
    pMsg = (FTE_TRAP_MSG_PTR)FTE_MEM_allocZero(sizeof(FTE_TRAP_MSG));
    if (pMsg == NULL)
    {
        return   FTE_RET_ERROR;
    }

    pMsg->xType     = FTE_NET_SNMP_TRAP_TYPE_DISCOVERY;
    pMsg->xParams.xDiscovery.xHostIP = xHostIP;
    
    if (FTE_LIST_pushBack(&_trapList, pMsg) != FTE_RET_OK)
    {
        ERROR("Not enough memory!\n");
        FTE_SYS_setUnstable();
        FTE_MEM_free(pMsg);
        return  FTE_RET_ERROR;
    }

    return  FTE_RET_OK;
}        

FTE_RET   FTE_SNMPD_TRAP_addServer
(
    _ip_address     xServerIP
)
{
    FTE_TRAP_MSG_PTR    pMsg = (FTE_TRAP_MSG_PTR)FTE_MEM_allocZero(sizeof(FTE_TRAP_MSG));
    if (pMsg == NULL)
    {
        return   FTE_RET_ERROR;
    }
        
    pMsg->xType                         = FTE_NET_SNMP_TRAP_TYPE_ADD;
    pMsg->xParams.xManagement.xServerIP = xServerIP;

    if (FTE_LIST_pushBack(&_trapList, pMsg) != FTE_RET_OK)
    {
        DEBUG("Not enough memory!\n");
        FTE_SYS_setUnstable();
        FTE_MEM_free(pMsg);
        return  FTE_RET_ERROR;
    }

   return   FTE_RET_OK;
}

FTE_RET   FTE_SNMPD_TRAP_delServer
(
    _ip_address     xServerIP
)
{
    FTE_TRAP_MSG_PTR    pMsg = (FTE_TRAP_MSG_PTR)FTE_MEM_allocZero(sizeof(FTE_TRAP_MSG));
    if (pMsg == NULL)
    {
        return   FTE_RET_ERROR;
    }
        
    pMsg->xType                         = FTE_NET_SNMP_TRAP_TYPE_DEL;
    pMsg->xParams.xManagement.xServerIP = xServerIP;

    if (FTE_LIST_pushBack(&_trapList, pMsg) != FTE_RET_OK)
    {
        DEBUG("Not enough memory!\n");
        FTE_SYS_setUnstable();
        FTE_MEM_free(pMsg);
        return  FTE_RET_ERROR;
    }

   return   FTE_RET_OK;
}


/*******************************************************************************
 * Support for MIB
 ******************************************************************************/
FTE_CHAR const _PTR_ MIB_get_productID
(
    FTE_VOID_PTR     dummy
)
{
    FTE_PRODUCT_DESC const *dev_desc = FTE_getProductDescription();
    if (dev_desc == NULL)
    {
        return  _unknown;
    }
    
    return  FTE_SYS_getOIDString();
}

FTE_CHAR const _PTR_ MIB_get_productModel
(
    FTE_VOID_PTR dummy
)
{
    FTE_PRODUCT_DESC const *dev_desc = FTE_getProductDescription();
    if (dev_desc == NULL)
    {
        return  _unknown;
    }

    return  dev_desc->pModel;
}

FTE_CHAR const _PTR_ MIB_get_vendorID
(
    FTE_VOID_PTR     dummy
)
{
    FTE_PRODUCT_DESC const *dev_desc = FTE_getProductDescription();
    if (dev_desc == NULL)
    {
        return  _unknown;
    }

    return  dev_desc->pManufacturer;
}

FTE_CHAR const _PTR_ MIB_get_HWVersion
(
    FTE_VOID_PTR     dummy
)
{
    FTE_PRODUCT_DESC const *desc = FTE_getProductDescription();
    if (desc == NULL)
    {
        return  _unknown;
    }

    sprintf(_buff, "%d.%d.%d.%d",
            (desc->xVersion.hw >> 24) & 0xFF,
            (desc->xVersion.hw >> 16) & 0xFF,
            (desc->xVersion.hw >>  8) & 0xFF,
            (desc->xVersion.hw      ) & 0xFF);

    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_SWVersion
(
    FTE_VOID_PTR     dummy
)
{
    FTE_PRODUCT_DESC const *desc = FTE_getProductDescription();
    if (desc == NULL)
    {
        return  _unknown;
    } 

    sprintf(_buff, "%d.%d.%d.%d",
            (desc->xVersion.sw >> 24) & 0xFF,
            (desc->xVersion.sw >> 16) & 0xFF,
            (desc->xVersion.sw >>  8) & 0xFF,
            (desc->xVersion.sw      ) & 0xFF);

    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_prodDesc
(
    FTE_VOID_PTR     dummy
)
{
    memset(_buff, 0, sizeof(_buff));
    FTE_CFG_getLocation(_buff, FTE_LOCATION_MAX_LEN);
    
    return  _buff;
}

FTE_UINT32 MIB_set_prodDesc
(
    FTE_VOID_PTR         dummy, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    if (ulVarLen > FTE_LOCATION_MAX_LEN) 
    {
        return  SNMP_ERROR_wrongLength;
    }
 
    FTE_CFG_setLocation((FTE_CHAR_PTR)pVar, ulVarLen);
    
    return   SNMP_ERROR_noError;
}

/******************************************************************************
 * Network Configuration 
 ******************************************************************************/
FTE_UINT32 MIB_get_netType
(
    FTE_VOID_PTR     dummy
)
{
    FTE_RET xRet;
    FTE_NET_CFG_PTR pCfgNet;
    
    xRet = FTE_CFG_NET_get(&pCfgNet);
    if (xRet != FTE_RET_OK)
    {
        return  FTE_NET_TYPE_STATIC;
    }
    
    return  pCfgNet->nType;
}

FTE_CHAR const _PTR_ MIB_get_netMacAddr
(
    FTE_VOID_PTR     dummy
)
{
    _enet_address   xMACAddress;
    
    FTE_SYS_getMAC(xMACAddress);
    sprintf(_buff, "%02x:%02x:%02x:%02x:%02x:%02x", 
            xMACAddress[0], xMACAddress[1],
            xMACAddress[2], xMACAddress[3],
            xMACAddress[4], xMACAddress[5]);
    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_netIpAddr
(
    FTE_VOID_PTR     dummy
)
{
    FTE_RET xRet;
    FTE_NET_CFG_PTR pCfgNet;
    
    xRet = FTE_CFG_NET_get(&pCfgNet);
    if (xRet != FTE_RET_OK)
    {
        return  "0.0.0.0";
    }
    
    sprintf(_buff, "%d.%d.%d.%d", IPBYTES(pCfgNet->xIPData.ip));
    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_netNetMask
(
    FTE_VOID_PTR     dummy
)
{
    FTE_RET xRet;
    FTE_NET_CFG_PTR pCfgNet;
   
    xRet = FTE_CFG_NET_get(&pCfgNet);
    if (xRet != FTE_RET_OK)
    {
        return  "255.255.255.255";
    }
    
    sprintf(_buff, "%d.%d.%d.%d", IPBYTES(pCfgNet->xIPData.mask));
    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_netGateway
(
    FTE_VOID_PTR     dummy
)
{
    FTE_RET xRet;
    FTE_NET_CFG_PTR pCfgNet;
   
    xRet = FTE_CFG_NET_get(&pCfgNet);
    if (xRet != FTE_RET_OK)
    {
        return  "";
    }
    
    sprintf(_buff, "%d.%d.%d.%d", IPBYTES(pCfgNet->xIPData.gateway));
    
    return  _buff;
}

/******************************************************************************
 * Object Management
 ******************************************************************************/
FTE_CHAR const _PTR_ MIB_get_objID
(
    FTE_VOID_PTR     pParam
) 
{
    assert(pParam != NULL);
    
    sprintf(_buff, "%08x", ((FTE_OBJECT_PTR)pParam)->pConfig->xCommon.nID);
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_objType
(
    FTE_VOID_PTR     pParam
) 
{
    assert(pParam != NULL);
    
    FTE_OBJECT_PTR  obj = (FTE_OBJECT_PTR)pParam;
    
    return FTE_OBJ_typeString(obj);
}

FTE_CHAR const _PTR_ MIB_get_objSN
(
    FTE_VOID_PTR     pParam
) 
{
    assert(pParam != NULL);
    
    FTE_OBJECT_PTR  obj = (FTE_OBJECT_PTR)pParam;
    
    FTE_OBJ_getSN(obj, _buff, sizeof(_buff));
    
    return _buff;
}


FTE_CHAR const _PTR_ MIB_get_objName
(
    FTE_VOID_PTR     pParam
)
{
    assert(pParam != NULL);
    
    FTE_OBJ_getName((FTE_OBJECT_PTR)pParam, _buff, sizeof(_buff));
    
    return  _buff;
}

FTE_UINT32 MIB_set_objName
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    assert(pParam != NULL);
    
    FTE_OBJECT_PTR  obj = (FTE_OBJECT_PTR)pParam;
    
    if (ulVarLen > MAX_OBJECT_NAME_LEN) 
    {
        return  SNMP_ERROR_wrongLength;
    }
    FTE_OBJ_setName(obj, (FTE_CHAR_PTR)pVar, ulVarLen);
    
    return   SNMP_ERROR_noError;
}

FTE_CHAR const _PTR_ MIB_get_objState
(
    FTE_VOID_PTR     pParam
)
{
    assert(pParam != NULL);

    if (FTE_OBJ_IS_ENABLED((FTE_OBJECT_PTR)pParam))
    {
        sprintf(_buff, "RUN");
    }
    else
    {
        sprintf(_buff, "STOP");
    }
    
    return  _buff;
}

FTE_UINT32 MIB_set_objState
(
    FTE_VOID_PTR     pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32  ulVarLen
)
{
    assert(pParam != NULL);

    if (ulVarLen >= sizeof(_buff))
    {
        return  SNMP_ERROR_wrongValue;
    }
        
    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';
    
    if (strcasecmp((FTE_CHAR_PTR)_buff, "run") == 0)
    {
        FTE_OBJ_activate((FTE_OBJECT_PTR)pParam, TRUE);
    }
    else if (strcasecmp((FTE_CHAR_PTR)_buff, "stop") == 0)
    {
        FTE_OBJ_activate((FTE_OBJECT_PTR)pParam, FALSE);
    }
    else
    {
        return  SNMP_ERROR_wrongValue;
    }

    return SNMP_ERROR_noError;
}

FTE_UINT32 MIB_set_objValue
(
    FTE_VOID_PTR     pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32  ulVarLen
)
{
    FTE_VALUE       xValue;
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;

    assert(pObj != NULL);
        
    if (ulVarLen >= sizeof(_buff))
    {
        return  SNMP_ERROR_wrongValue;
    }

    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';        

    if (FTE_OBJ_CLASS(pObj) == FTE_OBJ_CLASS_MULTI)
    {
        FTE_OBJ_setConfig(pObj, _buff);
    }
    else
    {       
        FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
        if (FTE_VALUE_set(&xValue, _buff) != FTE_RET_OK)
        { 
            return SNMP_ERROR_wrongValue;
        }
        
        if (FTE_OBJ_setValue(pObj, &xValue) != FTE_RET_OK)
        {
            return SNMP_ERROR_wrongValue;
        }
    }
    
    return SNMP_ERROR_noError;
}

FTE_CHAR const _PTR_ MIB_get_objValue
(   
    FTE_VOID_PTR     pParam
)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;    
    ASSERT(pObj != NULL);
    
    FTE_NET_liveTouch();
     
    if ( FTE_OBJ_IS_ENABLED(pObj))
    {
        if (FTE_OBJ_CLASS(pObj) == FTE_OBJ_CLASS_MULTI)
        {
            FTE_OBJ_getConfig(pObj, _buff, sizeof(_buff));
        }
        else
        {
            FTE_VALUE_toString(pObj->pStatus->pValue, _buff, sizeof(_buff));
        }
    }
    else
    {
        strcpy(_buff, "N/A");
    }
    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_objLastValue
(
    FTE_VOID_PTR     pParam
)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;    
    ASSERT(pObj != NULL);
    
    if ( FTE_OBJ_IS_ENABLED(pObj))
    {
        FTE_VALUE_toString(pObj->pStatus->pValue, _buff, sizeof(_buff));
    }
    else
    {
        strcpy(_buff, "N/A");
    }
    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_objLastTime
(
    FTE_VOID_PTR    pParam
)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    
    if (FTE_VALUE_isValid(pObj->pStatus->pValue))
    {
        TIME_STRUCT xTimeStamp;
        
        FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTimeStamp);
        sprintf(_buff,"%d", xTimeStamp.SECONDS);
    }
    else
    {
        strcpy(_buff, "0");
    }

    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_objInitValue
(
    FTE_VOID_PTR    pParam
)
{
    return  _buff;
}

FTE_UINT32 MIB_set_objInitValue
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    return  0;
}


FTE_UINT32 MIB_get_objTriggerMode
(
    FTE_VOID_PTR    pParam
)
{
    return  0;
}

FTE_UINT32 MIB_set_objTriggerMode
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    return  0;
}


FTE_UINT32 MIB_get_objUpdateInterval
(
    FTE_VOID_PTR    pParam
)
{
    FTE_RET xRet;
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    FTE_UINT32      ulInterval = 0;
    
    xRet = FTE_OBJ_getInterval(pObj, &ulInterval);
    if (xRet != FTE_RET_OK)
    {
        return  0;
    }

    return  ulInterval;
}

FTE_UINT32 MIB_set_objUpdateInterval
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    FTE_INT32 nUpdateInterval;
    
    FTE_OBJECT_PTR pObj = (FTE_OBJECT_PTR)pParam;
    
    assert(pObj != NULL);

    nUpdateInterval = RTCSMIB_int_read(pVar, ulVarLen);
    if ((1 <= nUpdateInterval) || (nUpdateInterval <= 60 * 60 * 24))
    {
        FTE_RET xRet;
        
        xRet = FTE_OBJ_setInterval(pObj, nUpdateInterval);
        if (xRet == FTE_RET_OK)
        {
            return SNMP_ERROR_noError;
        }
    }

    return  SNMP_ERROR_wrongValue;
}


FTE_UINT32 MIB_get_objTotalTrial
(
    FTE_VOID_PTR    pParam
)
{
    FTE_RET xRet;
    FTE_OBJECT_STATISTICS    xStatistics;
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam; 
    
    xRet = FTE_OBJ_getStatistics(pObj, &xStatistics);
    if (xRet != FTE_RET_OK)
    {
        return  0;
    }

    return  xStatistics.nTotalTrial;
}

FTE_UINT32 MIB_get_objTotalFailed
(
    FTE_VOID_PTR    pParam
)
{
    FTE_RET xRet;
    FTE_OBJECT_STATISTICS    xStatistics;
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam; 
    
    xRet = FTE_OBJ_getStatistics(pObj, &xStatistics);
    if (xRet != FTE_RET_OK)
    {
        return  0;
    }
    
    return  xStatistics.nTotalFail;
}


static  FTE_INT32    _nDiscoveryKey = 0;

FTE_INT32  MIB_get_smDiscovery
(
    FTE_VOID_PTR    pParam
)
{
    _nDiscoveryKey = (RTCS_rand() & 0x7FFFFFFF);
    
    return  _nDiscoveryKey;
}

FTE_UINT32 MIB_set_smDiscovery
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    FTE_INT32   nKey= (FTE_UINT32)RTCSMIB_int_read(pVar, ulVarLen);
    
#if FTE_1WIRE_SUPPORTED
    if ((_nDiscoveryKey != 0) && (_nDiscoveryKey == nKey))
    {
        FTE_1WIRE_PTR   p1Wire;
        FTE_DS18B20_CREATE_PARAMS xParams;
        FTE_UINT32  nIndex;
        FTE_CHAR    pFamilyName[32];
        
        memset(pFamilyName, 0, sizeof(pFamilyName));
        
        p1Wire = FTE_1WIRE_getFirst();
        while(p1Wire != 0)
        {
            xParams.nBUSID = p1Wire->pConfig->nID;
            
            FTE_UINT32  ulDevCount = 0;
            FTE_1WIRE_DEV_count(p1Wire, &ulDevCount);            
            for(nIndex = 0 ; nIndex < ulDevCount ; nIndex++)
            {
                if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != FTE_RET_OK)
                {
                    return  SNMP_ERROR_resourceUnavailable;
                }
             
                FTE_1WIRE_getFamilyName(xParams.pROMCode[0], pFamilyName, sizeof(pFamilyName) -1 );
                if (strcmp(pFamilyName, "18B20") == 0)
                {
#if FTE_DS18B20_SUPPORTED 
                    if (!FTE_DS18B20_isExistROMCode(xParams.pROMCode) )
                    {
                        FTE_OBJECT_PTR          pObj = FTE_DS18B20_create(&xParams);
                        if (pObj == NULL)
                        {
                            return  SNMP_ERROR_resourceUnavailable;
                        }

                        FTE_OBJ_start(pObj);                        
                        FTE_CFG_OBJ_save(pObj);
                    }
#endif
                }
            }
            
            p1Wire = FTE_1WIRE_getNext(p1Wire);
        } 
        
        FTE_CFG_save(TRUE);

    }
#endif
    
    return  SNMP_ERROR_noError;
}

FTE_CHAR const _PTR_  MIB_get_smDestroyDynamicObject
(
    FTE_VOID_PTR    pParam
)
{
    return  "";
}

FTE_UINT32 MIB_set_smDestroyDynamicObject
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    FTE_UINT32 nOID = 0;
    
    if (ulVarLen > 8)
    {
        return  SNMP_ERROR_badValue;
    }
    
    for(int i = 0 ; i < ulVarLen ; i++)
    {
        FTE_UINT32 nValue;
        
        if ( isdigit(pVar[i]))
        {
            nValue = pVar[i] - '0';
        }
        else if ( isxdigit(pVar[i]))
        {
            if (islower(pVar[i]))
            {
                nValue = pVar[i] - 'a' + 10;            
            }
            else
            {
                nValue = pVar[i] - 'A' + 10;            
            }
        }
        else
        {
            return  SNMP_ERROR_badValue;
        }
        
        nOID = (nOID << 4) + nValue;
    }

    if (nOID == 0)
    {
        
    }
    else 
    {
        FTE_OBJECT_PTR pObj = FTE_OBJ_get(nOID);
        if (pObj == NULL)
        {
            return  SNMP_ERROR_badValue;
        }
        
        FTE_OBJ_destroy(pObj);
    }
    
     return  SNMP_ERROR_noError;
}
/******************************************************************************
 * Digital Input 
 ******************************************************************************/
struct MIB_get_sensor_struct 
{
   FTE_UINT32                 current_index;
   FTE_UINT32                 index;
};

FTE_UINT32  MIB_get_diCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_DI, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_diIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_diEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ 
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_DI, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;

} /* Endbody */


FTE_UINT32 MIB_set_diState
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_diName
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_diUpdateInterval
(
    FTE_VOID_PTR    pParam, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}


/******************************************************************************
 * Temperature Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_tempCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_TEMPERATURE, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_tempIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_tempEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_TEMPERATURE, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */


FTE_UINT32 MIB_set_tempName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_tempState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_tempUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Humidity Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_humiCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_HUMIDITY, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_humiIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_humiEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_HUMIDITY, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

FTE_UINT32 MIB_set_humiName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_humiState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_humiUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Voltage Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_vltCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_VOLTAGE, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_vltIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_vltEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_VOLTAGE, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

FTE_UINT32 MIB_set_vltName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_vltState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_vltUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Current Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_currCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_CURRENT, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_currIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_currEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_CURRENT, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

FTE_UINT32 MIB_set_currName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_currState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_currUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}
/******************************************************************************
 * Digital Output 
 ******************************************************************************/
FTE_UINT32 MIB_get_doCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_DO, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_doIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_doEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_DO, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

const char *MIB_get_doInitState(FTE_VOID_PTR pParam)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    FTE_VALUE       xValue;
    assert(pObj != NULL);

    FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
    FTE_DO_getInitState(pObj, &xValue.xData.ulValue);    
    FTE_VALUE_toString(pObj->pStatus->pValue, _buff, sizeof(_buff));

    return _buff;
}

FTE_UINT32 MIB_set_doInitState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    assert(pObj != NULL);

    if (ulVarLen >= sizeof(_buff))
    {
        return  SNMP_ERROR_wrongValue;
    }
        
    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';
    
    if ((strcasecmp((FTE_CHAR_PTR)_buff, "on") == 0) || (strcmp((FTE_CHAR_PTR)_buff, "1") == 0))
    {
        FTE_DO_setInitState(pObj, TRUE);        
    }
    else if ((strcasecmp((FTE_CHAR_PTR)_buff, "off") == 0) || (strcmp((FTE_CHAR_PTR)_buff, "0") == 0))
    {
        FTE_DO_setInitState(pObj, FALSE);        
    }
    else
    {
        return SNMP_ERROR_wrongValue;
    }

    return SNMP_ERROR_noError;    
}

FTE_UINT32 MIB_set_doName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_doState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_doValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objValue(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_doInitValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objInitValue(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Relay
 ******************************************************************************/
FTE_UINT32 MIB_get_rlCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_RL, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_rlIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_rlEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_RL, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;

} /* Endbody */

const char *MIB_get_rlInitState(FTE_VOID_PTR pParam)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    FTE_VALUE       xValue;
    
    assert(pObj != NULL);

    FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
    FTE_DO_getInitState(pObj, &xValue.xData.ulValue);    
    FTE_VALUE_toString(pObj->pStatus->pValue, _buff, sizeof(_buff));

    return _buff;
}

FTE_UINT32 MIB_set_rlInitState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    assert(pObj != NULL);
        
    if (ulVarLen >= sizeof(_buff))
    {
        return  SNMP_ERROR_wrongValue;
    }
        
    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';
    
    if ((strcasecmp((FTE_CHAR_PTR)_buff, "on") == 0) || (strcmp((FTE_CHAR_PTR)_buff, "1") == 0))
    {
        FTE_RL_setInitState(pObj, TRUE);        
    }
    else if ((strcasecmp((FTE_CHAR_PTR)_buff, "off") == 0) || (strcmp((FTE_CHAR_PTR)_buff, "0") == 0))
    {
        FTE_RL_setInitState(pObj, FALSE);        
    }
    else
    {
        return SNMP_ERROR_wrongValue;
    }

    return SNMP_ERROR_noError;    
}

FTE_UINT32 MIB_set_rlName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_rlState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_rlValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objValue(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_rlInitValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objInitValue(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Power Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_pwrCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_POWER, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_pwrIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_pwrEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_POWER, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */


FTE_UINT32 MIB_set_pwrName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_pwrState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_pwrUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * GAS Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_gasCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_GAS, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_gasIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_gasEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_GAS, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */


FTE_UINT32 MIB_set_gasName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_gasState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_gasUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Dust Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_dustCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_DUST, FTE_OBJ_TYPE_MASK, FALSE);
} 

FTE_UINT32 MIB_set_dustIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_dustEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_DUST, FTE_OBJ_TYPE_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */


FTE_UINT32 MIB_set_dustName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_dustState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_dustUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Count
 ******************************************************************************/
FTE_UINT32 MIB_get_cntCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_COUNT, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_cntIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_cntEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_COUNT, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;

} /* Endbody */

const char *MIB_get_cntInitState(FTE_VOID_PTR pParam)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    FTE_VALUE       xValue;
    assert(pObj != NULL);

    FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
    FTE_DO_getInitState(pObj, &xValue.xData.ulValue);    
    FTE_VALUE_toString(pObj->pStatus->pValue, _buff, sizeof(_buff));

    return _buff;
}

FTE_UINT32 MIB_set_cntInitState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)pParam;
    assert(pObj != NULL);
        
    if (ulVarLen >= sizeof(_buff))
    {
        return  SNMP_ERROR_wrongValue;
    }
        
    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';
    
    if ((strcasecmp((FTE_CHAR_PTR)_buff, "on") == 0) || (strcmp((FTE_CHAR_PTR)_buff, "1") == 0))
    {
        //fte_cnt_set_init_state(pObj, TRUE);        
    }
    else if ((strcasecmp((FTE_CHAR_PTR)_buff, "off") == 0) || (strcmp((FTE_CHAR_PTR)_buff, "0") == 0))
    {
        //fte_cnt_set_init_state(pObj, FALSE);        
    }
    else
    {
        return SNMP_ERROR_wrongValue;
    }

    return SNMP_ERROR_noError;    
}

FTE_UINT32 MIB_set_cntName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_cntState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_cntValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objValue(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_cntInitValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objInitValue(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Pressure Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_prsCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_PRESSURE, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_prsIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_prsEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_PRESSURE, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

FTE_UINT32 MIB_set_prsName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_prsState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_prsUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_prsValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objValue(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_prsInitValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objInitValue(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Discrete Sensor
 ******************************************************************************/
FTE_UINT32 MIB_get_dscCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_DISCRETE, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_dscIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_dscEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_DISCRETE, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

FTE_UINT32 MIB_set_dscName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_dscState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_dscUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_dscValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objValue(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_dscInitValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objInitValue(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Multi
 ******************************************************************************/
FTE_UINT32 MIB_get_multiCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_MULTI, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_multiIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_multiEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MULTI, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

FTE_UINT32 MIB_set_multiName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_multiState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_multiUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_multiValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objValue(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_multiInitValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objInitValue(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Device
 ******************************************************************************/
FTE_UINT32 MIB_get_devCount(FTE_VOID_PTR dummy)
{ 
    return  FTE_OBJ_count(FTE_OBJ_TYPE_MULTI, FTE_OBJ_CLASS_MASK, FALSE);
} 

FTE_UINT32 MIB_set_devIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_devEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MULTI, FTE_OBJ_CLASS_MASK, nIndex - 1, FALSE);
   if (!pObj) 
   {
      return FALSE;
   } /* Endif */ 
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;
 
} /* Endbody */

FTE_UINT32 MIB_set_devName(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objName(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_devState(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objState(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_devUpdateInterval(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objUpdateInterval(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_devValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objValue(pParam, pVar, ulVarLen);
}

FTE_UINT32 MIB_set_devInitValue(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return  MIB_set_objInitValue(pParam, pVar, ulVarLen);
}

/******************************************************************************
 * Event
 ******************************************************************************/
FTE_UINT32 MIB_get_eventCount(FTE_VOID_PTR dummy)
{ 
    return  1;
} 
FTE_UINT32 MIB_set_eventIndex (FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen) {return SNMP_ERROR_inconsistentValue;}

FTE_BOOL MIB_find_eventEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ /* Body */
   FTE_UINT32           nIndex = *(FTE_UINT32_PTR)index;
   FTE_VOID_PTR           pObj;

   if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
   {
      nIndex = 1;
   } /* Endif */

   if (FTE_EVENT_getAt(nIndex - 1, (FTE_EVENT_PTR _PTR_)&pObj) != FTE_RET_OK)
   {
      return FALSE;
   } /* Endif */
   *(FTE_UINT32_PTR)index = nIndex;
   *instance = pObj;
   return TRUE;

} /* Endbody */

const char *MIB_get_eventCondition(FTE_VOID_PTR pParam)
{
    assert(pParam != NULL);
    
    FTE_EVENT_PTR   pEvent = (FTE_EVENT_PTR)pParam;
    switch(pEvent->pConfig->xCondition)
    {
    case    FTE_EVENT_CONDITION_ABOVE:   
        {
            snprintf(_buff, sizeof(_buff), 
                     "{ \"type\":\"%s\", \"limit\" : \"%d\", \"threshold\" : \"%d\"}", 
                     "above", 
                     pEvent->pConfig->xParams.xLimit.nValue,
                     pEvent->pConfig->xParams.xLimit.ulThreshold);                     
        }
        break;
            
    case    FTE_EVENT_CONDITION_BELOW:   
        {
            snprintf(_buff, sizeof(_buff), 
                     "{ \"type\":\"%s\", \"limit\" : \"%d\", \"threshold\" : \"%d\"}", 
                     "below", 
                     pEvent->pConfig->xParams.xLimit.nValue,
                     pEvent->pConfig->xParams.xLimit.ulThreshold);                     
        }
        break;
        
    case    FTE_EVENT_CONDITION_INSIDE:  
        {
            snprintf(_buff, sizeof(_buff), 
                     "{ \"type\":\"%s\", \"upper\" : \"%d\", \"lower\" : \"%d\", \"threshold\" : \"%d\"}", 
                     "inside", 
                     pEvent->pConfig->xParams.xRange.nUpper,
                     pEvent->pConfig->xParams.xRange.nLower,
                     pEvent->pConfig->xParams.xLimit.ulThreshold);                     
        }
        break;
        
    case    FTE_EVENT_CONDITION_OUTSIDE: 
        {
            snprintf(_buff, sizeof(_buff), 
                     "{ \"type\":\"%s\", \"upper\" : \"%d\", \"lower\" : \"%d\", \"threshold\" : \"%d\"}", 
                     "outside", 
                     pEvent->pConfig->xParams.xRange.nUpper,
                     pEvent->pConfig->xParams.xRange.nLower,
                     pEvent->pConfig->xParams.xLimit.ulThreshold);                     
        }
        break;
        
    case    FTE_EVENT_CONDITION_INTERVAL:
        {
            snprintf(_buff, sizeof(_buff), 
                     "{ \"type\":\"%s\", \"interval\" : \"%d\"}", 
                     "interval", 
                     pEvent->pConfig->xParams.ulInterval);                     
        }
        break;
        
    case    FTE_EVENT_CONDITION_TIME:    
        {
            snprintf(_buff, sizeof(_buff), 
                     "{ \"type\":\"%s\" }", 
                     "time");                     
        }
        break;
    }

    return _buff;
}

FTE_UINT32 MIB_set_eventCondition(FTE_VOID_PTR pParam, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{     
    assert(pParam != NULL);

    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    
    const nx_json* json = nx_json_parse_utf8(_buff);
    if (json == NULL)
    {
        return  SNMP_ERROR_wrongValue;
    } 
    
    const nx_json* type_json = nx_json_get(json, "type");

    printf("type : %s\n", type_json->text_value);
    
    nx_json_free(json);

    return SNMP_ERROR_noError;    
}
 
const char *MIB_get_eventEPID(FTE_VOID_PTR pParam)
{
    assert(pParam != NULL);
    
    FTE_EVENT_PTR   pEvent = (FTE_EVENT_PTR)pParam;
    
    sprintf(_buff, "%08lx", pEvent->pConfig->ulEPID);
    
    return  _buff;
} 
     

const char *MIB_get_eventLevel(FTE_VOID_PTR pParam)
{
    assert(pParam != NULL);

    FTE_EVENT_PTR   pEvent = (FTE_EVENT_PTR)pParam;
    FTE_EVENT_level_string(pEvent->pConfig->xLevel, _buff, sizeof(_buff));
    
    return  _buff;
}

const char *MIB_get_eventType(FTE_VOID_PTR pParam)
{
    assert(pParam != NULL);

    FTE_EVENT_PTR   pEvent = (FTE_EVENT_PTR)pParam;
    FTE_EVENT_type_string(pEvent->pConfig->xType, _buff, sizeof(_buff));
    
    return  _buff;
}

    
/******************************************************************************
 * for Server Configuratoin
 ******************************************************************************/

FTE_BOOL MIB_find_svrEntry 
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ 
   FTE_UINT32           serverIndex = *(FTE_UINT32_PTR)index;

   if ((op == RTCSMIB_OP_GETNEXT) && (serverIndex == 0)) 
   {
      serverIndex  = 1;
   } /* Endif */

   if (serverIndex - 1 < FTE_NET_SERVER_count())
   {
        *instance = (FTE_VOID_PTR)(serverIndex - 1);
        *(FTE_UINT32_PTR)index = serverIndex;
        
        return TRUE;
   }
   else
   {
        *(FTE_UINT32_PTR)index = 0;
        
        return FALSE;   
   }


} /* Endbody */

FTE_CHAR const _PTR_ MIB_get_srvIpAddr(FTE_VOID_PTR pParams)
{
    FTE_UINT32 idx = (FTE_UINT32)pParams;
    FTE_UINT32 ip;

    ip = FTE_NET_SERVER_getAt(idx);
    
    sprintf(_buff, "%d.%d.%d.%d", IPBYTES(ip));
    
    return  _buff;
}

/******************************************************************************
 * Log Management
 ******************************************************************************/

FTE_UINT32 MIB_get_logCount(FTE_VOID_PTR dummy)
{
    return  FTE_LOG_count();
}

FTE_BOOL MIB_find_logEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ 
    FTE_UINT32             logIndex = *(FTE_UINT32_PTR)index;

    if ((op == RTCSMIB_OP_GETNEXT) && (logIndex == 0)) 
    {
        logIndex = 1;
    } 

    if (logIndex > FTE_LOG_count())
    {
        return  FALSE;
    }

    *instance = (FTE_VOID_PTR)(FTE_LOG_count() - logIndex + 1);
    *(FTE_UINT32_PTR)index = logIndex;
                    
    return TRUE;
} /* Endbody */


FTE_UINT32 MIB_get_logIndex(FTE_VOID_PTR dummy)
{
    return  (FTE_UINT32)dummy;
}

FTE_CHAR const _PTR_ MIB_get_logTime(FTE_VOID_PTR pIndex)
{
    FTE_LOG_PTR pLog;
    
    pLog = FTE_LOG_getAt((FTE_UINT32)pIndex - 1);
    if (pLog == NULL)
    {
        return  _unknown;
    } 
    
    FTE_TIME_toStr(&pLog->xTimeStamp, _buff, sizeof(_buff));
    
    return  _buff;
}

FTE_UINT32 MIB_get_logLevel(FTE_VOID_PTR pIndex)
{
    FTE_UINT32     nIndex = (FTE_UINT32)pIndex;
    FTE_LOG_PTR pLog = FTE_LOG_getAt(nIndex - 1);
    
    if (pLog == NULL)
    {
        return  0;
    }
    
    return  0;
}

FTE_CHAR const _PTR_ MIB_get_logID(FTE_VOID_PTR pIndex)
{
    FTE_UINT32     nIndex = (FTE_UINT32)pIndex;
    FTE_LOG_PTR pLog = FTE_LOG_getAt(nIndex - 1);
    
    if (pLog == NULL)
    {
        return  _unknown;
    }
    
    sprintf(_buff, "");

    return  _buff;
}
 
FTE_CHAR const _PTR_ MIB_get_logValue(FTE_VOID_PTR pIndex)
{
    FTE_UINT32         nIndex = (FTE_UINT32)pIndex;
    FTE_LOG_PTR     pLog = FTE_LOG_getAt(nIndex - 1);
    
    if (pLog == NULL)
    {
        return  _unknown;
    }
    
    sprintf(_buff,"unknown");
    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_logString(FTE_VOID_PTR pIndex)
{
    FTE_UINT32         nIndex = (FTE_UINT32)pIndex;
    FTE_LOG_PTR     pLog = FTE_LOG_getAt(nIndex - 1);
    
    return  _unknown;
}

/******************************************************************************
 * for TRAP 
 ******************************************************************************/

FTE_BOOL MIB_find_tsEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ 
    FTE_RET         xRet;
    FTE_UINT32      ulServerIndex = *(FTE_UINT32_PTR)index;
    FTE_NET_CFG_PTR pCfgNet;
   
    xRet = FTE_CFG_NET_get(&pCfgNet);
    if (xRet != FTE_RET_OK)
    {
        return  FALSE;
    }

    if (pCfgNet->xSNMP.xTrap.ulCount == 0)
    {
        return  FALSE;
    }
    
    if ((op == RTCSMIB_OP_GETNEXT) && (ulServerIndex == 0)) 
    {
        ulServerIndex = 1;
    } 
    
    if (ulServerIndex > pCfgNet->xSNMP.xTrap.ulCount)
    {
        return  FALSE;
    }

    *instance = (FTE_VOID_PTR)&pCfgNet->xSNMP.xTrap.pList[ulServerIndex - 1];
    *(FTE_UINT32_PTR)index = ulServerIndex;
                    
    return TRUE;
} /* Endbody */


FTE_CHAR const _PTR_ MIB_get_tsIpAddr(FTE_VOID_PTR dummy)
{
    _ip_address *ip = dummy;
    
    sprintf(_buff, "%d.%d.%d.%d", IPBYTES(*ip));
    
    return  _buff;
}

FTE_UINT32 MIB_get_tsCount(FTE_VOID_PTR dummy)
{
    FTE_RET xRet;
    FTE_NET_CFG_PTR pCfgNet;
   
    xRet = FTE_CFG_NET_get(&pCfgNet);
    if (xRet != FTE_RET_OK)
    {
        return  0;
    }

    if (pCfgNet == NULL)
    {
        return  0;
    }
    
    return  pCfgNet->xSNMP.xTrap.ulCount; 
}

FTE_CHAR const _PTR_ MIB_get_tsAdd(FTE_VOID_PTR dummy)
{
    sprintf(_buff,"xxx.xxx.xxx.xxx");
    return  _buff;
}


FTE_UINT32 MIB_set_tsAdd(FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    FTE_UINT32 nIP;
    
    if (ulVarLen >= sizeof(_buff))
    {
        return  SNMP_ERROR_wrongValue;
    }
        
    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';
    
    if (FTE_strToIP((FTE_CHAR_PTR)_buff, &nIP) != FTE_RET_OK)
    {
        return  SNMP_ERROR_badValue;
    }

    FTE_SNMPD_TRAP_addServer(nIP);
    
    return   SNMP_ERROR_noError;
}

FTE_CHAR const _PTR_ MIB_get_tsDel(FTE_VOID_PTR dummy)
{
    return   SNMP_ERROR_noError;    
}

FTE_UINT32 MIB_set_tsDel(FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    FTE_UINT32 nIP;
    
    if (ulVarLen >= sizeof(_buff))
    {
        return  SNMP_ERROR_wrongValue;
    }
        
    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';
    
    if (FTE_strToIP((FTE_CHAR_PTR)_buff, &nIP) != FTE_RET_OK)
    {
         return  SNMP_ERROR_badValue;
    }

    if (FTE_SNMPD_TRAP_delServer(nIP) != FTE_RET_OK)
    {
        return  SNMP_ERROR_badValue;
    }
    
     return   SNMP_ERROR_noError;
}


FTE_CHAR const _PTR_ MIB_get_msgDiscovery
(
    FTE_VOID_PTR    pParam
)
{ 
    FTE_RET xRet;
    
    _buff[sizeof(_buff) - 1] = '\0';
    xRet = FTE_SMNG_getDiscoveryMessage(_buff, sizeof(_buff) - 1);
    if (xRet != FTE_RET_OK)
    {
        _buff[0] = '\0';
    }
    
    return  _buff;
}

FTE_CHAR const _PTR_ MIB_get_msgAlert
(
    FTE_VOID_PTR    pParam
)
{
    FTE_RET xRet;
    
    if ((pCurrentTrapMsg != NULL) && (pCurrentTrapMsg->xType == FTE_NET_SNMP_TRAP_TYPE_ALERT))
    {
        if (pCurrentTrapMsg->pBuff == NULL)
        {
            FTE_UINT32         ulLen;
            FTE_OBJECT_PTR  pObj = FTE_OBJ_get(pCurrentTrapMsg->xParams.xAlert.nOID);    
            if (pObj == NULL)
            {
                return  _unknown;
            }
            
            FTE_JSON_VALUE_PTR  pJSONValue;
           
            xRet  = FTE_OBJ_createJSON(pObj, FTE_OBJ_FIELD_DID | FTE_OBJ_FIELD_EP_VALUE, (FTE_JSON_OBJECT_PTR _PTR_)&pJSONValue);
            if (xRet != FTE_RET_OK)
            {
                ERROR("Not enough memory!\n");
                return  _unknown;
            }            
            
            ulLen = FTE_JSON_VALUE_buffSize(pJSONValue) + 1;
            pCurrentTrapMsg->pBuff = (FTE_CHAR_PTR)FTE_MEM_alloc(ulLen);
            if (pCurrentTrapMsg == NULL)
            {
                ERROR("Not enough memory!\n");
            }
            else
            {
                FTE_JSON_VALUE_snprint(pCurrentTrapMsg->pBuff, ulLen, pJSONValue);            
            }
            
            FTE_JSON_VALUE_destroy(pJSONValue);
        }
        
        return  pCurrentTrapMsg->pBuff;
    }
    
    return  _unknown;
}

/******************************************************************************
 * for Admin
 ******************************************************************************/

FTE_UINT32 MIB_set_adminOID(FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return   SNMP_ERROR_noError;
}

FTE_CHAR const _PTR_ MIB_get_adminMAC(FTE_VOID_PTR dummy)
{
    _enet_address   pMAC;
    
    if (FTE_SYS_getMAC(pMAC) != FTE_RET_OK)
    {
        return  _unknown;
    }

    sprintf(_buff, "%02x:%02x:%02x:%02x:%02x:%02x", 
            pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
    
    return  _buff;
}


FTE_UINT32 MIB_set_adminMAC(FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    return   SNMP_ERROR_noError;
}

static  FTE_INT32    _nFactoryResetKey = 0;

FTE_CHAR const _PTR_ MIB_get_adminFactoryReset(FTE_VOID_PTR dummy)
{
    _nFactoryResetKey = (RTCS_rand() & 0x7FFFFFFF);

    sprintf(_buff, "%08x", _nFactoryResetKey);
    
    return  _buff;
}


FTE_UINT32 MIB_set_adminFactoryReset(FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    sprintf(_buff, "%08x", _nFactoryResetKey);

    if ((_nFactoryResetKey == 0) || ulVarLen != 8 || strncmp(_buff, (char*)pVar, 8) != 0)
    {
        return  SNMP_ERROR_wrongValue;
    }
    
    FTE_CFG_clear();

    return   SNMP_ERROR_noError;
}


FTE_CHAR const _PTR_ MIB_get_adminSystemTime(FTE_VOID_PTR dummy)
{
    TIME_STRUCT xTime;
    
    _time_get(&xTime);
    FTE_TIME_toStr(&xTime, _buff, sizeof(_buff));

    return  _buff;
}


FTE_UINT32 MIB_set_adminSystemTime(FTE_VOID_PTR dummy, FTE_UINT8_PTR pVar, FTE_UINT32 ulVarLen)
{
    TIME_STRUCT xTime;
    

    strncpy(_buff, (FTE_CHAR_PTR)pVar, ulVarLen);
    _buff[ulVarLen] = '\0';
    if (FTE_TIME_fromStr(&xTime, (FTE_CHAR_PTR)_buff) != FTE_RET_OK)
    {
        return  SNMP_ERROR_badValue;
    }

    _time_set(&xTime);
    _rtc_init(RTC_INIT_FLAG_CLEAR | RTC_INIT_FLAG_ENABLE);
    if( _rtc_sync_with_mqx(FALSE) != FTE_RET_OK )
    {
        printf("\nError synchronize time!\n");
    }

    return   SNMP_ERROR_noError;
}

static  FTE_INT32    _nResetKey = 0;

FTE_CHAR const _PTR_ MIB_get_adminReset(FTE_VOID_PTR dummy)
{
    _nResetKey = (RTCS_rand() & 0x7FFFFFFF);

    sprintf(_buff, "%08x", _nResetKey);
    
    return  _buff;
}

static 
void FTE_SNMP_configSave
(
    FTE_TIMER_ID   xTimerID, 
    FTE_VOID_PTR   pData, 
    FTE_UINT32      ulSecs,
    FTE_UINT32      ulMilliSecs
)
{
    FTE_CFG_save(TRUE);
}

void FTE_SNMP_reset
(
    FTE_TIMER_ID   xTimerID, 
    FTE_VOID_PTR   pData, 
    FTE_UINT32      ulSecs,
    FTE_UINT32      ulMilliSecs
)
{
    FTE_SYS_reset();
}


FTE_UINT32 MIB_set_adminReset
(
    FTE_VOID_PTR    dummy, 
    FTE_UINT8_PTR   pVar, 
    FTE_UINT32      ulVarLen
)
{
    sprintf(_buff, "%08x", _nResetKey);

    if ((_nResetKey == 0) || ulVarLen != 8 || strncmp(_buff, (FTE_CHAR_PTR)pVar, 8) != 0)
    {
        return  SNMP_ERROR_wrongValue;
    }
    
    FTE_TIMER_startOneshotAfter(FTE_SNMP_configSave, NULL, 1000, NULL);
    FTE_TIMER_startOneshotAfter(FTE_SNMP_reset, NULL, 3000, NULL);

    return   SNMP_ERROR_noError;
}


FTE_BOOL MIB_find_dbgBTEntry
(
    FTE_UINT32        op,
    FTE_VOID_PTR        index,
    FTE_VOID_PTR _PTR_  instance
)
{ 
    FTE_UINT32              nIndex = *(FTE_UINT32_PTR)index;

    if ((op == RTCSMIB_OP_GETNEXT) && (nIndex == 0)) 
    {
        nIndex = 1;
    } 
    
    if (nIndex > FTE_LOG_BOOT_TIME_MAX_COUNT)
    {
        return  FALSE;
    }

    *instance = (FTE_VOID_PTR)(nIndex - 1);
    *(FTE_UINT32_PTR)index = nIndex;
                    
    return TRUE;
} /* Endbody */


FTE_UINT32 MIB_get_dbgBTIndex(FTE_VOID_PTR dummy)
{
    return  (FTE_UINT32)dummy + 1;
}

FTE_CHAR const _PTR_ MIB_get_dbgBTTime(FTE_VOID_PTR dummy)
{
    return  _unknown;
}

/******************************************************************************
 * for MIB
 ******************************************************************************/
const RTCSMIB_VALUE MIBVALUE_futuretek = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_fts = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_p5 = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_p5Desc = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_p5Config = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_descDevice = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_descSensors = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_descControls = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_descPower = 
{
	RTCSMIB_NODETYPE_DISPSTR_PTR,
	(void _PTR_)"FTS-P5"
};

const RTCSMIB_VALUE MIBVALUE_prodID= 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_productID
};

const RTCSMIB_VALUE MIBVALUE_prodModel= 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_productModel
};

const RTCSMIB_VALUE MIBVALUE_prodVendor = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_vendorID
};

const RTCSMIB_VALUE MIBVALUE_prodHWVer = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_HWVersion
};

const RTCSMIB_VALUE MIBVALUE_prodSWVer = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_SWVersion
};

const RTCSMIB_VALUE MIBVALUE_prodDesc = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_prodDesc
};

const RTCSMIB_VALUE MIBVALUE_netType= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_netType
};

const RTCSMIB_VALUE MIBVALUE_netMacAddr = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_netMacAddr
};

const RTCSMIB_VALUE MIBVALUE_netIpAddr = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_netIpAddr
};

const RTCSMIB_VALUE MIBVALUE_netNetMask = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_netNetMask
};

const RTCSMIB_VALUE MIBVALUE_netGateway = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_netGateway
};

const RTCSMIB_VALUE MIBVALUE_smDiscovery = 
{
	RTCSMIB_NODETYPE_INT_FN,
	(void _PTR_)MIB_get_smDiscovery
};

const RTCSMIB_VALUE MIBVALUE_smDestroyDynamicObject= 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_smDestroyDynamicObject 
};

/******************************************************************************
 * futuretek.fte.endpoints.epDI.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_diCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_diCount
};

const RTCSMIB_VALUE MIBVALUE_diTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_diEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_diIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_diID = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_diType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_diSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_diName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_diState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_diValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_diLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_diLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_diUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objUpdateInterval
};

/******************************************************************************
 * futuretek.fts.endpoints.epTemperature.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_tempCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_tempCount
};

const RTCSMIB_VALUE MIBVALUE_tempTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_tempEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_tempIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_tempID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_tempType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_tempSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_tempName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_tempState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_tempValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_tempLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_tempLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_tempUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objUpdateInterval
};

/******************************************************************************
 * futuretek.fts.endpoints.epHumidity.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_humiCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_humiCount
};

const RTCSMIB_VALUE MIBVALUE_humiTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_humiEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_humiIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_humiID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_humiType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_humiSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_humiName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_humiState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_humiValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_humiLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_humiLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_humiUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_objUpdateInterval
};

/******************************************************************************
 * futuretek.fts.endpoints.epVoltage.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_vltCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_vltCount
};

const RTCSMIB_VALUE MIBVALUE_vltTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_vltEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_vltIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_vltID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_vltType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_vltSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_vltName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_vltState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_vltValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_vltLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_vltLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_vltUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_objUpdateInterval
};

/******************************************************************************
 * futuretek.fts.endpoints.epCurrent.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_currCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_currCount
};

const RTCSMIB_VALUE MIBVALUE_currTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_currEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_currIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_currID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_currType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_currSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_currName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_currState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_currValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_currLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_currLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_currUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_objUpdateInterval
};


/******************************************************************************
 * futuretek.fts.endpoints.epDO.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_doCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_doCount
};

const RTCSMIB_VALUE MIBVALUE_doTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_doEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_doIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_doID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_doType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_doInitState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_doInitState
};

const RTCSMIB_VALUE MIBVALUE_doName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_doSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_doState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_doValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_doLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_doLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_doInitValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objInitValue
};

const RTCSMIB_VALUE MIBVALUE_rlCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_rlCount
};

const RTCSMIB_VALUE MIBVALUE_rlTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_rlEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_rlIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_rlID = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_rlType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_rlName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_rlSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_rlState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_rlValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_rlLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_rlLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_rlInitValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objInitValue
}; 

/*************************************************************
* futuretek.fts.endpoints.epPower.xxx
 *************************************************************/

const RTCSMIB_VALUE MIBVALUE_pwrCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_pwrCount
};

const RTCSMIB_VALUE MIBVALUE_pwrTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_pwrEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_pwrIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_pwrID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_pwrType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_pwrSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_pwrName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_pwrState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_pwrValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_pwrLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_pwrLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_pwrUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objUpdateInterval
};

/*************************************************************
 * futuretek.fts.endpoints.epGAS.xxx
 *************************************************************/

const RTCSMIB_VALUE MIBVALUE_gasCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_gasCount
};

const RTCSMIB_VALUE MIBVALUE_gasTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_gasEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_gasIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_gasID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_gasType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_gasSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_gasName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_gasState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_gasValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_gasLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_gasLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_gasUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objUpdateInterval
};

const RTCSMIB_VALUE MIBVALUE_gasTotalTrial = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objTotalTrial
};

const RTCSMIB_VALUE MIBVALUE_gasTotalFailed = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objTotalFailed
};

/*************************************************************
 * futuretek.fts.endpoints.epDust.xxx
 *************************************************************/

const RTCSMIB_VALUE MIBVALUE_dustCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_dustCount
};

const RTCSMIB_VALUE MIBVALUE_dustTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_dustEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_dustIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_dustID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_dustType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_dustSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_dustName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_dustState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_dustValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_dustLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_dustLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_dustUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objUpdateInterval
};

const RTCSMIB_VALUE MIBVALUE_dustTotalTrial = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objTotalTrial
};

const RTCSMIB_VALUE MIBVALUE_dustTotalFailed = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_objTotalFailed
};

/******************************************************************************
 *
 ******************************************************************************/
const RTCSMIB_VALUE MIBVALUE_cntCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_cntCount
};

const RTCSMIB_VALUE MIBVALUE_cntTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_cntEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_cntIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_cntID = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_cntType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_cntName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_cntSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_cntState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_cntValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_cntLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_cntLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_cntInitValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objInitValue
}; 

/******************************************************************************
 * futuretek.fts.endpoints.epPressure.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_prsCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_prsCount
};

const RTCSMIB_VALUE MIBVALUE_prsTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_prsEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_prsIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_prsID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_prsType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_prsSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_prsName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_prsState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_prsValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_prsLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_prsLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_prsUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_objUpdateInterval
};


const RTCSMIB_VALUE MIBVALUE_prsInitValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objInitValue
}; 

/******************************************************************************
 * futuretek.fts.endpoints.epDiscrete.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_dscCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_dscCount
};

const RTCSMIB_VALUE MIBVALUE_dscTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_dscEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_dscIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_dscID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_dscType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_dscSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_dscName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_dscState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_dscValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

const RTCSMIB_VALUE MIBVALUE_dscLastValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastValue
};

const RTCSMIB_VALUE MIBVALUE_dscLastTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objLastTime
};

const RTCSMIB_VALUE MIBVALUE_dscUpdateInterval = 
{
	RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_objUpdateInterval
};


const RTCSMIB_VALUE MIBVALUE_dscInitValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objInitValue
}; 

/******************************************************************************
 * futuretek.fts.endpoints.epMulti.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_multiCount = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_multiCount
};

const RTCSMIB_VALUE MIBVALUE_multiTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_multiEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_multiIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_multiID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_multiType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_multiSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_multiName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_multiState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_multiValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};

/******************************************************************************
 * futuretek.fts.endpoints.epDevice.xxx
 ******************************************************************************/

const RTCSMIB_VALUE MIBVALUE_devCount = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_devCount
};

const RTCSMIB_VALUE MIBVALUE_devTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_devEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_devIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_devID= 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objID
};

const RTCSMIB_VALUE MIBVALUE_devType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objType
};

const RTCSMIB_VALUE MIBVALUE_devSN = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objSN
};

const RTCSMIB_VALUE MIBVALUE_devName = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objName
};

const RTCSMIB_VALUE MIBVALUE_devState = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objState
};

const RTCSMIB_VALUE MIBVALUE_devValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_objValue
};


/******************************************************************************
 *
 ******************************************************************************/
const RTCSMIB_VALUE MIBVALUE_eventCount= 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_eventCount
};

const RTCSMIB_VALUE MIBVALUE_eventTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_eventEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_eventIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_eventEPID = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_eventEPID
};

const RTCSMIB_VALUE MIBVALUE_eventLevel = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_eventLevel
};

const RTCSMIB_VALUE MIBVALUE_eventType = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_eventType
};

const RTCSMIB_VALUE MIBVALUE_eventCondition = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_eventCondition
};

    /******************************************************************************
 *
 ******************************************************************************/
const RTCSMIB_VALUE MIBVALUE_svrTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_svrEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_svrIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_svrIpAddr = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_srvIpAddr
};

const RTCSMIB_VALUE MIBVALUE_logCount = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_logCount
};

const RTCSMIB_VALUE MIBVALUE_logTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_logEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_logIndex = 
{
    RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_logIndex
};

const RTCSMIB_VALUE MIBVALUE_logTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_logTime
};

const RTCSMIB_VALUE MIBVALUE_logID = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_logID
};

const RTCSMIB_VALUE MIBVALUE_logValue = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_logValue
};

const RTCSMIB_VALUE MIBVALUE_logLevel = 
{
    RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_logLevel
};

const RTCSMIB_VALUE MIBVALUE_logString = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_logString
};


const RTCSMIB_VALUE MIBVALUE_psType = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_psVoltageMax = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_psVoltageMin = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};


const RTCSMIB_VALUE MIBVALUE_tsTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_tsEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_tsIndex = 
{
    RTCSMIB_NODETYPE_INT_CONST,
    NULL
};

const RTCSMIB_VALUE MIBVALUE_tsIpAddr = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_tsIpAddr
};

const RTCSMIB_VALUE MIBVALUE_tsCount = 
{
	RTCSMIB_NODETYPE_UINT_FN,
	(void _PTR_)MIB_get_tsCount
};

const RTCSMIB_VALUE MIBVALUE_tsAdd = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_tsAdd
};

const RTCSMIB_VALUE MIBVALUE_tsDel = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_tsDel
};

const RTCSMIB_VALUE MIBVALUE_msgDiscovery = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_msgDiscovery
};

const RTCSMIB_VALUE MIBVALUE_msgAlert = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_msgAlert
};

/******************************************************************************
 * Administration 
 ******************************************************************************/
const RTCSMIB_VALUE MIBVALUE_adminOID = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_productID
};

const RTCSMIB_VALUE MIBVALUE_adminMAC = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_adminMAC
};

const RTCSMIB_VALUE MIBVALUE_adminFactoryReset = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_adminFactoryReset
};

const RTCSMIB_VALUE MIBVALUE_adminSystemTime = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_adminSystemTime
};


const RTCSMIB_VALUE MIBVALUE_adminReset = 
{
	RTCSMIB_NODETYPE_DISPSTR_FN,
	(void _PTR_)MIB_get_adminReset
};

const RTCSMIB_VALUE MIBVALUE_dbgBTTable = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_dbgBTEntry = 
{
	RTCSMIB_NODETYPE_INT_CONST,
	NULL
};

const RTCSMIB_VALUE MIBVALUE_dbgBTIndex = 
{
    RTCSMIB_NODETYPE_UINT_FN,
    (void _PTR_)MIB_get_dbgBTIndex
};

const RTCSMIB_VALUE MIBVALUE_dbgBTTime = 
{
    RTCSMIB_NODETYPE_DISPSTR_FN,
    (void _PTR_)MIB_get_dbgBTTime
};

/******************************************************************************
 * Shell command
 ******************************************************************************/
FTE_INT32  FTE_SNMPD_SHELL_cmd(FTE_INT32 argc, FTE_CHAR_PTR argv[] )
{
    FTE_UINT32  result;
    FTE_BOOL  print_usage, shorthelp = FALSE;
    FTE_INT32   return_code = SHELL_EXIT_SUCCESS;
 
    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)  
    {
        if (argc == 2)  
        {
            if (strcmp(argv[1], "start") == 0)
            {
                result = SNMP_init(FTE_NET_SNMP_NAME, FTE_NET_SNMP_PRIO, FTE_NET_SNMP_STACK);
                if (result ==  0)  
                {
                    printf("SNMP Agent Started.\n");
                    /* Install some MIBs for the SNMP agent */
#if FTE_NET_SNMP_MIB1213
                    /* init RFC 1213 MIB */
                    MIB1213_init();
#endif
    
#if FTE_NET_SNMP_MIBMQX
                    /* init MQX MIB */
                    MIBMQX_init();
#endif
                } 
                else  
                {
                    printf("Unable to start SNMP Agent, error = 0x%x\n",result);
                    return_code = SHELL_EXIT_ERROR;
                }
            } 
            else if (strcmp(argv[1], "stop") == 0)  
            {
                result = SNMP_stop();
                if (result ==  0)  
                {
                    printf("SNMP Agent Stopped.\n");
                }
                else  
                {
                    printf("Unable to stop SNMP Agent, error = 0x%x\n",result);
                    return_code = SHELL_EXIT_ERROR;
                }
            } 
            else  
            {
                printf("Error, %s invoked with incorrect option\n", argv[0]);
                print_usage = TRUE;
            }
        } 
        else if (argc == 3)
        {
            if (strcmp(argv[1], "trap") == 0)
            {
                if (strcmp(argv[2], "list") == 0)
                { 
                    FTE_UINT32     i;
                    
                    for( i = 0 ; i < FTE_CFG_NET_TRAP_count() ; i++)
                    {
                        _ip_address xIP = FTE_CFG_NET_TRAP_getAt(i);
                        
                        printf("%d: %d.%d.%d.%d\n", i+1, IPBYTES(xIP));
                    }
                }
            }
            else  
            {
                printf("Error, %s invoked with incorrect option\n", argv[0]);
                print_usage = TRUE;
            }
        }
        else if (argc == 4)
        {
            if (strcmp(argv[1], "trap") == 0)
            {
                if (strcmp(argv[2], "add") == 0)
                {
                    _ip_address ip;
                    
                    if (FTE_strToIP(argv[3], &ip) != FTE_RET_OK)
                    {
                        printf ("Error!, invalid ip address!\n");
                        
                        print_usage = TRUE;
                        return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (FTE_SNMPD_TRAP_add(ip, TRUE) != FTE_RET_OK)
                    {
                        return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }                    
                }
                else if (strcmp(argv[2], "del") == 0)
                {
                    _ip_address ip;
                    
                    if (FTE_strToIP(argv[3], &ip) != FTE_RET_OK)
                    {
                        printf ("Error!, invalid ip address!\n");
                        
                        print_usage = TRUE;
                        return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (FTE_SNMPD_TRAP_del(ip) != FTE_RET_OK)
                    {
                        return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }                    
                }
                else if (strcmp(argv[2], "send") == 0)
                {
                    FTE_SNMPD_TRAP_sendAlert(FTE_ALERT_INFO, TRUE);
                }
                else if (strcmp(argv[2], "discovery") == 0)
                {
                    _ip_address ip;
                    
                    if (FTE_strToIP(argv[3], &ip) != FTE_RET_OK)
                    {
                        printf ("Error!, invalid ip address!\n");
                        
                        print_usage = TRUE;
                        return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    FTE_SNMPD_TRAP_discovery(ip);
                }
                else
                {
                    print_usage = TRUE;
                    return_code = SHELL_EXIT_ERROR;
                    goto error;
                }
            }
        }
        else
        {
            printf("Error, %s invoked with incorrect number of arguments\n", argv[0]);
            print_usage = TRUE;
        }
    }
    
error:    
    if (print_usage)  
    {
        if (shorthelp)  
        {
            printf("%s [start | stop]\n", argv[0]);
        } 
        else  
        {
            printf("Usage: %s [start | stop]\n",argv[0]);
        }
    }
    
    
   return return_code;
} /* Endbody */

#endif  /* FTE_SNMPD_SUPPORTED */

/* EOF */
