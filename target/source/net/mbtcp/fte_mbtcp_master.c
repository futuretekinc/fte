#include <ctype.h>
#include "fte_target.h"
#include "fte_config.h"
#include "fte_mbtcp_master.h"
#include "sys/fte_sys.h"
#include "fte_time.h"
#include <rtcs.h>
#include <sh_rtcs.h>
#include "nxjson.h"
#include "fte_json.h"
#include "fte_ssl.h"

#if FTE_MBTCP_MASTER_SUPPORTED
#undef  __MODULE__
#define __MODULE__  FTE_MODULE_NET_MBTCP

typedef struct
{
    FTE_CHAR_PTR    pTopic;
    FTE_CHAR_PTR    pMsg;
    FTE_UINT8      nQoS;
    FTE_UINT8      pData[];
}   FTE_MBTCP_MASTER_SEND_MSG, _PTR_ FTE_MBTCP_MASTER_SEND_MSG_PTR;

typedef struct
{
    FTE_UINT8      nCmd;
    FTE_UINT8      nTarget;
    FTE_UINT32     ulDataLen;
    FTE_UINT8      pData[];
}   FTE_MBTCP_MASTER_MSG, _PTR_ FTE_MBTCP_MASTER_MSG_PTR;

typedef struct
{
    FTE_MBTCP_MASTER_STATE  xState;
    FTE_UINT32         (*callback)(FTE_MBTCP_MASTER_PTR);
}   FTE_MBTCP_MASTER_STATE_CALLBACK, _PTR_ FTE_MBTCP_MASTER_STATE_CALLBACK_PTR;

typedef struct
{
    FTE_UINT32     ulMsgID;
    FTE_UINT32     (*callback)(FTE_MBTCP_MASTER_PTR);
}   FTE_MBTCP_MASTER_MSG_CALLBACK, _PTR_ FTE_MBTCP_MASTER_MSG_CALLBACK_PTR;


static 
void     FTE_MBTCP_MASTER_task(pointer pTraps, pointer pCreator);

static 
FTE_UINT32  FTE_MBTCP_MASTER_initSocket(FTE_MBTCP_MASTER_PTR pMaster);

static 
FTE_RET FTE_MBTCP_MASTER_closeSocket(FTE_MBTCP_MASTER_PTR pMaster);

static 
FTE_UINT32  FTE_MBTCP_MASTER_connect(FTE_MBTCP_MASTER_PTR pMaster);

static 
FTE_UINT32  FTE_MBTCP_MASTER_disconnect(FTE_MBTCP_MASTER_PTR pMaster);

static 
FTE_UINT32 FTE_MBTCP_MASTER_sendPacket
(
    FTE_MBTCP_MASTER_PTR    pMaster, 
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      ulDataLen
);

static 
FTE_UINT32 FTE_MBTCP_MASTER_recvPacket
(
    FTE_MBTCP_MASTER_PTR    pMaster, 
    FTE_UINT8_PTR           pBuffer,
    FTE_UINT32              ulBufferSize,
    FTE_UINT32_PTR          pReadLen,
    FTE_UINT32              ulTimeout
);

static
FTE_SYS_LOCK_PTR    pLock = NULL;

static 
FTE_UINT32  FTE_MBTCP_MASTER_PING_send(FTE_MBTCP_MASTER_PTR pMaster);

static 
void     FTE_MBTCP_MASTER_PING_timeout(FTE_TIMER_ID, FTE_VOID_PTR, MQX_TICK_STRUCT_PTR);

static 
FTE_MBTCP_MASTER_PTR _pGlobalMasters = NULL;

FTE_UINT32 FTE_MBTCP_MASTER_load_default
(
    FTE_MBTCP_MASTER_CFG_PTR    pConfig
)
{
    _enet_address   xMAC;
    
    FTE_NET_getMACAddress(xMAC);

    pConfig->xIPAddress     = FTE_NET_MBTCP_MASTER_SLAVE_IP;
    pConfig->usPort         = FTE_MBTCP_MASTER_DEFAULT_PORT;
    pConfig->ulKeepalive    = FTE_MBTCP_MASTER_DEFAULT_KEEPALIVE;
    
    return  FTE_RET_OK;
}

