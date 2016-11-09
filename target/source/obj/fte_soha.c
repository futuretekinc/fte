#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h" 

typedef struct  FTE_SOHA_MV250_FRAME_STRUCT
{
    FTE_UINT8   uiSTX;
    FTE_UINT8   uiID;
    FTE_UINT8   uiReserved1;
    FTE_CHAR    pCO2[5];
    FTE_CHAR    pTemp[6];
    FTE_CHAR    pHumi[5];
    FTE_UINT8   uiSHT;
    FTE_UINT8   uiMode;
    FTE_UINT8   uiCS;
    FTE_UINT8   uiCR;
    FTE_UINT8   uiLF;    
}   FTE_SOHA_MV250_FRAME, _PTR_ FTE_SOHA_MV250_FRAME_PTR;


static
FTE_RET FTE_SOHA_final
(
    FTE_OBJECT_PTR  pObj
);

static
FTE_RET FTE_SOHA_run
(
    FTE_OBJECT_PTR  pObj
);

static
FTE_RET FTE_SOHA_stop
(
    FTE_OBJECT_PTR  pObj
);

static
FTE_RET     FTE_SOHA_attach
(
    FTE_OBJECT_PTR pObj
);

static
FTE_RET     FTE_SOHA_detach
(
    FTE_OBJECT_PTR pObj
);

static
FTE_RET     FTE_SOHA_get
(
    FTE_OBJECT_PTR pObj, 
    FTE_UINT32 ulIndex, 
    FTE_VALUE_PTR pValue
);


typedef struct  FTE_SOHA_DEVICE_STRUCT
{
    _task_id        xTaskID;
    FTE_OBJECT_PTR  pObj;
}   FTE_SOHA_DEVICE, _PTR_ FTE_SOHA_DEVICE_PTR;

static const 
FTE_IFCE_CONFIG    FTE_SOHA_MV250_co2Config =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_CO2, 0),
        .pName  = "MV250-CO2",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0),
    .nRegID     = FTE_SOHA_MV250_REG_VALUE_CO2,
    .nInterval  = FTE_SOHA_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG    FTE_SOHA_MV250_tempConfig =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName  = "MV250-TEMP",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0),
    .nRegID     = FTE_SOHA_MV250_REG_VALUE_TEMPERATURE,
    .nInterval  = FTE_SOHA_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG    FTE_SOHA_MV250_humiConfig =
{
    .xCommon    =
    {
        .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0),
        .pName  = "MV250-HUMI",
        .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0),
    .nRegID     = FTE_SOHA_MV250_REG_VALUE_HUMIDITY,
    .nInterval  = FTE_SOHA_DEFAULT_UPDATE_INTERVAL
};
static const 
FTE_OBJECT_CONFIG_PTR FTE_SOHA_MV250HT_sensorConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_SOHA_MV250_co2Config,
    (FTE_OBJECT_CONFIG_PTR)&FTE_SOHA_MV250_tempConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_SOHA_MV250_humiConfig
};

FTE_SOHA_CONFIG FTE_SOHA_MV250HT_defaultConfig =
{
    .xGUS       =
    {
        .xCommon    =
        {
            .nID    = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0x0000),
            .pName  = "MV250",
            .xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE,
            .ulChild= sizeof(FTE_SOHA_MV250HT_sensorConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_SOHA_MV250HT_sensorConfigs
        },
        .nModel     = FTE_GUS_MODEL_SH_MV250,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = FTE_SOHA_DEFAULT_UPDATE_INTERVAL,
    }
};

FTE_VALUE_TYPE  FTE_SOHA_MV250_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_HUMIDITY
};

static  
FTE_SOHA_EXT_CONFIG xGlobalConfig = 
{
    .ulLoopPeriod       = FTE_SOHA_DEFAULT_LOOP_PERIOD,
    .ulUpdatePeriod     = FTE_SOHA_DEFAULT_UPDATE_INTERVAL,
    .ulRequestTimeout   = FTE_SOHA_DEFAULT_REQUEST_TIMEOUT,
    .ulRetryCount       = FTE_SOHA_DEFAULT_RETRY_COUNT
};

