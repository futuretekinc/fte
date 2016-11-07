#ifndef _FTE_SOHA_H__
#define _FTE_SOHA_H__

#define FTE_SOHA_DEVICE_MAX                2

#ifndef FTE_SOHA_DEFAULT_LOOP_PERIOD
#define FTE_SOHA_DEFAULT_LOOP_PERIOD        2000 // ms
#endif

#ifndef FTE_SOHA_DEFAULT_UPDATE_INTERVAL
#define FTE_SOHA_DEFAULT_UPDATE_INTERVAL    10000 // ms
#endif

#ifndef FTE_SOHA_DEFAULT_REQUEST_TIMEOUT
#define FTE_SOHA_DEFAULT_REQUEST_TIMEOUT    500
#endif

#ifndef FTE_SOHA_DEFAULT_RETRY_COUNT
#define FTE_SOHA_DEFAULT_RETRY_COUNT        3
#endif

#define FTE_SOHA_MV250_REG_VALUE_START          0
#define FTE_SOHA_MV250_REG_VALUE_CO2            0
#define FTE_SOHA_MV250_REG_VALUE_TEMPERATURE    1
#define FTE_SOHA_MV250_REG_VALUE_HUMIDITY       2
#define FTE_SOHA_MV250_REG_VALUE_MAX            3

#define FTE_SOHA_MV250_REG_ADDR_START           100
#define FTE_SOHA_MV250_REG_ADDR_CO2             100
#define FTE_SOHA_MV250_REG_ADDR_TEMPERATURE     102
#define FTE_SOHA_MV250_REG_ADDR_HUMIDITY        104

#define FTE_SOHA_MV250_DEFAULT_FULL_DUPLEX  FALSE
#define FTE_SOHA_MV250_DEFAULT_BAUDRATE     9600
#define FTE_SOHA_MV250_DEFAULT_DATABITS     8
#define FTE_SOHA_MV250_DEFAULT_PARITY       FTE_UART_PARITY_NONE
#define FTE_SOHA_MV250_DEFAULT_STOPBITS     FTE_UART_STOP_BITS_1

typedef struct  FTE_SOHA_CONFIG_STRUCT
{
    FTE_GUS_CONFIG  xGUS;
}   FTE_SOHA_CONFIG, _PTR_ FTE_SOHA_CONFIG_PTR;

typedef struct  FTE_SOHA_EXT_CONFIG_STRUCT
{
    FTE_UINT32      ulLoopPeriod;
    FTE_UINT32      ulUpdatePeriod;
    FTE_UINT32      ulRequestTimeout;
    FTE_UINT32      ulRetryCount;
}   FTE_SOHA_EXT_CONFIG, _PTR_ FTE_SOHA_EXT_CONFIG_PTR;

typedef struct  FTE_SOHA_STATUS_STRUCT
{
    FTE_GUS_STATUS  xGUS;
    FTE_UINT32      ulRetryCount;
    FTE_TIME        xLastRequestTime;
    FTE_TIME        xLastUpdateTime;
} FTE_SOHA_STATUS, _PTR_ FTE_SOHA_STATUS_PTR;

FTE_RET     FTE_SOHA_init
(
    FTE_OBJECT_PTR pObj
);

void    FTE_SOHA_task
(
    FTE_UINT32 datas
);

FTE_INT32 FTE_SOHA_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
);

FTE_RET FTE_SOHA_MV250HT_create
(
    FTE_CHAR_PTR    pSlaveID, 
    FTE_OBJECT_PTR _PTR_ ppObj
);

extern  
FTE_VALUE_TYPE      FTE_SOHA_MV250_valueTypes[];

extern  
FTE_SOHA_CONFIG     FTE_SOHA_MV250HT_defaultConfig;

extern  const 
FTE_GUS_MODEL_INFO  FTE_SOHA_MV250_GUSModelInfo;


#endif