#ifndef __FTE_LWGPIO_H__
#define __FTE_LWGPIO_H__

#define FTE_LWGPIO_INT_RISING     0x01
#define FTE_LWGPIO_INT_FALLING    0x02
#define FTE_LWGPIO_INT_LOW        0x03
#define FTE_LWGPIO_INT_HIGH       0x04

typedef struct _FTE_LWGPIO_configuration_struct
{
    uint_32         nID;

    LWGPIO_PIN_ID   nLWGPIO;
    uint_32         nMUX;
    LWGPIO_DIR      nDIR;
    LWGPIO_VALUE    nInit;
    LWGPIO_VALUE    nActive;
    LWGPIO_VALUE    nInactive;
}   FTE_LWGPIO_CONFIG, _PTR_ FTE_LWGPIO_CONFIG_PTR;

typedef FTE_LWGPIO_CONFIG const _PTR_ FTE_LWGPIO_CONFIG_CONST_PTR;

typedef struct _FTE_LWGPIO_STRUCT
{
    struct _FTE_LWGPIO_STRUCT *     pNext;
    FTE_LWGPIO_CONFIG_CONST_PTR     pConfig;
    uint_32                         nParent;
    LWGPIO_STRUCT                   xLWGPIO;
}   FTE_LWGPIO, _PTR_ FTE_LWGPIO_PTR;

_mqx_uint       FTE_LWGPIO_create(FTE_LWGPIO_CONFIG_CONST_PTR  pConfig);
_mqx_uint       FTE_LWGPIO_attach(FTE_LWGPIO_PTR pLWGPIO, uint_32 nParent);
_mqx_uint       FTE_LWGPIO_detach(FTE_LWGPIO_PTR pLWGPIO);

FTE_LWGPIO_PTR  FTE_LWGPIO_get(uint_32 nID);
uint_32         FTE_LWGPIO_count(void);

_mqx_uint       FTE_LWGPIO_setValue(FTE_LWGPIO_PTR pLWGPIO, boolean value);
_mqx_uint       FTE_LWGPIO_getValue(FTE_LWGPIO_PTR pLWGPIO, boolean *value);
_mqx_uint       FTE_LWGPIO_setDirection(FTE_LWGPIO_PTR pLWGPIO, LWGPIO_DIR nValue);
_mqx_uint       FTE_LWGPIO_setPullUp(FTE_LWGPIO_PTR pLWGPIO, boolean bEnable);
_mqx_uint       FTE_LWGPIO_setPullDown(FTE_LWGPIO_PTR pLWGPIO, boolean bEnable);
_mqx_uint       FTE_LWGPIO_setISR(FTE_LWGPIO_PTR pLWGPIO, void (*isr)(void*), void*);
_mqx_uint       FTE_LWGPIO_INT_init(FTE_LWGPIO_PTR pLWGPIO, uint_32 priority, uint_32 subpriority, boolean enable);
_mqx_uint       FTE_LWGPIO_INT_setPolarity(FTE_LWGPIO_PTR pLWGPIO, uint_32 polarity);
_mqx_uint       FTE_LWGPIO_INT_setEnable(FTE_LWGPIO_PTR pLWGPIO, boolean enable);
_mqx_uint       FTE_LWGPIO_INT_getFlag(FTE_LWGPIO_PTR pLWGPIO, boolean *flag);
_mqx_uint       FTE_LWGPIO_INT_clrFlag(FTE_LWGPIO_PTR pLWGPIO);

int_32          FTE_LWGPIO_SHELL_cmd(int_32 argc, char_ptr argv[] );
#endif
