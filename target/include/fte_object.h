#ifndef __FTE_OBJECT_H__
#define __FTE_OBJECT_H__

#include "fte_value.h"
#include "fte_json.h"
#include "fte_time.h"
#include "fte_list.h"
#include "fte_event.h"

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
#define FTE_OBJ_CLASS_DISCRETE      ((0x0D) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_TEMP_CTRL     ((0x40) << FTE_OBJ_CLASS_SHIFT)

#define FTE_OBJ_CLASS_MULTI         ((0x7E) << FTE_OBJ_CLASS_SHIFT)
#define FTE_OBJ_CLASS_CTRL          ((0x7F) << FTE_OBJ_CLASS_SHIFT)

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

#define FTE_OBJ_TYPE_MULTI_AI           ((FTE_OBJ_CLASS_AI          ) | (0x81 << FTE_OBJ_TYPE_SHIFT))

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

#define FTE_OBJ_TYPE_DISCRETE           ((FTE_OBJ_CLASS_DISCRETE   ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_DISCRETE     ((FTE_OBJ_CLASS_DISCRETE   ) | (0x80 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_CIAS_ALARM   ((FTE_OBJ_CLASS_DISCRETE   ) | (0x81 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_CIAS_ZONE    ((FTE_OBJ_CLASS_DISCRETE   ) | (0x82 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_TEMP_CTRL          ((FTE_OBJ_CLASS_TEMP_CTRL   ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_TEMP_CTRL    ((FTE_OBJ_CLASS_TEMP_CTRL   ) | (0x80 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_CTRL               ((FTE_OBJ_CLASS_CTRL)         | (0x00 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_TYPE_MULTI              ((FTE_OBJ_CLASS_MULTI       ) | (0x00 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_SHT          ((FTE_OBJ_CLASS_MULTI       ) | (0x01 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_SH_MV250     ((FTE_OBJ_CLASS_MULTI       ) | (0x02 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_AQM100       ((FTE_OBJ_CLASS_MULTI       ) | (0x03 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_AIRQ         ((FTE_OBJ_CLASS_MULTI       ) | (0x04 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_MEX510C      ((FTE_OBJ_CLASS_MULTI       ) | (0x05 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_PN1500       ((FTE_OBJ_CLASS_MULTI       ) | (0x06 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_DPC_HL       ((FTE_OBJ_CLASS_MULTI       ) | (0x07 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_HEM12_06M    ((FTE_OBJ_CLASS_MULTI       ) | (0x08 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_FTLM         ((FTE_OBJ_CLASS_MULTI       ) | (0x09 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_CIAS_SIOUX_CU ((FTE_OBJ_CLASS_MULTI      ) | (0x0A << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_IOEX         ((FTE_OBJ_CLASS_MULTI      )  | (0x0B << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_HEM12        ((FTE_OBJ_CLASS_MULTI       ) | (0x0C << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_DOTECH_FX3D  ((FTE_OBJ_CLASS_MULTI      )  | (0x0D << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_AQM100M      ((FTE_OBJ_CLASS_MULTI       ) | (0x0E << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_DIO_NODE     ((FTE_OBJ_CLASS_MULTI       ) | (0x0F << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20       ((FTE_OBJ_CLASS_MULTI      )  | (0x10 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_FUTURETEK_MS10       ((FTE_OBJ_CLASS_MULTI      )  | (0x11 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_FUTURETEK_KM21B20    ((FTE_OBJ_CLASS_MULTI      )  | (0x12 << FTE_OBJ_TYPE_SHIFT))
#define FTE_OBJ_TYPE_MULTI_FUTURETEK_JCG06      ((FTE_OBJ_CLASS_MULTI      )  | (0x13 << FTE_OBJ_TYPE_SHIFT))

#define FTE_OBJ_ID_MASK                     0xFFFFFFFF
typedef FTE_UINT32 FTE_OBJECT_ID, _PTR_ FTE_OBJECT_ID_PTR;

#define MAKE_ID(xType, nIndex)              ((FTE_UINT32)(((xType) & FTE_OBJ_TYPE_MASK) | ((nIndex) & 0x0000FFFF)))

#define FTE_ID_TYPE(x)                      ((x) &  FTE_OBJ_TYPE_MASK)
#define FTE_ID_INDEX(x)                     ((x) & ~FTE_OBJ_TYPE_MASK)

