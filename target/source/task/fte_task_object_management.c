#include "fte_target.h"
#include "fte_debug.h"
#include "fte_event.h"
#include "fte_config.h"
/*TASK*-----------------------------------------------------------------
*
* Function Name  : FTE_TASK_objectManagement
* Returned Value : void 
* Comments       : 
*
*END------------------------------------------------------------------*/
void FTE_TASK_objectManagement
(
    FTE_UINT32  params
)
{
    FTE_RET             xRet;
    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xNextTicks;            
    FTE_BOOL            bOverflow;
    FTE_UINT32          ulLoopCount = 0;
    FTE_TASK_ID         xTaskID = _task_get_id();
        
#if FTE_1WIRE_SUPPORTED
    FTE_UINT32 ulNewObject;
   
    xRet = FTE_OBJ_1WIRE_discovery(TRUE, &ulNewObject);
    if (xRet != FTE_RET_OK)
    {
        TRACE(DEBUG_OBJECT, "1Wire discovery not supported!\n");
    }
#endif
   _time_get_elapsed_ticks(&xNextTicks);
    
    while(TRUE)
    {
        int_32  nDiffTime; 
        FTE_UINT32 ulObjCount = FTE_OBJ_count(0, 0, FALSE);
        
        FTE_DI_update();
        
        for(FTE_UINT32 i = 0 ; i < ulObjCount ; i++)
        {
            FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(0, 0, i, FALSE);
            
            if (FTE_OBJ_STATE_isSet(pObj, FTE_OBJ_STATUS_FLAG_CHANGED))
            {
                FTE_LIST_ITERATOR xIter;
               
               if (FTE_LIST_ITER_init(&pObj->xEventList, &xIter) == MQX_OK)
               {
                    FTE_EVENT_PTR   pEvent;
                    
                    while((pEvent = FTE_LIST_ITER_getNext(&xIter)) != NULL)
                    {
                        FTE_EVENT_check(pEvent, pObj);
                    }
               }
               
               FTE_OBJ_STATE_clear(pObj, FTE_OBJ_STATUS_FLAG_CHANGED);
            }
        }
               
        _time_get_elapsed_ticks(&xTicks);        
        
        if (++ulLoopCount > (FTE_OBJ_LIVE_CHECK_INTERVAL * 1000) / FTE_OBJ_EVENT_CHECK_INTERVAL)
        {
            
           for(FTE_UINT32 i = 0 ; i < ulObjCount ; i++)
            {
                FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(0, 0, i, FALSE);
                
                if (FTE_OBJ_IS_ENABLED(pObj) && (pObj->pStatus->xStartTicks.HW_TICKS != 0))
                {
                    bOverflow = FALSE;        
                    nDiffTime = _time_diff_seconds(&xTicks, &pObj->pStatus->xStartTicks, &bOverflow);
                    if (nDiffTime > FTE_OBJ_LIVE_CHECK_INTERVAL)
                    {
                        FTE_SYS_setUnstable();
                    }                    
                }
            }
            ulLoopCount = 0;
        }
    
        do 
        {
            _time_add_msec_to_ticks(&xNextTicks, FTE_OBJ_EVENT_CHECK_INTERVAL);
            bOverflow = FALSE;        
            nDiffTime = _time_diff_milliseconds(&xNextTicks, &xTicks, &bOverflow);
            if (bOverflow)
            {
                nDiffTime = FTE_OBJ_EVENT_CHECK_INTERVAL;
            }
        } while(nDiffTime <= 0);
        
        _time_delay(nDiffTime);        
    }      
}
