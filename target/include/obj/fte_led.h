#ifndef __FTE_LED_H__
#define __FTE_LED_H__

/*****************************************************************************
 * Digital Ouptut Object Structure Description
 *****************************************************************************/

#define FTE_LED_STATE_OFF       0
#define FTE_LED_STATE_ON        1
#define FTE_LED_STATE_BLINK     2
     
#define FTE_LED_NORMAL_OPEN     LWGPIO_VALUE_LOW
#define FTE_LED_NORMAL_CLOSE    LWGPIO_VALUE_HIGH

typedef struct _FTE_LED_CONFIG_PTR
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nGPIO;
    uint_32             nInit;
}   FTE_LED_CONFIG, _PTR_ FTE_LED_CONFIG_PTR;

typedef struct  _FTE_LED_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIO;
}   FTE_LED_STATUS, _PTR_ FTE_LED_STATUS_PTR;

typedef struct _fte_led_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_get)(FTE_OBJECT_PTR pSelf, uint_32_ptr pValue, TIME_STRUCT_PTR pTimeStamp);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
}   FTE_LED_ACTION, _PTR_ FTE_LED_ACTION_PTR;

_mqx_uint   FTE_LED_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_LED_detach(FTE_OBJECT_PTR pObj);
uint_32     FTE_LED_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);

uint_32     FTE_LED_count(void);
pointer     FTE_LED_get(FTE_OBJECT_ID nID);

_mqx_uint   FTE_LED_setValue(FTE_OBJECT_ID  nID, uint_32 nValue);
boolean     FTE_LED_isActive(FTE_OBJECT_ID  nID);

#endif
