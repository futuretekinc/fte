#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "fte_value.h"

#ifdef  FTE_ELT_AQM100_SUPPORTED

#ifndef FTE_ELT_DEFAULT_LOOP_PERIOD
#define FTE_ELT_DEFAULT_LOOP_PERIOD        2000 // ms
#endif

#ifndef FTE_ELT_DEFAULT_UPDATE_INTERVAL
#define FTE_ELT_DEFAULT_UPDATE_INTERVAL    10000 // ms
#endif

#ifndef FTE_ELT_DEFAULT_REQUEST_TIMEOUT
#define FTE_ELT_DEFAULT_REQUEST_TIMEOUT    500
#endif

#ifndef FTE_ELT_DEFAULT_RETRY_COUNT
#define FTE_ELT_DEFAULT_RETRY_COUNT        3
#endif


#define FTE_ELT_DEVICE_MAX  2

#define FTE_ELT_AQM100M_REG_ADDR_START          0x50
#define FTE_ELT_AQM100M_REG_ADDR_CO2            0x50
#define FTE_ELT_AQM100M_REG_ADDR_TEMPERATURE    0x51
#define FTE_ELT_AQM100M_REG_ADDR_HUMIDITY       0x52
#define FTE_ELT_AQM100M_REG_ADDR_VOC            0x53

typedef struct  FTE_ELT_DEVICE_STRUCT
{
    _task_id        xTaskID;
    FTE_OBJECT_PTR  pObj;
}   FTE_ELT_DEVICE, _PTR_ FTE_ELT_DEVICE_PTR;

static const FTE_IFCE_CONFIG FTE_ELT_AQM100_CO2_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CO2, 0),
        .pName      = "AQM100-CO2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0),
    .nRegID     = FTE_ELT_AQM100_FIELD_CO2,
    .nInterval  = FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_ELT_AQM100_TEMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "AQM100-TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0),
    .nRegID     = FTE_ELT_AQM100_FIELD_TEMP,
    .nInterval  = FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_ELT_AQM100_HUMI_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0),
        .pName      = "AQM100-HUMI",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0),
    .nRegID     = FTE_ELT_AQM100_FIELD_HUMI,
    .nInterval  = FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_ELT_AQM100_VOC_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VOC, 0),
        .pName      = "AQM100-VOC",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0),
    .nRegID     = FTE_ELT_AQM100_FIELD_VOC,
    .nInterval  = FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL
};

static const FTE_OBJECT_CONFIG_PTR FTE_ELT_AQM100_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_ELT_AQM100_CO2_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_ELT_AQM100_TEMP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_ELT_AQM100_HUMI_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_ELT_AQM100_VOC_defaultConfig
};

FTE_ELT_CONFIG FTE_ELT_AQM100_defaultConfig =
{
    .xGUS   =
    {
        .xCommon    =
        {
            .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0x0001),
            .pName      = "AQM100",
            .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
            .ulChild    = sizeof(FTE_ELT_AQM100_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_ELT_AQM100_defaultChildConfigs
        },
        .nModel     = FTE_GUS_MODEL_ELT_AQM100,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL
    }
};

FTE_ELT_CONFIG FTE_ELT_AQM100M_defaultConfig =
{
    .xGUS   =
    {
        .xCommon    =
        {
            .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100M, 0x0001),
            .pName      = "AQM100M",
            .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
            .ulChild    = sizeof(FTE_ELT_AQM100_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_ELT_AQM100_defaultChildConfigs
        },
        .nModel     = FTE_GUS_MODEL_ELT_AQM100M,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = FTE_ELT_AQM100_DEFAULT_UPDATE_INTERVAL
    }
};

FTE_VALUE_TYPE  FTE_ELT_AQM100_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_HUMIDITY,
    FTE_VALUE_TYPE_DIO
};