FTE_UINT32 FTE_MBTCP_MASTER_init
(
    FTE_MBTCP_MASTER_CFG_PTR    pConfigs,
    FTE_UINT32 nCount    
)
{
    ASSERT(pConfigs != NULL);

    if (_pGlobalMasters != NULL)
    {
        return  FTE_MBTCP_MASTER_RET_ERROR;
    }

    _pGlobalMasters = (FTE_MBTCP_MASTER_PTR)FTE_MEM_allocZero(sizeof(FTE_MBTCP_MASTER) * nCount);
    if (_pGlobalMasters == NULL)
    {
        return  FTE_MBTCP_MASTER_RET_NOT_ENOUGH_MEMORY;
    }

    FTE_SYS_LOCK_create(&pLock);
    FTE_SYS_LOCK_init(pLock, 1);
    
    for(FTE_UINT32 i = 0 ; i < nCount ; i++)
    {
        if (pConfigs[i].bEnable)
        {        
            FTE_LIST_init(&_pGlobalMasters[i].xMsgPool);
                
            _pGlobalMasters[i].xState    = FTE_MBTCP_MASTER_STATE_UNINITIALIZED;
            _pGlobalMasters[i].pConfig   = &pConfigs[i];
        
            RTCS_task_create(FTE_NET_MBTCP_MASTER_NAME, FTE_NET_MBTCP_MASTER_PRIO, FTE_NET_MBTCP_MASTER_STACK, FTE_MBTCP_MASTER_task, &_pGlobalMasters[i]);    
        }
    }
    
    return  FTE_MBTCP_MASTER_RET_OK;
}    

void FTE_MBTCP_MASTER_task
(
    FTE_VOID_PTR    pParams, 
    FTE_VOID_PTR    pCreator
)
{
    ASSERT(pParams != NULL);
    ASSERT(pCreator != NULL);
    
    FTE_UINT32  ulRet;
    FTE_MBTCP_MASTER_PTR    pMaster = (FTE_MBTCP_MASTER_PTR)pParams;

    RTCS_task_resume_creator(pCreator, RTCS_OK);
    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id());
    
    while(1)
    {
        switch(pMaster->xState)
        {
        case    FTE_MBTCP_MASTER_STATE_UNINITIALIZED:
            {
                ulRet = FTE_MBTCP_MASTER_initSocket(pMaster);
                if (ulRet != FTE_RET_OK)
                {
                    ERROR("Failed to init socket!");
                }
            }
            break;
            
        case    FTE_MBTCP_MASTER_STATE_INITIALIZED:
            {
                if (FTE_NET_isStable())
                {
                    ulRet = FTE_MBTCP_MASTER_connect(pMaster);
                    if (ulRet != FTE_RET_OK)
                    {
                        ERROR("Failed to connect!");
                    }
                }
            }
            break;
            
        case    FTE_MBTCP_MASTER_STATE_CONNECTED:
            {
            }
            break;
            
        case    FTE_MBTCP_MASTER_STATE_DISCONNECTED:
            {
            }
            break;
        }
        _time_delay(1000);

    }
}

