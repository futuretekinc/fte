#ifndef __FTE_RL_H__
#define __FTE_RL_H__

/*****************************************************************************
 * Relay Control Object Structure Description
 *****************************************************************************/

#define FTE_RL_CONFIG_INIT_OPEN     0x000000
#define FTE_RL_CONFIG_INIT_CLOSE    0x010000

#define FTE_RL_NORMAL_OPEN  LWGPIO_VALUE_LOW
#define FTE_RL_NORMAL_CLOSE LWGPIO_VALUE_HIGH

typedef struct _FTE_RL_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nGPIOOpen;
    uint_32             nGPIOClose;
    uint_32             nInitClosed;
    uint_32             nLED;
}   FTE_RL_CONFIG, _PTR_ FTE_RL_CONFIG_PTR;

typedef struct  _FTE_RL_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIOOpen;
    FTE_GPIO_PTR        pGPIOClose;
}   FTE_RL_STATUS, _PTR_ FTE_RL_STATUS_PTR;

_mqx_uint   FTE_RL_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_RL_detach(FTE_OBJECT_PTR pObj);
uint_32     FTE_RL_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);

uint_32     FTE_RL_count(void);

_mqx_uint  FTE_RL_setValue(FTE_OBJECT_ID  nID, boolean bValue);
_mqx_uint   FTE_RL_getInitState(FTE_OBJECT_PTR obj, uint_32_ptr pState);
_mqx_uint   FTE_RL_setInitState(FTE_OBJECT_PTR obj, uint_32 nState);

_mqx_uint   FTE_RL_setPermanent(FTE_OBJECT_ID  nID);

int_32      FTE_RL_SHELL_cmd(int_32 argc, char_ptr argv[]);

#endif
