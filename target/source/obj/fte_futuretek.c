#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h" 

static  FTE_RET FTE_FUTURETEK_final(FTE_OBJECT_PTR  pObj);
static  FTE_RET FTE_FUTURETEK_run(FTE_OBJECT_PTR  pObj);
static  FTE_RET FTE_FUTURETEK_stop(FTE_OBJECT_PTR  pObj);
static  FTE_RET FTE_FUTURETEK_attach(FTE_OBJECT_PTR pObj);
static  FTE_RET FTE_FUTURETEK_detach(FTE_OBJECT_PTR pObj);
static  FTE_RET FTE_FUTURETEK_get(FTE_OBJECT_PTR pObj, FTE_UINT32 ulIndex, FTE_VALUE_PTR pValue);
static  FTE_RET FTE_FUTURETEK_set(FTE_OBJECT_PTR  pObj, FTE_UINT32      ulIndex, FTE_VALUE_PTR   pValue);

static  FTE_RET FTE_FUTURETEK_MS10_run(FTE_OBJECT_PTR pObj);
static  FTE_RET FTE_FUTURETEK_MS10_update(FTE_OBJECT_PTR pObj);

static  FTE_RET FTE_FUTURETEK_KM21B20_run(FTE_OBJECT_PTR pObj);
static  FTE_RET FTE_FUTURETEK_KM21B20_update(FTE_OBJECT_PTR pObj);

static  FTE_RET FTE_FUTURETEK_JCG06_run(FTE_OBJECT_PTR pObj);
static  FTE_RET FTE_FUTURETEK_JCG06_update(FTE_OBJECT_PTR pObj);
static  FTE_RET FTE_FUTURETEK_JCG06_set(FTE_OBJECT_PTR  pObj, FTE_UINT32      ulIndex, FTE_VALUE_PTR   pValue);
static  FTE_RET FTE_FUTURETEK_JCG06_resetRainfall(FTE_OBJECT_PTR  pObj);

typedef struct  FTE_FUTURETEK_DEVICE_STRUCT
{
    _task_id        xTaskID;
    FTE_OBJECT_PTR  pObj;
}   FTE_FUTURETEK_DEVICE, _PTR_ FTE_FUTURETEK_DEVICE_PTR;

static const 
FTE_IFCE_CONFIG    FTE_FUTURETEK_MS10_moistureConfig =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0),
        .pName  = "MS10-MOISTURE",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_MS10, 0),
    .nRegID     = FTE_FUTURETEK_MS10_REG_VALUE_MOISTURE,
    .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_OBJECT_CONFIG_PTR FTE_FUTURETEK_MS10_sensorConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_MS10_moistureConfig
};

FTE_FUTURETEK_CONFIG FTE_FUTURETEK_MS10_defaultConfig =
{
    .xGUS       =
    {
        .xCommon    =
        {
            .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_MS10, 0x0000),
            .pName  = "MS10",
            .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
            .ulChild= sizeof(FTE_FUTURETEK_MS10_sensorConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_FUTURETEK_MS10_sensorConfigs
        },
        .nModel     = FTE_GUS_MODEL_FUTURETE_MS10,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL,
    }
};

FTE_VALUE_TYPE  FTE_FUTURETEK_MS10_valueTypes[] =
{
    FTE_VALUE_TYPE_HUMIDITY
};

const 
FTE_GUS_MODEL_INFO    FTE_FUTURETEK_MS10_GUSModelInfo = 
{
    .nModel             = FTE_GUS_MODEL_FUTURETE_MS10,
    .pName              = "FUTURETEK MS10",
    .xFlags             = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_FUTURETEK_MS10_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_FUTURETEK_MS10_DEFAULT_DATABITS,
        .nParity    =   FTE_FUTURETEK_MS10_DEFAULT_PARITY,
        .nStopBits  =   FTE_FUTURETEK_MS10_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_FUTURETEK_MS10_DEFAULT_FULL_DUPLEX
    },
    .nMaxResponseTime   = FTE_FUTURETEK_DEFAULT_REQUEST_TIMEOUT,
    .nFieldCount    = FTE_FUTURETEK_MS10_REG_VALUE_MAX,
    .pValueTypes    = FTE_FUTURETEK_MS10_valueTypes,
    .fInit         = FTE_FUTURETEK_init, 
    .fFinal        = FTE_FUTURETEK_final, 
    .fCreate       = FTE_FUTURETEK_MS10_create,
    .fRun          = FTE_FUTURETEK_MS10_run,
    .fStop         = FTE_FUTURETEK_stop,
    .fAttach       = FTE_FUTURETEK_attach,
    .fDetach       = FTE_FUTURETEK_detach,
    .fGet          = FTE_FUTURETEK_get
};


