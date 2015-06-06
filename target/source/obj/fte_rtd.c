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
    .f_get          = _rtd_get,
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
    
    pADC = fte_ad7785_get(pConfig->nDevID);
    if (pADC == FALSE)
    {
        goto error;
    }
    
    if (fte_ad7785_attach(pADC, pConfig->nID) != MQX_OK)
    {
        goto error;
    }
    
    pStatus->pADC = pADC;
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_rtd_action;
    
    if (_rtd_init(pObj) != MQX_OK)
    {
        fte_ad7785_detach(pADC);
        pStatus->pADC = NULL;
        
        goto error;
    }

    FTE_LIST_pushBack(&_xObjList, pObj);

    return  MQX_OK;
    
error:
    
    if (pStatus != NULL)
    {
        fte_ad7785_detach(pADC);
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

uint_32 FTE_RTD_printValue(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen)
{
    ASSERT((pObj != NULL) && FTE_OBJ_CLASS(pObj) == FTE_OBJ_CLASS_TEMPERATURE));

    if (((FTE_RTD_STATUS_PTR)pObj->pStatus)->nValue >= 0)
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

_mqx_uint   _rtd_init(FTE_OBJECT_PTR pObj)
{
    assert(pObj != NULL);
    
    uint_32 setValue, getValue;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    fte_ad7785_get_config(pStatus->pADC, &getValue);
    setValue = getValue | (FTE_AD7785_CONFIG_UNIPOLAR_MODE << FTE_AD7785_CONFIG_POL_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_GAIN_MASK) | (FTE_AD7785_CONFIG_GAIN_32 << FTE_AD7785_CONFIG_GAIN_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_REF_MASK) | (FTE_AD7785_CONFIG_REF_EXTR << FTE_AD7785_CONFIG_REF_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_BUF_MASK) | (FTE_AD7785_CONFIG_BUF_MODE << FTE_AD7785_CONFIG_BUF_SHIFT);
    setValue = (setValue & ~FTE_AD7785_CONFIG_CHANNEL_MASK) | (FTE_AD7785_CONFIG_CHANNEL_2 << FTE_AD7785_CONFIG_CHANNEL_SHIFT);
    fte_ad7785_set_config(pStatus->pADC, setValue);
    fte_ad7785_get_config(pStatus->pADC, &getValue);
    if (setValue != getValue)
    {
        goto error;
    }

    fte_ad7785_set_iout(pStatus->pADC, FTE_AD7785_IOUT_DIR_DIRECT);
    fte_ad7785_get_iout(pStatus->pADC, &getValue);
    if (getValue != FTE_AD7785_IOUT_DIR_DIRECT)
    {
        goto error;
    }
    fte_ad7785_set_current(pStatus->pADC, FTE_AD7785_IOUT_CURRENT_210UA);    
    fte_ad7785_get_current(pStatus->pADC, &getValue);    
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
    _time_get_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_rtd_restart_convert, pObj, TIMER_KERNEL_TIME_MODE, &xTicks, &xDTicks);
    fte_ad7785_set_op_mode(pStatus->pADC, FTE_AD7785_OP_MODE_SINGLE);

    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, 500);    
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_rtd_done, pObj, TIMER_KERNEL_TIME_MODE, &xDTicks);

    
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
    
    fte_ad7785_set_op_mode(pStatus->pADC, FTE_AD7785_OP_MODE_PWR_DOWN);
    
    return  MQX_OK;
    
}

static void _rtd_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    uint_32             data = 0;
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    if (fte_ad7785_get_raw_data(pStatus->pADC, &data) == MQX_OK)
    {
        uint_32 index, value;
        int_32  temp;
        
        value = (uint_32)((float)(data >> 4) / (float)(1 << FTE_AD7785_CONFIG_GAIN_32) / 0x100000 * 4990 * 100);
            
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
            _time_get (&pStatus->xTimeStamp);
            pStatus->nValue = temp;
            pStatus->xFlags = FTE_FLAG_SET(pStatus->xFlags, FTE_OBJ_STATUS_FLAG_VALID);
            
            if (FTE_FLAG_IS_SET(pObj->pConfig->xFlags, FTE_OBJ_CONFIG_FLAG_TRAP))
            {
                FTE_LOG_add(&pStatus->xTimeStamp, pObj->pConfig->nID, pStatus->nValue);
            }
        }
        else
        {
            pStatus->xFlags = FTE_FLAG_CLR(pStatus->xFlags, FTE_OBJ_STATUS_FLAG_VALID);
            _rtd_init(pObj);
        }
    }
    else
    {
        pStatus->xFlags = FTE_FLAG_CLR(pStatus->xFlags, FTE_OBJ_STATUS_FLAG_VALID);
    }
}

static void _rtd_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_RTD_STATUS_PTR  pStatus = (FTE_RTD_STATUS_PTR)pObj->pStatus;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        fte_ad7785_set_op_mode(pStatus->pADC, FTE_AD7785_OP_MODE_SINGLE);
        
        _time_init_ticks(&xDTicks, 0);
        _time_add_msec_to_ticks(&xDTicks, 500);    
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_rtd_done, pObj, TIMER_KERNEL_TIME_MODE, &xDTicks);
    }
    else
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

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
