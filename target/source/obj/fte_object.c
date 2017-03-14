#include "fte_target.h"
#include "fte_object.h"
#include "fte_net.h" 
#include "fte_config.h" 
#include "fte_log.h" 
#include "fte_time.h"

const FTE_CHAR_PTR FTE_JSON_MSG_TYPE_STRING   = "type";

const FTE_CHAR_PTR FTE_JSON_DEV_ID_STRING     = "id";
const FTE_CHAR_PTR FTE_JSON_DEV_TIME_STRING   = "time";
#if 0
const FTE_CHAR_PTR FTE_JSON_DEV_EP_STRING     = "ep";
#else
const FTE_CHAR_PTR FTE_JSON_DEV_EP_STRING     = "value";
#endif
const FTE_CHAR_PTR FTE_JSON_DEV_EPS_STRING    = "eps";

const FTE_CHAR_PTR FTE_JSON_OBJ_DID_STRING     = "did";
const FTE_CHAR_PTR FTE_JSON_OBJ_ID_STRING     = "id";
const FTE_CHAR_PTR FTE_JSON_OBJ_NAME_STRING   = "name";
const FTE_CHAR_PTR FTE_JSON_OBJ_VALUE_STRING  = "value";
const FTE_CHAR_PTR FTE_JSON_OBJ_TIME_STRING   = "time";
const FTE_CHAR_PTR FTE_JSON_OBJ_STATE_STRING  = "state";
const FTE_CHAR_PTR FTE_JSON_OBJ_METHOD_STRING = "method";
const FTE_CHAR_PTR FTE_JSON_OBJ_PARAM_STRING  = "param";
const FTE_CHAR_PTR FTE_JSON_OBJ_PARAMS_STRING  = "params";

static  
FTE_LIST    _xObjList    = { 0, NULL, NULL };

static  
FTE_INT32   FTE_OBJ_comaratorID
(
    FTE_VOID_PTR    pObj1, 
    FTE_VOID_PTR    pObj2
)
{
    FTE_UINT32 ulID1 = ((FTE_OBJECT_PTR)pObj1)->pConfig->xCommon.nID & (FTE_OBJ_CLASS_MASK | FTE_OBJ_CLASS_INDEX);
    FTE_UINT32 ulID2 = ((FTE_OBJECT_PTR)pObj2)->pConfig->xCommon.nID & (FTE_OBJ_CLASS_MASK | FTE_OBJ_CLASS_INDEX);

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

FTE_OBJECT_PTR  FTE_OBJ_create
(
    FTE_OBJECT_CONFIG_PTR   pConfig
)
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
    
    if (pDesc->f_attach(pObj, pDesc->pOpts) != FTE_RET_OK)
    {
        goto error;
    }

    if (FTE_LIST_pushSort(&_xObjList, pObj, FTE_OBJ_comaratorID) != FTE_RET_OK)
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

FTE_RET FTE_OBJ_destroy
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_OBJECT_DESC_PTR pDesc;
    
    ASSERT(pObj != NULL);    
    
    pDesc = FTE_OBJ_DESC_get(FTE_OBJ_TYPE(pObj));
    if (pDesc == NULL)
    {
        goto error;
    }

    FTE_OBJ_stop(pObj);
    
    FTE_LIST_remove(&_xObjList, pObj);
    
    pDesc->f_detach(pObj);
    
    FTE_MEM_free(pObj->pStatus);
    FTE_MEM_free(pObj->pConfig);
    FTE_MEM_free(pObj);

    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_OBJ_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    if ((pObj->pAction == NULL) || (pObj->pAction->fInit == NULL))
    {
        return  FTE_RET_NOT_SUPPORTED_FUNCTION;
    }
    
    return  pObj->pAction->fInit(pObj);
}

FTE_OBJECT_PTR  FTE_OBJ_get
(
    FTE_OBJECT_ID  nID
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

FTE_OBJECT_PTR  FTE_OBJ_get2
(
    FTE_OBJECT_ID   nID,
    FTE_UINT32      nMask
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            if ((pObj->pConfig->xCommon.nID & nMask) == (nID & nMask))
            {
                return  pObj;
            }
        }
    }
    
    return  NULL;
}

