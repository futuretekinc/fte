#include "fte_target.h"
#include "fte_net.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_value.h"
#include "sys/fte_sys_timer.h"
#include "fte_time.h"

#ifndef FTE_DIO_REMOVE_GLITCH
#define FTE_DIO_REMOVE_GLITCH   1
#endif
 
static  
FTE_RET   FTE_DI_init(FTE_OBJECT_PTR pObj);

static  
FTE_RET   FTE_DI_run(FTE_OBJECT_PTR pObj);

static  
FTE_RET   FTE_DI_stop(FTE_OBJECT_PTR pObj);

static  
void        FTE_DI_ISR(FTE_VOID_PTR);

void _FTE_DI_ISR(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);

FTE_DI_CONFIG FTE_GPIO_DI_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 1),
        .pName      = "DI0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    },
    .nGPIO      = FTE_DEV_GPIO_DI_0,
    .ulDelay    = 0,
    .ulHold     = 0
};

static  
FTE_LIST                _xObjList = { 0, NULL, NULL}; 

static  
FTE_OBJECT_ACTION       _xAction = 
{
    .fInit  =   FTE_DI_init,
    .fRun   =   FTE_DI_run,
    .fStop  =   FTE_DI_stop
};

FTE_RET FTE_DI_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT(pObj != NULL);
    
    FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
    FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;

    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_xAction;    

    pStatus->xCommon.nValueCount= 1;
    pStatus->xCommon.pValue     = FTE_VALUE_createDIO();
    if (pStatus->xCommon.pValue == NULL)
    {
        goto error;
    }
        
    pStatus->pGPIO = FTE_GPIO_get(pConfig->nGPIO);
    if (pStatus->pGPIO == NULL)
    {
        goto error;
    }
    
    if (FTE_GPIO_attach(pStatus->pGPIO, pConfig->xCommon.nID) != FTE_RET_OK)
    {
        goto error;
    }

    
    if (FTE_DI_init(pObj) != FTE_RET_OK)
    {
        FTE_GPIO_detach(pStatus->pGPIO);
        goto error;
    }

    FTE_LIST_pushBack(&_xObjList, pObj);

    return  FTE_RET_OK;
    
error:
   
    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.pValue = NULL;
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_DI_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    if (!FTE_LIST_isExist(&_xObjList, pObj))
    {
        goto error;
    }

    FTE_LIST_remove(&_xObjList, pObj);
    pObj->pAction = NULL;
    
    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_OBJECT_PTR _di_get_object
(
    FTE_OBJECT_ID   nID
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
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

FTE_UINT32  FTE_DI_count(void)
{
    return  FTE_LIST_count(&_xObjList);
}

FTE_RET   FTE_DI_getValue
(
    FTE_OBJECT_ID   nID, 
    FTE_UINT32_PTR  pValue
)
{
    ASSERT(pValue != NULL);
    
    FTE_OBJECT_PTR pObj = _di_get_object(nID);
    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }

    *pValue = pObj->pStatus->pValue->xData.bValue;
        
    return  FTE_RET_OK;    
}

FTE_BOOL     FTE_DI_isActive
(
    FTE_OBJECT_ID  nID
)
{
    FTE_OBJECT_PTR pObj = _di_get_object(nID);
    if (pObj == NULL)
    {
        return  FALSE;
    }

    return  pObj->pStatus->pValue->xData.bValue;    
}

FTE_RET       FTE_DI_update(void)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
            FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;
            
            if (pStatus->xCommon.pValue->xData.bValue != pStatus->xPresetValue.xData.bValue)
            {
                FTE_INT32   nDelayTime = 0;
                FTE_INT32   nHoldTime = 0;
                TIME_STRUCT xTime;

                _time_get(&xTime);

                FTE_TIME_diffMilliseconds(&pStatus->xPresetValue.xTimeStamp, &xTime, &nDelayTime);
                FTE_TIME_diffMilliseconds(&pStatus->xCommon.pValue->xTimeStamp, &xTime, &nHoldTime);
                
                if (((nHoldTime == 0) || (nHoldTime >= pConfig->ulHold)) && ((pConfig->ulDelay == 0) || (nDelayTime >= pConfig->ulDelay)))
                {
                    FTE_VALUE_copy(pStatus->xCommon.pValue, &pStatus->xPresetValue);
                    
                    if (pConfig->nLED != 0)
                    {
                        FTE_LED_setValue(pConfig->nLED, pStatus->xCommon.pValue->xData.bValue);
                    }

                    FTE_OBJ_wasChanged(pObj);
                    TRACE(DEBUG_DI, "The DI detection applied.[ Interval > %d msecs]\n", nDelayTime);
                }
            }
            else if (pStatus->xPresetValue.bChanged)
            {
                pStatus->xCommon.pValue->xTimeStamp = pStatus->xPresetValue.xTimeStamp;
            }
        }
    }
    
    return  FTE_RET_OK;

}

