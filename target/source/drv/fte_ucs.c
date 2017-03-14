#include "fte_target.h"
#include "fte_ucs.h"

#if FTE_UCS_SUPPORTED

static  
FTE_RET     _FTE_UCS_lock(FTE_UCS_PTR pUCS);

static  
FTE_RET     _FTE_UCS_unlock(FTE_UCS_PTR pUCS);

static  
FTE_UINT32  _FTE_UCS_clear(FTE_UCS_PTR pUCS);

static  
FTE_UINT32  _FTE_UCS_recv(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen);

static  
FTE_UINT32  _FTE_UCS_send(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen);

static  
FTE_LIST    xUCSList = { 0, NULL };

FTE_RET   FTE_UCS_create
(   
    FTE_UCS_CONFIG_CONST_PTR pConfig
)
{
    ASSERT(pConfig != NULL);
    
    FTE_UCS_PTR pUCS = NULL;

    if (xUCSList.pHead == NULL)
    {
        FTE_LIST_init(&xUCSList);
    }

    pUCS = (FTE_UCS_PTR)FTE_MEM_allocZero(sizeof(FTE_UCS));
    if (pUCS == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }

    pUCS->pRecvBuf = (FTE_UINT8_PTR)FTE_MEM_allocZero(pConfig->nRecvBufLen);
    pUCS->pSendBuf= (FTE_UINT8_PTR)FTE_MEM_allocZero(pConfig->nSendBufLen);

    FTE_LIST_init(&pUCS->xParents);

    FTE_SYS_LOCK_init(&pUCS->xSEMLock, 1);
    FTE_SYS_LOCK_init(&pUCS->xSEMSend, 1);
    FTE_SYS_LOCK_init(&pUCS->xSEMRecv, 1);
    FTE_SYS_LOCK_init(&pUCS->xSEMSendNotEmtry, 0);

    pUCS->bFullDuplex=pConfig->bFullDuplex;
    pUCS->nBaudrate = pConfig->nBaudrate;
    pUCS->nDataBits = pConfig->nDataBits;
    pUCS->nParity   = pConfig->nParity;
    pUCS->nStopBits = pConfig->nStopBits;

    pUCS->pConfig   = pConfig;

    FTE_LIST_pushBack(&xUCSList, pUCS);

    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_destroy
(
    FTE_UCS_PTR pUCS
)
{
    ASSERT(pUCS != NULL);
    
    FTE_SYS_LOCK_final(&pUCS->xSEMLock);
    FTE_SYS_LOCK_final(&pUCS->xSEMSend);
    FTE_SYS_LOCK_final(&pUCS->xSEMRecv);
    FTE_SYS_LOCK_final(&pUCS->xSEMSendNotEmtry);

    FTE_LIST_final(&pUCS->xParents);
    FTE_LIST_remove(&xUCSList, pUCS);

    FTE_MEM_free(pUCS->pRecvBuf);
    FTE_MEM_free(pUCS->pSendBuf);

    FTE_MEM_free(pUCS);

    return  FTE_RET_OK;
}

FTE_RET FTE_UCS_attach
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32         nParent
)
{
    ASSERT(pUCS != NULL);

    if (FTE_UCS_init(pUCS) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }

    if (pUCS->pConfig->nFlowCtrlID != 0)
    {
        pUCS->pFlowCtrl = FTE_LWGPIO_get(pUCS->pConfig->nFlowCtrlID);
        if (pUCS->pFlowCtrl != NULL)
        {
            FTE_LWGPIO_attach(pUCS->pFlowCtrl, pUCS->pConfig->nID);
        }
    }
    
    if (pUCS->pConfig->nFlowCtrl2ID != 0)
    {
        pUCS->pFlowCtrl2 = FTE_LWGPIO_get(pUCS->pConfig->nFlowCtrl2ID);
        if (pUCS->pFlowCtrl2 != NULL)
        {
            FTE_LWGPIO_attach(pUCS->pFlowCtrl2, pUCS->pConfig->nID);
        }
    }

    if (nParent != 0)
    {
        FTE_LIST_pushBack(&pUCS->xParents, (pointer)nParent);
    }

    return  FTE_RET_OK;
}

