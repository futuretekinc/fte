#include "fte_target.h"
#include "fte_object.h"
#include "fte_net.h" 
#include "fte_config.h" 
#include "fte_log.h" 
#include "fte_time.h"

const char_ptr FTE_JSON_MSG_TYPE_STRING   = "type";

const char_ptr FTE_JSON_DEV_ID_STRING     = "id";
const char_ptr FTE_JSON_DEV_TIME_STRING   = "time";
#if 0
const char_ptr FTE_JSON_DEV_EP_STRING     = "ep";
#else
const char_ptr FTE_JSON_DEV_EP_STRING     = "value";
#endif
const char_ptr FTE_JSON_DEV_EPS_STRING    = "eps";

const char_ptr FTE_JSON_OBJ_ID_STRING     = "id";
const char_ptr FTE_JSON_OBJ_NAME_STRING   = "name";
const char_ptr FTE_JSON_OBJ_VALUE_STRING  = "value";
const char_ptr FTE_JSON_OBJ_TIME_STRING   = "time";
const char_ptr FTE_JSON_OBJ_STATE_STRING  = "state";
const char_ptr FTE_JSON_OBJ_METHOD_STRING = "method";
const char_ptr FTE_JSON_OBJ_PARAM_STRING  = "param";
const char_ptr FTE_JSON_OBJ_PARAMS_STRING  = "params";

static  FTE_LIST            _xObjList    = { 0, NULL, NULL };
static  int FTE_OBJ_comaratorID(pointer pObj1, pointer pObj2)
{
    uint_32 ulID1 = ((FTE_OBJECT_PTR)pObj1)->pConfig->xCommon.nID & (FTE_OBJ_CLASS_MASK | FTE_OBJ_CLASS_INDEX);
    uint_32 ulID2 = ((FTE_OBJECT_PTR)pObj2)->pConfig->xCommon.nID & (FTE_OBJ_CLASS_MASK | FTE_OBJ_CLASS_INDEX);

    if (ulID1 < ulID2)
    {
        return  -1;
    }
    else if (ulID1 > ulID2)
    {
        return  1;
    }
    
    return  0;
}

FTE_OBJECT_PTR  FTE_OBJ_create(FTE_OBJECT_CONFIG_PTR pConfig)
{
    FTE_OBJECT_PTR      pObj = NULL;
    FTE_OBJECT_DESC_PTR pDesc = NULL;
    
    ASSERT(pConfig != NULL);

    pDesc = FTE_OBJ_DESC_get(FTE_ID_TYPE(pConfig->xCommon.nID));
    if (pDesc == NULL)
    {
        goto error;
    }

    pObj = (FTE_OBJECT_PTR)FTE_MEM_allocZero(sizeof(FTE_OBJECT));
    if(pObj == NULL)
    {
        goto error;
    }
    FTE_LIST_init(&pObj->xEventList);
    
    pObj->pConfig = FTE_MEM_allocZero(pDesc->nConfigSize);
    if (pObj->pConfig == NULL)
    {
        goto error;
    }        
    memcpy(pObj->pConfig, pConfig, pDesc->nConfigSize);

    pObj->pStatus = FTE_MEM_allocZero(pDesc->nStatusSize);
    if (pObj->pStatus == NULL)
    {
        goto error;
    }        
    
    if (pDesc->f_attach(pObj) != MQX_OK)
    {
        goto error;
    }

    if (FTE_LIST_pushSort(&_xObjList, pObj, FTE_OBJ_comaratorID) != MQX_OK)
    {
        goto error;
    }
    
    return  pObj;

error:    
    
    if (pObj != NULL)
    {
        FTE_LIST_final(&pObj->xEventList);
        
        if (pObj->pStatus != NULL)
        {
            FTE_MEM_free(pObj->pStatus);
            pObj->pStatus = NULL;
        }

         if (pObj->pConfig != NULL)
         {
            FTE_MEM_free(pObj->pConfig);
            pObj->pConfig = NULL;
         }
        
        FTE_MEM_free(pObj);
    }
    
    return  NULL;
}

_mqx_uint       FTE_OBJ_destroy(FTE_OBJECT_PTR pObj)
{
    FTE_OBJECT_DESC_PTR pDesc;
    
    ASSERT(pObj != NULL);    
    
    pDesc = FTE_OBJ_DESC_get(FTE_OBJ_TYPE(pObj));
    if (pDesc == NULL)
    {
        goto error;
    }

    FTE_LIST_remove(&_xObjList, pObj);
    
    pDesc->f_detach(pObj);
    
    FTE_MEM_free(pObj->pStatus);
    FTE_MEM_free(pObj->pConfig);
    FTE_MEM_free(pObj);

    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

FTE_OBJECT_PTR  FTE_OBJ_get(FTE_OBJECT_ID  nID)
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

FTE_OBJECT_PTR  FTE_OBJ_getAt(uint_32 nType, uint_32 nMask, uint_32 ulIndex, boolean bSystem)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            if (bSystem || !(pObj->pConfig->xCommon.nID & FTE_OBJ_SYSTEM))
            {
                if ((nType == FTE_OBJ_TYPE_UNKNOWN) ||((FTE_OBJ_ID(pObj) & nMask) == (nType & nMask)))
                {
                    if (ulIndex == 0)
                    {
                        return  pObj;
                    }
                    ulIndex--;
                }
            }
        }
    }
    
    return  NULL;
}

