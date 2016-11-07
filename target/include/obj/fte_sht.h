#ifndef __FTE_SHT_H__
#define __FTE_SHT_H__

#define FTE_SHT_DEVICE_MAX                  1

#define FTE_SHT_FIELD_HUMI                  0
#define FTE_SHT_FIELD_TEMP                  1
#define FTE_SHT_FIELD_MAX                   2

#define FTE_SHT_DEFAULT_UPDATE_INTERVAL     5000
#define FTE_SHT_DEFAULT_SCL_INTERVAL        0

#define FTE_SHT_FLAG_CONVERT_HUMIDITY       0x0100
#define FTE_SHT_FLAG_CONVERT_TEMPERATURE    0x0200

#define FTE_SHT_DELAY_MAX                   5

/*****************************************************************************
 * SHT Object Structure Description
 *****************************************************************************/
typedef struct FTE_SHT_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32          nSDA;
    FTE_UINT32          nSCL;
    FTE_UINT32          nInterval;
    FTE_UINT32          ulDelay;
}   FTE_SHT_CONFIG, _PTR_ FTE_SHT_CONFIG_PTR;

typedef struct  FTE_SHT_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_UINT32          hRepeatTimer;
    FTE_UINT32          hConvertTimer;
    FTE_UINT32          bHumidity;
    FTE_LWGPIO_PTR      pLWGPIO_SDA;
    FTE_LWGPIO_PTR      pLWGPIO_SCL;
    FTE_OBJECT_ID       xTObjID;
    FTE_OBJECT_ID       xHObjID;
}   FTE_SHT_STATUS, _PTR_ FTE_SHT_STATUS_PTR;

FTE_RET     FTE_SHT_create(FTE_OBJECT_CONFIG_PTR pConfig, FTE_OBJECT_PTR _PTR_ ppObj);
FTE_RET     FTE_SHT_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_SHT_detach(FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_SHT_printValue(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);

FTE_INT32   FTE_SHT_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);

extern  
FTE_SHT_CONFIG FTE_SHT_defaultConfig;
#endif