///////////////////////////////////////////////////////////////////////////////
static const 
FTE_IFCE_CONFIG    FTE_FUTURETEK_KM21B20_acidityConfig =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0),
        .pName  = "KM21B20-ACIDITY",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_KM21B20, 0),
    .nRegID     = FTE_FUTURETEK_KM21B20_REG_VALUE_ACIDITY,
    .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_OBJECT_CONFIG_PTR FTE_FUTURETEK_KM21B20_sensorConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_KM21B20_acidityConfig
};

FTE_FUTURETEK_CONFIG FTE_FUTURETEK_KM21B20_defaultConfig =
{
    .xGUS       =
    {
        .xCommon    =
        {
            .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_KM21B20, 0x0000),
            .pName  = "KM21B20",
            .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
            .ulChild= sizeof(FTE_FUTURETEK_KM21B20_sensorConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_FUTURETEK_KM21B20_sensorConfigs
        },
        .nModel     = FTE_GUS_MODEL_FUTURETE_KM21B20,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL,
    }
};

FTE_VALUE_TYPE  FTE_FUTURETEK_KM21B20_valueTypes[] =
{
    FTE_VALUE_TYPE_PH
};

const 
FTE_GUS_MODEL_INFO    FTE_FUTURETEK_KM21B20_GUSModelInfo = 
{
    .nModel             = FTE_GUS_MODEL_FUTURETE_KM21B20,
    .pName              = "FUTURETEK KM21B20",
    .xFlags             = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_FUTURETEK_KM21B20_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_FUTURETEK_KM21B20_DEFAULT_DATABITS,
        .nParity    =   FTE_FUTURETEK_KM21B20_DEFAULT_PARITY,
        .nStopBits  =   FTE_FUTURETEK_KM21B20_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_FUTURETEK_KM21B20_DEFAULT_FULL_DUPLEX
    },
    .nMaxResponseTime   = FTE_FUTURETEK_DEFAULT_REQUEST_TIMEOUT,
    .nFieldCount    = FTE_FUTURETEK_KM21B20_REG_VALUE_MAX,
    .pValueTypes    = FTE_FUTURETEK_KM21B20_valueTypes,
    .fInit         = FTE_FUTURETEK_init, 
    .fFinal        = FTE_FUTURETEK_final, 
    .fCreate       = FTE_FUTURETEK_KM21B20_create,
    .fRun          = FTE_FUTURETEK_KM21B20_run,
    .fStop         = FTE_FUTURETEK_stop,
    .fAttach       = FTE_FUTURETEK_attach,
    .fDetach       = FTE_FUTURETEK_detach,
    .fGet          = FTE_FUTURETEK_get
};


///////////////////////////////////////////////////////////////////////////////
static const 
FTE_IFCE_CONFIG    FTE_FUTURETEK_JCG06_railfallConfig =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0),
        .pName  = "JCG06-RF",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_JCG06, 0),
    .nRegID     = FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL,
    .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG    FTE_FUTURETEK_JCG06_railfallPerHourConfig =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0),
        .pName  = "JCG06-RFPH",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_JCG06, 0),
    .nRegID     = FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL_PER_HOUR,
    .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG    FTE_FUTURETEK_JCG06_resetConfig =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName  = "JCG06-RESET",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_JCG06, 0),
    .nRegID     = FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL_RESET,
    .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_OBJECT_CONFIG_PTR FTE_FUTURETEK_JCG06_sensorConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_JCG06_railfallConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_JCG06_railfallPerHourConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_JCG06_resetConfig
};

FTE_FUTURETEK_CONFIG FTE_FUTURETEK_JCG06_defaultConfig =
{
    .xGUS       =
    {
        .xCommon    =
        {
            .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_FUTURETEK_JCG06, 0x0000),
            .pName  = "JCG06",
            .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
            .ulChild= sizeof(FTE_FUTURETEK_JCG06_sensorConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_FUTURETEK_JCG06_sensorConfigs
        },
        .nModel     = FTE_GUS_MODEL_FUTURETE_JCG06,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL,
    }
};

FTE_VALUE_TYPE  FTE_FUTURETEK_JCG06_valueTypes[] =
{
    FTE_VALUE_TYPE_MM,
    FTE_VALUE_TYPE_MM,
    FTE_VALUE_TYPE_DIO
};