FTE_OBJECT_PTR  FTE_OBJ_getAt
(
    FTE_UINT32  nType, 
    FTE_UINT32  nMask, 
    FTE_UINT32  ulIndex, 
    FTE_BOOL    bSystem
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
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

FTE_UINT32  FTE_OBJ_count
(
    FTE_UINT32  nType, 
    FTE_UINT32  nMask, 
    FTE_BOOL    bSystem
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    FTE_UINT32             ulCount = 0;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
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

FTE_UINT32 FTE_OBJ_getList
(
    FTE_UINT32  nType, 
    FTE_UINT32  nMask, 
    FTE_OBJECT_PTR _PTR_ pObjectList, 
    FTE_UINT32  nMaxCount
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    FTE_UINT32             ulCount = 0;

    ASSERT(pObjectList != NULL);
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
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

FTE_RET FTE_OBJ_getIDList
(
    FTE_UINT32  nType, 
    FTE_UINT32  nMask, 
    FTE_OBJECT_ID_PTR   pObjIDs, 
    FTE_UINT32  nMaxCount,
    FTE_UINT32_PTR  pCount
)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    FTE_UINT32             ulCount = 0;

    ASSERT(pObjIDs != NULL);
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == FTE_RET_OK)
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
                    pObjIDs[ulCount++] = pObj->pConfig->xCommon.nID;
                }
            }
        }
    }
    
    *pCount = ulCount;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_OBJ_getValue
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
        
    FTE_VALUE_copy(pValue, pObj->pStatus->pValue);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_OBJ_setValue
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
        
    if (pObj->pAction->fSet != NULL)
    {
        return  pObj->pAction->fSet(pObj, pValue);
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_OBJ_setValueAt
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT((pObj != NULL) && (pValue != NULL));
        
    if (pObj->pAction->fSetMulti != NULL)
    {
        return  pObj->pAction->fSetMulti(pObj, ulIndex, pValue);
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET       FTE_OBJ_getValueAt
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIdx, 
    FTE_VALUE_PTR   pValue
)
{
    if (pObj->pAction->fGetMulti != NULL)
    {
        return  pObj->pAction->fGetMulti(pObj, ulIdx, pValue);
    }
    else
    {
        if (ulIdx < pObj->pStatus->nValueCount)
        {
            FTE_VALUE_copy(pValue, &pObj->pStatus->pValue[ulIdx]);
            
            return  FTE_RET_OK;
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_OBJ_setValueString
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pString
)
{
    FTE_VALUE   xValue;
    
    ASSERT(pObj != NULL);
    
    FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
    if (FTE_VALUE_set(&xValue, pString) == FTE_RET_OK)
    {
        return  FTE_OBJ_setValue(pObj, &xValue);
    }
    
    return  FTE_RET_ERROR;
}

FTE_VALUE_TYPE  FTE_OBJ_getValueType
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    return  pObj->pStatus->pValue->xType;
}

FTE_CHAR_PTR    FTE_OBJ_typeString
(
    FTE_OBJECT_PTR  pObj
)
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

FTE_RET   FTE_OBJ_getConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    ASSERT((pObj != NULL) && (pBuff!= NULL));
        
    if (pObj->pAction->fGetConfig != NULL)
    {
        return  pObj->pAction->fGetConfig(pObj, pBuff, ulBuffLen);
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_OBJ_setConfig
(   
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pJSON
)
{
    ASSERT((pObj != NULL) && (pJSON != NULL));
        
    if (pObj->pAction->fSetConfig!= NULL)
    {
        return  pObj->pAction->fSetConfig(pObj, pJSON);
    }
    
    return  FTE_RET_ERROR;
}


FTE_RET    FTE_OBJ_getSN
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      nLen
)
{
    ASSERT(pObj != NULL);
        
    if (pObj->pAction->fGetSN != NULL)
    {
        pObj->pAction->fGetSN(pObj, pBuff, nLen);
        
        return  FTE_RET_OK;
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
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_getInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pulInterval
)
{
    ASSERT((pObj != NULL) && (pulInterval != NULL));
    
    if ((pObj->pAction == NULL) || (pObj->pAction->fGetInterval == NULL))
    {
        return  FTE_RET_NOT_SUPPORTED_FUNCTION;
    }

    return  pObj->pAction->fGetInterval(pObj, pulInterval);
}

FTE_RET FTE_OBJ_setInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulInterval
)
{
    ASSERT(pObj != NULL)
        
    if ((pObj->pAction == NULL) || (pObj->pAction->fSetInterval == NULL))
    {
        return  FTE_RET_NOT_SUPPORTED_FUNCTION;
    }

    return  pObj->pAction->fSetInterval(pObj,ulInterval);
}

FTE_RET    FTE_OBJ_getStatistics
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTICS_PTR pStatistics
)
{
    ASSERT((pObj != NULL) && (pStatistics != NULL));
        
    if (pObj->pAction->fGetStatistics != NULL)
    {
        pObj->pAction->fGetStatistics(pObj, pStatistics);
        
        return  FTE_RET_OK;
    }
    
    return  FTE_RET_NOT_SUPPORTED_FUNCTION;
}

FTE_RET       FTE_OBJ_activate
(
    FTE_OBJECT_PTR  pObj, 
    FTE_BOOL        enabled
)
{
    ASSERT(pObj != NULL);
    
    if (enabled)
    {
        if (!FTE_OBJ_IS_ENABLED(pObj))
        {
            if (FTE_OBJ_start(pObj) == FTE_RET_OK)
            {
                if (pObj->pAction->fGetChildCount != NULL)
                {
                    FTE_UINT32 ulChild = 0;
                
                    pObj->pAction->fGetChildCount(pObj, &ulChild);
                    if (ulChild != 0)
                    {
                        FTE_UINT32 i;
                        
                        for(i = 0 ; i < ulChild ; i++)
                        {
                            FTE_OBJECT_ID   xChildID = 0;
                            
                            if (pObj->pAction->fGetChild(pObj, i, &xChildID) == FTE_RET_OK)
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
            if (FTE_OBJ_stop(pObj) == FTE_RET_OK)
            {
                if (pObj->pAction->fGetChildCount != NULL)
                {
                    FTE_UINT32 ulChild = 0;
                    
                    pObj->pAction->fGetChildCount(pObj, &ulChild);
                    if (ulChild != 0)
                    {
                        FTE_UINT32 i;
                        
                        for(i = 0 ; i < ulChild ; i++)
                        {
                            FTE_OBJECT_ID   xChildID = 0;
                            
                            if (pObj->pAction->fGetChild(pObj, i, &xChildID) == FTE_RET_OK)
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
     
    return  FTE_RET_OK;
}

FTE_RET       FTE_OBJ_start
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_RET   ulRet;
    
    ASSERT(pObj != NULL);
    
    if (pObj->pAction->fRun == NULL)
    {
        return  FTE_RET_NOT_SUPPORTED_FUNCTION;
    }
    
    if (FTE_OBJ_IS_CHILD(pObj) == TRUE)
    {
        FTE_OBJECT_PTR  pParent = FTE_OBJ_get(((FTE_IFCE_CONFIG_PTR)pObj->pConfig)->nDevID);
        if (pParent == NULL)
        {
            return  FTE_RET_INVALID_OBJECT;
        }

        if (!FTE_OBJ_IS_RUN(pParent))
        {
            return  FTE_RET_PARENT_IS_STOPPED;
        }
    }
    
    ulRet = pObj->pAction->fRun(pObj);
    if (ulRet == FTE_RET_OK)
    {
        FTE_OBJ_FLAG_set(pObj, FTE_OBJ_CONFIG_FLAG_ENABLE);        
        if (pObj->pAction->fGetChildCount != NULL)
        {
            FTE_UINT32 ulChild;
            
            pObj->pAction->fGetChildCount(pObj, &ulChild);
            if (ulChild != 0)
            {
                FTE_UINT32 i;
                
                for(i = 0 ; i < ulChild ; i++)
                {
                    FTE_OBJECT_ID   xChildID = 0;
                    
                    if (pObj->pAction->fGetChild(pObj, i, &xChildID) == FTE_RET_OK)
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

FTE_RET       FTE_OBJ_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_RET   ulRet;
    ASSERT(pObj != NULL);
    
    if (pObj->pAction->fStop == NULL)
    {
        return  MQX_NOT_SUPPORTED_FUNCTION;
    }
    
    ulRet = pObj->pAction->fStop(pObj);
    if (ulRet == FTE_RET_OK)
    {
        FTE_OBJ_FLAG_clear(pObj, FTE_OBJ_CONFIG_FLAG_ENABLE);
        if (pObj->pAction->fGetChildCount != NULL)
        {
            FTE_UINT32 ulChild;
            
            pObj->pAction->fGetChildCount(pObj, &ulChild);
            if (ulChild != 0)
            {
                FTE_UINT32 i;
                
                for(i = 0 ; i < ulChild ; i++)
                {
                    FTE_OBJECT_ID   xChildID = 0;
                    
                    if (pObj->pAction->fGetChild(pObj, i, &xChildID) == FTE_RET_OK)
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

FTE_RET       FTE_OBJ_wasUpdated
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    FTE_OBJ_STATE_set(pObj, FTE_OBJ_STATUS_FLAG_UPDATED);
    
    return  FTE_RET_OK;
}

FTE_RET       FTE_OBJ_wasChanged
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    FTE_OBJ_STATE_set(pObj, FTE_OBJ_STATUS_FLAG_UPDATED | FTE_OBJ_STATUS_FLAG_CHANGED);
    
    return  FTE_RET_OK;
}

FTE_RET       FTE_OBJ_EVENT_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_EVENT_PTR   pEvent
)
{
    ASSERT(pObj != NULL);
    
    FTE_LIST_pushBack(&pObj->xEventList, pEvent);
    FTE_EVENT_attachObject(pEvent, pObj);

    return  FTE_RET_OK;
}

FTE_RET       FTE_OBJ_EVENT_detach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_EVENT_PTR   pEvent
)
{
    ASSERT((pObj != NULL) && (pEvent != NULL));
    
    FTE_EVENT_detachObject(pEvent, pObj);
    FTE_LIST_remove(&pObj->xEventList, pEvent);
    
    return  FTE_RET_OK;
}
 
FTE_RET   FTE_OBJ_getName
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pName, 
    FTE_UINT32      nBuffLen
)
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
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_OBJ_setName
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pName, 
    FTE_UINT32      len
)
{
    ASSERT((pObj != NULL) && ((pName != NULL) || (len == 0)));
    
    if (len <= MAX_OBJECT_NAME_LEN)
    {
        memset(pObj->pConfig->xCommon.pName, 0, sizeof(pObj->pConfig->xCommon.pName));
        memcpy(pObj->pConfig->xCommon.pName, pName, len);
        
    FTE_OBJ_save(pObj);
        
        return  FTE_RET_OK;
    }
    
    return  FTE_RET_ERROR;
}

FTE_BOOL FTE_OBJ_FLAG_isSet
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      flag
)
{
    ASSERT(pObj != NULL);
    
    return  FTE_FLAG_IS_SET(pObj->pConfig->xCommon.xFlags, flag);
}

FTE_RET   FTE_OBJ_FLAG_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      flag
)
{
    ASSERT(pObj != NULL);
    
    pObj->pConfig->xCommon.xFlags = FTE_FLAG_SET(pObj->pConfig->xCommon.xFlags, flag);
    FTE_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_OBJ_FLAG_clear
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      flag
)
{
    ASSERT(pObj != NULL);
    
    pObj->pConfig->xCommon.xFlags = FTE_FLAG_CLR(pObj->pConfig->xCommon.xFlags, flag);
    FTE_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET       FTE_OBJ_save
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    FTE_CFG_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_runLoop
(   
    FTE_OBJECT_PTR      pObj, 
    TIMER_NOTIFICATION_TICK_FPTR f_callback, 
    FTE_UINT32          ulInterval,
    FTE_TIMER_ID_PTR    pTimerID
)
{
    MQX_TICK_STRUCT     xTicks;            
    FTE_TIMER_ID        xTimerID;
    
    ASSERT((pObj != NULL) && (pTimerID != NULL));
    
    _time_init_ticks(&xTicks, 0);
    _time_add_msec_to_ticks(&xTicks, ulInterval);
    
    xTimerID = _timer_start_periodic_every_ticks(f_callback, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks);    
    if ( xTimerID != 0)
    {
        if (pTimerID != NULL)
        {
            *pTimerID = xTimerID;
        }
        
        return  FTE_RET_OK;
    }
    
    return  FTE_RET_ERROR;
}
        
FTE_RET FTE_OBJ_runMeasurement
(
    FTE_OBJECT_PTR      pObj, 
    TIMER_NOTIFICATION_TICK_FPTR f_callback, 
    FTE_UINT32          nTimeout,
    FTE_TIMER_ID_PTR    pTimerID
)
{
    MQX_TICK_STRUCT     xTicks;            
    FTE_TIMER_ID        xTimerID;
    
    _time_init_ticks(&xTicks, 0);
    _time_add_msec_to_ticks(&xTicks, nTimeout);
    xTimerID = _timer_start_oneshot_after_ticks(f_callback, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks);    

    if ( xTimerID != 0)
    {
        if (pTimerID != NULL)
        {
            *pTimerID = xTimerID;
        }
        
        return  FTE_RET_OK;
    }
    
    return  FTE_RET_ERROR;
    
}
        

void FTE_OBJ_STATE_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32  xFlags
)
{
    pObj->pStatus->xFlags = FTE_FLAG_SET(pObj->pStatus->xFlags, xFlags);
}

void FTE_OBJ_STATE_clear
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32  xFlags
)
{
    pObj->pStatus->xFlags = FTE_FLAG_CLR(pObj->pStatus->xFlags, xFlags);
}

FTE_BOOL FTE_OBJ_STATE_isSet
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      xFlags
)
{
    return  FTE_FLAG_IS_SET(pObj->pStatus->xFlags, xFlags);
}


FTE_RET FTE_OBJ_createJSON
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32  xOptions,
    FTE_JSON_OBJECT_PTR  _PTR_ ppObject
)
{
    FTE_RET xRet;
    FTE_JSON_OBJECT_PTR pObject = NULL;
    FTE_JSON_VALUE_PTR  pValue = NULL;

    ASSERT(pObj != NULL);

    pObject = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(8);
    if (pObject == NULL)
    {
        xRet = FTE_RET_NOT_ENOUGH_MEMORY;
        goto error;
    }
    
    for(FTE_UINT32 nOption = FTE_OBJ_FIELD_ID ; nOption != 0 ; nOption <<= 1)
    {
        if (FTE_FLAG_IS_CLR(xOptions, nOption))
        {
            continue;
        }
        
        switch(nOption)
        {
        case    FTE_OBJ_FIELD_DID:
            {
                _enet_address   xMACAddress;
                FTE_CHAR        pBuff[20];
                
                FTE_SYS_getMAC(xMACAddress);                
                snprintf(pBuff, sizeof(pBuff), "%02x%02x%02x%02x%02x%02x", 
                                    xMACAddress[0], xMACAddress[1],
                                    xMACAddress[2], xMACAddress[3],
                                    xMACAddress[4], xMACAddress[5]);
                
                pValue = FTE_JSON_VALUE_createString(pBuff);
                if (pValue == NULL)
                {
                    xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                    goto error;
                }

                xRet = FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_DID_STRING, pValue);
                if (xRet != FTE_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
                }
            }
            break;
            
        case    FTE_OBJ_FIELD_ID:
            {
                FTE_CHAR pBuff[16];
                
                sprintf(pBuff, "%08x", pObj->pConfig->xCommon.nID);
                pValue = FTE_JSON_VALUE_createString(pBuff);
                if (pValue == NULL)
                {
                    xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                    goto error;
                }
                
                xRet = FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_ID_STRING, pValue);
                if (xRet != FTE_RET_OK)
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
                    xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                    goto error;
                }

                xRet = FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_NAME_STRING, pValue);
                if ( xRet != FTE_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
                }
            }
            break;
            
        case    FTE_OBJ_FIELD_VALUE:
            {
                if (pObj->pAction->fCreateJSON != NULL)
                {
                    xRet = pObj->pAction->fCreateJSON(pObj, FTE_OBJ_FIELD_VALUE, (FTE_JSON_OBJECT_PTR _PTR_)&pValue);
                    if (xRet != FTE_RET_OK)
                    {
                        goto error;
                    }
                }
                else
                {
                    pValue = FTE_JSON_VALUE_createValue(pObj->pStatus->pValue);
                    if (pValue == NULL)
                    {
                        xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                        goto error;
                    }
                }

                xRet = FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_VALUE_STRING, pValue);
                if (xRet != FTE_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
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
                    xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                    goto error;
                }

                xRet = FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_TIME_STRING, pValue);
                if (xRet != FTE_RET_OK)
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
                    xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                    goto error;
                }

                xRet = FTE_JSON_OBJECT_setPair(pObject, FTE_JSON_OBJ_STATE_STRING, pValue);
                if (xRet != FTE_RET_OK)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    goto error;
                }
            }
            break;
        }
    }
    
    *ppObject = (FTE_JSON_OBJECT_PTR)pObject;    
    
    return  FTE_RET_OK;
    
error:
    if (pObject != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pObject);        
    }
    
    return  xRet;
}

