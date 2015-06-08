#ifndef __FTE_SHT_H__
#define __FTE_SHT_H__

#define FTE_SHT_FLAG_CONVERT_HUMIDITY    0x0100
#define FTE_SHT_FLAG_CONVERT_TEMPERATURE 0x0200

#define FTE_SHT_DELAY_MAX                   5

/*****************************************************************************
 * SHT Object Structure Description
 *****************************************************************************/
typedef struct _fte_sht_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nSDA;
    uint_32             nSCL;
    uint_32             nInterval;
    uint_32             ulDelay;
}   FTE_SHT_CONFIG, _PTR_ FTE_SHT_CONFIG_PTR;

typedef struct  _fte_sht_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    uint_32             hRepeatTimer;
    uint_32             hConvertTimer;
    uint_32             bHumidity;
    FTE_LWGPIO_PTR      pLWGPIO_SDA;
    FTE_LWGPIO_PTR      pLWGPIO_SCL;
    
}   FTE_SHT_STATUS, _PTR_ FTE_SHT_STATUS_PTR;

typedef struct _fte_sht_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_get)(FTE_OBJECT_PTR pSelf, uint_32_ptr pValue, TIME_STRUCT_PTR pTimeStamp);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
}   FTE_SHT_ACTION, _PTR_ FTE_SHT_ACTION_PTR;

_mqx_uint       FTE_SHT_attach(FTE_OBJECT_PTR pObj);
_mqx_uint       FTE_SHT_detach(FTE_OBJECT_PTR pObj);
uint_32         FTE_SHT_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);

int_32          FTE_SHT_SHELL_cmd(int_32 argc, char_ptr argv[]);

#endif
