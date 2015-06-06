#ifndef __FTE_FIS_H__
#define __FTE_FIS_H__

typedef struct FTE_FIS_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nModel;
    uint_32             nUCSID;
    uint_32             nInterval;
    uint_32             nREQID;
}   FTE_FIS_CONFIG, _PTR_ FTE_FIS_CONFIG_PTR;

typedef struct FTE_FIS_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    uint_32             hRepeatTimer;
    uint_32             hConvertTimer;
    FTE_UCS_PTR         pUCS;
    pointer             pModelInfo;
    uint_32             nTrial;

    int_32          nTemperature;
    int_32          nAlarm;
    int_32          nDate;
    FTE_LWGPIO_PTR  pREQ;
}   FTE_FIS_STATUS, _PTR_ FTE_FIS_STATUS_PTR;

_mqx_uint   FTE_FIS_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_FIS_detach(FTE_OBJECT_PTR pObj);
uint_32     FTE_FIS_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_FIS_received(FTE_OBJECT_PTR pObj);

extern  FTE_VALUE_TYPE FTE_FIS_valueTypes[];

#define FTE_FIS3061_DESCRIPTOR  {                   \
        .nModel     = FTE_GUS_MODEL_FIS3061,    \
        .pName      = "FIS3061",                \
        .nFieldCount= 1,                        \
        .pValueTypes= FTE_FIS_valueTypes,       \
        .f_attach   = FTE_FIS_attach,           \
        .f_detach   = FTE_FIS_detach,           \
        .f_request  = FTE_FIS_request,          \
        .f_received = FTE_FIS_received          \
    }

#endif
