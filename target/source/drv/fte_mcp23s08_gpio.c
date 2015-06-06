#include "fte_target.h"
#include "fte_mcp23s08_gpio.h"

#if FTE_MCP23S08_SUPPORTED

static FTE_MCP23S08_GPIO_PTR    _pHead = NULL;
static uint_32                  _nObjects = 0;
 
_mqx_uint   _fte_mcp23s08_set_reg_value(FTE_MCP23S08_GPIO_PTR pGPIO, uint_32 nReg, uint_32 nValue);
_mqx_uint   _fte_mcp23s08_get_reg_value(FTE_MCP23S08_GPIO_PTR pGPIO, uint_32 nReg, uint_32 *pValue);
void        _fte_mcp23s08_gpio_isr(void *pParams);

_mqx_uint   fte_mcp23s08_gpio_create(FTE_MCP23S08_GPIO_CONFIG_PTR pConfig)
{
    FTE_MCP23S08_GPIO_PTR    pGPIO;
    
    pGPIO = (FTE_MCP23S08_GPIO_PTR)FTE_MEM_allocZero(sizeof(FTE_MCP23S08_GPIO));
    if (pGPIO == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }

    pGPIO->pNext        = _pHead;
    pGPIO->pConfig      = pConfig;
    pGPIO->nPolarity    = FTE_MCP23S08_INT_RISING;
    pGPIO->bINTFlag     = FALSE;
    _pHead = pGPIO;
    _nObjects++;
    
    return  MQX_OK;
}