#define FTE_FLAG_IS_SET(flags, field)       (((flags) & (field)) == (field))
#define FTE_FLAG_IS_CLR(flags, field)       (((flags) & (field)) != (field))

#define FTE_FLAG_SET(flags, field)          ((flags) | (field))
#define FTE_FLAG_CLR(flags, field)          ((flags) & ~(field))

#define MAKE_SYSTEM_ID(xType, nIndex)       (MAKE_ID(xType, nIndex) | FTE_OBJ_SYSTEM)
#define FTE_OBJ_CLASS(x)                    (((x)->pConfig->xCommon.nID) &  FTE_OBJ_CLASS_MASK)
#define FTE_OBJ_TYPE(x)                     (((x)->pConfig->xCommon.nID) &  FTE_OBJ_TYPE_MASK)
#define FTE_OBJ_INDEX(x)                    (((x)->pConfig->xCommon.nID) & ~FTE_OBJ_TYPE_MASK)
#define FTE_OBJ_ID(x)                       ((x)->pConfig->xCommon.nID)

#define FTE_OBJ_IS_CHILD(x)                 (((((x)->pConfig->xCommon.nID) >> 16) & 0x80) == 0x80)
#define FTE_OBJ_IS_RUN(x)                   (FTE_FLAG_IS_SET((x)->pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))

#define FTE_OBJ_CONFIG_FLAG_DISABLE         0x00000000
#define FTE_OBJ_CONFIG_FLAG_ENABLE          0x00000001
#define FTE_OBJ_CONFIG_FLAG_DYNAMIC         0x00000002
#define FTE_OBJ_CONFIG_FLAG_TRAP            0x00000010
#define FTE_OBJ_CONFIG_FLAG_TRAP_DIFF       0x00000030
#define FTE_OBJ_CONFIG_FLAG_TRAP_EQUAL      0x00000050
#define FTE_OBJ_CONFIG_FLAG_TRAP_CHANGED    0x00000070
#define FTE_OBJ_CONFIG_FLAG_REVERSE         0x00000100
#define FTE_OBJ_CONFIG_FLAG_SYNC            0x00000200

#define FTE_OBJ_STATUS_FLAG_VALID           0x00000001
#define FTE_OBJ_STATUS_FLAG_WARN            0x00000002
#define FTE_OBJ_STATUS_FLAG_ALERT           0x00000004
#define FTE_OBJ_STATUS_FLAG_UPDATED         0x00000008
#define FTE_OBJ_STATUS_FLAG_CHANGED         0x00000010

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
#define FTE_OBJ_FIELD_DID               0x01000000
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


#define FTE_OBJ_FLAG_SN                 (1 << 0)
#define FTE_OBJ_FLAG_CTRL               (1 << 1)
#define FTE_OBJ_FLAG_GUS                (1 << 2)
#define FTE_OBJ_FLAG_DYNAMIC            (1 << 3)

extern  const FTE_CHAR_PTR FTE_JSON_MSG_TYPE_STRING;

extern  const FTE_CHAR_PTR FTE_JSON_DEV_ID_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_DEV_TIME_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_DEV_EP_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_DEV_EPS_STRING;

extern  const FTE_CHAR_PTR FTE_JSON_OBJ_ID_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_OBJ_NAME_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_OBJ_VALUE_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_OBJ_TIME_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_OBJ_STATE_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_OBJ_METHOD_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_OBJ_PARAM_STRING;
extern  const FTE_CHAR_PTR FTE_JSON_OBJ_PARAMS_STRING;

typedef struct FTE_OBJECT_STATISTICS
{
    FTE_UINT32 nTotalTrial;
    FTE_UINT32 nTotalFail;
    FTE_UINT32 nPartialFail;
    FTE_UINT32 pStatBits[FTE_OBJ_CHECK_FAILURE_COUNT_MAX / sizeof(FTE_UINT32)];
}   FTE_OBJECT_STATISTICS, _PTR_ FTE_OBJECT_STATISTICS_PTR;

typedef struct  FTE_OBJECT_STRUCT  _PTR_   FTE_OBJECT_PTR;

/*****************************************************************************
 * Sensor Object Structure Description 
 *****************************************************************************/
