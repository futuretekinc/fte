#ifndef __FTE_DS18B20_H__
#define __FTE_DS18B20_H__

#include "fte_1wire.h"

#define FTE_DS18B20_DEFAULT_UPDATE_INTERVAL 10000

#define FTE_DS18B20_INITIAL_VALUE   8500
#define FTE_DS18B20_MAX_VALUE       12500
#define FTE_DS18B20_MIN_VALUE       -5500
/*****************************************************************************
 * DS18B20 Object Structure Description
 *****************************************************************************/
typedef struct _FTE_DS18B20_CREATE_PARAMS_STRUCT
{
    FTE_UINT32             nBUSID;
    FTE_1WIRE_ROM_CODE  pROMCode;
}   FTE_DS18B20_CREATE_PARAMS, _PTR_ FTE_DS18B20_CREATE_PARAMS_PTR;

typedef struct _FTE_DS18B20_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          nBUSID;
    FTE_1WIRE_ROM_CODE  pROMCode;
    FTE_UINT32          nInterval;
}   FTE_DS18B20_CONFIG, _PTR_ FTE_DS18B20_CONFIG_PTR;

typedef struct  _FTE_DS18B20_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32             hRepeatTimer;
    FTE_UINT32             hConvertTimer;
    FTE_1WIRE_PTR       p1Wire;
    
}   FTE_DS18B20_STATUS, _PTR_ FTE_DS18B20_STATUS_PTR;

FTE_RET     FTE_DS18B20_preinit(FTE_UINT32 nMaxObjects);
FTE_OBJECT_PTR  FTE_DS18B20_create(FTE_DS18B20_CREATE_PARAMS_PTR pParams);
FTE_RET     FTE_DS18B20_destroy(FTE_OBJECT_PTR pObj);

FTE_RET     FTE_DS18B20_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_DS18B20_detach(FTE_OBJECT_PTR pObj);

FTE_RET     FTE_DS18B20_setROMCode(FTE_OBJECT_PTR pObj, FTE_UINT8 pROMCode[8]);
FTE_BOOL    FTE_DS18B20_isValid(FTE_OBJECT_PTR pObj);

FTE_BOOL    FTE_DS18B20_isExistROMCode(FTE_UINT8 pROMCode[8]);

FTE_INT32   FTE_DS18B20_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );

#endif
