#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"

#if FTE_MULTI_SUPPORTED
 
#ifndef FTE_MULTI_TRIAL_MAX
    #define FTE_MULTI_TRIAL_MAX 3
#endif

static  FTE_RET   FTE_MULTI_run(FTE_OBJECT_PTR pObj);
static  FTE_RET   FTE_MULTI_stop(FTE_OBJECT_PTR pObj);
static  FTE_RET   FTE_MULTI_startMeasurement(FTE_OBJECT_PTR pObj);
static  void      FTE_MULTI_done(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);
static  FTE_RET   FTE_MULTI_get(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pValue, TIME_STRUCT_PTR xTimeStamp);
static  FTE_RET   FTE_MULTI_set(FTE_OBJECT_PTR pSelf, FTE_UINT32 nValue);
static  FTE_RET   FTE_MULTI_setMulti(FTE_OBJECT_PTR pSelf, FTE_UINT32 nIndex, FTE_UINT32 nValue);
static  void      FTE_MULTI_restartConvert(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);
static  FTE_UINT32     FTE_MULTI_getUpdateInterval(FTE_OBJECT_PTR pObj);
static  FTE_RET   FTE_MULTI_setUpdateInterval(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);
#if 0
static  FTE_RET   FTE_MULTI_getEventType(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pEventType);
static  FTE_RET   FTE_MULTI_setEventType(FTE_OBJECT_PTR pObj, FTE_UINT32   nEventType);
static  FTE_RET   FTE_MULTI_getUpperLimit(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pValue);
static  FTE_RET   FTE_MULTI_setUpperLimit(FTE_OBJECT_PTR pObj, FTE_UINT32   nValue);
static  FTE_RET   FTE_MULTI_getLowerLimit(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pValue);
static  FTE_RET   FTE_MULTI_setLowerLimit(FTE_OBJECT_PTR pObj, FTE_UINT32   nValue);
static  FTE_RET   FTE_MULTI_getThreshold(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pValue);
static  FTE_RET   FTE_MULTI_setThreshold(FTE_OBJECT_PTR pObj, FTE_UINT32   nValue);
static  FTE_RET   FTE_MULTI_getEventDelay(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pValue);
static  FTE_RET   FTE_MULTI_setEventDelay(FTE_OBJECT_PTR pObj, FTE_UINT32   nValue);
static  FTE_RET   FTE_MULTI_eventCondition(FTE_OBJECT_PTR pObj, FTE_BOOL_PTR pResult);
#endif
static  FTE_RET   FTE_MULTI_get_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTIC_PTR pStatistic);
static  FTE_MULTI_MODEL_INFO_CONST_PTR FTE_MULTI_getModelInfo(FTE_UINT32 nModel);


static  FTE_OBJECT_ACTION _Action = 
{
    .f_run          = FTE_MULTI_run,
    .f_stop         = FTE_MULTI_stop, 
    .f_get          = FTE_MULTI_get,
    .f_set          = FTE_MULTI_set,
    .f_set_multi    = FTE_MULTI_setMulti,
    .f_get_update_interval  = FTE_MULTI_getUpdateInterval,
    .f_set_update_interval  = FTE_MULTI_setUpdateInterval,
#if 0
    .f_get_event_type       = FTE_MULTI_getEventType,
    .f_set_event_type       = FTE_MULTI_setEventType,
    .f_get_upper_limit      = FTE_MULTI_getUpperLimit,
    .f_set_upper_limit      = FTE_MULTI_setUpperLimit,
    .f_get_lower_limit      = FTE_MULTI_getLowerLimit,
    .f_set_lower_limit      = FTE_MULTI_setLowerLimit,
    .f_get_threshold        = FTE_MULTI_getThreshold,
    .f_set_threshold        = FTE_MULTI_setThreshold,
    .f_get_event_delay      = FTE_MULTI_getEventDelay,
    .f_set_event_delay      = FTE_MULTI_setEventDelay,
    .f_event_condition      = FTE_MULTI_eventCondition,
#endif
    .f_get_statistic        = FTE_MULTI_get_statistic
}; 
 
const 
FTE_MULTI_MODEL_INFO  _pMULTIModelInfos[] =
{
    FTE_MODEL_BOTEM_PN1500_SENS
};


