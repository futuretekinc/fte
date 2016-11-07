#ifndef __FTE_MULTI_H__
#define __FTE_MULTI_H__

#define FTE_MULTI_MODEL_BOTEM_PN1500_SENS               1

#define FTE_MULTI_RESPONSE_TIME   500

typedef struct FTE_MULTI_MODEL_INFO_STRUCT
{
    FTE_UINT32  nModel;
    FTE_CHAR_PTR    pName;
    FTE_UINT32  nMaxResponseTime;
    FTE_UINT32  nFieldCount;    
    FTE_RET     (*f_attach)(FTE_OBJECT_PTR pObj);
    FTE_RET     (*f_detach)(FTE_OBJECT_PTR pObj);
    FTE_RET     (*f_start_measurement)(FTE_OBJECT_PTR pObj);
    FTE_RET     (*f_request_data)(FTE_OBJECT_PTR pObj);
    FTE_RET     (*f_receive_data)(FTE_OBJECT_PTR pObj);
    FTE_RET     (*f_set)(FTE_OBJECT_PTR pObj, FTE_UINT32 nIndex, FTE_UINT32 nValue);
} FTE_MULTI_MODEL_INFO, _PTR_ FTE_MULTI_MODEL_INFO_PTR;

typedef FTE_MULTI_MODEL_INFO const _PTR_ FTE_MULTI_MODEL_INFO_CONST_PTR;
/*****************************************************************************
 * MULTI Object Structure Description
 *****************************************************************************/
typedef struct FTE_MULTI_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32      nModel;
    FTE_UINT32      nSensorID;
    FTE_UINT32      nUCSID;
    FTE_UINT32      nInterval;
}   FTE_MULTI_CONFIG, _PTR_ FTE_MULTI_CONFIG_PTR;

typedef FTE_MULTI_CONFIG const _PTR_ FTE_MULTI_CONFIG_CONST_PTR;

typedef struct  FTE_MULTI_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32      hRepeatTimer;
    FTE_UINT32      hConvertTimer;
    FTE_UCS_PTR     pUCS;
    FTE_MULTI_MODEL_INFO_CONST_PTR  pModelInfo;
    FTE_UINT32      nTrial;

}   FTE_MULTI_STATUS, _PTR_ FTE_MULTI_STATUS_PTR;

FTE_RET       FTE_MULTI_attach(FTE_OBJECT_PTR pObj);
FTE_RET       FTE_MULTI_detach(FTE_OBJECT_PTR pObj);

FTE_INT32      FTE_MULTI_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );

#endif
