#ifndef _FTE_IOEX_H__
#define _FTE_IOEX_H__

#define FTE_IOEX_DEV_MAX        1
#define FTE_IOEX_DI_MAX         16

typedef struct  FTE_IOEX_CONFIG_STRUCT
{
    FTE_GUS_CONFIG  xGUS;
}   FTE_IOEX_CONFIG, _PTR_ FTE_IOEX_CONFIG_PTR;

typedef struct  FTE_IOEX_EXT_CONFIG_STRUCT
{
    uint_32         ulLoopPeriod;
    uint_32         ulUpdatePeriod;
    uint_32         ulRequestTimeout;
    uint_32         ulRetryCount;
}   FTE_IOEX_EXT_CONFIG, _PTR_ FTE_IOEX_EXT_CONFIG_PTR;

typedef struct FTE_IOEX_DI_STRUCT
{
    boolean     bValue;
    TIME_STRUCT xVOT;           // valid operation time
}   FTE_IOEX_DI, _PTR_ FTE_IOEX_DI_PTR;

typedef struct  FTE_IOEX_STATUS_STRUCT
{
    FTE_GUS_STATUS  xGUS;
    _task_id        xTaskID;
    uint_32         ulRetryCount;
    TIME_STRUCT     xLastRequestTime;
    TIME_STRUCT     xLastUpdateTime;
    FTE_IOEX_DI     pDI[FTE_IOEX_DI_MAX];
} FTE_IOEX_STATUS, _PTR_ FTE_IOEX_STATUS_PTR;

_mqx_uint FTE_IOEX_init(FTE_OBJECT_PTR pObj);
_mqx_uint FTE_IOEX_reset(void);

void FTE_IOEX_task(uint_32 datas);

_mqx_uint FTE_IOEX_initDefaultExtConfig(FTE_IOEX_EXT_CONFIG_PTR pConfig);
_mqx_uint FTE_IOEX_saveExtConfig(FTE_OBJECT_PTR pObj);
_mqx_uint FTE_IOEX_loadExtConfig(FTE_OBJECT_PTR pObj);

int_32 FTE_IOEX_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] );


_mqx_uint   FTE_IOEX_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_IOEX_detach(FTE_OBJECT_PTR pObj);
uint_32     FTE_IOEX_get(FTE_OBJECT_PTR pObj, uint_32 ulIndex, FTE_VALUE_PTR pValue);

extern FTE_VALUE_TYPE  FTE_IOEX_valueTypes[];

#define FTE_IOEX_DESCRIPTOR  {\
        .nModel         = FTE_GUS_MODEL_IOEX,   \
        .pName          = "IOEX",               \
        .nFieldCount    = FTE_IOEX_DI_MAX,      \
        .pValueTypes    = FTE_IOEX_valueTypes,  \
        .f_attach       = FTE_IOEX_attach,      \
        .f_detach       = FTE_IOEX_detach,      \
        .f_get          = FTE_IOEX_get,         \
    }
#endif