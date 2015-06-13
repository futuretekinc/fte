#include "fte_target.h"
#include "fte_event.h"
#include "fte_list.h" 
#include "fte_config.h"
#include "fte_time.h"
#include "fte_log.h"

static _mqx_uint    FTE_EVENT_proc(FTE_EVENT_PTR pEvent, TIME_STRUCT_PTR pTime);

static FTE_LIST         _eventList =
{
    .nCount = 0,
    .pHead  = NULL
};

static FTE_SYS_LOCK_PTR    pListLockKey = NULL;

_mqx_uint   FTE_EVENT_init(void)
{
    FTE_LIST_init(&_eventList);
    fte_sys_lock_create(&pListLockKey);

    _task_create(0, FTE_TASK_EVENT, 0);
    
    return  MQX_OK;
}

_mqx_uint   FTE_EVENT_final(void)
{
    fte_sys_lock_destroy(pListLockKey);

    return  MQX_OK;
}

_mqx_uint   FTE_EVENT_create(FTE_CFG_EVENT_PTR pConfig, FTE_EVENT_PTR _PTR_ ppEvent)
{
    FTE_EVENT_PTR   pEvent;
    
    pEvent = (FTE_EVENT_PTR)FTE_MEM_allocZero(sizeof(FTE_EVENT));
    if (pEvent == NULL)
    {
        return  MQX_ERROR;
    }
     
    pEvent->pConfig         = pConfig;
    pEvent->xState.bOccurred= FALSE;

    FTE_LIST_init(&pEvent->xState.xObjectList);

    fte_sys_lock_enable(pListLockKey);
    
    FTE_LIST_pushBack(&_eventList, pEvent);

    fte_sys_lock_disable(pListLockKey);
    
    *ppEvent = pEvent;
    
    return  MQX_OK;
}

_mqx_uint   FTE_EVENT_destroy(FTE_EVENT_PTR pEvent)
{
    fte_sys_lock_enable(pListLockKey);
    
    FTE_LIST_remove(&_eventList, pEvent);

    fte_sys_lock_disable(pListLockKey);

    FTE_LIST_final(&pEvent->xState.xObjectList);
    
    FTE_MEM_free(pEvent);
    
    return  MQX_OK;
}

_mqx_uint   FTE_EVENT_count(uint_32_ptr pulCount)
{
    *pulCount = FTE_LIST_count(&_eventList);
    
    return  MQX_OK;
}

FTE_EVENT_PTR   FTE_EVENT_getAt(uint_32 ulIndex)
{
    return  (FTE_EVENT_PTR)FTE_LIST_getAt(&_eventList, ulIndex);
}

_mqx_uint   FTE_EVENT_attachObject(FTE_EVENT_PTR pEvent, struct _FTE_OBJECT_STRUCT _PTR_ pObj)
{
    FTE_LIST_pushBack(&pEvent->xState.xObjectList, pObj);
    
    return  MQX_OK;
}

_mqx_uint   FTE_EVENT_detachObject(FTE_EVENT_PTR pEvent, struct _FTE_OBJECT_STRUCT _PTR_ pObj)
{
    FTE_LIST_remove(&pEvent->xState.xObjectList, pObj);
    
    return  MQX_OK;
}

_mqx_uint   FTE_EVENT_isSatisfied(FTE_EVENT_PTR pEvent, int_32 nValue, boolean *bResult)
{
    switch(pEvent->pConfig->xCondition)
    {
    case    FTE_EVENT_CONDITION_ABOVE:
        {
            *bResult =  (pEvent->pConfig->xParams.xLimit.nValue >= nValue);
        }
        break;
        
    case    FTE_EVENT_CONDITION_BELOW:
        {
            *bResult =  (pEvent->pConfig->xParams.xLimit.nValue <= nValue);
        }
        break;
        
    case    FTE_EVENT_CONDITION_INSIDE:
        {
            *bResult =  ((pEvent->pConfig->xParams.xRange.nUpper >= nValue) && (pEvent->pConfig->xParams.xRange.nLower <= nValue));
        }
        break;
        
    case    FTE_EVENT_CONDITION_OUTSIDE:
        {
            *bResult =  ((pEvent->pConfig->xParams.xRange.nUpper < nValue) && (pEvent->pConfig->xParams.xRange.nLower > nValue));
        }
        break;
        
    default:
        return  MQX_INVALID_PARAMETER;
    }
    
    return  MQX_OK;
}


