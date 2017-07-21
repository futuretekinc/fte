#ifndef _FTE_IOEX_H__
#define _FTE_IOEX_H__

#define FTE_IOEX_DEV_MAX        1
#define FTE_IOEX_DI_MAX         16

#define FTE_IOEX_DEFAULT_UPDATE_INTERVAL    10000
#define FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL 1000

typedef struct  FTE_IOEX_CONFIG_STRUCT
{
    FTE_GUS_CONFIG  xGUS;
}   FTE_IOEX_CONFIG, _PTR_ FTE_IOEX_CONFIG_PTR;

typedef struct  FTE_IOEX_EXT_CONFIG_STRUCT
{
    FTE_UINT32         ulLoopPeriod;
    FTE_UINT32         ulUpdatePeriod;
    FTE_UINT32         ulRequestTimeout;
    FTE_UINT32         ulRetryCount;
}   FTE_IOEX_EXT_CONFIG, _PTR_ FTE_IOEX_EXT_CONFIG_PTR;

typedef struct FTE_IOEX_DI_STRUCT
{
    FTE_BOOL    bValue;
    TIME_STRUCT xVOT;           // valid operation time
}   FTE_IOEX_DI, _PTR_ FTE_IOEX_DI_PTR;

typedef struct  FTE_IOEX_STATUS_STRUCT
{
    FTE_GUS_STATUS  xGUS;
    FTE_TASK_ID     xTaskID;
    FTE_UINT32      ulRetryCount;
    TIME_STRUCT     xLastRequestTime;
    TIME_STRUCT     xLastUpdateTime;
    FTE_IOEX_DI     pDI[FTE_IOEX_DI_MAX];
} FTE_IOEX_STATUS, _PTR_ FTE_IOEX_STATUS_PTR;

FTE_RET FTE_IOEX_init(FTE_OBJECT_PTR pObj);
FTE_RET FTE_IOEX_reset(void);

void FTE_IOEX_task(FTE_UINT32 datas);

FTE_RET FTE_IOEX_initDefaultExtConfig(FTE_IOEX_EXT_CONFIG_PTR pConfig);
FTE_RET FTE_IOEX_saveExtConfig(FTE_OBJECT_PTR pObj);
FTE_RET FTE_IOEX_loadExtConfig(FTE_OBJECT_PTR pObj);

FTE_INT32 FTE_IOEX_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );

FTE_RET     FTE_IOEX_create(FTE_CHAR_PTR    pSlaveID, FTE_OBJECT_PTR _PTR_ ppObj);
FTE_RET     FTE_IOEX_attach(FTE_OBJECT_PTR pObj);
FTE_RET     FTE_IOEX_detach(FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_IOEX_get(FTE_OBJECT_PTR pObj, FTE_UINT32 ulIndex, FTE_VALUE_PTR pValue);

extern  
FTE_GUS_CONFIG FTE_IOEX_defaultConfig;

extern  
FTE_VALUE_TYPE  FTE_IOEX_valueTypes[];

extern  const 
FTE_GUS_MODEL_INFO    FTE_IOEX_GUSModelInfo;

#endif