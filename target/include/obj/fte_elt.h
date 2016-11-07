#ifndef _FTE_ELT_H__
#define _FTE_ELT_H__

#ifndef FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL
#define FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL  10000
#endif

#ifndef FTE_ELT_AQM100_REPONSE_TIME         
#define FTE_ELT_AQM100_REPONSE_TIME         500
#endif
#ifndef FTE_ELT_AQM100_EVENT_TYPE_ABOVE     
#define FTE_ELT_AQM100_EVENT_TYPE_ABOVE     FTE_EVENT_CONDITION_ABOVE
#endif

#ifndef FTE_ELT_AQM100_EVENT_UPPER_LIMIT    
#define FTE_ELT_AQM100_EVENT_UPPER_LIMIT    1000
#endif

#ifndef FTE_ELT_AQM100_EVENT_LOWER_LIMIT    
#define FTE_ELT_AQM100_EVENT_LOWER_LIMIT    0
#endif

#ifndef FTE_ELT_AQM100_EVENT_THRESHOLD      
#define FTE_ELT_AQM100_EVENT_THRESHOLD      50
#endif

#ifndef FTE_ELT_AQM100_EVENT_DELAY          
#define FTE_ELT_AQM100_EVENT_DELAY          0
#endif

#define FTE_ELT_AQM100_DEFAULT_FULL_DUPLEX  FALSE
#define FTE_ELT_AQM100_DEFAULT_BAUDRATE     9600
#define FTE_ELT_AQM100_DEFAULT_DATABITS     8
#define FTE_ELT_AQM100_DEFAULT_PARITY       FTE_UART_PARITY_NONE
#define FTE_ELT_AQM100_DEFAULT_STOPBITS     FTE_UART_STOP_BITS_1

#define FTE_ELT_AQM100M_DEFAULT_FULL_DUPLEX FALSE
#define FTE_ELT_AQM100M_DEFAULT_BAUDRATE    9600
#define FTE_ELT_AQM100M_DEFAULT_DATABITS    8
#define FTE_ELT_AQM100M_DEFAULT_PARITY      FTE_UART_PARITY_NONE
#define FTE_ELT_AQM100M_DEFAULT_STOPBITS    FTE_UART_STOP_BITS_1

typedef enum    FTE_ELT_AQM100_FIELD_ENUM
{
    FTE_ELT_AQM100_FIELD_CO2    = 0,
    FTE_ELT_AQM100_FIELD_TEMP,
    FTE_ELT_AQM100_FIELD_HUMI,
    FTE_ELT_AQM100_FIELD_VOC,
    FTE_ELT_AQM100_FIELD_MAX
}   FTE_ELT_AQM100_FIELD, _PTR_ FTE_ELT_AQM100_FIELD_PTR;


typedef struct  FTE_ELT_CONFIG_STRUCT
{
    FTE_GUS_CONFIG  xGUS;
}   FTE_ELT_CONFIG, _PTR_ FTE_ELT_CONFIG_PTR;

typedef struct  FTE_ELT_STATUS_STRUCT
{
    FTE_GUS_STATUS  xGUS;
    FTE_UINT32      ulRetryCount;
    FTE_TIME        xLastRequestTime;
    FTE_TIME        xLastUpdateTime;
} FTE_ELT_STATUS, _PTR_ FTE_ELT_STATUS_PTR;

typedef struct  FTE_ELT_EXT_CONFIG_STRUCT
{
    FTE_UINT32      ulLoopPeriod;
    FTE_UINT32      ulUpdatePeriod;
    FTE_UINT32      ulRequestTimeout;
    FTE_UINT32      ulRetryCount;
}   FTE_ELT_EXT_CONFIG, _PTR_ FTE_ELT_EXT_CONFIG_PTR;

FTE_RET   FTE_ELT_init
(
    FTE_OBJECT_PTR pObj
);

FTE_RET FTE_ELT_create
(
    FTE_UINT32      xType,
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
);

FTE_RET FTE_ELT_destroy
(
    FTE_OBJECT_PTR pObj
);

FTE_RET   FTE_ELT_attach
(
    FTE_OBJECT_PTR pObj
);

FTE_RET   FTE_ELT_detach
(
    FTE_OBJECT_PTR pObj
);

FTE_RET FTE_ELT_get
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
);

FTE_RET FTE_ELT_AQM100_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
);

FTE_RET     FTE_ELT_AQM100_update
(
    FTE_OBJECT_PTR pObj
);

FTE_RET FTE_ELT_AQM100M_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
);

FTE_RET     FTE_ELT_AQM100M_update
(
    FTE_OBJECT_PTR pObj
);

void FTE_ELT_task
(
    FTE_UINT32     ulObjectID
);

FTE_INT32 FTE_ELT_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
);

FTE_RET FTE_ELT_AQM100_requestData
(
    FTE_OBJECT_PTR  pObj
);

FTE_RET FTE_ELT_AQM100_receiveData
(
    FTE_OBJECT_PTR  pObj
);

extern  
FTE_VALUE_TYPE      FTE_ELT_AQM100_valueTypes[];

extern  
FTE_ELT_CONFIG      FTE_ELT_AQM100_defaultConfig;

extern
FTE_ELT_CONFIG      FTE_ELT_AQM100M_defaultConfig;

extern  const 
FTE_GUS_MODEL_INFO  FTE_ELT_AQM100_GUSModelInfo;

extern  const 
FTE_GUS_MODEL_INFO  FTE_ELT_AQM100M_GUSModelInfo;

#endif