const FTE_GUS_MODEL_INFO    FTE_ELT_AQM100_GUSModelInfo = 
{
    .nModel         = FTE_GUS_MODEL_ELT_AQM100,
    .pName          = "ELT AQM100",
    .xFlags         = 0,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_ELT_AQM100_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_ELT_AQM100_DEFAULT_DATABITS,
        .nParity    =   FTE_ELT_AQM100_DEFAULT_PARITY,
        .nStopBits  =   FTE_ELT_AQM100_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_ELT_AQM100_DEFAULT_FULL_DUPLEX
    },
    .nMaxResponseTime=FTE_ELT_AQM100_REPONSE_TIME,
    .nFieldCount    = FTE_ELT_AQM100_FIELD_MAX,
    .pValueTypes    = FTE_ELT_AQM100_valueTypes,
    .fCreate        = FTE_ELT_AQM100_create,
    .fAttach        = FTE_ELT_attach,
    .fDetach        = FTE_ELT_detach,
    .fGet           = FTE_ELT_get,
    .fUpdate        = FTE_ELT_AQM100_update
};

const FTE_GUS_MODEL_INFO    FTE_ELT_AQM100M_GUSModelInfo = 
{
    .nModel         = FTE_GUS_MODEL_ELT_AQM100M,
    .pName          = "ELT AQM100M",
    .xFlags         = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_ELT_AQM100M_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_ELT_AQM100M_DEFAULT_DATABITS,
        .nParity    =   FTE_ELT_AQM100M_DEFAULT_PARITY,
        .nStopBits  =   FTE_ELT_AQM100M_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_ELT_AQM100M_DEFAULT_FULL_DUPLEX
    },
    .nMaxResponseTime=FTE_ELT_AQM100_REPONSE_TIME,
    .nFieldCount    = FTE_ELT_AQM100_FIELD_MAX,
    .pValueTypes    = FTE_ELT_AQM100_valueTypes,
    .fCreate        = FTE_ELT_AQM100M_create,
    .fAttach        = FTE_ELT_attach,
    .fDetach        = FTE_ELT_detach,
    .fGet           = FTE_ELT_get,
    .fUpdate        = FTE_ELT_AQM100M_update
};

static  
FTE_ELT_EXT_CONFIG xGlobalConfig = 
{
    .ulLoopPeriod       = FTE_ELT_DEFAULT_LOOP_PERIOD,
    .ulUpdatePeriod     = FTE_ELT_DEFAULT_UPDATE_INTERVAL,
    .ulRequestTimeout   = FTE_ELT_DEFAULT_REQUEST_TIMEOUT,
    .ulRetryCount       = FTE_ELT_DEFAULT_RETRY_COUNT
};

static
FTE_ELT_DEVICE  pDevices[FTE_ELT_DEVICE_MAX];

static
FTE_UINT32      ulDeviceCount = 0;

FTE_RET   FTE_ELT_init
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_RET     xRet;
    FTE_INT32   i;
    FTE_ELT_DEVICE_PTR   pDevice = NULL;

    for(i = 0 ; i < FTE_ELT_DEVICE_MAX ; i++)
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
    
    xRet = FTE_TASK_create(FTE_TASK_ELT, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
    if (xRet != FTE_RET_OK)
    {
        return  FTE_RET_TASK_CREATION_FAILED;
    }             
                
    pDevice->pObj = pObj;    
    ulDeviceCount++;
                
    return  FTE_RET_OK;
}

FTE_RET   FTE_ELT_final
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_ELT_DEVICE_PTR   pDevice = NULL;

    for(i = 0 ; i < FTE_ELT_DEVICE_MAX ; i++)
    {
        if (pDevices[i].pObj == pObj)
        {   
            pDevice = &pDevices[i];
            break;
        }
    }
    
    if (pDevice == NULL)
    {
        return  FTE_RET_OBJECT_NOT_FOUND;
    }    
    
    if (pDevice->xTaskID != 0)
    {
        FTE_TASK_remove(pDevice->xTaskID);
        _task_destroy(pDevice->xTaskID);
    }
    
    pDevice->xTaskID = 0;
    pDevice->pObj = NULL;    
    --ulDeviceCount;
                
    return  FTE_RET_OK;
}