const 
FTE_GUS_MODEL_INFO    FTE_FUTURETEK_JCG06_GUSModelInfo = 
{
    .nModel             = FTE_GUS_MODEL_FUTURETE_JCG06,
    .pName              = "FUTURETEK JCG06",
    .xFlags             = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_FUTURETEK_JCG06_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_FUTURETEK_JCG06_DEFAULT_DATABITS,
        .nParity    =   FTE_FUTURETEK_JCG06_DEFAULT_PARITY,
        .nStopBits  =   FTE_FUTURETEK_JCG06_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_FUTURETEK_JCG06_DEFAULT_FULL_DUPLEX
    },
    .nMaxResponseTime   = FTE_FUTURETEK_DEFAULT_REQUEST_TIMEOUT,
    .nFieldCount    = FTE_FUTURETEK_JCG06_REG_VALUE_MAX,
    .pValueTypes    = FTE_FUTURETEK_JCG06_valueTypes,
    .fInit         = FTE_FUTURETEK_init, 
    .fFinal        = FTE_FUTURETEK_final, 
    .fCreate       = FTE_FUTURETEK_JCG06_create,
    .fRun          = FTE_FUTURETEK_JCG06_run,
    .fStop         = FTE_FUTURETEK_stop,
    .fAttach       = FTE_FUTURETEK_attach,
    .fDetach       = FTE_FUTURETEK_detach,
    .fGet          = FTE_FUTURETEK_get,
    .fSet          = FTE_FUTURETEK_JCG06_set
};
static  
FTE_FUTURETEK_EXT_CONFIG xGlobalConfig = 
{
    .ulLoopPeriod       = FTE_FUTURETEK_DEFAULT_LOOP_PERIOD,
    .ulUpdatePeriod     = FTE_FUTURETEK_DEFAULT_UPDATE_INTERVAL,
    .ulRequestTimeout   = FTE_FUTURETEK_DEFAULT_REQUEST_TIMEOUT,
    .ulRetryCount       = FTE_FUTURETEK_DEFAULT_RETRY_COUNT
};

static 
FTE_UINT32       ulDeviceCount=0;

static 
FTE_FUTURETEK_DEVICE  pDevices[FTE_FUTURETEK_DEVICE_MAX];

FTE_RET   FTE_FUTURETEK_init
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_FUTURETEK_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
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
    
    pDevice->pObj = pObj;    
    ulDeviceCount++;
                
    return  FTE_RET_OK;
}
 
FTE_RET FTE_FUTURETEK_final
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32 i;
    FTE_FUTURETEK_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
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
        FTE_FUTURETEK_stop(pObj);
    }
    
    pDevice->pObj = NULL;
    ulDeviceCount--;
                
    return  FTE_RET_OK;
}

FTE_RET   FTE_FUTURETEK_run
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_FUTURETEK_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
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
        return  FTE_RET_RUNNING;
    }
    
     return FTE_TASK_create(FTE_TASK_FUTURETEK, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
}
 
FTE_RET FTE_FUTURETEK_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32 i;
    FTE_FUTURETEK_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
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
    
    if (pDevice->xTaskID == 0)
    {
        return  FTE_RET_OK;
    }
    
    _task_destroy(pDevice->xTaskID);
    FTE_TASK_remove(pDevice->xTaskID);
    pDevice->xTaskID = 0;
                
    return  FTE_RET_OK;
}

FTE_RET   FTE_FUTURETEK_attach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_RET xRet;
    FTE_FUTURETEK_CONFIG_PTR    pConfig;
    FTE_FUTURETEK_STATUS_PTR    pStatus;
    FTE_UCS_PTR  pUCS;
    
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));

    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;    
    
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
    
    FTE_FUTURETEK_init(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_FUTURETEK_detach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_FUTURETEK_STATUS_PTR  pStatus;

    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));

    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;    
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_FUTURETEK_get
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL) && (pValue != NULL));

    FTE_FUTURETEK_STATUS_PTR pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    if (ulIndex >= pStatus->xGUS.xCommon.nValueCount)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_VALUE_copy(pValue, &pStatus->xGUS.xCommon.pValue[ulIndex]);
}

FTE_RET FTE_FUTURETEK_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL) && (pValue != NULL));

    
    return  FTE_RET_OK;
}