FTE_UINT32 FTE_MBTCP_MASTER_connect
(   
    FTE_MBTCP_MASTER_PTR    pMaster
)
{
    ASSERT(pMaster != NULL);
    
    FTE_UINT32  ulRet;
    FTE_INT32   nValue;
	struct sockaddr_in xAddress;

    if (pMaster->xState != FTE_MBTCP_MASTER_STATE_INITIALIZED)
    {
        ERROR("MBTCP master socket is not initialized.");    
        ulRet = FTE_MBTCP_MASTER_RET_OPERATION_NOT_PERMITTED;
        goto finished;
    }

    if (pMaster->nSocketID > 0)
    {
        FTE_MBTCP_MASTER_closeSocket(pMaster);
    }
    
	// Create the socket
    pMaster->nSocketID = socket(PF_INET, SOCK_STREAM, 0);
	if(pMaster->nSocketID == RTCS_SOCKET_ERROR)
    {
        ERROR("Failed to create socket.");    
		ulRet = FTE_MBTCP_MASTER_RET_ERROR;
        goto finished;
    }

	// Disable Nagle Algorithm
    nValue = 1;
    ulRet = setsockopt(pMaster->nSocketID, SOL_TCP, OPT_RECEIVE_NOWAIT, (FTE_CHAR_PTR)&nValue, sizeof(nValue));
	if ( ulRet != RTCS_OK)
    {
        ERROR("Failed to set disable nagle algorithm!");
        goto finished;
    }
    
    // Create the stuff we need to connect
    xAddress.sin_family       = AF_INET;
    xAddress.sin_addr.s_addr  = pMaster->pConfig->xIPAddress;
    xAddress.sin_port         = pMaster->pConfig->usPort;

    // Connect the socket
    _time_delay(100);
    ulRet = connect(pMaster->nSocketID, (struct sockaddr*)&xAddress, sizeof(xAddress));
    if(ulRet != RTCS_OK)
    {
        if ( ulRet != RTCSERR_TCP_CONN_RLSD)
        {
            ERROR("Failed to connect!");
            goto finished;
        }
        else
        {
            ulRet = FTE_MBTCP_MASTER_RET_OK;
        }
    }
    
    pMaster->ulPingTimeout = 0;
    pMaster->xPingTimer = 0;
    pMaster->xState = FTE_MBTCP_MASTER_STATE_CONNECTED;
    TRACE("MBTCP master socket[%d] connected.\n", pMaster->nSocketID);        
    
finished:
    if (ulRet != FTE_MBTCP_MASTER_RET_OK)
    {
        if (pMaster->nSocketID > 0)
        {
            shutdown(pMaster->nSocketID, FLAG_ABORT_CONNECTION);
            pMaster->nSocketID = 0;
        }
    }
    
    return  ulRet;    
}

FTE_UINT32 FTE_MBTCP_MASTER_disconnect
(
    FTE_MBTCP_MASTER_PTR    pMaster
)
{
    ASSERT(pMaster != NULL);
    
    if (pMaster->xPingTimer != 0)
    {
        FTE_TIMER_cancel(&pMaster->xPingTimer);
        pMaster->ulPingTimeout = 0;
    }
    
    if (pMaster->nSocketID != 0)
    {
        shutdown(pMaster->nSocketID, FLAG_ABORT_CONNECTION);
        pMaster->nSocketID = 0;
    }
    
    pMaster->xState = FTE_MBTCP_MASTER_STATE_DISCONNECTED;
    TRACE("MBTCP master socket disconnected.");    
    
    return  FTE_MBTCP_MASTER_RET_OK;
}

FTE_BOOL    FTE_MBTCP_MASTER_isConnected
(
    FTE_UINT32      ulID
)
{
    return  (_pGlobalMasters[ulID].xState == FTE_MBTCP_MASTER_STATE_CONNECTED);
}

FTE_UINT32  FTE_MBTCP_MASTER_initSocket
(
    FTE_MBTCP_MASTER_PTR    pMaster
)
{
	// MBTCP_MASTER stuffs
    
    TRACE("MBTCP master socket initialize.");
    pMaster->xState = FTE_MBTCP_MASTER_STATE_INITIALIZED;
  
	return FTE_MBTCP_MASTER_RET_OK;
}


FTE_RET FTE_MBTCP_MASTER_closeSocket
(
    FTE_MBTCP_MASTER_PTR    pMaster
)
{
    if ((pMaster->xState != FTE_MBTCP_MASTER_STATE_INITIALIZED) && (pMaster->xState != FTE_MBTCP_MASTER_STATE_DISCONNECTED))
    {
        FTE_MBTCP_MASTER_disconnect(pMaster);
    }
    
    pMaster->xState = FTE_MBTCP_MASTER_STATE_UNINITIALIZED;
    TRACE("MBTCP master socket closed.");    

    return FTE_MBTCP_MASTER_RET_OK;
}


