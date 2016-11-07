#include "fte_target.h"

#if FTE_MCP23S08_SUPPORTED
#define FTE_MCP23S08_UPDATE_TIME        1000
#define FTE_MCP23S08_RESET_DELAY        10   // 5ms

#define FTE_MCP23S08_FLAG_RUN           0x01
 

static FTE_RET        _FTE_MCP23S08_init(FTE_MCP23S08_PTR pMCP23S08);
static FTE_RET        _FTE_MCP23S08_reset(FTE_MCP23S08_PTR pDev);
static FTE_RET        _FTE_MCP23S08_setReg(FTE_MCP23S08_PTR pDev, FTE_UINT32 nReg, FTE_UINT32 nValue);
static FTE_RET        _FTE_MCP23S08_getRegs(FTE_MCP23S08_PTR pDev, FTE_UINT32 nReg, FTE_UINT8_PTR pData, FTE_UINT32 nData);
static FTE_RET        _FTE_MCP23S08_INT_enable(FTE_MCP23S08_PTR pDev);
static FTE_RET        _FTE_MCP23S08_INT_disable(FTE_MCP23S08_PTR pDev);
static FTE_RET        _FTE_MCP23S08_TIMER_start(FTE_MCP23S08_PTR pDev);
static void             _FTE_MCP23S08_TIMER_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static FTE_RET        _FTE_MCP23S08_POLL_start(FTE_MCP23S08_PTR pDev);
static FTE_RET        _FTE_MCP23S08_POLL_stop(FTE_MCP23S08_PTR pDev);

static FTE_MCP23S08_PTR _pHead = NULL;
static FTE_UINT32          _nMCP23S08s = 0;


FTE_MCP23S08_PTR FTE_MCP23S08_get
(
    FTE_UINT32  nID
)
{
    FTE_MCP23S08_PTR    pMCP23S08;
    
    pMCP23S08 = _pHead;
    while(pMCP23S08 != NULL)
    {
        if (pMCP23S08->pConfig->nID == (nID & (~0x7)))
        {
            return  pMCP23S08;
        }
        
        pMCP23S08 = pMCP23S08->pNext;
    }
    
    return  NULL;
}

FTE_MCP23S08_PTR FTE_MCP23S08_get_first(void)
{
    return  _pHead;
}

FTE_MCP23S08_PTR FTE_MCP23S08_getNext
(
    FTE_MCP23S08_PTR    pObj
)
{
    return  pObj->pNext;
}

FTE_RET   FTE_MCP23S08_create
(
    FTE_MCP23S08_CONFIG_PTR     pConfig
)
{
    FTE_MCP23S08_PTR    pMCP23S08;
    
    pMCP23S08 = (FTE_MCP23S08_PTR)FTE_MEM_allocZero(sizeof(FTE_MCP23S08));
    if (pMCP23S08 == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }

    pMCP23S08->pNext    = _pHead;
    pMCP23S08->pConfig  = pConfig;
    memset(pMCP23S08->pPolaries, FTE_MCP23S08_INT_CHANGE, sizeof(pMCP23S08->pPolaries));
        
    _pHead = pMCP23S08;
    _nMCP23S08s++;
    
    return  MQX_OK;
}

