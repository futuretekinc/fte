#include "fte_target.h"
#include "fte_crc.h"
#include "fte_cias.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"
#include "fte_task.h"
#include "nxjson.h"
#include "fte_dotech.h"

#if FTE_DOTECH_SUPPORTED

#ifndef FTE_DOTECH_DEFAULT_LOOP_PERIOD
#define FTE_DOTECH_DEFAULT_LOOP_PERIOD        2000 // ms
#endif

#ifndef FTE_DOTECH_DEFAULT_UPDATE_PERIOD
#define FTE_DOTECH_DEFAULT_UPDATE_PERIOD      10000 // ms
#endif

#ifndef FTE_DOTECH_DEFAULT_REQUEST_TIMEOUT
#define FTE_DOTECH_DEFAULT_REQUEST_TIMEOUT    500
#endif

#ifndef FTE_DOTECH_DEFAULT_RETRY_COUNT
#define FTE_DOTECH_DEFAULT_RETRY_COUNT        3
#endif


#define FTE_DOTECH_FX3D_REG_CTRL_START      0
#define FTE_DOTECH_FX3D_REG_CTRL_TEMP0      0
#define FTE_DOTECH_FX3D_REG_CTRL_TEMP1      1
#define FTE_DOTECH_FX3D_REG_CTRL_MODE0      2
#define FTE_DOTECH_FX3D_REG_CTRL_DEVIATION0 3
#define FTE_DOTECH_FX3D_REG_CTRL_ON_DEALY0  4   
#define FTE_DOTECH_FX3D_REG_CTRL_OFF_MIN0   5
#define FTE_DOTECH_FX3D_REG_CTRL_ON_MIN0    6
#define FTE_DOTECH_FX3D_REG_CTRL_DEVI_MODE0 7
#define FTE_DOTECH_FX3D_REG_CTRL_TEMP_MAX0  8
#define FTE_DOTECH_FX3D_REG_CTRL_TEMP_MIN0  9
#define FTE_DOTECH_FX3D_REG_CTRL_MODE1      10
#define FTE_DOTECH_FX3D_REG_CTRL_DEVIATION1 11
#define FTE_DOTECH_FX3D_REG_CTRL_ON_DEALY1  12 
#define FTE_DOTECH_FX3D_REG_CTRL_OFF_MIN1   13
#define FTE_DOTECH_FX3D_REG_CTRL_ON_MIN1    14
#define FTE_DOTECH_FX3D_REG_CTRL_DEVI_MODE1 15
#define FTE_DOTECH_FX3D_REG_CTRL_TEMP_MAX1  16
#define FTE_DOTECH_FX3D_REG_CTRL_TEMP_MIN1  17

#define FTE_DOTECH_FX3D_REG_CTRL_MAX        18

#define FTE_DOTECH_FX3D_REG_STAT_START      100
#define FTE_DOTECH_FX3D_REG_STAT            100
#define FTE_DOTECH_FX3D_REG_STAT_CURR_TEMP  101
#define FTE_DOTECH_FX3D_REG_STAT_DISP_TEMP  102
#define FTE_DOTECH_FX3D_REG_STAT_CTRL_TEMP0 103
#define FTE_DOTECH_FX3D_REG_STAT_CTRL_TEMP1 104

#define FTE_DOTECH_FX3D_REG_STAT_MAX        5

typedef struct  FTE_DOTECH_DEVICE_STRUCT
{
    _task_id        xTaskID;
    FTE_OBJECT_PTR  pObj;
}   FTE_DOTECH_DEVICE, _PTR_ FTE_DOTECH_DEVICE_PTR;


FTE_RET FTE_DOTECH_FX3D_writeReg
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32      ulIndex,
    FTE_INT32       nValue
);

static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_temp_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "TEMP0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_TEMP0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_ctrl_temp0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "CTRL0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_ctrl_temp1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "CTRL1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_CTRL_TEMP1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_ctrl_state0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "CTRL0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_CTRL_STATE0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_ctrl_state1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "CTRL1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_CTRL_STATE1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_outctrl0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0),
        .pName      = "CTRL0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_CTRL0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_outctrl1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0),
        .pName      = "CTRL1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_CTRL1,
    .nInterval  = 2
};

#ifdef FTE_DOTECH_FX3D_FIELD_SENSOR_ALARM
static const FTE_IFCE_CONFIG fte_init_dotech_fx3d_sensor_state_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "ALARM",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
    .nRegID     = FTE_DOTECH_FX3D_FIELD_SENSOR_ALARM,
    .nInterval  = 2
};
#endif

static const FTE_EVENT_CONFIG fte_changed_ctrl_temp_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_OBJECT_CONFIG_PTR pDevicesChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_temp_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_ctrl_temp0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_ctrl_temp1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_ctrl_state0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_ctrl_state1_config,
#ifdef FTE_DOTECH_FX3D_FIELD_SENSOR_ALARM
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_sensor_state_config,
#endif
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_outctrl0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_dotech_fx3d_outctrl1_config,
};

