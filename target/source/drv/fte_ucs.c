#include "fte_target.h"
#include "fte_ucs.h"

#if FTE_UCS_SUPPORTED

static  _mqx_uint   _FTE_UCS_lock(FTE_UCS_PTR pUCS);
static  _mqx_uint   _FTE_UCS_unlock(FTE_UCS_PTR pUCS);

static  uint_32     _FTE_UCS_clear(FTE_UCS_PTR pUCS);
static  uint_32     _FTE_UCS_recv(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen);
static  uint_32     _FTE_UCS_send(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen);

static  FTE_LIST    xUCSList = { 0, NULL };

_mqx_uint   FTE_UCS_create(FTE_UCS_CONFIG_CONST_PTR pConfig)
{
    FTE_UCS_PTR pUCS = NULL;

    if (xUCSList.pHead == NULL)
    {
        FTE_LIST_init(&xUCSList);
    }

    pUCS = (FTE_UCS_PTR)FTE_MEM_allocZero(sizeof(FTE_UCS));
    if (pUCS == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }

    pUCS->pRecvBuf = (uint_8_ptr)FTE_MEM_allocZero(pConfig->nRecvBufLen);
    pUCS->pSendBuf= (uint_8_ptr)FTE_MEM_allocZero(pConfig->nSendBufLen);

    FTE_LIST_init(&pUCS->xParents);

    _lwsem_create(&pUCS->xSEMLock, 1);
    _lwsem_create(&pUCS->xSEMSend, 1);
    _lwsem_create(&pUCS->xSEMRecv, 1);
    _lwsem_create(&pUCS->xSEMSendNotEmtry, 0);

    pUCS->bFullDuplex=pConfig->bFullDuplex;
    pUCS->nBaudrate = pConfig->nBaudrate;
    pUCS->nDataBits = pConfig->nDataBits;
    pUCS->nParity   = pConfig->nParity;
    pUCS->nStopBits = pConfig->nStopBits;

    pUCS->pConfig   = pConfig;

    FTE_LIST_pushBack(&xUCSList, pUCS);

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_destroy(FTE_UCS_PTR pUCS)
{
    _lwsem_destroy(&pUCS->xSEMLock);
    _lwsem_destroy(&pUCS->xSEMSend);
    _lwsem_destroy(&pUCS->xSEMRecv);
    _lwsem_destroy(&pUCS->xSEMSendNotEmtry);

    FTE_LIST_final(&pUCS->xParents);
    FTE_LIST_remove(&xUCSList, pUCS);

    FTE_MEM_free(pUCS->pRecvBuf);
    FTE_MEM_free(pUCS->pSendBuf);

    FTE_MEM_free(pUCS);

    return  MQX_OK;
}

_mqx_uint       FTE_UCS_attach(FTE_UCS_PTR pUCS, uint_32 nParent)
{
    ASSERT(pUCS != NULL);

    if (FTE_UCS_init(pUCS) != MQX_OK)
    {
        return  MQX_ERROR;
    }

    if (pUCS->pConfig->nFlowCtrlID != 0)
    {
        pUCS->pFlowCtrl = FTE_LWGPIO_get(pUCS->pConfig->nFlowCtrlID);
        if (pUCS->pFlowCtrl != NULL)
        {
            FTE_LWGPIO_attach(pUCS->pFlowCtrl, pUCS->pConfig->nID);
        }

        pUCS->pFlowCtrl2 = FTE_LWGPIO_get(pUCS->pConfig->nFlowCtrl2ID);
        if (pUCS->pFlowCtrl2 != NULL)
        {
            FTE_LWGPIO_attach(pUCS->pFlowCtrl2, pUCS->pConfig->nID);
        }
    }

    FTE_LIST_pushBack(&pUCS->xParents, (pointer)nParent);

    return  MQX_OK;
}

_mqx_uint       FTE_UCS_detach(FTE_UCS_PTR pUCS, uint_32 nParent)
{
    ASSERT(pUCS != NULL);

    FTE_LIST_remove(&pUCS->xParents, (pointer)nParent);

    if (FTE_LIST_count(&pUCS->xParents) == 0)
    {
        if (pUCS->pFlowCtrl != NULL)
        {
            FTE_LWGPIO_detach(pUCS->pFlowCtrl);
            pUCS->pFlowCtrl = NULL;
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

     return  MQX_OK;
}

FTE_UCS_PTR     FTE_UCS_get(uint_32 nID)
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

uint_32         FTE_UCS_count(void)
{
    return  FTE_LIST_count(&xUCSList);
}

FTE_UCS_PTR FTE_UCS_getAt(uint_32 ulIndex)
{
    return  (FTE_UCS_PTR)FTE_LIST_getAt(&xUCSList, ulIndex);
}

_mqx_uint   FTE_UCS_init(FTE_UCS_PTR pUCS)
{
    uint_32 nValue;

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
                pUCS->pFD = fopen( pUCS->pConfig->pUART, (char const *)IO_SERIAL_HW_485_FLOW_CONTROL);
            }
            else
            {
                pUCS->pFD = fopen( pUCS->pConfig->pUART, 0 );
            }
        }


        if (pUCS->pFD == NULL)
        {
            return  MQX_INVALID_PARAMETER;
        }

        nValue = pUCS->nBaudrate;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_BAUD, &nValue);
        nValue = pUCS->nDataBits;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_DATA_BITS, &nValue);
        nValue = pUCS->nParity;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_PARITY, &nValue);
        nValue = pUCS->nStopBits;
        ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_STOP_BITS, &nValue);

        pUCS->hTaskRX = _task_create(0, FTE_TASK_UCS_RX, pUCS->pConfig->nID);
        if (pUCS->hTaskRX == MQX_NULL_TASK_ID)
        {
            goto error;
        }
        pUCS->hTaskTX = _task_create(0, FTE_TASK_UCS_TX, pUCS->pConfig->nID);
    }

    return  MQX_OK;

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

    return  MQX_ERROR;
}