_mqx_uint    FTE_EVENT_check(FTE_EVENT_PTR pEvent, struct _FTE_OBJECT_STRUCT _PTR_ pObj)
{
    FTE_VALUE           xValue;
    boolean             bState = FALSE;
    TIME_STRUCT         xCurrentTime;
    
    if (FTE_FLAG_IS_CLR(pEvent->pConfig->xType, FTE_EVENT_TYPE_ENABLE) || 
        (pEvent->pConfig->xCondition == FTE_EVENT_CONDITION_INTERVAL) ||
        (pEvent->pConfig->xCondition == FTE_EVENT_CONDITION_TIME))
    {
        return  MQX_OK;
    }

    if (pObj == NULL)
    {        
        if (pEvent->xState.pObj == NULL)
        {
            pObj = FTE_OBJ_get(pEvent->pConfig->ulEPID);
            if (pObj == NULL)
            {
                TRACE(DEBUG_EVENT, "%s : Object not found[ulEPID = %08lx]\n", __func__, pEvent->pConfig->ulEPID);
                return  MQX_INVALID_HANDLE;
            }
        }
        else
        {
            pObj = pEvent->xState.pObj;
        }
    }
    
    if (FTE_OBJ_getValue(pObj, &xValue) != MQX_OK)
    {
        return   MQX_ERROR;
    }

    _time_get (&xCurrentTime);
    
    if (pEvent->xState.bOccurred && (FTE_TIME_diff(&xCurrentTime, &pEvent->xState.xTimeStamp) <= pEvent->pConfig->ulHoldTime))
    {
        return  MQX_OK;
    }    
    
    switch(pEvent->pConfig->xCondition)
    {
    case    FTE_EVENT_CONDITION_ABOVE:
        if (pEvent->xState.bOccurred)
        {
            if (pEvent->pConfig->xParams.xLimit.nValue - pEvent->pConfig->xParams.xLimit.ulThreshold <= xValue.xData.nValue)
            {
                bState = TRUE;
            }        
        }
        else
        {            
            if (pEvent->pConfig->xParams.xLimit.nValue <= xValue.xData.nValue)
            {
                bState = TRUE;

            }
        }

        break;
        
    case    FTE_EVENT_CONDITION_BELOW:
        if (pEvent->xState.bOccurred)
        {
            if (pEvent->pConfig->xParams.xLimit.nValue + pEvent->pConfig->xParams.xLimit.ulThreshold >= xValue.xData.nValue)
            {
                bState = TRUE;
            }        
        }
        else
        {            
            if (pEvent->pConfig->xParams.xLimit.nValue >= xValue.xData.nValue)
            {
                bState = TRUE;

            }
        }
        break;
        
    case    FTE_EVENT_CONDITION_INSIDE:
        if (pEvent->xState.bOccurred)
        {
            if ((pEvent->pConfig->xParams.xRange.nUpper + pEvent->pConfig->xParams.xRange.ulThreshold >= xValue.xData.nValue) &&
                (pEvent->pConfig->xParams.xRange.nLower - pEvent->pConfig->xParams.xRange.ulThreshold <= xValue.xData.nValue))
            {
                bState = TRUE;
            }
        }
        else
        {
            if ((pEvent->pConfig->xParams.xRange.nUpper >= xValue.xData.nValue) &&
                (pEvent->pConfig->xParams.xRange.nLower <= xValue.xData.nValue))
            {
                bState = TRUE;
            }
        }
        break;
        
    case    FTE_EVENT_CONDITION_OUTSIDE:
        if (pEvent->xState.bOccurred)
        {
            if ((pEvent->pConfig->xParams.xRange.nUpper - pEvent->pConfig->xParams.xRange.ulThreshold < xValue.xData.nValue) ||
                (pEvent->pConfig->xParams.xRange.nLower + pEvent->pConfig->xParams.xRange.ulThreshold > xValue.xData.nValue))
            {
                bState = TRUE;
            }
        }
        else
        {            
            if ((pEvent->pConfig->xParams.xRange.nUpper < xValue.xData.nValue) ||
                (pEvent->pConfig->xParams.xRange.nLower > xValue.xData.nValue))
            {
                bState = TRUE;
            }
        }
        break;
        
    case    FTE_EVENT_CONDITION_INTERVAL:
        {
            TIME_STRUCT xObjTime;
            
            FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xObjTime);
            if (FTE_TIME_diff(&xObjTime, &pEvent->xState.xTimeStamp) >= pEvent->pConfig->xParams.ulInterval)
            {
                pEvent->xState.xTimeStamp   = xObjTime;
                pEvent->xState.bChanged     = TRUE;
                pEvent->xState.bOccurred    = TRUE;
            }
            
            return  MQX_OK;
        }
        break;
        
    case    FTE_EVENT_CONDITION_CHANGED:
        {
            if (FTE_OBJ_STATE_isSet(pObj, FTE_OBJ_STATUS_FLAG_CHANGED))
            {
                FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &pEvent->xState.xTimeStamp);
                pEvent->xState.bChanged     = TRUE;
                pEvent->xState.bOccurred    = TRUE;
                
                FTE_OBJ_STATE_clear(pObj, FTE_OBJ_STATUS_FLAG_CHANGED);
            }                
                
            return  TRUE;
        }

    default:
        { 
            return  MQX_INVALID_PARAMETER;
        }
    }    
    
    if (pEvent->pConfig->ulDelayTime != 0)
    {
        TIME_STRUCT xObjTime;
        
        pEvent->xState.bStateChanged = (pEvent->xState.bState != bState);
        pEvent->xState.bState   = bState;

        FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xObjTime);
        
        if (pEvent->xState.bStateChanged)
        {
            pEvent->xState.xTimeStamp = xObjTime;
        }
        
        if ((pEvent->xState.bStateChanged == FALSE) && pEvent->xState.bState != pEvent->xState.bOccurred)
        {
            if (FTE_TIME_diff(&xCurrentTime, &pEvent->xState.xTimeStamp) > pEvent->pConfig->ulDelayTime)
            {
                pEvent->xState.xTimeStamp   = xObjTime;
                pEvent->xState.bChanged     = TRUE;
                pEvent->xState.bOccurred    = pEvent->xState.bState;
            }
        }
    }
    else
    {
        FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &pEvent->xState.xTimeStamp);
        pEvent->xState.bChanged     = (bState && !pEvent->xState.bOccurred);
        pEvent->xState.bOccurred    = bState;
    }
    
    return  MQX_OK;
}

