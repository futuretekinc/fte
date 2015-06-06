#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"

#if FTE_MULTI_SUPPORTED
 
#ifndef FTE_MULTI_TRIAL_MAX
    #define FTE_MULTI_TRIAL_MAX 3
#endif

static  _mqx_uint   _multi_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _multi_stop(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _multi_start_measurement(FTE_OBJECT_PTR pObj);
static  void        _multi_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static  _mqx_uint   _multi_get(FTE_OBJECT_PTR pObj, uint_32_ptr pValue, TIME_STRUCT *xTimeStamp);
static  _mqx_uint   _multi_set(FTE_OBJECT_PTR pSelf, uint_32 nValue);
static  _mqx_uint   _multi_set_multi(FTE_OBJECT_PTR pSelf, uint_32 nIndex, uint_32 nValue);
static  void        _multi_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static  uint_32     _multi_get_update_interval(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _multi_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval);
#if 0
static  _mqx_uint   _multi_get_event_type(FTE_OBJECT_PTR pObj, uint_32_ptr pEventType);
static  _mqx_uint   _multi_set_event_type(FTE_OBJECT_PTR pObj, uint_32   nEventType);
static  _mqx_uint   _multi_get_upper_limit(FTE_OBJECT_PTR pObj, uint_32_ptr pValue);
static  _mqx_uint   _multi_set_upper_limit(FTE_OBJECT_PTR pObj, uint_32   nValue);
static  _mqx_uint   _multi_get_lower_limit(FTE_OBJECT_PTR pObj, uint_32_ptr pValue);
static  _mqx_uint   _multi_set_lower_limit(FTE_OBJECT_PTR pObj, uint_32   nValue);
static  _mqx_uint   _multi_get_threshold(FTE_OBJECT_PTR pObj, uint_32_ptr pValue);
static  _mqx_uint   _multi_set_threshold(FTE_OBJECT_PTR pObj, uint_32   nValue);
static  _mqx_uint   _multi_get_event_delay(FTE_OBJECT_PTR pObj, uint_32_ptr pValue);
static  _mqx_uint   _multi_set_event_delay(FTE_OBJECT_PTR pObj, uint_32   nValue);
static  _mqx_uint   _multi_event_condition(FTE_OBJECT_PTR pObj, boolean *pResult);
#endif
static  _mqx_uint   _multi_get_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTIC_PTR pStatistic);
static  FTE_MULTI_MODEL_INFO_CONST_PTR _multi_get_model_info(uint_32 nModel);


static  FTE_OBJECT_ACTION _Action = 
{
    .f_run          = _multi_run,
    .f_stop         = _multi_stop, 
    .f_get          = _multi_get,
    .f_set          = _multi_set,
    .f_set_multi    = _multi_set_multi,
    .f_get_update_interval  = _multi_get_update_interval,
    .f_set_update_interval  = _multi_set_update_interval,
#if 0
    .f_get_event_type       = _multi_get_event_type,
    .f_set_event_type       = _multi_set_event_type,
    .f_get_upper_limit      = _multi_get_upper_limit,
    .f_set_upper_limit      = _multi_set_upper_limit,
    .f_get_lower_limit      = _multi_get_lower_limit,
    .f_set_lower_limit      = _multi_set_lower_limit,
    .f_get_threshold        = _multi_get_threshold,
    .f_set_threshold        = _multi_set_threshold,
    .f_get_event_delay      = _multi_get_event_delay,
    .f_set_event_delay      = _multi_set_event_delay,
    .f_event_condition      = _multi_event_condition,
#endif
    .f_get_statistic        = _multi_get_statistic
}; 
 
const FTE_MULTI_MODEL_INFO  _pMULTIModelInfos[] =
{
    FTE_MODEL_BOTEM_PN1500_SENS
};


_mqx_uint   fte_multi_attach(FTE_OBJECT_PTR pObj)
{
    FTE_MULTI_STATUS_PTR              pStatus;
    FTE_UCS_PTR                     pUCS = NULL;
    
    ASSERT(pObj != NULL);

    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;    
    
    pStatus->pModelInfo = _multi_get_model_info(((FTE_MULTI_CONFIG_PTR)pObj->pConfig)->nModel);
    if (pStatus->pModelInfo == NULL)
    {
        goto error;
    }
    
    if (pStatus->pModelInfo->f_attach != NULL)
    {
        if (pStatus->pModelInfo->f_attach(pObj) != MQX_OK)
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
        
        if (fte_ucs_attach(pUCS, pObj->pConfig->nID) != MQX_OK)
        {
            goto error;
        }
        
        pStatus->pUCS = pUCS;
    }

    pObj->pAction = &_Action;

    return  MQX_OK;
    
error:
    if (pUCS != NULL)
    {
        fte_ucs_detach(pUCS, 0);
    }
    
    return  MQX_ERROR;
    
}

_mqx_uint fte_multi_detach(FTE_OBJECT_PTR pObj)
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
    
    return  MQX_OK;
}

FTE_MULTI_MODEL_INFO_CONST_PTR _multi_get_model_info(uint_32 nModel)
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

