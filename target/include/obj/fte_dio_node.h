#ifndef _FTE_DIO_NODE_H__
#define _FTE_DIO_NODE_H__

#define FTE_DIO_NODE_DEV_MAX        1
#define FTE_DIO_NODE_DIO_MAX        7



#define FTE_DIO_NODE_DEFAULT_FULL_DUPLEX     TRUE
#define FTE_DIO_NODE_DEFAULT_BAUDRATE        38400
#define FTE_DIO_NODE_DEFAULT_DATABITS        8
#define FTE_DIO_NODE_DEFAULT_PARITY          FTE_UART_PARITY_NONE
#define FTE_DIO_NODE_DEFAULT_STOPBITS        FTE_UART_STOP_BITS_1

#define FTE_DIO_NODE_DEFAULT_UPDATE_INTERVAL 2000

#define FTE_DIO_NODE_FIELD_MAX           7


typedef struct  FTE_DIO_NODE_CONFIG_STRUCT
{
    FTE_GUS_CONFIG  xGUS;
}   FTE_DIO_NODE_CONFIG, _PTR_ FTE_DIO_NODE_CONFIG_PTR;

typedef struct  FTE_DIO_NODE_EXT_CONFIG_STRUCT
{
    uint_32         ulLoopPeriod;
    uint_32         ulUpdatePeriod;
    uint_32         ulRequestTimeout;
    uint_32         ulRetryCount;
}   FTE_DIO_NODE_EXT_CONFIG, _PTR_ FTE_DIO_NODE_EXT_CONFIG_PTR;

typedef struct FTE_DIO_NODE_DIO_STRUCT
{
    boolean     bValue;
	boolean     bType;
    TIME_STRUCT xVOT;           // valid operation time
}   FTE_DIO_NODE_DIO, _PTR_ FTE_DIO_NODE_DI_PTR;

typedef struct  FTE_DIO_NODE_STATUS_STRUCT
{
    FTE_GUS_STATUS  xGUS;
    _task_id        xTaskID;
    uint_32         ulRetryCount;
    TIME_STRUCT     xLastRequestTime;
    TIME_STRUCT     xLastUpdateTime;
    FTE_DIO_NODE_DIO     pDIO[FTE_DIO_NODE_DIO_MAX];
} FTE_DIO_NODE_STATUS, _PTR_ FTE_DIO_NODE_STATUS_PTR;

_mqx_uint FTE_DIO_NODE_init(FTE_OBJECT_PTR pObj);
_mqx_uint FTE_DIO_NODE_reset(void);

void FTE_DIO_NODE_task(uint_32 datas);

_mqx_uint FTE_DIO_NODE_initDefaultExtConfig(FTE_DIO_NODE_EXT_CONFIG_PTR pConfig);
_mqx_uint FTE_DIO_NODE_saveExtConfig(FTE_OBJECT_PTR pObj);
_mqx_uint FTE_DIO_NODE_loadExtConfig(FTE_OBJECT_PTR pObj);

int_32 FTE_DIO_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] );


_mqx_uint   FTE_DIO_NODE_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_DIO_NODE_detach(FTE_OBJECT_PTR pObj);
uint_32     FTE_DIO_NODE_get(FTE_OBJECT_PTR pObj, uint_32 ulIndex, FTE_VALUE_PTR pValue);
uint_32     FTE_DIO_NODE_set(FTE_OBJECT_PTR pObj, uint_32 ulIndex, FTE_VALUE_PTR pValue);


FTE_RET FTE_DIO_NODE_run
(
    FTE_OBJECT_PTR  pObj
);


FTE_RET FTE_DIO_NODE_stop
(
    FTE_OBJECT_PTR  pObj
);


FTE_RET FTE_DIO_NODE_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
);


extern FTE_VALUE_TYPE  FTE_DIO_NODE_valueTypes[];

extern const FTE_GUS_MODEL_INFO    FTE_DIO_NODE_GUSModelInfo;

#define FTE_DIO_NODE_DESCRIPTOR  {\
        .nModel         = FTE_GUS_MODEL_DIO_NODE,   \
        .pName          = "DIO_NODE",               \
        .nFieldCount    = FTE_DIO_NODE_DIO_MAX,      \
        .pValueTypes    = FTE_DIO_NODE_valueTypes,  \
        .f_attach       = FTE_DIO_NODE_attach,      \
        .f_detach       = FTE_DIO_NODE_detach,      \
        .f_get          = FTE_DIO_NODE_get,         \
		.f_set          = FTE_DIO_NODE_set,         \
    }
#endif