FTE_RET   FTE_MCP23S08_attach
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nParent
)
{
    FTE_SPI_PTR             pSPI;
    FTE_MCP23S08_GPIO_PTR   pMCP23S08_GPIO;
    FTE_LWGPIO_PTR pLWGPIO, pLWGPIO_Reset;
        
    ASSERT(pMCP23S08 != NULL);
    
    pMCP23S08_GPIO = FTE_MCP23S08_GPIO_get(nParent);
    if (pMCP23S08_GPIO == NULL)
    {
        return  MQX_ERROR;
    }
    
    if (pMCP23S08->pParent[pMCP23S08_GPIO->pConfig->nBit] != 0)
    {
        return  MQX_ERROR;
    }
    
    if (pMCP23S08->nEnableMask == 0)
    {
        pSPI = FTE_SPI_get(pMCP23S08->pConfig->xSPI);
        if (pSPI == NULL)
        {
            goto error1;
        }
    
        pLWGPIO = FTE_LWGPIO_get(pMCP23S08->pConfig->xGPIOInt);
        if (pLWGPIO == NULL)
        {
            goto error1;
        }
        
        pLWGPIO_Reset = FTE_LWGPIO_get(pMCP23S08->pConfig->xGPIOReset);
        if (pLWGPIO_Reset == NULL)
        {
            goto error1;
        }
        
        if (FTE_LWGPIO_attach(pLWGPIO, pMCP23S08->pConfig->nID) != MQX_OK)
        {
            goto error2;
        }

#if FTE_MCP23S08_INT_MODE        
        FTE_LWGPIO_setISR(pLWGPIO, _FTE_MCP23S08_isr, pMCP23S08);
        FTE_LWGPIO_INT_setPolarity(pLWGPIO, FTE_LWGPIO_INT_RISING);
#endif
        
        if (FTE_LWGPIO_attach(pLWGPIO_Reset, pMCP23S08->pConfig->nID) != MQX_OK)
        {
            goto error2;
        }
        
        if (FTE_SPI_attach(pSPI, pMCP23S08->pConfig->nID) != MQX_OK)
        {
            goto error2;
        }

        pMCP23S08->pSPI = pSPI;
        pMCP23S08->pLWGPIO = pLWGPIO;
        pMCP23S08->pLWGPIO_Reset = pLWGPIO_Reset;        
        
        _FTE_MCP23S08_init(pMCP23S08);
        _FTE_MCP23S08_POLL_start(pMCP23S08);
    }

    pMCP23S08->pParent[pMCP23S08_GPIO->pConfig->nBit] = pMCP23S08_GPIO->pConfig->nID;
    pMCP23S08->nEnableMask |= (1 << pMCP23S08_GPIO->pConfig->nBit);
    return  MQX_OK;
    
error2:
    
    if (pSPI != NULL)
    {
        FTE_SPI_detach(pSPI);
    }
    
    if (pLWGPIO != NULL)
    {
        FTE_LWGPIO_detach(pLWGPIO);
    }
    
    if (pLWGPIO_Reset != NULL)
    {
        FTE_LWGPIO_detach(pLWGPIO_Reset);
    }

error1:
    
    return  MQX_ERROR;
}

FTE_RET   FTE_MCP23S08_detach
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nParentID
)
{
    FTE_MCP23S08_GPIO_PTR   pMCP23S08_GPIO;
    
    ASSERT(pMCP23S08 != NULL);
    
    pMCP23S08_GPIO = FTE_MCP23S08_GPIO_get(nParentID);
    if (pMCP23S08_GPIO == NULL)
    {
        return  MQX_ERROR;
    }
        
    pMCP23S08->pParent[pMCP23S08_GPIO->pConfig->nBit] = 0;
    pMCP23S08->nEnableMask &= ~(1 << pMCP23S08_GPIO->pConfig->nBit);
    
    if (pMCP23S08->nEnableMask == 0)
    {
        _FTE_MCP23S08_POLL_stop(pMCP23S08);
        FTE_SPI_detach(pMCP23S08->pSPI);
        pMCP23S08->pSPI = NULL;
        FTE_LWGPIO_detach(pMCP23S08->pLWGPIO);
        pMCP23S08->pLWGPIO = NULL;
        FTE_LWGPIO_detach(pMCP23S08->pLWGPIO_Reset);
        pMCP23S08->pLWGPIO_Reset = NULL;
    }    

    return  MQX_OK;
}


