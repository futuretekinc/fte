#ifndef __FTE_MCP23S08_GPIO_H__
#define __FTE_MCP23S08_GPIO_H__

typedef struct _FTE_MCP23S08_GPIO_CONFIG_STRUCT
{
    uint_32         nID;
    char_ptr        pName;
    uint_32         nDrvID;
    uint_32         nBit;
    FTE_GPIO_DIR    nDir;
}   FTE_MCP23S08_GPIO_CONFIG, _PTR_ FTE_MCP23S08_GPIO_CONFIG_PTR;

typedef FTE_MCP23S08_GPIO_CONFIG const _PTR_ FTE_MCP23S08_GPIO_CONFIG_CONST_PTR;

typedef struct  _FTE_MCP23S08_GPIO_STRUCT
{
    struct _FTE_MCP23S08_GPIO_STRUCT *  pNext;
    FTE_MCP23S08_GPIO_CONFIG_CONST_PTR  pConfig;
    FTE_MCP23S08_PTR                    pMCP23S08;
    uint_32                             nParent;
    uint_8                              nPolarity;
    uint_32                             nFlags;
    uint_8                              nDir;
    uint_8                              nValue;
    uint_8                              nIntEnable;
    uint_8                              bINTFlag;
    void                                (*fISR)(void *);
    void                                *pParams;
}   FTE_MCP23S08_GPIO, _PTR_ FTE_MCP23S08_GPIO_PTR;

_mqx_uint   FTE_MCP23S08_GPIO_create(FTE_MCP23S08_GPIO_CONFIG_PTR pConfig);
_mqx_uint   FTE_MCP23S08_GPIO_attach(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, uint_32 nParent);
_mqx_uint   FTE_MCP23S08_GPIO_detach(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO);
FTE_MCP23S08_GPIO_PTR   FTE_MCP23S08_GPIO_get(uint_32 nID);

_mqx_uint   FTE_MCP23S08_GPIO_setValue(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, uint_32 value);
_mqx_uint   FTE_MCP23S08_GPIO_getValue(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, uint_32 *value);
_mqx_uint   FTE_MCP23S08_GPIO_setDIR(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, FTE_GPIO_DIR nValue);
_mqx_uint   FTE_MCP23S08_GPIO_getDIR(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, FTE_GPIO_DIR_PTR pValue);

_mqx_uint   FTE_MCP23S08_GPIO_INT_setPolarity(FTE_MCP23S08_GPIO_PTR pGPIO, uint_32 nPolarity);
_mqx_uint   FTE_MCP23S08_GPIO_INT_enable(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, boolean bEnable);
_mqx_uint   FTE_MCP23S08_GPIO_INT_getFlag(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, boolean *pFlag);
_mqx_uint   FTE_MCP23S08_GPIO_INT_clrFlag(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO);
_mqx_uint   FTE_MCP23S08_GPIO_ISR_set(FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO, void (*func)(void *), void *params);

#endif