#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"
 
static 
FTE_OBJECT_PTR   _FTE_RL_getObject(FTE_OBJECT_ID nID);

static 
FTE_RET        _FTE_RL_init(FTE_OBJECT_PTR pObj);

static 
FTE_RET        _FTE_RL_setValue(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);

static  FTE_LIST            _xObjList = { 0, NULL, NULL};
static  FTE_OBJECT_ACTION   _xAction = 
{
    .fInit  = _FTE_RL_init,
    .fSet   = _FTE_RL_setValue,
};

FTE_RET FTE_RL_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT(pObj != NULL);
    
    FTE_RL_CONFIG_PTR   pConfig = (FTE_RL_CONFIG_PTR)pObj->pConfig;
    FTE_RL_STATUS_PTR   pStatus = (FTE_RL_STATUS_PTR)pObj->pStatus;
    
    pStatus->pGPIOOpen = FTE_GPIO_get(pConfig->nGPIOOpen);
    if (pStatus->pGPIOOpen == NULL)
    {
        goto error;
    }
        
    if (FTE_GPIO_attach(pStatus->pGPIOOpen, pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }
    
    pStatus->pGPIOClose = FTE_GPIO_get(pConfig->nGPIOClose);
    if (pStatus->pGPIOClose == NULL)
    {
        goto error;
    }
        
    if (FTE_GPIO_attach(pStatus->pGPIOClose, pConfig->xCommon.nID) != MQX_OK)
    {
        FTE_GPIO_detach(pStatus->pGPIOOpen);
        goto error;
    }
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_xAction;
    
    if (_FTE_RL_init(pObj) != MQX_OK)
    {
        FTE_GPIO_detach(pStatus->pGPIOOpen);
        FTE_GPIO_detach(pStatus->pGPIOClose);
        
        goto error;
    }
    
    FTE_LIST_pushBack(&_xObjList, pObj);
    
    return  MQX_OK;
error:
   
    return  MQX_ERROR;
}

FTE_RET FTE_RL_detach
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
   

    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

/******************************************************************************/
FTE_UINT32     FTE_RL_count(void)
{
    return  FTE_LIST_count(&_xObjList);
}

FTE_RET   FTE_RL_getInitState
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pState
)
{
    ASSERT((pObj != NULL) && (pState != NULL));
    
    *pState = FTE_FLAG_IS_SET(pObj->pConfig->xCommon.xFlags, FTE_RL_CONFIG_INIT_CLOSE);
    
    return  MQX_OK;
}

FTE_RET   FTE_RL_setInitState
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nState
)
{
    ASSERT(pObj != NULL);
    if (nState)
    {
        FTE_OBJ_FLAG_set(pObj, FTE_RL_CONFIG_INIT_CLOSE);
    }
    else
    {
        FTE_OBJ_FLAG_clear(pObj, FTE_RL_CONFIG_INIT_CLOSE);
    }
    
    FTE_OBJ_save(pObj);
    
    return  MQX_OK;
}

FTE_RET   FTE_RL_setValue
(
    FTE_OBJECT_ID   nID, 
    FTE_BOOL        bValue
)
{
    FTE_VALUE       xValue;
    FTE_OBJECT_PTR  pObj = _FTE_RL_getObject(nID);
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }

    if (FTE_VALUE_initDIO(&xValue, bValue) == MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    _FTE_RL_setValue(pObj, &xValue);
   
    return  MQX_OK;    
}


FTE_BOOL     fte_rl_is_closed
(
    FTE_OBJECT_ID   nID
)
{
    FTE_BOOL bValue;
    FTE_OBJECT_PTR pObj = _FTE_RL_getObject(nID);
    if (pObj == NULL)
    {
        return  FALSE;
    }

    FTE_VALUE_getDIO(((FTE_RL_STATUS_PTR)pObj->pStatus)->xCommon.pValue, &bValue);    
    
    return  bValue;
}

FTE_RET   FTE_RL_setPermanent
(
    FTE_OBJECT_ID   nID
)
{
    FTE_OBJECT_PTR pObj = _FTE_RL_getObject(nID);
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }

    FTE_VALUE_getULONG(((FTE_RL_STATUS_PTR)pObj->pStatus)->xCommon.pValue, &((FTE_RL_CONFIG_PTR)pObj->pConfig)->nInitClosed);
    FTE_OBJ_save(pObj);
    
    return  MQX_OK;    
   
}

/******************************************************************************/
FTE_OBJECT_PTR  _FTE_RL_getObject
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

FTE_RET   _FTE_RL_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_RL_CONFIG_PTR   pConfig = (FTE_RL_CONFIG_PTR)pObj->pConfig;
    FTE_RL_STATUS_PTR   pStatus = (FTE_RL_STATUS_PTR)pObj->pStatus;
    
    pStatus->pGPIOOpen = FTE_GPIO_get(pConfig->nGPIOOpen);
    pStatus->pGPIOClose= FTE_GPIO_get(pConfig->nGPIOClose);
    
    FTE_GPIO_attach(pStatus->pGPIOOpen, pConfig->xCommon.nID);
    FTE_GPIO_attach(pStatus->pGPIOClose, pConfig->xCommon.nID);

    FTE_RL_setValue(pObj->pConfig->xCommon.nID, pConfig->nInitClosed);
    
    return  MQX_OK;
}

