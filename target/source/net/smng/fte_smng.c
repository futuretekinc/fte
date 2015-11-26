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
    uint_32     nMsgType;    
}   FTE_SMNG_PACKET, _PTR_ FTE_SMNG_PACKET_PTR;

static  char_ptr    pDiscoveryMsg = NULL;

void FTE_SMNG_task(pointer pParams, pointer pCreator)
{
    char        _pBuff[1024];
    uint_32     hSock = socket(PF_INET, SOCK_DGRAM, 0);
    uint_32     bOption;
    sockaddr_in xAnyAddr, xRecvAddr;
    uint_16     nRecvAddrLen;
    uint_32     nLen;
    _ip_address xLastRequestHost;
    TIME_STRUCT xLastResponseTime;


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
        uint_32 nMsgType;
        
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
                
                _time_get(&xCurrentTime);
                
                if ((xLastRequestHost != xRecvAddr.sin_addr.s_addr) || (FTE_TIME_diff(&xCurrentTime, &xLastResponseTime) > 10))
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

uint_32 FTE_SMNGD_init(void _PTR_ Params)
{ 
   TRACE_ON(DEBUG_NET_SMNG);
   
    FTE_NET_CFG_PTR     pCfgNet = FTE_CFG_NET_get();
    FTE_JSON_VALUE_PTR  pObject;
    FTE_JSON_VALUE_PTR  pValue;
    FTE_JSON_VALUE_PTR  pOIDs;
    _enet_address       xMACAddress;
    char                pMACString[20];
    uint_32             ulMsgLen;

    pObject = FTE_JSON_VALUE_createObject(3);
    if (pObject == NULL)
    {
        return  MQX_ERROR;
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

    for(uint_32 i = 0 ; i < FTE_OBJ_DESC_CLASS_count() ; i++)
    {
        char    pOID[32];
        uint_32 ulClass = FTE_OBJ_DESC_CLASS_getAt(i);
        
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
    pDiscoveryMsg = (char_ptr)FTE_MEM_alloc(ulMsgLen);
    if (pDiscoveryMsg == NULL)
    {
        ERROR("Not enough memory!\n");
        return  MQX_ERROR;
    }
        
    FTE_JSON_VALUE_snprint(pDiscoveryMsg, ulMsgLen, pObject);            
    FTE_JSON_VALUE_destroy(pObject);   
    
    return RTCS_task_create("smng", FTE_NET_SMNG_PRIO, FTE_NET_SMNG_STACK, FTE_SMNG_task, NULL);
} 

char_ptr FTE_SMNG_getDiscoveryMessage(void)
{
    return  pDiscoveryMsg;
}


/******************************************************************************
 * Shell command
 ******************************************************************************/
int_32  FTE_SMNGD_SHELL_cmd(int_32 argc, char_ptr argv[] )
{
    boolean  print_usage, shorthelp = FALSE;
    int_32   return_code = SHELL_EXIT_SUCCESS;
 
    print_usage = Shell_check_help_request(argc, argv, &shorthelp );

    if (!print_usage)  
    {
        switch(argc)  
        {
        case    3:
            {
                if (strcmp(argv[1], "class") == 0)
                {
                    if (strcmp(argv[2], "list") == 0)
                    {
                        uint_32 i,j, ulCount;
                        uint_32 pulClassIDs[16];

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
                                    uint_32 ulTemp = pulClassIDs[i];
                                    pulClassIDs[i] = pulClassIDs[j];
                                    pulClassIDs[j] = ulTemp;
                                }
                            }
                        }
                        
                        for(i = 0 ; i < ulCount ; i++)
                        {
                            uint_32 ulClass = pulClassIDs[i];
                            char    pClassName[32];
                            
                            FTE_OBJ_CLASS_getName(ulClass, pClassName, sizeof(pClassName));
                            printf("%d : %6d %s\n", i + 1, ulClass >> 16, pClassName);
                        }
                    }
                }
            }
            break;
            
        case    4:
            {
                if (strcmp(argv[1], "class") == 0)
                {
                    if (strcmp(argv[2], "hide") == 0)
                    {
                        uint_32 ulClass;
                        
                        if (Shell_parse_hexnum(argv[3], &ulClass) != TRUE)
                        {
                            printf("Invalid Class ID[%s]\n", argv[3]);
                            return_code = SHELL_EXIT_ERROR;
                            break;
                        }

                    }
                }
            }
            break;
            
        default:
            print_usage = TRUE;
            goto error;
        }
    }
    
error:    
    if (print_usage)  
    {
        if (shorthelp)  
        {
            printf("%s <command>\n", argv[0]);
        } 
        else  
        {
            printf("Usage: %s <command>\n",argv[0]);
            printf("  Commands:\n");
            printf("    class list\n");
            printf("        Supported object type list\n");
        }
    }
    
    
   return return_code;
} /* Endbody */
