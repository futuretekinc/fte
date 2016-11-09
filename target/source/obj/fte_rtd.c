#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"

#if FTE_RTD_SUPPORTED

static  
FTE_RET     FTE_RTD_init(FTE_OBJECT_PTR pObj);

static  
FTE_RET     FTE_RTD_run(FTE_OBJECT_PTR pObj);

static  
FTE_RET     FTE_RTD_stop(FTE_OBJECT_PTR pObj);

static  
void        _rtd_done(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);

static  
void        _rtd_restart_convert(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);

static 
FTE_RET     FTE_RTD_get(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR value, TIME_STRUCT_PTR pTimeStamp);

static 
FTE_UINT32  FTE_RTD_getUpdateInterval(FTE_OBJECT_PTR pObj);

static 
FTE_RET     FTE_RTD_setUpdateInterval(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);


FTE_RTD_CONFIG FTE_RTD_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_PT100, 0x0001),
        .pName      = "RTD",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = FTE_DEV_AD7785_0,
    .nInterval  = FTE_RTD_INTERVAL
};

static  FTE_OBJECT_ACTION FTE_RTD_action =  
{
    .f_init         = FTE_RTD_init,
    .f_run          = FTE_RTD_run,
    .f_stop         = FTE_RTD_stop,
    .f_get_update_interval = FTE_RTD_getUpdateInterval,
    .f_set_update_interval = FTE_RTD_setUpdateInterval
};

static const 
FTE_INT32 FTE_RTD_PT100[] = 
{   
    -24688, -22295, -19886, -17461, -15020, -12561, -10085, -7591, -5080, -2549,
    0, 2569, 5157, 7766, 10395, 13046, 15718, 18413, 21131, 23872, 26637 
};

static 
FTE_LIST _xObjList;

FTE_RET   FTE_RTD_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT(pObj != NULL);
    
    FTE_RTD_CONFIG_PTR  pConfig = (FTE_RTD_CONFIG_PTR)pObj->pConfig;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;
    FTE_AD7785_PTR      pADC = NULL;
    
    pADC = FTE_AD7785_get(pConfig->nDevID);
    if (pADC == FALSE)
    {
        goto error;
    }
    
    if (FTE_AD7785_attach(pADC, pConfig->xCommon.nID) != FTE_RET_OK)
    {
        goto error;
    }
    
    pStatus->xCommon.nValueCount = 1;
    pStatus->xCommon.pValue = FTE_VALUE_createTemperature();
    if (pStatus->xCommon.pValue == NULL)
    {
        goto error;
    }
    
    pStatus->pADC = pADC;
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&FTE_RTD_action;
    
    if (FTE_RTD_init(pObj) != FTE_RET_OK)
    {
        FTE_AD7785_detach(pADC);
        pStatus->pADC = NULL;
        
        goto error;
    }

    FTE_LIST_pushBack(&_xObjList, pObj);

    return  FTE_RET_OK;
    
error:
    
    if (pStatus != NULL)
    {
        FTE_AD7785_detach(pADC);
        pObj->pStatus = NULL;
    }
    
    return  FTE_RET_ERROR;
    
}

FTE_RET FTE_RTD_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    if (!FTE_LIST_isExist(&_xObjList, pObj))
    {
        goto error;
    }

    FTE_LIST_remove(&_xObjList, pObj);
    pObj->pAction = NULL;
    
    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}