FTE_RET _FTE_RL_setValue
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT(pObj != NULL);
    
    FTE_RL_CONFIG_PTR   pConfig = (FTE_RL_CONFIG_PTR)pObj->pConfig;
    FTE_RL_STATUS_PTR   pStatus = (FTE_RL_STATUS_PTR)pObj->pStatus;
    
    if (pValue->xData.bValue)
    {
        if (FTE_GPIO_setValue(pStatus->pGPIOClose, TRUE) != MQX_OK)
        {
            goto    error;
        }
        _time_delay(50);
        if (FTE_GPIO_setValue(pStatus->pGPIOClose, FALSE) != MQX_OK)
        {
            goto    error;
        }
    }
    else
    {
        if (FTE_GPIO_setValue(pStatus->pGPIOOpen, TRUE) != MQX_OK)
        {
            goto    error;
        }
        _time_delay(50);
        if (FTE_GPIO_setValue(pStatus->pGPIOOpen, FALSE) != MQX_OK)
        {
            goto    error;
        }
    }
 
    FTE_VALUE_setDIO(pStatus->xCommon.pValue, pValue->xData.bValue);
    if (pConfig->nLED != 0)
    {
        FTE_LED_setValue(pConfig->nLED, pValue->xData.bValue);
    }
    
    return   MQX_OK;
    
error:
    pStatus->xCommon.xFlags = FTE_FLAG_CLR(pStatus->xCommon.xFlags, FTE_OBJ_STATUS_FLAG_VALID);
    return  MQX_ERROR;
}

FTE_INT32   FTE_RL_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{ /* Body */
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        FTE_UINT32     nID;
        FTE_BOOL     switch_on = FALSE;
        FTE_BOOL     permanent = FALSE;
        
        if (nArgc > 3)
        {
            bPrintUsage = TRUE;
            goto error;
        }
        
        if (nArgc > 2)
        {
            if (strcasecmp(pArgv[2], "close") == 0)
            {
                switch_on = TRUE;
            }
            else if (strcasecmp(pArgv[2], "close+") == 0)
            {
                switch_on = TRUE;
                permanent = TRUE;
            }
            else if (strcasecmp(pArgv[2], "open") == 0)
            {
                switch_on = FALSE;
            }
            else if (strcasecmp(pArgv[2], "open+") == 0)
            {
                switch_on = FALSE;
                permanent = TRUE;
            }
            else
            {
                bPrintUsage = TRUE;
                goto error;
            }
        }
        
        if (nArgc > 1)
        {
            if (!Shell_parse_number(pArgv[1], &nID))
            {
                bPrintUsage = TRUE;
                goto error;
            }
            
            if (FTE_ID_TYPE(nID) == 0)
            {
                nID = MAKE_ID(FTE_OBJ_TYPE_RL, nID + 1);
            }        
            
        }
        
        switch(nArgc)
        {
        case    1:
            {
                FTE_OBJECT_PTR  pObj;
                FTE_LIST_ITERATOR   xIter;
               
                if (FTE_RL_count() == 0)
                {
                    printf("The RL interface is not exists.\n");
                    break;
                }
                
                printf("    %08s %16s %6s %20s\n", "ID", "NAME", "STATUS", "TIME");
                FTE_LIST_ITER_init(&_xObjList, &xIter);
                while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
                {
                    char    pValueString[32];
                    char    pBuff[32];
                    TIME_STRUCT xTimeStamp;
                    
                    
                    FTE_VALUE_getTimeStamp(((FTE_RL_STATUS_PTR)pObj->pStatus)->xCommon.pValue, &xTimeStamp);
                    FTE_TIME_toStr(&xTimeStamp, pBuff, sizeof(pBuff));
                    FTE_VALUE_toString(((FTE_RL_STATUS_PTR)pObj->pStatus)->xCommon.pValue, pValueString, sizeof(pValueString));
                    printf("%08x %16s %6s %20s\n", 
                           pObj->pConfig->xCommon.nID,
                           pObj->pConfig->xCommon.pName,
                           pValueString,
                           pBuff);
                }
            }
            break;
            
        case    2:
            {
                FTE_OBJECT_PTR  pObj;
                char            pBuff[32];
                char            pValueString[32];
                TIME_STRUCT     xTimeStamp;
                    
                pObj = FTE_OBJ_get(nID); 
                if (pObj == NULL)
                {
                    printf("Error : DI[%s] is not exists.\n", pArgv[1]);
                    goto error;
                }
                
                FTE_VALUE_getTimeStamp(((FTE_RL_STATUS_PTR)pObj->pStatus)->xCommon.pValue, &xTimeStamp);
                FTE_TIME_toStr(&xTimeStamp, pBuff, sizeof(pBuff));
                FTE_VALUE_toString(((FTE_RL_STATUS_PTR)pObj->pStatus)->xCommon.pValue, pValueString, sizeof(pValueString));
                
                printf("                ID : %08x\n", pObj->pConfig->xCommon.nID);
                printf("              Name : %16s\n", pObj->pConfig->xCommon.pName);
                printf("            Status : %6s\n", pValueString);
                printf(" Last Changed Time : %20s\n", pBuff);
            }
            break;
            
         case    3:               
            {
                FTE_RL_setValue(nID, switch_on);
                if (permanent)
                {
                    FTE_RL_setPermanent(nID);
                }
            }
            break;
            
                
        default:
            bPrintUsage = TRUE;
        }
    }
                
error:    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<index>] [ open | close ]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<index>]  [ open | close ]\n", pArgv[0]);
            printf("        index - index of digital output\n");
            printf("        open  - open the switch\n");
            printf("        close - close the switch\n");
        }
    }
    return   xRet;
}
