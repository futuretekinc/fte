#include "fte_target.h"
#include "fte_spi.h"

#define MAX_SPI_COUNT   3
static _mqx_int    _FTE_SPI_setCS(FTE_UINT32 cs_mask, pointer user_data);
static FTE_RET  _FTE_SPI_lock(FTE_SPI_PTR pSPI);
static FTE_RET  _FTE_SPI_unlock(FTE_SPI_PTR pSPI);

static const   FTE_CHAR_PTR channels[MAX_SPI_COUNT] = 
{
    "spi0:", 
    "spi1:",
    "spi2:"
};

static FTE_SPI_CHANNEL    _pChannels[MAX_SPI_COUNT] = 
{
    { .xFD = NULL },
    { .xFD = NULL },
    { .xFD = NULL }
};

static FTE_SPI_PTR  _pHead  = NULL;
static FTE_UINT32      _nSPI   = 0;


FTE_RET   FTE_SPI_create
(
    FTE_SPI_CONFIG_PTR  pConfig
)
{
    FTE_SPI_PTR pSPI;
    
    pSPI = (FTE_SPI_PTR)FTE_MEM_allocZero(sizeof(FTE_SPI));
    if (pSPI == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }
    
    _pChannels[pConfig->xPort].nBaudrate    = pConfig->nBaudrate;
    _pChannels[pConfig->xPort].xFlags       = pConfig->xFlags;

    pSPI->pConfig           = pConfig;
    pSPI->pChannel          = &_pChannels[pConfig->xPort];
    pSPI->callback.CALLBACK = _FTE_SPI_setCS;
    pSPI->callback.USERDATA = (pointer)pSPI;    
    pSPI->pNext             = _pHead;
    
    _pHead = pSPI;
    _nSPI++;
        
    return  MQX_OK;
}