_mqx_uint    FTE_EVENT_proc(FTE_EVENT_PTR pEvent, TIME_STRUCT_PTR pTime)
{
    if (pEvent->pConfig->xCondition == FTE_EVENT_CONDITION_INTERVAL)
    {
        if (pEvent->xState.xTimeStamp.SECONDS == 0)
        {
            pEvent->xState.xTimeStamp.SECONDS = pTime->SECONDS + pEvent->pConfig->xParams.ulInterval;
        }
        else if (pEvent->xState.xTimeStamp.SECONDS < pTime->SECONDS)
        {
            pEvent->xState.bChanged = TRUE;
            pEvent->xState.xTimeStamp.SECONDS += pEvent->pConfig->xParams.ulInterval;
        }            
    }
    
    if (pEvent->xState.bChanged)
    {
        FTE_OBJECT_PTR  pObj;

        for(int i = 0 ; i < FTE_LIST_count(&pEvent->xState.xObjectList) ; i++)
        {
            pObj = (FTE_OBJECT_PTR)FTE_LIST_getAt(&pEvent->xState.xObjectList, i);
#if 0
            if (pEvent->xState.pObj == NULL)
            {
                pEvent->xState.pObj = FTE_OBJ_get(pEvent->pConfig->ulEPID);
                if (pEvent->xState.pObj == NULL)
                {
    //                TRACE(DEBUG_EVENT, "%s : Object not found[ulEPID = %08lx]\n", __func__, pEvent->pConfig->ulEPID);
                    return  MQX_INVALID_HANDLE;
                }
            }
            
            pObj = pEvent->xState.pObj;
#endif
            
            pEvent->xState.bChanged = FALSE;
            
            if (FTE_FLAG_IS_SET(pEvent->pConfig->xType, FTE_EVENT_TYPE_LOG))
            {
                FTE_LOG_add(pObj->pConfig->xCommon.nID, pObj->pStatus->pValue);
            }

#if FTE_SNMPD_SUPPORTED
            if (FTE_FLAG_IS_SET(pEvent->pConfig->xType, FTE_EVENT_TYPE_SNMP_TRAP))
            {
                FTE_SNMPD_TRAP_sendAlert(pObj->pConfig->xCommon.nID, pEvent->xState.bOccurred);
            }
#endif
            
#if FTE_MQTT_SUPPORTED
            if (FTE_FLAG_IS_SET(pEvent->pConfig->xType, FTE_EVENT_TYPE_MQTT_PUB))
            {
                FTE_MQTT_publishEPValue(pObj->pConfig->xCommon.nID, FTE_MQTT_QOS_2);
            }
#endif
        }
    }
    
    return  MQX_OK;
}