FTE_RET       FTE_UCS_detach
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32         nParent
)
{
    ASSERT(pUCS != NULL);

    if (nParent != 0)
    {
        FTE_LIST_remove(&pUCS->xParents, (pointer)nParent);
    }

    if (FTE_LIST_count(&pUCS->xParents) == 0)
    {
        if (pUCS->pFlowCtrl != NULL)
        {
            FTE_LWGPIO_detach(pUCS->pFlowCtrl);
            pUCS->pFlowCtrl = NULL;
        }

        if (pUCS->pFlowCtrl2 != NULL)
        {
            FTE_LWGPIO_detach(pUCS->pFlowCtrl2);
            pUCS->pFlowCtrl2 = NULL;
        }

        if (pUCS->hTaskRX != 0)
        {
            _task_destroy(pUCS->hTaskRX);
            pUCS->hTaskRX = 0;
        }

        if (pUCS->hTaskTX != 0)
        {
            _task_destroy(pUCS->hTaskTX);
            pUCS->hTaskTX = 0;
        }

        fclose(pUCS->pFD);
        pUCS->pFD = NULL;
    }

     return  FTE_RET_OK;
}

FTE_UCS_PTR FTE_UCS_get
(
    FTE_UINT32 nID
)
{
    FTE_UCS_PTR         pUCS;
    FTE_LIST_ITERATOR   xIter;

    FTE_LIST_ITER_init(&xUCSList, &xIter);
    while((pUCS = (FTE_UCS_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        if (pUCS->pConfig->nID == nID)
        {
            return  pUCS;
        }
    }

    return  NULL;
}

FTE_UINT32  FTE_UCS_count
(
    void
)
{
    return  FTE_LIST_count(&xUCSList);
}

FTE_UCS_PTR FTE_UCS_getAt
(
    FTE_UINT32 ulIndex
)
{
    return  (FTE_UCS_PTR)FTE_LIST_getAt(&xUCSList, ulIndex);
}

FTE_RET   FTE_UCS_init
(
    FTE_UCS_PTR     pUCS
)
{
    FTE_RET     xRet;
    FTE_UINT32  nValue;

    if (pUCS->pFD == NULL)
    {
        if (pUCS->bFullDuplex)
        {
            pUCS->pFD = fopen( pUCS->pConfig->pUART, 0 );
        }
        else
        {
            if (pUCS->pConfig->nFlowCtrlID == 0)
            {
                pUCS->pFD = fopen( pUCS->pConfig->pUART, (FTE_CHAR const *)IO_SERIAL_HW_485_FLOW_CONTROL);
            }
            else
            {
                pUCS->pFD = fopen( pUCS->pConfig->pUART, 0 );
            }
        }


        if (pUCS->pFD == NULL)
        {
            return  FTE_RET_INVALID_OBJECT;
        }

        nValue = pUCS->nBaudrate;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_BAUD, &nValue);
        nValue = pUCS->nDataBits;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_DATA_BITS, &nValue);
        nValue = pUCS->nParity;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_PARITY, &nValue);
        nValue = pUCS->nStopBits;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_STOP_BITS, &nValue);

        xRet = FTE_TASK_create(FTE_TASK_UCS_RX, pUCS->pConfig->nID, &pUCS->hTaskRX);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
        
        FTE_TASK_create(FTE_TASK_UCS_TX, pUCS->pConfig->nID, &pUCS->hTaskTX);
    }

    return  FTE_RET_OK;

error:
    if (pUCS->pFD != NULL)
    {
        fclose(pUCS->pFD);
        pUCS->pFD = NULL;
    }

    if (pUCS->hTaskRX == MQX_NULL_TASK_ID)
    {
        _task_destroy(pUCS->hTaskRX);
        pUCS->hTaskRX = MQX_NULL_TASK_ID;
    }

    if (pUCS->hTaskTX == MQX_NULL_TASK_ID)
    {
        _task_destroy(pUCS->hTaskTX);
        pUCS->hTaskTX = MQX_NULL_TASK_ID;
    }

    return  FTE_RET_ERROR;
}