FTE_RET FTE_ELT_create
(
    FTE_UINT32      ulType,
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    ASSERT((pSlaveID != NULL) && (ppObj != NULL));
    
    int i;
    FTE_RET                 xRet;
    FTE_OBJECT_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pBaseConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_ELT_AQM100_FIELD_MAX];
    FTE_UINT32              ulChildCount = 0;
    FTE_UINT32              ulSlaveID;
    FTE_OBJECT_PTR          pObj;
    
    xRet = FTE_strToUINT32(pSlaveID, &ulSlaveID);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    for(i = 0 ; i < FTE_ELT_DEVICE_MAX ; i++)
    {
        if (pDevices[i].pObj != NULL)
        {
            if (((FTE_ELT_CONFIG_PTR)pDevices[i].pObj->pConfig)->xGUS.nSensorID== ulSlaveID)
            {
                return  FTE_RET_OK;
            }
        }
    }
    
    switch(ulType)
    {
    case    FTE_OBJ_TYPE_MULTI_AQM100:
        {
            pBaseConfig = (FTE_OBJECT_CONFIG_PTR)&FTE_ELT_AQM100_defaultConfig;
        }
        break;
        
    case    FTE_OBJ_TYPE_MULTI_AQM100M:
        {
            pBaseConfig = (FTE_OBJECT_CONFIG_PTR)&FTE_ELT_AQM100M_defaultConfig;
        }
        break;
        
    default:
        return  FTE_RET_INVALID_MODEL;
    }
        
    ((FTE_ELT_CONFIG_PTR)pBaseConfig)->xGUS.nSensorID = ulSlaveID;
    
    xRet = FTE_CFG_OBJ_create(pBaseConfig, &pConfig, pChildConfig, FTE_ELT_AQM100_FIELD_MAX, &ulChildCount);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }    
    
    pObj = FTE_OBJ_create(pConfig);
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

FTE_RET   FTE_ELT_attach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_RET xRet;
    FTE_ELT_CONFIG_PTR    pConfig;
    FTE_ELT_STATUS_PTR    pStatus;
    FTE_UCS_PTR  pUCS;
    
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));

    pConfig = (FTE_ELT_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_ELT_STATUS_PTR)pObj->pStatus;    
    
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
    
    FTE_ELT_init(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_ELT_detach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_ELT_STATUS_PTR  pStatus;

    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));

    pStatus = (FTE_ELT_STATUS_PTR)pObj->pStatus;    
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
 
    FTE_ELT_final(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_ELT_get
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL) && (pValue != NULL));

    FTE_ELT_STATUS_PTR pStatus = (FTE_ELT_STATUS_PTR)pObj->pStatus;
    
    if (ulIndex >= pStatus->xGUS.xCommon.nValueCount)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_VALUE_copy(pValue, &pStatus->xGUS.xCommon.pValue[ulIndex]);
}

FTE_RET FTE_ELT_AQM100M_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    return  FTE_ELT_create(FTE_OBJ_TYPE_MULTI_AQM100M, pSlaveID, ppObj);
}

FTE_RET     FTE_ELT_AQM100M_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    FTE_UINT32 xRet;
    FTE_ELT_CONFIG_PTR pConfig;
    FTE_ELT_STATUS_PTR pStatus;
    FTE_INT16   pRegs[FTE_ELT_AQM100_FIELD_MAX];
    FTE_UCS_UART_CONFIG xUARTConfig;
        
    pConfig = (FTE_ELT_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_ELT_STATUS_PTR)pObj->pStatus;
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);
    
    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);    
        
    xRet = FTE_UCS_MODBUS_getRegs(pStatus->xGUS.pUCS, pConfig->xGUS.nSensorID, FTE_ELT_AQM100M_REG_ADDR_START, (FTE_UINT16_PTR)pRegs, FTE_ELT_AQM100_FIELD_MAX, xGlobalConfig.ulRequestTimeout);
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
        
    if (xRet != FTE_RET_OK)
    {
        if (pStatus->ulRetryCount >= xGlobalConfig.ulRetryCount)
        {
            FT_OBJ_STAT_incFailed(&pStatus->xGUS.xCommon.xStatistics);
            pStatus->ulRetryCount=0;
        }
        else
        {
            pStatus->ulRetryCount++;
        }
        
        return  xRet;
    }
    
    FTE_VALUE_setPPM(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_CO2], pRegs[FTE_ELT_AQM100_FIELD_CO2]);
    FTE_VALUE_setTemperature(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_TEMP], pRegs[FTE_ELT_AQM100_FIELD_TEMP]* 10);
    FTE_VALUE_setHumidity(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_HUMI], pRegs[FTE_ELT_AQM100_FIELD_HUMI] * 100);
    FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_VOC], (pRegs[FTE_ELT_AQM100_FIELD_VOC] == 0));
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    
    
    FT_OBJ_STAT_incSucceed(&pStatus->xGUS.xCommon.xStatistics);
    pStatus->ulRetryCount=0;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_ELT_AQM100_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    return  FTE_ELT_create(FTE_OBJ_TYPE_MULTI_AQM100, pSlaveID, ppObj);
}


