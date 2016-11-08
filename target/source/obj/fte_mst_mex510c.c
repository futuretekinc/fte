#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

#if FTE_MST_MEX510C_SUPPORTED

#define FTE_MST_MEX510C_FRAME_FIELD_INPUT               1
#define FTE_MST_MEX510C_FRAME_FIELD_ALERT1              2
#define FTE_MST_MEX510C_FRAME_FIELD_ALERT2              3
#define FTE_MST_MEX510C_FRAME_FIELD_OP                  4
#define FTE_MST_MEX510C_FRAME_FIELD_OUTPUT_STATE        5
#define FTE_MST_MEX510C_FRAME_FIELD_CURR_TEMP           6
#define FTE_MST_MEX510C_FRAME_FIELD_SETUP_TEMP          7
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_DEVIATION      8
#define FTE_MST_MEX510C_FRAME_FIELD_OP_DELAY            9
#define FTE_MST_MEX510C_FRAME_FIELD_STOP_DELAY          10
#define FTE_MST_MEX510C_FRAME_FIELD_TEMP_CAL            11
#define FTE_MST_MEX510C_FRAME_FIELD_HIGH_TEMP           12
#define FTE_MST_MEX510C_FRAME_FIELD_LOW_TEMP            13
#define FTE_MST_MEX510C_FRAME_FIELD_HEAT_DEVIATION      14
#define FTE_MST_MEX510C_FRAME_FIELD_FAN_CTRL            15
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_OP_DELAY       16
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_STOP_DELAY     17
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_REOP_DELAY     18
#define FTE_MST_MEX510C_FRAME_FIELD_LP_ALERT_DELAY      19
#define FTE_MST_MEX510C_FRAME_FIELD_DEVICE_ID           24
#define FTE_MST_MEX510C_FRAME_FIELD_DIRECT_CTRL         32

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_TEMPERATURE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_TEMPERATURE,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};


static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_SET_TEMPERATURE_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "SET TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_SET_TEMPERATURE,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_HIGH_TEMPERATURE_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "SET HIGH TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_SET_HIGH_TEMP,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_LOW_TEMPERATURE_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "SET LOW TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_SET_LOW_TEMP,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_TEMPERATURE_DEVIATION_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "COOL DEVIATION",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_HEAT_TEMPERATURE_DEVIATION_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "HEAT DEVIATION",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C__OPERATION_STATE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "STATE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_OPERATION,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_ALARM_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "ALARM",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_ALARM,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};


static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_STATE_HEAT_defaultConfig=
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "HEAT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_HEAT,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_STATE_DHEAT_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "D.HEAT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_DHEAT,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_STATE_FAN_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "FAN",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_FAN,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_STATE_SOLVALVE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "SOLVALVE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_SOLVALVE,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_STATE_COMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "COMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_COMP,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_ENABLE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL ENABLE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_A_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL A",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_A,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_B_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL B",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_B,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_C_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL C",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_C,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_D_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL D",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_D,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_E_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL E",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_E,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_F_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL F",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_F,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_G_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL G",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_G,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_MST_MEX510C_DIRECT_CTRL_H_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL H",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_OBJECT_CONFIG_PTR FTE_MST_MEX510C_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_TEMPERATURE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_SET_TEMPERATURE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_SET_HIGH_TEMPERATURE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_SET_LOW_TEMPERATURE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_COOL_TEMPERATURE_DEVIATION_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_HEAT_TEMPERATURE_DEVIATION_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_OPERATION_STATE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_ALARM_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_STATE_HEAT_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_TATE_DHEAT_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_STATE_FAN_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_STATE_SOLVALVE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_STATE_COMP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_ENABLE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_A_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_B_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_C_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_D_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_E_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_F_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_G_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_DIRECT_CTRL_H_defaultConfig
};

FTE_GUS_CONFIG FTE_MST_MEX510C_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
        .pName      = "MEX-510C",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
        .ulChild    = sizeof(FTE_MST_MEX510C_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_MST_MEX510C_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_MST_MEX510C,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_MST_MEX510C_DEFAULT_UPDATE_INTERVAL
};

FTE_VALUE_TYPE  FTE_MST_MEX510C_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_TEMPERATURE, FTE_VALUE_TYPE_TEMPERATURE,   FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE, FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_TEMPERATURE,   FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG
};

const 
FTE_GUS_MODEL_INFO    FTE_MST_MEX510C_GUSModelInfo = 
{
    .nModel         = FTE_GUS_MODEL_MST_MEX510C,
    .pName          = "MEX-510C",
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_MST_MEX510C_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_MST_MEX510C_DEFAULT_DATABITS,
        .nParity    =   FTE_MST_MEX510C_DEFAULT_PARITY,
        .nStopBits  =   FTE_MST_MEX510C_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_MST_MEX510C_DEFAULT_FULL_DUPLEX
    },
    .nFieldCount    = FTE_MST_MEX510C_INDEX_MAX+1,
    .pValueTypes    = FTE_MST_MEX510C_valueTypes,
    .f_request      = fte_mst_mex510c_request_data,
    .f_received     = fte_mst_mex510c_receive_data,
    .f_set          = fte_mst_mex510c_set,
    .f_set_config = FTE_MST_MEX510C_setConfig,
    .f_get_config = FTE_MST_MEX510C_getConfig
};