FTE_RET   FTE_MCP23S08_setReg
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nRegID, 
    FTE_UINT32          nValue
)
{
    ASSERT(pMCP23S08 != NULL);
    
    if  (nRegID >= FTE_MCP23S08_REG_COUNT)
    {
        goto error;
    }
    
    if (_FTE_MCP23S08_setReg(pMCP23S08, nRegID, nValue) != MQX_OK)
    {
        goto error;
    }
    pMCP23S08->pRegs[nRegID] = nValue;
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

FTE_RET   FTE_MCP23S08_regGet
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nRegID, 
    FTE_UINT32_PTR      pValue
)
{
    ASSERT(pMCP23S08 != NULL);

    if  (nRegID >= FTE_MCP23S08_REG_COUNT)
    {
        goto error;
    }

    *pValue = pMCP23S08->pRegs[nRegID];
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

FTE_RET   FTE_MCP23S08_setRegBit
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nReg, 
    FTE_UINT32          nBit, 
    FTE_UINT32          nValue
)
{
    FTE_UINT32 nRegValue;
    
    if (pMCP23S08 == NULL)
    {
        goto error;
    }
    
    if (FTE_MCP23S08_regGet(pMCP23S08, nReg, &nRegValue) != MQX_OK)
    {
        goto error;
    }
    
    if (nValue)
    {
        nRegValue = nRegValue | (1 << nBit);
    }
    else
    {
        nRegValue = nRegValue & ~(1 << nBit);
    }
    
    if (FTE_MCP23S08_setReg(pMCP23S08, nReg, nRegValue) != MQX_OK)
    {
        goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

FTE_RET   FTE_MCP23S08_getRegBit
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nReg, 
    FTE_UINT32          nBit, 
    FTE_UINT32_PTR      pValue
)
{
    FTE_UINT32 nRegValue;
    
    if (pMCP23S08 == NULL)
    {
        goto error;
    }

    if (FTE_MCP23S08_regGet(pMCP23S08, nReg, &nRegValue) != MQX_OK)
    {
        goto error;
    }

    *pValue = (nRegValue >> nBit) & 0x01;
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

FTE_RET   FTE_MCP23S08_INT_enableRegBit
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nBit, 
    FTE_BOOL            bEnable
)
{
    if (pMCP23S08 == NULL)
    {
        return  MQX_ERROR;
    }

    if (((pMCP23S08->nINTMask >> nBit) & 0x01) != bEnable)
    {
        if (bEnable)
        {        
            pMCP23S08->nINTMask |= (1 << nBit);
        }
        else
        {
            pMCP23S08->nINTMask &= ~(1 << nBit);
        }
    } 
    
    return  MQX_OK;
}

FTE_RET   FTE_MCP23S08_INT_initRegBit
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nBit, 
    void                (*func)(FTE_VOID_PTR ), 
    FTE_VOID_PTR        pParams
)
{
    ASSERT((pMCP23S08 != NULL) && (nBit < 8));
    
    pMCP23S08->f_isr[nBit] = func;
    pMCP23S08->pParams[nBit] = pParams;

    return  MQX_OK;
}


FTE_RET   FTE_MCP23S08_INT_setPolarityRegBit
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nBit, 
    FTE_UINT32          nPolarity
)
{
    ASSERT((pMCP23S08 != NULL) && (nBit < 8));
    
    pMCP23S08->pPolaries[nBit] = nPolarity;
    
    return  MQX_OK;
}


FTE_INT32  FTE_MCP23S08_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (bPrintUsage)
    { 
        goto    error;
    }
    
    switch(nArgc)
    {
    case    1:
        {
            FTE_UINT8  pRegValaes[FTE_MCP23S08_REG_COUNT];
            FTE_MCP23S08_PTR pMCP23S08 = FTE_MCP23S08_get_first();
            if (pMCP23S08 == NULL)
            {
                printf("MCP23S08 not exist!\n");
                goto error;
            }
            
            _FTE_MCP23S08_getRegs(pMCP23S08, 0, pRegValaes, sizeof(pRegValaes));
            
            for(int i = 0 ; i < sizeof(pRegValaes) ;i++)
            {
                printf("%02x : %02x\n", i, pRegValaes[i]);
            }
        }
        break;
        
          
    case    5:
    case    6:
        {
            FTE_UINT32     nMCP23S08, nBit;
            
            if ((!Shell_parse_number(pArgv[1], &nMCP23S08)) ||
                (strcmp(pArgv[2], "gpio") != 0) ||
                    (!Shell_parse_number(pArgv[3], &nBit)))
            {
                goto error;
            }

#if 0
            FTE_UINT32 nID = FTE_DEV_TYPE_MCP23S08 | ((0x0F & nMCP23S08) << 4) | (0x0F & nBit);
            if (strcmp(pArgv[4], "dir") == 0)
            {
                FTE_MCP23S08_PTR pObj;
                
                pObj = FTE_MCP23S08_get(nID);
                if (pObj == 0)
                {
                    goto error;
                }
                
                if (nArgc == 6)
                {
                    if (strcmp(pArgv[5], "out") == 0)
                    {
                        FTE_MCP23S08_gpio_dir_set(pObj, (nID & 0x07), FTE_GPIO_DIR_OUTPUT);
                    }
                    else if(strcmp(pArgv[5], "in") == 0)
                    {
                        FTE_MCP23S08_gpio_dir_set(pObj, (nID & 0x07), FTE_GPIO_DIR_INPUT);
                    }
                    else
                    {
                        goto error;
                    }
                }
                else
                {
                    FTE_GPIO_DIR nValue;
                    if (FTE_MCP23S08_gpio_get_direction(pObj, (nID & 0x07), &nValue) != MQX_OK)
                    {
                        goto error;
                    }
                    
                    printf("%08x : Direction %s\n", nID, (nValue == FTE_GPIO_DIR_INPUT)?"INPUT":"OUTPUT");
                }
            }
            else 
            if (strcmp(pArgv[4], "get") == 0)
            {
                FTE_MCP23S08_PTR    pObj;
                FTE_UINT32 nValue;
                
                pObj = FTE_MCP23S08_get(nID);
                if (pObj == NULL)
                {
                    goto error;
                }
                
                if (FTE_MCP23S08_gpio_get_value(pObj, (nID & 0x07), &nValue) == MQX_OK)
                {
                    printf("%08x : %s\n", nID, (nValue)?"ON":"OFF");
                }
                else
                {
                    goto error;
                }
            }
            else if (strcmp(pArgv[4], "set") == 0)
            {
                FTE_MCP23S08_PTR    pObj;
                
                pObj = FTE_MCP23S08_get(nID);
                if (pObj == NULL)
                {
                    goto error;
                }
                
                if (strcmp(pArgv[5], "on") == 0)
                {
                    FTE_MCP23S08_gpio_set_value(pObj, (nID & 0x07), TRUE);
                }
                else if(strcmp(pArgv[5], "off") == 0)
                {
                    FTE_MCP23S08_gpio_set_value(pObj, (nID & 0x07), FALSE);
                }
                else
                {
                    goto error;
                }
            }
            else
            {
                goto error;
            }
#endif
        }
    }

    goto success;
