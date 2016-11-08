#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_value.h"

#if FTE_GAI_SUPPORTED

FTE_GAI_CONFIG FTE_GAI_VOLTAGE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_GAI_VOLTAGE, 0),
        .pName      = "VOLTAGE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID         = FTE_DEV_AD7785_0,
    .nInterval      = FTE_GAI_DEFAULT_UPDATE_INTERVAL,
    .nGain          = 1,
    .ulDivide       = 1,
    .xValueType     = FTE_GAI_VALUE_TYPE_0_2_TO_1_518V
};

FTE_GAI_CONFIG FTE_GAI_CURRENT_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_GAI_CURRENT, 0),
        .pName      = "CURRENT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID         = FTE_DEV_AD7785_0,
    .nInterval      = FTE_GAI_DEFAULT_UPDATE_INTERVAL,
    .nGain          = 1,
    .ulDivide       = 1,
    .xValueType     = FTE_GAI_VALUE_TYPE_0_2_TO_1_10A
};

static 
FTE_GAI_VALUE_DESCRIPT _pGAIValueDescript[] =
{
    {
        .xType = FTE_GAI_VALUE_TYPE_0_2_TO_1_518V,
        .xInputRange =
        {
            .ulMin  =   200000,
            .ulMax  =   1000000
        },
        .xValueRange = 
        {
            .xType  =   FTE_VALUE_TYPE_VOLTAGE,
            .ulMin  =   0,
            .ulMax  =   518 * 1000
        },
        .ulCalibration = 1010
    },
    {
        .xType = FTE_GAI_VALUE_TYPE_0_2_TO_1_10A,
        .xInputRange =
        {
            .ulMin  =   200000,
            .ulMax  =   1000000
        },
        .xValueRange = 
        {
            .xType  =   FTE_VALUE_TYPE_CURRENT,
            .ulMin  =   0,
            .ulMax  =   10 * 1000
        },
        .ulCalibration = 1010
    },
    {
        .xType = FTE_GAI_VALUE_TYPE_UNKNOWN,
    }
};

static  
FTE_RET FTE_GAI_init(FTE_OBJECT_PTR pObj);

static  
FTE_RET FTE_GAI_run(FTE_OBJECT_PTR pObj);

static  
FTE_RET FTE_GAI_stop(FTE_OBJECT_PTR pObj);

static  
void    FTE_GAI_done(_timer_id id, pointer pData, MQX_TICK_STRUCT_PTR pTick);

static 
void    FTE_GAI_restartConvert(_timer_id id, pointer pData, MQX_TICK_STRUCT_PTR pTick);

static 
FTE_UINT32  FTE_GAI_getUpdateInterval(FTE_OBJECT_PTR pObj);

static 
FTE_RET FTE_GAI_setUpdateInterval(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);

static 
FTE_GAI_VALUE_DESCRIPT_PTR  FTE_GAI_getValueDescript(FTE_GAI_VALUE_TYPE xType);

static  
FTE_OBJECT_ACTION FTE_GAI_action = 
{
    .f_init         = FTE_GAI_init,
    .f_run          = FTE_GAI_run,
    .f_stop         = FTE_GAI_stop,
    .f_get_update_interval = FTE_GAI_getUpdateInterval,
    .f_set_update_interval = FTE_GAI_setUpdateInterval
};

FTE_RET   FTE_GAI_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    FTE_AD7785_PTR      pADC = NULL;
    
    ASSERT(pObj != NULL);
    
    FTE_GAI_CONFIG_PTR  pConfig = (FTE_GAI_CONFIG_PTR)pObj->pConfig;
    FTE_GAI_STATUS_PTR  pStatus = (FTE_GAI_STATUS_PTR)pObj->pStatus;
   
    pADC = FTE_AD7785_get(pConfig->nDevID);
    if (pADC == FALSE)
    {
        goto error;
    }
    
    if (FTE_AD7785_attach(pADC, pConfig->xCommon.nID) != FTE_RET_OK)
    {
        goto error;
    }
    
    pStatus->pValueDescript = FTE_GAI_getValueDescript(pConfig->xValueType);
    if (pStatus->pValueDescript == NULL)
    {
        goto error;
    }
    
    pStatus->pADC = pADC;
    pStatus->xCommon.nValueCount = 1;
    pStatus->xCommon.pValue = FTE_VALUE_create(pStatus->pValueDescript->xValueRange.xType);
    if (pStatus->xCommon.pValue == NULL)
    {
        goto error;
    }
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&FTE_GAI_action;
    pObj->pStatus = (FTE_OBJECT_STATUS_PTR)pStatus;
    
    if (FTE_GAI_init(pObj) != FTE_RET_OK)
    {
        goto error;
    }

    return  FTE_RET_OK;
    
error:
    
    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.pValue = NULL;
        pStatus->xCommon.nValueCount = 0;            
    }
    
    if (pADC != NULL)
    {
        FTE_AD7785_detach(pADC);
    }
    
    return  FTE_RET_ERROR;
    
}

