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
    uint_32             nGPIO;
    uint_32             nLED;
}   FTE_DO_CONFIG, _PTR_ FTE_DO_CONFIG_PTR;

typedef struct  _fte_do_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIO;
}   FTE_DO_STATUS, _PTR_ FTE_DO_STATUS_PTR;

typedef struct _fte_do_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_get)(FTE_OBJECT_PTR pSelf, uint_32_ptr pValue, TIME_STRUCT_PTR pTimeStamp);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
    _mqx_uint       (*f_opened)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_closed)(FTE_OBJECT_PTR pSelf);
}   FTE_DO_ACTION, _PTR_ FTE_DO_ACTION_PTR;

_mqx_uint       FTE_DO_attach(FTE_OBJECT_PTR pObj);
_mqx_uint       FTE_DO_detach(FTE_OBJECT_PTR pObj);
uint_32         FTE_DO_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);

uint_32         FTE_DO_count(void);

_mqx_uint       FTD_DO_getValue(FTE_OBJECT_ID  nID, boolean *pbValue);
_mqx_uint       FTE_DO_setValue(FTE_OBJECT_ID  nID, boolean bValue);
_mqx_uint       FTE_DO_getInitState(FTE_OBJECT_PTR pObj,uint_32_ptr pState);
_mqx_uint       FTE_DO_setInitState(FTE_OBJECT_PTR pObj, uint_32 nState);

_mqx_uint       FTE_DO_setPermanent(FTE_OBJECT_ID  nID);

int_32          FTE_DO_SHELL_cmd(int_32 argc, char_ptr argv[]);
#endif