error:    
    xRet = SHELL_EXIT_ERROR;
    
success:        
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [read | write] [<regid>] [<value>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [read | write] [<regid>] [<value>]\n", pArgv[0]);
            printf("        read - read register\n");
            printf("        write - write register\n");
            printf("        regid - reigster index\n");
            printf("        value - value of register\n");
        }
    }
    
    return   xRet;
}
 
/******************************************************************************
 * Static Functions
 ******************************************************************************/

static 
FTE_RET    _FTE_MCP23S08_init
(
    FTE_MCP23S08_PTR    pMCP23S08
)
{
    ASSERT(pMCP23S08 != NULL);
    
    _FTE_MCP23S08_reset(pMCP23S08);
    
    if (_FTE_MCP23S08_getRegs(pMCP23S08, FTE_MCP23S08_REG_IODIR, pMCP23S08->pRegs, sizeof(pMCP23S08->pRegs)) != MQX_OK)
    {
        goto error;
    }
    
    if (FTE_MCP23S08_setReg(pMCP23S08, FTE_MCP23S08_REG_IOCON, 0x02) != MQX_OK)
    {
        goto error;
    }
    
    if (FTE_MCP23S08_setReg(pMCP23S08, FTE_MCP23S08_REG_INTCON, 0xFF) != MQX_OK)
    {
        goto error;
    }

    FTE_MCP23S08_setReg(pMCP23S08, FTE_MCP23S08_REG_DEFVAL, pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO]);
    FTE_MCP23S08_setReg(pMCP23S08, FTE_MCP23S08_REG_GPINTEN, pMCP23S08->pRegs[FTE_MCP23S08_REG_IODIR]);
    
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

#if 0
static 
FTE_RET   _FTE_MCP23S08_regGet
(
    FTE_MCP23S08_PTR    pDev, 
    FTE_UINT32          nReg, 
    FTE_UINT32_PTR      pValue
)
{
    FTE_UINT8  nValue;
    FTE_UINT8  pFrame[2];
    
    ASSERT(pDev != NULL && pValue != NULL);
    
    pFrame[0] = FTE_MCP23S08_BASE_ADDR | 
                ((pDev->pConfig->xSlaveAddr & FTE_MCP23S08_SLAVE_ADDR_MASK) << FTE_MCP23S08_SLAVE_ADDR_SHIFT) |
                FTE_MCP23S08_SPI_READ;
    pFrame[1] = (nReg & FTE_MCP23S08_REG_MASK);
    
    if (MQX_OK != FTE_SPI_read(pDev->pSPI, pFrame, sizeof(pFrame), &nValue, 1))
    {
        return  MQX_ERROR;
    }
    
    *pValue = nValue;
    
    return  MQX_OK;
}
#endif

