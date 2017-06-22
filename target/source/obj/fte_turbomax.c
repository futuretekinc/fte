#include "fte_target.h"
#include "fte_crc.h"
#include "fte_cias.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"
#include "fte_task.h"
#include "nxjson.h"
#include "fte_turbomax.h"

#if FTE_TURBOMAX_SUPPORTED 

#ifndef FTE_TURBOMAX_DEFAULT_LOOP_PERIOD
#define FTE_TURBOMAX_DEFAULT_LOOP_PERIOD        2000 // ms
#endif

#ifndef FTE_TURBOMAX_DEFAULT_UPDATE_PERIOD
#define FTE_TURBOMAX_DEFAULT_UPDATE_PERIOD      10000 // ms
#endif

#ifndef FTE_TURBOMAX_DEFAULT_REQUEST_TIMEOUT
#define FTE_TURBOMAX_DEFAULT_REQUEST_TIMEOUT    500
#endif

#ifndef FTE_TURBOMAX_DEFAULT_RETRY_COUNT
#define FTE_TURBOMAX_DEFAULT_RETRY_COUNT        3
#endif


#define FTE_TURBOMAX_MAX20_REG_MOTOR_SPEED      0
#define FTE_TURBOMAX_MAX20_REG_FITER_PRESS_DROP 3
#define FTE_TURBOMAX_MAX20_REG_POWER            4
#define FTE_TURBOMAX_MAX20_REG_SUCTION_TEMP     5
#define FTE_TURBOMAX_MAX20_REG_DISCHARGE_TEMP   6
#define FTE_TURBOMAX_MAX20_REG_MOTOR_TEMP       7
#define FTE_TURBOMAX_MAX20_REG_RUN_TIME         10
#define FTE_TURBOMAX_MAX20_REG_INPUT_CURRENT    15
#define FTE_TURBOMAX_MAX20_REG_OUTPUT_CURRENT   16
#define FTE_TURBOMAX_MAX20_REG_INVERTER_TEMP    17
#define FTE_TURBOMAX_MAX20_REG_AIR_TEMP         20
#define FTE_TURBOMAX_MAX20_REG_STATUS           26
#define FTE_TURBOMAX_MAX20_REG_ERROR_CODE       27

FTE_RET   FTE_TURBOMAX_init
(
    FTE_OBJECT_PTR  pObj
);

FTE_RET   FTE_TURBOMAX_run
(
    FTE_OBJECT_PTR  pObj
);

FTE_RET   FTE_TURBOMAX_stop
(
    FTE_OBJECT_PTR  pObj
);


typedef struct  FTE_TURBOMAX_DEVICE_STRUCT
{
    _task_id        xTaskID;
    FTE_OBJECT_PTR  pObj;
}   FTE_TURBOMAX_DEVICE, _PTR_ FTE_TURBOMAX_DEVICE_PTR;

FTE_RET FTE_TURBOMAX_MAX20_run
(
    FTE_OBJECT_PTR  pObj
);

FTE_RET FTE_TURBOMAX_MAX20_stop
(
    FTE_OBJECT_PTR  pObj
);

FTE_RET FTE_TURBOMAX_MAX20_loadConfig
(
    FTE_OBJECT_PTR pObj
);

FTE_RET FTE_TURBOMAX_MAX20_writeReg
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32      ulIndex,
    FTE_INT32       nValue
);


static 
FTE_RET FTE_TURBOMAX_getUpdateInterval
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32_PTR  pulInterval
);

static 
FTE_RET  FTE_TURBOMAX_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
);

static 
FTE_RET  FTE_TURBOMAX_statistic
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTICS_PTR pStatistics
);

static 
FTE_RET  FTE_TURBOMAX_getChildCount
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pCount
);

static 
FTE_RET FTE_TURBOMAX_getChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_OBJECT_ID _PTR_ pxChildID
);

static
FTE_RET FTE_TURBOMAX_attachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID   xChildID
);

static
FTE_RET FTE_TURBOMAX_detachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID   xChildID
);

