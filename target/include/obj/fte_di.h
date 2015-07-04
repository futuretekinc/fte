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
    uint_32             nGPIO;
    uint_32             nLED;
}   FTE_DI_CONFIG, _PTR_ FTE_DI_CONFIG_PTR;

typedef struct  _fte_di_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIO;
    TIME_STRUCT         xLastOccurredTime;
}   FTE_DI_STATUS, _PTR_ FTE_DI_STATUS_PTR;

_mqx_uint       FTE_DI_attach(FTE_OBJECT_PTR pObj);
_mqx_uint       FTE_DI_detach (FTE_OBJECT_PTR pObj);
uint_32         FTE_DI_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);

uint_32         FTE_DI_count(void);

_mqx_uint       FTE_DI_getValue(FTE_OBJECT_ID  nID, uint_32_ptr pValue);
boolean         FTE_DI_isActive(FTE_OBJECT_ID  nID);
_mqx_uint       FTE_DI_INT_lock(FTE_OBJECT_PTR  pObj);
_mqx_uint       FTE_DI_INT_unlock(FTE_OBJECT_PTR  pObj);
_mqx_uint       FTE_DI_setPolarity(FTE_OBJECT_PTR pObj, boolean bActiveHI);


#define     FTE_DI_POLARITY(pObj)    FTE_OBJ_FLAG_IS_SET((pObj)->pConfig->xFlags, FTE_DI_CONFIG_FLAG_POLARITY_HI)

int_32      FTE_DI_SHELL_cmd(int_32 argc, char_ptr argv[]);

#endif
