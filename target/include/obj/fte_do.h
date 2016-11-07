#ifndef __FTE_DO_H__
#define __FTE_DO_H__

/*****************************************************************************
 * Digital Ouptut Object Structure Description
 *****************************************************************************/

#define FTE_DO_TYPE(x)          (((x) & 0x00FF0000) >> FTE_OBJ_TYPE_SHIFT)

#define FTE_DO_CONFIG_INIT_ON   0x010000

#define FTE_DO_NORMAL_OPEN  LWGPIO_VALUE_LOW
#define FTE_DO_NORMAL_CLOSE LWGPIO_VALUE_HIGH

typedef struct _fte_do_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32             nGPIO;
    FTE_UINT32             nLED;
}   FTE_DO_CONFIG, _PTR_ FTE_DO_CONFIG_PTR;

typedef struct  _fte_do_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIO;
}   FTE_DO_STATUS, _PTR_ FTE_DO_STATUS_PTR;

FTE_RET     FTE_DO_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_DO_detach(FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_DO_printValue(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);

FTE_UINT32  FTE_DO_count(void);

FTE_RET     FTD_DO_getValue(FTE_OBJECT_ID  nID, FTE_BOOL_PTR pbValue);
FTE_RET     FTE_DO_setValue(FTE_OBJECT_ID  nID, FTE_BOOL bValue);
FTE_RET     FTE_DO_getInitState(FTE_OBJECT_PTR pObj,FTE_UINT32_PTR pState);
FTE_RET     FTE_DO_setInitState(FTE_OBJECT_PTR pObj, FTE_UINT32 nState);

FTE_RET     FTE_DO_setPermanent(FTE_OBJECT_ID  nID);

FTE_INT32   FTE_DO_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
#endif