static
FTE_RET FTE_TURBOMAX_getMulti
(
    FTE_OBJECT_PTR  pObject, 
    FTE_UINT32      ulIndex,
    FTE_VALUE_PTR   pValue
);

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_MOTOR_SPEED_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0),
        .pName      = "MotorS",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_MOTOR_SPEED,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_FILTER_PRESS_DROP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_PRESSURE, 0),
        .pName      = "FilterPD",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_FITER_PRESS_DROP,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_POWER_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_POWER, 0),
        .pName      = "Power",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_POWER,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_SUCTION_TEMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "SuctionT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_SUCTION_TEMP,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_DISCHARGE_TEMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "DischargeT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_DISCHARGE_TEMP,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_MOTOR_TEMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "MotorT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_MOTOR_TEMP,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_RUN_TIME_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0),
        .pName      = "RunTime",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_RUN_TIME,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_INPUT_CURRENT_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CURRENT, 0),
        .pName      = "InputC",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_INPUT_CURRENT,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_OUTPUT_CURRENT_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CURRENT, 0),
        .pName      = "OutputC",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_OUTPUT_CURRENT,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_INVERTER_TEMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "InverterT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_INVERTER_TEMP,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_AIR_TEMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "AirT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_AIR_TEMP,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_STATUS_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0),
        .pName      = "Status",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_STATUS,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TURBOMAX_MAX20_ERROR_CODE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0),
        .pName      = "ErrorCode",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
    .nRegID     = FTE_TURBOMAX_MAX20_REG_ERROR_CODE,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

static const FTE_OBJECT_CONFIG_PTR FTE_TURBOMAX_MAX20_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_MOTOR_SPEED_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_FILTER_PRESS_DROP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_POWER_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_SUCTION_TEMP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_DISCHARGE_TEMP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_MOTOR_TEMP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_RUN_TIME_defaultConfig,
//    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_INPUT_CURRENT_defaultConfig ,
//    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_OUTPUT_CURRENT_defaultConfig, 
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_INVERTER_TEMP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_AIR_TEMP_defaultConfig,
//    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_STATUS_defaultConfig,
//    (FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_ERROR_CODE_defaultConfig,
};

FTE_TURBOMAX_CONFIG FTE_TURBOMAX_MAX20_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, 0),
        .pName      = "MAX20",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
        .ulChild    = sizeof(FTE_TURBOMAX_MAX20_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_TURBOMAX_MAX20_defaultChildConfigs
    },
//    .nSensorID  = 0x01,
    .nBusID     = 0x00,
    .nInterval  = FTE_TURBOMAX_MAX20_DEFAULT_UPDATE_INTERVAL
};

FTE_VALUE_TYPE  FTE_TURBOMAX_MAX20_valueTypes[] =
{    
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_PRESSURE,
    FTE_VALUE_TYPE_PWR_KW,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_ULONG,
//    FTE_VALUE_TYPE_CURRENT,
    //FTE_VALUE_TYPE_CURRENT,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE,
//    FTE_VALUE_TYPE_ULONG,
//    FTE_VALUE_TYPE_ULONG,
};

static  FTE_TURBOMAX_EXT_CONFIG xGlobalConfig = 
{
    .ulLoopPeriod       = FTE_TURBOMAX_DEFAULT_LOOP_PERIOD,
    .ulUpdatePeriod     = FTE_TURBOMAX_DEFAULT_UPDATE_PERIOD,
    .ulRequestTimeout   = FTE_TURBOMAX_DEFAULT_REQUEST_TIMEOUT,
    .ulRetryCount       = FTE_TURBOMAX_DEFAULT_RETRY_COUNT
};

static FTE_UINT32           ulDeviceCount=0;
static FTE_TURBOMAX_DEVICE    pDevices[FTE_TURBOMAX_MAX20_MAX];

static  
FTE_OBJECT_ACTION _Action = 
{
    .fInit  = FTE_TURBOMAX_init,
    .fRun   = FTE_TURBOMAX_run,
    .fStop  = FTE_TURBOMAX_stop, 
    .fSet   = NULL,
    .fGetInterval   = FTE_TURBOMAX_getUpdateInterval,
    .fSetInterval   = FTE_TURBOMAX_setUpdateInterval,
    .fGetStatistics = FTE_TURBOMAX_statistic,
    .fGetChildCount = FTE_TURBOMAX_getChildCount,
    .fGetChild      = FTE_TURBOMAX_getChild,
    .fAttachChild   = FTE_TURBOMAX_attachChild,
    .fDetachChild   = FTE_TURBOMAX_detachChild,
    .fGetMulti      = FTE_TURBOMAX_getMulti,
};

FTE_RET   FTE_TURBOMAX_attach2
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT( pObj != NULL);
    
    //FTE_RET xRet;
