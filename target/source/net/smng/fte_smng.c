#include "fte_target.h"
#include "fte_net.h"
#include "fte_task.h"
#include "fte_config.h"
#include "fte_sys_bl.h"
#include "fte_time.h"
#include <rtcs.h>

#define FTE_SMNG_MSG_TYPE_DISCOVERY             0x01
#define FTE_SMNG_MSG_TYPE_DISCOVERY_WITH_TRAPS  0x02
#define FTE_SMNG_MSG_TYPE_RESET                 0x03
#define FTE_SMNG_MSG_TYPE_UPGRADE               0x04

#define SMNG_TRACE(...)  TRACE(DEBUG_NET_SMNG, __VA_ARGS__)

typedef struct
{
    FTE_UINT32     nMsgType;    
}   FTE_SMNG_PACKET, _PTR_ FTE_SMNG_PACKET_PTR;

static  FTE_CHAR_PTR    pDiscoveryMsg = NULL;

void FTE_SMNG_task(pointer pParams, pointer pCreator)
{
    FTE_CHAR        _pBuff[1024];
    FTE_UINT32      hSock = socket(PF_INET, SOCK_DGRAM, 0);
    FTE_UINT32      bOption;
    sockaddr_in     xAnyAddr, xRecvAddr;
    FTE_UINT16      nRecvAddrLen;
    FTE_UINT32      nLen;
    _ip_address     xLastRequestHost;
    TIME_STRUCT     xLastResponseTime;


    xAnyAddr.sin_family      = AF_INET;
    xAnyAddr.sin_port        = FTE_NET_SMNG_PORT;
    xAnyAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (hSock == RTCS_SOCKET_ERROR) 
    {
        RTCS_task_exit(pCreator, RTCSERR_OUT_OF_SOCKETS);
    } 
   
    /* Begin ENGR00243356 */
    /* Set socket options to no wait */
    bOption = TRUE;
    setsockopt(hSock, SOL_UDP, OPT_SEND_NOWAIT, &bOption, sizeof(bOption));
    /* End ENGR00243356 */  

    bind(hSock, &xAnyAddr, sizeof(xAnyAddr));
    nRecvAddrLen = sizeof(xRecvAddr);
    memset(&xRecvAddr, 0, nRecvAddrLen);
    
    
    RTCS_task_resume_creator(pCreator, RTCS_OK);

    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id());

    _time_get(&xLastResponseTime);

    while(1)
    {
        FTE_UINT32 nMsgType;
        
        memset(_pBuff, 0, sizeof(_pBuff));
        nLen = recvfrom(hSock, _pBuff, sizeof(_pBuff), 0, &xRecvAddr, &nRecvAddrLen);
        if (nLen <= 0)
        {
            continue;
        }

        FTE_NET_liveTouch();

        if (strcmp(_pBuff, "Hello?") == 0)
        {
            nMsgType  = FTE_SMNG_MSG_TYPE_DISCOVERY_WITH_TRAPS;
        }
        else if (strcmp(_pBuff, "Reset") == 0)
        {
            nMsgType  = FTE_SMNG_MSG_TYPE_RESET;
        }
        else if (strcmp(_pBuff, "Upgrade") == 0)
        {
            nMsgType  = FTE_SMNG_MSG_TYPE_UPGRADE;
        }
        else
        {
            nMsgType = ntohl(&((FTE_SMNG_PACKET_PTR)_pBuff)->nMsgType);
        }
        
        switch(nMsgType)
        {
        case    FTE_SMNG_MSG_TYPE_DISCOVERY_WITH_TRAPS:
            {
                TIME_STRUCT xCurrentTime;
                FTE_INT32   nDiffTime;
                
                _time_get(&xCurrentTime);
                
                FTE_TIME_diff(&xCurrentTime, &xLastResponseTime, &nDiffTime);
                
                if ((xLastRequestHost != xRecvAddr.sin_addr.s_addr) || ( nDiffTime > 10))
                {
#if FTE_SNMPD_SUPPORTED
                    FTE_SNMPD_TRAP_discovery(xRecvAddr.sin_addr.s_addr);
                    xLastResponseTime = xCurrentTime;
                    xLastRequestHost = xRecvAddr.sin_addr.s_addr;
#endif
                }
            }
            break;

        case    FTE_SMNG_MSG_TYPE_RESET:
            {
                _time_delay(1000);
                FTE_SYS_reset();
            }
            break;
            
        case    FTE_SMNG_MSG_TYPE_UPGRADE:
            {
                _time_delay(1000);
                FTE_SYS_BL_startUpgrade();
            }
            break;
        }
    }
    
}