FTE_UINT32 FTE_MBTCP_MASTER_sendPacket
(
    FTE_MBTCP_MASTER_PTR    pMaster, 
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      ulDataLen
)
{
    ASSERT(pMaster != NULL);
    ASSERT(pData != NULL);
    
    FTE_RET ulRet = FTE_RET_OK;
    FTE_INT nSendLen;
   
   nSendLen = send(pMaster->nSocketID, pData, ulDataLen, 0);
   if (nSendLen <= 0)
   {
        ERROR("mbtcp master disconnecte.");
        pMaster->xState = FTE_MBTCP_MASTER_STATE_DISCONNECTED;
        ulRet = FTE_RET_SOCKET_CLOSED;
   }
   else if (nSendLen != ulDataLen)
   {
        ulRet = FTE_MBTCP_MASTER_RET_SEND_FAILED;
   }
   
   return   ulRet;
}

FTE_UINT32 FTE_MBTCP_MASTER_recvPacket
(
    FTE_MBTCP_MASTER_PTR    pMaster, 
    FTE_UINT8_PTR           pBuffer,
    FTE_UINT32              ulBufferSize,
    FTE_UINT32_PTR          pReadLen,
    FTE_UINT32              ulTimeout
)
{
    ASSERT(pMaster != NULL);
    ASSERT(pReadLen != NULL);
    
    FTE_RET     xRet = FTE_RET_OK;
    FTE_INT32   nReadLen = 0;
    FTE_INT32   nRcvdLen = 0;
    FTE_TIME    xStartTime;
    FTE_TIME    xCurrentTime;
    FTE_INT32   nDiffMSec;
    
    FTE_TIME_getCurrent(&xStartTime);
    
	while(ulTimeout > 0)
	{
        if (RTCS_selectset(&pMaster->nSocketID, 1, ulTimeout) == 0)
        {
            //TRACE("The packet reception wait time has been exceeded.!");
            xRet = FTE_MBTCP_MASTER_RET_READ_ABOARTED;
            goto finished;
        }

        memset(pMaster->pRcvdBuffer, 0, sizeof(pMaster->pRcvdBuffer));
        pMaster->ulRcvdLen = 0;
        
        nRcvdLen = recv(pMaster->nSocketID, &pBuffer[nReadLen], ulBufferSize - nReadLen, 0);    
        if (nRcvdLen < 0)
        {
            pMaster->xState = FTE_MBTCP_MASTER_STATE_DISCONNECTED;
            xRet = FTE_RET_SOCKET_CLOSED;
            goto finished;
        }
        else if (nRcvdLen > 0)
        {
            nReadLen = nRcvdLen;
            break;
        }
        
        FTE_TIME_getCurrent(&xCurrentTime);
        
        xRet = FTE_TIME_diffMilliseconds(&xCurrentTime, &xStartTime, &nDiffMSec);
        if (xRet != FTE_RET_OK)
        {
            ERROR("Failed to calculate time!");
            goto finished;
        }
        
        if ((nDiffMSec < 0) && (ulTimeout < nDiffMSec))
        {
            nReadLen = 0;
            break;
        }
        else
        {
            ulTimeout -= nDiffMSec;
        }
    }
    
    if (nReadLen != (6 + (((FTE_UINT32)pBuffer[4] << 8) | (FTE_UINT32)pBuffer[5])))
    {
        xRet = FTE_MBTCP_MASTER_RET_INVALID_FRAME_RECEIVED;
        goto finished;
    }    
    
    *pReadLen = nReadLen;
   
finished:    
    
	return xRet;
}


