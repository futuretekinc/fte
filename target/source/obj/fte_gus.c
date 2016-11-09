#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "fte_value.h"

#if FTE_GUS_SUPPORTED
 
#ifndef FTE_GUS_TRIAL_MAX
    #define FTE_GUS_TRIAL_MAX 3
#endif

static  FTE_RET     FTE_GUS_run(FTE_OBJECT_PTR pObj);
static  FTE_RET     FTE_GUS_stop(FTE_OBJECT_PTR pObj);
static  FTE_RET     FTE_GUS_startMeasurement(FTE_OBJECT_PTR pObj);
static  void        FTE_GUS_done(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);
static  FTE_RET     FTE_GUS_setMulti(FTE_OBJECT_PTR pObj, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);
static  FTE_RET     FTE_GUS_getMulti(FTE_OBJECT_PTR pObj, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);
static  void        FTE_GUS_restartConvert(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);
static  FTE_RET     FTE_GUS_getUpdateInterval(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pulInterval);
static  FTE_RET     FTE_GUS_setUpdateInterval(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);
static  FTE_RET     FTE_GUS_getStatistics(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistic);
static  FTE_RET     FTE_GUS_setConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pJSON);
static  FTE_RET     FTE_GUS_getConfig(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);
static  FTE_RET     FTE_GUS_setChildConfig(FTE_OBJECT_PTR pObj, FTE_OBJECT_PTR pChild, FTE_CHAR_PTR pJSON);
static  FTE_RET     FTE_GUS_getChildConfig(FTE_OBJECT_PTR pObj, FTE_OBJECT_PTR pChild, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);
static  FTE_RET     FTE_GUS_attachChild(FTE_OBJECT_PTR pObj, FTE_UINT32 nChild);
static  FTE_RET     FTE_GUS_detachChild(FTE_OBJECT_PTR pObj, FTE_UINT32 nChild);
static  FTE_RET     FTE_GUS_getChildCount(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pulCount);
static  FTE_RET     FTE_GUS_getChild(FTE_OBJECT_PTR pObj, FTE_UINT32 ulIndex, FTE_OBJECT_ID _PTR_ pxID);
static  FTE_RET     FTE_GUS_createJSON(FTE_OBJECT_PTR pObj, FTE_UINT32 ulOption, FTE_JSON_OBJECT_PTR _PTR_ ppJSON);

static  FTE_OBJECT_ACTION _Action = 
{ 
    .fRun   = FTE_GUS_run,
    .fStop  = FTE_GUS_stop, 
    .fSetMulti  = FTE_GUS_setMulti,
    .fGetMulti  = FTE_GUS_getMulti,
    .fGetInterval   = FTE_GUS_getUpdateInterval,
    .fSetInterval   = FTE_GUS_setUpdateInterval,
    .fGetStatistics = FTE_GUS_getStatistics,
    .fSetConfig     = FTE_GUS_setConfig,
    .fGetConfig     = FTE_GUS_getConfig,
    .fAttachChild   = FTE_GUS_attachChild,
    .fDetachChild   = FTE_GUS_detachChild,
    .fGetChildCount = FTE_GUS_getChildCount,
    .fGetChild      = FTE_GUS_getChild,
    .fSetChildConfig= FTE_GUS_setChildConfig,
    .fGetChildConfig= FTE_GUS_getChildConfig,
    .fCreateJSON    = FTE_GUS_createJSON
}; 
 
FTE_RET   FTE_GUS_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    FTE_GUS_STATUS_PTR              pStatus;
    FTE_UCS_PTR                     pUCS = NULL;
    
    ASSERT(pObj != NULL);

    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;    
    
    pStatus->pModelInfo = pOpts;
    if (pStatus->pModelInfo == NULL)
    {
        goto error;
    }    
    
    pStatus->xCommon.nValueCount = pStatus->pModelInfo->nFieldCount;
    pStatus->xCommon.pValue = FTE_VALUE_createArray(pStatus->pModelInfo->pValueTypes, pStatus->pModelInfo->nFieldCount);
        
    FTE_LIST_init(&pStatus->xChildList);
    
    pObj->pAction = &_Action;

    if (pStatus->pModelInfo->fAttach != NULL)
    {
        if (pStatus->pModelInfo->fAttach(pObj) != FTE_RET_OK)
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
        
        if (FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID) != FTE_RET_OK)
        {
            goto error;
        }
        
        pStatus->pUCS = pUCS;
    }
    
    return  FTE_RET_OK;
    
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
    
    return  FTE_RET_ERROR;
    
}

FTE_RET FTE_GUS_detach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_GUS_STATUS_PTR  pStatus;

    ASSERT(pObj != NULL);

    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    if (pStatus->pModelInfo->fDetach != NULL)
    {
        pStatus->pModelInfo->fDetach(pObj);
    }
    else if (pStatus->pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->pUCS, pObj->pConfig->xCommon.nID);
        pStatus->pUCS = NULL;
    }
    
    pObj->pAction = NULL;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_GUS_getChildCount
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pulCount
)
{
    ASSERT((pObj != NULL) && (pulCount != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    *pulCount = FTE_LIST_count(&pStatus->xChildList);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_GUS_getChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_OBJECT_ID _PTR_ pxID
)
{
    ASSERT((pObj != NULL) && (pxID != NULL));

    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;

    FTE_OBJECT_ID xID = (FTE_OBJECT_ID)FTE_LIST_getAt(&pStatus->xChildList, ulIndex);
    if (xID == 0)
    {
        return  FTE_RET_ERROR;
    }
    
    *pxID = xID;
    
    return  FTE_RET_OK;

}


FTE_RET FTE_GUS_attachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nChild
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;

    FTE_LIST_pushBack(&pStatus->xChildList, (FTE_VOID_PTR)nChild);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_GUS_detachChild
(
    FTE_OBJECT_PTR pObj, 
    FTE_UINT32  nChild
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;

    FTE_LIST_remove(&pStatus->xChildList, (FTE_VOID_PTR)nChild);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_GUS_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->pModelInfo->fRun != NULL)
    {
        return  pStatus->pModelInfo->fRun(pObj);
    }

    FTE_GUS_CONFIG_PTR    pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
        
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

    FTE_GUS_startMeasurement(pObj);

    return  FTE_OBJ_runLoop(pObj, FTE_GUS_restartConvert, pConfig->nInterval, &pStatus->hRepeatTimer);
}

FTE_RET   FTE_GUS_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->pModelInfo->fRun != NULL)
    {
        return  pStatus->pModelInfo->fStop(pObj);
    }

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
    
    return  FTE_RET_OK;
    
}

