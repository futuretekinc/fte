#include "fte_target.h"
#include "fte_net.h"
#include "fte_task.h"
#include "fte_config.h"
#include "fte_sys_bl.h"
#include "fte_time.h"
#include "fte_modbus.h"
#include "fte_mbtcp.h"
#include <rtcs.h>

#undef  __MODULE__
#define __MODULE__  FTE_MODULE_NET_MBTCP

void FTE_MBTCP_SLAVE_task
(
    FTE_VOID_PTR    pParams, 
    FTE_VOID_PTR    pCreator
)
{
    ASSERT(pParams != NULL);
    
    FTE_RET         xRet;
    sockaddr_in     xAnyAddr;
    FTE_MBTCP_SLAVE_PTR pSlave = (FTE_MBTCP_SLAVE_PTR)pParams;

    xRet = FTE_MODBUS_SLAVE_init( &pSlave->xMBSlave, &pSlave->xConfig.xMBSlave);
    if (xRet != FTE_RET_OK)
    {
        ERROR("MODBUS init failed!\n");
        RTCS_task_exit(pCreator, RTCSERR_OUT_OF_SOCKETS);
    }
    
    RTCS_task_resume_creator(pCreator, RTCS_OK);
    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id());

    pSlave->hSock = socket(PF_INET, SOCK_STREAM, 0);
    if (pSlave->hSock == RTCS_SOCKET_ERROR) 
    {
        ERROR("Socket error!\n");
        RTCS_task_exit(pCreator, RTCSERR_OUT_OF_SOCKETS);
    } 

    xAnyAddr.sin_family      = AF_INET;
    xAnyAddr.sin_port        = pSlave->xConfig.usPort;
    xAnyAddr.sin_addr.s_addr = INADDR_ANY;
    
    xRet = bind(pSlave->hSock, &xAnyAddr, sizeof(xAnyAddr));
    if (xRet != RTCS_OK)
    {
        ERROR("Unable to bind IPv4 socket.\n");
        RTCS_task_exit(pCreator, RTCSERR_IP_BIND_ADDR);
    }
    
    xRet = listen(pSlave->hSock, 0);
    if (xRet != RTCS_OK)
    {
        ERROR("Unable to put IPv4 socket in listening state.\n");
        RTCS_task_exit(pCreator, RTCSERR_CREATE_FAILED);
    }

    
   _lwsem_create(&pSlave->xLWSEM, 0);
    
    while(TRUE)
    {
        FTE_INT     i;
        FTE_UINT16  usLen;
        FTE_UINT32  bOption;
        FTE_UINT32  hConnSock;
        FTE_MBTCP_SLAVE_SESSION_PTR pSession = NULL;
        
        for(i = 0 ; i < pSlave->xConfig.ulMaxSessions ; i++)
        {
            if (pSlave->pSessions[i].hTask == 0)
            {
                pSession = &pSlave->pSessions[i];
            }
        }
        
        hConnSock = RTCS_selectset(&pSlave->hSock, 1, 0);
        if (hConnSock == RTCS_SOCKET_ERROR)
        {
            ERROR("Fatal Error: Unable to determine active socket.");
            _task_block();  
        }
        
        pSession->pParent  = pSlave;
        pSession->pMBSlave = &pSlave->xMBSlave;
        
        /* Accept incoming connection */
        usLen = sizeof(pSession->xRemoteAddr);
        pSession->hSock = accept(hConnSock, &pSession->xRemoteAddr, &usLen);
       if (pSession->hSock == RTCS_SOCKET_ERROR)
       {
           FTE_UINT32   ulStatus;
           ulStatus = RTCS_geterror(pSession->hSock);
           ERROR("Fatal Error: Unable to accept incoming connection. ");
           if (ulStatus == RTCS_OK)
           {
               TRACE("Connection reset by peer.");
           }
           else 
           {
               ERROR("Accept() failed with error code 0x%X.\n", ulStatus);
           }
           _task_block();
        }
        
        /* Set socket options */
        bOption = TRUE;
        xRet = setsockopt(pSession->hSock, SOL_TCP, OPT_RECEIVE_NOWAIT, (FTE_CHAR_PTR)&bOption, sizeof(bOption));
        if (xRet != RTCS_OK)
        {
           ERROR("Fatal Error: Unable to set socket options.", stderr);
            _task_block();
        }

        
        _lwsem_create(&pSession->xLWSEM, 0);
        
        pSession->hTask = _task_create(0, FTE_TASK_MBTCP_SESSION, (uint_32) pSession);
        if (pSession->hTask == MQX_NULL_TASK_ID)
        {
           ERROR("Fatal Error 0x%X: Unable to create receive task.", _task_get_error());
           _task_block();
        }

        _lwsem_wait(&pSession->xLWSEM);
       
        xRet = _task_destroy(pSession->hTask);
        if (xRet != MQX_OK)
        {
            ERROR("Fatal Error: Unable to destroy rxtask.", stderr);
            _task_block();
        }
        pSession->hTask = 0;
        
        _time_delay(10);
    }
    
}