FTE_RET FTE_SMNGD_init
(
    FTE_VOID_PTR    pParams
)
{ 
   TRACE_ON(DEBUG_NET_SMNG);
#if 0   
    FTE_NET_CFG_PTR     pCfgNet = FTE_CFG_NET_get();
    FTE_JSON_VALUE_PTR  pObject;
    FTE_JSON_VALUE_PTR  pValue;
    FTE_JSON_VALUE_PTR  pOIDs;
    _enet_address       xMACAddress;
    FTE_CHAR            pMACString[20];
    FTE_UINT32             ulMsgLen;

    pObject = FTE_JSON_VALUE_createObject(3);
    if (pObject == NULL)
    {
        return  FTE_RET_ERROR;
    }

    pValue = FTE_JSON_VALUE_createString(FTE_SYS_getOIDString());
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pObject, "id", pValue);

    FTE_SYS_getMAC(xMACAddress);
    snprintf(pMACString, sizeof(pMACString), "%02x:%02x:%02x:%02x:%02x:%02x", 
                        xMACAddress[0], xMACAddress[1],
                        xMACAddress[2], xMACAddress[3],
                        xMACAddress[4], xMACAddress[5]);
    pValue = FTE_JSON_VALUE_createString(pMACString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pObject, "mac", pValue);

    pOIDs = FTE_JSON_VALUE_createArray(FTE_OBJ_DESC_CLASS_count() + FTE_DEVICE_count());

    for(FTE_UINT32 i = 0 ; i < FTE_OBJ_DESC_CLASS_count() ; i++)
    {
        FTE_CHAR pOID[32];
        FTE_UINT32 ulClass = FTE_OBJ_DESC_CLASS_getAt(i);
        
        if (ulClass != 0)
        {
            sprintf(pOID, "%d", ulClass >> 16);
            pValue  = FTE_JSON_VALUE_createString(pOID);
            if (pValue != NULL)
            {
                FTE_JSON_ARRAY_setElement((FTE_JSON_ARRAY_PTR)pOIDs, pValue);
            }
        }
    }

    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pObject, "oids", pOIDs);
    ulMsgLen = FTE_JSON_VALUE_buffSize(pObject) + 1;            
    pDiscoveryMsg = (FTE_CHAR_PTR)FTE_MEM_alloc(ulMsgLen);
    if (pDiscoveryMsg == NULL)
    {
        ERROR("Not enough memory!\n");
        FTE_SYS_setUnstable();
        return  FTE_RET_ERROR;
    }
        
    FTE_JSON_VALUE_snprint(pDiscoveryMsg, ulMsgLen, pObject);            
    FTE_JSON_VALUE_destroy(pObject);   
#endif
    return RTCS_task_create("smng", FTE_NET_SMNG_PRIO, FTE_NET_SMNG_STACK, FTE_SMNG_task, NULL);
} 

