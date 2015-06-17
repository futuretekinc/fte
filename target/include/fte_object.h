#ifndef __FTE_OBJECT_H__
#define __FTE_OBJECT_H__

#include "fte_event.h"
#include "fte_value.h"
#include "fte_json.h"

#define MAX_OBJECT_NAME_LEN         16
#define MAX_CONFIG_SIZE             52
#define MAX_ACTION_SIZE             60

#define FTE_OBJ_SYSTEM              0x80000000

#define FTE_OBJ_CLASS_MASK          0x7F000000
#define FTE_OBJ_CLASS_INDEX         0x000000FF
#define FTE_OBJ_CLASS_SHIFT         24

#define FTE_OBJ_CLASS_TEMPERATURE   ((0x01) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_HUMIDITY      ((0x02) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_VOLTAGE       ((0x03) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_CURRENT       ((0x04) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_DI            ((0x05) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_DO            ((0x06) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_GAS           ((0x07) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_POWER         ((0x08) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_SRF           ((0x09) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_AI            ((0x0A) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_COUNT         ((0x0B) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_PRESSURE      ((0x0C) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_TEMP_CTRL     ((0x40) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_MULTI         ((0x7F) << FTE_OBJ_CLASS_SHIFT)

#define FTE_OBJ_TYPE_UNKNOWN        0x7FFFFFFF
#define FTE_OBJ_TYPE_MASK           0x7FFF0000
#define FTE_OBJ_TYPE_SHIFT          16

#define FTE_OBJ_TYPE_TEMPERATURE        ((FTE_OBJ_CLASS_TEMPERATURE ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_PT100              ((FTE_OBJ_CLASS_TEMPERATURE ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_DS18B20            ((FTE_OBJ_CLASS_TEMPERATURE ) | (0x02 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_TEMP         ((FTE_OBJ_CLASS_TEMPERATURE ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_HUMIDITY           ((FTE_OBJ_CLASS_HUMIDITY    ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_HUMI         ((FTE_OBJ_CLASS_HUMIDITY    ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_VOLTAGE            ((FTE_OBJ_CLASS_VOLTAGE     ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_GAI_VOLTAGE        ((FTE_OBJ_CLASS_VOLTAGE     ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_VOLTAGE      ((FTE_OBJ_CLASS_VOLTAGE     ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_CURRENT            ((FTE_OBJ_CLASS_CURRENT     ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_GAI_CURRENT        ((FTE_OBJ_CLASS_CURRENT     ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_CURRENT      ((FTE_OBJ_CLASS_CURRENT     ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_DI                 ((FTE_OBJ_CLASS_DI          ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_DI           ((FTE_OBJ_CLASS_DI          ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_DO                 ((FTE_OBJ_CLASS_DO          ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_RL                 ((FTE_OBJ_CLASS_DO          ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_LED                ((FTE_OBJ_CLASS_DO          ) | (0x02 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_DO           ((FTE_OBJ_CLASS_DO          ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_POWER              ((FTE_OBJ_CLASS_POWER       ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_POWER        ((FTE_OBJ_CLASS_POWER       ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_SRF                ((FTE_OBJ_CLASS_SRF         ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MB7092             ((FTE_OBJ_CLASS_SRF         ) | (0x01 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_AI                 ((FTE_OBJ_CLASS_AI          ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_FLEXIFORCE         ((FTE_OBJ_CLASS_AI          ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_DUST               ((FTE_OBJ_CLASS_AI          ) | (0x02 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_GAS                ((FTE_OBJ_CLASS_GAS         ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_CO2                ((FTE_OBJ_CLASS_GAS         ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_LPG                ((FTE_OBJ_CLASS_GAS         ) | (0x02 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_VOC                ((FTE_OBJ_CLASS_GAS         ) | (0x03 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_MULTI_CO2          ((FTE_OBJ_CLASS_GAS         ) | (0x81 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_LPG          ((FTE_OBJ_CLASS_GAS         ) | (0x82 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_VOC          ((FTE_OBJ_CLASS_GAS         ) | (0x83 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_COUNT              ((FTE_OBJ_CLASS_COUNT       ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_COUNT        ((FTE_OBJ_CLASS_COUNT       ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_PRESSURE           ((FTE_OBJ_CLASS_PRESSURE    ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_PRESSURE     ((FTE_OBJ_CLASS_PRESSURE    ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_TEMP_CTRL          ((FTE_OBJ_CLASS_TEMP_CTRL   ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_TEMP_CTRL    ((FTE_OBJ_CLASS_TEMP_CTRL   ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_MULTI              ((FTE_OBJ_CLASS_MULTI       ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_SHT          ((FTE_OBJ_CLASS_MULTI       ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_SH_MV250     ((FTE_OBJ_CLASS_MULTI       ) | (0x02 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_AQM100       ((FTE_OBJ_CLASS_MULTI       ) | (0x03 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_AIRQ         ((FTE_OBJ_CLASS_MULTI       ) | (0x04 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_MEX510C      ((FTE_OBJ_CLASS_MULTI       ) | (0x05 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_PN1500       ((FTE_OBJ_CLASS_MULTI       ) | (0x06 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_DPC_HL       ((FTE_OBJ_CLASS_MULTI       ) | (0x07 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_HEM12_06M    ((FTE_OBJ_CLASS_MULTI       ) | (0x08 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_ID_MASK                     0xFFFFFFFF
typedef uint_32 FTE_OBJECT_ID;

