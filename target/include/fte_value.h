#ifndef __FTE_VALUE_H__
#define __FTE_VALUE_H__

typedef enum
{
    FTE_VALUE_TYPE_ULONG    = 0,
    FTE_VALUE_TYPE_BOOL,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_HUMIDITY,
    FTE_VALUE_TYPE_PPM,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_VOLTAGE,
    FTE_VALUE_TYPE_CURRENT,
    FTE_VALUE_TYPE_PRESSURE,
    FTE_VALUE_TYPE_LED,
    FTE_VALUE_TYPE_PWR_KWH,
    FTE_VALUE_TYPE_PWR_KW,
    FTE_VALUE_TYPE_MAX
}   FTE_VALUE_TYPE, _PTR_ FTE_VALUE_TYPE_PTR;

typedef struct
{
    FTE_VALUE_TYPE  xType;
    char_ptr        pName;
}   FTE_VALUE_INFO, _PTR_ FTE_VALUE_INFO_PTR;

typedef struct
{
    FTE_VALUE_TYPE  xType;
    boolean         bValid;
    uint_32         ulFailCount;
    union
    {
        boolean     bValue;
        uint_32     ulValue;
        int_32      nValue;
        uint_32     xState;
    }   xData;
    
    TIME_STRUCT     xTimeStamp;    
}   FTE_VALUE, _PTR_ FTE_VALUE_PTR;

FTE_VALUE_PTR   FTE_VALUE_create(FTE_VALUE_TYPE xType);
FTE_VALUE_PTR   FTE_VALUE_createULONG(void);
FTE_VALUE_PTR   FTE_VALUE_createTemperature(void);
FTE_VALUE_PTR   FTE_VALUE_createHumidity(void);
FTE_VALUE_PTR   FTE_VALUE_createDIO(void);
FTE_VALUE_PTR   FTE_VALUE_createPPM(void);
FTE_VALUE_PTR   FTE_VALUE_createVoltage(void);
FTE_VALUE_PTR   FTE_VALUE_createCurrent(void);
FTE_VALUE_PTR   FTE_VALUE_createPressure(void);
FTE_VALUE_PTR   FTE_VALUE_createPowerKW(void);
FTE_VALUE_PTR   FTE_VALUE_createPowerKWH(void);
FTE_VALUE_PTR   FTE_VALUE_createLED(void);
FTE_VALUE_PTR   FTE_VALUE_createArray(FTE_VALUE_TYPE_PTR pTypes, uint_32 ulCount);
_mqx_uint       FTE_VALUE_destroy(FTE_VALUE_PTR pValue);

_mqx_uint       FTE_VALUE_init(FTE_VALUE_PTR pValue, FTE_VALUE_TYPE xType);
_mqx_uint       FTE_VALUE_initULONG(FTE_VALUE_PTR pValue, uint_32 ulValue);
_mqx_uint       FTE_VALUE_initTemperature(FTE_VALUE_PTR pValue, int_32 nValue);
_mqx_uint       FTE_VALUE_initHumidity(FTE_VALUE_PTR pValue, uint_32 ulValue);
_mqx_uint       FTE_VALUE_initDIO(FTE_VALUE_PTR pValue, boolean bValue);
_mqx_uint       FTE_VALUE_initPPM(FTE_VALUE_PTR pValue, uint_32 ulValue);
_mqx_uint       FTE_VALUE_initLED(FTE_VALUE_PTR pValue, uint_32 ulState);
_mqx_uint       FTE_VALUE_initVoltage(FTE_VALUE_PTR pValue, uint_32 ulValue);
_mqx_uint       FTE_VALUE_initCurrent(FTE_VALUE_PTR pValue, uint_32 ulValue);
_mqx_uint       FTE_VALUE_initPressure(FTE_VALUE_PTR pValue, uint_32 ulValue);
_mqx_uint       FTE_VALUE_initPowerKW(FTE_VALUE_PTR pValue, uint_32 ulValue);
_mqx_uint       FTE_VALUE_initPowerKWH(FTE_VALUE_PTR pValue, uint_32 ulValue);