FTE_DOTECH_CONFIG FTE_DOTECH_FX3D_defaultConfig =
{
    .xModel     =       FTE_DOTECH_MODEL_FX3D,
    .xGUS       =
    {
        .xCommon    =
        {
            .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DOTECH_FX3D, 0),
            .pName      = "FX3D",
            .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
            .ulChild    = sizeof(pDevicesChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)pDevicesChildConfigs
        },
        .nModel     = FTE_GUS_MODEL_DOTECH_FX3D,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = 10
    }
};

FTE_VALUE_TYPE  FTE_DOTECH_FX3D_valueTypes[] =
{    
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
#ifdef FTE_DOTECH_FX3D_FIELD_SENSOR_ALARM
    FTE_VALUE_TYPE_DIO,
#endif
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE,
};

static  FTE_DOTECH_EXT_CONFIG xGlobalConfig = 
{
    .ulLoopPeriod       = FTE_DOTECH_DEFAULT_LOOP_PERIOD,
    .ulUpdatePeriod     = FTE_DOTECH_DEFAULT_UPDATE_PERIOD,
    .ulRequestTimeout   = FTE_DOTECH_DEFAULT_REQUEST_TIMEOUT,
    .ulRetryCount       = FTE_DOTECH_DEFAULT_RETRY_COUNT
};

static FTE_UINT32           ulDeviceCount=0;
static FTE_DOTECH_DEVICE    pDevices[FTE_DOTECH_FX3D_MAX];
static FTE_BOOL             bExtConfigLoaded = FALSE;

FTE_RET   FTE_DOTECH_FX3D_attach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_RET xRet;
    FTE_DOTECH_CONFIG_PTR    pConfig;
    FTE_DOTECH_FX3D_STATUS_PTR    pStatus;
    FTE_UCS_PTR  pUCS;
    
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));

    pConfig = (FTE_DOTECH_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pObj->pStatus;    
    
    pUCS = (FTE_UCS_PTR)FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return  FTE_RET_OBJECT_NOT_FOUND;
    }

    xRet = FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
        
    pStatus->xGUS.pUCS = pUCS;
    
    FTE_DOTECH_FX3D_init(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_DOTECH_FX3D_detach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_DOTECH_FX3D_STATUS_PTR  pStatus;

    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));

    pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pObj->pStatus;    
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
    
    return  FTE_RET_OK;
}

static const   char_ptr    pStringTargetTemperature= "targetTemperature";
static const   char_ptr    pStringCoolingDeviation = "coolingDeviation";
static const   char_ptr    pStringHeatingDeviation = "heatingDeviation";