const 
FTE_GUS_MODEL_INFO    FTE_SOHA_MV250_GUSModelInfo = 
{
    .nModel             = FTE_GUS_MODEL_SH_MV250,
    .pName              = "SOHA MV250",
    .xFlags             = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_SOHA_MV250_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_SOHA_MV250_DEFAULT_DATABITS,
        .nParity    =   FTE_SOHA_MV250_DEFAULT_PARITY,
        .nStopBits  =   FTE_SOHA_MV250_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_SOHA_MV250_DEFAULT_FULL_DUPLEX
    },
    .nMaxResponseTime   = FTE_SOHA_DEFAULT_REQUEST_TIMEOUT,
    .nFieldCount    = FTE_SOHA_MV250_REG_VALUE_MAX,
    .pValueTypes    = FTE_SOHA_MV250_valueTypes,
    .fInit         = FTE_SOHA_init, 
    .fFinal        = FTE_SOHA_final, 
    .fCreate       = FTE_SOHA_MV250HT_create,
    .fRun          = FTE_SOHA_run,
    .fStop         = FTE_SOHA_stop,
    .fAttach       = FTE_SOHA_attach,
    .fDetach       = FTE_SOHA_detach,
    .fGet          = FTE_SOHA_get
};


static 
FTE_UINT32       ulDeviceCount=0;

static 
FTE_SOHA_DEVICE  pDevices[FTE_SOHA_DEVICE_MAX];

FTE_RET   FTE_SOHA_init
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_SOHA_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_SOHA_DEVICE_MAX ; i++)
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
 
FTE_RET FTE_SOHA_final
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32 i;
    FTE_SOHA_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_SOHA_DEVICE_MAX ; i++)
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
        FTE_SOHA_stop(pObj);
    }
    
    pDevice->pObj = NULL;
    ulDeviceCount--;
                
    return  FTE_RET_OK;
}

FTE_RET   FTE_SOHA_run
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32   i;
    FTE_SOHA_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_SOHA_DEVICE_MAX ; i++)
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
    
     return FTE_TASK_create(FTE_TASK_SOHA, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
}
 
FTE_RET FTE_SOHA_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32 i;
    FTE_SOHA_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_SOHA_DEVICE_MAX ; i++)
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

FTE_RET   FTE_SOHA_attach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_RET xRet;
    FTE_SOHA_CONFIG_PTR    pConfig;
    FTE_SOHA_STATUS_PTR    pStatus;
    FTE_UCS_PTR  pUCS;
    
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));

    pConfig = (FTE_SOHA_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_SOHA_STATUS_PTR)pObj->pStatus;    
    
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
    
    FTE_SOHA_init(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SOHA_detach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_SOHA_STATUS_PTR  pStatus;

    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));

    pStatus = (FTE_SOHA_STATUS_PTR)pObj->pStatus;    
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_SOHA_get
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL) && (pValue != NULL));

    FTE_SOHA_STATUS_PTR pStatus = (FTE_SOHA_STATUS_PTR)pObj->pStatus;
    
    if (ulIndex >= pStatus->xGUS.xCommon.nValueCount)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_VALUE_copy(pValue, &pStatus->xGUS.xCommon.pValue[ulIndex]);
}


