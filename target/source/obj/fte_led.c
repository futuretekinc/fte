#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "sys/fte_sys_timer.h"

static FTE_RET  _FTE_LED_init(FTE_OBJECT_PTR pObj);
static FTE_RET  _FTE_LED_run(FTE_OBJECT_PTR pObj);
static FTE_RET  _FTE_LED_stop(FTE_OBJECT_PTR pObj);
static FTE_RET  _FTE_LED_setValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
static void     _FTE_LED_timerDone(_timer_id id, pointer pData, MQX_TICK_STRUCT_PTR pTick);

static  
FTE_LIST            _xObjList = {0, NULL, NULL};

static  
FTE_OBJECT_ACTION   _xAction = 
{ 
    .fInit  = _FTE_LED_init,
    .fRun   = _FTE_LED_run,
    .fStop  = _FTE_LED_stop,
    .fSet   = _FTE_LED_setValue
};

static  
FTE_UINT32  _hTimer = 0;

FTE_RET FTE_LED_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    FTE_LED_CONFIG_PTR   pConfig = (FTE_LED_CONFIG_PTR)pObj->pConfig;
    FTE_LED_STATUS_PTR   pStatus = (FTE_LED_STATUS_PTR)pObj->pStatus;
    
    ASSERT(pObj != NULL);    
        
    pStatus->xCommon.pValue = FTE_VALUE_createDIO();
    if (pStatus->xCommon.pValue == NULL)
    {
        goto error;
    }
    
    pStatus->pGPIO = FTE_GPIO_get(pConfig->nGPIO);
    if (pStatus->pGPIO == NULL)
    {
        goto error;
    }
    
    if (FTE_GPIO_attach(pStatus->pGPIO, pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }

    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_xAction;
    pObj->pStatus = (FTE_OBJECT_STATUS_PTR)pStatus;

    if (_FTE_LED_init(pObj) != MQX_OK)
    {
        FTE_GPIO_detach(pStatus->pGPIO);
        goto error;
    }
        
    FTE_LIST_pushBack(&_xObjList, pObj);

    return  MQX_OK;
error:

    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.pValue = NULL;
    }
    
    return  MQX_ERROR;
}

FTE_RET FTE_LED_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    if (!FTE_LIST_isExist(&_xObjList, pObj))
    {
        goto error;
    }

    FTE_LIST_remove(&_xObjList, pObj);

    FTE_MEM_free(pObj->pStatus);
    pObj->pAction = NULL;
    pObj->pStatus = NULL;
     
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

pointer     FTE_LED_get
(
    FTE_OBJECT_ID   nID
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            if (pObj->pConfig->xCommon.nID == nID)
            {
                return  pObj;
            }
        }
    }
    
    return  NULL;
}

/******************************************************************************/
FTE_UINT32     FTE_LED_count(void)
{
    return  FTE_LIST_count(&_xObjList);
}

FTE_RET   FTE_LED_setValue
(
    FTE_OBJECT_ID   nID, 
    FTE_BOOL        bState
)
{
    FTE_VALUE       xValue;
    FTE_OBJECT_PTR pObj = FTE_LED_get(nID);
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }

    FTE_VALUE_initDIO(&xValue, bState);
    
    return  _FTE_LED_setValue(pObj, &xValue);
}

FTE_BOOL     FTE_LED_isActive
(
    FTE_OBJECT_ID   nID
)
{
    FTE_BOOL bValue;
    FTE_OBJECT_PTR pObj = FTE_LED_get(nID);
    if (pObj == NULL)
    {
        return  FALSE;
    }

    FTE_VALUE_getDIO(((FTE_LED_STATUS_PTR)pObj->pStatus)->xCommon.pValue, &bValue);
    
    return  bValue != FTE_LED_STATE_OFF;    
}

/******************************************************************************/
FTE_RET   _FTE_LED_init
(
    FTE_OBJECT_PTR  pObj
)
{
    assert(pObj != NULL);

    FTE_LED_CONFIG_PTR   pConfig = (FTE_LED_CONFIG_PTR)pObj->pConfig;    
    FTE_VALUE           xValue;
    
    FTE_VALUE_initLED(&xValue, pConfig->nInit);
    _FTE_LED_setValue(pObj, &xValue);
    
    return  MQX_OK;
}

FTE_RET   _FTE_LED_run
(
    FTE_OBJECT_PTR  pObj
)
{
    return  MQX_OK;
}

FTE_RET   _FTE_LED_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    return  MQX_OK;
}

FTE_RET _FTE_LED_setValue
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VALUE_PTR   pValue
)
{
    assert(pObj != NULL);
    
    FTE_LED_STATUS_PTR   pStatus = (FTE_LED_STATUS_PTR)pObj->pStatus;
    
    switch(pValue->xData.xState)
    {
    case    FTE_LED_STATE_OFF:
        {
            if (FTE_GPIO_setValue(pStatus->pGPIO,  FALSE) != MQX_OK)
            {
                goto error;
            }

            if (_hTimer != 0)
            {
                _timer_cancel(_hTimer);
                _hTimer = 0;
            }
        }
        break;
        
    case    FTE_LED_STATE_ON:
        {
            if (FTE_GPIO_setValue(pStatus->pGPIO,  TRUE) != MQX_OK)
            {
                goto error;
            }

            if (_hTimer != 0)
            {
                _timer_cancel(_hTimer);
                _hTimer = 0;
            }
        }
        break; 

    case    FTE_LED_STATE_BLINK:
        {
            if (_hTimer == 0)
            {
                if (pObj != NULL)
                {
                    FTE_GPIO_setValue(pStatus->pGPIO,  TRUE);
                }
                
                _hTimer = FTE_OBJ_runLoop(pObj, _FTE_LED_timerDone, 500); 
            }
        
        }
        break;
        
    default:
        goto error;
    }
        
    FTE_VALUE_setULONG(pStatus->xCommon.pValue, pValue->xData.xState);
    
    return   MQX_OK;
error:
    pStatus->xCommon.xFlags&= ~FTE_OBJ_STATUS_FLAG_VALID;
    
    return   MQX_ERROR;
}

void _FTE_LED_timerDone
(
    _timer_id   id, 
    pointer     pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            FTE_UINT32 ulValue;
            
            FTE_VALUE_getULONG(((FTE_LED_STATUS_PTR)pObj->pStatus)->xCommon.pValue, &ulValue);
            if (ulValue == FTE_LED_STATE_BLINK)
            {
                FTE_BOOL bValue;
                
                FTE_GPIO_getValue(((FTE_LED_STATUS_PTR)pObj->pStatus)->pGPIO, &bValue);
                FTE_GPIO_setValue(((FTE_LED_STATUS_PTR)pObj->pStatus)->pGPIO,  !bValue);
            }
        }
    }
}