FTE_RET   FTE_UCS_setBaudrate
(   
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32      nBaudrate
)
{
    ASSERT(pUCS != NULL);

    if (pUCS->nBaudrate != nBaudrate)
    {
        if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_BAUD, &nBaudrate))
        {
            return  FTE_RET_ERROR;
        }

        pUCS->nBaudrate = nBaudrate;
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_getBaudrate
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32_PTR  pBaudrate
)
{
    ASSERT(pUCS != NULL);

    *pBaudrate = pUCS->nBaudrate;

    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_setDatabits
(   
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32      nDataBits
)
{
    ASSERT(pUCS != NULL);

    if (pUCS->nDataBits != nDataBits)
    {
        if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_DATA_BITS, &nDataBits))
        {
            return  FTE_RET_ERROR;
        }

        pUCS->nDataBits = nDataBits;
    }

    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_getDatabits
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32_PTR  pDataBits
)
{
    ASSERT(pUCS != NULL);

    *pDataBits = pUCS->nDataBits;

    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_setParity
(   
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32      nParity
)
{
    ASSERT(pUCS != NULL);

    if (pUCS->nParity != nParity)
    {
        if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_PARITY, &nParity))
        {
            return  FTE_RET_ERROR;
        }

        pUCS->nParity = nParity;
    }

    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_getParity
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32_PTR  pParity
)
{
    ASSERT(pUCS != NULL);

    *pParity = pUCS->nParity;

    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_setStopbits
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32      nStopBits
)
{
    ASSERT(pUCS != NULL);

    if (pUCS->nStopBits != nStopBits)
    {
        if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_STOP_BITS, &nStopBits))
        {
            return  FTE_RET_ERROR;
        }

        pUCS->nStopBits= nStopBits;
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_getStopbits
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32_PTR  pStopBits
)
{
    ASSERT(pUCS != NULL);

    *pStopBits = pUCS->nStopBits;

    return  FTE_RET_OK;
}


FTE_RET   FTE_UCS_setDuplexMode
(   
    FTE_UCS_PTR     pUCS, 
    FTE_BOOL         bFullDuplex
)
{
    ASSERT(pUCS != NULL);

    pUCS->bFullDuplex = bFullDuplex;

    return  FTE_RET_OK;
}

FTE_RET   FTE_UCS_getDuplexMode
(
    FTE_UCS_PTR     pUCS, 
    FTE_BOOL_PTR    pFullDuplex
)
{
    ASSERT(pUCS != NULL);

    *pFullDuplex = pUCS->bFullDuplex;

    return  FTE_RET_OK;
}

FTE_RET     FTE_UCS_setUART
(
    FTE_UCS_PTR pUCS,
    FTE_UCS_UART_CONFIG const _PTR_ pConfig
)
{
    FTE_UCS_setBaudrate(pUCS, pConfig->nBaudrate);    
    FTE_UCS_setDatabits(pUCS, pConfig->nDataBits);
    FTE_UCS_setParity(pUCS, pConfig->nParity);
    FTE_UCS_setStopbits(pUCS, pConfig->nStopBits);
    FTE_UCS_setDuplexMode(pUCS, pConfig->bFullDuplex);
    
    return  FTE_RET_OK;
}
    
FTE_RET     FTE_UCS_getUART
(
    FTE_UCS_PTR     pUCS,
    FTE_UCS_UART_CONFIG_PTR pConfig
)
{
    FTE_UCS_getBaudrate(pUCS, &pConfig->nBaudrate);    
    FTE_UCS_getDatabits(pUCS, &pConfig->nDataBits);
    FTE_UCS_getParity(pUCS, &pConfig->nParity);
    FTE_UCS_getStopbits(pUCS, &pConfig->nStopBits);
    FTE_UCS_getDuplexMode(pUCS, &pConfig->bFullDuplex);
    
    return  FTE_RET_OK;
}