void FTE_MBTCP_sessionTask
(
    FTE_UINT32  xParam
)
{
    ASSERT(xParam != 0);
        
    FTE_RET     xRet;
    FTE_MBTCP_SLAVE_SESSION_PTR pSession = (FTE_MBTCP_SLAVE_SESSION_PTR)xParam;
    FTE_MBTCP_SLAVE_PTR         pMBTCPSlave = pSession->pParent;
    FTE_UINT32  ulReqPDULen;
    FTE_UINT32  ulRespPDULen;
    FTE_UINT8   pReqPDU[FTE_MODBUS_PDU_LEN];    
    FTE_UINT8   pRespPDU[FTE_MODBUS_PDU_LEN];    
    
    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id());
    
    FTE_TIME_getCurrent(&pSession->xCreationTime);
    
    while(TRUE)
    {        
        xRet = FTE_MBTCP_SLAVE_receiveFrame(pSession, pReqPDU, sizeof(pReqPDU), &ulReqPDULen);
        if (xRet == FTE_RET_SOCKET_CLOSED)
        {
            shutdown(pSession->hSock, 0);
            _lwsem_post(&pSession->xLWSEM);
            _task_block();
            break;
        }
        
        pMBTCPSlave->xMBSlave.xStatistics.ulRxPkt++;
        
        if (xRet == FTE_RET_OK)
        {
            FTE_TIME_getCurrent(&pSession->xLastTime);
            
            xRet = FTE_MODBUS_SLAVE_process(pSession->pMBSlave, pReqPDU, ulReqPDULen, pRespPDU, sizeof(pRespPDU), &ulRespPDULen);
            if (xRet == FTE_RET_OK)
            {
                xRet = FTE_MBTCP_SLAVE_sendFrame(pSession, pRespPDU, ulRespPDULen);
                if (xRet == FTE_RET_SOCKET_CLOSED)
                {
                    shutdown(pSession->hSock, 0);
                    _lwsem_post(&pSession->xLWSEM);
                    break;
                }
                
                pMBTCPSlave->xMBSlave.xStatistics.ulTxPkt++;
            }
        }
        else
        {
            pMBTCPSlave->xMBSlave.xStatistics.ulRxError++;
        }
        
        _time_delay(1);
    }
}

static
FTE_MBTCP_SLAVE xMBTCPSlave = 
{
    .xConfig    = 
    {
        .usPort = 0,
        .ulMaxSessions = 0
    },
    .hSock      = 0,
    .pSessions  = NULL,
    .ulSessions = 0
};