#define MAKE_ID(xType, nIndex)              ((uint_32)(((xType) & FTE_OBJ_TYPE_MASK) | ((nIndex) & 0x0000FFFF)))

#define MAKE_SYSTEM_ID(xType, nIndex)       (MAKE_ID(xType, nIndex) | FTE_OBJ_SYSTEM)
#define FTE_OBJ_CLASS(x)                    (((x)->pConfig->xCommon.nID) &  FTE_OBJ_CLASS_MASK)
#define FTE_OBJ_TYPE(x)                     (((x)->pConfig->xCommon.nID) &  FTE_OBJ_TYPE_MASK)
#define FTE_OBJ_INDEX(x)                    (((x)->pConfig->xCommon.nID) & ~FTE_OBJ_TYPE_MASK)
#define FTE_OBJ_ID(x)                       ((x)->pConfig->xCommon.nID)

#define FTE_ID_TYPE(x)                      ((x) &  FTE_OBJ_TYPE_MASK)
#define FTE_ID_INDEX(x)                     ((x) & ~FTE_OBJ_TYPE_MASK)

#define FTE_FLAG_IS_SET(flags, field)       (((flags) & (field)) == (field))
#define FTE_FLAG_IS_CLR(flags, field)       (((flags) & (field)) != (field))

#define FTE_FLAG_SET(flags, field)          ((flags) | (field))
#define FTE_FLAG_CLR(flags, field)          ((flags) & ~(field))

#define FTE_OBJ_CONFIG_FLAG_ENABLE          0x01
#define FTE_OBJ_CONFIG_FLAG_DYNAMIC         0x02
#define FTE_OBJ_CONFIG_FLAG_TRAP            0x10
#define FTE_OBJ_CONFIG_FLAG_TRAP_DIFF       0x30
#define FTE_OBJ_CONFIG_FLAG_TRAP_EQUAL      0x50
#define FTE_OBJ_CONFIG_FLAG_TRAP_CHANGED    0x70

#define FTE_OBJ_STATUS_FLAG_VALID           0x01
#define FTE_OBJ_STATUS_FLAG_WARN            0x02
#define FTE_OBJ_STATUS_FLAG_ALERT           0x04
#define FTE_OBJ_STATUS_FLAG_UPDATED         0x08
#define FTE_OBJ_STATUS_FLAG_CHANGED         0x10

#define FTE_OBJ_FIELD_ID                0x00000001
#define FTE_OBJ_FIELD_NAME              0x00000004
#define FTE_OBJ_FIELD_NAME_EDIT         0x00000008
#define FTE_OBJ_FIELD_VALUE             0x00000010
#define FTE_OBJ_FIELD_TIME              0x00000040
#define FTE_OBJ_FIELD_STATE             0x00000100
#define FTE_OBJ_FIELD_ENABLE            0x00000400
#define FTE_OBJ_FIELD_ENABLE_EDIT       0x00000800
#define FTE_OBJ_FIELD_SN                0x00001000
#define FTE_OBJ_FIELD_CTRL              0x00004000
#define FTE_OBJ_FIELD_INIT              0x00010000
#define FTE_OBJ_FIELD_INIT_EDIT         0x00020000
#define FTE_OBJ_FIELD_END               0x80000000