_mqx_uint       FTE_VALUE_setValid(FTE_VALUE_PTR pValue, boolean bValid);
boolean         FTE_VALUE_isValid(FTE_VALUE_PTR pValue);

_mqx_uint       FTE_VALUE_set(FTE_VALUE_PTR pObj, char_ptr pString);

_mqx_uint       FTE_VALUE_getTemperature(FTE_VALUE_PTR pObj, int_32_ptr pnValue);
_mqx_uint       FTE_VALUE_setTemperature(FTE_VALUE_PTR pObj, int_32 nValue);

_mqx_uint       FTE_VALUE_getHumidity(FTE_VALUE_PTR pObj, uint_32_ptr pulValue);
_mqx_uint       FTE_VALUE_setHumidity(FTE_VALUE_PTR pObj, uint_32 ulValue);

_mqx_uint       FTE_VALUE_getDIO(FTE_VALUE_PTR pObj, boolean *pbValue);
_mqx_uint       FTE_VALUE_setDIO(FTE_VALUE_PTR pObj, boolean bValue);

_mqx_uint       FTE_VALUE_getPPM(FTE_VALUE_PTR pObj, uint_32_ptr pulValue);
_mqx_uint       FTE_VALUE_setPPM(FTE_VALUE_PTR pObj, uint_32 ulValue);

_mqx_uint       FTE_VALUE_setULONG(FTE_VALUE_PTR pObj, uint_32 ulValue);
_mqx_uint       FTE_VALUE_getULONG(FTE_VALUE_PTR pObj, uint_32_ptr pulValue);

_mqx_uint       FTE_VALUE_setLED(FTE_VALUE_PTR pObj, uint_32 xState);
_mqx_uint       FTE_VALUE_getLED(FTE_VALUE_PTR pObj, uint_32_ptr pxState);

_mqx_uint       FTE_VALUE_setVoltage(FTE_VALUE_PTR pObj, uint_32 ulValue);
_mqx_uint       FTE_VALUE_getVoltage(FTE_VALUE_PTR pObj, uint_32_ptr pulValue);

_mqx_uint       FTE_VALUE_setCurrent(FTE_VALUE_PTR pObj, uint_32 ulValue);
_mqx_uint       FTE_VALUE_getCurrent(FTE_VALUE_PTR pObj, uint_32_ptr pulValue);

_mqx_uint       FTE_VALUE_setPressure(FTE_VALUE_PTR pObj, int_32 ulValue);
_mqx_uint       FTE_VALUE_getPressure(FTE_VALUE_PTR pObj, int_32_ptr pulValue);

_mqx_uint       FTE_VALUE_setPowerKW(FTE_VALUE_PTR pObj, uint_32 ulValue);
_mqx_uint       FTE_VALUE_getPowerKW(FTE_VALUE_PTR pObj, uint_32_ptr pulValue);

_mqx_uint       FTE_VALUE_setPowerKWH(FTE_VALUE_PTR pObj, uint_32 ulValue);
_mqx_uint       FTE_VALUE_getPowerKWH(FTE_VALUE_PTR pObj, uint_32_ptr pulValue);

_mqx_uint       FTE_VALUE_getTimeStamp(FTE_VALUE_PTR pObj, TIME_STRUCT_PTR pTimeStamp);

_mqx_uint       FTE_VALUE_copy(FTE_VALUE_PTR pDest, FTE_VALUE_PTR pSrc);
char_ptr        FTE_VALUE_toString(FTE_VALUE_PTR pObj, char_ptr pValueString, uint_32 nLen);
char_ptr        FTE_VALUE_unit(FTE_VALUE_PTR pObj, char_ptr pUnitString, uint_32 nLen);
char_ptr        FTE_VALUE_printTimeStamp(FTE_VALUE_PTR pObj);

boolean         FTE_VALUE_equal(FTE_VALUE_PTR pValue1, FTE_VALUE_PTR pValue2);
#endif