_mqx_uint   FTE_UCS_setBaudrate(FTE_UCS_PTR pUCS, uint_32 nBaudrate)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_BAUD, &nBaudrate))
    {
        return  MQX_ERROR;
    }

    pUCS->nBaudrate = nBaudrate;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_getBaudrate(FTE_UCS_PTR pUCS, uint_32 *pBaudrate)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    *pBaudrate = pUCS->nBaudrate;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_setDatabits(FTE_UCS_PTR pUCS, uint_32 nDataBits)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_DATA_BITS, &nDataBits))
    {
        return  MQX_ERROR;
    }

    pUCS->nDataBits = nDataBits;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_getDatabits(FTE_UCS_PTR pUCS, uint_32 *pDataBits)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    *pDataBits = pUCS->nDataBits;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_setParity(FTE_UCS_PTR pUCS, uint_32 nParity)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_PARITY, &nParity))
    {
        return  MQX_ERROR;
    }

    pUCS->nParity = nParity;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_getParity(FTE_UCS_PTR pUCS, uint_32_ptr pParity)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    *pParity = pUCS->nParity;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_setStopbits(FTE_UCS_PTR pUCS, uint_32 nStopBits)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    if (IO_OK != ioctl (pUCS->pFD, IO_IOCTL_SERIAL_SET_STOP_BITS, &nStopBits))
    {
        return  MQX_ERROR;
    }

    pUCS->nStopBits= nStopBits;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_getStopbits(FTE_UCS_PTR pUCS, uint_32_ptr pStopBits)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    *pStopBits = pUCS->nStopBits;

    return  MQX_OK;
}


_mqx_uint   FTE_UCS_setDuplexMode(FTE_UCS_PTR pUCS, boolean bFullDuplex)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    pUCS->bFullDuplex = bFullDuplex;

    return  MQX_OK;
}

_mqx_uint   FTE_UCS_getDuplexMode(FTE_UCS_PTR pUCS, boolean *pFullDuplex)
{
    ASSERT(pUCS != NULL);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    *pFullDuplex = pUCS->bFullDuplex;

    return  MQX_OK;
}

uint_32         FTE_UCS_clear(FTE_UCS_PTR pUCS)
{
    uint_32 nCount;

    _lwsem_wait(&pUCS->xSEMRecv);
    nCount = _FTE_UCS_clear(pUCS);
    _lwsem_post(&pUCS->xSEMRecv);

    return  nCount;
}

uint_32     FTE_UCS_recvdLen(FTE_UCS_PTR pUCS)
{
    return  pUCS->nRecvCount;
}

boolean         FTE_UCS_sendBufferIsEmpty(FTE_UCS_PTR pUCS)
{
    return  (pUCS->nSendCount == 0);
}

uint_32     FTE_UCS_recv(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen)
{
    uint_32 nCount = 0;

    _FTE_UCS_lock(pUCS);
    
    _lwsem_wait(&pUCS->xSEMRecv);

    nCount = _FTE_UCS_recv(pUCS, pBuff, nBuffLen);

    _lwsem_post(&pUCS->xSEMRecv);

    _FTE_UCS_unlock(pUCS);
    
    return  nCount;
}