FTE_RET FTE_DOTECH_FX3D_setChildConfig
(
    FTE_OBJECT_PTR  pObj, 
    char_ptr        pBuff
)
{
    FTE_RET xRet = FTE_RET_OK;
    FTE_IFCE_CONFIG_PTR         pConfig;
    FTE_OBJECT_PTR              pParent;
    FTE_DOTECH_FX3D_STATUS_PTR  pParentStatus;
    FTE_UINT32                     nCtrlID;
    FTE_INT32          nTargetTemperature;
    FTE_UINT32         ulCoolingDeviation = 0, ulHeatingDeviation = 0;
    FTE_UINT32         ulMode = FTE_DOTECH_FX3D_MODE_OFF;
    FTE_INT32          nCurrentTemperature = 0;
    
    ASSERT ((pObj != NULL) && (pObj->pConfig != NULL) && (pBuff != NULL));

    const nx_json* pxJSON = nx_json_parse_utf8(pBuff);
    if (pxJSON == NULL)
    {
        return  FTE_RET_ERROR;
    } 

    const nx_json* pxTargetTemperature = nx_json_get(pxJSON, pStringTargetTemperature);
    const nx_json* pxCoolingDeviation  = nx_json_get(pxJSON, pStringCoolingDeviation);
    const nx_json* pxHeatingDeviation  = nx_json_get(pxJSON, pStringHeatingDeviation);
    
    if ((FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_MULTI_VALUE) || (((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent == NULL))
    {
        xRet = FTE_RET_ERROR;
        goto error;
    }
    
    pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;

    nCtrlID = (pConfig->nRegID - FTE_DOTECH_FX3D_FIELD_CTRL0);
    if (nCtrlID > FTE_DOTECH_FX3D_CTRL_COUNT)
    {
        xRet = FTE_RET_ERROR;
        goto error;
    }
    
    pParent = ((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent;
    pParentStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pParent->pStatus;
    
    if ((pxTargetTemperature != NULL) && (pxTargetTemperature->type != NX_JSON_NULL))
    {
        if (pxTargetTemperature->type == NX_JSON_DOUBLE)
        {
            nTargetTemperature = (FTE_INT32)(pxTargetTemperature->dbl_value * 100);
        }
        else if (pxTargetTemperature->type == NX_JSON_INTEGER)
        {
            nTargetTemperature = (FTE_INT32)(pxTargetTemperature->int_value * 100);
        }
        else if (pxTargetTemperature->type == NX_JSON_STRING)
        {
            nTargetTemperature = (FTE_INT32)(strtod(pxTargetTemperature->text_value, NULL) * 100);
        }
        else
        {
            xRet = FTE_RET_ERROR;
            goto error;
        }        
    }
    
    if ((pxCoolingDeviation != NULL) && (pxCoolingDeviation->type != NX_JSON_NULL))
    {
        if (pxCoolingDeviation->type == NX_JSON_DOUBLE)
        {
            ulCoolingDeviation = (FTE_UINT32)(pxCoolingDeviation->dbl_value * 100);
        }
        else if (pxCoolingDeviation->type == NX_JSON_INTEGER)
        {
            ulCoolingDeviation = (FTE_UINT32)(pxCoolingDeviation->int_value * 100);
        }
        else if (pxCoolingDeviation->type == NX_JSON_STRING)
        {
            ulCoolingDeviation = (FTE_UINT32)(strtod(pxCoolingDeviation->text_value, NULL) * 100);
        }
        else
        {
            xRet = FTE_RET_ERROR;
            goto error;
        }        
    }
    
    if ((pxHeatingDeviation != NULL) && (pxHeatingDeviation->type != NX_JSON_NULL))
    {
        if (pxHeatingDeviation->type == NX_JSON_DOUBLE)
        {
            ulHeatingDeviation = (FTE_UINT32)(pxHeatingDeviation->dbl_value * 100);
        }
        else if (pxHeatingDeviation->type == NX_JSON_INTEGER)
        {
            ulHeatingDeviation = (FTE_UINT32)(pxHeatingDeviation->int_value * 100);
        }
        else if (pxHeatingDeviation->type == NX_JSON_STRING)
        {
            ulHeatingDeviation = (FTE_UINT32)(strtod(pxHeatingDeviation->text_value, NULL) * 100);
        }
        else
        {
            xRet = FTE_RET_ERROR;
            goto error;
        }        
    }
    
    if ((ulCoolingDeviation == 0) && (ulHeatingDeviation == 0))
    {
        ulMode = pParentStatus->pOutputConfigs[nCtrlID].ulMode;
    }
    else if (ulCoolingDeviation != 0)
    {
        if (ulCoolingDeviation < 100)
        {
            ulCoolingDeviation = 100;
        }
        ulMode = FTE_DOTECH_FX3D_MODE_COOLING;
    }
    else if (ulHeatingDeviation != 0)
    {
        if (ulHeatingDeviation < 100)
        {
            ulHeatingDeviation = 100;
        }
        ulMode = FTE_DOTECH_FX3D_MODE_HEATING;
    }
    else
    {
        xRet = FTE_RET_ERROR;
        goto error;
    }
    
    xRet = FTE_VALUE_getTemperature(&pParentStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0 + nCtrlID], &nCurrentTemperature);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }

    if (nCurrentTemperature != nTargetTemperature)
    {
        xRet = FTE_DOTECH_FX3D_writeReg(pParent, nCtrlID, nTargetTemperature / 10);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
    }
    
    if (pParentStatus->pOutputConfigs[nCtrlID].ulMode != ulMode)
    {
        xRet = FTE_DOTECH_FX3D_writeReg(pParent, nCtrlID * 8 + 2, ulMode);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
        
        pParentStatus->pOutputConfigs[nCtrlID].ulMode = ulMode;
    }    
    
    if ((ulMode == FTE_DOTECH_FX3D_MODE_COOLING) && (ulCoolingDeviation != 0))
    {
        xRet = FTE_DOTECH_FX3D_writeReg(pParent, nCtrlID * 8 + 3, ulCoolingDeviation / 10);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
        
        pParentStatus->pOutputConfigs[nCtrlID].ulDeviation = ulCoolingDeviation;
    }
    else if ((ulMode == FTE_DOTECH_FX3D_MODE_HEATING) && (ulHeatingDeviation != 0))
    {
        xRet = FTE_DOTECH_FX3D_writeReg(pParent, nCtrlID * 8 + 3, ulHeatingDeviation / 10);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
        
        pParentStatus->pOutputConfigs[nCtrlID].ulDeviation = ulHeatingDeviation;
    }    

error:
            
    nx_json_free(pxJSON);
    
    return  xRet;
}

FTE_RET FTE_DOTECH_FX3D_getChildConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    FTE_RET                     xRet;
    FTE_IFCE_CONFIG_PTR         pConfig;
    FTE_OBJECT_PTR              pParent;
    FTE_DOTECH_CONFIG_PTR       pParentConfig;
    FTE_DOTECH_FX3D_STATUS_PTR  pParentStatus;
    FTE_UCS_PTR                 pUCS;
    FTE_JSON_VALUE_PTR          pJOSNObject;
    FTE_JSON_VALUE_PTR          pJOSNValue;
    FTE_UINT16                  pRegs[1];
    
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL) && (pBuff != NULL));

    if ((FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_MULTI_VALUE) || (((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent == NULL))
    {
        return  FTE_RET_ERROR;
    }
    
    pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;

    FTE_UINT32 nCtrlID = (pConfig->nRegID - FTE_DOTECH_FX3D_FIELD_CTRL0);
    if (nCtrlID >= FTE_DOTECH_FX3D_CTRL_COUNT)
    {
        return  FTE_RET_INVALID_ID;
    }
    
    pParent = ((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent;
    pParentConfig = (FTE_DOTECH_CONFIG_PTR)pParent->pConfig;
    pParentStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pParent->pStatus;
  
    pUCS = FTE_UCS_get(pParentConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pParentConfig->xGUS.nUCSID);
        return  FTE_RET_ERROR;
    }
    
    xRet = FTE_UCS_MODBUS_getRegs(pUCS, pParentConfig->xGUS.nSensorID, nCtrlID, pRegs, 1, xGlobalConfig.ulRequestTimeout);
    if (xRet != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    FTE_VALUE_setTemperature(&pParentStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0 + nCtrlID], (*(int_16_ptr)&pRegs[0]) * 10);
    
    pJOSNObject = FTE_JSON_VALUE_createObject(3);
    if (pJOSNObject == NULL)
    {
        return  FTE_RET_INSUFFICIENT_MEMORY;
    }
    
#if 0
    FTE_CHAR                    pValueString[32];
    
    FTE_VALUE_toString(&pParentStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0 + nCtrlID], pValueString, sizeof(pValueString));
    pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringTargetTemperature, pJOSNValue);

    sprintf(pValueString, "%d.%02d", pParentStatus->pOutputConfigs[nCtrlID].ulDeviation / 100,
            pParentStatus->pOutputConfigs[nCtrlID].ulDeviation % 100);
    
    if (pParentStatus->pOutputConfigs[nCtrlID].ulMode == FTE_DOTECH_FX3D_MODE_COOLING)
    {
        pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        pJOSNValue = FTE_JSON_VALUE_createString("0");
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
    }
    if (pParentStatus->pOutputConfigs[nCtrlID].ulMode == FTE_DOTECH_FX3D_MODE_HEATING)
    {
        pJOSNValue = FTE_JSON_VALUE_createString("0");
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
    }
    else
    {
        pJOSNValue = FTE_JSON_VALUE_createString("0");
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        pJOSNValue = FTE_JSON_VALUE_createString("0");
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
    }
#else
    FTE_INT32                   nValue;

    FTE_VALUE_getTemperature(&pParentStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0 + nCtrlID], &nValue);
    
    pJOSNValue = FTE_JSON_VALUE_createFloat(nValue);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringTargetTemperature, pJOSNValue);
    
    if (pParentStatus->pOutputConfigs[nCtrlID].ulMode == FTE_DOTECH_FX3D_MODE_COOLING)
    {
        pJOSNValue = FTE_JSON_VALUE_createFloat(pParentStatus->pOutputConfigs[nCtrlID].ulDeviation);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        pJOSNValue = FTE_JSON_VALUE_createFloat(0);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
    }
    if (pParentStatus->pOutputConfigs[nCtrlID].ulMode == FTE_DOTECH_FX3D_MODE_HEATING)
    {
        pJOSNValue = FTE_JSON_VALUE_createFloat(0);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        pJOSNValue = FTE_JSON_VALUE_createFloat(pParentStatus->pOutputConfigs[nCtrlID].ulDeviation);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
    }
    else
    {
        pJOSNValue = FTE_JSON_VALUE_createFloat(0);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        pJOSNValue = FTE_JSON_VALUE_createFloat(0);
        FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
    }
