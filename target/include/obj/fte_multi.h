#ifndef __FTE_MULTI_H__
#define __FTE_MULTI_H__

#define FTE_MULTI_MODEL_BOTEM_PN1500_SENS               1

#define FTE_MULTI_RESPONSE_TIME   500

typedef struct _fte_multi_model_info
{
    uint_32     nModel;
    char_ptr    pName;
    uint_32     nMaxResponseTime;
    uint_32     nFieldCount;    
    _mqx_uint   (*f_attach)(FTE_OBJECT_PTR pObj);
    _mqx_uint   (*f_detach)(FTE_OBJECT_PTR pObj);
    _mqx_uint   (*f_start_measurement)(FTE_OBJECT_PTR pObj);
    _mqx_uint   (*f_request_data)(FTE_OBJECT_PTR pObj);
    _mqx_uint   (*f_receive_data)(FTE_OBJECT_PTR pObj);
    _mqx_uint   (*f_set)(FTE_OBJECT_PTR pObj, uint_32 nIndex, uint_32 nValue);
} FTE_MULTI_MODEL_INFO, _PTR_ FTE_MULTI_MODEL_INFO_PTR;

typedef FTE_MULTI_MODEL_INFO const _PTR_ FTE_MULTI_MODEL_INFO_CONST_PTR;
/*****************************************************************************
 * MULTI Object Structure Description
 *****************************************************************************/
typedef struct _fte_multi_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nModel;
    uint_32             nSensorID;
    uint_32             nUCSID;
    uint_32             nInterval;
}   FTE_MULTI_CONFIG, _PTR_ FTE_MULTI_CONFIG_PTR;

typedef FTE_MULTI_CONFIG const _PTR_ FTE_MULTI_CONFIG_CONST_PTR;

typedef struct  _fte_multi_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    uint_32             hRepeatTimer;
    uint_32             hConvertTimer;
    FTE_UCS_PTR         pUCS;
    FTE_MULTI_MODEL_INFO_CONST_PTR  pModelInfo;
    uint_32         nTrial;

}   FTE_MULTI_STATUS, _PTR_ FTE_MULTI_STATUS_PTR;

typedef struct _fte_multi_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, uint_32 nValue);
    _mqx_uint       (*f_set_multi)(FTE_OBJECT_PTR pSelf, uint_32 nIndex, uint_32 nValue);
    uint_32         (*f_print_value)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 nBuffLen);
}   FTE_MULTI_ACTION, _PTR_ FTE_MULTI_ACTION_PTR;

_mqx_uint       fte_multi_attach(FTE_OBJECT_PTR pObj);
_mqx_uint       fte_multi_detach(FTE_OBJECT_PTR pObj);

int_32      fte_multi_shell_cmd(int_32 argc, char_ptr argv[] );

#endif
