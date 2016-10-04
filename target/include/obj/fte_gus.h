#ifndef __FTE_GUS_H__
#define __FTE_GUS_H__

#define FTE_GUS_MODEL_PM1001                1
#define FTE_GUS_MODEL_COZIR_AX5000          2
#define FTE_GUS_MODEL_TASCON_HEM12          3
#define FTE_GUS_MODEL_FIS3061               4
#define FTE_GUS_MODEL_SH_MV250              5
#define FTE_GUS_MODEL_TRUEYES_AIRQ          6
#define FTE_GUS_MODEL_ELT_AQM100            7
#define FTE_GUS_MODEL_MST_MEX510C           8
#define FTE_GUS_MODEL_GS_DPC_HL             9
#define FTE_GUS_MODEL_TASCON_HEM12_06M      10
#define FTE_GUS_MODEL_BOTEM_PN1500          11
#define FTE_GUS_MODEL_FTLM                  12
#define FTE_GUS_MODEL_CIAS_SIOUX_CU         13
#define FTE_GUS_MODEL_IOEX                  14
#define FTE_GUS_MODEL_DOTECH_FX3D           15
#define FTE_GUS_RESPONSE_TIME   500

typedef struct FTE_GUS_MODEL_INFO_STRUCT
{
    uint_32             nModel;
    char_ptr            pName;
    uint_32             nMaxResponseTime;
    uint_32             nFieldCount;  
    FTE_VALUE_TYPE_PTR  pValueTypes;
    _mqx_uint           (*f_attach)(FTE_OBJECT_PTR pObj);
    _mqx_uint           (*f_detach)(FTE_OBJECT_PTR pObj);
    _mqx_uint           (*f_start_measurement)(FTE_OBJECT_PTR pObj);
    _mqx_uint           (*f_init)(FTE_OBJECT_PTR pObj);
    _mqx_uint           (*f_request)(FTE_OBJECT_PTR pObj);
    _mqx_uint           (*f_received)(FTE_OBJECT_PTR pObj);
    _mqx_uint           (*f_set)(FTE_OBJECT_PTR pObject, uint_32 nIndex, FTE_VALUE_PTR pValue);
    _mqx_uint           (*f_get)(FTE_OBJECT_PTR pObject, uint_32 nIndex, FTE_VALUE_PTR pValue);
    _mqx_uint           (*f_set_config)(FTE_OBJECT_PTR pObject, char_ptr pBuff);
    _mqx_uint           (*f_get_config)(FTE_OBJECT_PTR pObject, char_ptr pBuff, uint_32 ulBuffLen);
    _mqx_uint           (*f_set_child_config)(FTE_OBJECT_PTR pChild, char_ptr pBuff);
    _mqx_uint           (*f_get_child_config)(FTE_OBJECT_PTR pChild, char_ptr pBuff, uint_32 ulBuffLen);

} FTE_GUS_MODEL_INFO, _PTR_ FTE_GUS_MODEL_INFO_PTR;

typedef FTE_GUS_MODEL_INFO const _PTR_ FTE_GUS_MODEL_INFO_CONST_PTR;
/*****************************************************************************
 * GUS Object Structure Description
 *****************************************************************************/
typedef struct FTE_GUS_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nModel;
    uint_32             nUCSID;
    uint_32             nInterval;
    uint_32             nSensorID;
}   FTE_GUS_CONFIG, _PTR_ FTE_GUS_CONFIG_PTR;

typedef FTE_GUS_CONFIG const _PTR_ FTE_GUS_CONFIG_CONST_PTR;

typedef struct FTE_GUS_STATUS_STRUCT
{
    FTE_OBJECT_STATUS       xCommon;
    uint_32                 hRepeatTimer;
    uint_32                 hConvertTimer;
    FTE_UCS_PTR             pUCS;
    FTE_GUS_MODEL_INFO_CONST_PTR  pModelInfo;
    uint_32                 nTrial;
    
    _mqx_uint               xRet;
    FTE_LIST                xChildList;
}   FTE_GUS_STATUS, _PTR_ FTE_GUS_STATUS_PTR;

_mqx_uint       FTE_GUS_search(void);
_mqx_uint       FTE_GUS_attach(FTE_OBJECT_PTR pObj);
_mqx_uint       FTE_GUS_detach(FTE_OBJECT_PTR pObj);
_mqx_uint       FTE_GUS_attachChild(FTE_OBJECT_PTR pSelf, uint_32 nChild);
_mqx_uint       FTE_GUS_detachChild(FTE_OBJECT_PTR pSelf, uint_32 nChild);
int_32     FTE_GUS_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
