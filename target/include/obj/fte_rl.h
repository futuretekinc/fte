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
    FTE_UINT32             nGPIOOpen;
    FTE_UINT32             nGPIOClose;
    FTE_UINT32             nInitClosed;
    FTE_UINT32             nLED;
}   FTE_RL_CONFIG, _PTR_ FTE_RL_CONFIG_PTR;

typedef struct  _FTE_RL_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIOOpen;
    FTE_GPIO_PTR        pGPIOClose;
}   FTE_RL_STATUS, _PTR_ FTE_RL_STATUS_PTR;

FTE_RET     FTE_RL_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_RL_detach(FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_RL_printValue(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);

FTE_UINT32  FTE_RL_count(void);

FTE_RET     FTE_RL_setValue(FTE_OBJECT_ID  nID, FTE_BOOL bValue);
FTE_RET     FTE_RL_getInitState(FTE_OBJECT_PTR obj, FTE_UINT32_PTR pState);
FTE_RET     FTE_RL_setInitState(FTE_OBJECT_PTR obj, FTE_UINT32 nState);

FTE_RET     FTE_RL_setPermanent(FTE_OBJECT_ID  nID);

FTE_INT32   FTE_RL_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);

#endif