FTE_RET   FTE_MULTI_attach
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_MULTI_STATUS_PTR              pStatus;
    FTE_UCS_PTR                     pUCS = NULL;
    
    ASSERT(pObj != NULL);

    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;    
    
    pStatus->pModelInfo = FTE_MULTI_getModelInfo(((FTE_MULTI_CONFIG_PTR)pObj->pConfig)->nModel);
    if (pStatus->pModelInfo == NULL)
    {
        goto error;
    }
    
    if (pStatus->pModelInfo->f_attach != NULL)
    {
        if (pStatus->pModelInfo->f_attach(pObj) != FTE_RET_OK)
        {
            goto error;
        }
        
        if (pObj->pAction == NULL)
        {
            pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_Action;
        }
    }
    else
    {
        pUCS = (FTE_UCS_PTR)fte_ucs_get(((FTE_MULTI_CONFIG_PTR)pObj->pConfig)->nUCSID);
        if (pUCS == NULL)
        {
            goto error;
        }
        
        if (fte_ucs_attach(pUCS, pObj->pConfig->nID) != FTE_RET_OK)
        {
            goto error;
        }
        
        pStatus->pUCS = pUCS;
    }

    pObj->pAction = &_Action;

    return  FTE_RET_OK;
    
error:
    if (pUCS != NULL)
    {
        fte_ucs_detach(pUCS, 0);
    }
    
    return  FTE_RET_ERROR;
    
}

FTE_RET FTE_MULTI_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_MULTI_STATUS_PTR  pStatus;

    ASSERT(pObj != NULL);

    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    if (pStatus->pModelInfo->f_detach != NULL)
    {
        return  pStatus->pModelInfo->f_detach(pObj);
    }

    if (pStatus->pUCS != NULL)
    {
        fte_ucs_detach(pStatus->pUCS, pObj->pConfig->nID);
        pStatus->pUCS = NULL;
    }
    
    FTE_MEM_free(pStatus);
    pObj->pAction = NULL;
    pObj->pStatus = NULL;
    
    return  FTE_RET_OK;
}

FTE_MULTI_MODEL_INFO_CONST_PTR FTE_MULTI_getModelInfo
(
    FTE_UINT32  nModel
)
{
    for(int i = 0 ; i < sizeof(_pMULTIModelInfos) / sizeof(FTE_MULTI_MODEL_INFO) ; i++)
    {
        if (nModel == _pMULTIModelInfos[i].nModel)
        {
            return  &_pMULTIModelInfos[i];
        }
    }
    
    return  NULL;
}

FTE_RET   FTE_MULTI_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    FTE_MULTI_CONFIG_PTR    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    FTE_MULTI_stop(pObj);
    FTE_MULTI_startMeasurement(pObj);
    
    return  FTE_OBJ_runLoop(pObj, FTE_MULTI_restartConvert, pConfig->nInterval, &pStatus->hRepeatTimer);    
}

FTE_RET   FTE_MULTI_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->hConvertTimer != 0)
    {
        _timer_cancel(pStatus->hConvertTimer);
        pStatus->hConvertTimer = 0;
    }

    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    return  FTE_RET_OK;
    
}

FTE_RET FTE_MULTI_startMeasurement
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));
    FTE_RET xRet;
    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
        
    if (pStatus->pModelInfo->f_request_data != NULL)
    {
        pStatus->pModelInfo->f_request_data(pObj);
        xRet = FTE_OBJ_runMeasurement(pObj, FTE_MULTI_done, FTE_MULTI_RESPONSE_TIME, &pStatus->hConvertTimer);    
#if FTE_DEBUG
        pStatus->xStatistic.nTotalTrial++;
#endif    
    }
    else
    {
        xRet = FTE_RET_NOT_SUPPORTED_FUNCTION;
    }

    return  xRet;
    
}

void FTE_MULTI_done
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR          pObj = (FTE_OBJECT_PTR)pData;
//    FTE_MULTI_CONFIG_PTR    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    //FTE_BOOL                 bOccurred = FALSE;
    
    if (pStatus->pModelInfo->f_receive_data(pObj) != FTE_RET_OK)
    {
#if FTE_DEBUG
        pStatus->xStatistic.nTotalFail++;
#endif          
        goto error;
    }

    pStatus->nTrial = 0;
    
    return;
    
