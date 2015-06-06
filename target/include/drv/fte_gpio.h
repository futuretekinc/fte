#ifndef __FTE_GPIO_H__
#define __FTE_GPIO_H__

#define FTE_GPIO_INT_RISING     0x01
#define FTE_GPIO_INT_FALLING    0x02
#define FTE_GPIO_INT_LOW        0x03
#define FTE_GPIO_INT_HIGH       0x04

typedef enum 
{
    FTE_GPIO_DIR_INPUT,
    FTE_GPIO_DIR_OUTPUT,
    FTE_GPIO_DIR_NOCHANGE
} FTE_GPIO_DIR, _PTR_ FTE_GPIO_DIR_PTR;

typedef enum 
{
    FTE_GPIO_VALUE_LOW,
    FTE_GPIO_VALUE_HIGH,
    FTE_GPIO_VALUE_NOCHANGE
} FTE_GPIO_VALUE, _PTR_ FTE_GPIO_VALUE_PTR;

typedef struct _FTE_GPIO_CONFIG_STRUCT
{
    uint_32         nID;
    uint_32         nDevID;
    FTE_GPIO_DIR    nDIR;
    FTE_GPIO_VALUE  nInit;
    FTE_GPIO_VALUE  nActive;
}   FTE_GPIO_CONFIG, _PTR_ FTE_GPIO_CONFIG_PTR;

typedef FTE_GPIO_CONFIG const _PTR_ FTE_GPIO_CONFIG_CONST_PTR;
 
typedef struct _FTE_GPIO_STRUCT
{
    struct _FTE_GPIO_STRUCT *   pNext;
    FTE_GPIO_CONFIG_CONST_PTR   pConfig;
    uint_32                     nParent;
    FTE_DRIVER_PTR              pPort;
}   FTE_GPIO, _PTR_ FTE_GPIO_PTR;

_mqx_uint   FTE_GPIO_create(FTE_GPIO_CONFIG_PTR pConfig);
_mqx_uint   FTE_GPIO_attach(FTE_GPIO_PTR pGPIO, uint_32 nParent);
_mqx_uint   FTE_GPIO_detach(FTE_GPIO_PTR pGPIO);
uint_32     FTE_GPIO_count(void);

FTE_GPIO_PTR FTE_GPIO_get(uint_32 xDevID);

_mqx_uint   FTE_GPIO_setValue(FTE_GPIO_PTR pGPIO, boolean bValue);
_mqx_uint   FTE_GPIO_getValue(FTE_GPIO_PTR pGPIO, boolean *pbValue);
_mqx_uint   FTE_GPIO_setDir(FTE_GPIO_PTR pGPIO, FTE_GPIO_DIR xValue);
_mqx_uint   FTE_GPIO_setISR(FTE_GPIO_PTR pGPIO,void (*isr)(void*), void*);
_mqx_uint   FTE_GPIO_INT_init(FTE_GPIO_PTR pGPIO, uint_32 ulPriority, uint_32 ulSubPriority, boolean bEnable);
_mqx_uint   FTE_GPIO_INT_setPolarity(FTE_GPIO_PTR pGPIO, uint_32 ulPolarity);
_mqx_uint   FTE_GPIO_INT_setEnable(FTE_GPIO_PTR pGPIO, boolean bEnable);
_mqx_uint   FTE_GPIO_INT_getFlag(FTE_GPIO_PTR pGPIO, boolean *pbFlag);
_mqx_uint   FTE_GPIO_INT_clrFlag(FTE_GPIO_PTR pGPIO);

int_32      FTE_GPIO_SHELL_cmd(int_32 argc, char_ptr argv[] );
#endif