FTE_UINT32  FTE_UCS_clear
(   
    FTE_UCS_PTR     pUCS
)
{
    FTE_UINT32 nCount;

    FTE_SYS_LOCK_enable(&pUCS->xSEMRecv);
    nCount = _FTE_UCS_clear(pUCS);
    FTE_SYS_LOCK_disable(&pUCS->xSEMRecv);

    return  nCount;
}

FTE_UINT32     FTE_UCS_recvdLen
(
    FTE_UCS_PTR     pUCS
)
{
    return  pUCS->nRecvCount;
}

FTE_BOOL    FTE_UCS_sendBufferIsEmpty
(
    FTE_UCS_PTR     pUCS
)
{
    return  (pUCS->nSendCount == 0);
}

FTE_UINT32     FTE_UCS_recv(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen)
{
    FTE_UINT32 nCount = 0;

    _FTE_UCS_lock(pUCS);
    
    FTE_SYS_LOCK_enable(&pUCS->xSEMRecv);

    nCount = _FTE_UCS_recv(pUCS, pBuff, nBuffLen);

    FTE_SYS_LOCK_disable(&pUCS->xSEMRecv);

    _FTE_UCS_unlock(pUCS);
    
    return  nCount;
}

FTE_UINT32         FTE_UCS_recvLast(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen)
{
    FTE_UINT32 nCount = 0;

    _FTE_UCS_lock(pUCS);
    
    FTE_SYS_LOCK_enable(&pUCS->xSEMRecv);

    if (pUCS->nRecvCount > nBuffLen)
    {
        pUCS->nRecvHead = (pUCS->nRecvHead + (pUCS->nRecvCount - nBuffLen)) % pUCS->pConfig->nRecvBufLen;
        pUCS->nRecvCount = nBuffLen;
    }
    
    nCount = _FTE_UCS_recv(pUCS, pBuff, nBuffLen);

    FTE_SYS_LOCK_disable(&pUCS->xSEMRecv);

    _FTE_UCS_unlock(pUCS);
    
    return  nCount;
}

FTE_UINT32       FTE_UCS_send(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen, FTE_BOOL bHold)
{
    FTE_UINT32 nCount;

    _FTE_UCS_lock(pUCS);

    FTE_SYS_LOCK_enable(&pUCS->xSEMSend);

    nCount = _FTE_UCS_send(pUCS, pBuff, nBuffLen);

    FTE_SYS_LOCK_disable(&pUCS->xSEMSend);
    FTE_SYS_LOCK_disable(&pUCS->xSEMSendNotEmtry);

    _FTE_UCS_unlock(pUCS);

    return  nCount;
}

FTE_UINT32 FTE_UCS_sendAndRecv
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      nDataLen, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      nBuffLen, 
    FTE_UINT32      nDelay, 
    FTE_UINT32      nTimeout
)
{
    MQX_TICK_STRUCT xStart, xCurrent;
    FTE_UINT32 nCount = 0;
    FTE_BOOL bOverflow;

    _FTE_UCS_lock(pUCS);    
    
    _time_get_elapsed_ticks(&xStart);
    
    FTE_UCS_clear(pUCS);
    
    FTE_SYS_LOCK_enable(&pUCS->xSEMSend);

    nCount = _FTE_UCS_send(pUCS, pData, nDataLen);

    FTE_SYS_LOCK_disable(&pUCS->xSEMSend);
    FTE_SYS_LOCK_disable(&pUCS->xSEMSendNotEmtry);

    _time_delay(nDelay);
    
    while (FTE_UCS_recvdLen(pUCS) < nBuffLen)
    {
        _time_delay(1);
        _time_get_elapsed_ticks(&xCurrent);
        if (_time_diff_milliseconds(&xCurrent, &xStart, &bOverflow) > nTimeout)
        {
            break;
        }
    }  
	FTE_SYS_LOCK_enable(&pUCS->xSEMRecv);

    nCount = _FTE_UCS_recv(pUCS, pBuff, nBuffLen);

    FTE_SYS_LOCK_disable(&pUCS->xSEMRecv);

    _FTE_UCS_unlock(pUCS);
    
    return  nCount;
}

