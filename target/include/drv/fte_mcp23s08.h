#ifndef __FTE_MCP23S08_H__
#define __FTE_MCP23S08_H__

#define FTE_MCP23S08_BASE_ADDR          0x40
#define FTE_MCP23S08_SLAVE_ADDR_MASK    0x03
#define FTE_MCP23S08_SLAVE_ADDR_SHIFT   0x01

#define FTE_MCP23S08_SPI_WRITE          0x00
#define FTE_MCP23S08_SPI_READ           0x01

#define FTE_MCP23S08_REG_COUNT          11
#define FTE_MCP23S08_REG_MASK           0x0F

#define FTE_MCP23S08_REG_IODIR          0x00
#define FTE_MCP23S08_REG_IPOL           0x01
#define FTE_MCP23S08_REG_GPINTEN        0x02
#define FTE_MCP23S08_REG_DEFVAL         0x03
#define FTE_MCP23S08_REG_INTCON         0x04
#define FTE_MCP23S08_REG_IOCON          0x05
#define FTE_MCP23S08_REG_GPPU           0x06
#define FTE_MCP23S08_REG_INTF           0x07
#define FTE_MCP23S08_REG_INTCAP         0x08
#define FTE_MCP23S08_REG_GPIO           0x09
#define FTE_MCP23S08_REG_OLAT           0x0A

#define FTE_MCP23S08_INT_RISING         0x01
#define FTE_MCP23S08_INT_FALLING        0x02
#define FTE_MCP23S08_INT_LOW            0x03
#define FTE_MCP23S08_INT_HIGH           0x04
#define FTE_MCP23S08_INT_CHANGE         0x05

typedef struct _FTE_MCP23S08_CONFIG_STRUCT
{
    uint_32     nID;
    char_ptr    pName;
    uint_32     xSPI;       // SPI channel id
    uint_32     xSlaveAddr;
    uint_32     xGPIOReset;
    uint_32     xGPIOInt;
}   FTE_MCP23S08_CONFIG, _PTR_ FTE_MCP23S08_CONFIG_PTR;

typedef FTE_MCP23S08_CONFIG const _PTR_ FTE_MCP23S08_CONFIG_CONST_PTR;

typedef struct  _FTE_MCP23S08_STRUCT
{
    struct _FTE_MCP23S08_STRUCT *   pNext;
    FTE_MCP23S08_CONFIG_CONST_PTR   pConfig;
    uint_32                         pParent[8];
    void                            (*f_isr[8])(void*);
    void*                           pParams[8];
    uint_8                          pPolaries[8];
    uint_8                          pRegs[11];
    uint_32                         nFlags;
    uint_8                          nEnableMask;
    uint_8                          nINTMask;
    uint_8                          nIntFlags;
    _mqx_uint                       xTimerID;
    FTE_SPI_PTR                     pSPI;
    FTE_LWGPIO_PTR                  pLWGPIO;
    FTE_LWGPIO_PTR                  pLWGPIO_Reset;
}   FTE_MCP23S08, _PTR_ FTE_MCP23S08_PTR;

FTE_MCP23S08_PTR FTE_MCP23S08_get(uint_32 nID);
FTE_MCP23S08_PTR FTE_MCP23S08_get_first(void);
FTE_MCP23S08_PTR FTE_MCP23S08_getNext(FTE_MCP23S08_PTR pObj);

_mqx_uint   FTE_MCP23S08_create(FTE_MCP23S08_CONFIG_PTR pConfig);
_mqx_uint   FTE_MCP23S08_attach(FTE_MCP23S08_PTR pObj, uint_32 nParentID);
_mqx_uint   FTE_MCP23S08_detach(FTE_MCP23S08_PTR pObj, uint_32 nParentID);

_mqx_uint   FTE_MCP23S08_getReg(FTE_MCP23S08_PTR pMCP23S08, uint_32 nReg, uint_32_ptr pValue);
_mqx_uint   FTE_MCP23S08_setReg(FTE_MCP23S08_PTR pMCP23S08, uint_32 nReg, uint_32     nValue);

_mqx_uint   FTE_MCP23S08_setRegBit(FTE_MCP23S08_PTR pMCP23S08, uint_32 nReg, uint_32 nBit, uint_32     nValue);
_mqx_uint   FTE_MCP23S08_getRegBit(FTE_MCP23S08_PTR pMCP23S08, uint_32 nReg, uint_32 nBit, uint_32_ptr pValue);

_mqx_uint   FTE_MCP23S08_INT_initRegBit(FTE_MCP23S08_PTR pMCP23S08, uint_32 nBit, void (*func)(void *), void *params);
_mqx_uint   FTE_MCP23S08_INT_enableRegBit(FTE_MCP23S08_PTR pMCP23S08, uint_32 nBit, boolean bEnable);
_mqx_uint   FTE_MCP23S08_INT_setPolarityRegBit(FTE_MCP23S08_PTR pMCP23S08, uint_32 nBit, uint_32 nPolarity);

int_32      FTE_MCP23S08_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