FTE_RET     FTE_FUTURETEK_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    FTE_UINT32 xRet;
    FTE_FUTURETEK_CONFIG_PTR pConfig;
    FTE_FUTURETEK_STATUS_PTR pStatus;
    FTE_INT16   pRegs[FTE_FUTURETEK_MS10_REG_VALUE_MAX];
    FTE_UCS_UART_CONFIG xUARTConfig;
    
    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);
    
    xRet = FTE_UCS_MODBUS_getRegs(pStatus->xGUS.pUCS, pConfig->xGUS.nSensorID, FTE_FUTURETEK_MS10_REG_ADDR_START, (FTE_UINT16_PTR)pRegs, FTE_FUTURETEK_MS10_REG_VALUE_MAX, xGlobalConfig.ulRequestTimeout);
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    FTE_VALUE_setHumidity(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_MS10_REG_VALUE_MOISTURE], pRegs[FTE_FUTURETEK_MS10_REG_VALUE_MOISTURE]);
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    
    
    return  FTE_RET_OK;
}

void FTE_FUTURETEK_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET         xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_UCS_PTR pUCS;
    FTE_TIME_DELAY xDelay;
    FTE_FUTURETEK_CONFIG_PTR pConfig;
    FTE_FUTURETEK_STATUS_PTR pStatus;
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return;
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
                xRet = FTE_FUTURETEK_update(pObj);
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


FTE_INT32 FTE_FUTURETEK_SHELL_cmd
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
            FTE_INT32 i;
            
            printf("%8s %4s %8s %8s %8s\n", "ID", "SID", "CO2", "Temp.", "Humi.");
            
            for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
            {
                FTE_FUTURETEK_CONFIG_PTR pConfig;
                FTE_FUTURETEK_STATUS_PTR pStatus;
                FTE_CHAR    pBuff[32];
                
                if (pDevices[i].pObj == NULL)
                {
                    continue;
                }
                
                pConfig = (FTE_FUTURETEK_CONFIG_PTR)pDevices[i].pObj->pConfig;
                pStatus = (FTE_FUTURETEK_STATUS_PTR )pDevices[i].pObj->pStatus;
  
                printf("%8x %4d", pConfig->xGUS.xCommon.nID, pConfig->xGUS.nSensorID);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_MS10_REG_VALUE_MOISTURE], pBuff, 32);
                printf(" %8s", pBuff);
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

FTE_RET FTE_FUTURETEK_MS10_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    FTE_INT32 i;
    FTE_RET xRet;
    FTE_OBJECT_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_FUTURETEK_MS10_REG_VALUE_MAX];
    FTE_UINT32              ulSlaveID;
    FTE_UINT32              ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    xRet = FTE_strToUINT32(pSlaveID,&ulSlaveID);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
    {
        if (pDevices[i].pObj != NULL)
        {
            if (((FTE_FUTURETEK_CONFIG_PTR)pDevices[i].pObj->pConfig)->xGUS.nSensorID == ulSlaveID)
            {
                return  FTE_RET_OK;
            }
        }
    }
    
    FTE_FUTURETEK_MS10_defaultConfig.xGUS.nSensorID = ulSlaveID;
    
    xRet = FTE_CFG_OBJ_create((FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_MS10_defaultConfig, &pConfig, pChildConfig, FTE_FUTURETEK_MS10_REG_VALUE_MAX, &ulChildCount);
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

FTE_RET   FTE_FUTURETEK_MS10_run
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_FUTURETEK_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
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
        return  FTE_RET_RUNNING;
    }
    
     return FTE_TASK_create(FTE_TASK_FUTURETEK_MS10, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
}

FTE_RET     FTE_FUTURETEK_MS10_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    FTE_UINT32 xRet;
    FTE_FUTURETEK_CONFIG_PTR pConfig;
    FTE_FUTURETEK_STATUS_PTR pStatus;
    FTE_INT16   pRegs[FTE_FUTURETEK_MS10_REG_VALUE_MAX];
    FTE_UCS_UART_CONFIG xUARTConfig;
    
    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);
    
    xRet = FTE_UCS_MODBUS_getRegs(pStatus->xGUS.pUCS, pConfig->xGUS.nSensorID, FTE_FUTURETEK_MS10_REG_ADDR_START, (FTE_UINT16_PTR)pRegs, FTE_FUTURETEK_MS10_REG_VALUE_MAX, xGlobalConfig.ulRequestTimeout);
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    FTE_VALUE_setHumidity(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_MS10_REG_VALUE_MOISTURE], pRegs[FTE_FUTURETEK_MS10_REG_VALUE_MOISTURE]);
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    
    
    return  FTE_RET_OK;
}