FTE_BOOL FTE_UCS_waitForSendCompletion
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT32      nTimeout
)
{
    MQX_TICK_STRUCT xStart, xCurrent;
    FTE_BOOL bOverflow;

    _time_get_elapsed_ticks(&xStart);

    _FTE_UCS_lock(pUCS);    
    do
    {
        _time_delay(10);
        if (pUCS->nSendCount == 0)
        {
            return  TRUE;
        }

        _time_get_elapsed_ticks(&xCurrent);
    } while(_time_diff_milliseconds(&xCurrent, &xStart, &bOverflow) < nTimeout);

    _FTE_UCS_unlock(pUCS);
    
    return  FALSE;
}

/******************************************************************************
 * MODBUS operation
 ******************************************************************************/
FTE_UINT32 FTE_UCS_MODBUS_getRegs
(
    FTE_UCS_PTR         pUCS, 
    FTE_UINT8          ucDeviceID,
    FTE_UINT16         usAddr, 
    FTE_UINT16_PTR     pRegs, 
    FTE_UINT8          nCount, 
    FTE_UINT32         nTimeout
)
{
    ASSERT((pUCS != NULL) && (pRegs != NULL));
    
    FTE_UINT8      pReqFrame[10];
    FTE_UINT32     ulReqLen;
    FTE_UINT8_PTR  pRecvFrame;
    FTE_UINT8_PTR  pRecvBuff;
    FTE_UINT16     uiCRC;
    FTE_UINT32     ulRecvLen;
    FTE_UINT32     ulValidRespFrameLen;
    FTE_UINT32     i;
    
    ulValidRespFrameLen = (5 + sizeof(FTE_UINT16)*nCount);
    pRecvBuff = (FTE_UINT8_PTR)FTE_MEM_allocZero(ulValidRespFrameLen * 2);
    if (pRecvBuff == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }    
    memset(pReqFrame, 0x00, sizeof(FTE_UINT8)*10);    
    ulReqLen = 0;
    pReqFrame[ulReqLen++] = ucDeviceID;
    pReqFrame[ulReqLen++] = 0x03;
    pReqFrame[ulReqLen++] = (usAddr >> 8) & 0xFF;
    pReqFrame[ulReqLen++] = (usAddr     ) & 0xFF;
    pReqFrame[ulReqLen++] = 0x00;
    pReqFrame[ulReqLen++] = nCount;
    uiCRC = FTE_CRC16(pReqFrame, ulReqLen);
    pReqFrame[ulReqLen++] = (uiCRC     ) & 0xFF;
    pReqFrame[ulReqLen++] = (uiCRC >> 8) & 0xFF;
	//ulReqLen++;
                
    ulRecvLen = FTE_UCS_sendAndRecv(pUCS, pReqFrame, ulReqLen, pRecvBuff, ulValidRespFrameLen * 2, 0, nTimeout);
    if (ulRecvLen < ulValidRespFrameLen)
    {
        FTE_MEM_free(pRecvBuff);
        return  FTE_RET_ERROR;
    }
    
    pRecvFrame = NULL;
    for(i = 0 ; i <= (ulRecvLen - ulValidRespFrameLen) ; i++)
    {
        if ((pRecvBuff[i] == ucDeviceID) && (pRecvBuff[i + 1] == 0x03) && (pRecvBuff[i + 2] == nCount * 2))
        {
            uiCRC = FTE_CRC16(&pRecvBuff[i], ulValidRespFrameLen - 2);
            if (uiCRC == *((FTE_UINT16_PTR)&pRecvBuff[i+ ulValidRespFrameLen - 2]))
            {            
                pRecvFrame = &pRecvBuff[i];
                break;
            }
        }
    }
    
    if (pRecvFrame == NULL)
    {
        FTE_MEM_free(pRecvBuff);
        return  FTE_RET_ERROR;
    }
    
    for(i = 0 ; i < nCount ; i++)
    {
        pRegs[i] = ((FTE_UINT16)pRecvFrame[3+i*2] << 8) | (FTE_UINT16)pRecvFrame[3+i*2+1];
    }
    
    FTE_MEM_free(pRecvBuff);
    return  FTE_RET_OK;
}