char_ptr FTE_EVENT_CONDITION_string(uint_32 ulType)
{
    switch(ulType)
    {
    case    FTE_EVENT_CONDITION_ABOVE:   return  "MORE THAN";        
    case    FTE_EVENT_CONDITION_BELOW:   return  "BELOW";        
    case    FTE_EVENT_CONDITION_INSIDE:  return  "INSIDE";
    case    FTE_EVENT_CONDITION_OUTSIDE: return  "OUTSIDE";
    case    FTE_EVENT_CONDITION_INTERVAL:return  "INTERVAL";
    case    FTE_EVENT_CONDITION_TIME:    return  "FIXED TIME";
    
    default:                        return  "UNKNOWN";
    }
       
}

int_32 FTE_EVENT_shell_cmd(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_EVENT_PTR       pEvent;
                FTE_LIST_ITERATOR   xIter;
                uint_32             ulIndex = 0;
                
                FTE_LIST_ITER_init(&_eventList, &xIter);
                while((pEvent = FTE_LIST_ITER_getNext(&xIter)) != 0)
                {
                    ulIndex++;
                    
                    printf("%2d : %08lx %-12s\n", ulIndex, pEvent->pConfig->ulEPID, FTE_EVENT_CONDITION_string(pEvent->pConfig->xCondition));
                }

            }
            break;
            
        case    3:
            {
                if (strcmp(pArgv[1], "del") == 0)
                {
                    FTE_EVENT_PTR           pEvent;
                    uint_32                 ulIndex;
                    
                    if (!Shell_parse_number(pArgv[2], &ulIndex))
                    {
                        bPrintUsage = TRUE;
                        break;
                    }
                    
                    if ((pEvent = FTE_EVENT_getAt(ulIndex)) != MQX_OK)                        
                    {
                         FTE_OBJECT_PTR pObject = FTE_OBJ_get(pEvent->pConfig->ulEPID);
                         if (pObject != NULL)
                         {
                            FTE_OBJ_EVENT_detach(pObject, pEvent);
                            FTE_CFG_EVENT_free(pEvent->pConfig);
                            FTE_EVENT_destroy(pEvent);
                            FTE_CFG_save(TRUE);
                            return  SHELL_EXIT_SUCCESS;                                
                        }
                    }                               
                }
            }
        case    5:
            {
                if (strcmp(pArgv[1], "add") == 0)
                {
                    FTE_CFG_EVENT_PTR pEventConfig;
                    FTE_EVENT_PTR     pEvent;
                    FTE_OBJECT_PTR    pObject;
                    uint_32           ulEPID;
                    uint_32           ulInterval;
                    
                    if (!Shell_parse_hexnum(pArgv[2], &ulEPID))
                    {
                        bPrintUsage = TRUE;
                        break;
                    }
                    
                    pObject = FTE_OBJ_get(ulEPID);
                    if (pObject == NULL)
                    {
                        printf("The object[%08lx] not found!\n", ulEPID);
                        break;
                    }
                    
                    if (strcmp(pArgv[3], "interval") == 0)
                    {
                        if (!Shell_parse_number(pArgv[4], &ulInterval))
                        {
                            bPrintUsage = TRUE;
                            break;
                        }

                        pEventConfig = FTE_CFG_EVENT_alloc(ulEPID);
                        if (pEventConfig == NULL)
                        {
                            bPrintUsage = TRUE;
                            break;
                        }
                        
                        pEventConfig->xCondition= FTE_EVENT_CONDITION_INTERVAL;
                        pEventConfig->xParams.ulInterval = ulInterval;
                        
                        FTE_EVENT_create(pEventConfig, &pEvent);
                        
                        FTE_OBJ_EVENT_attach(pObject, pEvent);
                        
                        FTE_CFG_save(TRUE);

                    }
                }
                else
                {
                    bPrintUsage = TRUE;
                }
            }
            break;
            
        default:
            bPrintUsage = TRUE;
        }
    }
                    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s <cmd>\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s <cmd>\n", pArgv[0]);
        }
    }
    
    
    return  nReturnCode;
}

