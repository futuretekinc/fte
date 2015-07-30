#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"

#if 1 //FTE_IFCE_SUPPORTED

static  _mqx_uint   _ifce_init(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _ifce_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _ifce_stop(FTE_OBJECT_PTR pObj);
static void         _ifce_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static _mqx_uint    _ifce_set(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
static uint_32      _ifce_get_update_interval(FTE_OBJECT_PTR pObj);
static _mqx_uint    _ifce_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval);

static  FTE_OBJECT_ACTION _ifce_action = 
{
    .f_init         = _ifce_init,
    .f_run          = _ifce_run,
    .f_stop         = _ifce_stop,
    .f_set          = _ifce_set,
    .f_get_update_interval  = _ifce_get_update_interval,
    .f_set_update_interval  = _ifce_set_update_interval
};

_mqx_uint   fte_ifce_attach(FTE_OBJECT_PTR pObj)
{
    FTE_OBJECT_PTR      pParent = NULL;
    
    ASSERT(pObj != NULL);
    
    FTE_IFCE_CONFIG_PTR pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;
    FTE_IFCE_STATUS_PTR pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;
    
    pParent = FTE_OBJ_get(pConfig->nDevID);
    if (pParent == FALSE)
    {
        goto error;
    }
/*    
    if (pParent->pAction->f_attach_child == NULL)
    {
        goto error;
    }
 */ 
    
    switch(FTE_OBJ_CLASS(pObj))
    {
    case    FTE_OBJ_CLASS_TEMPERATURE: 
        pStatus->xCommon.pValue = FTE_VALUE_createTemperature();
        break;
        
    case    FTE_OBJ_CLASS_HUMIDITY: 
        pStatus->xCommon.pValue = FTE_VALUE_createHumidity();
        break;
        
    case    FTE_OBJ_CLASS_VOLTAGE:
        pStatus->xCommon.pValue = FTE_VALUE_createVoltage();
        break;
        
    case    FTE_OBJ_CLASS_CURRENT:
        pStatus->xCommon.pValue = FTE_VALUE_createCurrent();
        break;
        
    case    FTE_OBJ_CLASS_DI:
    case    FTE_OBJ_CLASS_DO:
        pStatus->xCommon.pValue = FTE_VALUE_createDIO();
        break;
        
    case    FTE_OBJ_CLASS_GAS:
        pStatus->xCommon.pValue = FTE_VALUE_createPPM();
        break;
        
    default:
        pStatus->xCommon.pValue = FTE_VALUE_createULONG();
    }

    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_ifce_action;
    
    pStatus->pParent = pParent;    
    /*
    if (pParent->pAction->f_attach_child(pParent, pConfig->nID) != MQX_OK)
    {
        goto error;
    }
    */
    
    _ifce_init(pObj);

    return  MQX_OK;
    
error:
    
    if (pStatus != NULL)
    {
        if ((pStatus->pParent != NULL) && (pStatus->pParent->pAction->f_detach_child != NULL))
        {
            pStatus->pParent->pAction->f_detach_child(pParent, pConfig->xCommon.nID);
        }
        FTE_MEM_free(pStatus);
        pObj->pStatus = NULL;
    }
    
    return  MQX_ERROR;
    
}

_mqx_uint fte_ifce_detach(FTE_OBJECT_PTR pObj)
{
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }

   if (pObj->pStatus != NULL)
    {
        FTE_IFCE_STATUS_PTR pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;
        FTE_IFCE_CONFIG_PTR  pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;

        if ((pStatus->pParent != NULL) && (pStatus->pParent->pAction->f_detach_child != NULL))
        {
            pStatus->pParent->pAction->f_detach_child(pStatus->pParent, pConfig->xCommon.nID);
        }
        FTE_MEM_free(pObj->pStatus);        
    }
    
     pObj->pAction = NULL;
     pObj->pStatus = NULL;
    
    return  MQX_OK;
}

_mqx_uint   _ifce_init(FTE_OBJECT_PTR pObj)
{
    assert(pObj != NULL);
    
    return  MQX_OK;
}

_mqx_uint   _ifce_run(FTE_OBJECT_PTR pObj)
{
    assert(pObj != NULL);

    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;
    FTE_IFCE_CONFIG_PTR  pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;
    FTE_IFCE_STATUS_PTR  pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_sec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_ifce_restart_convert, pObj, TIMER_KERNEL_TIME_MODE, &xTicks, &xDTicks);
    
    return  MQX_OK;
}
 
_mqx_uint   _ifce_stop(FTE_OBJECT_PTR pObj)
{ 
    return  MQX_OK;
}

static void _ifce_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    FTE_IFCE_CONFIG_PTR  pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;
    FTE_IFCE_STATUS_PTR  pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;
        
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        FTE_VALUE   xValue;
        
        FTE_VALUE_copy(&xValue, pStatus->xCommon.pValue);
        
        FTE_OBJ_getValueAt(pStatus->pParent, pConfig->nRegID, pStatus->xCommon.pValue);        
        
        if (!FTE_VALUE_equal(&xValue, pStatus->xCommon.pValue))
        {
            FTE_OBJ_wasChanged(pObj);
        }
        else
        {
            FTE_OBJ_wasUpdated(pObj);
        }
    }
    else
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

_mqx_uint    _ifce_set(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue)
{
    assert(pObj != NULL);
    
    FTE_IFCE_STATUS_PTR  pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;
    FTE_IFCE_CONFIG_PTR  pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;
   
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pParent->pAction->f_set_multi != NULL)
        {
            if (pStatus->pParent->pAction->f_set_multi(pStatus->pParent, pConfig->nRegID, pValue) == MQX_OK)
            {
                FTE_VALUE   xValue;
                
                FTE_VALUE_copy(&xValue, pStatus->xCommon.pValue);
                
                FTE_OBJ_getValueAt(pStatus->pParent, pConfig->nRegID, pStatus->xCommon.pValue);        
                
                if ( pStatus->xCommon.pValue->bChanged || !FTE_VALUE_equal(&xValue, pStatus->xCommon.pValue))
                {
                    FTE_OBJ_wasChanged(pObj);
                }
                else
                {
                    FTE_OBJ_wasUpdated(pObj);
                }
                
                return  MQX_OK;
            }
            else
            {
                DEBUG("f_set_multi failed\n");
            }
        }
    }

    return  MQX_ERROR;
}

uint_32      _ifce_get_update_interval(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    return  ((FTE_IFCE_CONFIG_PTR)pObj->pConfig)->nInterval;
}

_mqx_uint    _ifce_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval)
{
    ASSERT(pObj != NULL);

    ((FTE_IFCE_CONFIG_PTR)pObj->pConfig)->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
 
    return  MQX_OK;
}


#endif
