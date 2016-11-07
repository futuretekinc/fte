#ifndef __FTE_GAI_H__
#define __FTE_GAI_H__

#define FTE_GAI_DEFAULT_UPDATE_INTERVAL     2000

typedef enum
{
    FTE_GAI_VALUE_TYPE_UNKNOWN      = 0,
    FTE_GAI_VALUE_TYPE_0_2_TO_1_518V  = 1,
    FTE_GAI_VALUE_TYPE_0_2_TO_1_10A
}   FTE_GAI_VALUE_TYPE, _PTR_ FTE_GAI_VALUE_TYPE_PTR;

typedef struct
{
    FTE_GAI_VALUE_TYPE  xType;
    struct
    {
        FTE_UINT32 ulMin;
        FTE_UINT32 ulMax;
    }   xInputRange;
    struct
    {
        FTE_VALUE_TYPE  xType;
        FTE_UINT32  ulMin;
        FTE_UINT32  ulMax;
    }   xValueRange;
    FTE_UINT32     ulCalibration;
}   FTE_GAI_VALUE_DESCRIPT, _PTR_ FTE_GAI_VALUE_DESCRIPT_PTR;


/*****************************************************************************
 * AD7785-based GAI Object Structure Description
 *****************************************************************************/
typedef struct _FTE_GAI_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          nDevID;
    FTE_UINT32          nInterval;
    FTE_UINT32          nGain;
    FTE_UINT32          ulDivide;
    FTE_GAI_VALUE_TYPE  xValueType;
}   FTE_GAI_CONFIG, _PTR_ FTE_GAI_CONFIG_PTR;

typedef struct  FTE_GAI_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32          hRepeatTimer;
    FTE_UINT32          hConvertTimer;
    FTE_AD7785_PTR      pADC;
    FTE_GAI_VALUE_DESCRIPT_PTR  pValueDescript;
}   FTE_GAI_STATUS, _PTR_ FTE_GAI_STATUS_PTR;

FTE_RET   FTE_GAI_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET   FTE_GAI_detach(FTE_OBJECT_PTR pObj);

extern  
FTE_GAI_CONFIG FTE_GAI_VOLTAGE_defaultConfig;

extern  
FTE_GAI_CONFIG FTE_GAI_CURRENT_defaultConfig;

#endif