uint_32         FTE_OBJ_count(uint_32 nType, uint_32 nMask, boolean bSystem)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    uint_32             ulCount = 0;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            if (bSystem || !(pObj->pConfig->xCommon.nID & FTE_OBJ_SYSTEM))
            {
                if ((nType == FTE_OBJ_TYPE_UNKNOWN) ||((FTE_OBJ_ID(pObj) & nMask) == (nType & nMask)))
                {
                    ulCount++;
                }
            }
        }
    }
    
    return  ulCount;
}

uint_32 FTE_OBJ_getList(uint_32 nType, uint_32 nMask, FTE_OBJECT_PTR _PTR_ pObjectList, uint_32 nMaxCount)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    uint_32             ulCount = 0;

    ASSERT(pObjectList != NULL);
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            if (ulCount >= nMaxCount)
            {
                break;
            }
             
            if (!(pObj->pConfig->xCommon.nID & FTE_OBJ_SYSTEM))
            {
                if ((nType == FTE_OBJ_TYPE_UNKNOWN) || ((FTE_OBJ_ID(pObj) & nMask) == (nType & nMask)))
                {
                    pObjectList[ulCount++] = pObj;
                }
            }
        }
    }
    
    return  ulCount;
}

_mqx_uint   FTE_OBJ_getValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
        
    FTE_VALUE_copy(pValue, pObj->pStatus->pValue);
    
    return  MQX_OK;
}

_mqx_uint   FTE_OBJ_setValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
        
    if (pObj->pAction->f_set != NULL)
    {
        return  pObj->pAction->f_set(pObj, pValue);
    }
    
    return  MQX_ERROR;
}