static 
FTE_RET   _FTE_MCP23S08_getRegs
(
    FTE_MCP23S08_PTR    pDev, 
    FTE_UINT32          nReg, 
    FTE_UINT8_PTR       pData, 
    FTE_UINT32          nData
)
{
    FTE_UINT8  pFrame[2];
    
    ASSERT(pDev != NULL && pData != NULL);
    
    pFrame[0] = FTE_MCP23S08_BASE_ADDR | 
                ((pDev->pConfig->xSlaveAddr & FTE_MCP23S08_SLAVE_ADDR_MASK) << FTE_MCP23S08_SLAVE_ADDR_SHIFT) |
                FTE_MCP23S08_SPI_READ;
    pFrame[1] = (nReg & FTE_MCP23S08_REG_MASK);
    
    if (MQX_OK != FTE_SPI_read(pDev->pSPI, pFrame, sizeof(pFrame), pData, nData))
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

static 
FTE_RET   _FTE_MCP23S08_setReg
(
    FTE_MCP23S08_PTR    pDev, 
    FTE_UINT32          nReg, 
    FTE_UINT32          nValue
)
{
    FTE_UINT8  pFrame[3];

    ASSERT(pDev != NULL);
    
    pFrame[0] = FTE_MCP23S08_BASE_ADDR | 
                ((pDev->pConfig->xSlaveAddr & FTE_MCP23S08_SLAVE_ADDR_MASK) << FTE_MCP23S08_SLAVE_ADDR_SHIFT) |
                FTE_MCP23S08_SPI_WRITE;
    pFrame[1] = (nReg & FTE_MCP23S08_REG_MASK);
    pFrame[2] = nValue;
    if (MQX_OK != FTE_SPI_write(pDev->pSPI, pFrame, 3, NULL, 0))
    {
        goto error;
    }

    if (MQX_OK != FTE_SPI_write(pDev->pSPI, pFrame, 3, NULL, 0))
    {
        goto error;
    }
    
    return  MQX_OK;
error:
    return  MQX_ERROR;
}



#if 0
static 
FTE_RET   _FTE_MCP23S08_setRegs
(
    FTE_MCP23S08_PTR    pMCP23S08, 
    FTE_UINT32          nReg, 
    FTE_UINT8_PTR       pData, 
    FTE_UINT32          nData
)
{
    FTE_UINT8  pFrame[3];

    ASSERT(pMCP23S08 != NULL);
    
    pFrame[0] = FTE_MCP23S08_BASE_ADDR | 
                ((pMCP23S08->pConfig->xSlaveAddr & FTE_MCP23S08_SLAVE_ADDR_MASK) << FTE_MCP23S08_SLAVE_ADDR_SHIFT) |
                FTE_MCP23S08_SPI_WRITE;
    pFrame[1] = (nReg & FTE_MCP23S08_REG_MASK);
    if (MQX_OK != FTE_SPI_write(pMCP23S08->pSPI, pFrame, 3, pData, nData))
    {
        goto error;
    }

    if (MQX_OK != FTE_SPI_write(pMCP23S08->pSPI, pFrame, 3, pData, nData))
    {
        goto error;
    }
    
    return  MQX_OK;
error:
    return  MQX_ERROR;
}

void _FTE_MCP23S08_isr
(
    FTE_VOID_PTR    pParam
)
{
#if FTE_MCP23S08_INT_MODE
    FTE_MCP23S08_PTR    pMCP23S08 = (FTE_MCP23S08_PTR)pParam;

    FTE_LWGPIO_int_set_enable(pMCP23S08->pLWGPIO, FALSE);    
#endif
}
#endif

FTE_RET        _FTE_MCP23S08_INT_enable(FTE_MCP23S08_PTR pMCP23S08)
{
#if FTE_MCP23S08_INT_MODE
    FTE_MCP23S08_setReg(pMCP23S08, FTE_MCP23S08_REG_GPINTEN, pMCP23S08->nINTMask);
    
    FTE_LWGPIO_int_init(pMCP23S08->pLWGPIO, 3, 0, TRUE);
    FTE_LWGPIO_int_set_enable(pMCP23S08->pLWGPIO, TRUE);
#endif
    
    return  MQX_OK;
}

FTE_RET   _FTE_MCP23S08_INT_disable
(
    FTE_MCP23S08_PTR    pMCP23S08
)
{
#if FTE_MCP23S08_INT_MODE
    FTE_LWGPIO_int_set_enable(pMCP23S08->pLWGPIO, FALSE);
    FTE_LWGPIO_int_init(pMCP23S08->pLWGPIO, 3, 0, FALSE);
#endif    
    return  MQX_OK;
}

FTE_RET   _FTE_MCP23S08_POLL_start
(
    FTE_MCP23S08_PTR    pMCP23S08
)
{
    pMCP23S08->nFlags |= FTE_MCP23S08_FLAG_RUN;
    
    if (_FTE_MCP23S08_TIMER_start(pMCP23S08) != MQX_OK)
    {
        printf("MCP23S08 Timer failed\n");
        return  MQX_ERROR;
    }
    _FTE_MCP23S08_INT_enable(pMCP23S08);
    
    return  MQX_OK;
}

FTE_RET   _FTE_MCP23S08_POLL_stop
(
    FTE_MCP23S08_PTR    pMCP23S08
)
{
    _FTE_MCP23S08_INT_disable(pMCP23S08);
    _timer_cancel(pMCP23S08->xTimerID);
    
    pMCP23S08->nFlags &= ~FTE_MCP23S08_FLAG_RUN;
    
    return  MQX_OK;
}

FTE_RET   _FTE_MCP23S08_TIMER_start
(
    FTE_MCP23S08_PTR    pMCP23S08
)
{
    MQX_TICK_STRUCT     xTicks, xDTicks;            
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, FTE_MCP23S08_UPDATE_TIME);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    pMCP23S08->xTimerID = _timer_start_periodic_at_ticks(_FTE_MCP23S08_TIMER_done, pMCP23S08, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);    
    if (pMCP23S08->xTimerID != 0)
    {
        return MQX_OK;
    }
    else
    {
        return  MQX_ERROR;
    }
}