FTE_RET     FTE_ELT_AQM100_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    FTE_ELT_STATUS_PTR pStatus;
//    FTE_UINT8       pReqBuff[6] = { 0xF5, 0xFC, 0x00, 0xF1, 0x05, 0x00};
    FTE_UINT8       pRcvdBuff[24];
    FTE_INT32       nRcvdLen;
    FTE_UINT8_PTR   pFrame;
    FTE_INT32             nFrameStart;
    FTE_UINT32      nCO2 = 0;
    FTE_INT32       nTemp = 0;
    FTE_UINT32      nHumi = 0;
    FTE_UINT32      nVOC = 0;
    FTE_UINT8       nCRC = 0;
    FTE_UINT8       nFlag= 0;
    FTE_UCS_UART_CONFIG xUARTConfig;

    pStatus = (FTE_ELT_STATUS_PTR)pObj->pStatus;
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);    


    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);    
        
    nRcvdLen = FTE_UCS_recvLast(pStatus->xGUS.pUCS, pRcvdBuff, sizeof(pRcvdBuff));
    FTE_UCS_clear(pStatus->xGUS.pUCS);    
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
    
    nFrameStart = -1;
    for(FTE_INT32 i = nRcvdLen - 11 ; i >= 0; i--)
    {
        if ((pRcvdBuff[i] == 0xFA) && (pRcvdBuff[i + 11] == 0x05))
        {
            nFrameStart = i;
            break;
        }
    }

    if (nFrameStart < 0)
    {
        pStatus->xGUS.xRet = FTE_RET_ERROR;
        return  FTE_RET_ERROR;
    }
    
    pFrame = &pRcvdBuff[nFrameStart];
    
    for(FTE_INT32 i = 0 ; i < 10 ; i++)
    {
        nCRC += pFrame[i];
    }
    
    if (nCRC != pFrame[10])
    {
        pStatus->xGUS.xRet = FTE_RET_ERROR;
        return  FTE_RET_ERROR;
    }
    
    nFlag = pFrame[3];
    nCO2  = ((uint_32)pFrame[4] << 8) | pFrame[5];
    nTemp = (((int_32)pFrame[6] << 8) | pFrame[7]) * 10;
    nHumi = (uint_32)pFrame[8] * 100;
    switch (pFrame[9])
    {
    case 0x00:     nVOC = 0; break;
    case 0x10:     nVOC = 1; break;
    case 0x40:     nVOC = 2; break;
    default:       nVOC = 3; break;
    }
    
    if (nFlag & 0x08)
    {
        FTE_VALUE_setPPM(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_CO2], nCO2);
    }
    else
    {
        FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_CO2], FALSE);
    }
    
    if (nFlag & 0x04)
    {
        FTE_VALUE_setTemperature(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_TEMP], nTemp);
    }
    else
    {
        FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_TEMP], FALSE);
    }
    
    if (nFlag & 0x02)
    {
        FTE_VALUE_setHumidity(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_HUMI], nHumi);
    }
    else
    {
        FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_HUMI], FALSE);
    }

    if (nFlag & 0x01)
    {
        FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_VOC], nVOC);
    }
    else
    {
        FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_VOC], FALSE);
    }
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    
    
    FT_OBJ_STAT_incSucceed(&pStatus->xGUS.xCommon.xStatistics);
    pStatus->ulRetryCount=0;
    
    return  FTE_RET_OK;
}