void FTE_FUTURETEK_MS10_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET         xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_UCS_PTR pUCS;
    FTE_TIME_DELAY xDelay;
    FTE_FUTURETEK_CONFIG_PTR pConfig;
    FTE_FUTURETEK_STATUS_PTR pStatus;
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return;
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
                xRet = FTE_FUTURETEK_MS10_update(pObj);
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



FTE_RET FTE_FUTURETEK_KM21B20_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    FTE_INT32 i;
    FTE_RET xRet;
    FTE_OBJECT_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_FUTURETEK_KM21B20_REG_VALUE_MAX];
    FTE_UINT32              ulSlaveID;
    FTE_UINT32              ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    xRet = FTE_strToUINT32(pSlaveID,&ulSlaveID);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
    {
        if (pDevices[i].pObj != NULL)
        {
            if (((FTE_FUTURETEK_CONFIG_PTR)pDevices[i].pObj->pConfig)->xGUS.nSensorID == ulSlaveID)
            {
                return  FTE_RET_OK;
            }
        }
    }
    
    FTE_FUTURETEK_KM21B20_defaultConfig.xGUS.nSensorID = ulSlaveID;
    
    xRet = FTE_CFG_OBJ_create((FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_KM21B20_defaultConfig, &pConfig, pChildConfig, FTE_FUTURETEK_KM21B20_REG_VALUE_MAX, &ulChildCount);
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

FTE_RET   FTE_FUTURETEK_KM21B20_run
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_FUTURETEK_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
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
        return  FTE_RET_RUNNING;
    }
    
     return FTE_TASK_create(FTE_TASK_FUTURETEK_KM21B20, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
}

FTE_RET     FTE_FUTURETEK_KM21B20_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    FTE_UINT32 xRet;
    FTE_FUTURETEK_CONFIG_PTR pConfig;
    FTE_FUTURETEK_STATUS_PTR pStatus;
    FTE_INT16   pRegs[FTE_FUTURETEK_KM21B20_REG_VALUE_MAX];
    FTE_UCS_UART_CONFIG xUARTConfig;
    
    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);
    
    xRet = FTE_UCS_MODBUS_getRegs(pStatus->xGUS.pUCS, pConfig->xGUS.nSensorID, FTE_FUTURETEK_KM21B20_REG_ADDR_START, (FTE_UINT16_PTR)pRegs, FTE_FUTURETEK_KM21B20_REG_VALUE_MAX, xGlobalConfig.ulRequestTimeout);
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    FTE_VALUE_setHumidity(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_KM21B20_REG_ADDR_ACIDITY], pRegs[FTE_FUTURETEK_KM21B20_REG_ADDR_ACIDITY]);
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    
    
    return  FTE_RET_OK;
}

void FTE_FUTURETEK_KM21B20_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET         xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_UCS_PTR pUCS;
    FTE_TIME_DELAY xDelay;
    FTE_FUTURETEK_CONFIG_PTR pConfig;
    FTE_FUTURETEK_STATUS_PTR pStatus;
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return;
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
                xRet = FTE_FUTURETEK_KM21B20_update(pObj);
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


FTE_RET FTE_FUTURETEK_JCG06_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    FTE_INT32 i;
    FTE_RET xRet;
    FTE_OBJECT_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_FUTURETEK_JCG06_REG_VALUE_MAX];
    FTE_UINT32              ulSlaveID;
    FTE_UINT32              ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    xRet = FTE_strToUINT32(pSlaveID,&ulSlaveID);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
    {
        if (pDevices[i].pObj != NULL)
        {
            if (((FTE_FUTURETEK_CONFIG_PTR)pDevices[i].pObj->pConfig)->xGUS.nSensorID == ulSlaveID)
            {
                return  FTE_RET_INVALID_ID;
            }
        }
    }
    
    FTE_FUTURETEK_JCG06_defaultConfig.xGUS.nSensorID = ulSlaveID;
    
    xRet = FTE_CFG_OBJ_create((FTE_OBJECT_CONFIG_PTR)&FTE_FUTURETEK_JCG06_defaultConfig, &pConfig, pChildConfig, FTE_FUTURETEK_JCG06_REG_VALUE_MAX, &ulChildCount);
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


FTE_RET   FTE_FUTURETEK_JCG06_run
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_FUTURETEK_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_FUTURETEK_DEVICE_MAX ; i++)
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
        return  FTE_RET_RUNNING;
    }
    
     return FTE_TASK_create(FTE_TASK_FUTURETEK_JCG06, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
}
 