//    FTE_TURBOMAX_CONFIG_PTR   pConfig = (FTE_TURBOMAX_CONFIG_PTR)pObj->pConfig;
//    FTE_TURBOMAX_STATUS_PTR   pStatus = (FTE_TURBOMAX_STATUS_PTR)pObj->pStatus;
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_Action;
    
//    _FTE_TURBOMAX_init(pObj);
    
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_detach2
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

//    _FTE_TURBOMAX_stop(pObj);
    pObj->pAction = NULL;

    return  FTE_RET_OK;
}

FTE_RET     FTE_TURBOMAX_update
(
    FTE_OBJECT_PTR pObj
)
{ 
    ASSERT(pObj != NULL);
    
    FTE_RET     xRet;
    FTE_UINT16  pRegisters[30];
    FTE_TURBOMAX_CONFIG_PTR pConfig;
    FTE_TURBOMAX_STATUS_PTR pStatus;
        
    pConfig = (FTE_TURBOMAX_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_TURBOMAX_STATUS_PTR)pObj->pStatus;
    
    if (!FTE_MBTCP_MASTER_isConnected(pConfig->nBusID))
    {
        return  FTE_RET_ERROR;
    }
        
    xRet = FTE_MBTCP_MASTER_readRegisters(pConfig->nBusID, 0, 30, pRegisters);
    if (xRet != FTE_RET_OK)
    {
        ERROR("Failed to read register!");            
        return  xRet;
    }

#if 0    
    xRet = FTE_MBTCP_MASTER_readRegisters(pConfig->nBusID, 54, 54, &pRegisters[54]);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
#endif
    //pRegisters[FTE_TURBOMAX_MAX20_REG_FITER_PRESS_DROP] /= 10;
    //pRegisters[FTE_TURBOMAX_MAX20_REG_POWER] /= 10;
    pRegisters[FTE_TURBOMAX_MAX20_REG_SUCTION_TEMP] *= 10;
    pRegisters[FTE_TURBOMAX_MAX20_REG_DISCHARGE_TEMP] *= 10;
    pRegisters[FTE_TURBOMAX_MAX20_REG_MOTOR_TEMP] *= 10;
    pRegisters[FTE_TURBOMAX_MAX20_REG_INPUT_CURRENT] *= 10;
    pRegisters[FTE_TURBOMAX_MAX20_REG_OUTPUT_CURRENT] *= 10;
    pRegisters[FTE_TURBOMAX_MAX20_REG_INVERTER_TEMP] *= 10;
    pRegisters[FTE_TURBOMAX_MAX20_REG_AIR_TEMP] *= 10;

    memcpy(pStatus->pRegisters, pRegisters, sizeof(pRegisters));
    
    FTE_TIME_getCurrent(&pStatus->xLastUpdateTime);    
        
    return  FTE_RET_OK;
}

void FTE_TURBOMAX_task
(
    FTE_UINT32     ulObjectID
)
{
    FTE_RET xRet;
    FTE_OBJECT_PTR  pObj;
    FTE_TIME_DELAY xDelay;
    FTE_TURBOMAX_STATUS_PTR pStatus;
          
    pObj = FTE_OBJ_get(ulObjectID);
    if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", ulObjectID);
        return;
    }
    
    ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
    
    pStatus = (FTE_TURBOMAX_STATUS_PTR)pObj->pStatus;
    
    pStatus->ulRetryCount = 0;
    _time_get(&pStatus->xLastUpdateTime);    
    
    FTE_TIME_DELAY_init(&xDelay, xGlobalConfig.ulLoopPeriod);
    
    while(TRUE)
    {
        TIME_STRUCT xCurrentTime;
        FTE_INT32   nElapsedUpdateTime = 0;
        
        _time_get(&xCurrentTime);
        
        FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastUpdateTime, &nElapsedUpdateTime);        
        if (nElapsedUpdateTime >= xGlobalConfig.ulUpdatePeriod)
        {   
            FTE_INT32 nElapsedRequestTime = 0;
            
            FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastRequestTime, &nElapsedRequestTime);
            if( nElapsedRequestTime >= xGlobalConfig.ulRequestTimeout)
            {
                xRet = FTE_TURBOMAX_update(pObj);                
                if (xRet == FTE_RET_OK)
                {
                    pStatus->ulRetryCount=0;
                }
                else if (pStatus->ulRetryCount >= xGlobalConfig.ulRetryCount)
                {
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

FTE_RET FTE_TURBOMAX_create
(
    FTE_OBJECT_CONFIG_PTR   pBaseConfig,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    ASSERT(ppObj != NULL);
    
    int i;
    FTE_RET xRet;
    FTE_OBJECT_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_TURBOMAX_MAX20_FIELD_MAX];
    FTE_UINT32              ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    xRet = FTE_CFG_OBJ_create((FTE_OBJECT_CONFIG_PTR)&FTE_TURBOMAX_MAX20_defaultConfig, &pConfig, pChildConfig, FTE_TURBOMAX_MAX20_FIELD_MAX, &ulChildCount);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }    

    ((FTE_TURBOMAX_CONFIG_PTR)pConfig)->nBusID = FTE_OBJ_count (FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20, FTE_OBJ_TYPE_MASK, 0);
    
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
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_Action;
    
    *ppObj = pObj;
        
    return  FTE_RET_OK;    
}