void _FTE_MCP23S08_TIMER_done
(
    _timer_id   xTimerID, 
    pointer     pData, 
    MQX_TICK_STRUCT_PTR     pTick
)
{
    FTE_UINT8              pRegValues[FTE_MCP23S08_REG_COUNT];
    FTE_MCP23S08_PTR    pMCP23S08 = (FTE_MCP23S08_PTR)pData;

    if(_FTE_MCP23S08_getRegs(pMCP23S08, FTE_MCP23S08_REG_IODIR, pRegValues, sizeof(pRegValues)) != MQX_OK)
    {
        return;
    }

    if (pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO] != pRegValues[FTE_MCP23S08_REG_GPIO])
    {
        FTE_UINT8  nINTFlags = (pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO] ^ pRegValues[FTE_MCP23S08_REG_GPIO]) & 
                            pMCP23S08->nEnableMask & pMCP23S08->pRegs[FTE_MCP23S08_REG_GPINTEN];
        
        pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO] = pRegValues[FTE_MCP23S08_REG_GPIO];
        FTE_MCP23S08_setReg(pMCP23S08, FTE_MCP23S08_REG_DEFVAL, pRegValues[FTE_MCP23S08_REG_GPIO]);

        for(int i = 0 ; i < 8 ; i++)
        {
            FTE_BOOL bOccured = FALSE;
            
            if (nINTFlags & (1 << i))
            {
                switch(pMCP23S08->pPolaries[i])
                {
                case    FTE_MCP23S08_INT_RISING:
                case    FTE_MCP23S08_INT_HIGH:
                    {
                        bOccured = pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO] & (1 << i);
                    }
                    break;
                    
                case    FTE_MCP23S08_INT_FALLING:
                case    FTE_MCP23S08_INT_LOW:
                    {
                        bOccured = (~pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO]) & (1 << i);
                    }
                    break;
                    
                case    FTE_MCP23S08_INT_CHANGE:
                    {
                        bOccured = TRUE;
                    }
                    break;
                }
                
                if (bOccured)
                {                        
                    pMCP23S08->f_isr[i](pMCP23S08->pParams[i]);
                }
            }
        }
    }
    
    _FTE_MCP23S08_INT_enable(pMCP23S08);
}

FTE_RET   _FTE_MCP23S08_reset
(
    FTE_MCP23S08_PTR    pDev
)
{
    FTE_LWGPIO_setValue(pDev->pLWGPIO_Reset, TRUE);
    //_time_delay(FTE_MCP23S08_RESET_DELAY);
    FTE_LWGPIO_setValue(pDev->pLWGPIO_Reset, FALSE);
    //_time_delay(FTE_MCP23S08_RESET_DELAY);
    
    return  MQX_OK;
}

#endif
