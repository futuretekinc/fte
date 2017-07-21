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
#define FTE_GUS_MODEL_ELT_AQM100M           16
#define FTE_GUS_MODEL_DIO_NODE              17
#define FTE_GUS_MODEL_FUTURETE_MS10         18
#define FTE_GUS_MODEL_FUTURETE_KM21B20      19
#define FTE_GUS_MODEL_FUTURETE_JCG06        20
#define FTE_GUS_RESPONSE_TIME               500

#define FTE_GUS_FLAG_SHARED                 0x00000001

typedef struct FTE_GUS_MODEL_INFO_STRUCT
{
    FTE_UINT32          nModel;
    FTE_CHAR_PTR        pName;
    FTE_UINT32          xFlags;
    FTE_UCS_UART_CONFIG xUARTConfig;
    FTE_UINT32          nMaxResponseTime;
    FTE_UINT32          nFieldCount;  
    FTE_VALUE_TYPE_PTR  pValueTypes;
    FTE_RET             (*fCreate)(FTE_CHAR_PTR pDeviceID, FTE_OBJECT_PTR _PTR_ ppObj);
    FTE_RET             (*fDestroy)(FTE_OBJECT_PTR _PTR_ ppObj);
    FTE_RET             (*fAttach)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fDetach)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fStartMeasurement)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fInit)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fFinal)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fRun)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fStop)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fUpdate)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fRequest)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fReceived)(FTE_OBJECT_PTR pObj);
    FTE_RET             (*fSet)(FTE_OBJECT_PTR pObject, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);
    FTE_RET             (*fGet)(FTE_OBJECT_PTR pObject, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);
    FTE_RET             (*fSetConfig)(FTE_OBJECT_PTR pObject, FTE_CHAR_PTR pBuff);
    FTE_RET             (*fGetConfig)(FTE_OBJECT_PTR pObject, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);
    FTE_RET             (*fSetChildConfig)(FTE_OBJECT_PTR pChild, FTE_CHAR_PTR pBuff);
    FTE_RET             (*fGetChildConfig)(FTE_OBJECT_PTR pChild, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);
    FTE_RET             (*fCreateJSON)(FTE_OBJECT_PTR pObject, FTE_UINT32 ulOption, FTE_JSON_OBJECT_PTR _PTR_ ppJSON);
} FTE_GUS_MODEL_INFO, _PTR_ FTE_GUS_MODEL_INFO_PTR;

typedef FTE_GUS_MODEL_INFO const _PTR_ FTE_GUS_MODEL_INFO_CONST_PTR;
/*****************************************************************************
 * GUS Object Structure Description
 *****************************************************************************/
typedef struct FTE_GUS_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          nModel;
    FTE_UINT32          nUCSID;
    FTE_UINT32          nInterval;
    FTE_UINT32          nSensorID;
}   FTE_GUS_CONFIG, _PTR_ FTE_GUS_CONFIG_PTR;

typedef FTE_GUS_CONFIG const _PTR_ FTE_GUS_CONFIG_CONST_PTR;

typedef struct FTE_GUS_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32          hRepeatTimer;
    FTE_UINT32          hConvertTimer;
    FTE_UCS_PTR         pUCS;
    FTE_GUS_MODEL_INFO_CONST_PTR  pModelInfo;
    FTE_UINT32          nTrial;
    
    FTE_RET             xRet;
    FTE_LIST            xChildList;
}   FTE_GUS_STATUS, _PTR_ FTE_GUS_STATUS_PTR;

FTE_RET     FTE_GUS_search(void);
FTE_RET     FTE_GUS_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_GUS_detach(FTE_OBJECT_PTR pObj);
FTE_RET     FTE_GUS_attachChild(FTE_OBJECT_PTR pSelf, FTE_UINT32 nChild);
FTE_RET     FTE_GUS_detachChild(FTE_OBJECT_PTR pSelf, FTE_UINT32 nChild);

FTE_INT32   FTE_GUS_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
);

#endif