FTE_RET FTE_TURBOMAX_MAX20_destroy
(
    FTE_OBJECT_PTR pObj
)
{
    ASSERT(pObj != NULL);
    
    FTE_RET xRet;
    FTE_UINT32              ulChildCount = 0 ;

    xRet = FTE_OBJ_getChildCount(pObj, &ulChildCount);
    if (xRet == FTE_RET_OK)
    {
        for(FTE_INT32 i = ulChildCount - 1; i >= 0; i--)
        {
            FTE_OBJECT_PTR  pChild;
            
            xRet = FTE_OBJ_getChild(pObj, i, &pChild);
            if (xRet == FTE_RET_OK)
            {
                FTE_CFG_OBJ_free(pChild->pConfig->xCommon.nID);
                FTE_OBJ_destroy(pChild);
            }
        }
    }

    FTE_CFG_OBJ_free(pObj->pConfig->xCommon.nID);
    FTE_OBJ_destroy(pObj);
        
    return  FTE_RET_OK;    
}

FTE_RET FTE_TURBOMAX_MAX20_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    int i;
    FTE_TURBOMAX_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_TURBOMAX_MAX20_MAX ; i++)
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
        return  FTE_RET_OK;
    }
    
    return  FTE_TASK_create(FTE_TASK_TURBOMAX, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
}

FTE_RET FTE_TURBOMAX_MAX20_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    int i;
    FTE_TURBOMAX_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_TURBOMAX_MAX20_MAX ; i++)
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

FTE_RET   FTE_TURBOMAX_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));


    FTE_TURBOMAX_DEVICE_PTR   pDevice = NULL;
    
    for(FTE_INT32 i = 0 ; i < FTE_TURBOMAX_MAX20_MAX ; i++)
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
    FTE_TURBOMAX_MAX20_loadConfig(pObj);
    
//    pConfig = (FTE_TURBOMAX_CONFIG_PTR)pObj->pConfig;
//    pStatus = (FTE_TURBOMAX_STATUS_PTR)pObj->pStatus;    
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_Action;
    
    pDevice->pObj = pObj;    
    ulDeviceCount++;
    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_TURBOMAX_detach