FTE_UINT32  FTE_MBTCP_MASTER_readRegisters
(
    FTE_UINT32 ulID, 
    FTE_UINT32      ulAddress, 
    FTE_UINT32      ulCount, 
    FTE_UINT16_PTR  pRegisters
)
{
    ASSERT(pRegisters != NULL);

    FTE_UINT32  ulRet = FTE_RET_OK;
    FTE_UINT8   pFrame[] = { 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0};
    FTE_UINT32  i;
    FTE_MBTCP_MASTER_PTR    pMaster; 

  
    FTE_SYS_LOCK_enable(pLock);
    TRACE("Read Register(%d, %d) - Entry\n", ulID, ulAddress);
    pMaster = &_pGlobalMasters[ulID];
    
    if (pMaster->xState != FTE_MBTCP_MASTER_STATE_CONNECTED)
    {
        ulRet = FTE_MBTCP_MASTER_RET_NOT_CONNECTED;
        goto finished;
    }
    
    ulAddress += pMaster->pConfig->usOffset;
    ++pMaster->ulTransactionID;
    
    pFrame[FTE_MBTCP_FRAME_OFFSET_TID]      = (pMaster->ulTransactionID >> 8) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_TID + 1]  = (pMaster->ulTransactionID     )& 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_UID]      = pMaster->pConfig->ulUnitID;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 0]  = FTE_MODBUS_FUNCTION_READ_HOLDING_REGISTERS;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 1]  = (ulAddress >> 8) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 2]  = (ulAddress     ) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 3]  = (ulCount >> 8) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 4]  = (ulCount     ) & 0xFF;
    
    
    ulRet = FTE_MBTCP_MASTER_sendPacket(pMaster, pFrame, sizeof(pFrame));
    if (ulRet != FTE_RET_OK)
    {
        ERROR("Failed to send packet!");
        goto finished;
    }

    ulRet = FTE_MBTCP_MASTER_recvPacket(pMaster, pMaster->pRcvdBuffer, sizeof(pMaster->pRcvdBuffer), &pMaster->ulRcvdLen, 2000);
    if (ulRet != FTE_RET_OK)
    {
        ERROR("Failed to receive packet[%d]!", pMaster->nSocketID);
        goto finished;
    }    
    
    if (pMaster->pRcvdBuffer[FTE_MBTCP_FRAME_OFFSET_UID] != pMaster->pConfig->ulUnitID)
    {
        ulRet = FTE_MBTCP_MASTER_RET_INVALID_FRAME_RECEIVED;
        goto finished;
    }
    
    if (pMaster->pRcvdBuffer[FTE_MBTCP_FRAME_OFFSET_PDU] != FTE_MODBUS_FUNCTION_READ_HOLDING_REGISTERS)
    {
        ulRet = FTE_MBTCP_MASTER_RET_INVALID_FRAME_RECEIVED;
        goto finished;
    }
    
    if (pMaster->pRcvdBuffer[FTE_MBTCP_FRAME_OFFSET_PDU + 1] != ulCount * 2)
    {
        ulRet = FTE_MBTCP_MASTER_RET_INVALID_FRAME_RECEIVED;
        goto finished;
    }
    
    for(i = 0 ; i < ulCount ; i++)
    {
        pRegisters[i] = ((FTE_UINT16)pMaster->pRcvdBuffer[FTE_MBTCP_FRAME_OFFSET_PDU + 2 + i*2] << 8) + pMaster->pRcvdBuffer[FTE_MBTCP_FRAME_OFFSET_PDU + 2 + i*2 + 1];
    }    
    
finished:    
    TRACE("Read Register(%d, %d) - Exit\n", ulID, ulAddress);
    FTE_SYS_LOCK_disable(pLock);
    
    return  ulRet;

}