_mqx_uint   _multi_run(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);

    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    FTE_MULTI_CONFIG_PTR    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    _multi_stop(pObj);
    _multi_start_measurement(pObj);
    
    pStatus->hRepeatTimer   = FTE_OBJ_runLoop(pObj, _multi_restart_convert, pConfig->nInterval * 1000);    

    return  MQX_OK;
}

_mqx_uint   _multi_stop(FTE_OBJECT_PTR pObj)
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
    
    return  MQX_OK;
    
}

_mqx_uint _multi_start_measurement(FTE_OBJECT_PTR pObj)
{
    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));
    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
        
    if (pStatus->pModelInfo->f_request_data != NULL)
    {
        pStatus->pModelInfo->f_request_data(pObj);
        pStatus->hConvertTimer  = FTE_OBJ_runMeasurement(pObj, _multi_done, FTE_MULTI_RESPONSE_TIME);    
#if FTE_DEBUG
        pStatus->xStatistic.nTotalTrial++;
#endif    
    }

    return  MQX_OK;
    
}

void _multi_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR          pObj = (FTE_OBJECT_PTR)data_ptr;
//    FTE_MULTI_CONFIG_PTR    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    //boolean                 bOccurred = FALSE;
    
    if (pStatus->pModelInfo->f_receive_data(pObj) != MQX_OK)
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


static void _multi_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        _multi_start_measurement(pObj);
    }
    else
    {
        _multi_stop(pObj);
    }
}

_mqx_uint    _multi_get(FTE_OBJECT_PTR pObj, uint_32 *pValue, TIME_STRUCT *xTimeStamp)
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
        
        return  MQX_OK;
    }

    return  MQX_ERROR;
}

_mqx_uint    _multi_set(FTE_OBJECT_PTR pObj, uint_32 nValue)
{
    ASSERT(pObj != NULL);
    
//    FTE_MULTI_STATUS_PTR  pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        return  MQX_OK;
    }

    return  MQX_ERROR;
}

_mqx_uint    _multi_set_multi(FTE_OBJECT_PTR pObj, uint_32 nIndex, uint_32 nValue)
{
    ASSERT(pObj != NULL);
    
    FTE_MULTI_STATUS_PTR  pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if ((nIndex < 16) && (pStatus->pModelInfo->f_set != NULL))
        {
            pStatus->pModelInfo->f_set(pObj, nIndex, nValue);
           
            return  MQX_OK;
        }
    }

    return  MQX_ERROR;
}

uint_32      _multi_get_update_interval(FTE_OBJECT_PTR pObj)
{
    FTE_MULTI_CONFIG_PTR  pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

_mqx_uint    _multi_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval)
{
    FTE_MULTI_CONFIG_PTR  pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  MQX_OK;
}


#if 0
_mqx_uint   _multi_get_event_type(FTE_OBJECT_PTR pObj, uint_32_ptr pEventType)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;
    
    *pEventType = pConfig->nEventType;
    
    return  MQX_OK;
}

_mqx_uint   _multi_set_event_type(FTE_OBJECT_PTR pObj, uint_32   nEventType)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nEventType = nEventType;
    
    return  MQX_OK;
}

_mqx_uint   _multi_get_upper_limit(FTE_OBJECT_PTR pObj, uint_32_ptr pValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nUpperLimit;
    
    return  MQX_OK;
}

_mqx_uint   _multi_set_upper_limit(FTE_OBJECT_PTR pObj, uint_32   nValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nUpperLimit = nValue;
    
    return  MQX_OK;
}

_mqx_uint   _multi_get_lower_limit(FTE_OBJECT_PTR pObj, uint_32_ptr pValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nLowerLimit;
    
    return  MQX_OK;
}

_mqx_uint   _multi_set_lower_limit(FTE_OBJECT_PTR pObj, uint_32   nValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

   pConfig->nLowerLimit = nValue;
    
    return  MQX_OK;
}

_mqx_uint   _multi_get_threshold(FTE_OBJECT_PTR pObj, uint_32_ptr pValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nThreshold;
    
    return  MQX_OK;
}

_mqx_uint   _multi_set_threshold(FTE_OBJECT_PTR pObj, uint_32   nValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nThreshold = nValue;
    
    return  MQX_OK;
}

_mqx_uint   _multi_get_event_delay(FTE_OBJECT_PTR pObj, uint_32_ptr pValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    *pValue = pConfig->nEventDelay;
    
    return  MQX_OK;
}

_mqx_uint   _multi_set_event_delay(FTE_OBJECT_PTR pObj, uint_32   nValue)
{
    FTE_MULTI_CONFIG_PTR    pConfig;
    
    ASSERT(pObj != NULL);
           
    pConfig = (FTE_MULTI_CONFIG_PTR)pObj->pConfig;

    pConfig->nEventDelay = nValue;
    
    return  MQX_OK;
}

_mqx_uint   _multi_event_condition(FTE_OBJECT_PTR pObj, boolean *pResult)
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
    
    return  MQX_OK;
}
#endif

static  _mqx_uint   _multi_get_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTIC_PTR pStatistic)
{
    ASSERT((pObj != NULL) && (pStatistic != NULL));
    
    FTE_MULTI_STATUS_PTR    pStatus = (FTE_MULTI_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistic, &pStatus->xStatistic, sizeof(FTE_OBJECT_STATISTIC));
    
    return  MQX_OK;
}


#endif