static const   
FTE_CHAR_PTR    pStringTargetTemperature= "targetTemperature";
static const   
FTE_CHAR_PTR    pStringCoolingDeviation = "coolingDeviation";
static const   
FTE_CHAR_PTR    pStringHeatingDeviation = "heatingDeviation";

FTE_RET   FTE_MST_MEX510C_setConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pString
)
{
    FTE_INT32  nValue = 0;
    FTE_GUS_STATUS_PTR  pStatus;

    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
   
    const nx_json* pxJSON = nx_json_parse_utf8(pString);
    if (pxJSON == NULL)
    {
        return  FTE_RET_ERROR;
    } 
    
    const nx_json* pxTemperature    = nx_json_get(pxJSON, pStringTargetTemperature);
    const nx_json* pxCoolDeviation  = nx_json_get(pxJSON, pStringCoolingDeviation);
    const nx_json* pxHeatDeviation  = nx_json_get(pxJSON, pStringHeatingDeviation);

    if ((pxTemperature != NULL) && (pxTemperature->type != NX_JSON_NULL))
    {
        FTE_UINT32 nLen;
        FTE_UINT16 uiCRC = 0;  
        FTE_UINT8  pBuff[8];
        FTE_UINT8  pCMD[9] = { 0x01, 0x06, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};

        if (pxTemperature->type == NX_JSON_DOUBLE)
        {
            nValue = (FTE_UINT32)(pxTemperature->dbl_value * 10);
        }
        else if (pxTemperature->type == NX_JSON_INTEGER)
        {
            nValue = (FTE_UINT32)(pxTemperature->int_value * 10);
        }
        else if (pxTemperature->type == NX_JSON_STRING)
        {
            nValue = (FTE_UINT32)(strtod(pxTemperature->text_value, NULL) * 10);
        }
        else
        {
            return  FTE_RET_ERROR;
        }        
        
        pCMD[4] = (nValue >> 8) & 0xFF;
        pCMD[5] = (nValue     ) & 0xFF;

        uiCRC = FTE_CRC16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 200, 1000);
        if (nLen == sizeof(pBuff))
        {
            if (memcmp(pCMD, pBuff, sizeof(pBuff)) == 0)
            {
                FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_TEMPERATURE], nValue*10);
            }
        }
    }
    
    if ((pxCoolDeviation != NULL) && (pxCoolDeviation->type != NX_JSON_NULL))
    {
        FTE_UINT32 nLen;
        FTE_UINT16 uiCRC = 0;  
        FTE_UINT8  pBuff[8];
        FTE_UINT8  pCMD[9] = { 0x01, 0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        if (pxCoolDeviation->type == NX_JSON_DOUBLE)
        {
            nValue = (FTE_UINT32)(pxCoolDeviation->dbl_value * 10);
        }
        else if (pxCoolDeviation->type == NX_JSON_INTEGER)
        {
            nValue = (FTE_UINT32)(pxCoolDeviation->int_value * 10);
        }
        else if (pxCoolDeviation->type == NX_JSON_STRING)
        {
            nValue = (FTE_UINT32)(strtod(pxCoolDeviation->text_value, NULL) * 10);
        }
        else
        {
            return  FTE_RET_ERROR;
        }        

        pCMD[4] = (nValue >> 8) & 0xFF;
        pCMD[5] = (nValue     ) & 0xFF;

        uiCRC = FTE_CRC16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 200, 1000);
        if (nLen == sizeof(pBuff))
        {
            if (memcmp(pCMD, pBuff, sizeof(pBuff)) == 0)
            {
                FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION], nValue*10);
            }
        }
    }
    
    if ((pxHeatDeviation != NULL) && (pxHeatDeviation->type != NX_JSON_NULL))
    {
        FTE_UINT32 nLen;
        FTE_UINT16 uiCRC = 0;  
        FTE_UINT8  pBuff[8];
        FTE_UINT8  pCMD[9] = { 0x01, 0x06, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00};

        if (pxHeatDeviation->type == NX_JSON_DOUBLE)
        {
            nValue = (FTE_UINT32)(pxHeatDeviation->dbl_value * 10);
        }
        else if (pxHeatDeviation->type == NX_JSON_INTEGER)
        {
            nValue = (FTE_UINT32)(pxHeatDeviation->int_value * 10);
        }
        else if (pxHeatDeviation->type == NX_JSON_STRING)
        {
            nValue = (FTE_UINT32)(strtod(pxHeatDeviation->text_value, NULL) * 10);
        }
        else
        {
            return  FTE_RET_ERROR;
        }        

        pCMD[4] = (nValue >> 8) & 0xFF;
        pCMD[5] = (nValue     ) & 0xFF;

        uiCRC = FTE_CRC16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 200, 1000);
        if (nLen == sizeof(pBuff))
        {
            if (memcmp(pCMD, pBuff, sizeof(pBuff)) == 0)
            {
                FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION], nValue*10);
            }
        }
    }
    
    nx_json_free(pxJSON);
   
    return  FTE_RET_OK;
}