FTE_UINT32  FTE_MBTCP_MASTER_writeRegister
(
    FTE_UINT32      ulID,
    FTE_UINT32      ulAddress, 
    FTE_UINT16      ulValue
)
{
    FTE_UINT32  ulRet = FTE_RET_OK;
    FTE_UINT8   pFrame[] = { 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0};
    FTE_MBTCP_MASTER_PTR    pMaster; 

    pMaster = &_pGlobalMasters[ulID];
    
    if (pMaster->xState != FTE_MBTCP_MASTER_STATE_CONNECTED)
    {
        ulRet = FTE_MBTCP_MASTER_RET_NOT_CONNECTED;
        goto finished;
    }
    
    ++pMaster->ulTransactionID;
    
    pFrame[FTE_MBTCP_FRAME_OFFSET_TID]      = (pMaster->ulTransactionID >> 8) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_TID + 1]  = (pMaster->ulTransactionID     )& 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_UID]      = pMaster->pConfig->ulUnitID;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 0]  = FTE_MODBUS_FUNCTION_WRITE_SINGLE_REGISTER;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 1]  = (ulAddress >> 8) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 2]  = (ulAddress     ) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 3]  = (ulValue >> 8) & 0xFF;
    pFrame[FTE_MBTCP_FRAME_OFFSET_PDU + 4]  = (ulValue     ) & 0xFF;
    
    ulRet = FTE_MBTCP_MASTER_sendPacket(pMaster, pFrame, sizeof(pFrame));
    if (ulRet != FTE_RET_OK)
    {
        goto finished;
    }

    ulRet = FTE_MBTCP_MASTER_recvPacket(pMaster, pMaster->pRcvdBuffer, sizeof(pMaster->pRcvdBuffer), &pMaster->ulRcvdLen, 1000);
    if (ulRet != FTE_RET_OK)
    {
        goto finished;
    }    
    
    if (pMaster->pRcvdBuffer[FTE_MBTCP_FRAME_OFFSET_UID] != pMaster->pConfig->ulUnitID)
    {
        ulRet = FTE_MBTCP_MASTER_RET_INVALID_FRAME_RECEIVED;
        goto finished;
    }
    
    if (pMaster->pRcvdBuffer[FTE_MBTCP_FRAME_OFFSET_PDU] != FTE_MODBUS_FUNCTION_WRITE_SINGLE_REGISTER)
    {
        ulRet = FTE_MBTCP_MASTER_RET_INVALID_FRAME_RECEIVED;
        goto finished;
    }
    
finished:    
    return  ulRet;

}
FTE_RET FTE_MBTCP_MASTER_PING_send
(
    FTE_MBTCP_MASTER_PTR    pMaster
)
{   
    FTE_RET xRet;
    MQX_TICK_STRUCT     xTicks;            
        
    pMaster->ulPingTimeout++;
    
    _time_get_elapsed_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, pMaster->ulPingTimeout);
        
     xRet = FTE_TIMER_startOneshotAtTicks(FTE_MBTCP_MASTER_PING_timeout, pMaster, TIMER_ELAPSED_TIME_MODE, &xTicks, &pMaster->xPingTimer);    
     if (xRet != FTE_RET_OK)
     {
        return  xRet;
     }

    TRACE("PING SEND : TIMEOUT = %d secs\n", pMaster->ulPingTimeout);
    
    return  xRet;        
}

void   FTE_MBTCP_MASTER_PING_timeout
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_MBTCP_MASTER_PTR pMaster = (FTE_MBTCP_MASTER_PTR)pData;
    
    if (pMaster->ulPingTimeout < 3)
    {
        FTE_MBTCP_MASTER_PING_send(pMaster);
    }
    else
    {
        pMaster->xPingTimer = 0;
        FTE_MBTCP_MASTER_disconnect(pMaster);
    }
}

/******************************************************************************
 * Shell command
 ******************************************************************************/
