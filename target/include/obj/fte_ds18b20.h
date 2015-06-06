#ifndef __FTE_DS18B20_H__
#define __FTE_DS18B20_H__

#include "fte_1wire.h"

#define FTE_DS18B20_INITIAL_VALUE   8500
#define FTE_DS18B20_MAX_VALUE       12500
#define FTE_DS18B20_MIN_VALUE       -5500
/*****************************************************************************
 * DS18B20 Object Structure Description
 *****************************************************************************/
typedef struct _fte_ds18b20_create_params
{
    uint_32             nBUSID;
    FTE_1WIRE_ROM_CODE  pROMCode;
}   FTE_DS18B20_CREATE_PARAMS, _PTR_ FTE_DS18B20_CREATE_PARAMS_PTR;

typedef struct _fte_ds18b20_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nBUSID;
    FTE_1WIRE_ROM_CODE  pROMCode;
    uint_32             nInterval;
}   FTE_DS18B20_CONFIG, _PTR_ FTE_DS18B20_CONFIG_PTR;

typedef struct  _fte_ds18b20_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    uint_32             hRepeatTimer;
    uint_32             hConvertTimer;
    FTE_1WIRE_PTR       p1Wire;
    
}   FTE_DS18B20_STATUS, _PTR_ FTE_DS18B20_STATUS_PTR;

_mqx_uint       fte_ds18b20_preinit(uint_32 nMaxObjects);
FTE_OBJECT_PTR  fte_ds18b20_create(FTE_DS18B20_CREATE_PARAMS_PTR pParams);
_mqx_uint       fte_ds18b20_destroy(FTE_OBJECT_PTR pObj);

_mqx_uint       fte_ds18b20_attach(FTE_OBJECT_PTR pObj);
_mqx_uint       fte_ds18b20_detach(FTE_OBJECT_PTR pObj);

_mqx_uint       fte_ds18b20_set_rom_code(FTE_OBJECT_PTR pObj, uint_8 pROMCode[8]);
boolean         fte_ds18b20_is_valid(FTE_OBJECT_PTR pObj);

boolean         fte_ds18b20_is_exist_rom_code(uint_8 pROMCode[8]);

int_32          fte_ds18b20_shell_cmd(int_32 argc, char_ptr argv[] );

#endif