#define FTE_OBJ_FIELD_DEV_INFO          (FTE_OBJ_FIELD_ID |        \
                                        FTE_OBJ_FIELD_NAME |    \
                                        FTE_OBJ_FIELD_VALUE |   \
                                        FTE_OBJ_FIELD_TIME |    \
                                        FTE_OBJ_FIELD_STATE)

#define FTE_OBJ_FIELD_DEV_VALUE         (FTE_OBJ_FIELD_ID |     \
                                        FTE_OBJ_FIELD_VALUE |   \
                                        FTE_OBJ_FIELD_TIME |    \
                                        FTE_OBJ_FIELD_STATE)

#define FTE_OBJ_FIELD_EP_INFO           (FTE_OBJ_FIELD_ID |     \
                                        FTE_OBJ_FIELD_NAME |    \
                                        FTE_OBJ_FIELD_VALUE |   \
                                        FTE_OBJ_FIELD_TIME |    \
                                        FTE_OBJ_FIELD_STATE)

#define FTE_OBJ_FIELD_EP_VALUE          (FTE_OBJ_FIELD_ID |     \
                                        FTE_OBJ_FIELD_VALUE |   \
                                        FTE_OBJ_FIELD_TIME |    \
                                        FTE_OBJ_FIELD_STATE)

#define FTE_OBJ_FIELD_ALL               (0xFFFFFFFF)


#define FTE_OBJ_HAVE_SN                 0x01
#define FTE_OBJ_HAVE_CTRL               0x02

extern  const char_ptr FTE_JSON_MSG_TYPE_STRING;

extern  const char_ptr FTE_JSON_DEV_ID_STRING;
extern  const char_ptr FTE_JSON_DEV_TIME_STRING;
extern  const char_ptr FTE_JSON_DEV_EP_STRING;
extern  const char_ptr FTE_JSON_DEV_EPS_STRING;

extern  const char_ptr FTE_JSON_OBJ_ID_STRING;
extern  const char_ptr FTE_JSON_OBJ_NAME_STRING;
extern  const char_ptr FTE_JSON_OBJ_VALUE_STRING;
extern  const char_ptr FTE_JSON_OBJ_TIME_STRING;
extern  const char_ptr FTE_JSON_OBJ_STATE_STRING;
extern  const char_ptr FTE_JSON_OBJ_METHOD_STRING;
extern  const char_ptr FTE_JSON_OBJ_PARAM_STRING;

typedef struct _FTE_OBJECT_STATISTICS
{
    uint_32 nTotalTrial;
    uint_32 nTotalFail;
    uint_32 nPartialFail;
    uint_32 pStatBits[FTE_OBJ_CHECK_FAILURE_COUNT_MAX / sizeof(uint_32)];
}   FTE_OBJECT_STATISTICS, _PTR_ FTE_OBJECT_STATISTICS_PTR;

typedef struct  _FTE_OBJECT_STRUCT  _PTR_   FTE_OBJECT_PTR;

/*****************************************************************************
 * Sensor Object Structure Description 
 *****************************************************************************/
typedef struct   _FTE_OBJECT_COMMON_CONFIG_STRUCT
{
    uint_32     nID;
    char        pName[MAX_OBJECT_NAME_LEN+1];
    uint_32     xFlags;
    uint_32     ulChild;
    struct  _FTE_OBJECT_CONFIG_STRUCT  **pChild;
}   FTE_COMMON_CONFIG, _PTR_ FTE_COMMON_CONFIG_PTR;

typedef struct   _FTE_OBJECT_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             xReserved[6];
}   FTE_OBJECT_CONFIG, _PTR_ FTE_OBJECT_CONFIG_PTR;