uint_32         FTE_UCS_recvLast(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen)
{
    uint_32 nCount = 0;

    _FTE_UCS_lock(pUCS);
    
    _lwsem_wait(&pUCS->xSEMRecv);

    if (pUCS->nRecvCount > nBuffLen)
    {
        pUCS->nRecvHead = (pUCS->nRecvHead + (pUCS->nRecvCount - nBuffLen)) % pUCS->pConfig->nRecvBufLen;
        pUCS->nRecvCount -=nBuffLen;
    }
    
    nCount = _FTE_UCS_recv(pUCS, pBuff, nBuffLen);

    _lwsem_post(&pUCS->xSEMRecv);

    _FTE_UCS_unlock(pUCS);
    
    return  nCount;
}

uint_32       FTE_UCS_send(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen, boolean bHold)
{
    uint_32 nCount;

    _FTE_UCS_lock(pUCS);

    _lwsem_wait(&pUCS->xSEMSend);

    nCount = _FTE_UCS_send(pUCS, pBuff, nBuffLen);

    _lwsem_post(&pUCS->xSEMSend);
    _lwsem_post(&pUCS->xSEMSendNotEmtry);

    _FTE_UCS_unlock(pUCS);

    return  nCount;
}

uint_32 FTE_UCS_sendAndRecv(FTE_UCS_PTR pUCS, uint_8_ptr pData, uint_32 nDataLen, uint_8_ptr pBuff, uint_32 nBuffLen, uint_32 nDelay, uint_32 nTimeout)
{
    MQX_TICK_STRUCT xStart, xCurrent;
    uint_32 nCount = 0;
    boolean bOverflow;

    _FTE_UCS_lock(pUCS);    
    
    _time_get_ticks(&xStart);
    
    FTE_UCS_clear(pUCS);
    
    _lwsem_wait(&pUCS->xSEMSend);

    nCount = _FTE_UCS_send(pUCS, pData, nDataLen);

    _lwsem_post(&pUCS->xSEMSend);
    _lwsem_post(&pUCS->xSEMSendNotEmtry);
    
    _time_delay(nDelay);
    
    while (FTE_UCS_recvdLen(pUCS) < nBuffLen)
    {
        _time_delay(1);
        _time_get_ticks(&xCurrent);
        if (_time_diff_milliseconds(&xCurrent, &xStart, &bOverflow) > nTimeout)
        {
            break;
        }
    }
  
    _lwsem_wait(&pUCS->xSEMRecv);

    nCount = _FTE_UCS_recv(pUCS, pBuff, nBuffLen);

    _lwsem_post(&pUCS->xSEMRecv);

    _FTE_UCS_unlock(pUCS);
    
    return  nCount;
}

boolean FTE_UCS_waitForSendCompletion(FTE_UCS_PTR pUCS, uint_32 nTimeout)
{
    MQX_TICK_STRUCT xStart, xCurrent;
    boolean bOverflow;

    _time_get_ticks(&xStart);

    _FTE_UCS_lock(pUCS);    
    do
    {
        _time_delay(10);
        if (pUCS->nSendCount == 0)
        {
            return  TRUE;
        }

        _time_get_ticks(&xCurrent);
    } while(_time_diff_milliseconds(&xCurrent, &xStart, &bOverflow) < nTimeout);

    _FTE_UCS_unlock(pUCS);
    
    return  FALSE;
}

_mqx_uint   _FTE_UCS_lock(FTE_UCS_PTR pUCS)
{
    ASSERT(pUCS != NULL);

    if (_lwsem_wait(&pUCS->xSEMLock) != MQX_OK)
    {
        DEBUG("\n_lwsem_wait failed");
        goto error;
    }

    return  MQX_OK;
error:

    return  MQX_ERROR;
}

_mqx_uint   _FTE_UCS_unlock(FTE_UCS_PTR pUCS)
{
    ASSERT(pUCS != NULL);

    if (_lwsem_post(&pUCS->xSEMLock) != MQX_OK)
    {
        DEBUG("\n_lwsem_post failed");
        return  MQX_ERROR;
    }

    return  MQX_OK;
}


