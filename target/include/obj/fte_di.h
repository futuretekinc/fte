#ifndef _FTE_DI_H__
#define _FTE_DI_H__

#define FTE_DI_TYPE(x)      (((x) & 0x00FF0000) >> FTE_OBJ_TYPE_SHIFT)
#define FTE_DI_CONFIG_FLAG_POLARITY_HI  0x010000

/*****************************************************************************
 * Digital Input Object Structure Description
 *****************************************************************************/

typedef struct _fte_di_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32             nGPIO;
    FTE_UINT32             nLED;
    FTE_UINT32             ulDelay;
    FTE_UINT32             ulHold;
}   FTE_DI_CONFIG, _PTR_ FTE_DI_CONFIG_PTR;

typedef struct  _fte_di_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIO;
    FTE_VALUE           xPresetValue;
}   FTE_DI_STATUS, _PTR_ FTE_DI_STATUS_PTR;

FTE_RET     FTE_DI_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_DI_detach (FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_DI_printValue(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);

FTE_UINT32  FTE_DI_count(void);
FTE_RET     FTE_DI_update(void);

FTE_RET     FTE_DI_getValue(FTE_OBJECT_ID  nID, FTE_UINT32_PTR pValue);
FTE_BOOL    FTE_DI_isActive(FTE_OBJECT_ID  nID);
FTE_RET     FTE_DI_INT_lock(FTE_OBJECT_PTR  pObj);
FTE_RET     FTE_DI_INT_unlock(FTE_OBJECT_PTR  pObj);
FTE_RET     FTE_DI_setPolarity(FTE_OBJECT_PTR pObj, FTE_BOOL bActiveHI);


#define     FTE_DI_POLARITY(pObj)    FTE_OBJ_FLAG_IS_SET((pObj)->pConfig->xFlags, FTE_DI_CONFIG_FLAG_POLARITY_HI)

FTE_INT32   FTE_DI_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);

extern  
FTE_DI_CONFIG FTE_GPIO_DI_defaultConfig;

#endif
