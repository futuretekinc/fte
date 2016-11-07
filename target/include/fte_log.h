#ifndef __FTE_LOG_H__
#define __FTE_LOG_H__

#include "fte_config.h"

typedef enum    FTE_LOG_TYPE_ENUM
{
    FTE_LOG_TYPE_SYSTEM = 0,
    FTE_LOG_TYPE_EVENT
}   FTE_LOG_TYPE, _PTR_ FTE_LOG_TYPE_PTR;

typedef enum    _FTE_LOG_SYSTEM_MESSAGE_ENUM
{
    FTE_LOG_SYSTEM_MESSAGE_BOOT = 0,
    FTE_LOG_SYSTEM_MESSAGE_END
}   FTE_LOG_SYSTEM_MESSAGE, _PTR_ FTE_LOG_SYSTEM_MESSAGE_PTR;

typedef struct  _FTE_LOG_SYSTEM_PARAM_STRUCT
{
    FTE_LOG_SYSTEM_MESSAGE  xMsg;    
}   FTE_LOG_SYSTEM_PARAM, _PTR_ FTE_LOG_SYSTEM_PARAM_PTR;

typedef struct  _FTE_LOG_EVENT_PARAM_STRUCT
{
    FTE_UINT32         ulEPID;
    FTE_UINT32         ulLevel;
    struct
    {
        FTE_UINT32         ulType;
        FTE_UINT32         ulValue;
    }   xValue;
}   FTE_LOG_EVENT_PARAM, _PTR_ FTE_LOG_EVENT_PARAM_PTR;

typedef struct _FTE_LOG_STRUCT
{
    FTE_LOG_TYPE    xType;
    TIME_STRUCT     xTimeStamp;    
    union 
    {
        FTE_LOG_SYSTEM_PARAM    xSystem;
        FTE_LOG_EVENT_PARAM     xEvent;
    }   xParam;
}   FTE_LOG, _PTR_ FTE_LOG_PTR;

FTE_RET     FTE_LOG_init(void);
FTE_RET     FTE_LOG_save(void);
FTE_RET     FTE_LOG_addSystem(FTE_LOG_SYSTEM_MESSAGE xMsg);
FTE_RET     FTE_LOG_addEvent(FTE_OBJECT_ID nID, FTE_UINT32 ulLevel, FTE_VALUE_PTR pValue);
FTE_UINT32  FTE_LOG_del(FTE_UINT32 ulCount);

FTE_LOG_PTR FTE_LOG_getAt(FTE_UINT32 nID);

FTE_INT32   FTE_LOG_count(void);

FTE_INT32   FTE_LOG_SHELL_cmd(FTE_INT32 argc, FTE_CHAR_PTR argv[]);
FTE_CHAR_PTR    FTE_LOG_getSystemMessageString(FTE_LOG_SYSTEM_MESSAGE xMsg);
#endif