int_32  FTE_UCS_SHELL_cmd(int_32 argc, char_ptr argv[] )
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
                uint_32 ulCount;

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
                        printf("%16s : %d bps\n", "Baudrate", pUCS->pConfig->nBaudrate);
                        printf("%16s : %d bits\n", "Data Bits", pUCS->pConfig->nDataBits);
                        switch(pUCS->pConfig->nParity)
                        {
                        case    FTE_UART_PARITY_NONE:   printf("%16s : %s\n", "Parity", "none"); break;
                        case    FTE_UART_PARITY_ODD:    printf("%16s : %s\n", "Parity", "odd"); break;
                        case    FTE_UART_PARITY_EVEN:   printf("%16s : %s\n", "Parity", "even"); break;
                        case    FTE_UART_PARITY_FORCE:  printf("%16s : %s\n", "Parity", "force"); break;
                        case    FTE_UART_PARITY_MARK:   printf("%16s : %s\n", "Parity", "mark"); break;
                        case    FTE_UART_PARITY_SPACE:  printf("%16s : %s\n", "Parity", "space"); break;
                        }

                        printf("%16s : %d bits\n", "Stop Bits", pUCS->pConfig->nStopBits);
                    }
                }
            }
            break;

        case    2:
            {
                FTE_UCS_PTR pUCS = FTE_UCS_get(FTE_DEV_UCS_1);
                if (pUCS == NULL)
                {
                    printf("UCS not exists.\n");
                    break;
                }

                if (strcmp(argv[1], "attach") == 0)
                {
                    FTE_UCS_attach(pUCS, FTE_DEV_TYPE_ROOT);
                }
                else
                {
                    FTE_UCS_detach(pUCS, FTE_DEV_TYPE_ROOT);
                }
            }
            break;
        }
    }

    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<id>] [ baudrate | flags | read | send ] [<len>] [<data>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<id>] [ baudrate | flags | read | send ] [<len>] [<data>]\n", argv[0]);
            printf("        id       - UART Channel \n");
            printf("        baudrate - UART speed \n");
        }
    }

    return   return_code;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
void FTE_UCS_TASK_send
(
      uint_32 nParams
)
{
    FTE_UCS_PTR pUCS = FTE_UCS_get(nParams);
    if (pUCS == NULL)
    {
        goto error;
    }

    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    while(1)
    {
        uint_32 nWrittenCount;

        _lwsem_wait(&pUCS->xSEMSendNotEmtry);
        _lwsem_wait(&pUCS->xSEMSend);

        if (pUCS->pFlowCtrl != NULL)
        {
//            FTE_LWGPIO_setValue(pUCS->pFlowCtrl2, FALSE);
            FTE_LWGPIO_setValue(pUCS->pFlowCtrl, TRUE);
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

        fflush(pUCS->pFD);
        ioctl( pUCS->pFD, IO_IOCTL_SERIAL_WAIT_FOR_TC, NULL );

        pUCS->nSendHead = 0;
        pUCS->nSendTail = 0;

        if (pUCS->pFlowCtrl != NULL)
        {
            FTE_LWGPIO_setValue(pUCS->pFlowCtrl, FALSE);
 //           FTE_LWGPIO_setValue(pUCS->pFlowCtrl2, TRUE);
        }

        _lwsem_post(&pUCS->xSEMSend);
    }

error:
    _task_block();
}

void    FTE_UCS_TASK_recv
(
      uint_32 nParams
)
{
    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    FTE_UCS_PTR pUCS = FTE_UCS_get(nParams);
    if (pUCS == NULL)
    {
        goto error;
    }

    while(1)
    {
        uint_8  nData;

        if (fread(&nData, 1, 1, pUCS->pFD) != 0)
        {
            _lwsem_wait(&pUCS->xSEMRecv);
            pUCS->pRecvBuf[pUCS->nRecvTail] = nData;
            pUCS->nRecvTail = (pUCS->nRecvTail + 1) % pUCS->pConfig->nRecvBufLen;
            pUCS->nRecvCount ++;

            if (pUCS->nRecvCount == pUCS->pConfig->nRecvBufLen)
            {
                pUCS->nRecvHead = (pUCS->nRecvHead + 1) % pUCS->pConfig->nRecvBufLen;
                pUCS->nRecvCount--;
            }
            _lwsem_post(&pUCS->xSEMRecv);
        }
    }

error:
    _task_block();
}


uint_32 _FTE_UCS_clear(FTE_UCS_PTR pUCS)
{
    uint_32 nCount;

    nCount = pUCS->nRecvCount;
    pUCS->nRecvCount = 0;

    return  nCount;
}

uint_32     _FTE_UCS_recv(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen)
{
    uint_32 nCount = 0;

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
            uint_32 nTailCount = pUCS->pConfig->nRecvBufLen - pUCS->nRecvHead;

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

uint_32       _FTE_UCS_send(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen)
{
    uint_32 nCount;

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
        uint_32 nTailCount = pUCS->pConfig->nRecvBufLen - pUCS->nSendTail;

        memcpy(&pUCS->pSendBuf[pUCS->nSendTail], pBuff, nTailCount);
        memcpy(&pUCS->pSendBuf[0], &pBuff[nTailCount], nCount - nTailCount);
    }

    pUCS->nSendTail = (pUCS->nSendTail + nCount) % pUCS->pConfig->nSendBufLen;
    pUCS->nSendCount += nCount;

    return  nCount;
}

#endif