typedef struct   FTE_OBJECT_COMMON_CONFIG_STRUCT
{
    FTE_UINT32  nID;
    FTE_CHAR    pName[MAX_OBJECT_NAME_LEN+1];
    FTE_UINT32  xFlags;
    FTE_UINT32  ulChild;
    struct  FTE_OBJECT_CONFIG_STRUCT  _PTR_ _PTR_ pChild;
}   FTE_COMMON_CONFIG, _PTR_ FTE_COMMON_CONFIG_PTR;

typedef struct   FTE_OBJECT_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          xReserved[6];
}   FTE_OBJECT_CONFIG, _PTR_ FTE_OBJECT_CONFIG_PTR;

typedef struct FTE_OBJECT_ACTION_STRUCT
{
    FTE_RET     (*fInit)(FTE_OBJECT_PTR pSelf);
    FTE_RET     (*fRun)(FTE_OBJECT_PTR pSelf);
    FTE_RET     (*fStop)(FTE_OBJECT_PTR pSelf);
    FTE_RET     (*fSet)(FTE_OBJECT_PTR pSelf, FTE_VALUE_PTR pValue);
    FTE_RET     (*fGet)(FTE_OBJECT_PTR pSelf, FTE_VALUE_PTR pValue);
    FTE_RET     (*fSetMulti)(FTE_OBJECT_PTR pSelf, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);
    FTE_RET     (*fGetMulti)(FTE_OBJECT_PTR pSelf, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);
    FTE_RET     (*fGetSN)(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);
    FTE_RET     (*fGetInterval)(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pInterval);
    FTE_RET     (*fSetInterval)(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);
    FTE_RET     (*fGetStatistics)(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStat);
    FTE_RET     (*fAttachChild)(FTE_OBJECT_PTR pSelf, FTE_UINT32 nChild);
    FTE_RET     (*fDetachChild)(FTE_OBJECT_PTR pSelf, FTE_UINT32 nChild);
    FTE_RET     (*fGetChildCount)(FTE_OBJECT_PTR pSelf, FTE_UINT32_PTR pulCount);
    FTE_RET     (*fGetChild)(FTE_OBJECT_PTR pSelf, FTE_UINT32 ulIndex, FTE_OBJECT_ID _PTR_ pxID);
    FTE_RET     (*fSetConfig)(FTE_OBJECT_PTR pSelf, FTE_CHAR_PTR pJSON);
    FTE_RET     (*fGetConfig)(FTE_OBJECT_PTR pSelf, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);
    FTE_RET     (*fSetChildConfig)(FTE_OBJECT_PTR pSelf, FTE_OBJECT_PTR pChild, FTE_CHAR_PTR pJSON);
    FTE_RET     (*fGetChildConfig)(FTE_OBJECT_PTR pSelf, FTE_OBJECT_PTR pChild, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);
    FTE_RET     (*fCreateJSON)(FTE_OBJECT_PTR pSelf, FTE_UINT32  xOption, FTE_JSON_OBJECT_PTR _PTR_ ppJSON);
}   FTE_OBJECT_ACTION, _PTR_ FTE_OBJECT_ACTION_PTR;

typedef struct FTE_OBJECT_STATUS_STRUCT
{
    FTE_UINT32              xFlags;
    FTE_INT32               nValueCount;
    FTE_VALUE_PTR           pValue;
    TIME_STRUCT             xEventTimeStamp;
    MQX_TICK_STRUCT         xStartTicks;            
    FTE_OBJECT_STATISTICS   xStatistics;
}   FTE_OBJECT_STATUS, _PTR_ FTE_OBJECT_STATUS_PTR;

typedef struct FTE_OBJECT_STRUCT
{
    FTE_OBJECT_CONFIG_PTR       pConfig;
    FTE_OBJECT_ACTION_PTR       pAction;
    FTE_OBJECT_STATUS_PTR       pStatus;
    FTE_LIST                    xEventList;
}   FTE_OBJECT;