FTE_UINT32 FTE_UCS_MODBUS_setReg
(
    FTE_UCS_PTR pUCS, 
    FTE_UINT8   ucDeviceID,
    FTE_UINT16  usAddr, 
    FTE_UINT16  usValue, 
    FTE_UINT32  nTimeout
)
{
    ASSERT(pUCS != NULL);
    
    FTE_UINT8      pReqFrame[10];
    FTE_UINT32     ulReqLen;
    FTE_UINT8      pRecvBuff[10];
    FTE_UINT32     ulRecvLen;
    FTE_UINT8_PTR  pRecvFrame;
    FTE_UINT32     ulValidFrameLen;
    FTE_UINT32     uiCRC;
    FTE_UINT32     i;
    
    ulReqLen = 0;
    pReqFrame[ulReqLen++] = ucDeviceID;
    pReqFrame[ulReqLen++] = 0x06;
    pReqFrame[ulReqLen++] = (usAddr >> 8) & 0xFF;
    pReqFrame[ulReqLen++] = (usAddr     ) & 0xFF;
    pReqFrame[ulReqLen++] = (usValue>> 8) & 0xFF;
    pReqFrame[ulReqLen++] = (usValue    ) & 0xFF;
    uiCRC = FTE_CRC16(pReqFrame, ulReqLen);
    pReqFrame[ulReqLen++] = (uiCRC     ) & 0xFF;
    pReqFrame[ulReqLen++] = (uiCRC >> 8) & 0xFF;
    //pReqFrame[ulReqLen++] = 0;
                
    ulValidFrameLen = 8;
    
    ulRecvLen = FTE_UCS_sendAndRecv(pUCS, pReqFrame, ulReqLen, pRecvBuff, sizeof(pRecvBuff), 0, nTimeout);
    if (ulRecvLen < ulValidFrameLen)
    {
        return  FTE_RET_ERROR;
    }
    
    pRecvFrame = NULL;
    for(i = 0 ; i <= (ulRecvLen - ulValidFrameLen) ; i++)
    {
        if ((pRecvBuff[i] == ucDeviceID) && (pRecvBuff[i + 1] == 0x06))
        {
            uiCRC = FTE_CRC16(&pRecvBuff[i], ulValidFrameLen - 2);
            if (uiCRC == *((FTE_UINT16_PTR)&pRecvBuff[i+ ulValidFrameLen - 2]))
            {            
                pRecvFrame = &pRecvBuff[i];
                break;
            }
        }
    }
    
    if (pRecvFrame == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}


FTE_RET   _FTE_UCS_lock
(   
    FTE_UCS_PTR     pUCS
)
{
    ASSERT(pUCS != NULL);

    if (FTE_SYS_LOCK_enable(&pUCS->xSEMLock) != FTE_RET_OK)
    {
        DEBUG("\nFTE_SYS_LOCK_enable failed");
        goto error;
    }

    return  FTE_RET_OK;
error:

    return  FTE_RET_ERROR;
}

FTE_RET   _FTE_UCS_unlock
(
    FTE_UCS_PTR     pUCS
)
{
    ASSERT(pUCS != NULL);

    if (FTE_SYS_LOCK_disable(&pUCS->xSEMLock) != FTE_RET_OK)
    {
        DEBUG("\nFTE_SYS_LOCK_disable failed");
        return  FTE_RET_ERROR;
    }

    return  FTE_RET_OK;
}


FTE_INT32  FTE_UCS_SHELL_cmd
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
                FTE_UINT32 ulCount;

                ulCount = FTE_UCS_count();

                for(int i = 0 ; i < ulCount ; i++)
                {
                    FTE_UCS_PTR pUCS;

                    if ((pUCS = FTE_UCS_getAt(i)) != NULL)
                    {
                        printf("%16s : %08x\n", "ID",   pUCS->pConfig->nID);
                        printf("%16s : %s\n",   "Name", pUCS->pConfig->pName);
                        printf("%16s : %s\n",   "UART", pUCS->pConfig->pUART);
                        printf("%16s : %08x\n", "FlowCtrl", pUCS->pConfig->nFlowCtrlID);
                        printf("%16s : %d bps\n", "Baudrate", pUCS->nBaudrate);
                        printf("%16s : %d bits\n", "Data Bits", pUCS->nDataBits);
                        switch(pUCS->nParity)
                        {
                        case    FTE_UART_PARITY_NONE:   printf("%16s : %s\n", "Parity", "none"); break;
                        case    FTE_UART_PARITY_ODD:    printf("%16s : %s\n", "Parity", "odd"); break;
                        case    FTE_UART_PARITY_EVEN:   printf("%16s : %s\n", "Parity", "even"); break;
                        case    FTE_UART_PARITY_FORCE:  printf("%16s : %s\n", "Parity", "force"); break;
                        case    FTE_UART_PARITY_MARK:   printf("%16s : %s\n", "Parity", "mark"); break;
                        case    FTE_UART_PARITY_SPACE:  printf("%16s : %s\n", "Parity", "space"); break;
                        }

                        printf("%16s : %d bits\n", "Stop Bits", pUCS->nStopBits);
                    }
                }
            }
            break;
        }
    }

    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>] [ baudrate | flags | read | send ] [<len>] [<data>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>] [ baudrate | flags | read | send ] [<len>] [<data>]\n", pArgv[0]);
            printf("        id       - UART Channel \n");
            printf("        baudrate - UART speed \n");
        }
    }

    return   xRet;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