void FTE_FUTURETEK_JCG06_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET         xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_UCS_PTR pUCS;
    FTE_TIME_DELAY xDelay;
    FTE_FUTURETEK_CONFIG_PTR pConfig;
    FTE_FUTURETEK_STATUS_PTR pStatus;
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pConfig = (FTE_FUTURETEK_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return;
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
                xRet = FTE_FUTURETEK_JCG06_update(pObj);
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

FTE_RET     FTE_FUTURETEK_JCG06_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    FTE_FUTURETEK_STATUS_PTR pStatus;
    FTE_UCS_UART_CONFIG xUARTConfig;
    
    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);
    
    FTE_UINT8   pSendData[8] = {0x02, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xf9};
    FTE_UINT8   pRcvdData[7] = {0x00};
    FTE_UINT32  ulRcvdLen = 0;
    
    ulRcvdLen = FTE_UCS_sendAndRecv(pStatus->xGUS.pUCS, pSendData, sizeof(pSendData), pRcvdData, sizeof(pRcvdData), 0, 100);
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
    if (ulRcvdLen != 7)
    {
        return  FTE_RET_ERROR;
    }

    FTE_UINT16 uiCRC = FTE_CRC16(pRcvdData, ulRcvdLen - 2);
    if (uiCRC != *((FTE_UINT16_PTR)&pRcvdData[ulRcvdLen - 2]))
    {
        return  FTE_RET_ERROR;
    }
    
    FTE_UINT32  ulValue = (((FTE_UINT32)pRcvdData[3] << 8) + pRcvdData[4]) * 5;
    
    FTE_VALUE_setMM(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL], ulValue);
    FTE_VALUE_setMM(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL_PER_HOUR], ulValue);

    FTE_TIME    xCurrentTime;
    FTE_TIME_getCurrent(&xCurrentTime);    

    DATE_STRUCT xLastUpdateDate;
    DATE_STRUCT xCurrentDate;
   
    _time_to_date (&pStatus->xLastUpdateTime, &xLastUpdateDate);
    _time_to_date (&xCurrentTime, &xCurrentDate);

    if ((xLastUpdateDate.YEAR != 1970) && (xLastUpdateDate.DAY != xCurrentDate.DAY))
    {
        FTE_FUTURETEK_JCG06_resetRainfall(pObj);            
    }
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    

    
    
    return  FTE_RET_OK;
}

FTE_RET FTE_FUTURETEK_JCG06_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL) && (pValue != NULL));

    if (ulIndex >= pObj->pConfig->xCommon.ulChild)
    {
        return  FTE_RET_INVALID_ID;
    }
        
    if (ulIndex == FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL_RESET)
    {
        FTE_FUTURETEK_STATUS_PTR pStatus;

        pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;            
        
        FTE_BOOL    value = 0;
        if (FTE_VALUE_getDIO(pValue, &value) != FTE_RET_OK)
        {
            return  FTE_RET_INVALID_ARGUMENT;
        }

        if (value == 1)
        {
            return  FTE_FUTURETEK_JCG06_resetRainfall(pObj);
        }
        else
        {
            FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL_RESET], value);
        }
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_FUTURETEK_JCG06_resetRainfall
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL));

    FTE_FUTURETEK_STATUS_PTR pStatus;

    pStatus = (FTE_FUTURETEK_STATUS_PTR)pObj->pStatus;            
    
    FTE_UCS_UART_CONFIG xUARTConfig;
    
    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);
    
    FTE_UINT8   pSendData[11] = {0x02, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0xb2, 0xa0};
    FTE_UINT8   pRcvdData[8] = {0x00};
    FTE_UINT32  ulRcvdLen = 0;
    
    ulRcvdLen = FTE_UCS_sendAndRecv(pStatus->xGUS.pUCS, pSendData, sizeof(pSendData), pRcvdData, sizeof(pRcvdData), 0, 100);
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
    if (ulRcvdLen != 8)
    {
        return  FTE_RET_ERROR;
    }

    FTE_UINT16 uiCRC = FTE_CRC16(pRcvdData, ulRcvdLen - 2);
    if (uiCRC != *((FTE_UINT16_PTR)&pRcvdData[ulRcvdLen - 2]))
    {
        return  FTE_RET_ERROR;
    }

    FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[FTE_FUTURETEK_JCG06_REG_VALUE_RAINFALL_RESET], 1);
    
    return  FTE_RET_OK;
}