#endif
    if (FTE_JSON_VALUE_buffSize(pJOSNObject) >= ulBuffLen)
    {
        FTE_JSON_VALUE_destroy(pJOSNObject);
        return  FTE_RET_ERROR;
    }
    
    FTE_JSON_VALUE_snprint(pBuff, ulBuffLen, pJOSNObject);
    
    FTE_JSON_VALUE_destroy(pJOSNObject);
  
    return  FTE_RET_OK;
}

_mqx_uint FTE_DOTECH_FX3D_loadConfig
(
    FTE_OBJECT_PTR pObj
)
{
    if (!bExtConfigLoaded)
    {
        if (FTE_CFG_DOTECH_getExtConfig(&xGlobalConfig, sizeof(xGlobalConfig)) != FTE_RET_OK)
        {
            return  FTE_RET_ERROR;
        }
        bExtConfigLoaded = TRUE;
    }

    return  FTE_RET_OK;
}

_mqx_uint FTE_DOTECH_FX3D_saveConfig
(
    void
)
{
    return  FTE_CFG_DOTECH_setExtConfig(&xGlobalConfig, sizeof(xGlobalConfig));
}

FTE_RET FTE_DOTECH_initDefaultExtConfig
(
    FTE_DOTECH_EXT_CONFIG_PTR pConfig
)
{
    ASSERT(pConfig != NULL);

    pConfig->ulLoopPeriod       = FTE_DOTECH_DEFAULT_LOOP_PERIOD;
    pConfig->ulUpdatePeriod     = FTE_DOTECH_DEFAULT_UPDATE_PERIOD;
    pConfig->ulRequestTimeout   = FTE_DOTECH_DEFAULT_REQUEST_TIMEOUT;
    pConfig->ulRetryCount       = FTE_DOTECH_DEFAULT_RETRY_COUNT;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_DOTECH_FX3D_get
(
    FTE_OBJECT_PTR  pObject, 
    FTE_UINT32         ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    FTE_DOTECH_FX3D_STATUS_PTR pStatus;
    ASSERT ((pObject != NULL) && (pValue != NULL));

    pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pObject->pStatus;
    
    if (ulIndex >= pStatus->xGUS.xCommon.nValueCount)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_VALUE_copy(pValue, &pStatus->xGUS.xCommon.pValue[ulIndex]);
}

FTE_RET FTE_DOTECH_FX3D_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    int i;
    FTE_DOTECH_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_DOTECH_FX3D_MAX ; i++)
    {
        if (pDevices[i].pObj == NULL)
        {   
            pDevice = &pDevices[i];
            break;
        }
    }
    
    if (pDevice == NULL)
    {
        return  FTE_RET_OBJECT_FULL;
    }
    
    FTE_DOTECH_FX3D_loadConfig(pObj);
    
    pDevice->xTaskID = _task_create(0, FTE_TASK_DOTECH, (FTE_UINT32)pObj->pConfig->xCommon.nID);
    if (pDevice->xTaskID <= 0)
    {
        return  FTE_RET_TASK_CREATION_FAILED;
    }             
                
    pDevice->pObj = pObj;    
    FTE_TASK_append(FTE_TASK_TYPE_MQX, pDevice->xTaskID);
    ulDeviceCount++;
                
    return  FTE_RET_OK;
}