FTE_RET FTE_OBJ_getChildCount
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32_PTR  pCount
 )
{
    ASSERT((pObj != NULL) && (pCount != NULL));
    
    *pCount = pObj->pStatus->nValueCount;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_getChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_OBJECT_PTR  _PTR_ pxChild
)
{
    ASSERT((pObj != NULL) && (pxChild != NULL));
    FTE_RET xRet;
    FTE_OBJECT_PTR  pChild = NULL;
    
    if ((pObj->pAction != NULL) && (pObj->pAction->fGetChild != NULL))
    {
        FTE_OBJECT_ID   xChildID = 0;
                    
        xRet = pObj->pAction->fGetChild(pObj, ulIndex, &xChildID);
        if (xRet != FTE_RET_OK)
        {
            return  xRet;
        }
        
        pChild = FTE_OBJ_get(xChildID);        
    }
    else
    {
        FTE_UINT32  ulGroupID = (pObj->pConfig->xCommon.nID & 0x00FF0000) >> 8;
        
        FTE_OBJECT_PTR  pChild = FTE_OBJ_getAt(ulGroupID, 0x0000FF00, ulIndex, FALSE);
    }
    
    if (pChild != NULL)
    {
        *pxChild = pChild;
                
        return  FTE_RET_OK;
    }

    return  FTE_RET_OBJECT_NOT_FOUND;
}

