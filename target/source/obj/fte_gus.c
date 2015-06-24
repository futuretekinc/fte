#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "fte_value.h"

#if FTE_GUS_SUPPORTED
 
#ifndef FTE_GUS_TRIAL_MAX
    #define FTE_GUS_TRIAL_MAX 3
#endif

static  _mqx_uint   _gus_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _gus_stop(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _gus_start_measurement(FTE_OBJECT_PTR pObj);
static  void        _gus_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static  _mqx_uint   _gus_set_multi(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue);
static  void        _gus_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static  uint_32     _gus_get_update_interval(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _gus_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval);
static  _mqx_uint   _gus_get_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistic);
static  _mqx_uint   _gus_set_config(FTE_OBJECT_PTR pObj, char_ptr pJSON);
static  _mqx_uint   _gus_get_config(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen);

static  FTE_GUS_MODEL_INFO_CONST_PTR _gus_get_model_info(uint_32 nModel);
 
static  FTE_OBJECT_ACTION _Action = 
{ 
    .f_run          = _gus_run,
    .f_stop         = _gus_stop, 
    .f_set_multi    = _gus_set_multi,
    .f_get_update_interval  = _gus_get_update_interval,
    .f_set_update_interval  = _gus_set_update_interval,
    .f_get_statistic        = _gus_get_statistic,
    .f_set_config           = _gus_set_config,
    .f_get_config           = _gus_get_config
}; 
 
const FTE_GUS_MODEL_INFO  _pGUSModelInfos[] =
{
    FTE_PM1001_DESCRIPTOR,
    FTE_COZIR_AX5000_DESCRIPTOR,
    FTE_TASCON_HEM12_DESCRIPTOR,
    FTE_FIS3061_DESCRIPTOR,
    FTE_SH_MV250_DESCRIPTOR,
    FTE_TRUEYES_AIRQ_DESCRIPTOR,
    FTE_ELT_AQM100_DESCRIPTOR,
    FTE_MST_MEX510C_DESCRIPTOR,
    FTE_GS_DPC_HL_DESCRIPTOR,
    FTE_TASCON_HEM12_06M_DESCRIPTOR,
    FTE_BOTEM_PN1500_SENS,
    FTE_FTLM_SENS
};
 
_mqx_uint   FTE_GUS_attach(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR              pStatus;
    FTE_UCS_PTR                     pUCS = NULL;
    
    ASSERT(pObj != NULL);

    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;    
    
    pStatus->pModelInfo = _gus_get_model_info(((FTE_GUS_CONFIG_PTR)pObj->pConfig)->nModel);
    if (pStatus->pModelInfo == NULL)
    {
        goto error;
    }    
    
    pStatus->xCommon.nValueCount = pStatus->pModelInfo->nFieldCount;
    pStatus->xCommon.pValue = FTE_VALUE_createArray(pStatus->pModelInfo->pValueTypes, pStatus->pModelInfo->nFieldCount);
        
    pObj->pAction = &_Action;

    if (pStatus->pModelInfo->f_attach != NULL)
    {
        if (pStatus->pModelInfo->f_attach(pObj) != MQX_OK)
        {
            goto error;
        }
    }
    else
    {
        pUCS = (FTE_UCS_PTR)FTE_UCS_get(((FTE_GUS_CONFIG_PTR)pObj->pConfig)->nUCSID);
        if (pUCS == NULL)
        {
            goto error;
        }
        
        if (FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID) != MQX_OK)
        {
            goto error;
        }
        
        pStatus->pUCS = pUCS;
    }
    
    return  MQX_OK;
    
error:
    if (pUCS != NULL)
    {
        FTE_UCS_detach(pUCS, 0);
    }
    
    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.nValueCount = 0;
        pStatus->xCommon.pValue = NULL;
    }
    
    return  MQX_ERROR;
    
}

