#ifndef _FTE_CIAS_H__
#define _FTE_CIAS_H__

#define FTE_CIAS_SIOUX_CU_MAX       1
#define FTE_CIAS_SIOUX_CU_ALARM_MAX 8
#define FTE_CIAS_SIOUX_CU_ZONE_MAX  20

#define FTE_CIAS_SIOUX_CU_DEFAULT_DISTANCE      50
#define FTE_CIAS_SIOUX_CU_DEFAULT_SNESOR_COUNT  7

typedef struct FTE_CIAS_ALARM_STRUCT
{
    uint_32     ulValue;
    TIME_STRUCT xVOT;           // valid operation time
}   FTE_CIAS_ALARM, _PTR_ FTE_CIAS_ALARM_PTR;

typedef struct FTE_CIAS_ZONE_STRUCT
{
    uint_8      nDeviceNumber;
    boolean     bInOperation;
    uint_8      bSensorCount;
    uint_32     ulAlarm;
    uint_32     ulValue;
    TIME_STRUCT xVOT;           // valid operation time
}   FTE_CIAS_ZONE, _PTR_ FTE_CIAS_ZONE_PTR;

typedef struct  FTE_CIAS_SIOUX_CU_CONFIG_STRUCT
{
    FTE_GUS_CONFIG  xGUS;
}   FTE_CIAS_SIOUX_CU_CONFIG, _PTR_ FTE_CIAS_SIOUX_CU_CONFIG_PTR;

typedef struct  FTE_CIAS_SIOUX_CU_STATUS_STRUCT
{
    FTE_GUS_STATUS  xGUS;
} FTE_CIAS_SIOUX_CU_STATUS, _PTR_ FTE_CIAS_SIOUX_CU_STATUS_PTR;

typedef struct  FTE_CIAS_SIOUX_CU_STRUCT
{
    _task_id        xTaskID;
    uint_32         ulObjectID;
    uint_32         ulSensorCount;
    uint_32         ulDistance;    
    FTE_CIAS_ALARM  pAlarms[8];
    FTE_CIAS_ZONE   pZones[FTE_CIAS_SIOUX_CU_ZONE_MAX];
}   FTE_CIAS_SIOUX_CU, _PTR_ FTE_CIAS_SIOUX_CU_PTR;

typedef struct  FTE_CIAS_SIOUX_CU_TASK_PARAM_STRUCT
{
    uint_32     ulUCSDevID;
}   FTE_CIAS_SIOUX_CU_TASK_PARAM, _PTR_ FTE_CIAS_SIOUX_CU_TASK_PARAM_PTR;

typedef struct  FTE_CIAS_SIOUX_CU_ZONE_CONFIG_STRUCT
{
    uint_8      nDeviceNumber;
    uint_8      bActivation;
}   FTE_CIAS_SIOUX_CU_ZONE_CONFIG, _PTR_ FTE_CIAS_SIOUX_CU_ZONE_CONFIG_PTR;

typedef struct  FTE_CIAS_SIOUX_CU_EXT_CONFIG_STRUCT
{
    
    uint_32     ulDistance;
    uint_32     ulSensorCount;
    FTE_CIAS_SIOUX_CU_ZONE_CONFIG   pZones[FTE_CIAS_SIOUX_CU_ZONE_MAX];
}   FTE_CIAS_SIOUX_CU_EXT_CONFIG, _PTR_ FTE_CIAS_SIOUX_CU_EXT_CONFIG_PTR;

void        FTE_CIAS_SIOUX_CU_init(uint_32 ulObjectID);
_mqx_uint   FTE_CIAS_SIOUX_CU_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_CIAS_SIOUX_CU_detach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_CIAS_SIOUX_CU_setConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff);
_mqx_uint   FTE_CIAS_SIOUX_CU_getConfig(FTE_OBJECT_PTR pObject, char_ptr pBuff, uint_32 ulBuffLen);

_mqx_uint   FTE_CIAS_SIOUX_CU_initDefaultExtConfig(FTE_CIAS_SIOUX_CU_EXT_CONFIG_PTR pConfig);

void        FTE_CIAS_SIOUX_CU_task(uint_32 datas);
int_32      FTE_CIAS_SIOUX_CU_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] );

uint_32     FTE_CIAS_SIOUX_CU_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_CIAS_SIOUX_CU_received(FTE_OBJECT_PTR pObj);
uint_32     FTE_CIAS_SIOUX_CU_get(FTE_OBJECT_PTR pObject, uint_32 ulIndex, FTE_VALUE_PTR pValue);

extern FTE_VALUE_TYPE  FTE_CIAS_SIOUX_CU_valueTypes[];

#define FTE_CIAS_SIOUX_CU_SENS  {\
        .nModel         = FTE_GUS_MODEL_CIAS_SIOUX_CU,  \
        .pName          = "CIAS_SIOUX_CU",              \
        .nFieldCount    = FTE_CIAS_SIOUX_CU_ALARM_MAX + FTE_CIAS_SIOUX_CU_ZONE_MAX, \
        .pValueTypes    = FTE_CIAS_SIOUX_CU_valueTypes, \
        .f_attach       = FTE_CIAS_SIOUX_CU_attach,     \
        .f_detach       = FTE_CIAS_SIOUX_CU_detach,     \
        .f_get          = FTE_CIAS_SIOUX_CU_get,        \
        .f_request      = FTE_CIAS_SIOUX_CU_request,    \
        .f_received     = FTE_CIAS_SIOUX_CU_received,   \
        .f_set_config   = FTE_CIAS_SIOUX_CU_setConfig,  \
        .f_get_config   = FTE_CIAS_SIOUX_CU_getConfig,  \
    }
#endif