void FTE_UCS_TASK_send
(
      FTE_UINT32 nParams
)
{
    FTE_UCS_PTR pUCS = FTE_UCS_get(nParams);
    if (pUCS == NULL)
    {
        goto error;
    }

    while(TRUE)
    {
        FTE_UINT32 nWrittenCount;

        FTE_SYS_LOCK_enable(&pUCS->xSEMSendNotEmtry);
        FTE_SYS_LOCK_enable(&pUCS->xSEMSend);

        if (pUCS->pFlowCtrl != NULL)
        {
            FTE_LWGPIO_setValue(pUCS->pFlowCtrl, TRUE);
        }

        if (pUCS->pFlowCtrl2 != NULL)
        {
            FTE_LWGPIO_setValue(pUCS->pFlowCtrl2, FALSE);
        }

        while(pUCS->nSendCount != 0)
        {
            if (pUCS->nSendHead + pUCS->nSendCount <= pUCS->pConfig->nSendBufLen)
            {
                nWrittenCount = fwrite(&pUCS->pSendBuf[pUCS->nSendHead], 1, pUCS->nSendCount, pUCS->pFD);
            }
            else
            {
                nWrittenCount = fwrite(&pUCS->pSendBuf[pUCS->nSendHead], 1, pUCS->pConfig->nSendBufLen - pUCS->nSendHead, pUCS->pFD);
            }

            pUCS->nSendHead  += (pUCS->nSendHead + nWrittenCount) % pUCS->pConfig->nSendBufLen;
            pUCS->nSendCount -= nWrittenCount;
        }

        //_time_delay(1);
        //ioctl( pUCS->pFD, IO_IOCTL_SERIAL_WAIT_FOR_TC, NULL );
		
		
		fflush(pUCS->pFD);
        ioctl( pUCS->pFD, IO_IOCTL_SERIAL_WAIT_FOR_TC, NULL );
        fflush(pUCS->pFD);
		
        pUCS->nSendHead = 0;
        pUCS->nSendTail = 0;

        
        if (pUCS->pFlowCtrl != NULL)
        {
            for(int i = 0; i < 60 ;i++)
                _time_delay(0);
            if (pUCS->pFlowCtrl != NULL)
            {
                FTE_LWGPIO_setValue(pUCS->pFlowCtrl, FALSE);
            }
            if (pUCS->pFlowCtrl2 != NULL)
            {
                FTE_LWGPIO_setValue(pUCS->pFlowCtrl2, TRUE);
            }
        }

        FTE_SYS_LOCK_disable(&pUCS->xSEMSend);
    }

error:
    _task_block();
}

