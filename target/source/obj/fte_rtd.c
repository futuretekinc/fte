#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"

#if FTE_RTD_SUPPORTED

static  _mqx_uint   _rtd_init(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _rtd_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _rtd_stop(FTE_OBJECT_PTR pObj);
static  void        _rtd_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static void         _rtd_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static _mqx_uint    _rtd_get(FTE_OBJECT_PTR pObj, uint_32 *value, TIME_STRUCT *time_stamp);
static uint_32      _rtd_get_update_interval(FTE_OBJECT_PTR pObj);
static _mqx_uint    _rtd_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval);

static  FTE_OBJECT_ACTION _rtd_action =  
{
    .f_init         = _rtd_init,
    .f_run          = _rtd_run,
    .f_stop         = _rtd_stop,
    //.f_get          = _rtd_get,
    .f_set          = NULL,
    .f_get_update_interval = _rtd_get_update_interval,
    .f_set_update_interval = _rtd_set_update_interval
};

static const int_32 PT100Table[] = 
{   
    -24688, -22295, -19886, -17461, -15020, -12561, -10085, -7591, -5080, -2549,
    0, 2569, 5157, 7766, 10395, 13046, 15718, 18413, 21131, 23872, 26637 
};

static FTE_LIST _xObjList;

_mqx_uint   fte_rtd_attach(FTE_OBJECT_PTR pObj)
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
    
    if (FTE_AD7785_attach(pADC, pConfig->xCommon.nID) != MQX_OK)
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
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_rtd_action;
    
    if (_rtd_init(pObj) != MQX_OK)
    {
        FTE_AD7785_detach(pADC);
        pStatus->pADC = NULL;
        
        goto error;
    }

    FTE_LIST_pushBack(&_xObjList, pObj);

    return  MQX_OK;
    
error:
    
    if (pStatus != NULL)
    {
        FTE_AD7785_detach(pADC);
        pObj->pStatus = NULL;
    }
    
    return  MQX_ERROR;
    
}

_mqx_uint fte_rtd_detach(FTE_OBJECT_PTR pObj)
{
    if (!FTE_LIST_isExist(&_xObjList, pObj))
    {
        goto error;
    }

    FTE_LIST_remove(&_xObjList, pObj);
    pObj->pAction = NULL;
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}
/*
uint_32 FTE_RTD_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen)
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
        int_32 nValue = 0 - ((FTE_RTD_STATUS_PTR)pObj->pStatus)->nValue;
        
        return  snprintf(pBuff, nLen, "-%d.%02d", nValue / 100, nValue % 100);
    }
}
*/
_mqx_uint   _rtd_init(FTE_OBJECT_PTR pObj)
{
    assert(pObj != NULL);
    
    uint_32 setValue, getValue;
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
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

_mqx_uint   _rtd_run(FTE_OBJECT_PTR pObj)
{
    assert(pObj != NULL);

    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;
    FTE_RTD_CONFIG_PTR  pConfig = (FTE_RTD_CONFIG_PTR)pObj->pConfig;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    _rtd_init(pObj);
    
    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_sec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_rtd_restart_convert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);
    FTE_AD7785_setOPMode(pStatus->pADC, FTE_AD7785_OP_MODE_SINGLE);

    _time_init_ticks(&xDTicks, _time_get_ticks_per_sec());
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_rtd_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);

    
    return  MQX_OK;
}
 
_mqx_uint   _rtd_stop(FTE_OBJECT_PTR pObj)
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
    
    return  MQX_OK;
    
}

static void _rtd_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    uint_32             data = 0;
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    if (FTE_AD7785_getRawData(pStatus->pADC, &data) == MQX_OK)
    {
        uint_32 index, value;
        int_32  temp;
        
        value = (uint_32)((float)(data) / (float)(1 << FTE_AD7785_CONFIG_GAIN_32) / 0x100000 * 4990 * 100);
            
        index = value / 1000;
    
        if (index < 20)
        {
            temp = PT100Table[index] + (PT100Table[index+1] - PT100Table[index]) * (value - index * 1000) / 1000;        
        }
        else
        {
            temp = PT100Table[20];
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
            _rtd_init(pObj);
        }
    }
    else
    {
            FTE_VALUE_setValid(pStatus->xCommon.pValue, FALSE);
            FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
    }
}

static void _rtd_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
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
_mqx_uint    _rtd_get(FTE_OBJECT_PTR pObj, uint_32 *value, TIME_STRUCT *time_stamp)
{
    assert(pObj != NULL && value != NULL);
    
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        *value = pStatus->nValue;
        if (time_stamp != NULL)
        {
            *time_stamp = pStatus->xTimeStamp;
        }
        
        return  MQX_OK;
    }

    return  MQX_ERROR;
}
#endif
uint_32      _rtd_get_update_interval(FTE_OBJECT_PTR pObj)
{
    FTE_RTD_CONFIG_PTR  pConfig = (FTE_RTD_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

_mqx_uint    _rtd_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval)
{
    FTE_RTD_CONFIG_PTR  pConfig = (FTE_RTD_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);

    
    return  MQX_OK;
}

#endif
