#ifndef __FTE_SRF_H__
#define __FTE_SRF_H__


/*****************************************************************************
 * SRF Object Structure Description
 *****************************************************************************/
typedef struct _fte_srf_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nDevID;
    uint_32             nInterval;
}   FTE_SRF_CONFIG, _PTR_ FTE_SRF_CONFIG_PTR;

typedef FTE_SRF_CONFIG const _PTR_ FTE_SRF_CONFIG_CONST_PTR;

typedef struct  _fte_srf_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    uint_32             hRepeatTimer;
    uint_32             hConvertTimer;
    MQX_FILE_PTR        pFP;
}   FTE_SRF_STATUS, _PTR_ FTE_SRF_STATUS_PTR;

typedef struct _fte_srf_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_get)(FTE_OBJECT_PTR pSelf, uint_32_ptr pValue, TIME_STRUCT_PTR pTimeStamp);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
}   FTE_SRF_ACTION, _PTR_ FTE_SRF_ACTION_PTR;

_mqx_uint fte_srf_preinit(uint_32 nMaxObjects);

_mqx_uint   fte_srf_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_srf_detach(FTE_OBJECT_PTR pObj);

#endif