FTE_RET FTE_MBTCP_SLAVE_init
(
    FTE_MBTCP_SLAVE_CFG_PTR pConfig
)
{ 
    ASSERT(pConfig != NULL);
    
    TRACE_ON();

    xMBTCPSlave.pSessions = FTE_MEM_allocZero(sizeof(FTE_MBTCP_SLAVE_SESSION) * pConfig->ulMaxSessions);
    if (xMBTCPSlave.pSessions == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    memcpy(&xMBTCPSlave.xConfig, pConfig, sizeof(FTE_MBTCP_SLAVE_CFG));

    return RTCS_task_create("mbtcp", FTE_NET_MBTCP_PRIO, FTE_NET_MBTCP_STACK, FTE_MBTCP_SLAVE_task, &xMBTCPSlave);
} 

FTE_RET FTE_MBTCP_SLAVE_receiveFrame
(
    FTE_MBTCP_SLAVE_SESSION_PTR pSession,
    FTE_UINT8_PTR               pBuff,
    FTE_UINT32                  ulBuffSize,
    FTE_UINT32_PTR              pulLen
)
{
    ASSERT(pSession != NULL);
    FTE_INT nRcvdLen;
    memset(pSession->pBuff, 0, sizeof(pSession->pBuff));
        
    while(TRUE)
    {
        nRcvdLen = recv(pSession->hSock, &pSession->pBuff, sizeof(pSession->pBuff), 0);
        if (nRcvdLen == RTCS_ERROR)
        {
            return  FTE_RET_SOCKET_CLOSED;
        }

        if (nRcvdLen != 0)
        {
            if ((nRcvdLen < 12) || (FTE_MODBUS_PDU_LEN + 12 < nRcvdLen))
            {
                TRACE("Invalid frame length received!\n");
                return  FTE_RET_INVALID_MSG;
            }
            else
            {
                FTE_UINT16  usLen;
                
                usLen = pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_LEN]  << 8U;
                usLen |= pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_LEN+1];

                if (usLen + 6 != nRcvdLen)
                {
                    TRACE("Invalid frame length received!\n");
                    return  FTE_RET_INVALID_MSG;
                }
                else
                {
                    FTE_UINT16  usPID;
                    
                    usPID = pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_PID]  << 8U;
                    usPID |= pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_PID+1];

                    if (usPID != FTE_MBTCP_PROTOCOL_ID)
                    {
                        TRACE("Invalid protocol ID!\n");
                        return  FTE_RET_INVALID_MSG;
                    }
                    else
                    {
                        if (0 != pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_UID])
                        {
                            TRACE("The address mismatched!\n");
                            return  FTE_RET_MODBUS_INVALID_ADDRESS;
                        }
                        else
                        {
                            if (usLen > ulBuffSize)
                            {
                                TRACE("Buffer too small!\n");
                                return  FTE_RET_INVALID_MSG;
                            }                                           
                            else
                            {
                                memcpy(pBuff, &pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_PDU], usLen - 1);
                                *pulLen = usLen - 1;
                                
                                return  FTE_RET_OK;
                            }
                        }
                    }
                }
            }
        }
        
        _time_delay(10);
    }
}
  
FTE_RET FTE_MBTCP_SLAVE_sendFrame
(
    FTE_MBTCP_SLAVE_SESSION_PTR pSession,
    FTE_UINT8_PTR               pBuff,
    FTE_UINT32                  ulBuffLen
)
{
    ASSERT(pSession != NULL);
    FTE_INT nSendLen;
    
    if (ulBuffLen > FTE_MODBUS_PDU_LEN)
    {
        return  FTE_RET_INVALID_MSG_FRAME;
    }
    
    
    pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_UID] = 0;
    pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_PID] = 0;
    pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_PID+ 1 ] = 0;
    pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_LEN] = ((ulBuffLen + 1)>> 8) & 0xFF;
    pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_LEN + 1] = (ulBuffLen + 1) & 0xFF;
    
    memcpy(&pSession->pBuff[FTE_MBTCP_FRAME_OFFSET_PDU], pBuff, ulBuffLen);
    
    nSendLen = send(pSession->hSock, pSession->pBuff, ulBuffLen + 7, 0);    
    if (nSendLen == RTCS_ERROR)
    {
        return  FTE_RET_SOCKET_CLOSED;
    }

    return  FTE_RET_OK;
}
  
FTE_RET FTE_MBTCP_SLAVE_process
(
    FTE_UINT8_PTR   pReqPDU,
    FTE_UINT16      ulReqPDULen,
    FTE_UINT8_PTR   pRespBuff,
    FTE_UINT16      ulRespBuffLen,
    FTE_UINT16_PTR  pRespPDULen
)
{
    ASSERT(pReqPDU != NULL);
    ASSERT(pRespBuff != NULL);
    ASSERT(pRespPDULen != NULL);
    
    switch(pReqPDU[0])
    {
    case    FTE_MODBUS_FUNCTION_READ_COILS:
        {
        }
        break;

    case    FTE_MODBUS_FUNCTION_READ_DISCRETE_INPUTS:
        {
        }
        break;

    case    FTE_MODBUS_FUNCTION_READ_HOLDING_REGISTERS:
        {
        }
        break;

    case    FTE_MODBUS_FUNCTION_READ_INPUT_REGISTERS:
        {
        }
        break;

    }
    return  FTE_RET_OK;
}

FTE_RET FTE_MBTCP_SLAVE_readCoils
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8       pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_UINT32_PTR  pByteCount
)
{
    return  FTE_RET_OK;
}

