#include "fte_target.h"
#include "FTE_MCP23S08_gpio.h"

#if FTE_MCP23S08_SUPPORTED

static FTE_MCP23S08_GPIO_PTR    _pHead = NULL;
static FTE_UINT32               _nObjects = 0;
 
void    _FTE_MCP23S08_GPIO_isr(FTE_VOID_PTR pParams);

FTE_RET   FTE_MCP23S08_GPIO_create
(
    FTE_MCP23S08_GPIO_CONFIG_PTR    pConfig
)
{
    FTE_MCP23S08_GPIO_PTR    pGPIO;
    
    pGPIO = (FTE_MCP23S08_GPIO_PTR)FTE_MEM_allocZero(sizeof(FTE_MCP23S08_GPIO));
    if (pGPIO == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }

    pGPIO->pNext        = _pHead;
    pGPIO->pConfig      = pConfig;
    pGPIO->nPolarity    = FTE_MCP23S08_INT_RISING;
    pGPIO->bINTFlag     = FALSE;
    _pHead = pGPIO;
    _nObjects++;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MCP23S08_GPIO_attach
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_UINT32              nParent
)
{
    ASSERT(pGPIO != NULL);
    
    pGPIO->pMCP23S08 = FTE_MCP23S08_get(pGPIO->pConfig->nDrvID);
    if (pGPIO->pMCP23S08 == NULL)
    {
        goto error;
    }
    
    if (FTE_MCP23S08_attach(pGPIO->pMCP23S08, pGPIO->pConfig->nID) != FTE_RET_OK)
    {
        goto error;
    }
    
    if (pGPIO->pConfig->nDir == FTE_GPIO_DIR_INPUT)
    {
        FTE_UINT32 nValue;
        FTE_MCP23S08_getRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPIO, pGPIO->pConfig->nBit, &nValue);
        
        pGPIO->nValue = nValue;
        
        FTE_MCP23S08_setRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_DEFVAL, pGPIO->pConfig->nBit, nValue);
        FTE_MCP23S08_INT_enableRegBit(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, TRUE);        
    }
    
    pGPIO->nParent = nParent;
    
    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_MCP23S08_GPIO_detach
(
    FTE_MCP23S08_GPIO_PTR   pGPIO
)
{
    ASSERT(pGPIO != NULL);

    if (pGPIO->pConfig->nDir == FTE_GPIO_DIR_INPUT)
    {
        FTE_MCP23S08_INT_enableRegBit(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, FALSE);        
    }    
    
    FTE_MCP23S08_detach(pGPIO->pMCP23S08, pGPIO->pConfig->nID);
    
    pGPIO->nParent = 0;
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_BOOL    FTE_MCP23S08_GPIO_exist
(
    FTE_UINT32  nID
)
{
    return  FTE_MCP23S08_GPIO_get(nID) != NULL;
}

FTE_MCP23S08_GPIO_PTR   FTE_MCP23S08_GPIO_get
(
    FTE_UINT32  nID
)
{
    FTE_MCP23S08_GPIO_PTR   pGPIO;
    
    pGPIO = _pHead;
    while(pGPIO != NULL)
    {
        if (pGPIO->pConfig->nID == nID)
        {
            return  pGPIO;
        }
        pGPIO = pGPIO->pNext;
    }
    
    return  NULL;
}


FTE_RET   FTE_MCP23S08_GPIO_setValue
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_UINT32              nValue
)
{
    ASSERT(pGPIO != NULL);
    
    if (FTE_MCP23S08_setRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPIO, pGPIO->pConfig->nBit, nValue) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    pGPIO->nValue = nValue;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MCP23S08_GPIO_getValue
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_UINT32_PTR          pValue
)
{
    ASSERT(pGPIO != NULL);

    *pValue = pGPIO->nValue;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MCP23S08_GPIO_setDIR
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_GPIO_DIR            nValue
)
{
    ASSERT(pGPIO != NULL);
    
    switch(nValue)
    {
    case    FTE_GPIO_DIR_INPUT: 
        FTE_MCP23S08_setRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_IODIR, pGPIO->pConfig->nBit, 1); 
        FTE_MCP23S08_setRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPINTEN, pGPIO->pConfig->nBit, 1); 
        break;
        
    case    FTE_GPIO_DIR_OUTPUT:
        FTE_MCP23S08_setRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPINTEN, pGPIO->pConfig->nBit, 0); 
        FTE_MCP23S08_setRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_IODIR, pGPIO->pConfig->nBit, 0); 
        break;
        
    default: 
        goto error;
    }

    return  FTE_RET_OK;

error:    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_MCP23S08_GPIO_getDIR
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_GPIO_DIR_PTR        pValue
)
{
    FTE_UINT32 nValue;
    
    ASSERT(pGPIO != NULL);

    if (FTE_MCP23S08_getRegBit(pGPIO->pMCP23S08, FTE_MCP23S08_REG_IODIR, pGPIO->pConfig->nBit, &nValue) != FTE_RET_OK)
    {
        goto error;
    }
    
    if (nValue)
    {
        *pValue = FTE_GPIO_DIR_INPUT;
    }
    else
    {
        *pValue = FTE_GPIO_DIR_OUTPUT;
    }
    
    return  FTE_RET_OK;
 
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_MCP23S08_GPIO_INT_setPolarity
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_UINT32              nPolarity
)
{
    ASSERT(pGPIO != NULL);

    pGPIO->nPolarity = nPolarity;    
    FTE_MCP23S08_INT_setPolarityRegBit(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, nPolarity);

    return  FTE_RET_OK;
}


FTE_RET   FTE_MCP23S08_GPIO_ISR_set
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    void (*fISR)(FTE_VOID_PTR ), 
    FTE_VOID_PTR            pParams
)
{
    ASSERT(pGPIO != NULL);

    pGPIO->fISR     = fISR;
    pGPIO->pParams  = pParams;    
    
    if (FTE_MCP23S08_INT_initRegBit(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, _FTE_MCP23S08_GPIO_isr, pGPIO) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MCP23S08_GPIO_INT_enable
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_BOOL                bEnable
)
{
    ASSERT(pGPIO != NULL);

    if (FTE_MCP23S08_INT_enableRegBit(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, bEnable) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MCP23S08_GPIO_INT_getFlag
(
    FTE_MCP23S08_GPIO_PTR   pGPIO, 
    FTE_BOOL_PTR            pFlag
)
{
    ASSERT((pGPIO != 0) && (pFlag != NULL));
    
    *pFlag = pGPIO->bINTFlag;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MCP23S08_GPIO_INT_clrFlag
(
    FTE_MCP23S08_GPIO_PTR   pGPIO
)
{
    ASSERT(pGPIO != 0);
    
    pGPIO->bINTFlag = FALSE;
    
    return  FTE_RET_OK;
}

void    _FTE_MCP23S08_GPIO_isr
(
    FTE_VOID_PTR    pParams
)
{
    FTE_UINT32 nValue = 0;
    
    FTE_MCP23S08_GPIO_PTR pGPIO = (FTE_MCP23S08_GPIO_PTR)pParams;
    
    ASSERT(pGPIO != NULL);

    nValue = (pGPIO->pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO] >> pGPIO->pConfig->nBit) & 0x01;
    //if (FTE_MCP23S08_reg_bit_get(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPIO, pGPIO->pConfig->nBit, &nValue) == FTE_RET_OK)
    {
        pGPIO->nValue = nValue;
        pGPIO->bINTFlag = TRUE;    
        
        if (pGPIO->fISR != NULL)
        {
            pGPIO->fISR(pGPIO->pParams);    
        }
    }
}
#endif