typedef struct FTE_OBJECT_DESC_STRUCT
{
    FTE_UINT32      nType;
    FTE_CHAR_PTR    pName;
    FTE_CHAR_PTR    pVendor;
    FTE_UINT32      xFlags;    
    FTE_UINT32      xSupportedFields;
    FTE_UINT32      nConfigSize;
    FTE_UINT32      nStatusSize;
    FTE_RET         (*f_create)(FTE_OBJECT_CONFIG_PTR pConfig, FTE_OBJECT_PTR _PTR_ ppObj);
    FTE_RET         (*f_attach)(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
    FTE_RET         (*f_detach)(FTE_OBJECT_PTR pObj);
    FTE_UINT32      (*f_printValue)(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);
    FTE_VOID_PTR    pOpts;
}   FTE_OBJECT_DESC, _PTR_ FTE_OBJECT_DESC_PTR;


FTE_UINT32          FTE_OBJ_DESC_count(void);
FTE_OBJECT_DESC_PTR FTE_OBJ_DESC_get(FTE_UINT32 nType);
FTE_OBJECT_DESC_PTR FTE_OBJ_DESC_getAt(FTE_UINT32 ulIndex);

FTE_UINT32          FTE_OBJ_DESC_CLASS_count(void);
FTE_UINT32          FTE_OBJ_DESC_CLASS_getAt(FTE_UINT32 ulIndex);

FTE_RET             FTE_OBJ_CLASS_getName(FTE_OBJECT_ID nID, FTE_CHAR_PTR pName, FTE_UINT32 nBuffLen);

FTE_OBJECT_PTR      FTE_OBJ_create(FTE_OBJECT_CONFIG_PTR pConfig);
FTE_RET             FTE_OBJ_destroy(FTE_OBJECT_PTR pObj);

FTE_RET             FTE_OBJ_init(FTE_OBJECT_PTR pObj);

FTE_OBJECT_PTR      FTE_OBJ_get(FTE_OBJECT_ID nID);
FTE_OBJECT_PTR      FTE_OBJ_get2(FTE_OBJECT_ID nID, FTE_UINT32 nMask);
FTE_OBJECT_PTR      FTE_OBJ_getFirst(FTE_OBJECT_ID nID, FTE_UINT32 nMask, FTE_BOOL bSystem);
FTE_OBJECT_PTR      FTE_OBJ_getNext(FTE_OBJECT_PTR  pObj, FTE_OBJECT_ID nID, FTE_UINT32 nMask, FTE_BOOL bSystem);
FTE_OBJECT_PTR      FTE_OBJ_getAt(FTE_OBJECT_ID nID, FTE_UINT32 nMask, FTE_UINT32 nIndex, FTE_BOOL bSystem);
FTE_UINT32          FTE_OBJ_count(FTE_OBJECT_ID nID, FTE_UINT32 nMask, FTE_BOOL bSystem);
FTE_UINT32          FTE_OBJ_getList(FTE_OBJECT_ID nID, FTE_UINT32 nMask, FTE_OBJECT_PTR _PTR_ pObjList, FTE_UINT32 nMaxCount);
FTE_RET FTE_OBJ_getIDList(FTE_OBJECT_ID nID, FTE_UINT32 nMask, FTE_OBJECT_ID_PTR pObjIDs, FTE_UINT32 nMaxCount, FTE_UINT32_PTR pCount);

FTE_RET FTE_OBJ_getValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
FTE_RET FTE_OBJ_setValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
FTE_RET FTE_OBJ_setValueAt(FTE_OBJECT_PTR pObj, FTE_UINT32 ulIndex, FTE_VALUE_PTR pValue);
FTE_RET FTE_OBJ_setValueString(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pValue);
FTE_RET FTE_OBJ_getValueAt(FTE_OBJECT_PTR pObj, FTE_UINT32 ulIdx, FTE_VALUE_PTR pValue);
FTE_VALUE_TYPE  FTE_OBJ_getValueType(FTE_OBJECT_PTR pObj);

FTE_RET FTE_OBJ_getConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);
FTE_RET FTE_OBJ_setConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pJSON);

FTE_CHAR_PTR        FTE_OBJ_typeString(FTE_OBJECT_PTR pObj);

FTE_RET FTE_OBJ_getName(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pName, FTE_UINT32 nBuffLen);
FTE_RET FTE_OBJ_setName(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pName, FTE_UINT32 nLen);

FTE_RET FTE_OBJ_getSN(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);

FTE_RET FTE_OBJ_getInterval(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pulInterval);
FTE_RET FTE_OBJ_setInterval(FTE_OBJECT_PTR pObj, FTE_UINT32 ulInterval);

FTE_RET FTE_OBJ_getStatistics(FTE_OBJECT_PTR  pObj, FTE_OBJECT_STATISTICS_PTR pStatistics);

