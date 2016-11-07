#ifndef __FTE_EVENT_H__
#define __FTE_EVENT_H__

#ifndef FTE_EVENT_REPORT_INTERVAL   
#define FTE_EVENT_REPORT_INTERVAL   10
#endif

typedef enum    _FTE_EVENT_CONDITION
{
    FTE_EVENT_CONDITION_NONE     = 0,
    FTE_EVENT_CONDITION_ABOVE    = 1,
    FTE_EVENT_CONDITION_BELOW    = 2,
    FTE_EVENT_CONDITION_INSIDE   = 3,
    FTE_EVENT_CONDITION_OUTSIDE  = 4,
    FTE_EVENT_CONDITION_INTERVAL = 5,
    FTE_EVENT_CONDITION_TIME     = 6,
    FTE_EVENT_CONDITION_CHANGED  = 7
}   FTE_EVENT_CONDITION, _PTR_ FTE_EVENT_CONDITION_PTR;

#define FTE_EVENT_TYPE_ENABLE       0x00000001
#define FTE_EVENT_TYPE_LOG          0x00000010
#define FTE_EVENT_TYPE_SNMP_TRAP    0x00000020
#define FTE_EVENT_TYPE_MQTT_PUB     0x00000040

typedef enum    
{
    FTE_EVENT_LEVEL_INFO        = 0,
    FTE_EVENT_LEVEL_WARNING     = 1,
    FTE_EVENT_LEVEL_ALERT       = 2,
    FTE_EVENT_LEVEL_CRITICAL    = 3
}   FTE_EVENT_LEVEL, _PTR_ FTE_EVENT_LEVEL_PTR;

typedef struct
{
    FTE_UINT32                 ulEPID;
    FTE_UINT32                 xType;
    FTE_EVENT_LEVEL         xLevel;
    FTE_EVENT_CONDITION     xCondition;
    struct
    {
        struct
        {
            FTE_INT32          nValue;
            FTE_UINT32         ulThreshold;
        }   xLimit;
        
        struct
        {
            FTE_INT32          nUpper;
            FTE_INT32          nLower;
            FTE_UINT32         ulThreshold;
        }   xRange;
        
        FTE_UINT32             ulInterval;
        
        TIME_STRUCT         xTime;
    }   xParams;
    
    uint_16                 ulDelayTime;
    uint_16                 ulHoldTime;
}   FTE_EVENT_CONFIG, _PTR_ FTE_CFG_EVENT_PTR;

typedef struct
{
    struct FTE_OBJECT_STRUCT _PTR_ pObj;
    uint_8                          bState;
    uint_8                          bStateChanged;
    uint_8                          bOccurred;    
    uint_8                          bChanged;
    TIME_STRUCT                     xTimeStamp;
    TIME_STRUCT                     xChangedTime;
    FTE_LIST                        xObjectList;
}   FTE_EVENT_STATE, _PTR_ FTE_EVENT_STATE_PTR;

typedef struct
{
    FTE_EVENT_STATE         xState;
    FTE_CFG_EVENT_PTR       pConfig;
}   FTE_EVENT, _PTR_ FTE_EVENT_PTR;

FTE_RET     FTE_EVENT_init(void);
FTE_RET     FTE_EVENT_create(FTE_CFG_EVENT_PTR pConfig, FTE_EVENT_PTR _PTR_ ppEvent);
FTE_RET     FTE_EVENT_destroy(FTE_EVENT_PTR pEvent);

FTE_RET     FTE_EVENT_count(FTE_UINT32_PTR pulCount);
FTE_RET     FTE_EVENT_getList(FTE_UINT32 xEPID, FTE_EVENT_PTR pEvents[], FTE_UINT32 ulMax, FTE_UINT32_PTR pulCount);
FTE_RET     FTE_EVENT_getAt(FTE_UINT32 ulIndex, FTE_EVENT_PTR _PTR_ ppEvent);

FTE_RET     FTE_EVENT_attachObject(FTE_EVENT_PTR pEvent, struct FTE_OBJECT_STRUCT _PTR_ pObj);
FTE_RET     FTE_EVENT_detachObject(FTE_EVENT_PTR pEvent, struct FTE_OBJECT_STRUCT _PTR_ pObj);
FTE_RET     FTE_EVENT_check(FTE_EVENT_PTR pEvent, struct FTE_OBJECT_STRUCT _PTR_ pObj);

FTE_RET     FTE_EVENT_isSatisfied(FTE_EVENT_PTR pEvent, FTE_INT32 nValue, FTE_BOOL_PTR bResult);
FTE_INT32   FTE_EVENT_shell_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);

FTE_RET     FTE_EVENT_type_string(FTE_UINT32 xType, FTE_CHAR_PTR pBuff, FTE_INT32 nBuffLen);
FTE_RET     FTE_EVENT_condition_string(FTE_EVENT_CONDITION xCondition, FTE_CHAR_PTR pBuff, FTE_INT32 nBuffLen);
FTE_RET     FTE_EVENT_level_string(FTE_EVENT_LEVEL  xLevel, FTE_CHAR_PTR pBuff, FTE_INT32 nBuffLen);

FTE_CHAR_PTR    FTE_EVENT_CONDITION_string(FTE_UINT32 ulType);

void        FTE_EVENT_task(FTE_UINT32 params);

#endif