FTE_RET FTE_MBTCP_SLAVE_readCDiscreteInputs
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8       pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_UINT32_PTR  pByteCount
)
{
    return  FTE_RET_OK;
}

FTE_RET FTE_MBTCP_SLAVE_readHoldingRegisters
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8       pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_UINT32_PTR  pByteCount
)
{
    return  FTE_RET_OK;
}

FTE_RET FTE_MBTCP_SLAVE_readInputRegisters
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8       pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_UINT32_PTR  pByteCount
)
{
    return  FTE_RET_OK;
}

/******************************************************************************
 * Shell command
 ******************************************************************************/
FTE_INT32  FTE_MBTCP_SLAVE_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{
    FTE_BOOL  bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
 
    bPrintUsage = Shell_check_help_request(nArgc, pArgv, &bShortHelp );
    if (bPrintUsage == TRUE)  
    {
        goto error;
    }
    
    switch(nArgc)  
    {
    case    1:
        {
            FTE_INT i, nSession = 0;
            FTE_CHAR    pBuff[32];
            
            printf("\n[ CONFIGURATION ]\n");
            printf("%16s : %lu\n", "Port", xMBTCPSlave.xConfig.usPort);
            for(i = 0 ; i < xMBTCPSlave.xConfig.ulMaxSessions ; i++)
            {
                if (xMBTCPSlave.pSessions[i].hTask != 0)
                {
                    printf("\n[ SESSION %d ]\n", ++nSession);
                    printf("%16s : %d.%d.%d.%d\n", "Host IP", IPBYTES(xMBTCPSlave.pSessions[i].xRemoteAddr.sin_addr.s_addr));
                    printf("%16s : %lu\n","Port", xMBTCPSlave.pSessions[i].xRemoteAddr.sin_port);
                    FTE_TIME_toStr(&xMBTCPSlave.pSessions[i].xCreationTime, pBuff, sizeof(pBuff));
                    printf("%16s : %s\n", "Creation Time", pBuff);
                    FTE_TIME_toStr(&xMBTCPSlave.pSessions[i].xLastTime, pBuff, sizeof(pBuff));
                    printf("%16s : %s\n", "Last Access Time", pBuff);
                }
            }
            
            printf("\n[ STATISTICS ]\n");
            printf("%16s :  %8lu\n", "Rx Packets", xMBTCPSlave.xMBSlave.xStatistics.ulRxPkt);
            printf("%16s :  %8lu\n", "Tx Packets", xMBTCPSlave.xMBSlave.xStatistics.ulTxPkt);
            printf("%16s :  %8lu\n", "Errors", xMBTCPSlave.xMBSlave.xStatistics.ulRxError);
        }
        break;        
   
    case    3:
        {
            FTE_NET_CFG             xNetCfg;
            

            if (strcasecmp(pArgv[1], "port") == 0)
            {
                FTE_UINT32  ulPort = strtoul(pArgv[2], 0, 10);
                if ((ulPort == 0) || (ulPort > 65535))
                {
                    printf("Invalid UID [1 ~ 65535]\n");
                    goto error;
                }
                else
                {
                    xMBTCPSlave.xConfig.usPort = (FTE_UINT16)ulPort;
                }
            }
            else
            {
                bPrintUsage = TRUE;
                goto error;
            }

            FTE_CFG_NET_copy(&xNetCfg);
#if FTE_MBTCP_SUPPORTED          
            memcpy(&xNetCfg.xMBTCP, &xMBTCPSlave.xConfig, sizeof(xMBTCPSlave.xConfig));
#endif            
            FTE_CFG_NET_set(&xNetCfg);
        }
        break;
        
    default:
        bPrintUsage = TRUE;
        goto error;
    }
    
error:    
    if (bPrintUsage)  
    {
        if (bShortHelp)  
        {
            printf("%s <cmd>\n", pArgv[0]);
        } 
        else  
        {
            printf("USAGE: %s <cmd>\n",pArgv[0]);
            printf("    MODBUS/TCP command set.\n");
            printf("COMMANDS:\n");
            printf("    %8s <PORT>\n", "port");
            printf("        %s\n", "Set MODBUS/TCP service port");
            printf("    %8s <UID>\n", "uid");
            printf("        %s\n", "Set unit identifier");
            printf("PARAMETERS:\n");
            printf("    %8s %s\n", "PORT", "1 ~ 65535");
            printf("    %8s %s\n", "UID",  "1 ~ 255");
        }
    }
    
    
   return xRet;
} /* Endbody */