FTE_RET     FTE_DOTECH_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT(pObj != NULL);
    
    FTE_UINT32 xRet;    
    FTE_DOTECH_CONFIG_PTR pConfig;
    FTE_DOTECH_FX3D_STATUS_PTR pStatus;
    FTE_UCS_PTR pUCS;
    FTE_UINT16 pRegs[FTE_DOTECH_FX3D_REG_STAT_MAX];
        
    pConfig = (FTE_DOTECH_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pObj->pStatus;
    
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return  FTE_RET_OBJECT_NOT_FOUND;
    }
 
    _time_get(&pStatus->xLastRequestTime);
    
    xRet = FTE_UCS_MODBUS_getRegs(pUCS, pConfig->xGUS.nSensorID, FTE_DOTECH_FX3D_REG_STAT_START, pRegs, FTE_DOTECH_FX3D_REG_STAT_MAX, xGlobalConfig.ulRequestTimeout);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    FTE_VALUE_setTemperature(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_TEMP0], ((int_16_ptr)pRegs)[FTE_DOTECH_FX3D_REG_STAT_CURR_TEMP - FTE_DOTECH_FX3D_REG_STAT_START] * 10);
    FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_STATE0], (pRegs[FTE_DOTECH_FX3D_REG_STAT - FTE_DOTECH_FX3D_REG_STAT_START] & 0x01));
    FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_STATE1], ((pRegs[FTE_DOTECH_FX3D_REG_STAT - FTE_DOTECH_FX3D_REG_STAT_START] >> 1) & 0x01));
#ifdef FTE_DOTECH_FX3D_FIELD_SENSOR_ALARM
    FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_SENSOR_ALARM], ((pRegs[FTE_DOTECH_FX3D_REG_STAT - FTE_DOTECH_FX3D_REG_STAT_START] >> 14) & 0x01));
#endif
    FTE_VALUE_setTemperature(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0], ((int_16_ptr)pRegs)[FTE_DOTECH_FX3D_REG_STAT_CTRL_TEMP0 - FTE_DOTECH_FX3D_REG_STAT_START] * 10);
    FTE_VALUE_setTemperature(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP1], ((int_16_ptr)pRegs)[FTE_DOTECH_FX3D_REG_STAT_CTRL_TEMP1 - FTE_DOTECH_FX3D_REG_STAT_START] * 10);
    
    _time_get(&pStatus->xLastUpdateTime);
    
    return  FTE_RET_OK;
}