_mqx_uint FTE_GUS_detach(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus;

    ASSERT(pObj != NULL);

    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    if (pStatus->pModelInfo->f_detach != NULL)
    {
        return  pStatus->pModelInfo->f_detach(pObj);
    }

    if (pStatus->pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->pUCS, pObj->pConfig->xCommon.nID);
        pStatus->pUCS = NULL;
    }
    
    pObj->pAction = NULL;
    
    return  MQX_OK;
}

FTE_GUS_MODEL_INFO_CONST_PTR _gus_get_model_info(uint_32 nModel)
{
    for(int i = 0 ; i < sizeof(_pGUSModelInfos) / sizeof(FTE_GUS_MODEL_INFO) ; i++)
    {
        if (nModel == _pGUSModelInfos[i].nModel)
        {
            return  &_pGUSModelInfos[i];
        }
    }
    
    return  NULL;
}

_mqx_uint   _gus_run(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);

    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR    pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    _gus_stop(pObj);
    _gus_start_measurement(pObj);
    
    pStatus->hRepeatTimer   = FTE_OBJ_runLoop(pObj, _gus_restart_convert, pConfig->nInterval * 1000);    

    return  MQX_OK;
}

_mqx_uint   _gus_stop(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);

    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
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

_mqx_uint _gus_start_measurement(FTE_OBJECT_PTR pObj)
{
    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
        
    _time_get_ticks(&pStatus->xCommon.xStartTicks);
    
    if (pStatus->pModelInfo->f_request != NULL)
    {
        pStatus->pModelInfo->f_request(pObj);
        if (pStatus->pModelInfo->nMaxResponseTime != 0)
        {
            pStatus->hConvertTimer  = FTE_OBJ_runMeasurement(pObj, _gus_done, pStatus->pModelInfo->nMaxResponseTime);
        }
        else
        {
            pStatus->hConvertTimer  = FTE_OBJ_runMeasurement(pObj, _gus_done, FTE_GUS_RESPONSE_TIME);    
        }
        
    }

    return  MQX_OK;
    
}

void _gus_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR        pObj = (FTE_OBJECT_PTR)data_ptr;
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->pModelInfo->f_received(pObj) != MQX_OK)
    {
        FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
        goto error;
    }

    FT_OBJ_STAT_incSucceed(&pStatus->xCommon.xStatistics);
    pStatus->nTrial = 0;

    FTE_OBJ_wasUpdated(pObj);

    return;
    
error:
    if (++pStatus->nTrial > FTE_GUS_TRIAL_MAX)
    {        
        pStatus->xCommon.xFlags = FTE_FLAG_CLR(pStatus->xCommon.xFlags, FTE_OBJ_STATUS_FLAG_VALID);
    }
}


static void _gus_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        _gus_start_measurement(pObj);
    }
    else
    {
        _gus_stop(pObj);
    }
}

_mqx_uint    _gus_set_multi(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pModelInfo->f_set != NULL)
        {
            return  pStatus->pModelInfo->f_set(pObj, nIndex, pValue);
        }
    }

    return  MQX_ERROR;
}

_mqx_uint   _gus_set_config(FTE_OBJECT_PTR pObj, char_ptr pJSON)
{
    ASSERT((pObj != NULL) && (pJSON != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pModelInfo->f_set_config != NULL)
        {
            return  pStatus->pModelInfo->f_set_config(pObj, pJSON);
        }
    }

    return  MQX_ERROR;
}

_mqx_uint   _gus_get_config(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen)
{
    ASSERT((pObj != NULL) && (pBuff != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pModelInfo->f_get_config != NULL)
        {
            return  pStatus->pModelInfo->f_get_config(pObj, pBuff, ulBuffLen);
        }
    }

    return  MQX_ERROR;
}

uint_32      _gus_get_update_interval(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

_mqx_uint    _gus_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval)
{
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  MQX_OK;
}

static  _mqx_uint   _gus_get_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistic)
{
    ASSERT((pObj != NULL) && (pStatistic != NULL));
    
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistic, &pStatus->xCommon.xStatistics, sizeof(FTE_OBJECT_STATISTICS));
    
    return  MQX_OK;
}


#endif