FTE_RET FTE_SMNG_getDiscoveryMessage
(
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffSize
)
{
    ASSERT(pBuff != NULL);
    
    if (pDiscoveryMsg == NULL)
    {
        FTE_RET xRet;
        FTE_NET_CFG_PTR     pCfgNet;
        FTE_JSON_VALUE_PTR  pObject;
        FTE_JSON_VALUE_PTR  pValue;
        FTE_JSON_VALUE_PTR  pOIDs;
        _enet_address       xMACAddress;
        FTE_CHAR            pMACString[20];
        FTE_CHAR            pIPString[20];
        FTE_UINT32             ulMsgLen;
        IPCFG_IP_ADDRESS_DATA   xIPData;

        xRet = FTE_CFG_NET_get(&pCfgNet);
        if (xRet != FTE_RET_OK)
        {
            pBuff[0] = '\0';
            return  xRet;
        }
        
        pObject = FTE_JSON_VALUE_createObject(4);
        if (pObject == NULL)
        {
            pBuff[0] = '\0';
            return  FTE_RET_NOT_ENOUGH_MEMORY;
        }

        pValue = FTE_JSON_VALUE_createString(FTE_SYS_getOIDString());
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pObject, "id", pValue);

        FTE_SYS_getMAC(xMACAddress);
        snprintf(pMACString, sizeof(pMACString), "%02x:%02x:%02x:%02x:%02x:%02x", 
                            xMACAddress[0], xMACAddress[1],
                            xMACAddress[2], xMACAddress[3],
                            xMACAddress[4], xMACAddress[5]);
        pValue = FTE_JSON_VALUE_createString(pMACString);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pObject, "mac", pValue);

        ipcfg_get_ip (BSP_DEFAULT_ENET_DEVICE, &xIPData);
        snprintf(pIPString, sizeof(pIPString), "%d.%d.%d.%d", IPBYTES(xIPData.ip));
        pValue = FTE_JSON_VALUE_createString(pIPString);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pObject, "ip", pValue);        

        pOIDs = FTE_JSON_VALUE_createArray(FTE_OBJ_DESC_CLASS_count() + FTE_DEVICE_count());

        for(FTE_UINT32 i = 0 ; i < FTE_OBJ_DESC_CLASS_count() ; i++)
        {
            FTE_CHAR pOID[32];
            FTE_UINT32 ulClass = FTE_OBJ_DESC_CLASS_getAt(i);
            
            if (ulClass != 0)
            {
                sprintf(pOID, "%d", ulClass >> 16);
                pValue  = FTE_JSON_VALUE_createString(pOID);
                if (pValue != NULL)
                {
                    FTE_JSON_ARRAY_setElement((FTE_JSON_ARRAY_PTR)pOIDs, pValue);
                }
            }
        }

        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pObject, "oids", pOIDs);
        ulMsgLen = FTE_JSON_VALUE_buffSize(pObject) + 1;            
        pDiscoveryMsg = (FTE_CHAR_PTR)FTE_MEM_alloc(ulMsgLen);
        if (pDiscoveryMsg == NULL)
        {
            ERROR("Not enough memory!\n");
            FTE_SYS_setUnstable();
            return  FTE_RET_NOT_ENOUGH_MEMORY;
        }
            
        FTE_JSON_VALUE_snprint(pDiscoveryMsg, ulMsgLen, pObject);            
        FTE_JSON_VALUE_destroy(pObject);   
    }
    
    strncpy(pBuff, pDiscoveryMsg, ulBuffSize);
    
    return  FTE_RET_OK;
}


/******************************************************************************
 * Shell command
 ******************************************************************************/
FTE_INT32  FTE_SMNGD_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{
    FTE_BOOL  bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
 
    bPrintUsage = Shell_check_help_request(nArgc, pArgv, &bShortHelp );

    if (!bPrintUsage)  
    {
        switch(nArgc)  
        {
        case    3:
            {
                if (strcmp(pArgv[1], "class") == 0)
                {
                    if (strcmp(pArgv[2], "list") == 0)
                    {
                        FTE_UINT32 i,j, ulCount;
                        FTE_UINT32 pulClassIDs[16];

                        ulCount = FTE_OBJ_DESC_CLASS_count();
                        
                        for(i = 0 ; i < ulCount ; i++)
                        {
                            pulClassIDs[i] = FTE_OBJ_DESC_CLASS_getAt(i);
                        }

                        for(i = 0 ; i < ulCount - 1 ; i++)
                        {
                            for(j = i+1 ; j < ulCount ; j++)
                            {
                                if (pulClassIDs[i] > pulClassIDs[j])
                                {
                                    FTE_UINT32 ulTemp = pulClassIDs[i];
                                    pulClassIDs[i] = pulClassIDs[j];
                                    pulClassIDs[j] = ulTemp;
                                }
                            }
                        }
                        
                        for(i = 0 ; i < ulCount ; i++)
                        {
                            FTE_UINT32 ulClass = pulClassIDs[i];
                            FTE_CHAR pClassName[32];
                            
                            FTE_OBJ_CLASS_getName(ulClass, pClassName, sizeof(pClassName));
                            printf("%d : %6d %s\n", i + 1, ulClass >> 16, pClassName);
                        }
                    }
                }
            }
            break;
            
        case    4:
            {
                if (strcmp(pArgv[1], "class") == 0)
                {
                    if (strcmp(pArgv[2], "hide") == 0)
                    {
                        FTE_UINT32 ulClass;
                        
                        if (FTE_strToHex(pArgv[3], &ulClass) != FTE_RET_OK)
                        {
                            printf("Invalid Class ID[%s]\n", pArgv[3]);
                            xRet = SHELL_EXIT_ERROR;
                            break;
                        }

                    }
                }
            }
            break;
            
        default:
            bPrintUsage = TRUE;
            goto error;
        }
    }
    
error:    
    if (bPrintUsage)  
    {
        if (bShortHelp)  
        {
            printf("%s <command>\n", pArgv[0]);
        } 
        else  
        {
            printf("Usage: %s <command>\n",pArgv[0]);
            printf("  Commands:\n");
            printf("    class list\n");
            printf("        Supported object type list\n");
        }
    }
    
    
   return xRet;
} /* Endbody */
