#ifndef __FTE_FIS_H__
#define __FTE_FIS_H__

#include "fte_gus.h"

#define FTE_FIS_DEFAULT_UPDATE_INTERVAL     10000

typedef struct FTE_FIS_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          nModel;
    FTE_UINT32          nUCSID;
    FTE_UINT32          nInterval;
    FTE_UINT32          nREQID;
}   FTE_FIS_CONFIG, _PTR_ FTE_FIS_CONFIG_PTR;

typedef struct FTE_FIS_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32          hRepeatTimer;
    FTE_UINT32          hConvertTimer;
    FTE_UCS_PTR         pUCS;
    pointer             pModelInfo;
    FTE_UINT32          nTrial;

    FTE_INT32          nTemperature;
    FTE_INT32          nAlarm;
    FTE_INT32          nDate;
    FTE_LWGPIO_PTR      pREQ;
}   FTE_FIS_STATUS, _PTR_ FTE_FIS_STATUS_PTR;

FTE_RET     FTE_FIS_attach(FTE_OBJECT_PTR pObj);
FTE_RET     FTE_FIS_detach(FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_FIS_request(FTE_OBJECT_PTR pObj);
FTE_RET     FTE_FIS_received(FTE_OBJECT_PTR pObj);

extern  
FTE_VALUE_TYPE FTE_FIS_valueTypes[];

extern  const 
FTE_GUS_MODEL_INFO    FTE_FIS3061_GUSModelInfo;


#endif