FTE_RET FTE_OBJ_attachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID xChildID
)
{
    ASSERT(pObj != NULL);
    
    if ((pObj->pAction != NULL) && (pObj->pAction->fAttachChild != NULL))
    {
        return  pObj->pAction->fAttachChild(pObj, xChildID);        
    }
    
    return  FTE_RET_NOT_SUPPORTED_FUNCTION;
}

FTE_RET FTE_OBJ_detachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID   xChildID
)
{
    ASSERT(pObj != NULL);
    
    if ((pObj->pAction != NULL) && (pObj->pAction->fDetachChild != NULL))
    {
        return  pObj->pAction->fDetachChild(pObj, xChildID);        
    }
    
    return  FTE_RET_NOT_SUPPORTED_FUNCTION;
}

FTE_RET FTE_OBJ_1WIRE_discovery(FTE_BOOL bSave, FTE_UINT32_PTR pCount)
{
#if FTE_1WIRE_SUPPORTED && FTE_DS18B20_SUPPORTED
    FTE_UINT32                     ulCount = 0;
    FTE_1WIRE_PTR               p1Wire;
    FTE_DS18B20_CREATE_PARAMS   xParams;
    FTE_UINT32                     nIndex;
    
    p1Wire = FTE_1WIRE_getFirst();
    while(p1Wire != 0)
    {
        FTE_CHAR    pFamilyName[32];
        
        memset(pFamilyName, 0, sizeof(pFamilyName));
        
        FTE_1WIRE_discovery(p1Wire);                

        xParams.nBUSID = p1Wire->pConfig->nID;
        
        FTE_UINT32  ulDevCount = 0;
        FTE_1WIRE_DEV_count(p1Wire, &ulDevCount);
        for(nIndex = 0 ; nIndex < ulDevCount ; nIndex++)
        {
            if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != FTE_RET_OK)
            {
                break;
            }
         
            FTE_1WIRE_getFamilyName(xParams.pROMCode[0], pFamilyName, sizeof(pFamilyName) - 1);
            if (strcasecmp(pFamilyName, "18B20") == 0)
            {
                if (!FTE_DS18B20_isExistROMCode(xParams.pROMCode) )
                {
                    FTE_OBJECT_PTR          pObj = FTE_DS18B20_create(&xParams);
                    if (pObj == NULL)
                    {
                        break;
                    }
                    
                    if (pObj->pAction->fInit != NULL);
                    {
                        pObj->pAction->fInit(pObj);
                    }
                    
                    if (pObj->pAction->fRun != NULL);
                    {
                        pObj->pAction->fRun(pObj);
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
    
    *pCount = ulCount;
    
    return  FTE_RET_OK;
#else
    return  FTE_RET_NOT_SUPPORTED_FUNCTION;
#endif
}

FTE_RET FT_OBJ_STAT_incSucceed
(
    FTE_OBJECT_STATISTICS_PTR pStatistics
)
{
    int i;
    const int nPartialCount = FTE_OBJ_CHECK_FAILURE_COUNT_MAX / sizeof(FTE_UINT32);
    
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
    
    return  FTE_RET_OK;
}

FTE_RET FT_OBJ_STAT_incFailed
(
    FTE_OBJECT_STATISTICS_PTR pStatistics
)
{
    int i;
    const int nPartialCount = FTE_OBJ_CHECK_FAILURE_COUNT_MAX / sizeof(FTE_UINT32);
   
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

    if (pStatistics->nPartialFail >= FTE_CFG_SYS_getAllowedFailureCount())
    {
        FTE_SYS_setUnstable();
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_showInfo
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_RET xRet;
    FTE_OBJECT_ID   xObjID;
    FTE_OBJECT_PTR  pObj;
    FTE_UINT32  i;
    TIME_STRUCT xTime;
    FTE_CHAR pTimeString[32];
    FTE_CHAR pValueString[16];
    FTE_CHAR pUnitString[8];
    
    if (nArgc < 1)
    {
        return  FTE_RET_INVALID_ARGUMENT;
    }
    
    xRet = FTE_strToHex(pArgv[0], &xObjID);
    if (xRet != FTE_RET_OK)
    {
        printf("Invalid OID[%s]\n", pArgv[0]);
        return  xRet;
    }

    pObj = FTE_OBJ_get(xObjID);                    
    if (pObj == NULL)
    {
        printf("Invalid OID[%08x]\n", xObjID);
        return  SHELL_EXIT_ERROR;
    }
    
    printf("%8s : %08x\n",  "ID",       pObj->pConfig->xCommon.nID);
    printf("%8s : %s\n",    "Type",     FTE_OBJ_typeString(pObj));
    printf("%8s : %s\n",    "Name",     pObj->pConfig->xCommon.pName);
    printf("%8s : %s\n",    "Status",   FTE_OBJ_IS_ENABLED(pObj)?"RUN":"STOP");                       
    
        
    FTE_VALUE_toString(pObj->pStatus->pValue, pValueString, sizeof(pValueString));
    FTE_VALUE_unit(pObj->pStatus->pValue, pUnitString, sizeof(pUnitString));
    FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTime);
    FTE_TIME_toStr(&xTime, pTimeString, sizeof(pTimeString));
    
    printf("%8s : %s\n",   "Value", pValueString);
    printf("%8s : %s\n",    "Unit", pUnitString);
    printf("%8s : %s\n",    "Time", pTimeString);
    
    if (pObj->pAction->fGetInterval != NULL)
    {
        FTE_UINT32  ulInterval = 0;
        pObj->pAction->fGetInterval(pObj, &ulInterval);
            
        printf("%8s : %d ms\n", "Update", ulInterval);
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
            FTE_CHAR        pTypeString[64];
            
            FTE_EVENT_type_string(pEvent->pConfig->xType, pTypeString, sizeof(pTypeString));
            printf("%8d : %08lx %16s %-10s\n", 
                   i, pEvent->pConfig->ulEPID, 
                   FTE_EVENT_CONDITION_string(pEvent->pConfig->xCondition),
                   pTypeString);   
        }
    }                            
    
    printf("%8s : %d\n", "Child", pObj->pStatus->nValueCount);
    
    for(i = 0 ; i < pObj->pStatus->nValueCount; i++)
    {
        FTE_OBJECT_PTR  pChild;
        xRet = FTE_OBJ_getChild(pObj, i, &pChild);
        if (xRet == FTE_RET_OK)
        {
            printf("%8s   %08x\n", "", pChild->pConfig->xCommon.nID);
        }
    }
    
    if (pObj->pAction->fGetStatistics != NULL)
    {
        FTE_OBJECT_STATISTICS    xStatistics;
        FTE_UINT32                 nRatio;
        
        pObj->pAction->fGetStatistics(pObj, &xStatistics);
        nRatio = (xStatistics.nTotalTrial - xStatistics.nTotalFail) * 10000 / xStatistics.nTotalTrial;
        
        printf("%8s : %3d.%02d%%\n", "Rate", nRatio/100, nRatio%100);
        printf("%8s : %d\n", "Total",   xStatistics.nTotalTrial);
        printf("%8s : %d\n", "Failed",  xStatistics.nTotalFail);
        printf("%8s : %d\n", "Partial", xStatistics.nPartialFail);
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_typeList
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_UINT32  ulIndex = 0;

    printf("%3s %-16s %s\n", "", "NAME", "VENDOR");
    for(FTE_UINT32 i = 0 ; i < FTE_OBJ_DESC_count() ; i++)
    {
        FTE_OBJECT_DESC_PTR pDesc = FTE_OBJ_DESC_getAt(i);

        if (pDesc->xFlags & FTE_OBJ_FLAG_DYNAMIC)
        {
            printf("%3d %-16s", ++ulIndex, pDesc->pName);
            
            if (pDesc->pVendor != NULL)
            {
                printf(" %s\n", pDesc->pVendor);
            }
            else
            {
                printf("\n");
            }
        }
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_run
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    ASSERT((nArgc > 0) || (pArgv != NULL));
    
    FTE_RET xRet;
    FTE_OBJECT_ID   xObjID;
    FTE_OBJECT_PTR  pObj;
    
    xRet = FTE_strToHex(pArgv[0], &xObjID);
    if (xRet != FTE_RET_OK)
    {
        printf("Invalid OID[%s]\n", pArgv[0]);
        return  xRet;
    }

    pObj = FTE_OBJ_get(xObjID);                    
    if (pObj == NULL)
    {
        printf("Invalid OID[%08x]\n", xObjID);
        return  FTE_RET_ERROR;
    }

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        printf("%08x is already run.\n", xObjID);
    }
    else
    {
        xRet = FTE_OBJ_activate(pObj, TRUE);
        if (xRet == FTE_RET_PARENT_IS_STOPPED)
        {
            printf("The parent object is stopped.\n");
        }
        else if (xRet != FTE_RET_OK)
        {
            printf("The object is not operated[%08x].\n");
        }
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_stop
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    ASSERT((nArgc > 0) && (pArgv != NULL));
    
    FTE_RET xRet;
    FTE_OBJECT_ID   xObjID;
    FTE_OBJECT_PTR  pObj;
    
    xRet = FTE_strToHex(pArgv[0], &xObjID);
    if (xRet != FTE_RET_OK)
    {
        printf("Invalid OID[%s]\n", pArgv[0]);
        return  xRet;
    }

    pObj = FTE_OBJ_get(xObjID);                    
    if (pObj == NULL)
    {
        printf("Invalid OID[%08x]\n", xObjID);
        return  FTE_RET_ERROR;
    }

    if (!FTE_OBJ_IS_ENABLED(pObj))
    {
        printf("%08x is already stopped.\n", xObjID);
    }
    else
    {
        FTE_OBJ_activate(pObj, FALSE);
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_list
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_UINT32 count = FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE);
    FTE_UINT32 i;

    printf("%-8s %-16s %-16s %8s %10s %7s %-s\n", 
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
                FTE_CHAR    pTimeString[64];
                FTE_CHAR    pValueString[32];
                FTE_CHAR    pUnitString[8];
                
                FTE_VALUE_toString(pObj->pStatus->pValue, pValueString, sizeof(pValueString));
                FTE_VALUE_unit(pObj->pStatus->pValue, pUnitString, sizeof(pUnitString));
                FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTime);
                FTE_TIME_toStr(&xTime, pTimeString, sizeof(pTimeString));
                printf(" %10s %-7s %s", pValueString, pUnitString, pTimeString);
            }
            
            if (pObj->pAction->fGetStatistics != NULL)
            {
                FTE_OBJECT_STATISTICS    xStatistics;
                FTE_UINT32                 nRatio;
                
                pObj->pAction->fGetStatistics(pObj, &xStatistics);
                nRatio = (xStatistics.nTotalTrial - xStatistics.nTotalFail) * 10000 / xStatistics.nTotalTrial;
                
                printf(" %3d.%02d%%(%d, %d, %d)", nRatio/100, nRatio%100, xStatistics.nTotalTrial, xStatistics.nTotalFail, xStatistics.nPartialFail);
            }
            printf("\n");
            
        }
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_add
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_RET xRet;
    FTE_OBJECT_PTR  pObj;    
    
    for(FTE_UINT32 i = 0 ; i < FTE_OBJ_DESC_count() ; i++)
    {
        FTE_OBJECT_DESC_PTR pDesc = FTE_OBJ_DESC_getAt(i);

        if (strcasecmp(pArgv[0], pDesc->pName) == 0)
        {
            if (pDesc->xFlags & FTE_OBJ_FLAG_DYNAMIC)
            {
                if (pDesc->xFlags & FTE_OBJ_FLAG_GUS)
                {
                    FTE_GUS_MODEL_INFO_PTR  pInfo = pDesc->pOpts;

                    ASSERT(pInfo != NULL);
                    if(pArgv[1] != NULL)
					{
                    	if (pInfo->fCreate != NULL)
                    	{
                        	xRet = pInfo->fCreate(pArgv[1], (FTE_OBJECT_PTR _PTR_)&pObj);
                        	if (xRet == FTE_RET_OK)
                        	{
                            	printf("The object[%08x] is created successfully.\n", pObj->pConfig->xCommon.nID);
                        	}
                        	else
                        	{
                            	printf("The object creation is failed[%08x].\n", xRet);
                        	}
                    	}
                    	else
                    	{
                        	printf("The type[%s] does not support the dynamic creation.\n", pArgv[0]);
                    	}
					}
					else
					{
					  printf("The type[%s] does not insert slave ID.\n", pArgv[0]);
					}
                }
                else
                {
                    if (pDesc->f_create != NULL)
                    {
                        pDesc->f_create(NULL, &pObj);
                    }
                    else
                    {
                        printf("The type[%s] does not support the dynamic creation.\n", pArgv[0]);
                    }
                }
            }
            else
            {
                printf("The type[%s] does not support the dynamic creation.\n", pArgv[0]);
            }
        }
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_del
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    ASSERT(pArgv != NULL);
    
    FTE_RET xRet;
    FTE_OBJECT_ID   xObjID;
    FTE_OBJECT_PTR  pObj;
    FTE_UINT32      ulChildCount;
    
    if (nArgc < 1)
    {
        return  FTE_RET_INVALID_ARGUMENT;
    }
    
    if (strcasecmp(pArgv[0], "all") == 0)
    {
        FTE_UINT32  ulCount;
        FTE_OBJECT_ID_PTR  pObjIDs;
        
        ulCount = FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE);        
        if (ulCount == 0)
        {
            return  FTE_RET_OK;
        }
        
        pObjIDs = (FTE_OBJECT_ID_PTR)FTE_MEM_allocZero(sizeof(FTE_OBJECT_ID) * ulCount);
        if (pObjIDs == NULL)
        {
            return FTE_RET_INSUFFICIENT_MEMORY;
        }
        
        xRet = FTE_OBJ_getIDList(FTE_OBJ_TYPE_UNKNOWN, 0, pObjIDs, ulCount, &ulCount);
        if (xRet == FTE_RET_OK)
        {
            for(FTE_INT32   i = 0 ; i < ulCount ; i++)
            {
                pObj = FTE_OBJ_get(pObjIDs[i]);
                if (pObj != NULL)
                {
                    FTE_OBJ_destroy(pObj);
                }
            }
        }

        FTE_MEM_free(pObjIDs);

        FTE_CFG_clearObject();
    }
    else
    {
        xRet = FTE_strToHex(pArgv[0], &xObjID);
        if (xRet != FTE_RET_OK)
        {
            printf("Invalid OID[%s]\n", pArgv[0]);
            return  xRet;
        }

        pObj = FTE_OBJ_get(xObjID);                    
        if (pObj == NULL)
        {
            printf("Invalid OID[%08x]\n", xObjID);
            return  FTE_RET_ERROR;
        }

        
        if (!(pObj->pConfig->xCommon.xFlags & FTE_OBJ_CONFIG_FLAG_DYNAMIC))
        {
            return  FTE_RET_STATIC_OBJECT;
        }
        
        xRet = FTE_OBJ_getChildCount(pObj, &ulChildCount);
        if (xRet != FTE_RET_OK)
        {
            printf("Internal unknown error[%08x]!\n", xRet);
            return  xRet;
        }
        
        for(FTE_INT32 i = (FTE_INT32)ulChildCount ; i > 0; i--)
        {
            FTE_OBJECT_PTR  pChild;
            xRet = FTE_OBJ_getChild(pObj, i - 1, &pChild);
            if (xRet == FTE_RET_OK)
            {
                xRet = FTE_CFG_OBJ_free(pChild->pConfig->xCommon.nID);
                if (xRet != FTE_RET_OK)
                {
                    printf("The object[%08x] destroy failed from storage.\n", pChild->pConfig->xCommon.nID);
                }
                
                xRet = FTE_OBJ_destroy(pChild);
                if (xRet != FTE_RET_OK)
                {
                    printf("The object[%08x] destroy failed.\n", pChild->pConfig->xCommon.nID);
                }
            }
        }
        
        xRet = FTE_CFG_OBJ_free(xObjID);
        if (xRet != FTE_RET_OK)
        {
            printf("The object[%08x] destroy failed from storage.\n", pObj->pConfig->xCommon.nID);
        }
        else
        {
            FTE_CFG_save(TRUE);
        }
        
        xRet = FTE_OBJ_destroy(pObj);
        if (xRet != FTE_RET_OK)
        {
            printf("The object[%08x] destroy failed.\n", xObjID);
            return  xRet;
        }
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_OBJ_CMD_set
(
    FTE_INT32       nArgc,
    FTE_CHAR_PTR    pArgv[]
)
{
    ASSERT(pArgv != NULL);
    
    FTE_RET xRet;
    FTE_OBJECT_ID   xObjID;
    FTE_OBJECT_PTR  pObj;
    FTE_VALUE       xValue;

    if (nArgc < 3)
    {
        return  FTE_RET_INVALID_ARGUMENT;
    }
    
    xRet = FTE_strToHex(pArgv[0], &xObjID);
    if (xRet != FTE_RET_OK)
    {
        printf("Invalid OID[%s]\n", pArgv[0]);
        return  xRet;
    }

    pObj = FTE_OBJ_get(xObjID);                    
    if (pObj == NULL)
    {
        printf("Invalid OID[%08x]\n", xObjID);
        return  FTE_RET_ERROR;
    }

    if (strcasecmp(pArgv[1], "value") == 0)
    {
        FTE_VALUE_init(&xValue, FTE_OBJ_getValueType(pObj));
        if (FTE_VALUE_set(&xValue, pArgv[2]) == FTE_RET_OK)
        {
            xRet = FTE_OBJ_setValue(pObj, &xValue);
        }        
    }
    else if (strcasecmp(pArgv[1], "name") == 0)
    {
        xRet = FTE_OBJ_setName(pObj, pArgv[2], strlen(pArgv[2]));
    }
    else if (strcasecmp(pArgv[1], "interval") == 0)
    {
        FTE_UINT32  ulValue;
    
        xRet = FTE_strToUINT32(pArgv[2], &ulValue);
        if (xRet != FTE_RET_OK)
        {
            printf("Invalid update interval[%s]\n", pArgv[2]);
            return  xRet;
        }

        if (pObj->pAction->fSetInterval == NULL)
        {
            printf("Not supported changing the update interval.\n");
            return  FTE_RET_ERROR;
        }
        
        if (pObj->pAction->fSetInterval(pObj, ulValue) != FTE_RET_OK)
        {
            printf("Object failed to change the update interval\n"); 
            return  FTE_RET_ERROR;
        }
    
        return  FTE_CFG_OBJ_save(pObj);
    }
    else 
    {
        if (FTE_OBJ_CLASS(pObj) != FTE_OBJ_CLASS_DI)
        {
            return  FTE_RET_NOT_SUPPORTED_FUNCTION;
        }
        
        if (strcasecmp(pArgv[1], "delay") == 0)
        {
            FTE_UINT32  ulValue;
            
            xRet = FTE_strToUINT32(pArgv[2], &ulValue);
            if (xRet != FTE_RET_OK)
            {
                printf("Invalid delay time[%s]\n", pArgv[2]);
                return  xRet;
            }
            
            FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;

            pConfig->ulDelay = ulValue;
            
            xRet = FTE_CFG_OBJ_save(pObj);
        }
        else if (strcasecmp(pArgv[1], "hold") == 0)
        {
            FTE_UINT32  ulValue;
            
            xRet = FTE_strToUINT32(pArgv[2], &ulValue);
            if (xRet != FTE_RET_OK)
            {
                printf("Invalid delay time[%s]\n", pArgv[2]);
                return  xRet;
            }
            
            FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;

            pConfig->ulHold = ulValue;
            
            xRet = FTE_CFG_OBJ_save(pObj);
        }
        else if (strcasecmp(pArgv[1], "reverse") == 0)
        {
            if (strcasecmp(pArgv[2], "on") == 0)
            {
                if (!FTE_OBJ_FLAG_isSet(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE))
                {                        
                    FTE_OBJ_FLAG_set(pObj, FTE_OBJ_CONFIG_FLAG_REVERSE);
                    pObj->pStatus->pValue->xData.bValue = !pObj->pStatus->pValue->xData.bValue;
                    
                    FTE_CFG_OBJ_save(pObj);
                }

            }
            else if (strcasecmp(pArgv[2], "off") == 0)
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
                return  FTE_RET_ERROR;
            }                            
        }
        else
        {
            return  FTE_RET_NOT_SUPPORTED_FUNCTION;
        }
    }
    
    return  xRet;
}

static
FTE_SHELL_CMD   pShellCmds[] = 
{
    {
        .pStrCmd    = "list",
        .fCmd       = FTE_OBJ_CMD_list,
        .pUsage     = "list\n"\
                      "        Show object list"
    },
    {
        .pStrCmd    = "info",
        .fCmd       = FTE_OBJ_CMD_showInfo,
        .pUsage     = "info <OID>\n"\
                      "        Show object info."
    },
    {
        .pStrCmd    = "add",
        .fCmd       = FTE_OBJ_CMD_add,
        .pUsage     = "add <TYPE> <NANE>\n"\
                      "        Create new object."
    },
    {
        .pStrCmd    = "del",
        .fCmd       = FTE_OBJ_CMD_del,
        .pUsage     = "del <OID>\n"\
                      "        Destroy object."
    },
    {
        .pStrCmd    = "run",
        .fCmd       = FTE_OBJ_CMD_run,
        .pUsage     = "run <OID>\n"\
                      "        Executes the object."
    },
    {
        .pStrCmd    = "stop",
        .fCmd       = FTE_OBJ_CMD_stop,
        .pUsage     = "stop <OID>\n"\
                      "        Stop to object."
    },
    {
        .pStrCmd    = "set",
        .fCmd       = FTE_OBJ_CMD_set,
        .pUsage     = "set <OID> <FIELD> <VALUE>\n"\
                      "        Set a field to the object."
    },
    {
        .pStrCmd    = "type",
        .fCmd       = FTE_OBJ_CMD_typeList,
        .pUsage     = "type\n"\
                      "        Show object type list."
    }            
};

FTE_INT32   FTE_OBJ_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_RET     xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    if (!bPrintUsage)
    {
        if (nArgc > 1)
        {
            bPrintUsage = TRUE;

            for(FTE_INT32   i = 0 ; i < sizeof(pShellCmds) / sizeof(FTE_SHELL_CMD) ; i++)
            {
                if (strcasecmp(pArgv[1], pShellCmds[i].pStrCmd) == 0)
                {
                    xRet = pShellCmds[i].fCmd(nArgc - 2, pArgv + 2);
                    bPrintUsage = FALSE;
                }            
            }        
        }
        else
        {
                xRet = FTE_OBJ_CMD_list(0, NULL);
        }
    }
    
    if (bPrintUsage || (xRet !=FTE_RET_OK))
    {
        if (bShortHelp)
        {
            printf ("%s [<command>]\n", pArgv[0]);
        }
        else
        {
            printf ("Usage: %s [<command>]\n", pArgv[0]);
            printf ("  Commands:\n");
            for(FTE_INT32   i = 0 ; i < sizeof(pShellCmds) / sizeof(FTE_SHELL_CMD) ; i++)
            {
                printf("    %s\n", pShellCmds[i].pUsage);
            }        
            
            printf ("  Parameters:\n");
            printf ("    <OID> = Object ID\n");
            printf ("          all : Destroy all object\n");
            printf ("    <TYPE> = Target object type\n");
            printf ("    <DELAY TIME> = msecs\n");
            printf ("    <HOLE TIME> = msecs\n");
            printf ("    <FIELD> = Object field\n");
            printf ("         name : Object name\n");
            printf ("        value : Set the value to the object\n");
            printf ("     interval : Set the data gattering time to the object\n");
            printf ("        delay : Set the activation delay time to the DI\n");
            printf ("         hold : Set the state held time to the DI\n");
            printf ("    <OP> = operation command\n");
            printf ("           on : operation active\n");
            printf ("          off : operation inactive\n");
        }
    }
    
    return   xRet;
}