FTE_INT32  FTE_MBTCP_MASTER_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_NET_CFG_PTR     pConfig;
                static FTE_CHAR_PTR pStateString[] =
                {
                    "UNINITIALIZED", "INITIALIZED", "CONNECTED", "DISCONNECTED"
                };
                
                xRet = FTE_CFG_NET_get(&pConfig);
                if (xRet != FTE_RET_OK)
                {
                    printf("Error : Can't get network configuration!\n");
                    break;
                }
                for(FTE_INT32 i = 0 ; i < pConfig->nMBTCPMasterCount ; i++)
                {
                    printf("%16s : %s\n",           "STATUS",   pConfig->xMBTCPMasters[i].bEnable?"run":"stop");
                    printf("%16s : %d.%d.%d.%d\n",  "HOST",     IPBYTES(pConfig->xMBTCPMasters[i].xIPAddress));
                    printf("%16s : %d\n",           "PORT",     pConfig->xMBTCPMasters[i].usPort);
                    printf("%16s : %u\n",           "UNIT ID",  pConfig->xMBTCPMasters[i].ulUnitID);
                    printf("%16s : %u\n",           "KEEPALIVE",pConfig->xMBTCPMasters[i].ulKeepalive);
                    printf("%16s : %u\n",           "OFFSET",   pConfig->xMBTCPMasters[i].usOffset);

                    if (_pGlobalMasters != NULL)
                    {
                        printf("%16s : %s\n",           "STATE",    pStateString[_pGlobalMasters[i].xState]);
                    }
                    printf("\n");
                }
            }
            break;
            
        case    2:
            {
                if (strcasecmp(pArgv[1], "help") == 0)
                {
                    bPrintUsage = TRUE;
                }
            }
            break;
            
        case    3:
            {
                FTE_NET_CFG_PTR     pConfig;
                FTE_UINT32          nIndex;
                
                FTE_strToUINT32( pArgv[1], &nIndex);
                
                xRet = FTE_CFG_NET_get(&pConfig);
                if (xRet != FTE_RET_OK)
                {
                    printf("Can't get network configuration!\n");
                    break;
                }
                
                if (strcasecmp(pArgv[2], "start") == 0)
                {
                    if (pConfig->xMBTCPMasters[nIndex].bEnable == FALSE)
                    {
                        pConfig->xMBTCPMasters[nIndex].bEnable = TRUE;
                        FTE_CFG_save(TRUE);
                    }
                }
                else if (strcasecmp(pArgv[2], "stop") == 0)
                {
                    if (pConfig->xMBTCPMasters[nIndex].bEnable == TRUE)
                    {
                         pConfig->xMBTCPMasters[nIndex].bEnable = FALSE;
                        FTE_CFG_save(TRUE);
                    }
                }
                else
                {
                    bPrintUsage = TRUE;
                }
            }
            break;

        case    4:
            {
                FTE_NET_CFG_PTR     pConfig;
                FTE_UINT32          nIndex;
                
                FTE_strToUINT32( pArgv[1], &nIndex);
               
                if (strcmp(pArgv[2], "connect") == 0)
                {
                    if (_pGlobalMasters != NULL)
                    {
                       _ip_address xIPAddress;

                         if (FTE_strToIP(pArgv[3], &xIPAddress) != FTE_RET_OK)
                        {
                            printf ("Error! invalid ip address!\n");
                            return SHELL_EXIT_ERROR;
                        }
                        
                        _pGlobalMasters[nIndex].pConfig->xIPAddress = xIPAddress;
                        
                        xRet = FTE_MBTCP_MASTER_connect(&_pGlobalMasters[nIndex]);
                        if (xRet != FTE_RET_OK)
                        {
                            printf("Failed to connect mbtcp !");
                        } 
                    }
                }
                else if (strcmp(pArgv[2], "read") == 0)
                {
                    if (_pGlobalMasters != NULL)
                    {
                        FTE_UINT16  pRegisters[10];
                        xRet = FTE_MBTCP_MASTER_readRegisters(nIndex, 0, 10, pRegisters);
                        if (xRet != FTE_RET_OK)
                        {
                            printf("Failed to connect mbtcp !");
                        } 
                    }
                }
                else if (strcmp(pArgv[2], "host") == 0)
                {
                    _ip_address xIPAddress;

                     if (FTE_strToIP(pArgv[3], &xIPAddress) != FTE_RET_OK)
                    {
                        printf ("Error! invalid ip address!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    
                    xRet = FTE_CFG_NET_get(&pConfig);
                    if (xRet != FTE_RET_OK)
                    {
                        printf("Can't get network configuration!\n");
                        break;
                    }
                    
                    pConfig->xMBTCPMasters[nIndex].xIPAddress = xIPAddress;
                    FTE_CFG_save(TRUE);
                }
                else if (strcmp(pArgv[2], "port") == 0)
                {
                    FTE_UINT16 usPort;

                     if (FTE_strToUINT16(pArgv[3], &usPort) != FTE_RET_OK)
                    {
                        printf ("Error! invalid port number!\n");
                        return SHELL_EXIT_ERROR;
                    }
       
                    xRet = FTE_CFG_NET_get(&pConfig);
                    if (xRet != FTE_RET_OK)
                    {
                        printf("Can't get network configuration!\n");
                        break;
                    }
                    
                    pConfig->xMBTCPMasters[nIndex].usPort = usPort;
                    FTE_CFG_save(TRUE);
                }
                else if (strcmp(pArgv[2], "unitid") == 0)
                { 
                    FTE_UINT32  ulUnitID;

                     if (FTE_strToUINT32(pArgv[3], &ulUnitID) != FTE_RET_OK)
                    {
                        printf ("Error! invalid Unit ID!\n");
                        return SHELL_EXIT_ERROR;
                    }
       
                    xRet = FTE_CFG_NET_get(&pConfig);
                    if (xRet != FTE_RET_OK)
                    {
                        printf("Can't get network configuration!\n");
                        break;
                    }
                    
                    pConfig->xMBTCPMasters[nIndex].ulUnitID = ulUnitID;
                    FTE_CFG_save(TRUE);
                }
                else if (strcmp(pArgv[2], "offset") == 0)
                {
                    FTE_UINT16 usOffset;

                     if (FTE_strToUINT16(pArgv[3], &usOffset))
                    {
                        printf ("Error! invalid port number!\n");
                        return SHELL_EXIT_ERROR;
                    }
       
                    xRet = FTE_CFG_NET_get(&pConfig);
                    if (xRet != FTE_RET_OK)
                    {
                        printf("Can't get network configuration!\n");
                        break;
                    }
                    
                    pConfig->xMBTCPMasters[nIndex].usOffset = usOffset;
                    FTE_CFG_save(TRUE);
                }
                else if (strcmp(pArgv[2], "keepalive") == 0)
                {
                    FTE_UINT32 ulKeepalive;

                     if (FTE_strToUINT32(pArgv[3], &ulKeepalive) != FTE_RET_OK)
                    {
                        printf ("Error! invalid keepalive!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    
                    xRet = FTE_CFG_NET_get(&pConfig);
                    if (xRet != FTE_RET_OK)
                    {
                        printf("Can't get network configuration!\n");
                        break;
                    }
                    
                    pConfig->xMBTCPMasters[nIndex].ulKeepalive = ulKeepalive;
                    FTE_CFG_save(TRUE);
                }
                else
                {
                    bPrintUsage = TRUE;
                }                
            }
            break;
        }
    }
    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>] [<command>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>] [<command>]\n", pArgv[0]);
            printf("  Commands:\n");
            printf("    info\n");
            printf("        Show MODBUS TCP information.\n");
            printf("    start\n");
            printf("        start mqtt.\n");
            printf("    stop\n");
            printf("        stop mqtt.\n");
            printf("    host <ip>\n");
            printf("        MODBUS TCP target IP.\n");
            printf("    port <port>\n");
            printf("        MODBUS TCP service port.\n");
            printf("    keepalive <sec>\n");
            printf("        Set keepalive time.\n");
            printf("    offset <value>\n");
            printf("        Base address\n");
            printf("  Parameters:\n");
            printf("    <id>      = MBTCP server id [0 or 1].\n");
            printf("    <ip>      = IPv4 address to use.\n");
        }
    }
    return   xRet;
}



#endif