_mqx_uint   fte_mcp23s08_gpio_attach(FTE_MCP23S08_GPIO_PTR pGPIO, uint_32 nParent)
{
    if (pGPIO == NULL)
    {
        goto error;
    }
    
    pGPIO->pMCP23S08 = fte_mcp23s08_get(pGPIO->pConfig->nDrvID);
    if (pGPIO->pMCP23S08 == NULL)
    {
        goto error;
    }
    
    if (fte_mcp23s08_attach(pGPIO->pMCP23S08, pGPIO->pConfig->nID) != MQX_OK)
    {
        goto error;
    }
    
    if (pGPIO->pConfig->nDir == FTE_GPIO_DIR_INPUT)
    {
        uint_32 nValue;
        fte_mcp23s08_reg_bit_get(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPIO, pGPIO->pConfig->nBit, &nValue);
        
        pGPIO->nValue = nValue;
        
        fte_mcp23s08_reg_bit_set(pGPIO->pMCP23S08, FTE_MCP23S08_REG_DEFVAL, pGPIO->pConfig->nBit, nValue);
        fte_mcp23s08_reg_bit_int_enable(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, TRUE);        
    }
    
    pGPIO->nParent = nParent;
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

_mqx_uint   fte_mcp23s08_gpio_detach(FTE_MCP23S08_GPIO_PTR pGPIO)
{
    if (pGPIO == NULL)
    {
        goto error;
    }

    if (pGPIO->pConfig->nDir == FTE_GPIO_DIR_INPUT)
    {
        fte_mcp23s08_reg_bit_int_enable(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, FALSE);        
    }    
    
    fte_mcp23s08_detach(pGPIO->pMCP23S08, pGPIO->pConfig->nID);
    
    pGPIO->nParent = 0;
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

boolean     fte_mcp23s08_gpio_exist(uint_32 nID)
{
    return  fte_mcp23s08_gpio_get(nID) != NULL;
}

FTE_MCP23S08_GPIO_PTR   fte_mcp23s08_gpio_get(uint_32 nID)
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


_mqx_uint   fte_mcp23s08_gpio_value_set(FTE_MCP23S08_GPIO_PTR pGPIO, uint_32 nValue)
{
    ASSERT(pGPIO != NULL);
    
    if (fte_mcp23s08_reg_bit_set(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPIO, pGPIO->pConfig->nBit, nValue) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pGPIO->nValue = nValue;
    
    return  MQX_OK;
}

_mqx_uint   fte_mcp23s08_gpio_value_get(FTE_MCP23S08_GPIO_PTR pGPIO, uint_32 *pValue)
{
    ASSERT(pGPIO != NULL);

    *pValue = pGPIO->nValue;
    
    return  MQX_OK;
}

_mqx_uint   fte_mcp23s08_gpio_dir_set(FTE_MCP23S08_GPIO_PTR pGPIO, FTE_GPIO_DIR nValue)
{
    if (pGPIO == NULL)
    {
        goto error;
    }
    
    switch(nValue)
    {
    case    FTE_GPIO_DIR_INPUT: 
        fte_mcp23s08_reg_bit_set(pGPIO->pMCP23S08, FTE_MCP23S08_REG_IODIR, pGPIO->pConfig->nBit, 1); 
        fte_mcp23s08_reg_bit_set(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPINTEN, pGPIO->pConfig->nBit, 1); 
        break;
        
    case    FTE_GPIO_DIR_OUTPUT:
        fte_mcp23s08_reg_bit_set(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPINTEN, pGPIO->pConfig->nBit, 0); 
        fte_mcp23s08_reg_bit_set(pGPIO->pMCP23S08, FTE_MCP23S08_REG_IODIR, pGPIO->pConfig->nBit, 0); 
        break;
        
    default: 
        goto error;
    }

    return  MQX_OK;

error:    
    return  MQX_ERROR;
}

_mqx_uint   fte_mcp23s08_gpio_dir_get(FTE_MCP23S08_GPIO_PTR pGPIO, FTE_GPIO_DIR_PTR pValue)
{
    uint_32 nValue;
    
    if (pGPIO == NULL)
    {
        goto error;
    }

    if (fte_mcp23s08_reg_bit_get(pGPIO->pMCP23S08, FTE_MCP23S08_REG_IODIR, pGPIO->pConfig->nBit, &nValue) != MQX_OK)
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
    
    return  MQX_OK;
 
error:
    return  MQX_ERROR;
}

_mqx_uint   fte_mcp23s08_gpio_int_polarity_set(FTE_MCP23S08_GPIO_PTR pGPIO, uint_32 nPolarity)
{
    if (pGPIO == NULL)
    {
        return  MQX_ERROR;
    }

    pGPIO->nPolarity = nPolarity;    
    fte_mcp23s08_reg_bit_int_polarity_set(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, nPolarity);

    return  MQX_OK;
}


_mqx_uint   fte_mcp23s08_gpio_isr_set(FTE_MCP23S08_GPIO_PTR pGPIO, void (*fISR)(void *), void *pParams)
{
    ASSERT(pGPIO != NULL);

    pGPIO->fISR     = fISR;
    pGPIO->pParams  = pParams;    
    
    if (fte_mcp23s08_reg_bit_int_init(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, _fte_mcp23s08_gpio_isr, pGPIO) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

_mqx_uint   fte_mcp23s08_gpio_int_enable(FTE_MCP23S08_GPIO_PTR pGPIO, boolean bEnable)
{
    ASSERT(pGPIO != NULL);

    if (fte_mcp23s08_reg_bit_int_enable(pGPIO->pMCP23S08, pGPIO->pConfig->nBit, bEnable) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

_mqx_uint   fte_mcp23s08_gpio_int_flag_get(FTE_MCP23S08_GPIO_PTR pGPIO, boolean *pFlag)
{
    ASSERT((pGPIO != 0) && (pFlag != NULL));
    
    *pFlag = pGPIO->bINTFlag;
    
    return  MQX_OK;
}

_mqx_uint   fte_mcp23s08_gpio_int_flag_clr(FTE_MCP23S08_GPIO_PTR pGPIO)
{
    ASSERT(pGPIO != 0);
    
    pGPIO->bINTFlag = FALSE;
    
    return  MQX_OK;
}

void        _fte_mcp23s08_gpio_isr(void *pParams)
{
    uint_32 nValue = 0;
    
    FTE_MCP23S08_GPIO_PTR pGPIO = (FTE_MCP23S08_GPIO_PTR)pParams;
    
    ASSERT(pGPIO != NULL);

    nValue = (pGPIO->pMCP23S08->pRegs[FTE_MCP23S08_REG_GPIO] >> pGPIO->pConfig->nBit) & 0x01;
    //if (fte_mcp23s08_reg_bit_get(pGPIO->pMCP23S08, FTE_MCP23S08_REG_GPIO, pGPIO->pConfig->nBit, &nValue) == MQX_OK)
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