#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"

#if 1 //FTE_IFCE_SUPPORTED

static FTE_RET  FTE_IFCE_init(FTE_OBJECT_PTR pObj);
static FTE_RET  FTE_IFCE_run(FTE_OBJECT_PTR pObj);
static FTE_RET  FTE_IFCE_stop(FTE_OBJECT_PTR pObj);
static void     FTE_IFCE_restartConvert(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);
static FTE_RET  FTE_IFCE_set(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
static FTE_RET  FTE_IFCE_getInterval(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pulInterval);
static FTE_RET  FTE_IFCE_setInterval(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);
static FTE_RET  FTE_IFCE_setConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff);
static FTE_RET  FTE_IFCE_getConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);

static  
FTE_OBJECT_ACTION FTE_IFCE_action = 
{
    .fInit  = FTE_IFCE_init,
    .fRun   = FTE_IFCE_run,
    .fStop  = FTE_IFCE_stop,
    .fSet   = FTE_IFCE_set,
    .fGetInterval   = FTE_IFCE_getInterval,
    .fSetInterval   = FTE_IFCE_setInterval,
    .fGetConfig     = FTE_IFCE_getConfig,   \
    .fSetConfig     = FTE_IFCE_setConfig,   \

};

FTE_RET   FTE_IFCE_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    FTE_RET xRet;
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
        
    case    FTE_OBJ_CLASS_DISCRETE:
        pStatus->xCommon.pValue = FTE_VALUE_createHex32();
        break;
        
    default:
        pStatus->xCommon.pValue = FTE_VALUE_createULONG();
    }

    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&FTE_IFCE_action;
    
    pStatus->pParent = pParent;

    xRet = FTE_OBJ_attachChild(pParent, pConfig->xCommon.nID);
    if (xRet == FTE_RET_OK)
    {
        FTE_IFCE_init(pObj);
        return  FTE_RET_OK;
    }
    
error:
    
    if (pStatus != NULL)
    {
        FTE_MEM_free(pStatus);
        pObj->pStatus = NULL;
    }
    
    return  FTE_RET_ERROR;
    
}

FTE_RET FTE_IFCE_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }

   if (pObj->pStatus != NULL)
    {
        FTE_IFCE_STATUS_PTR pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;
        FTE_IFCE_CONFIG_PTR  pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;

        if (pStatus->pParent != NULL) 
        {
            FTE_OBJ_detachChild(pStatus->pParent, pConfig->xCommon.nID);
        }
        FTE_MEM_free(pObj->pStatus);        
    }
    
     pObj->pAction = NULL;
     pObj->pStatus = NULL;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_IFCE_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    if (FTE_OBJ_IS_SET(pObj, FTE_OBJ_CONFIG_FLAG_ENABLE) && FTE_OBJ_IS_RESET(pObj, FTE_OBJ_CONFIG_FLAG_SYNC))
    {
        return  FTE_IFCE_run(pObj);
    }
        
    return  FTE_RET_OK;
}

FTE_RET   FTE_IFCE_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

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
    _time_add_msec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_msec_to_ticks(&xTicks, 1000);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(FTE_IFCE_restartConvert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);
    
    return  FTE_RET_OK;
}
 
FTE_RET   FTE_IFCE_stop
(
    FTE_OBJECT_PTR  pObj
)
{ 
    ASSERT(pObj != NULL);
    
    FTE_IFCE_STATUS_PTR  pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;

    _timer_cancel(pStatus->hRepeatTimer);
    pStatus->hRepeatTimer = 0;

    return  FTE_RET_OK;
}

void FTE_IFCE_restartConvert
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
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

FTE_RET    FTE_IFCE_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
    
    FTE_IFCE_STATUS_PTR  pStatus = (FTE_IFCE_STATUS_PTR)pObj->pStatus;
    FTE_IFCE_CONFIG_PTR  pConfig = (FTE_IFCE_CONFIG_PTR)pObj->pConfig;
   
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (FTE_OBJ_setValueAt(pStatus->pParent, pConfig->nRegID, pValue) == FTE_RET_OK)
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
            
            return  FTE_RET_OK;
        }
    }

    return  FTE_RET_ERROR;
}

FTE_RET FTE_IFCE_getInterval
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32_PTR  pulInterval
)
{
    ASSERT((pObj != NULL) && (pulInterval != NULL));
    
    *pulInterval = ((FTE_IFCE_CONFIG_PTR)pObj->pConfig)->nInterval;
    
    return  FTE_RET_OK;
}

FTE_RET    FTE_IFCE_setInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    ASSERT(pObj != NULL);

    ((FTE_IFCE_CONFIG_PTR)pObj->pConfig)->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
 
    return  FTE_RET_OK;
}

FTE_RET   FTE_IFCE_setConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR        pBuff
)
{
    ASSERT(pObj != NULL && pBuff != NULL);
    FTE_OBJECT_PTR  pParent = ((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent;
    
    if ((pParent == NULL) || (pParent->pAction->fSetChildConfig == NULL))
    {
        return  FTE_RET_ERROR;
    }

    return  pParent->pAction->fSetChildConfig(pParent, pObj, pBuff);
}

FTE_RET   FTE_IFCE_getConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    ASSERT(pObj != NULL && pBuff != NULL);
    FTE_OBJECT_PTR  pParent = ((FTE_IFCE_STATUS_PTR)pObj->pStatus)->pParent;
    
    if ((pParent == NULL) || (pParent->pAction->fGetChildConfig == NULL))
    {
        return  FTE_RET_ERROR;
    }

    return  pParent->pAction->fGetChildConfig(pParent, pObj, pBuff, ulBuffLen);
}

#endif