error:
    if (++pStatus->nTrial > FTE_MULTI_TRIAL_MAX)
    {        
        pStatus->xFlags = FTE_FLAG_CLR(pStatus->xFlags, FTE_OBJ_STATUS_FLAG_VALID);
    }
}


void FTE_MULTI_restartConvert
(   
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        FTE_MULTI_startMeasurement(pObj);
    }
    else
    {
        FTE_MULTI_stop(pObj);
    }
}

FTE_RET    FTE_MULTI_get
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pValue, 
    TIME_STRUCT_PTR xTimeStamp
)
{
    ASSERT(pObj != NULL && pValue != NULL);
    
    FTE_MULTI_STATUS_PTR  pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        *pValue = pStatus->nValue;
        if (xTimeStamp != NULL)
        {
            *xTimeStamp = pStatus->xTimeStamp;
        }
        
        return  FTE_RET_OK;
    }

    return  FTE_RET_ERROR;
}

FTE_RET    FTE_MULTI_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nValue
)
{
    ASSERT(pObj != NULL);
    
//    FTE_MULTI_STATUS_PTR  pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        return  FTE_RET_OK;
    }

    return  FTE_RET_ERROR;
}

FTE_RET    FTE_MULTI_setMulti
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nIndex, 
    FTE_UINT32      nValue
)
{
    ASSERT(pObj != NULL);
    
    FTE_MULTI_STATUS_PTR  pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if ((nIndex < 16) && (pStatus->pModelInfo->f_set != NULL))
        {
            pStatus->pModelInfo->f_set(pObj, nIndex, nValue);
           
            return  FTE_RET_OK;
        }
    }

    return  FTE_RET_ERROR;
}

FTE_UINT32  FTE_MULTI_getUpdateInterval
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_MULTI_CONFIG_PTR  pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

FTE_RET    FTE_MULTI_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_MULTI_CONFIG_PTR  pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}


#if 0
FTE_RET   FTE_MULTI_getEventType
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pEventType
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    *pEventType = pConfig->nEventType;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_setEventType
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nEventType
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nEventType = nEventType;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_getUpperLimit
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nUpperLimit;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_setUpperLimit
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nUpperLimit = nValue;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_getLowerLimit
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nLowerLimit;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_setLowerLimit
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

   pConfig->nLowerLimit = nValue;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_getThreshold
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nThreshold;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_setThreshold
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nThreshold = nValue;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_getEventDelay
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nEventDelay;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_setEventDelay
(   
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nValue
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nEventDelay = nValue;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_MULTI_eventCondition
(
    FTE_OBJECT_PTR  pObj, 
    FTE_BOOL_PTR    pResult
)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    FTE_MULTI_STATUS_PTR    pStatus;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    
    switch(pConfig->nEventType)
    {
    case    FTE_EVENT_TYPE_ABOVE:
        if (pConfig->nUpperLimit <= pStatus->nValue)
        {
            *pResult = TRUE;
        }
        break;
        
    case    FTE_EVENT_TYPE_BELOW:
        if (pConfig->nLowerLimit >= pStatus->nValue)
        {
            *pResult = TRUE;
        }
        break;
        
    case    FTE_EVENT_TYPE_INSIDE:
        if ((pConfig->nUpperLimit >= pStatus->nValue) && (pConfig->nLowerLimit <= pStatus->nValue))
        {
            *pResult = TRUE;
        }
        break;
        
    case    FTE_EVENT_TYPE_OUTSIDE:
        if ((pConfig->nUpperLimit <= pStatus->nValue) || (pConfig->nLowerLimit >= pStatus->nValue))
        {
            *pResult = TRUE;
        }
        break;
        
    default:
        {
            *pResult = FALSE;    
        }
    }
    
    return  FTE_RET_OK;
}
#endif

FTE_RET   FTE_MULTI_get_statistic
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTIC_PTR pStatistic
)
{
    ASSERT((pObj != NULL) && (pStatistic != NULL));
    
    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistic, &pStatus->xStatistic, sizeof(FTE_OBJECT_STATISTIC));
    
    return  FTE_RET_OK;
}


#endif