_mqx_uint    FTE_EVENT_type_string(uint_32 xType, char_ptr pBuff, int_32 nBuffLen)
{
    int nLen = 0;
    
    if ((xType & FTE_EVENT_TYPE_LOG) == FTE_EVENT_TYPE_LOG)
    {
        nLen = snprintf(pBuff, nBuffLen, "%s", "log");
    }
    
    if ((xType & FTE_EVENT_TYPE_LOG) == FTE_EVENT_TYPE_SNMP_TRAP)
    {
        if (nLen != 0)
        {
            nLen += snprintf(pBuff, nBuffLen - nLen, " | %s", "snmp");
        }
        else
        {
            nLen += snprintf(pBuff, nBuffLen, "%s", "snmp");
        }
    }
    
    if ((xType & FTE_EVENT_TYPE_LOG) == FTE_EVENT_TYPE_MQTT_PUB)
    {
        if (nLen != 0)
        {
            nLen += snprintf(pBuff, nBuffLen - nLen, " | %s", "mqtt");
        }
        else
        {
            nLen += snprintf(pBuff, nBuffLen, "%s", "mqtt");
        }
    } 

    if (nLen == 0)
    {
        snprintf(pBuff, nLen, "undefined");
    }
    
    return  MQX_OK;
}

_mqx_uint    FTE_EVENT_condition_string(FTE_EVENT_CONDITION xCondition, char_ptr pBuff, int_32 nBuffLen)
{
    switch(xCondition)
    {
    case    FTE_EVENT_CONDITION_NONE:       snprintf(pBuff, nBuffLen, "none");
    case    FTE_EVENT_CONDITION_ABOVE:      snprintf(pBuff, nBuffLen, "above");
    case    FTE_EVENT_CONDITION_BELOW:      snprintf(pBuff, nBuffLen, "below");
    case    FTE_EVENT_CONDITION_INSIDE:     snprintf(pBuff, nBuffLen, "inside");
    case    FTE_EVENT_CONDITION_OUTSIDE:    snprintf(pBuff, nBuffLen, "outside");
    case    FTE_EVENT_CONDITION_INTERVAL:   snprintf(pBuff, nBuffLen, "interval");
    case    FTE_EVENT_CONDITION_TIME:       snprintf(pBuff, nBuffLen, "time");
    }
    
    return  MQX_OK;
        
}

_mqx_uint    FTE_EVENT_level_string(FTE_EVENT_LEVEL  xLevel, char_ptr pBuff, int_32 nBuffLen)
{
    switch(xLevel)
    {
    case    FTE_EVENT_LEVEL_INFO:       snprintf(pBuff, nBuffLen, "information");
    case    FTE_EVENT_LEVEL_WARNING:    snprintf(pBuff, nBuffLen, "warning");
    case    FTE_EVENT_LEVEL_CRITICAL:   snprintf(pBuff, nBuffLen, "critical");
    }
    
    return  MQX_OK;
        
}

void FTE_EVENT_task(uint_32 params)
{
    TIME_STRUCT     xTime;
    uint_32         ulTime, ulNextTime;

    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    _time_get(&xTime);
    ulNextTime = xTime.SECONDS * 1000 + xTime.MILLISECONDS;
    
    while(1)
    {
        FTE_LIST_ITERATOR   xIter;
        
        FTE_LIST_ITER_init(&_eventList, &xIter);
        FTE_EVENT_PTR   pEvent;
        
        _time_get(&xTime);
        
        while((pEvent = (FTE_EVENT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            FTE_EVENT_proc(pEvent, &xTime);
        }
        
        _time_get(&xTime);
        ulNextTime += 100;
        ulTime = xTime.SECONDS * 1000 + xTime.MILLISECONDS;        
        
        _time_delay(ulNextTime - ulTime);        
    }      
}