void FTE_DOTECH_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_UCS_PTR pUCS;
    FTE_TIME_DELAY xDelay;
    FTE_DOTECH_CONFIG_PTR pConfig;
    FTE_DOTECH_FX3D_STATUS_PTR pStatus;
    FTE_UINT16 pRegs[FTE_DOTECH_FX3D_REG_CTRL_MAX];
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pConfig = (FTE_DOTECH_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pObj->pStatus;
    
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return;
    }
 
    pStatus->ulRetryCount = 0;
    _time_get(&pStatus->xLastUpdateTime);    
    
    while(TRUE)
    {
        xRet = FTE_UCS_MODBUS_getRegs(pUCS, pConfig->xGUS.nSensorID, FTE_DOTECH_FX3D_REG_CTRL_START, pRegs, FTE_DOTECH_FX3D_REG_CTRL_MAX, xGlobalConfig.ulRequestTimeout);
        if (xRet == FTE_RET_OK)
        {
            int i;
            
            for(i = 0 ; i < FTE_DOTECH_FX3D_CTRL_COUNT ; i++)
            {
                pStatus->pOutputConfigs[i].ulMode           = pRegs[i * 8 + 2];
                pStatus->pOutputConfigs[i].ulDeviation      = pRegs[i * 8 + 3] * 10;
                pStatus->pOutputConfigs[i].ulONDelay        = pRegs[i * 8 + 4];
                pStatus->pOutputConfigs[i].ulOFFMin         = pRegs[i * 8 + 5];
                pStatus->pOutputConfigs[i].ulONMin          = pRegs[i * 8 + 6];
                pStatus->pOutputConfigs[i].ulDeviationMode  = pRegs[i * 8 + 7];
                pStatus->pOutputConfigs[i].nMaxTemp         = (*(int_16_ptr)&pRegs[i * 8 + 8]) * 100;
                pStatus->pOutputConfigs[i].nMinTemp         = (*(int_16_ptr)&pRegs[i * 8 + 9]) * 100;
            }
            
            break;
        }
        
        _time_delay(1000);
    }

    FTE_TIME_DELAY_init(&xDelay, xGlobalConfig.ulLoopPeriod);
    
    while(TRUE)
    {
        TIME_STRUCT xCurrentTime;
        FTE_UINT32 ulElapsedUpdateTime = 0;

        _time_get(&xCurrentTime);
        
        ulElapsedUpdateTime = FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastUpdateTime);
        
        if (ulElapsedUpdateTime >= xGlobalConfig.ulUpdatePeriod)
        {   
            FTE_UINT32 ulElapsedRequestTime;
            
            ulElapsedRequestTime = FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastRequestTime);
            if( ulElapsedRequestTime >= xGlobalConfig.ulRequestTimeout)
            {
                xRet = FTE_DOTECH_update(pObj);
                if (xRet == FTE_RET_OK)
                {
                    FT_OBJ_STAT_incSucceed(&pStatus->xGUS.xCommon.xStatistics);
                    pStatus->ulRetryCount=0;
                }
                else if (pStatus->ulRetryCount >= xGlobalConfig.ulRetryCount)
                {
                    FT_OBJ_STAT_incFailed(&pStatus->xGUS.xCommon.xStatistics);
                    pStatus->ulRetryCount=0;
                }
                else
                {
                    pStatus->ulRetryCount++;
                }
                
            }
        }

        FTE_TIME_DELAY_waitingAndSetNext(&xDelay);
    }      
}

FTE_RET FTE_DOTECH_add
(
    FTE_UINT32 ulSlaveID, 
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    int i;
    FTE_DOTECH_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_DOTECH_FX3D_FIELD_MAX];
    FTE_UINT32                 ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    for(i = 0 ; i < FTE_DOTECH_FX3D_MAX ; i++)
    {
        if (pDevices[i].pObj != NULL)
        {
            if (((FTE_DOTECH_CONFIG_PTR)pDevices[i].pObj->pConfig)->xGUS.nSensorID== ulSlaveID)
            {
                return  FTE_RET_OK;
            }
        }
    }
    
    FTE_DOTECH_FX3D_defaultConfig.xGUS.nSensorID = ulSlaveID;
    
    pConfig = (FTE_DOTECH_CONFIG_PTR)FTE_CFG_OBJ_create((FTE_OBJECT_CONFIG_PTR)&FTE_DOTECH_FX3D_defaultConfig, pChildConfig, FTE_DOTECH_FX3D_FIELD_MAX, &ulChildCount);
    if (pConfig == NULL)
    {
        return  FTE_RET_INSUFFICIENT_MEMORY;
    }    
    
    pObj = FTE_OBJ_create((FTE_OBJECT_CONFIG_PTR)pConfig);
    if (pObj == NULL)
    {
        return  FTE_RET_INSUFFICIENT_MEMORY;
    }
    
    if (ulChildCount != 0)
    {
        for(i = 0 ; i < ulChildCount ; i++)
        {
            FTE_OBJECT_PTR  pChild;
            
            pChild = FTE_OBJ_create(pChildConfig[i]);
            if (pChild == NULL)
            {
                ERROR("The child object creation failed.\n");
            }
        }
    }
    
    *ppObj = pObj;
        
    return  FTE_RET_OK;    
}

FTE_RET     FTE_DOTECH_FX3D_writeReg
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32         ulIndex,
    FTE_INT32          nValue
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_RET     xRet;
    FTE_DOTECH_CONFIG_PTR pConfig;
    FTE_UCS_PTR pUCS;
    FTE_UINT16     i, usValue;
        
    pConfig = (FTE_DOTECH_CONFIG_PTR)pObj->pConfig;
    
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return  FTE_RET_OBJECT_NOT_FOUND;
    }
    
    usValue = *(FTE_UINT16_PTR)&nValue;
    
    for(i = 0 ; i < xGlobalConfig.ulRetryCount ;i++)
    {
        xRet = FTE_UCS_MODBUS_setReg(pUCS, pConfig->xGUS.nSensorID, ulIndex, usValue, xGlobalConfig.ulRequestTimeout);
        if (xRet == FTE_RET_OK)
        {
            break;
        }
    }
    
    return  xRet;
}
    