void    FTE_UCS_TASK_recv
(
      FTE_UINT32 nParams
)
{
    FTE_UCS_PTR pUCS = FTE_UCS_get(nParams);
    if (pUCS == NULL)
    {
        goto error;
    }

    while(TRUE)
    {
        FTE_UINT8  nData;

        if (fread(&nData, 1, 1, pUCS->pFD) != 0)
        {
            FTE_SYS_LOCK_enable(&pUCS->xSEMRecv);
            pUCS->pRecvBuf[pUCS->nRecvTail] = nData;
            pUCS->nRecvTail = (pUCS->nRecvTail + 1) % pUCS->pConfig->nRecvBufLen;
            pUCS->nRecvCount ++;

            if (pUCS->nRecvCount == pUCS->pConfig->nRecvBufLen)
            {
                pUCS->nRecvHead = (pUCS->nRecvHead + 1) % pUCS->pConfig->nRecvBufLen;
                pUCS->nRecvCount--;
            }
            FTE_SYS_LOCK_disable(&pUCS->xSEMRecv);
        }
    }

error:
    _task_block();
}


FTE_UINT32 _FTE_UCS_clear
(
    FTE_UCS_PTR     pUCS
)
{
    FTE_UINT32 nCount;

    nCount = pUCS->nRecvCount;
    pUCS->nRecvCount = 0;

    return  nCount;
}

FTE_UINT32     _FTE_UCS_recv
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      nBuffLen
)
{
    ASSERT((pUCS != NULL) && (pBuff != NULL));
    
    FTE_UINT32 nCount = 0;

    if (pUCS->nRecvCount != 0)
    {
        if (nBuffLen > pUCS->nRecvCount)
        {
            nCount = pUCS->nRecvCount;
        }
        else
        {
            nCount = nBuffLen;
        }

        if (pUCS->nRecvHead + nCount < pUCS->pConfig->nRecvBufLen)
        {
            memcpy(pBuff, &pUCS->pRecvBuf[pUCS->nRecvHead], nCount);
        }
        else 
        {
            FTE_UINT32 nTailCount = pUCS->pConfig->nRecvBufLen - pUCS->nRecvHead;

            memcpy(pBuff, &pUCS->pRecvBuf[pUCS->nRecvHead], nTailCount);
            memcpy(&pBuff[nTailCount], &pUCS->pRecvBuf[0], nCount - nTailCount);
        }

        pUCS->nRecvHead = (pUCS->nRecvHead + nCount) % pUCS->pConfig->nRecvBufLen;
        pUCS->nRecvCount -=nCount;

        if (pUCS->nRecvCount == 0)
        {
            pUCS->nRecvHead = 0;
            pUCS->nRecvTail = 0;
        }
    }

    return  nCount;
}
 
FTE_UINT32       _FTE_UCS_send
(
    FTE_UCS_PTR     pUCS, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      nBuffLen
)
{
    ASSERT((pUCS != NULL) && (pBuff != NULL));
    
    FTE_UINT32 nCount;

    if (nBuffLen > (pUCS->pConfig->nSendBufLen - pUCS->nSendCount - 1))
    {
        nCount = pUCS->pConfig->nSendBufLen - pUCS->nSendCount - 1;
    }
    else
    {
        nCount = nBuffLen;
    }

    if (pUCS->nSendTail + nCount < pUCS->pConfig->nSendBufLen)
    {
        memcpy(&pUCS->pSendBuf[pUCS->nSendTail], pBuff, nCount);
    }
    else
    {
        FTE_UINT32 nTailCount = pUCS->pConfig->nRecvBufLen - pUCS->nSendTail;

        memcpy(&pUCS->pSendBuf[pUCS->nSendTail], pBuff, nTailCount);
        memcpy(&pUCS->pSendBuf[0], &pBuff[nTailCount], nCount - nTailCount);
    }

    pUCS->nSendTail = (pUCS->nSendTail + nCount) % pUCS->pConfig->nSendBufLen;
    pUCS->nSendCount += nCount;

    return  nCount;
}

#endif