_mqx_uint       FTE_OBJ_getValueAt(FTE_OBJECT_PTR pObj, uint_32 ulIdx, FTE_VALUE_PTR pValue)
{
    if (ulIdx < pObj->pStatus->nValueCount)
    {
        FTE_VALUE_copy(pValue, &pObj->pStatus->pValue[ulIdx]);
        
        return  MQX_OK;
    }
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_OBJ_setValueString(FTE_OBJECT_PTR pObj, char_ptr pString)
{
    FTE_VALUE   xValue;
    
    ASSERT(pObj != NULL);
    
    FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
    if (FTE_VALUE_set(&xValue, pString) == MQX_OK)
    {
        return  FTE_OBJ_setValue(pObj, &xValue);
    }
    
    return  MQX_ERROR;
}

FTE_VALUE_TYPE  FTE_OBJ_getValueType(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    return  pObj->pStatus->pValue->xType;
}

char_ptr    FTE_OBJ_typeString(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    FTE_OBJECT_DESC_PTR pDesc;

    pDesc = FTE_OBJ_DESC_get(FTE_OBJ_TYPE(pObj));
    if (pDesc == NULL)
    {
        return  NULL;
    }
    
    return  pDesc->pName;
}

_mqx_uint   FTE_OBJ_getConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen)
{
    ASSERT((pObj != NULL) && (pBuff!= NULL));
        
    if (pObj->pAction->f_set_config != NULL)
    {
        return  pObj->pAction->f_get_config(pObj, pBuff, ulBuffLen);
    }
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_OBJ_setConfig(FTE_OBJECT_PTR pObj, char_ptr pJSON)
{
    ASSERT((pObj != NULL) && (pJSON != NULL));
        
    if (pObj->pAction->f_set_config != NULL)
    {
        return  pObj->pAction->f_set_config(pObj, pJSON);
    }
    
    return  MQX_ERROR;
}


_mqx_uint    FTE_OBJ_getSN(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen)
{
    ASSERT(pObj != NULL);
        
    if (pObj->pAction->f_get_sn != NULL)
    {
        pObj->pAction->f_get_sn(pObj, pBuff, nLen);
        
        return  MQX_OK;
    }
    else
    {
        _enet_address   xMACAddress;
        
        FTE_SYS_getMAC(xMACAddress);
        snprintf(pBuff, nLen, "%02x%02x%02x%02x%02x%02x%08x", 
                        xMACAddress[0], xMACAddress[1],
                        xMACAddress[2], xMACAddress[3],
                        xMACAddress[4], xMACAddress[5],
                        pObj->pConfig->xCommon.nID);
    } 
    
    return  MQX_OK;
}

_mqx_uint       FTE_OBJ_activate(FTE_OBJECT_PTR pObj, boolean enabled)
{
    ASSERT(pObj != NULL);
    
    if (enabled)
    {
        if (!FTE_OBJ_IS_ENABLED(pObj))
        {
            if (FTE_OBJ_start(pObj) == MQX_OK)
            {
                if (pObj->pAction->f_get_child_count != NULL)
                {
                    uint_32 ulChild = 0;
                
                    pObj->pAction->f_get_child_count(pObj, &ulChild);
                    if (ulChild != 0)
                    {
                        uint_32 i;
                        
                        for(i = 0 ; i < ulChild ; i++)
                        {
                            FTE_OBJECT_ID   xChildID = 0;
                            
                            if (pObj->pAction->f_get_child(pObj, i, &xChildID) == MQX_OK)
                            {
                                FTE_OBJECT_PTR  pChild = FTE_OBJ_get(xChildID);
                                if (pChild != NULL)
                                {
                                    FTE_OBJ_start(pChild);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (FTE_OBJ_IS_ENABLED(pObj))
        {
            if (FTE_OBJ_stop(pObj) == MQX_OK)
            {
                if (pObj->pAction->f_get_child_count != NULL)
                {
                    uint_32 ulChild = 0;
                    
                    pObj->pAction->f_get_child_count(pObj, &ulChild);
                    if (ulChild != 0)
                    {
                        uint_32 i;
                        
                        for(i = 0 ; i < ulChild ; i++)
                        {
                            FTE_OBJECT_ID   xChildID = 0;
                            
                            if (pObj->pAction->f_get_child(pObj, i, &xChildID) == MQX_OK)
                            {
                                FTE_OBJECT_PTR  pChild = FTE_OBJ_get(xChildID);
                                if (pChild != NULL)
                                {
                                    FTE_OBJ_stop(pChild);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    FTE_OBJ_save(pObj);
     
    return  MQX_OK;
}

_mqx_uint       FTE_OBJ_start(FTE_OBJECT_PTR pObj)
{
    _mqx_uint   ulRet;
    
    ASSERT(pObj != NULL);
    
    if (pObj->pAction->f_run == NULL)
    {
        return  MQX_NOT_SUPPORTED_FUNCTION;
    }
    
    ulRet = pObj->pAction->f_run(pObj);
    if (ulRet == MQX_OK)
    {
        FTE_OBJ_FLAG_set(pObj, FTE_OBJ_CONFIG_FLAG_ENABLE);        
        if (pObj->pAction->f_get_child_count != NULL)
        {
            uint_32 ulChild;
            
            pObj->pAction->f_get_child_count(pObj, &ulChild);
            if (ulChild != 0)
            {
                uint_32 i;
                
                for(i = 0 ; i < ulChild ; i++)
                {
                    FTE_OBJECT_ID   xChildID = 0;
                    
                    if (pObj->pAction->f_get_child(pObj, i, &xChildID) == MQX_OK)
                    {
                        FTE_OBJECT_PTR  pChild = FTE_OBJ_get(xChildID);
                        if (pChild != NULL)
                        {
                            FTE_OBJ_start(pChild);
                        }
                    }
                }
            }
        }        
    }
    
    return  ulRet;
}

_mqx_uint       FTE_OBJ_stop(FTE_OBJECT_PTR pObj)
{
    _mqx_uint   ulRet;
    ASSERT(pObj != NULL);
    
    if (pObj->pAction->f_stop == NULL)
    {
        return  MQX_NOT_SUPPORTED_FUNCTION;
    }
    
    ulRet = pObj->pAction->f_stop(pObj);
    if (ulRet == MQX_OK)
    {
        FTE_OBJ_FLAG_clear(pObj, FTE_OBJ_CONFIG_FLAG_ENABLE);
        if (pObj->pAction->f_get_child_count != NULL)
        {
            uint_32 ulChild;
            
            pObj->pAction->f_get_child_count(pObj, &ulChild);
            if (ulChild != 0)
            {
                uint_32 i;
                
                for(i = 0 ; i < ulChild ; i++)
                {
                    FTE_OBJECT_ID   xChildID = 0;
                    
                    if (pObj->pAction->f_get_child(pObj, i, &xChildID) == MQX_OK)
                    {
                        FTE_OBJECT_PTR  pChild = FTE_OBJ_get(xChildID);
                        if (pChild != NULL)
                        {
                            FTE_OBJ_stop(pChild);
                        }
                    }
                }
            }
        }        
    }
    
    return  ulRet;
}

_mqx_uint       FTE_OBJ_wasUpdated(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    FTE_OBJ_STATE_set(pObj, FTE_OBJ_STATUS_FLAG_UPDATED);
    
    return  MQX_OK;
}

_mqx_uint       FTE_OBJ_wasChanged(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    FTE_OBJ_STATE_set(pObj, FTE_OBJ_STATUS_FLAG_UPDATED | FTE_OBJ_STATUS_FLAG_CHANGED);
    
    return  MQX_OK;
}

_mqx_uint       FTE_OBJ_EVENT_attach(FTE_OBJECT_PTR pObj, FTE_EVENT_PTR pEvent)
{
    ASSERT(pObj != NULL);
    
    FTE_LIST_pushBack(&pObj->xEventList, pEvent);
    FTE_EVENT_attachObject(pEvent, pObj);

    return  MQX_OK;
}

_mqx_uint       FTE_OBJ_EVENT_detach(FTE_OBJECT_PTR pObj, FTE_EVENT_PTR pEvent)
{
    ASSERT(pObj != NULL);
    
    FTE_EVENT_detachObject(pEvent, pObj);
    FTE_LIST_remove(&pObj->xEventList, pEvent);
    
    return  MQX_OK;
}
 
_mqx_uint   FTE_OBJ_getName(FTE_OBJECT_PTR pObj, char_ptr pName, uint_32 nBuffLen)
{
    ASSERT((pObj != NULL) && (pName != NULL));
    
    if (nBuffLen <= sizeof(pObj->pConfig->xCommon.pName))
    {
        strncpy(pName, pObj->pConfig->xCommon.pName, nBuffLen);
    }
    else
    {
        strcpy(pName, pObj->pConfig->xCommon.pName);
    }
    
    return  MQX_OK;
}

_mqx_uint   FTE_OBJ_setName(FTE_OBJECT_PTR pObj, char_ptr pName, uint_32 len)
{
    ASSERT((pObj != NULL) && ((pName != NULL) || (len == 0)));
    
    if (len <= MAX_OBJECT_NAME_LEN)
    {
        memset(pObj->pConfig->xCommon.pName, 0, sizeof(pObj->pConfig->xCommon.pName));
        memcpy(pObj->pConfig->xCommon.pName, pName, len);
        
    FTE_OBJ_save(pObj);
        
        return  MQX_OK;
    }
    
    return  MQX_ERROR;
}

boolean FTE_OBJ_FLAG_isSet(FTE_OBJECT_PTR pObj, uint_32 flag)
{
    ASSERT(pObj != NULL);
    
    return  FTE_FLAG_IS_SET(pObj->pConfig->xCommon.xFlags, flag);
}

_mqx_uint   FTE_OBJ_FLAG_set(FTE_OBJECT_PTR pObj, uint_32 flag)
{
    ASSERT(pObj != NULL);
    
    pObj->pConfig->xCommon.xFlags = FTE_FLAG_SET(pObj->pConfig->xCommon.xFlags, flag);
    FTE_OBJ_save(pObj);
    
    return  MQX_OK;
}

_mqx_uint   FTE_OBJ_FLAG_clear(FTE_OBJECT_PTR pObj, uint_32 flag)
{
    ASSERT(pObj != NULL);
    
    pObj->pConfig->xCommon.xFlags = FTE_FLAG_CLR(pObj->pConfig->xCommon.xFlags, flag);
    FTE_OBJ_save(pObj);
    
    return  MQX_OK;
}

_mqx_uint       FTE_OBJ_save(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    FTE_CFG_OBJ_save(pObj);
    
    return  MQX_OK;
}

uint_32         FTE_OBJ_runLoop(FTE_OBJECT_PTR pObj, TIMER_NOTIFICATION_TICK_FPTR f_callback, uint_32 nInterval)
{
    MQX_TICK_STRUCT     xTicks;            

    _time_init_ticks(&xTicks, 0);
    _time_add_msec_to_ticks(&xTicks, nInterval);
    return  _timer_start_periodic_every_ticks(f_callback, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks);    
}
        
uint_32         FTE_OBJ_runMeasurement(FTE_OBJECT_PTR pObj, TIMER_NOTIFICATION_TICK_FPTR f_callback, uint_32 nTimeout)
{
    MQX_TICK_STRUCT     xTicks;            

    _time_init_ticks(&xTicks, 0);
    _time_add_msec_to_ticks(&xTicks, nTimeout);
    return  _timer_start_oneshot_after_ticks(f_callback, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks);    
}
        

void FTE_OBJ_STATE_set(FTE_OBJECT_PTR pObj, uint_32 xFlags)
{
    pObj->pStatus->xFlags = FTE_FLAG_SET(pObj->pStatus->xFlags, xFlags);
}

void FTE_OBJ_STATE_clear(FTE_OBJECT_PTR pObj, uint_32 xFlags)
{
    pObj->pStatus->xFlags = FTE_FLAG_CLR(pObj->pStatus->xFlags, xFlags);
}

boolean FTE_OBJ_STATE_isSet(FTE_OBJECT_PTR pObj, uint_32 xFlags)
{
    return  FTE_FLAG_IS_SET(pObj->pStatus->xFlags, xFlags);
}


FTE_JSON_OBJECT_PTR  FTE_OBJ_createJSON(FTE_OBJECT_PTR pObj, uint_32 xOptions)
{
    FTE_JSON_OBJECT_PTR pObject = NULL;
    FTE_JSON_VALUE_PTR  pValue = NULL;

    ASSERT(pObj != NULL);

    pObject = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(8);
    if (pObject == NULL)
    {
        goto error;
    }
    
    for(uint_32 nOption = FTE_OBJ_FIELD_ID ; nOption != 0 ; nOption <<= 1)
    {
        if (FTE_FLAG_IS_CLR(xOptions, nOption))
        {
            continue;
        }
        
        switch(nOption)
        {
        case    FTE_OBJ_FIELD_ID:
            {
                char    pBuff[16];
                
                sprintf(pBuff, "%08x", pObj->pConfig->xCommon.nID);
                pValue = FTE_JSON_VALUE_createString(pBuff);
                if (pValue == NULL)
                {
                    goto error;
                }
                
                if (FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_ID_STRING, pValue) != FTE_JSON_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
                }
            }
            break;
            
        case    FTE_OBJ_FIELD_NAME:
            {
                pValue = FTE_JSON_VALUE_createString(pObj->pConfig->xCommon.pName);
                if (pValue == NULL)
                {
                    goto error;
                }

                if (FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_NAME_STRING, pValue) != FTE_JSON_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
                }
            }
            break;
            
        case    FTE_OBJ_FIELD_VALUE:
            {
                switch(FTE_OBJ_TYPE(pObj))
                {
                case FTE_OBJ_TYPE_MULTI_PN1500:
                    {
                        uint_32 ulValue;
                        boolean bValue;
                        FTE_JSON_OBJECT_PTR  pValues = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(3);
                        if (pValues == NULL)
                        {
                            goto error;
                        }
                        
                        FTE_VALUE_getULONG(&pObj->pStatus->pValue[0], &ulValue);
                        pValue = FTE_JSON_VALUE_createNumber(ulValue);
                        if (pValue == NULL)
                        {
                            FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
                            goto error;
                        }
                        if (FTE_JSON_OBJECT_setPair(pValues, "count", pValue) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
                            FTE_JSON_VALUE_destroy(pValue);
                            goto error;
                        }
                        
                        FTE_VALUE_getULONG(&pObj->pStatus->pValue[1], &ulValue);
                        pValue = FTE_JSON_VALUE_createNumber(ulValue);
                        if (pValue == NULL)
                        {
                            FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
                            goto error;
                        }
                        if (FTE_JSON_OBJECT_setPair(pValues, "accum", pValue) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
                            FTE_JSON_VALUE_destroy(pValue);
                            goto error;
                        }

                        FTE_VALUE_getDIO(&pObj->pStatus->pValue[2], &bValue);
                        pValue = FTE_JSON_VALUE_createNumber(bValue);
                        if (pValue == NULL)
                        {
                            FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
                            goto error;
                        }
                        if (FTE_JSON_OBJECT_setPair(pValues, "switch", pValue) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
                            FTE_JSON_VALUE_destroy(pValue);
                            goto error;
                        }                    
                        
                        if (FTE_JSON_OBJECT_setPair(pObject, "value", (FTE_JSON_VALUE_PTR)pValues) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
                            goto error;
                        }                    
                        
                    }
                    break;
                    
                 default:
                   {
#if FTE_ES18
                       uint_32  ulCmd = pObj->pStatus->pValue->xData.ulValue & 0xFF;
                       uint_32  ulLevel = (pObj->pStatus->pValue->xData.ulValue >> 8) & 0xFF;
                       uint_32  ulTime = (pObj->pStatus->pValue->xData.ulValue >> 16) & 0xFF;
                       
                        pValue = FTE_JSON_VALUE_createNumber(ulCmd);
                        if (pValue == NULL)
                        {
                            goto error;
                        }

                        if (FTE_JSON_OBJECT_setPair(pObject, "cmd", pValue) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy(pValue);
                            goto error;
                        }

                        pValue = FTE_JSON_VALUE_createNumber(ulLevel);
                        if (pValue == NULL)
                        {
                            goto error;
                        }

                        if (FTE_JSON_OBJECT_setPair(pObject, "level", pValue) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy(pValue);
                            goto error;
                        }

                        pValue = FTE_JSON_VALUE_createNumber(ulTime);
                        if (pValue == NULL)
                        {
                            goto error;
                        }

                        if (FTE_JSON_OBJECT_setPair(pObject, "time", pValue) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy(pValue);
                            goto error;
                        }
#else
                        pValue = FTE_JSON_VALUE_createValue(pObj->pStatus->pValue);
                        if (pValue == NULL)
                        {
                            goto error;
                        }

                        if (FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_VALUE_STRING, pValue) != FTE_JSON_RET_OK)
                        {
                            FTE_JSON_VALUE_destroy(pValue);
                            goto error;
                        }
#endif
                    }
                }
                   
            }
            break;
            
        case    FTE_OBJ_FIELD_TIME:
            {
                TIME_STRUCT xTimeStamp;
                
                FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTimeStamp);
                
                pValue = FTE_JSON_VALUE_createNumber(xTimeStamp.SECONDS);
                if (pValue == NULL)
                {
                    goto error;
                }

                if (FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_TIME_STRING, pValue) != FTE_JSON_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
                }
            }
            break;
            
        case    FTE_OBJ_FIELD_STATE:
            {
                if (FTE_FLAG_IS_SET(pObj->pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
                {
                    pValue = FTE_JSON_VALUE_createString("enable");
                }
                else
                {
                    pValue = FTE_JSON_VALUE_createString("disable");
                }

                if (pValue == NULL)
                {
                    goto error;
                }

                if (FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_STATE_STRING, pValue) != FTE_JSON_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
                }
            }
            break;
        }
    }
    
    return  (FTE_JSON_OBJECT_PTR)pObject;    
    
error:
    if (pObject != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pObject);        
    }
    
    return  NULL;
}

FTE_JSON_OBJECT_PTR  FTE_OBJ_createJSON2(FTE_OBJECT_PTR pObj, uint_32 xOptions)
{
    FTE_JSON_OBJECT_PTR pObject = NULL;
    FTE_JSON_VALUE_PTR  pValue = NULL;

    ASSERT(pObj != NULL);

    pObject = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(5);
    if (pObject == NULL)
    {
        goto error;
    }
    
    for(uint_32 nOption = FTE_OBJ_FIELD_ID ; nOption != 0 ; nOption <<= 1)
    {
        if (FTE_FLAG_IS_CLR(xOptions, nOption))
        {
            continue;
        }
        
        switch(nOption)
        {
        case    FTE_OBJ_FIELD_ID:
            {
                char    pBuff[16];
                
                sprintf(pBuff, "%08x", pObj->pConfig->xCommon.nID);
                pValue = FTE_JSON_VALUE_createString(pBuff);
                FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_ID_STRING, pValue);
            }
            break;
            
        case    FTE_OBJ_FIELD_NAME:
            {
                pValue = FTE_JSON_VALUE_createString(pObj->pConfig->xCommon.pName);
                FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_NAME_STRING, pValue);
            }
            break;
            
        case    FTE_OBJ_FIELD_VALUE:
            {
                //char    pValueString[32];
                
                //FTE_VALUE_toString(pObj->pStatus->pValue, pValueString, sizeof(pValueString));
                pValue = FTE_JSON_VALUE_createValue(pObj->pStatus->pValue);
                FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_VALUE_STRING, pValue);
            }
            break;
            
        case    FTE_OBJ_FIELD_TIME:
            {
                TIME_STRUCT xTimeStamp;
                
                FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTimeStamp);
                
                pValue = FTE_JSON_VALUE_createNumber(xTimeStamp.SECONDS);
                FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_TIME_STRING, pValue);
            }
            break;
            
        case    FTE_OBJ_FIELD_STATE:
            {
                if (FTE_FLAG_IS_SET(pObj->pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
                {
                    pValue = FTE_JSON_VALUE_createString("enable");
                }
                else
                {
                    pValue = FTE_JSON_VALUE_createString("disable");
                }
                FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_STATE_STRING, pValue);
            }
            break;
        }
    }
    
    return  (FTE_JSON_OBJECT_PTR)pObject;    
    
error:
    if (pObject != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pObject);        
    }
    
    if (pValue!= NULL)
    {
        FTE_JSON_VALUE_destroy(pValue);        
    }
    
    return  NULL;
}
uint_32    FTE_OBJ_1WIRE_discovery(boolean bSave)
{
    uint_32                     ulCount = 0;
#if FTE_1WIRE_SUPPORTED && FTE_DS18B20_SUPPORTED
    FTE_1WIRE_PTR               p1Wire;
    FTE_DS18B20_CREATE_PARAMS   xParams;
    uint_32                     nIndex;
    
    p1Wire = FTE_1WIRE_getFirst();
    while(p1Wire != 0)
    {
        FTE_1WIRE_discovery(p1Wire);                

        xParams.nBUSID = p1Wire->pConfig->nID;
        for(nIndex = 0 ; nIndex < FTE_1WIRE_DEV_count(p1Wire) ; nIndex++)
        {
            if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != MQX_OK)
            {
                break;
            }
         
            if (strcmp(FTE_1WIRE_getFailmyName(xParams.pROMCode[0]), "18B20") == 0)
            {
                if (!FTE_DS18B20_isExistROMCode(xParams.pROMCode) )
                {
                    FTE_OBJECT_PTR          pObj = FTE_DS18B20_create(&xParams);
                    if (pObj == NULL)
                    {
                        break;
                    }
                    
                    if (pObj->pAction->f_init != NULL);
                    {
                        pObj->pAction->f_init(pObj);
                    }
                    
                    if (pObj->pAction->f_run != NULL);
                    {
                        pObj->pAction->f_run(pObj);
                    }

                    ulCount++;
                    
                    if (bSave)
                    {
                        FTE_CFG_OBJ_save(pObj);
                    }
                }
            }
        }
        
        p1Wire = FTE_1WIRE_getNext(p1Wire);
    } 
    
    if (bSave)
    {
        FTE_CFG_save(TRUE);
    }
#endif
    
    return  ulCount;
}

_mqx_uint       FT_OBJ_STAT_incSucceed(FTE_OBJECT_STATISTICS_PTR pStatistics)
{
    int i;
    const int nPartialCount = FTE_OBJ_CHECK_FAILURE_COUNT_MAX / sizeof(uint_32);
    
    pStatistics->nTotalTrial++;
    
    if (pStatistics->pStatBits[nPartialCount - 1] & 0x80000000)
    {
        if (pStatistics->nPartialFail > 0)
        {
            pStatistics->nPartialFail--;
        }
    }
    
    for(i = nPartialCount - 1 ; i > 0 ; i--)
    {
        pStatistics->pStatBits[i] = (pStatistics->pStatBits[i] << 1) | (pStatistics->pStatBits[i - 1] >> 31);
    }
    
    pStatistics->pStatBits[0] = pStatistics->pStatBits[0] << 1;
    
    return  MQX_OK;
}

_mqx_uint       FT_OBJ_STAT_incFailed(FTE_OBJECT_STATISTICS_PTR pStatistics)
{
    int i;
    const int nPartialCount = FTE_OBJ_CHECK_FAILURE_COUNT_MAX / sizeof(uint_32);
   
    pStatistics->nTotalTrial++;
    pStatistics->nTotalFail++;
    
    if (pStatistics->pStatBits[nPartialCount - 1] & 0x80000000)
    {
        if (pStatistics->nPartialFail > 0)
        {
            pStatistics->nPartialFail--;
        }
    }
    
    for(i = nPartialCount - 1 ; i > 0 ; i--)
    {
        pStatistics->pStatBits[i] = (pStatistics->pStatBits[i] << 1) | (pStatistics->pStatBits[i - 1] >> 31);
    }
    
    pStatistics->pStatBits[0] = (pStatistics->pStatBits[0] << 1) | 1;
    pStatistics->nPartialFail++;

    if (pStatistics->nPartialFail >= FTE_OBJ_ALLOW_FAILURE_COUNT)
    {
        FTE_SYS_setUnstable();
    }
    
    return  MQX_OK;
}


int_32          FTE_OBJ_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
                uint_32 count = FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE);
                uint_32 i;

                printf("%-8s %-16s %-16s %8s %8s %7s %-s\n", 
                        "ID", "TYPE", "NAME", "STATUS", "VALUE", "       ", "TIME");
                for(i = 0 ; i < count ; i++)
                {
                    FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_UNKNOWN, 0, i, FALSE);
                    
                    if (pObj != NULL)
                    {
                        printf("%08x %-16s %-16s %8s", 
                              pObj->pConfig->xCommon.nID, 
                              FTE_OBJ_typeString(pObj), 
                              pObj->pConfig->xCommon.pName,
                              FTE_OBJ_IS_ENABLED(pObj)?"RUN":"STOP");                       
                        
                        if (pObj->pStatus->pValue != NULL)
                        {
                            TIME_STRUCT xTime;
                            char        pTimeString[64];
                            char        pValueString[32];
                            char        pUnitString[8];
                            
                            FTE_VALUE_toString(pObj->pStatus->pValue, pValueString, sizeof(pValueString));
                            FTE_VALUE_unit(pObj->pStatus->pValue, pUnitString, sizeof(pUnitString));
                            FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTime);
                            FTE_TIME_toString(&xTime, pTimeString, sizeof(pTimeString));
                            printf(" %8s %-7s %s", pValueString, pUnitString, pTimeString);
                        }
                        
                        if (pObj->pAction->f_get_statistic != NULL)
                        {
                            FTE_OBJECT_STATISTICS    xStatistics;
                            uint_32                 nRatio;
                            
                            pObj->pAction->f_get_statistic(pObj, &xStatistics);
                            nRatio = (xStatistics.nTotalTrial - xStatistics.nTotalFail) * 10000 / xStatistics.nTotalTrial;
                            
                            printf(" %3d.%02d%%(%d, %d, %d)", nRatio/100, nRatio%100, xStatistics.nTotalTrial, xStatistics.nTotalFail, xStatistics.nPartialFail);
                        }
                        printf("\n");
                        
                    }
                }
            }
            break;
            
        case    2:
            {
                uint_32     nID, i;
                TIME_STRUCT xTime;
                char        pTimeString[32];
                char        pValueString[16];
                char        pUnitString[8];
                
                if (Shell_parse_hexnum(argv[1], &nID) != TRUE)
                {
                    printf("Invalid OID[%s]\n", argv[1]);
                    return_code = SHELL_EXIT_ERROR;
                    break;
                }
                
                FTE_OBJECT_PTR  pObj = FTE_OBJ_get(nID);                    
                if (pObj == NULL)
                {
                    printf("Invalid OID[%08x]\n", nID);
                    return_code = SHELL_EXIT_ERROR;
                    break;
                }
                
                printf("%8s : %08x\n",  "ID",       pObj->pConfig->xCommon.nID);
                printf("%8s : %s\n",    "Type",     FTE_OBJ_typeString(pObj));
                printf("%8s : %s\n",    "Name",     pObj->pConfig->xCommon.pName);
                printf("%8s : %s\n",    "Status",   FTE_OBJ_IS_ENABLED(pObj)?"RUN":"STOP");                       
                
                    
                FTE_VALUE_toString(pObj->pStatus->pValue, pValueString, sizeof(pValueString));
                FTE_VALUE_unit(pObj->pStatus->pValue, pUnitString, sizeof(pUnitString));
                FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTime);
                FTE_TIME_toString(&xTime, pTimeString, sizeof(pTimeString));
                
                printf("%8s : %s\n",   "Value", pValueString);
                printf("%8s : %s\n",    "Unit", pUnitString);
                printf("%8s : %s\n",    "Time", pTimeString);
                
                if (pObj->pAction->f_get_update_interval != NULL)
                {
                    printf("%8s : %d secs\n", "Update", pObj->pAction->f_get_update_interval(pObj));
                }                    
                    
                if (FTE_OBJ_TYPE(pObj) == FTE_OBJ_TYPE_DI)
                {
                    FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
                    
                    printf("%8s : %d msecs\n",    "Delay", pConfig->ulDelay);
                    printf("%8s : %d msecs\n",    "Hold", pConfig->ulHold);
                }
                
                printf("%8s : %d\n", "Event", FTE_LIST_count(&pObj->xEventList));
                for(i = 0 ; i < FTE_LIST_count(&pObj->xEventList) ; i++)
                {
                    FTE_EVENT_PTR pEvent = FTE_LIST_getAt(&pObj->xEventList, i);
                    if (pEvent != NULL)
                    {
                        char                pTypeString[64];
                        
                        FTE_EVENT_type_string(pEvent->pConfig->xType, pTypeString, sizeof(pTypeString));
                        printf("%8d : %08lx %16s %-10s\n", 
                               i, pEvent->pConfig->ulEPID, 
                               FTE_EVENT_CONDITION_string(pEvent->pConfig->xCondition),
                               pTypeString);   
                    }
                }                            
                
                if (pObj->pAction->f_get_statistic != NULL)
                {
                    FTE_OBJECT_STATISTICS    xStatistics;
                    uint_32                 nRatio;
                    
                    pObj->pAction->f_get_statistic(pObj, &xStatistics);
                    nRatio = (xStatistics.nTotalTrial - xStatistics.nTotalFail) * 10000 / xStatistics.nTotalTrial;
                    
                    printf("%8s : %3d.%02d%%\n", "Rate", nRatio/100, nRatio%100);
                    printf("%8s : %d\n", "Total",   xStatistics.nTotalTrial);
                    printf("%8s : %d\n", "Failed",  xStatistics.nTotalFail);
                    printf("%8s : %d\n", "Partial", xStatistics.nPartialFail);
                }
            }
            break;            
            
        case    3:
            {
                uint_32 nID;
                
                if (Shell_parse_hexnum(argv[1], &nID) != TRUE)
                {
                    printf("Invalid OID[%s]\n", argv[1]);
                    return_code = SHELL_EXIT_ERROR;
                    break;
                }
                
                FTE_OBJECT_PTR  pObj = FTE_OBJ_get(nID);                    
                if (pObj == NULL)
                {
                    printf("Invalid OID[%08x]\n", nID);
                    return_code = SHELL_EXIT_ERROR;
                    break;
                }
                
                if (strcmp(argv[2], "info") == 0)
                {                
                    printf("%10s : %08x\n", "ID", pObj->pConfig->xCommon.nID);
                    printf("%10s : %s\n", "NAME", pObj->pConfig->xCommon.pName);
                    switch(FTE_OBJ_TYPE(pObj))
                    {
                    case    FTE_OBJ_TYPE_PT100:
                        {
                            printf("%10s : %08x\n", "AD7785", ((FTE_RTD_CONFIG_PTR)pObj->pConfig)->nDevID);
                        }
                        break;
                    }
                }
                else if (strcmp(argv[2], "run") == 0)
                {
                    if (FTE_OBJ_IS_ENABLED(pObj))
                    {
                        printf("%08x is already run.\n", nID);
                    }
                    else
                    {
                        FTE_OBJ_activate(pObj, TRUE);
                    }
                }
                else if (strcmp(argv[2], "stop") == 0)
                {                
                    if (! FTE_OBJ_IS_ENABLED(pObj))
                    {
                        printf("%08x is already stop.\n", nID);
                    }
                    else
                    {
                        FTE_OBJ_activate(pObj, FALSE);
                    }
                }
                else if (strcmp(argv[2], "destroy") == 0)
                {
                }
            }
            break;
            
        case    4:
            {
                uint_32  nOID = 0;

                Shell_parse_hexnum(argv[1], &nOID);
                
                FTE_OBJECT_PTR  pObj = FTE_OBJ_get(nOID);                    
                if (pObj == NULL)
                {
                    return_code = SHELL_EXIT_ERROR;
                    break;
                }
                
                if (strcmp(argv[2], "set") == 0)
                {
                    FTE_VALUE   xValue;
                    
                    FTE_VALUE_init(&xValue, FTE_OBJ_getValueType(pObj));
                    if (FTE_VALUE_set(&xValue, argv[3]) == MQX_OK)
                    {
                        FTE_OBJ_setValue(pObj, &xValue);
                    }
                }
                else if (strcmp(argv[2], "delay") == 0)
                {
                    uint_32  ulDelay;
                    if (Shell_parse_number(argv[3], &ulDelay) != TRUE)
                    {
                        printf("Invalid parameter[%s]\n", argv[3]);
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    if (FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_DI)
                    {
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    if (ulDelay < FTE_OBJ_EVENT_CHECK_INTERVAL)
                    {
                        printf("Delay time is too short. This time must be longer then %d msecs.\n",
                               FTE_OBJ_EVENT_CHECK_INTERVAL);
                        break;
                    }
                    
                    FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
                    
                    pConfig->ulDelay = ulDelay;
                    FTE_CFG_OBJ_save(pObj);
                }
                else if (strcmp(argv[2], "hold") == 0)
                {
                    uint_32  ulHold;
                    if (Shell_parse_number(argv[3], &ulHold) != TRUE)
                    {
                        printf("Invalid hold time[%s]\n", argv[3]);
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    if (FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_DI)
                    {
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
                    
                    pConfig->ulHold = ulHold;
                    FTE_CFG_OBJ_save(pObj);
                }
                else if (strcmp(argv[2], "update") == 0)
                {
                    uint_32  ulSeconds;
                    if (Shell_parse_number(argv[3], &ulSeconds) != TRUE)
                    {
                        printf("Invalid update interval[%s]\n", argv[3]);
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }

                    if (pObj->pAction->f_set_update_interval == NULL)
                    {
                        printf("Not supported changing the update interval.\n");
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    if (pObj->pAction->f_set_update_interval(pObj, ulSeconds) != MQX_OK)
                    {
                        printf("Object failed to change the update interval\n"); 
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    FTE_CFG_OBJ_save(pObj);
                }
                else if (strcmp(argv[2], "reverse") == 0)
                {
                    if (FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_DI)
                    {
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }

                    if (strcmp(argv[3], "on") == 0)
                    {
                        if (!FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
                        {                        
                            FTE_OBJ_FLAG_set(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE);
                            pObj->pStatus->pValue->xData.bValue = !pObj->pStatus->pValue->xData.bValue;
                            
                            FTE_CFG_OBJ_save(pObj);
                        }

                    }
                    else if (strcmp(argv[3], "off") == 0)
                    {
                        if (FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
                        {                        
                            FTE_OBJ_FLAG_clear(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE);
                            pObj->pStatus->pValue->xData.bValue = !pObj->pStatus->pValue->xData.bValue;
                            
                            FTE_CFG_OBJ_save(pObj);
                        }
                    }
                    else
                    {
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                        
                }
            }
            break;
                        
        default:
            print_usage = TRUE;
        }
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<command>]\n", argv[0]);
        }
        else
        {
            printf ("Usage: %s [<command>]\n", argv[0]);
            printf ("  Commands:\n");
            printf ("    <id> info\n");
            printf ("        Show object information.\n");
            printf ("    <id> destroy\n");
            printf ("        Destroy object.\n");
            printf ("    <id> hold <msecs>\n");
            printf ("        signal hold time.\n");
            printf ("    <id> delay <msecs>\n");
            printf ("        signal delay time.\n");
            printf ("    <type> <op>\n");
            printf ("  Parameters:\n");
            printf ("    <type> = Target object type\n");
            printf ("         do : digital output\n");
            printf ("         di : digital input\n");
            printf ("         rl : relay\n");
            printf ("        led : LED\n");
            printf ("    <op> = operation command\n");
            printf ("         on : operation active\n");
            printf ("        off : operation inactive\n");
        }
    }
    return   return_code;
}
