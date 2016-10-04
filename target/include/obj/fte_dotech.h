#ifndef _FTE_DOTECH_H__
#define _FTE_DOTECH_H__

#define FTE_DOTECH_FX3D_MAX                 8

#define FTE_DOTECH_FX3D_MODE_OFF            0
#define FTE_DOTECH_FX3D_MODE_COOLING        1
#define FTE_DOTECH_FX3D_MODE_HEATING        2

#define FTE_DOTECH_FX3D_FIELD_TEMP0         0
#define FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0    1
#define FTE_DOTECH_FX3D_FIELD_CTRL_TEMP1    2
#define FTE_DOTECH_FX3D_FIELD_CTRL_STATE0   3
#define FTE_DOTECH_FX3D_FIELD_CTRL_STATE1   4
#define FTE_DOTECH_FX3D_FIELD_SENSOR_ALARM  5
#define FTE_DOTECH_FX3D_FIELD_CTRL0         6
#define FTE_DOTECH_FX3D_FIELD_CTRL1         7

#define FTE_DOTECH_FX3D_FIELD_MAX           8

#define FTE_DOTECH_FX3D_CTRL_COUNT          2

typedef enum    FTE_DOTECH_MODEL_ENUM
{
    FTE_DOTECH_MODEL_FX3D   = 1
} FTE_DOTECH_MODEL, _PTR_ FTE_DOTECH_MODEL_PTR;

typedef struct  FTE_DOTECH_CONFIG_STRUCT
{
    FTE_GUS_CONFIG      xGUS;
    FTE_DOTECH_MODEL    xModel;
}   FTE_DOTECH_CONFIG, _PTR_ FTE_DOTECH_CONFIG_PTR;

typedef struct  FTE_DOTECH_EXT_CONFIG_STRUCT
{
    FTE_UINT32      ulLoopPeriod;
    FTE_UINT32      ulUpdatePeriod;
    FTE_UINT32      ulRequestTimeout;
    FTE_UINT32      ulRetryCount;
}   FTE_DOTECH_EXT_CONFIG, _PTR_ FTE_DOTECH_EXT_CONFIG_PTR;

typedef struct  FTE_DOTECH_FX3D_STATUS_STRUCT
{
    FTE_GUS_STATUS  xGUS;
    FTE_UINT32      ulRetryCount;
    TIME_STRUCT     xLastRequestTime;
    TIME_STRUCT     xLastUpdateTime;
    
    struct
    {
        FTE_UINT32  ulMode;
        FTE_UINT32  ulDeviation;
        FTE_UINT32  ulONDelay;
        FTE_UINT32  ulONMin;
        FTE_UINT32  ulOFFMin;
        FTE_UINT32  ulDeviationMode;
        FTE_INT32   nMaxTemp;        
        FTE_INT32   nMinTemp;        
    }   pOutputConfigs[FTE_DOTECH_FX3D_CTRL_COUNT];
} FTE_DOTECH_FX3D_STATUS, _PTR_ FTE_DOTECH_FX3D_STATUS_PTR;

void    FTE_DOTECH_task(FTE_UINT32 datas);
FTE_RET FTE_DOTECH_initDefaultExtConfig(FTE_DOTECH_EXT_CONFIG_PTR pConfig);

FTE_INT32  FTE_DOTECH_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );

FTE_RET FTE_DOTECH_FX3D_init(FTE_OBJECT_PTR pObj);
FTE_RET FTE_DOTECH_FX3D_reset(void);
FTE_RET FTE_DOTECH_FX3D_attach(FTE_OBJECT_PTR pObj);
FTE_RET FTE_DOTECH_FX3D_detach(FTE_OBJECT_PTR pObj);
FTE_RET FTE_DOTECH_FX3D_setChildConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff);
FTE_RET FTE_DOTECH_FX3D_getChildConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);

FTE_RET FTE_DOTECH_FX3D_get(FTE_OBJECT_PTR pObj, FTE_UINT32 ulIndex, FTE_VALUE_PTR pValue);

extern FTE_VALUE_TYPE       FTE_DOTECH_FX3D_valueTypes[];
extern FTE_DOTECH_CONFIG    FTE_DOTECH_FX3D_defaultConfig;

#define FTE_DOTECH_FX3D_DESCRIPTOR  {\
        .nModel         = FTE_GUS_MODEL_DOTECH_FX3D,   \
        .pName          = "FX3D",               \
        .nFieldCount    = FTE_DOTECH_FX3D_FIELD_MAX,      \
        .pValueTypes    = FTE_DOTECH_FX3D_valueTypes,  \
        .f_attach       = FTE_DOTECH_FX3D_attach,      \
        .f_detach       = FTE_DOTECH_FX3D_detach,      \
        .f_get          = FTE_DOTECH_FX3D_get,         \
        .f_get_child_config   = FTE_DOTECH_FX3D_getChildConfig,   \
        .f_set_child_config   = FTE_DOTECH_FX3D_setChildConfig,   \
    }
#endif