FTE_INT32 FTE_DOTECH_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_RET     xRet = SHELL_EXIT_SUCCESS;
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            int i;
            FTE_BOOL    bValue;
            
            printf("%8s %4s %6s %12s %12s\n", "ID", "SID", "Temp.", "Out1", "Out2");
            
            for(i = 0 ; i < FTE_DOTECH_FX3D_MAX ; i++)
            {
                FTE_DOTECH_CONFIG_PTR pConfig;
                FTE_DOTECH_FX3D_STATUS_PTR pStatus;
                char    pBuff[32];
                
                if (pDevices[i].pObj == NULL)
                {
                    continue;
                }
                
                pConfig = (FTE_DOTECH_CONFIG_PTR)pDevices[i].pObj->pConfig;
                pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pDevices[i].pObj->pStatus;
  
                printf("%8x %4d", pConfig->xGUS.xCommon.nID, pConfig->xGUS.nSensorID);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_TEMP0], pBuff, 32);
                printf(" %6s", pBuff);
                FTE_VALUE_getDIO(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_STATE0], &bValue);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0], pBuff, 32);
                printf(" %4s[%6s]", (bValue)?"on":"off", pBuff);
                FTE_VALUE_getDIO(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_STATE1], &bValue);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP1], pBuff, 32);
                printf(" %4s[%6s]\n", (bValue)?"on":"off", pBuff);
            }
            
            printf("\n[ Configuration ]\n");
            printf("Interval : %5d msec\n", xGlobalConfig.ulLoopPeriod);
            printf("Timeout : %5d msec\n", xGlobalConfig.ulRequestTimeout);
            printf("Retry : %5\n", xGlobalConfig.ulRetryCount);                   
        }
        break;
        
   case  2:
        {
            FTE_OBJECT_ID   xOID;
            FTE_OBJECT_PTR  pObject;
            
            if (!Shell_parse_hexnum(pArgv[1], &xOID))
            {
                bPrintUsage = TRUE;
                goto error;
            }
            
            pObject = FTE_OBJ_get(xOID);
            if (pObject == NULL)
            {
                printf("Object[%08x] not found!\n", xOID);
                goto error;
            }
            
            if (FTE_OBJ_TYPE(pObject) == FTE_OBJ_TYPE_MULTI_DOTECH_FX3D)
            {
                int i;
                FTE_DOTECH_FX3D_STATUS_PTR  pStatus;
                char    pBuff[32];
               
                pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pObject->pStatus;

                for(i = 0 ; i < 2 ; i++)
                {
                    printf("Output %d \n", i+1);
                    switch(pStatus->pOutputConfigs[i].ulMode)
                    {
                    case    FTE_DOTECH_FX3D_MODE_OFF:       printf("       Type : %7s\n", "off"); break;
                    case    FTE_DOTECH_FX3D_MODE_COOLING:   printf("       Type : %7s\n", "Cooling"); break;
                    case    FTE_DOTECH_FX3D_MODE_HEATING:   printf("       Type : %7s\n", "Heating"); break;
                    default:   printf("       Type : %7s\n", "unknown"); break;
                    }
                    
                    FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL_TEMP0+i], pBuff, 32);
                    printf("Temperature : %7s\n", pBuff);
                    printf("  Deviation : %4d.%02d\n", pStatus->pOutputConfigs[i].ulDeviation / 100, pStatus->pOutputConfigs[i].ulDeviation % 100);
                    printf("   ON Delay : %7d\n", pStatus->pOutputConfigs[i].ulONDelay);
                    printf("   OFF Min. : %7d\n", pStatus->pOutputConfigs[i].ulOFFMin);
                    printf("    ON Min. : %7d\n", pStatus->pOutputConfigs[i].ulONMin);
                    printf("       Mode : %7d\n", pStatus->pOutputConfigs[i].ulDeviationMode);
                    printf("  Temp Max. : %4d.%02d\n", pStatus->pOutputConfigs[i].nMaxTemp / 100, abs(pStatus->pOutputConfigs[i].nMaxTemp) % 100);
                    printf("  Temp Min. : %4d.%02d\n", pStatus->pOutputConfigs[i].nMinTemp / 100, abs(pStatus->pOutputConfigs[i].nMinTemp) % 100);
                }
            }
            else if (FTE_OBJ_TYPE(pObject) == FTE_OBJ_TYPE_MULTI_VALUE)
            {
                FTE_OBJECT_PTR  pParent;
                FTE_DOTECH_FX3D_STATUS_PTR  pStatus;
                FTE_UINT32 ulCtrlID;
                char    pBuff[32];
                
                ulCtrlID = ((FTE_IFCE_CONFIG_PTR)pObject->pConfig)->nRegID - FTE_DOTECH_FX3D_FIELD_CTRL0;
                
                pParent = ((FTE_IFCE_STATUS_PTR)pObject->pStatus)->pParent;
                pStatus = (FTE_DOTECH_FX3D_STATUS_PTR)pParent->pStatus;

                
                printf("Output %d \n", ulCtrlID+1);
                switch(pStatus->pOutputConfigs[ulCtrlID].ulMode)
                {
                case    FTE_DOTECH_FX3D_MODE_OFF: printf("     Type : %7s\n", "off"); break;
                case    FTE_DOTECH_FX3D_MODE_COOLING: printf("     Type : %7s\n", "Cooling"); break;
                case    FTE_DOTECH_FX3D_MODE_HEATING: printf("     Type : %7s\n", "Heating"); break;
                default:   printf("     Type : %7s\n", "unknown"); break;
                }
                
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_DOTECH_FX3D_FIELD_CTRL0+ulCtrlID], pBuff, 32);
                printf("Temperature : %7s\n", pBuff);
                printf("  Deviation : %4d.%02d\n", pStatus->pOutputConfigs[ulCtrlID].ulDeviation / 100, pStatus->pOutputConfigs[ulCtrlID].ulDeviation % 100);
                printf("   ON Delay : %7d\n", pStatus->pOutputConfigs[ulCtrlID].ulONDelay);
                printf("   OFF Min. : %7d\n", pStatus->pOutputConfigs[ulCtrlID].ulOFFMin);
                printf("    ON Min. : %7d\n", pStatus->pOutputConfigs[ulCtrlID].ulONMin);
                printf("       Mode : %7d\n", pStatus->pOutputConfigs[ulCtrlID].ulDeviationMode);
                printf("  Temp Max. : %4d.%02d\n", pStatus->pOutputConfigs[ulCtrlID].nMaxTemp / 100, abs(pStatus->pOutputConfigs[ulCtrlID].nMaxTemp) % 100);
                printf("  Temp Min. : %4d.%02d\n", pStatus->pOutputConfigs[ulCtrlID].nMinTemp / 100, abs(pStatus->pOutputConfigs[ulCtrlID].nMinTemp) % 100);
            }
        }
        break;
        
    case  3:
        {
            if (strcasecmp("add", pArgv[1]) == 0)
            {
                FTE_OBJECT_PTR pObj;
                FTE_UINT32 ulSlaveID;
                
                if (!Shell_parse_uint_32(pArgv[2], &ulSlaveID))
                {
                    bPrintUsage = TRUE;
                    goto error;
                }
       
                xRet = FTE_DOTECH_add(ulSlaveID, (FTE_OBJECT_PTR _PTR_)&pObj);
                if (xRet == FTE_RET_OK)
                {
                    printf("The object[%08x] is created successfully.\n", pObj->pConfig->xCommon.nID);
                }
            }
        }
        break;
        
    case  4:
        {
            if (strcasecmp("set", pArgv[1]) == 0)
            {
                if ((strcasecmp("i", pArgv[2]) == 0) || (strcasecmp("interval", pArgv[2]) == 0))
                {
                    FTE_UINT32 ulInterval;
                    
                    if (!Shell_parse_uint_32(pArgv[3], &ulInterval))
                    {
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    if (ulInterval < 1000)                    
                    {
                        printf("Interval is too short.[ >= 1000 ms ]\n");
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    xGlobalConfig.ulLoopPeriod = ulInterval;
                }
                else if ((strcasecmp("t", pArgv[2]) == 0) || (strcasecmp("timeout", pArgv[2]) == 0))
                {
                    FTE_UINT32 ulTimeout;
                    
                    if (!Shell_parse_uint_32(pArgv[3], &ulTimeout))
                    {
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    if (ulTimeout >= xGlobalConfig.ulLoopPeriod)                    
                    {
                        printf("Timeout is too long.[ timeout[ %d ms ] < interval[ %d ms ] ]\n", 
                               ulTimeout,
                               xGlobalConfig.ulLoopPeriod);
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    xGlobalConfig.ulRequestTimeout = ulTimeout;
                }
                else 
                {
                    bPrintUsage = TRUE;
                    goto error;
                }
                
                xRet = FTE_DOTECH_FX3D_saveConfig();
                if (xRet != FTE_RET_OK)
                {
                    printf("FX3D global configuration is saved.\n");
                }
            }
        }
        break;
    
    case  5:
        {
            if (strcasecmp("set", pArgv[1]) == 0)
            {
                FTE_OBJECT_ID   xOID;
                FTE_OBJECT_PTR  pObject;
                
               if (!Shell_parse_hexnum(pArgv[2], &xOID))
                {
                    bPrintUsage = TRUE;
                    goto error;
                }
                
                pObject = FTE_OBJ_get(xOID);
                if (pObject == NULL)
                {
                    printf("Object[%08x] not found!\n", xOID);
                    goto error;
                }
                
                if (strcasecmp("out1", pArgv[3]) == 0)
                {
                    FTE_FLOAT   fValue = 0;
                    FTE_INT32   nValue;
                    
                    if (!fte_parse_float(pArgv[3], &fValue))
                    {
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    nValue = (FTE_INT32)(fValue * 10);
                    
                    xRet = FTE_DOTECH_FX3D_writeReg(pObject, FTE_DOTECH_FX3D_REG_CTRL_TEMP0, nValue);
                    if (xRet != FTE_RET_OK)
                    {    
                        printf("Out 1 set failed!\n");
                    }
                }
                else if (strcasecmp("out2", pArgv[3]) == 0)
                {
                    FTE_FLOAT   fValue = 0;
                    FTE_INT32  nValue;
                    
                    if (!fte_parse_float(pArgv[3], &fValue))
                    {
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    nValue = (FTE_INT32)(fValue * 10);
                    
                    xRet = FTE_DOTECH_FX3D_writeReg(pObject, FTE_DOTECH_FX3D_REG_CTRL_TEMP1, nValue);
                    if (xRet != FTE_RET_OK)
                    {    
                        printf("Out 2 set failed!\n");
                    }
                }
                else 
                {
                    bPrintUsage = TRUE;
                    goto error;
                }
            }
        }
    
    }
    
error:    
    if (bPrintUsage)  
    {
        if (bShortHelp)  
        {
            printf("%s <commands>\n", pArgv[0]);
        } 
        else  
        {
        }
    }
    
    return  FTE_RET_OK;
} 
#endif