(
    FTE_OBJECT_PTR pObj
)
{

    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));
  
    FTE_TURBOMAX_DEVICE_PTR   pDevice = NULL;
    
    for(FTE_INT32 i = 0 ; i < FTE_TURBOMAX_MAX20_MAX ; i++)
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
        FTE_TURBOMAX_MAX20_stop(pObj);
    }    
    
    pDevice->pObj = NULL;
    ulDeviceCount--;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_MAX20_setChildConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff
)
{
    FTE_RET xRet = FTE_RET_OK;
    
    ASSERT ((pObj != NULL) && (pObj->pConfig != NULL) && (pBuff != NULL));

    const nx_json* pxJSON = nx_json_parse_utf8(pBuff);
    if (pxJSON == NULL)
    {
        return  FTE_RET_ERROR;
    } 

    if ((FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_CTRL) || (((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent == NULL))
    {
        xRet = FTE_RET_ERROR;
    }
    
    nx_json_free(pxJSON);
    
    return  xRet;
}

FTE_RET FTE_TURBOMAX_MAX20_getChildConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    ASSERT ((pObj != NULL) && (pObj->pStatus != NULL) && (pBuff != NULL));

    if ((FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_CTRL) || (((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent == NULL))
    {
        return  FTE_RET_ERROR;
    }

    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_MAX20_loadConfig
(
    FTE_OBJECT_PTR pObj
)
{
#if 0
    if (!bExtConfigLoaded)
    {
        if (FTE_CFG_TURBOMAX_getExtConfig(&xGlobalConfig, sizeof(xGlobalConfig)) != FTE_RET_OK)
        {
            return  FTE_RET_ERROR;
        }
        bExtConfigLoaded = TRUE;
    }
#endif
    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_MAX20_saveConfig
(
    void
)
{
    //return  FTE_CFG_TURBOMAX_setExtConfig(&xGlobalConfig, sizeof(xGlobalConfig));
    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_initDefaultExtConfig
(
    FTE_TURBOMAX_EXT_CONFIG_PTR pConfig
)
{
    ASSERT(pConfig != NULL);

    pConfig->ulLoopPeriod       = FTE_TURBOMAX_DEFAULT_LOOP_PERIOD;
    pConfig->ulUpdatePeriod     = FTE_TURBOMAX_DEFAULT_UPDATE_PERIOD;
    pConfig->ulRequestTimeout   = FTE_TURBOMAX_DEFAULT_REQUEST_TIMEOUT;
    pConfig->ulRetryCount       = FTE_TURBOMAX_DEFAULT_RETRY_COUNT;
    
    return  FTE_RET_OK;
}

FTE_RET     FTE_TURBOMAX_MAX20_writeReg
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32      ulIndex,
    FTE_INT32       nValue
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    return  FTE_RET_OK;
}
    

FTE_INT32 FTE_TURBOMAX_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_RET     xRet = SHELL_EXIT_SUCCESS;
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_TURBOMAX_CONFIG_PTR pConfig;
    FTE_TURBOMAX_STATUS_PTR pStatus;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            int i;
            for(i = 0 ; i < FTE_TURBOMAX_MAX20_MAX ; i++)
            {
                if (pDevices[i].pObj == NULL)
                {
                    continue;
                }
                
                pConfig = (FTE_TURBOMAX_CONFIG_PTR)pDevices[i].pObj->pConfig;
                pStatus = (FTE_TURBOMAX_STATUS_PTR)pDevices[i].pObj->pStatus;
                
                printf("%16s : %s\n", "Name",   pConfig->xCommon.pName);
                printf("%16s : %08x\n", "UnitID", pConfig->xCommon.nID);
                printf("%16s : %d\n", "BusID", pConfig->nBusID);                
                printf("%16s : %d\n\n", "Time",   pStatus->xLastUpdateTime); 
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
            
            if (FTE_strToHex(pArgv[1], &xOID) != FTE_RET_OK)
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
            
            if (FTE_OBJ_TYPE(pObject) == FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20)
            {
                int i;
                FTE_TURBOMAX_STATUS_PTR  pStatus;
               
                pStatus = (FTE_TURBOMAX_STATUS_PTR)pObject->pStatus;

                for(i = 0 ; i < 108 ; i++)
                {
                    printf("%3d : %d\n", i+1, pStatus->pRegisters[i]);
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
                    
                    if (FTE_strToUINT32(pArgv[3], &ulInterval) != FTE_RET_OK)
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
                    
                    if (FTE_strToUINT32(pArgv[3], &ulTimeout) != FTE_RET_OK)
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
                
                xRet = FTE_TURBOMAX_MAX20_saveConfig();
                if (xRet != FTE_RET_OK)
                {
                    printf("MAX20 global configuration is saved.\n");
                }
            }
        }
        break;
    case  5:
        {
            FTE_OBJECT_ID   xOID;
            FTE_OBJECT_PTR  pObject;
            
            if (FTE_strToHex(pArgv[1], &xOID) != FTE_RET_OK)
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
            
            if (FTE_OBJ_TYPE(pObject) != FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20)
            {
                printf("Object is not turbomax device!\n");
                goto error;
            }

            if (strcasecmp("set", pArgv[2]) == 0)
            {
                if ((strcasecmp("b", pArgv[3]) == 0) || (strcasecmp("bus_id", pArgv[3]) == 0))
                {
                    FTE_UINT32 ulBusID;
                    
                    if (FTE_strToUINT32(pArgv[4], &ulBusID) != FTE_RET_OK)
                    {
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    if (ulBusID >= 2)                    
                    {
                        printf("Bus ID is 0 or 1\n");
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    ((FTE_TURBOMAX_CONFIG_PTR)pObject->pConfig)->nBusID = ulBusID;
                    
                    FTE_CFG_OBJ_save(pObject);
                }
            }
            else 
            {
                printf("Invalid arguments!\n");
                goto error;
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
            printf("Usage : %s <id> <commands>\n", pArgv[0]);
            printf("COMMANDS:\n");
            printf("    set <id> [bus_id]\n");
            printf("        Set MODBUS driver ID\n");
            printf("    set <id>\n");
            printf("        Show registers\n");
        }
    }
    
    return  FTE_RET_OK;
} 







FTE_RET   FTE_TURBOMAX_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
  
    return  FTE_RET_OK;
}

FTE_RET   FTE_TURBOMAX_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    int i;
    FTE_TURBOMAX_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_TURBOMAX_MAX20_MAX ; i++)
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
        return  FTE_RET_OK;
    }
    
    return  FTE_TASK_create(FTE_TASK_TURBOMAX, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
} 

FTE_RET   FTE_TURBOMAX_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    return  FTE_RET_OK;
    
}

FTE_UINT32      FTE_TURBOMAX_getUpdateInterval
(   
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32_PTR  pulInterval
)
{
    FTE_TURBOMAX_CONFIG_PTR  pConfig = (FTE_TURBOMAX_CONFIG_PTR)pObj->pConfig;
    
    *pulInterval = pConfig->nInterval;
    
    return  FTE_RET_OK;
}

FTE_RET    FTE_TURBOMAX_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_TURBOMAX_CONFIG_PTR  pConfig = (FTE_TURBOMAX_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_TURBOMAX_statistic
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTICS_PTR pStatistics
)
{
    ASSERT((pObj != NULL) && (pStatistics != NULL));
    
    //FTE_TURBOMAX_STATUS_PTR    pStatus = (FTE_TURBOMAX_STATUS_PTR)pObj->pStatus;
        
    
    return  FTE_RET_OK;
}

FTE_RET  FTE_TURBOMAX_getChildCount
(   
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pCount
)
{
    ASSERT((pObj != NULL) && (pCount != NULL));
    
    *pCount = pObj->pConfig->xCommon.ulChild;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_getChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_OBJECT_ID _PTR_ pxChildID
)
{
    ASSERT((pObj != NULL) && (pxChildID != NULL));
    FTE_UINT32  ulGroupID;
    FTE_OBJECT_PTR  pChild;
        
    if (pObj->pConfig->xCommon.ulChild <= ulIndex)
    {
        return  FTE_RET_OBJECT_NOT_FOUND;
    }

    ulGroupID = (pObj->pConfig->xCommon.nID & 0x0000FF00);
    

    pChild = FTE_OBJ_getAt(0x00800000 | ulGroupID, 0x0080FF00, ulIndex, FALSE);
    if (pChild != NULL)
    {
        *pxChildID = pChild->pConfig->xCommon.nID;
        return  FTE_RET_OK;
    }

    return  FTE_RET_OBJECT_NOT_FOUND;
}

FTE_RET FTE_TURBOMAX_attachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID   xChildID
)
{
    ASSERT(pObj != NULL);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_detachChild
(
    FTE_OBJECT_PTR pObj, 
    FTE_OBJECT_ID   xChildID
)
{
    ASSERT(pObj != NULL);
    
    //FTE_TURBOMAX_STATUS_PTR  pStatus = (FTE_TURBOMAX_STATUS_PTR)pObj->pStatus;

    FTE_UINT32  ulGroupID = ((pObj->pConfig->xCommon.nID & 0x00FF0000) >> 8);
    
    if (ulGroupID != (xChildID & 0x0000FF00))
    {
        return  FTE_RET_INVALID_OBJECT;
    }
    
        
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TURBOMAX_getMulti
(
    FTE_OBJECT_PTR  pObject, 
    FTE_UINT32      ulIndex,
    FTE_VALUE_PTR   pValue
)
{
    ASSERT (pObject != NULL);
    ASSERT(pValue != NULL);

    FTE_TURBOMAX_STATUS_PTR pStatus;

    pStatus = (FTE_TURBOMAX_STATUS_PTR)pObject->pStatus;

    if (ulIndex >=  108)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_VALUE_setULONG(pValue, pStatus->pRegisters[ulIndex]);
}






#endif