FTE_RET   FTE_MST_MEX510C_getConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    FTE_GUS_STATUS_PTR  pStatus;
    FTE_JSON_VALUE_PTR  pJOSNObject;
    FTE_JSON_VALUE_PTR  pJOSNValue;
    FTE_CHAR            pValueString[32];
    
    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    pJOSNObject = FTE_JSON_VALUE_createObject(3);
    if (pJOSNObject == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_TEMPERATURE], pValueString, sizeof(pValueString));
    pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringTargetTemperature, pJOSNValue);
        
    FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION], pValueString, sizeof(pValueString));
    pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        
    FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION], pValueString, sizeof(pValueString));
    pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
        
    if (FTE_JSON_VALUE_buffSize(pJOSNObject) >= ulBuffLen)
    {
        FTE_JSON_VALUE_destroy(pJOSNObject);
        return  FTE_RET_ERROR;
    }
    
    FTE_JSON_VALUE_snprint(pBuff, ulBuffLen, pJOSNObject);
    
    FTE_JSON_VALUE_destroy(pJOSNObject);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MST_MEX510C_requestData
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    FTE_UINT8  pCMD[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x21, 0x85, 0xd2, 0x00};
    
    pCMD[0] = (FTE_UINT8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), FALSE);    

    return  FTE_RET_OK;
}

FTE_RET   FTE_MST_MEX510C_receiveData
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_UINT8      pBuff[128];
    FTE_UINT32     nLen;
    FTE_UINT16     uiCRC;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 71)
    {
        return  FTE_RET_ERROR; 
    }

    if ((pBuff[0] != 0x01))
    {
        return  FTE_RET_ERROR;
    }
    
    uiCRC = FTE_CRC16(pBuff, 69);
    if (uiCRC != (pBuff[69] | ((FTE_UINT16)pBuff[70] << 8)))
    {
        return  FTE_RET_ERROR;
    }
    
    // Temperature
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DEVICE_ID], pBuff[FTE_MST_MEX510C_FRAME_FIELD_DEVICE_ID * 2 + 3]);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_ALARM], (pBuff[FTE_MST_MEX510C_FRAME_FIELD_ALERT2 * 2 + 3] >> 7) & 01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_OPERATION], pBuff[FTE_MST_MEX510C_FRAME_FIELD_OP * 2 + 4] & 01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_TEMPERATURE],(((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_CURR_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_CURR_TEMP * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION],(((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_COOL_DEVIATION * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_COOL_DEVIATION * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION],(((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_HEAT_DEVIATION * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_HEAT_DEVIATION * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_TEMPERATURE], (((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_SETUP_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_SETUP_TEMP * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_HIGH_TEMP], (((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_HIGH_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_HIGH_TEMP * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_LOW_TEMP], (((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_LOW_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_LOW_TEMP * 2 + 4])*10);
    
    FTE_UINT16 nOutputState = ((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_OUTPUT_STATE * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_OUTPUT_STATE * 2 + 4];
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_HEAT],(nOutputState >> 4) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_DHEAT], (nOutputState >> 3) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_FAN], (nOutputState >> 2) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_SOLVALVE], (nOutputState >> 1) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_COMP], (nOutputState >> 0) & 0x01);
    
    FTE_UINT16 nDirectCtrl = ((FTE_UINT16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_DIRECT_CTRL * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_DIRECT_CTRL * 2 + 4];

    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE], (nDirectCtrl >> 15) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_A], (nDirectCtrl >> 0) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_B], (nDirectCtrl >> 1) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_C], (nDirectCtrl >> 2) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_D], (nDirectCtrl >> 3) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_E], (nDirectCtrl >> 4) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_F], (nDirectCtrl >> 5) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_G], (nDirectCtrl >> 6) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H], (nDirectCtrl >> 7) & 0x01);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MST_MEX510C_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nIndex, 
    FTE_VALUE_PTR   pValue
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;

    if (FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE <= nIndex && nIndex <= FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H)
    {
        FTE_UINT32 ulValue;
        FTE_UINT16 uiValue, uiCRC;
        FTE_UINT8  pCMD[9] = { 0x01, 0x06, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        FTE_VALUE_copy(&pStatus->xCommon.pValue[nIndex], pValue);
        
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE], &ulValue);        
        uiValue = ((ulValue & 0x01) << 15);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_A], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 0);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_B], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 1);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_C], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 2);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_D], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 3);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_E], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 4);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_F], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 5);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_G], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 6);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 7);

        pCMD[4] = (uiValue >> 8) & 0xFF;
        pCMD[5] = (uiValue     ) & 0xFF;

        uiCRC = FTE_CRC16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), FALSE);               

        return  FTE_RET_OK;
    }
    
    return  FTE_RET_ERROR;
}

#endif