FTE_RET   FTE_DI_INT_lock
(
    FTE_OBJECT_PTR  pObj
)
{
    if (pObj != NULL)
    {
        return  FTE_GPIO_INT_setEnable(((FTE_DI_STATUS_PTR)pObj->pStatus)->pGPIO, FALSE);    
    }
    else
    {
        FTE_LIST_ITERATOR   xIter;
        
        if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
        {
            while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
            {
                FTE_DI_INT_lock(pObj);
            }
        }
        
        return  FTE_RET_OK;
    }
}

FTE_RET   FTE_DI_INT_unlock
(
    FTE_OBJECT_PTR      pObj
)
{
    if (pObj != NULL)
    {
        FTE_DI_STATUS_PTR   pStatus;
        TIME_STRUCT xTime;
        FTE_INT32   nDiffTime = 0;
        
        pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;
        
        _time_get(&xTime);
        
#if FTE_DIO_REMOVE_GLITCH
        FTE_TIME_diffMilliseconds(&xTime, &pStatus->xPresetValue.xTimeStamp, &nDiffTime);
        if (nDiffTime < 200)
        {
            return  0;
        }
        
        if (pStatus->xPresetValue.xData.bValue == TRUE)
#else
        FTE_TIME_diffMilliseconds(&xTime, &pStatus->xCommon.xValue.xTimeStamp, &nDiffTime);
        if (nDiffTime < 200)
        {
            return  0;
        }
        
        if (pStatus->pValue->xData.bValue == TRUE)
#endif
        {
            if (!FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
            {
                FTE_GPIO_INT_setPolarity(pStatus->pGPIO, FTE_LWGPIO_INT_LOW);
            }
            else
            {
                FTE_GPIO_INT_setPolarity(pStatus->pGPIO, FTE_LWGPIO_INT_HIGH);
            }
        }
        else
        {
            if (!FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
            {
                FTE_GPIO_INT_setPolarity(pStatus->pGPIO, FTE_LWGPIO_INT_HIGH);
            }
            else
            {
                FTE_GPIO_INT_setPolarity(pStatus->pGPIO, FTE_LWGPIO_INT_LOW);
            }
        }
        
        return  FTE_GPIO_INT_setEnable(pStatus->pGPIO, TRUE);
    }
    else
    {
        FTE_LIST_ITERATOR   xIter;
        
        if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
        {
            while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
            {
                FTE_DI_INT_unlock(pObj);
            }
        }
        
        return  FTE_RET_OK;
    }
}

/******************************************************************************/
FTE_RET   FTE_DI_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    FTE_UINT32             nValue;
    FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
    FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;
    
    if (FTE_GPIO_getValue(pStatus->pGPIO, &nValue) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }

    if (FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
    {
        nValue = !nValue;
    }
    
    FTE_VALUE_setDIO(pStatus->xCommon.pValue, nValue);
    FTE_VALUE_setDIO(&pStatus->xPresetValue, nValue);
    
    if (pConfig->nLED != 0)
    {
        FTE_LED_setValue(pConfig->nLED, pStatus->xCommon.pValue->xData.bValue);
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_DI_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
        
    FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;

    FTE_GPIO_setISR(pStatus->pGPIO, FTE_DI_ISR, pObj);
    FTE_GPIO_INT_init(pStatus->pGPIO, 3, 0, TRUE);

    FTE_DI_INT_unlock(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET    FTE_DI_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;

    FTE_GPIO_INT_init(pStatus->pGPIO, 3, 0, FALSE);
    FTE_GPIO_setISR(pStatus->pGPIO, 0, 0);
    
    return  FTE_RET_OK;
}
 
FTE_RET   FTE_DI_setPolarity(FTE_OBJECT_PTR pObj, FTE_BOOL bActiveHI)
{
    ASSERT(pObj != NULL);
    
    if (bActiveHI)
    {
        FTE_OBJ_FLAG_set(pObj, FTE_DI_CONFIG_FLAG_POLARITY_HI);
    }
    else
    {
        FTE_OBJ_FLAG_clear(pObj, FTE_DI_CONFIG_FLAG_POLARITY_HI);
    }
    
    return  FTE_RET_OK;
}

/*ISR*-----------------------------------------------------
*
* Task Name    : di_int
* Comments     :
* Digital Input Signal ISR
*END*-----------------------------------------------------*/
void FTE_DI_ISR
(
    FTE_VOID_PTR    pParams
)
{
    if (pParams != NULL)
    {
        FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pParams;
        FTE_DI_CONFIG_PTR   pConfig;
        
        pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
        
        if (FTE_FLAG_IS_SET(pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
        {
            FTE_DI_STATUS_PTR   pStatus;
            FTE_BOOL             bFlag = FALSE;
            
            pStatus  = (FTE_DI_STATUS_PTR)pObj->pStatus;
            
            FTE_GPIO_INT_getFlag(pStatus->pGPIO, &bFlag);
            
            if (bFlag)
            {   
                FTE_BOOL             bValue = FALSE;
                
                FTE_GPIO_getValue(pStatus->pGPIO, &bValue);
                if (FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
                {
                    bValue = !bValue;
                }
                
                
#if FTE_DIO_REMOVE_GLITCH
                FTE_VALUE_setDIO(&pStatus->xPresetValue, bValue);
#else
                if (pStatus->xCommon.pValue->xData.bValue != bValue)
                {
                    FTE_VALUE_setDIO(pStatus->xCommon.pValue, bValue);
                    
                    if (pConfig->nLED != 0)
                    {
                        FTE_LED_setValue(pConfig->nLED, bValue);
                    }

                    FTE_OBJ_wasChanged(pObj);
                    _time_get(&pStatus->xLastOccurredTime);                    
                }
#endif
                FTE_GPIO_INT_setEnable(pStatus->pGPIO, FALSE);
                FTE_GPIO_INT_clrFlag(pStatus->pGPIO);
            }
        }        
    }
    else
    {
        FTE_OBJECT_PTR      pObj;
        FTE_LIST_ITERATOR   xIter;
        
        if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
        {
            while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
            {
                FTE_DI_CONFIG_PTR   pConfig;
                
                pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
                
                if (FTE_FLAG_IS_SET(pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
                {
                    FTE_DI_STATUS_PTR   pStatus;
                    FTE_BOOL             flag = FALSE;
                    pStatus  = (FTE_DI_STATUS_PTR)pObj->pStatus;
                    
                    FTE_GPIO_INT_getFlag(pStatus->pGPIO, &flag);
                    
                    if (flag)
                    {       
                        FTE_BOOL             bValue = FALSE;
                        
                        FTE_GPIO_getValue(pStatus->pGPIO, &bValue);
                        if (FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
                        {
                            bValue = !bValue;
                        }
                        
                        
#if FTE_DIO_REMOVE_GLITCH
                        FTE_VALUE_setDIO(&pStatus->xPresetValue, bValue);
#else
                        if (pStatus->xCommon.pValue->xData.bValue != bValue)
                        {
                            FTE_VALUE_setDIO(pStatus->xCommon.pValue, bValue);
                            
                            if (pConfig->nLED != 0)
                            {
                                FTE_LED_setValue(pConfig->nLED, bValue);
                            }

                            FTE_OBJ_wasChanged(pObj);
                        }
#endif
                            
                        FTE_GPIO_INT_setEnable(pStatus->pGPIO, FALSE);
                        FTE_GPIO_INT_clrFlag(pStatus->pGPIO);
                    }
                }        
            }
        }
    }
        
}

void _FTE_DI_ISR
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
//    FTE_DI_ISR(NULL);    
    FTE_DI_INT_unlock(NULL);
}

FTE_INT32      FTE_DI_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{ /* Body */
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_OBJECT_PTR      pObj;
                FTE_LIST_ITERATOR   xIter;
                
                if (FTE_LIST_count(&_xObjList) == 0)
                {
                    printf("The DI interface is not exists.\n");
                    break;
                }
                
                printf("    %08s %16s %6s\n", "ID", "NAME", "STATUS");
                
                if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
                {
                    while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
                    {
                        FTE_CHAR pValueString[32];
                        
                        FTE_VALUE_toString(((FTE_DI_STATUS_PTR)pObj->pStatus)->xCommon.pValue, pValueString, sizeof(pValueString));
                        printf("%08x: %16s %s\n", 
                               pObj->pConfig->xCommon.nID,
                               pObj->pConfig->xCommon.pName, 
                               pValueString);
                    }
                }                
            }
            break;
            
        case    2:
            {
                FTE_UINT32 nID;
                FTE_OBJECT_PTR  pObj = NULL;
                
                if (FTE_strToUINT32(pArgv[1], &nID) != FTE_RET_OK)
                {
                    bPrintUsage = TRUE;
                    break;
                }

                switch(FTE_ID_TYPE(nID))
                {
                case    0:
                    {
                        FTE_OBJECT_PTR pObj = FTE_OBJ_get(MAKE_ID(FTE_OBJ_TYPE_DI, nID));
                    }
                    break;
                    
                case    FTE_OBJ_TYPE_DI:
                    {
                        FTE_OBJECT_PTR pObj = FTE_OBJ_get(nID);
                    }
                    break;
                }
                
                if (pObj != NULL)
                {
                    FTE_CHAR pValueString[32];

                    FTE_VALUE_toString(((FTE_DI_STATUS_PTR)pObj->pStatus)->xCommon.pValue, pValueString, sizeof(pValueString));                            
                    
                    printf("    ID : %08x\n", pObj->pConfig->xCommon.nID);
                    printf("  NAME : %-16s\n", pObj->pConfig->xCommon.pName);
                    printf("STATUS : %s\n", pValueString);                            
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
            printf ("%s [<index>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<index>]\n", pArgv[0]);
            printf("        index - index of digital input\n");
        }
    }
    return   nReturnCode;
}
