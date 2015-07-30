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
    uint_32                 ulEPID;
    uint_32                 xType;
    FTE_EVENT_LEVEL         xLevel;
    FTE_EVENT_CONDITION     xCondition;
    struct
    {
        struct
        {
            int_32          nValue;
            uint_32         ulThreshold;
        }   xLimit;
        
        struct
        {
            int_32          nUpper;
            int_32          nLower;
            uint_32         ulThreshold;
        }   xRange;
        
        uint_32             ulInterval;
        
        TIME_STRUCT         xTime;
    }   xParams;
    
    uint_16                 ulDelayTime;
    uint_16                 ulHoldTime;
}   FTE_EVENT_CONFIG, _PTR_ FTE_CFG_EVENT_PTR;

typedef struct
{
    struct _FTE_OBJECT_STRUCT _PTR_ pObj;
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

_mqx_uint       FTE_EVENT_init(void);
_mqx_uint       FTE_EVENT_create(FTE_CFG_EVENT_PTR pConfig, FTE_EVENT_PTR _PTR_ ppEvent);
_mqx_uint       FTE_EVENT_destroy(FTE_EVENT_PTR pEvent);

_mqx_uint       FTE_EVENT_count(uint_32_ptr pulCount);
FTE_EVENT_PTR   FTE_EVENT_getAt(uint_32 ulIndex);

_mqx_uint       FTE_EVENT_attachObject(FTE_EVENT_PTR pEvent, struct _FTE_OBJECT_STRUCT _PTR_ pObj);
_mqx_uint       FTE_EVENT_detachObject(FTE_EVENT_PTR pEvent, struct _FTE_OBJECT_STRUCT _PTR_ pObj);
_mqx_uint       FTE_EVENT_check(FTE_EVENT_PTR pEvent, struct _FTE_OBJECT_STRUCT _PTR_ pObj);

_mqx_uint       FTE_EVENT_isSatisfied(FTE_EVENT_PTR pEvent, int_32 nValue, boolean *bResult);
int_32          FTE_EVENT_shell_cmd(int_32 nArgc, char_ptr pArgv[]);

_mqx_uint       FTE_EVENT_type_string(uint_32 xType, char_ptr pBuff, int_32 nBuffLen);
_mqx_uint       FTE_EVENT_condition_string(FTE_EVENT_CONDITION xCondition, char_ptr pBuff, int_32 nBuffLen);
_mqx_uint       FTE_EVENT_level_string(FTE_EVENT_LEVEL  xLevel, char_ptr pBuff, int_32 nBuffLen);

char_ptr        FTE_EVENT_CONDITION_string(uint_32 ulType);

void            FTE_EVENT_task(uint_32 params);

#endif