FTE_RET FTE_OBJ_activate(FTE_OBJECT_PTR pObj, FTE_BOOL bEnable);
FTE_RET FTE_OBJ_wasUpdated(FTE_OBJECT_PTR pObj);
FTE_RET FTE_OBJ_wasChanged(FTE_OBJECT_PTR pObj);

FTE_RET FTE_OBJ_start(FTE_OBJECT_PTR pObj);
FTE_RET FTE_OBJ_stop(FTE_OBJECT_PTR pObj);

FTE_RET FTE_OBJ_save(FTE_OBJECT_PTR pObj);

FTE_RET FTE_OBJ_runLoop(FTE_OBJECT_PTR pObj, TIMER_NOTIFICATION_TICK_FPTR f_callback, FTE_UINT32 nInterval, FTE_TIMER_ID_PTR pTimerID);
FTE_RET FTE_OBJ_runMeasurement(FTE_OBJECT_PTR pObj, TIMER_NOTIFICATION_TICK_FPTR f_callback, FTE_UINT32 nTimeout, FTE_TIMER_ID_PTR pTimerID);

FTE_UINT32      FTE_OBJ_getFailureCount(FTE_OBJECT_PTR pObj);

FTE_BOOL        FTE_OBJ_FLAG_isSet(FTE_OBJECT_PTR pObj, FTE_UINT32 xFlag);
FTE_RET FTE_OBJ_FLAG_set(FTE_OBJECT_PTR pObj, FTE_UINT32 xFlag);
FTE_RET FTE_OBJ_FLAG_clear(FTE_OBJECT_PTR pObj, FTE_UINT32 xFlag);

FTE_RET FTE_OBJ_EVENT_attach(FTE_OBJECT_PTR pObj, FTE_EVENT_PTR pEvent);
FTE_RET FTE_OBJ_EVENT_detach(FTE_OBJECT_PTR pObj, FTE_EVENT_PTR pEvent);

void    FTE_OBJ_STATE_set(FTE_OBJECT_PTR pObj, FTE_UINT32 xFlags);
void    FTE_OBJ_STATE_clear(FTE_OBJECT_PTR pObj, FTE_UINT32 xFlags);
FTE_BOOL        FTE_OBJ_STATE_isSet(FTE_OBJECT_PTR pObj, FTE_UINT32 xFlags);

FTE_RET FTE_OBJ_getChildCount(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pCount);
FTE_RET FTE_OBJ_getChild(FTE_OBJECT_PTR pObj, FTE_UINT32 ulIndex, FTE_OBJECT_PTR _PTR_ ppChild);

FTE_RET FTE_OBJ_attachChild(FTE_OBJECT_PTR pObj, FTE_OBJECT_ID xChildID);
FTE_RET FTE_OBJ_detachChild(FTE_OBJECT_PTR pObj, FTE_OBJECT_ID xChildID);

FTE_RET FTE_OBJ_1WIRE_discovery(FTE_BOOL bSave, FTE_UINT32_PTR pCount);

FTE_RET FT_OBJ_STAT_incSucceed(FTE_OBJECT_STATISTICS_PTR pStat);
FTE_RET FT_OBJ_STAT_incFailed(FTE_OBJECT_STATISTICS_PTR pStat);

FTE_RET FTE_OBJ_createJSON(FTE_OBJECT_PTR pObj, FTE_UINT32 xOptions, FTE_JSON_OBJECT_PTR _PTR_ ppJSON);

FTE_INT32   FTE_OBJ_SHELL_cmd(FTE_INT32 argc, FTE_CHAR_PTR argv[]);

#define FTE_OBJ_IS_SET(pObj, xFlag)     FTE_FLAG_IS_SET((pObj)->pConfig->xCommon.xFlags, xFlag)
#define FTE_OBJ_IS_RESET(pObj, xFlag)     FTE_FLAG_IS_CLR((pObj)->pConfig->xCommon.xFlags, xFlag)

#define FTE_OBJ_IS_ENABLED(pObj)     FTE_FLAG_IS_SET((pObj)->pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE)

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
#include "obj/fte_ftlm.h"
#include "obj/fte_cias.h"
#include "obj/fte_ioex.h"
#include "obj/fte_dotech.h"
#include "obj/fte_turbomax.h"
#include "obj/fte_dio_node.h"
#include "obj/fte_futuretek.h"

#endif