typedef struct _fte_object_action_struct
{
    _mqx_uint       (*f_init)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_run)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_stop)(FTE_OBJECT_PTR pSelf);
    _mqx_uint       (*f_set)(FTE_OBJECT_PTR pSelf, FTE_VALUE_PTR pValue);
    _mqx_uint       (*f_set_multi)(FTE_OBJECT_PTR pSelf, uint_32 nIndex, FTE_VALUE_PTR pValue);
    _mqx_uint       (*f_get_sn)(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);
    uint_32         (*f_get_update_interval)(FTE_OBJECT_PTR pObj);
    _mqx_uint       (*f_set_update_interval)(FTE_OBJECT_PTR pObj, uint_32 nInterval);
    _mqx_uint       (*f_get_statistic)(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStat);
    _mqx_uint       (*f_attach_child)(FTE_OBJECT_PTR pSelf, uint_32 nChild);
    _mqx_uint       (*f_detach_child)(FTE_OBJECT_PTR pSelf, uint_32 nChild);
    _mqx_uint       (*f_set_config)(FTE_OBJECT_PTR pSelf, char_ptr pJSON);
    _mqx_uint       (*f_get_config)(FTE_OBJECT_PTR pSelf, char_ptr pBuff, uint_32 ulBuffLen);
    
}   FTE_OBJECT_ACTION, _PTR_ FTE_OBJECT_ACTION_PTR;

typedef struct _fte_object_status_struct
{
    uint_32                 xFlags;
    int_32                  nValueCount;
    FTE_VALUE_PTR           pValue;
    TIME_STRUCT             xEventTimeStamp;
    MQX_TICK_STRUCT         xStartTicks;            
    FTE_OBJECT_STATISTICS    xStatistics;
}   FTE_OBJECT_STATUS, _PTR_ FTE_OBJECT_STATUS_PTR;

typedef struct _fte_object_desc_struct
{
    uint_32         nType;
    char_ptr        pName;
    uint_32         nMaxCount;
    uint_32         xFlags;    
    uint_32         xSupportedFields;
    uint_32         nConfigSize;
    uint_32         nStatusSize;
    _mqx_uint       (*f_attach)(FTE_OBJECT_PTR pObj);
    _mqx_uint       (*f_detach)(FTE_OBJECT_PTR pObj);
    uint_32         (*f_printValue)(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);
}   FTE_OBJECT_DESC, _PTR_ FTE_OBJECT_DESC_PTR;


typedef struct  _FTE_OBJECT_STRUCT
{
    FTE_OBJECT_CONFIG_PTR       pConfig;
    FTE_OBJECT_ACTION_PTR       pAction;
    FTE_OBJECT_STATUS_PTR       pStatus;
    FTE_LIST                    xEventList;
}   FTE_OBJECT;

uint_32                 FTE_OBJ_DESC_count(void);
FTE_OBJECT_DESC_PTR     FTE_OBJ_DESC_get(uint_32 nType);
FTE_OBJECT_DESC_PTR     FTE_OBJ_DESC_getAt(uint_32 ulIndex);

uint_32         FTE_OBJ_DESC_CLASS_count(void);
uint_32         FTE_OBJ_DESC_CLASS_getAt(uint_32 ulIndex);

FTE_OBJECT_PTR  FTE_OBJ_create(FTE_OBJECT_CONFIG_PTR pConfig);
_mqx_uint       FTE_OBJ_destroy(FTE_OBJECT_PTR pObj);

FTE_OBJECT_PTR  FTE_OBJ_get(FTE_OBJECT_ID nID);
FTE_OBJECT_PTR  FTE_OBJ_getFirst(FTE_OBJECT_ID nID, uint_32 nMask, boolean bSystem);
FTE_OBJECT_PTR  FTE_OBJ_getNext(FTE_OBJECT_PTR  pObj, FTE_OBJECT_ID nID, uint_32 nMask, boolean bSystem);
FTE_OBJECT_PTR  FTE_OBJ_getAt(FTE_OBJECT_ID nID, uint_32 nMask, uint_32 nIndex, boolean bSystem);
uint_32         FTE_OBJ_count(FTE_OBJECT_ID nID, uint_32 nMask, boolean bSystem);
uint_32         FTE_OBJ_getList(FTE_OBJECT_ID nID, uint_32 nMask, FTE_OBJECT_PTR _PTR_ pObjList, uint_32 nMaxCount);

_mqx_uint       FTE_OBJ_getValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
_mqx_uint       FTE_OBJ_setValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
_mqx_uint       FTE_OBJ_setValueString(FTE_OBJECT_PTR pObj, char_ptr pValue);
_mqx_uint       FTE_OBJ_getValueAt(FTE_OBJECT_PTR pObj, uint_32 ulIdx, FTE_VALUE_PTR pValue);
FTE_VALUE_TYPE  FTE_OBJ_getValueType(FTE_OBJECT_PTR pObj);