FTE_RET FTE_GAI_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }

     FTE_MEM_free(pObj->pStatus);
     pObj->pAction = NULL;
     pObj->pStatus = NULL;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_GAI_init
(
    FTE_OBJECT_PTR  pObj
)
{
    assert(pObj != NULL);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_GAI_run
(
    FTE_OBJECT_PTR  pObj
)
{
    assert(pObj != NULL);

    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;
    FTE_GAI_CONFIG_PTR  pConfig = (FTE_GAI_CONFIG_PTR)pObj->pConfig;
    FTE_GAI_STATUS_PTR  pStatus = (FTE_GAI_STATUS_PTR)pObj->pStatus;

    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_msec_to_ticks(&xTicks, 1000);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(FTE_GAI_restartConvert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);
    FTE_AD7785_runSingle(pStatus->pADC);

    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, FTE_AD7785_measurementTime(pStatus->pADC));    
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(FTE_GAI_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);

    
    return  FTE_RET_OK;
}
 
FTE_RET   FTE_GAI_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    assert(pObj != NULL);
    
    FTE_GAI_STATUS_PTR   pStatus = (FTE_GAI_STATUS_PTR)pObj->pStatus;

    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    if (pStatus->hConvertTimer != 0)
    {
        _timer_cancel(pStatus->hConvertTimer);
        pStatus->hConvertTimer = 0;
    }
    
    FTE_AD7785_setOPMode(pStatus->pADC, FTE_AD7785_OP_MODE_PWR_DOWN);

    return  FTE_RET_OK;
}

void FTE_GAI_done
(
    _timer_id   id, 
    pointer     pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    ASSERT(pData != NULL);
    
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    FTE_GAI_CONFIG_PTR  pConfig = (FTE_GAI_CONFIG_PTR)pObj->pConfig;
    FTE_GAI_STATUS_PTR  pStatus = (FTE_GAI_STATUS_PTR)pObj->pStatus;
    FTE_UINT32             ulValue = 0;

    if (FTE_AD7785_getScaleData(pStatus->pADC, &ulValue) == FTE_RET_OK)
    {        
        if (pConfig->ulDivide != 0)
        {
            ulValue = ulValue * pConfig->ulDivide;
        }
        
        if (ulValue < pStatus->pValueDescript->xInputRange.ulMin)
        {
            ulValue = pStatus->pValueDescript->xInputRange.ulMin;
        }
        else if (ulValue > pStatus->pValueDescript->xInputRange.ulMax)
        {
            ulValue = pStatus->pValueDescript->xInputRange.ulMax;
        }

        FTE_GAI_VALUE_DESCRIPT_PTR pDescript = pStatus->pValueDescript;
        
        double  fRatio = (double)(ulValue - pDescript->xInputRange.ulMin) / (pDescript->xInputRange.ulMax - pDescript->xInputRange.ulMin) * pDescript->ulCalibration / 1000.0;
        ulValue = (FTE_UINT32)(fRatio * (pDescript->xValueRange.ulMax - pDescript->xValueRange.ulMin)) + pDescript->xValueRange.ulMin;
        
        switch(pDescript->xValueRange.xType)
        {
        case    FTE_VALUE_TYPE_VOLTAGE: FTE_VALUE_setVoltage(pStatus->xCommon.pValue, ulValue); break;
        case    FTE_VALUE_TYPE_CURRENT: FTE_VALUE_setCurrent(pStatus->xCommon.pValue, ulValue); break;
        }
    }
    else
    {
        FTE_GAI_init(pObj);
    }
}

void FTE_GAI_restartConvert
(
    _timer_id   id, 
    pointer     pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_GAI_STATUS_PTR  pStatus = (FTE_GAI_STATUS_PTR)pObj->pStatus;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        FTE_AD7785_runSingle(pStatus->pADC);
        
        _time_init_ticks(&xDTicks, 0);
        _time_add_msec_to_ticks(&xDTicks, FTE_AD7785_measurementTime(pStatus->pADC));    
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(FTE_GAI_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);
    }
    else
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

FTE_UINT32  FTE_GAI_getUpdateInterval
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GAI_CONFIG_PTR  pConfig = (FTE_GAI_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

FTE_RET    FTE_GAI_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_GAI_CONFIG_PTR  pConfig = (FTE_GAI_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);

    
    return  FTE_RET_OK;
}

FTE_GAI_VALUE_DESCRIPT_PTR  FTE_GAI_getValueDescript
(
    FTE_GAI_VALUE_TYPE  xType
)
{
    FTE_INT32   i;
    
    for(i = 0 ; _pGAIValueDescript[i].xType != FTE_GAI_VALUE_TYPE_UNKNOWN ; i++)
    {
        if (_pGAIValueDescript[i].xType == xType)
        {
            return  &_pGAIValueDescript[i];
        }
    }
    
    return  NULL;
}

#endif