void FTE_ELT_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET         xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_TIME_DELAY xDelay;
    FTE_ELT_CONFIG_PTR pConfig;
    FTE_ELT_STATUS_PTR pStatus;
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pConfig = (FTE_ELT_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_ELT_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->xGUS.pModelInfo->fUpdate == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return;
    }

    if (pStatus->xGUS.pUCS != NULL)
    {
        if (FTE_OBJ_TYPE(pObj) == FTE_OBJ_TYPE_MULTI_AQM100)
        {
            FTE_UCS_setDuplexMode(pStatus->xGUS.pUCS, FTE_ELT_AQM100_DEFAULT_FULL_DUPLEX);
            FTE_UCS_setBaudrate(pStatus->xGUS.pUCS, FTE_ELT_AQM100_DEFAULT_BAUDRATE);
            FTE_UCS_setDatabits(pStatus->xGUS.pUCS, FTE_ELT_AQM100_DEFAULT_DATABITS);
            FTE_UCS_setParity(pStatus->xGUS.pUCS, FTE_ELT_AQM100_DEFAULT_PARITY);
            FTE_UCS_setStopbits(pStatus->xGUS.pUCS, FTE_ELT_AQM100_DEFAULT_STOPBITS);
        }
        else if (FTE_OBJ_TYPE(pObj) == FTE_OBJ_TYPE_MULTI_AQM100M)
        {
            FTE_UCS_setDuplexMode(pStatus->xGUS.pUCS, FTE_ELT_AQM100M_DEFAULT_FULL_DUPLEX);
            FTE_UCS_setBaudrate(pStatus->xGUS.pUCS, FTE_ELT_AQM100M_DEFAULT_BAUDRATE);
            FTE_UCS_setDatabits(pStatus->xGUS.pUCS, FTE_ELT_AQM100M_DEFAULT_DATABITS);
            FTE_UCS_setParity(pStatus->xGUS.pUCS, FTE_ELT_AQM100M_DEFAULT_PARITY);
            FTE_UCS_setStopbits(pStatus->xGUS.pUCS, FTE_ELT_AQM100M_DEFAULT_STOPBITS);
        }
    }
    

    pStatus->ulRetryCount = 0;
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);        
    
    FTE_TIME_DELAY_init(&xDelay, xGlobalConfig.ulLoopPeriod);
    
    while(TRUE)
    {
        FTE_TIME    xCurrentTime;
        FTE_INT32   nElapsedUpdateTime = 0;

        FTE_TIME_getCurrent(&xCurrentTime);
        
        FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastUpdateTime, &nElapsedUpdateTime);        
        if (nElapsedUpdateTime >= xGlobalConfig.ulUpdatePeriod)
        {   
            FTE_INT32 nElapsedRequestTime;
            
            FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastRequestTime, &nElapsedRequestTime);
            if( nElapsedRequestTime >= xGlobalConfig.ulUpdatePeriod)
            {
                xRet = pStatus->xGUS.pModelInfo->fUpdate(pObj);
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

FTE_INT32 FTE_ELT_SHELL_cmd
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
            
            printf("%8s %4s %8s %8s %8s\n", "ID", "SID", "CO2", "Temp.", "Humi.");
            
            for(i = 0 ; i < FTE_ELT_DEVICE_MAX ; i++)
            {
                FTE_ELT_CONFIG_PTR pConfig;
                FTE_ELT_STATUS_PTR pStatus;
                FTE_CHAR pBuff[32];
                
                if (pDevices[i].pObj == NULL)
                {
                    continue;
                }
                
                pConfig = (FTE_ELT_CONFIG_PTR)pDevices[i].pObj->pConfig;
                pStatus = (FTE_ELT_STATUS_PTR )pDevices[i].pObj->pStatus;
  
                printf("%8x %4d", pConfig->xGUS.xCommon.nID, pConfig->xGUS.nSensorID);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_CO2], pBuff, 32);
                printf(" %8s", pBuff);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_TEMP], pBuff, 32);
                printf(" %8s", pBuff);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_ELT_AQM100_FIELD_HUMI], pBuff, 32);
                printf(" %8s\n", pBuff);
            }
            
            printf("\n[ Configuration ]\n");
            printf("Interval : %5d msec\n", xGlobalConfig.ulLoopPeriod);
            printf(" Timeout : %5d msec\n", xGlobalConfig.ulRequestTimeout);
            printf("   Retry : %5d\n", xGlobalConfig.ulRetryCount);                   
        }
        break;        
   
    }

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
    
    return  xRet;
} 
#endif