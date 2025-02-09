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
    FTE_UINT32      nID;
    FTE_CHAR_PTR    pName;
    FTE_UINT32      xSPI;       // SPI channel id
    FTE_UINT32      xSlaveAddr;
    FTE_UINT32      xGPIOReset;
    FTE_UINT32      xGPIOInt;
}   FTE_MCP23S08_CONFIG, _PTR_ FTE_MCP23S08_CONFIG_PTR;

typedef FTE_MCP23S08_CONFIG const _PTR_ FTE_MCP23S08_CONFIG_CONST_PTR;

typedef struct  _FTE_MCP23S08_STRUCT
{
    struct _FTE_MCP23S08_STRUCT *   pNext;
    FTE_MCP23S08_CONFIG_CONST_PTR   pConfig;
    FTE_UINT32      pParent[8];
    void            (*f_isr[8])(void*);
    FTE_VOID_PTR    pParams[8];
    FTE_UINT8       pPolaries[8];
    FTE_UINT8       pRegs[11];
    FTE_UINT32      nFlags;
    FTE_UINT8       nEnableMask;
    FTE_UINT8       nINTMask;
    FTE_UINT8       nIntFlags;
    FTE_RET         xTimerID;
    FTE_SPI_PTR     pSPI;
    FTE_LWGPIO_PTR  pLWGPIO;
    FTE_LWGPIO_PTR  pLWGPIO_Reset;
}   FTE_MCP23S08, _PTR_ FTE_MCP23S08_PTR;

FTE_MCP23S08_PTR FTE_MCP23S08_get(FTE_UINT32 nID);
FTE_MCP23S08_PTR FTE_MCP23S08_get_first(void);
FTE_MCP23S08_PTR FTE_MCP23S08_getNext(FTE_MCP23S08_PTR pObj);

FTE_RET   FTE_MCP23S08_create(FTE_MCP23S08_CONFIG_PTR pConfig);
FTE_RET   FTE_MCP23S08_attach(FTE_MCP23S08_PTR pObj, FTE_UINT32 nParentID);
FTE_RET   FTE_MCP23S08_detach(FTE_MCP23S08_PTR pObj, FTE_UINT32 nParentID);

FTE_RET   FTE_MCP23S08_getReg(FTE_MCP23S08_PTR pMCP23S08, FTE_UINT32 nReg, FTE_UINT32_PTR pValue);
FTE_RET   FTE_MCP23S08_setReg(FTE_MCP23S08_PTR pMCP23S08, FTE_UINT32 nReg, FTE_UINT32     nValue);

FTE_RET   FTE_MCP23S08_setRegBit(FTE_MCP23S08_PTR pMCP23S08, FTE_UINT32 nReg, FTE_UINT32 nBit, FTE_UINT32     nValue);
FTE_RET   FTE_MCP23S08_getRegBit(FTE_MCP23S08_PTR pMCP23S08, FTE_UINT32 nReg, FTE_UINT32 nBit, FTE_UINT32_PTR pValue);

FTE_RET   FTE_MCP23S08_INT_initRegBit(FTE_MCP23S08_PTR pMCP23S08, FTE_UINT32 nBit, void (*func)(FTE_VOID_PTR), FTE_VOID_PTR params);
FTE_RET   FTE_MCP23S08_INT_enableRegBit(FTE_MCP23S08_PTR pMCP23S08, FTE_UINT32 nBit, FTE_BOOL bEnable);
FTE_RET   FTE_MCP23S08_INT_setPolarityRegBit(FTE_MCP23S08_PTR pMCP23S08, FTE_UINT32 nBit, FTE_UINT32 nPolarity);

FTE_INT32   FTE_MCP23S08_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR    pArgv[] );

#endif