/*
FTE_UINT32 FTE_RTD_printValue(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen)
{
    ASSERT((pObj != NULL) && FTE_OBJ_CLASS(pObj) == FTE_OBJ_CLASS_TEMPERATURE));

    if (((FTE_RTD_STATUS_PTR)pObj->pStatus)->xCommon.nValue >= 0)
    {
        return  snprintf(pBuff, nLen, "%d.%02d", 
                         ((FTE_RTD_STATUS_PTR)pObj->pStatus)->nValue / 100, 
                         ((FTE_RTD_STATUS_PTR)pObj->pStatus)->nValue % 100);
    }
    else
    {
        FTE_INT32 nValue = 0 - ((FTE_RTD_STATUS_PTR)pObj->pStatus)->nValue;
        
        return  snprintf(pBuff, nLen, "-%d.%02d", nValue / 100, nValue % 100);
    }
}
*/
FTE_RET   FTE_RTD_init
(
    FTE_OBJECT_PTR  pObj
)
{
    assert(pObj != NULL);
    
    FTE_UINT32 setValue, getValue;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    FTE_AD7785_getConfig(pStatus->pADC, &getValue);
    setValue = getValue | (FTE_AD7785_CONFIG_UNIPOLAR_MODE << FTE_AD7785_CONFIG_POL_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_GAIN_MASK) | (FTE_AD7785_CONFIG_GAIN_32 << FTE_AD7785_CONFIG_GAIN_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_REF_MASK) | (FTE_AD7785_CONFIG_REF_EXTR << FTE_AD7785_CONFIG_REF_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_BUF_MASK) | (FTE_AD7785_CONFIG_BUF_MODE << FTE_AD7785_CONFIG_BUF_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_CHANNEL_MASK) | (FTE_AD7785_CONFIG_CHANNEL_2 << FTE_AD7785_CONFIG_CHANNEL_SHIFT);
    FTE_AD7785_setConfig(pStatus->pADC, setValue);
    FTE_AD7785_getConfig(pStatus->pADC, &getValue);
    if (setValue != getValue)
    {
        goto error;
    }

    FTE_AD7785_setIOut(pStatus->pADC, FTE_AD7785_IOUT_DIR_DIRECT);
    FTE_AD7785_getIOut(pStatus->pADC, &getValue);
    if (getValue != FTE_AD7785_IOUT_DIR_DIRECT)
    {
        goto error;
    }
    FTE_AD7785_setCurrent(pStatus->pADC, FTE_AD7785_IOUT_CURRENT_210UA);    
    FTE_AD7785_getCurrent(pStatus->pADC, &getValue);    
    if (getValue != FTE_AD7785_IOUT_CURRENT_210UA)
    {
        goto error;
    }
    
    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_RTD_run
(
    FTE_OBJECT_PTR  pObj
)
{
    assert(pObj != NULL);

    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;
    FTE_RTD_CONFIG_PTR  pConfig = (FTE_RTD_CONFIG_PTR)pObj->pConfig;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    FTE_RTD_init(pObj);
    
    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_msec_to_ticks(&xTicks, 1000);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_rtd_restart_convert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);
    FTE_AD7785_setOPMode(pStatus->pADC, FTE_AD7785_OP_MODE_SINGLE);

    _time_init_ticks(&xDTicks, _time_get_ticks_per_sec());
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_rtd_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);

    
    return  FTE_RET_OK;
}
 
FTE_RET   FTE_RTD_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    assert(pObj != NULL);
    
    FTE_RTD_STATUS_PTR   pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

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

void _rtd_done
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_UINT32          data = 0;
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    if (FTE_AD7785_getRawData(pStatus->pADC, &data) == FTE_RET_OK)
    {
        FTE_UINT32 index, value;
        FTE_INT32  temp;
        
        value = (FTE_UINT32)((float)(data) / (float)(1 << FTE_AD7785_CONFIG_GAIN_32) / 0x100000 * 4990 * 100);
            
        index = value / 1000;
    
        if (index < 20)
        {
            temp = FTE_RTD_PT100[index] + (FTE_RTD_PT100[index+1] - FTE_RTD_PT100[index]) * (value - index * 1000) / 1000;        
        }
        else
        {
            temp = FTE_RTD_PT100[20];
        }
        
        if (FTE_RTD_LOW_BOUND <= temp && temp <= FTE_RTD_HIGH_BOUND)
        {
            FTE_VALUE_setTemperature(pStatus->xCommon.pValue, temp);
            FT_OBJ_STAT_incSucceed(&pStatus->xCommon.xStatistics);

        }
        else
        {
            FTE_VALUE_setValid(pStatus->xCommon.pValue, FALSE);
            FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
            FTE_RTD_init(pObj);
        }
    }
    else
    {
            FTE_VALUE_setValid(pStatus->xCommon.pValue, FALSE);
            FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
    }
}

void _rtd_restart_convert
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR     pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        FTE_AD7785_setOPMode(pStatus->pADC, FTE_AD7785_OP_MODE_SINGLE);
        
        _time_init_ticks(&xDTicks, _time_get_ticks_per_sec());
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_rtd_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);
    }
    else
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

#if 0
FTE_RET    FTE_RTD_get
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  value, 
    TIME_STRUCT_PTR pTimeStamp
)
{
    assert(pObj != NULL && value != NULL);
    
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        *value = pStatus->nValue;
        if (pTimeStamp != NULL)
        {
            *pTimeStamp = pStatus->xTimeStamp;
        }
        
        return  FTE_RET_OK;
    }

    return  FTE_RET_ERROR;
}
#endif
FTE_UINT32      FTE_RTD_getUpdateInterval
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_RTD_CONFIG_PTR  pConfig = (FTE_RTD_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

FTE_RET    FTE_RTD_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_RTD_CONFIG_PTR  pConfig = (FTE_RTD_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);

    
    return  FTE_RET_OK;
}

#endif