FTE_RET   FTE_SPI_attach
(
    FTE_SPI_PTR     pSPI, 
    FTE_UINT32      nParent
)
{
    FTE_UINT32     nValue;

    assert(pSPI != NULL);
    if (pSPI == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
        
    if (pSPI->pChannel->xFD == NULL)
    {
        /* Open the SPI driver */
        pSPI->pChannel->xFD = fopen (channels[pSPI->pConfig->xPort], NULL);
        if (pSPI->pChannel->xFD == NULL)
        {
            goto error;
        }
        
        if (_lwsem_create(&pSPI->pChannel->xLWSEM, 1) != MQX_OK)
        {
            goto error;
        }

        nValue = pSPI->pConfig->nBaudrate;
        if (SPI_OK != ioctl (pSPI->pChannel->xFD, IO_IOCTL_SPI_SET_BAUD, &nValue))
        {
            goto error;
        }

        nValue = pSPI->pConfig->xFlags;
        if (SPI_OK != ioctl (pSPI->pChannel->xFD, IO_IOCTL_SPI_SET_FLAGS, &nValue))
        {
            goto error;
        }
        
        nValue = SPI_DEVICE_MASTER_MODE;
        if (SPI_OK != ioctl (pSPI->pChannel->xFD, IO_IOCTL_SPI_SET_TRANSFER_MODE, &nValue ))
        {
            goto error;
        }
        
#ifdef FTE_LORA_SUPPORTED
        nValue = SPI_CLK_POL_PHA_MODE1; // use lora
#else
        nValue = SPI_CLK_POL_PHA_MODE3; // use mcp23s08
#endif
         if (SPI_OK != ioctl (pSPI->pChannel->xFD, IO_IOCTL_SPI_SET_MODE, &nValue))
        {
            goto error;
        }
        
        pSPI->pChannel->nCount = 1;
    }  
    else
    {
        pSPI->pChannel->nCount++;
    }

    pSPI->pCSPort = FTE_LWGPIO_get(pSPI->pConfig->xCSGPIO);
    if (pSPI->pCSPort == NULL)
    {
        goto error;
    }
    
    if (FTE_LWGPIO_attach(pSPI->pCSPort, pSPI->pConfig->nID) != MQX_OK)
    {
        goto error;
    }
    
    pSPI->nParent = nParent;
    
    return  MQX_OK;
    
error:
    
    if (pSPI->pChannel->xFD != NULL)
    {
        fclose(pSPI->pChannel->xFD);
        pSPI->pChannel->xFD = NULL;
    }
    
    if (pSPI->pCSPort != NULL)
    {
        FTE_LWGPIO_detach(pSPI->pCSPort);    
    }
    
    return  MQX_ERROR;
}

FTE_RET   FTE_SPI_detach
(
    FTE_SPI_PTR     pSPI
)
{
    assert(pSPI != NULL);
    if (pSPI == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    if (pSPI->pChannel->nCount == 1)
    {
        /* Open the SPI driver */
        fclose(pSPI->pChannel->xFD);        
        pSPI->pChannel->xFD = NULL;
        _lwsem_destroy(&pSPI->pChannel->xLWSEM);        
        pSPI->pChannel->nCount = 0;
    }  
    else
    {
        pSPI->pChannel->nCount--;
    }

    FTE_LWGPIO_detach(pSPI->pCSPort);
    pSPI->nParent = 0;
    
    return  MQX_OK;
}

FTE_UINT32     FTE_SPI_count(void)
{
    return  _nSPI;
}

FTE_SPI_PTR FTE_SPI_getFirst(void)
{
    return  _pHead;
}

FTE_SPI_PTR FTE_SPI_getNext
(
    FTE_SPI_PTR     pSPI
)
{
    if (pSPI == NULL)
    {
        return  NULL;
    }
    
    return  pSPI->pNext;
}

FTE_SPI_PTR FTE_SPI_get
(
    FTE_UINT32  nID
)
{
    FTE_SPI_PTR pSPI;
    
    pSPI = _pHead;
    while(pSPI != NULL)
    {
        if (pSPI->pConfig->nID == nID)
        {
            return  pSPI;
        }
        
        pSPI = pSPI->pNext;
    }

    return  NULL;
}

FTE_UINT32     FTE_SPI_getParent
(
    FTE_SPI_PTR     pSPI
)
{
    assert(pSPI != NULL);
    
    return  pSPI->nParent;
}


FTE_RET   FTE_SPI_read
(   
    FTE_SPI_PTR     pSPI, 
    FTE_UINT8_PTR   cmd, 
    FTE_UINT32      cmd_len, 
    FTE_UINT8_PTR   buff, 
    FTE_UINT32      buff_len
)
{
    assert(pSPI != NULL);
    if (pSPI == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    _FTE_SPI_lock(pSPI);

#ifdef  DEBUG_SPI
    printf("RD : ");
    for(int i = 0 ; i < cmd_len ; i++)
    {
        printf("%02x ", cmd[i]);
    }
     printf("-> ");
#endif
    
    if (cmd_len != fwrite(cmd, 1, cmd_len, pSPI->pChannel->xFD))
    {
        goto error;
    }
    
    if (buff_len != fread(buff, 1, buff_len, pSPI->pChannel->xFD))
    {
        goto error;
    }
  
#ifdef  DEBUG_SPI
    for(int i = 0 ; i < buff_len ; i++)
    {
        printf("%02x ", buff[i]);
    }
    printf("\n");
#endif
    
    _FTE_SPI_unlock(pSPI);
    return MQX_OK;
    
error:  
    _FTE_SPI_unlock(pSPI);
    
    return MQX_ERROR;
}

FTE_RET   FTE_SPI_write
(
    FTE_SPI_PTR     pSPI, 
    FTE_UINT8_PTR   cmd, 
    FTE_UINT32      cmd_len, 
    FTE_UINT8_PTR   buff, 
    FTE_UINT32      buff_len
)
{
    assert(pSPI != NULL);
    if (pSPI == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    _FTE_SPI_lock(pSPI);
#if 1
#ifdef  DEBUG_SPI
    printf("WR : ");
    for(int i = 0 ; i < cmd_len ; i++)
    {
        printf("%02x ", cmd[i]);
    }
    printf("-> ");
#endif
    
    if (cmd_len != fwrite(cmd, 1, cmd_len, pSPI->pChannel->xFD))
    {
        goto error;
    }
  
#ifdef  DEBUG_SPI
    for(int i = 0 ; i < buff_len ; i++)
    {
        printf("%02x ", buff[i]);
    }
    printf("\n");
#endif    
    if (buff_len != 0)
    {
        if (buff_len != fwrite(buff, 1, buff_len, pSPI->pChannel->xFD))
        {
            goto error;
        }
    }
#else
    static uint_8 pBuff[64];
    static uint_8 ulBuffLen = 0;
    
    memcpy(&pBuff[ulBuffLen], cmd, cmd_len);
    ulBuffLen = cmd_len;
    memcpy(&pBuff[ulBuffLen], buff, buff_len);
    ulBuffLen += buff_len;
   
    if (ulBuffLen != fwrite(pBuff, 1, ulBuffLen, pSPI->pChannel->xFD))
    {
        goto error;
    }
    
#endif
    fflush(pSPI->pChannel->xFD);
    
    _FTE_SPI_unlock(pSPI);
    
    
    return MQX_OK;
    
error:  

    _FTE_SPI_unlock(pSPI);
    
    return MQX_ERROR;
}

FTE_RET   FTE_SPI_setBaudrate
(
    FTE_SPI_PTR     pSPI, 
    FTE_UINT32      baudrate
)
{
    return  MQX_OK;
}

FTE_RET   FTE_SPI_getBaudrate
(
    FTE_SPI_PTR     pSPI, 
    FTE_UINT32_PTR  baudrate
)
{
    assert(pSPI != NULL);
    if (pSPI == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    *baudrate = pSPI->pConfig->nBaudrate;

    return  MQX_OK;
}

FTE_RET   FTE_SPI_setFlags
(
    FTE_SPI_PTR     pSPI, 
    FTE_UINT32      flags
)
{
    return  MQX_OK;
}

FTE_RET   FTE_SPI_getFlags
(
    FTE_SPI_PTR     pSPI, 
    FTE_UINT32_PTR  flags
)
{
    assert(pSPI != NULL);
    if (pSPI == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    *flags = pSPI->pConfig->xFlags;

    return  MQX_OK;
}

FTE_INT32  FTE_SPI_SHELL_cmd
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
                FTE_SPI_PTR pSPI;

                printf("%8s %10s %10s\n", "ID", "Baudrate", "Flags");
                
                pSPI = FTE_SPI_getFirst();
                while(pSPI != NULL)
                {
                    FTE_UINT32 baudrate, flags;
                    
                    FTE_SPI_getBaudrate(pSPI, &baudrate);                         
                    FTE_SPI_getFlags(pSPI, &flags);

                    printf("%08x %10d %10d\n", pSPI->pConfig->nID, baudrate, flags);
                    
                    pSPI = pSPI->pNext;
                }
            }
            break;
        default:
            {
                if (strcmp(pArgv[2], "read") == 0)
                {
                    FTE_SPI_PTR pSPI;
                    FTE_UINT32 id, nCmdLen, nRecvLen = 1, i;
                    uint_8  pSendBuff[64];
                    uint_8  pRecvBuff[64];
                    
                    if (nArgc < 4)
                    {
                       xRet = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (! Shell_parse_hexnum( pArgv[1], &id))  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    pSPI = FTE_SPI_get(id);
                    if (pSPI == NULL)
                    {
                        goto error;
                    }
                    
                    if (! Shell_parse_number( pArgv[3], &nCmdLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (nArgc < 4 + nCmdLen + 1)
                    {
                       xRet = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    for(i = 0 ; i < nCmdLen ; i++)
                    {
                        FTE_UINT32 nValue;
                        Shell_parse_hexnum( pArgv[4 + i], &nValue);
                        pSendBuff[i] = nValue;
                    }
                    
                    if (! Shell_parse_number( pArgv[4 + nCmdLen], &nRecvLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (MQX_OK != FTE_SPI_read(pSPI, (uint_8 *)&pSendBuff, nCmdLen, pRecvBuff, nRecvLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    for (i = 0 ; i < nRecvLen ; i++)
                    {
                        printf("%02x ", pRecvBuff[i]);
                    }
                    printf("\n");
                }
                else if (strcmp(pArgv[2], "write") == 0)
                {
                    FTE_SPI_PTR pSPI;
                    FTE_UINT32 nID, nSendLen, i;
                    uint_8 pSendBuff[64];
                     
                    if (nArgc < 6)
                    {
                       xRet = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (! Shell_parse_hexnum( pArgv[1], &nID))  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    pSPI = FTE_SPI_get(nID);
                    
                    if (! Shell_parse_number( pArgv[3], &nSendLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    for(i = 0 ; i < nSendLen ; i++)
                    {
                        FTE_UINT32 nValue;
                        if (! Shell_parse_hexnum( pArgv[4+i], &nValue))
                        {
                           xRet = SHELL_EXIT_ERROR;
                           goto error;
                        }
                        pSendBuff[i] = nValue;
                    }
                    
                    if (MQX_OK != FTE_SPI_write(pSPI, pSendBuff, nSendLen, NULL, 0))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                }
                else if (strcmp(pArgv[2], "attach") == 0)
                {
                    FTE_SPI_PTR pSPI;
                    FTE_UINT32 nID;
                     
                    if (nArgc < 3)
                    {
                       xRet = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (! Shell_parse_hexnum( pArgv[1], &nID))  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    pSPI = FTE_SPI_get(nID);                    
                    if (pSPI == NULL)
                    {
                        goto error;
                    }

                    FTE_SPI_attach(pSPI, 0);
                }
                else
                {
                    bPrintUsage = TRUE;
                }
            }
        }
    }

error:    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>] [ baudrate | flags | read | write ] [<len>] [<data>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>] [ baudrate | flags | read | write ] [<len>] [<data>]\n", pArgv[0]);
            printf("        id       - SPI Channel \n");
            printf("        baudrate - SPI speed \n");
        }
    }

    return   xRet;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
FTE_RET   _FTE_SPI_lock
(
    FTE_SPI_PTR     pSPI
)
{
    assert(pSPI != NULL);

    //_int_enable();    
    
    if (_lwsem_wait(&pSPI->pChannel->xLWSEM) != MQX_OK)
    {  
//        DEBUG("\n_lwsem_wait failed");
        goto error;
    }
    
    if (SPI_OK != ioctl (pSPI->pChannel->xFD, IO_IOCTL_SPI_SET_CS_CALLBACK, &pSPI->callback))
    {
        goto error;
    }
    
    return  MQX_OK;
error:
    
    //_int_disable();    
    return  MQX_ERROR;
}

FTE_RET   _FTE_SPI_unlock
(
    FTE_SPI_PTR     pSPI
)
{   
    assert(pSPI != NULL);
    
    fflush(pSPI->pChannel->xFD);
    
    ioctl (pSPI->pChannel->xFD, IO_IOCTL_SPI_SET_CS_CALLBACK, NULL);
    
    if (_lwsem_post(&pSPI->pChannel->xLWSEM) != MQX_OK)
    {
        DEBUG("\n_lwsem_post failed");
    //_int_disable();    
        return  MQX_ERROR;
    }
    
    //_int_disable();    
    
    return  MQX_OK;
}

static 
_mqx_int _FTE_SPI_setCS
(
    FTE_UINT32  cs_mask, 
    pointer     user_data
)
{
    FTE_SPI_PTR  pSPI = (FTE_SPI_PTR)user_data;
    if (pSPI != NULL)
    {
        
        if (cs_mask & 0x01)
        {
            FTE_LWGPIO_setValue(pSPI->pCSPort, TRUE);
        }
        else
        {
            FTE_LWGPIO_setValue(pSPI->pCSPort, FALSE);
        }
    }
    return MQX_OK;
}