FTE_RET     FTE_SOHA_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    FTE_UINT32 xRet;
    FTE_SOHA_CONFIG_PTR pConfig;
    FTE_SOHA_STATUS_PTR pStatus;
    FTE_INT16   pRegs[FTE_SOHA_MV250_REG_VALUE_MAX];
    FTE_UCS_UART_CONFIG xUARTConfig;
    
    pConfig = (FTE_SOHA_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_SOHA_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
    
    FTE_TIME_getCurrent(&pStatus->xLastRequestTime);
    
    xRet = FTE_UCS_MODBUS_getRegs(pStatus->xGUS.pUCS, pConfig->xGUS.nSensorID, FTE_SOHA_MV250_REG_ADDR_START, (FTE_UINT16_PTR)pRegs, FTE_SOHA_MV250_REG_VALUE_MAX, xGlobalConfig.ulRequestTimeout);
    
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);    
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    FTE_VALUE_setPPM(&pStatus->xGUS.xCommon.pValue[FTE_SOHA_MV250_REG_VALUE_CO2], pRegs[FTE_SOHA_MV250_REG_VALUE_CO2]);
    FTE_VALUE_setTemperature(&pStatus->xGUS.xCommon.pValue[FTE_SOHA_MV250_REG_VALUE_TEMPERATURE], pRegs[FTE_SOHA_MV250_REG_VALUE_TEMPERATURE]* 10);
    FTE_VALUE_setHumidity(&pStatus->xGUS.xCommon.pValue[FTE_SOHA_MV250_REG_VALUE_HUMIDITY], pRegs[FTE_SOHA_MV250_REG_VALUE_HUMIDITY] * 10);
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    
    
    return  FTE_RET_OK;
}

void FTE_SOHA_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET         xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_UCS_PTR pUCS;
    FTE_TIME_DELAY xDelay;
    FTE_SOHA_CONFIG_PTR pConfig;
    FTE_SOHA_STATUS_PTR pStatus;
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pConfig = (FTE_SOHA_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_SOHA_STATUS_PTR)pObj->pStatus;
    
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
                xRet = FTE_SOHA_update(pObj);
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


FTE_INT32 FTE_SOHA_SHELL_cmd
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
            
            for(i = 0 ; i < FTE_SOHA_DEVICE_MAX ; i++)
            {
                FTE_SOHA_CONFIG_PTR pConfig;
                FTE_SOHA_STATUS_PTR pStatus;
                FTE_CHAR    pBuff[32];
                
                if (pDevices[i].pObj == NULL)
                {
                    continue;
                }
                
                pConfig = (FTE_SOHA_CONFIG_PTR)pDevices[i].pObj->pConfig;
                pStatus = (FTE_SOHA_STATUS_PTR )pDevices[i].pObj->pStatus;
  
                printf("%8x %4d", pConfig->xGUS.xCommon.nID, pConfig->xGUS.nSensorID);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_SOHA_MV250_REG_VALUE_CO2], pBuff, 32);
                printf(" %8s", pBuff);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_SOHA_MV250_REG_VALUE_TEMPERATURE], pBuff, 32);
                printf(" %8s", pBuff);
                FTE_VALUE_toString(&pStatus->xGUS.xCommon.pValue[FTE_SOHA_MV250_REG_VALUE_HUMIDITY], pBuff, 32);
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

FTE_RET FTE_SOHA_MV250HT_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    FTE_INT32 i;
    FTE_RET xRet;
    FTE_OBJECT_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_SOHA_MV250_REG_VALUE_MAX];
    FTE_UINT32              ulSlaveID;
    FTE_UINT32              ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    xRet = FTE_strToUINT32(pSlaveID,&ulSlaveID);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    for(i = 0 ; i < FTE_SOHA_DEVICE_MAX ; i++)
    {
        if (pDevices[i].pObj != NULL)
        {
            if (((FTE_SOHA_CONFIG_PTR)pDevices[i].pObj->pConfig)->xGUS.nSensorID == ulSlaveID)
            {
                return  FTE_RET_OK;
            }
        }
    }
    
    FTE_SOHA_MV250HT_defaultConfig.xGUS.nSensorID = ulSlaveID;
    
    xRet = FTE_CFG_OBJ_create((FTE_OBJECT_CONFIG_PTR)&FTE_SOHA_MV250HT_defaultConfig, &pConfig, pChildConfig, FTE_SOHA_MV250_REG_VALUE_MAX, &ulChildCount);
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