FTE_RET FTE_GUS_startMeasurement
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));
    
    FTE_RET xRet;
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
        
    _time_get_elapsed_ticks(&pStatus->xCommon.xStartTicks);
    
    if (pStatus->pModelInfo->fRequest != NULL)
    {
        pStatus->pModelInfo->fRequest(pObj);
        if (pStatus->pModelInfo->nMaxResponseTime != 0)
        {
            xRet = FTE_OBJ_runMeasurement(pObj, FTE_GUS_done, pStatus->pModelInfo->nMaxResponseTime, &pStatus->hConvertTimer);
        }
        else
        {
            xRet = FTE_OBJ_runMeasurement(pObj, FTE_GUS_done, FTE_GUS_RESPONSE_TIME, &pStatus->hConvertTimer);    
        }        
    }
    else
    {
        xRet = FTE_RET_NOT_SUPPORTED_FUNCTION;
    }

    return  xRet;
    
}

void FTE_GUS_done
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR        pObj = (FTE_OBJECT_PTR)pData;
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;

    if (pStatus->pModelInfo->fReceived != NULL)
    {
        if (pStatus->pModelInfo->fReceived(pObj) != FTE_RET_OK)
        {
            FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
            goto error;
        }
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


static 
void FTE_GUS_restartConvert
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        FTE_GUS_startMeasurement(pObj);
    }
    else
    {
        FTE_GUS_stop(pObj);
    }
}
 
FTE_RET    FTE_GUS_setMulti
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pModelInfo->fSet != NULL)
        {
            return  pStatus->pModelInfo->fSet(pObj, nIndex, pValue);
        }
    }

    return  FTE_RET_ERROR;
}

FTE_RET FTE_GUS_getMulti
(   
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pModelInfo->fGet != NULL)
        {
            return  pStatus->pModelInfo->fGet(pObj, nIndex, pValue);
        }
        else
        {
            if (nIndex < pObj->pStatus->nValueCount)
            {
                FTE_VALUE_copy(pValue, &pObj->pStatus->pValue[nIndex]);
                
                return  FTE_RET_OK;
            }
        }
    }

    return  FTE_RET_ERROR;
}

FTE_RET FTE_GUS_setConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pJSON
)
{
    ASSERT((pObj != NULL) && (pJSON != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if ((pStatus->pModelInfo == NULL) || (pStatus->pModelInfo->fSetConfig == NULL))
    {
        return  FTE_RET_NOT_SUPPORTED_FUNCTION;
    }
    
    return  pStatus->pModelInfo->fSetConfig(pObj, pJSON);
}

FTE_RET FTE_GUS_getConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    ASSERT((pObj != NULL) && (pBuff != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if ((pStatus->pModelInfo == NULL) || (pStatus->pModelInfo->fGetConfig == NULL))
    {
        return  FTE_RET_NOT_SUPPORTED_FUNCTION;
    }
    
    return  pStatus->pModelInfo->fGetConfig(pObj, pBuff, ulBuffLen);
}

FTE_RET FTE_GUS_setChildConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_PTR  pChild, 
    FTE_CHAR_PTR    pJSON
)
{
    ASSERT((pObj != NULL) && (pJSON != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if ((pStatus->pModelInfo == NULL) || (pStatus->pModelInfo->fSetChildConfig == NULL))
    {
        return  FTE_RET_NOT_SUPPORTED_FUNCTION;
    }

    return  pStatus->pModelInfo->fSetChildConfig(pChild, pJSON);
}

FTE_RET FTE_GUS_getChildConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_PTR  pChild, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    ASSERT((pObj != NULL) && (pBuff != NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pModelInfo->fGetChildConfig != NULL)
        {
            return  pStatus->pModelInfo->fGetChildConfig(pChild, pBuff, ulBuffLen);
        }
    }

    return  FTE_RET_ERROR;
}

FTE_RET FTE_GUS_getUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pulInterval
)
{
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    *pulInterval = pConfig->nInterval;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_GUS_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_GUS_getStatistics
(   
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTICS_PTR pStatistics
)
{
    ASSERT((pObj != NULL) && (pStatistics != NULL));
    
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistics, &pStatus->xCommon.xStatistics, sizeof(FTE_OBJECT_STATISTICS));
    
    return  FTE_RET_OK;
}


FTE_RET     FTE_GUS_createJSON
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulOption, 
    FTE_JSON_OBJECT_PTR _PTR_ ppJSON
)
{
    ASSERT((pObj != NULL) && (ppJSON!= NULL));
    
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pStatus->pModelInfo->fCreateJSON != NULL)
        {
            return  pStatus->pModelInfo->fCreateJSON(pObj, ulOption, ppJSON);
        }
    }

    return  FTE_RET_ERROR;
}

#endif