char_ptr        FTE_OBJ_typeString(FTE_OBJECT_PTR pObj);

_mqx_uint       FTE_OBJ_getName(FTE_OBJECT_PTR pObj, char_ptr pName, uint_32 nBuffLen);
_mqx_uint       FTE_OBJ_setName(FTE_OBJECT_PTR pObj, char_ptr pName, uint_32 nLen);

_mqx_uint       FTE_OBJ_getSN(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);

_mqx_uint       FTE_OBJ_activate(FTE_OBJECT_PTR pObj, boolean bEnable);
_mqx_uint       FTE_OBJ_wasUpdated(FTE_OBJECT_PTR pObj);

_mqx_uint       FTE_OBJ_start(FTE_OBJECT_PTR pObj);
_mqx_uint       FTE_OBJ_stop(FTE_OBJECT_PTR pObj);

_mqx_uint       FTE_OBJ_save(FTE_OBJECT_PTR pObj);

uint_32         FTE_OBJ_runLoop(FTE_OBJECT_PTR pObj, TIMER_NOTIFICATION_TICK_FPTR f_callback, uint_32 nInterval);
uint_32         FTE_OBJ_runMeasurement(FTE_OBJECT_PTR pObj, TIMER_NOTIFICATION_TICK_FPTR f_callback, uint_32 nTimeout);

uint_32         FTE_OBJ_getFailureCount(FTE_OBJECT_PTR pObj);

boolean         FTE_OBJ_FLAG_isSet(FTE_OBJECT_PTR pObj, uint_32 xFlag);
_mqx_uint       FTE_OBJ_FLAG_set(FTE_OBJECT_PTR pObj, uint_32 xFlag);
_mqx_uint       FTE_OBJ_FLAG_clear(FTE_OBJECT_PTR pObj, uint_32 xFlag);

_mqx_uint       FTE_OBJ_EVENT_attach(FTE_OBJECT_PTR pObj, FTE_EVENT_PTR pEvent);
_mqx_uint       FTE_OBJ_EVENT_detach(FTE_OBJECT_PTR pObj, FTE_EVENT_PTR pEvent);

void            FTE_OBJ_STATE_set(FTE_OBJECT_PTR pObj, uint_32 xFlags);
void            FTE_OBJ_STATE_clear(FTE_OBJECT_PTR pObj, uint_32 xFlags);
boolean         FTE_OBJ_STATE_isSet(FTE_OBJECT_PTR pObj, uint_32 xFlags);

uint_32         FTE_OBJ_1WIRE_discovery(boolean bSave);

_mqx_uint       FT_OBJ_STAT_incSucceed(FTE_OBJECT_STATISTICS_PTR pStat);
_mqx_uint       FT_OBJ_STAT_incFailed(FTE_OBJECT_STATISTICS_PTR pStat);

FTE_JSON_OBJECT_PTR  FTE_OBJ_createJSON(FTE_OBJECT_PTR pObj, uint_32 xOptions);

int_32          FTE_OBJ_SHELL_cmd(int_32 argc, char_ptr argv[]);

#define FTE_OBJ_IS_ENABLED(pObj)     FTE_FLAG_IS_SET((pObj)->pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE)

#include "dev/fte_dev.h"

#include "obj/fte_event.h"
#include "obj/fte_di.h"
#include "obj/fte_do.h"
#include "obj/fte_rl.h"
#include "obj/fte_led.h"
#include "obj/fte_rtd.h"
#include "obj/fte_ds18b20.h"
#include "obj/fte_sht.h"
#include "obj/fte_srf.h"
#include "obj/fte_fis.h"
#include "obj/fte_gai.h"
#include "obj/fte_gus.h"
#include "obj/fte_multi.h"
#include "obj/fte_cozir.h"
#include "obj/fte_soha.h"
#include "obj/fte_elt.h"
#include "obj/fte_tascon.h"
#include "obj/fte_pm1001.h"
#include "obj/fte_trueyes.h"
#include "obj/fte_ifce.h"
#include "obj/fte_botem.h"
#include "obj/fte_mst_mex510c.h"
#include "obj/fte_gs_dpc.h"

#endif
