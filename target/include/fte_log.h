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
    uint_32         ulEPID;
    uint_32         ulLevel;
    struct
    {
        uint_32         ulType;
        uint_32         ulValue;
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

_mqx_uint   FTE_LOG_init(void);
_mqx_uint   FTE_LOG_save(void);
_mqx_uint   FTE_LOG_addSystem(FTE_LOG_SYSTEM_MESSAGE xMsg);
_mqx_uint   FTE_LOG_addEvent(FTE_OBJECT_ID nID, uint_32 ulLevel, FTE_VALUE_PTR pValue);
uint_32     FTE_LOG_del(uint_32 ulCount);

FTE_LOG_PTR FTE_LOG_getAt(uint_32 nID);

int         FTE_LOG_count(void);

int_32      FTE_LOG_SHELL_cmd(int_32 argc, char_ptr argv[]);
char_ptr    FTE_LOG_getSystemMessageString(FTE_LOG_SYSTEM_MESSAGE xMsg);
#endif
