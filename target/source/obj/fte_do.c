#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h" 
 
static  
FTE_OBJECT_PTR  FTE_DO_getObject(FTE_OBJECT_ID nID);
static 
FTE_RET  FTE_DO_init(FTE_OBJECT_PTR pObj);
static 
FTE_RET  _do_set_value(FTE_OBJECT_PTR pObj, FTE_VALUE_PTR pValue);
 
static  
FTE_LIST    _xObjList = { 0, NULL, NULL};

static  
FTE_OBJECT_ACTION   _xAction = 
{
    .fInit  = FTE_DO_init,
    .fSet   = _do_set_value
};

FTE_RET FTE_DO_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT(pObj != NULL);
    
    FTE_DO_CONFIG_PTR   pConfig = (FTE_DO_CONFIG_PTR)pObj->pConfig;
    FTE_DO_STATUS_PTR   pStatus = (FTE_DO_STATUS_PTR)pObj->pStatus;
    
    pStatus->xCommon.nValueCount = 1;
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
        
    if (FTE_GPIO_attach(pStatus->pGPIO, pConfig->xCommon.nID) != FTE_RET_OK)
    {
        goto error;
    }

    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_xAction;

    if (FTE_DO_init(pObj) != FTE_RET_OK)
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

FTE_RET FTE_DO_detach
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

/******************************************************************************/
FTE_UINT32  FTE_DO_count(void)
{
    return  FTE_LIST_count(&_xObjList);
}

FTE_RET   FTD_DO_getValue
(
    FTE_OBJECT_ID   nID, 
    FTE_BOOL_PTR    pbValue
)
{
    ASSERT(pbValue != NULL);
    
    FTE_OBJECT_PTR pObj = FTE_DO_getObject(nID);
    if (pObj == NULL)
    {
        return  FALSE;
    }

    FTE_VALUE_getDIO(pObj->pStatus->pValue, pbValue);
        
    return  FTE_RET_OK;    
}

FTE_RET   FTE_DO_setValue
(
    FTE_OBJECT_ID   nID, 
    FTE_BOOL        bValue
)
{
    FTE_VALUE   xValue;
    FTE_OBJECT_PTR pObj = FTE_DO_getObject(nID);
    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }

    FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
    FTE_VALUE_setDIO(&xValue, bValue);
    
    _do_set_value(pObj, &xValue);
   
    return  FTE_RET_OK;    
}

FTE_RET   FTE_DO_setPermanent
(
    FTE_OBJECT_ID  nID
)
{
    FTE_OBJECT_PTR pObj = FTE_DO_getObject(nID);
    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }

    if (((FTE_DO_STATUS_PTR)pObj->pStatus)->xCommon.pValue->xData.bValue)
    {
        FTE_OBJ_FLAG_set(pObj, FTE_DO_CONFIG_INIT_ON);
    }
    else
    {
        FTE_OBJ_FLAG_clear(pObj, FTE_DO_CONFIG_INIT_ON);
    }
    
    FTE_CFG_OBJ_save(pObj);
    
    return  FTE_RET_OK;    
   
}

FTE_RET   FEE_DO_getInitState
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pState
)
{
    ASSERT(pObj != NULL);
    *pState = FTE_FLAG_IS_SET(pObj->pConfig->xCommon.xFlags, FTE_DO_CONFIG_INIT_ON);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_DO_setInitState
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nState
)
{
    ASSERT(pObj != NULL);
    if (nState)
    {
        FTE_OBJ_FLAG_set(pObj, FTE_DO_CONFIG_INIT_ON);
    }
    else
    {
        FTE_OBJ_FLAG_clear(pObj, FTE_DO_CONFIG_INIT_ON);
    }
    
    FTE_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

/******************************************************************************/
FTE_OBJECT_PTR  FTE_DO_getObject
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

FTE_RET   FTE_DO_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    FTE_VALUE           xValue;
    FTE_DO_CONFIG_PTR   pConfig = (FTE_DO_CONFIG_PTR)pObj->pConfig;
    
    FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
    FTE_VALUE_setDIO(&xValue, FTE_FLAG_IS_SET(pConfig->xCommon.xFlags, FTE_DO_CONFIG_INIT_ON));
    _do_set_value(pObj, &xValue);
    
    return  FTE_RET_OK;
}

FTE_RET _do_set_value
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT(pObj != NULL);
    
    FTE_DO_CONFIG_PTR   pConfig = (FTE_DO_CONFIG_PTR)pObj->pConfig;
    FTE_DO_STATUS_PTR   pStatus = (FTE_DO_STATUS_PTR)pObj->pStatus;
    
    
    if (FTE_VALUE_isValid(pStatus->xCommon.pValue) && (pStatus->xCommon.pValue->xData.bValue == pValue->xData.bValue))
    {
            return   FTE_RET_OK;
    }
    
    if (FTE_GPIO_setValue(pStatus->pGPIO,  pValue->xData.bValue) != FTE_RET_OK)
    {
        goto    error;
    }
    FTE_VALUE_setDIO(pStatus->xCommon.pValue, pValue->xData.bValue);

    if (pConfig->nLED != 0)
    {
        FTE_LED_setValue(pConfig->nLED, pValue->xData.bValue);
    }
                
    FTE_OBJ_wasUpdated(pObj);

    return   FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_INT32   FTE_DO_SHELL_cmd
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
        FTE_BOOL     permanent = FALSE;
        FTE_BOOL     switch_on = FALSE;
        
        if (nArgc > 3)
        {
            bPrintUsage = TRUE;
            goto error;
        }
        
        if (nArgc > 2)
        {
            if (strcasecmp(pArgv[2], "on") == 0)
            {
                switch_on = TRUE;
            }
            else if (strcasecmp(pArgv[2], "on+") == 0)
            {
                permanent = TRUE;
                switch_on = TRUE;
            }
            else if (strcasecmp(pArgv[2], "off") == 0)
            {
                switch_on = FALSE;
            }
            else if (strcasecmp(pArgv[2], "off+") == 0)
            {
                permanent = TRUE;
                switch_on = FALSE;
            }
            else
            {
                bPrintUsage = TRUE;
                goto error;
            }
        }
        
        if (nArgc > 1)
        {
            if (!Shell_parse_hexnum(pArgv[1], &nID))
            {
                bPrintUsage = TRUE;
                goto error;
            }
            
            if (FTE_ID_TYPE(nID) == 0)
            {
                nID = MAKE_ID(FTE_OBJ_TYPE_DO, nID);
            }        
            
        }
        
        switch(nArgc)
        {
        case    1:
            {
                FTE_OBJECT_PTR      pObj;
                FTE_LIST_ITERATOR   xIter;
                
                FTE_UINT32 i;
                
                if (FTE_LIST_count(&_xObjList) == 0)
                {
                    printf("The DO interface is not exists.\n");
                    break;
                }
                
                printf("    %08s %16s %6s %20s\n", "ID", "NAME", "STATUS", "TIME");
                
                FTE_LIST_ITER_init(&_xObjList, &xIter);
                while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
                {
                    char    pBuff[32];
                    char    pValueString[32];
                    
                    FTE_VALUE_toString(((FTE_DO_STATUS_PTR)pObj->pStatus)->xCommon.pValue, pValueString, sizeof(pValueString));
                    FTE_TIME_toStr(&((FTE_DO_STATUS_PTR)pObj->pStatus)->xCommon.pValue->xTimeStamp, pBuff, sizeof(pBuff));
                    printf("%2d: %08x %16s %6s %20s\n", i, 
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
                char    pBuff[32];
                char    pValueString[32];
                        
                pObj = FTE_OBJ_get(nID);
                if (pObj == NULL)
                {
                    printf("Error : DO[%s] is not exists.\n", pArgv[1]);
                    goto error;
                }

                FTE_VALUE_toString(((FTE_DO_STATUS_PTR)pObj->pStatus)->xCommon.pValue, pValueString, sizeof(pValueString));
                FTE_TIME_toStr(&((FTE_DO_STATUS_PTR)pObj->pStatus)->xCommon.pValue->xTimeStamp, pBuff, sizeof(pBuff));

                printf("                ID : %08x\n", pObj->pConfig->xCommon.nID);
                printf("              Name : %16s\n", pObj->pConfig->xCommon.pName);
                printf("    Initial Status : %6s\n", FTE_FLAG_IS_SET(pObj->pConfig->xCommon.xFlags, FTE_DO_CONFIG_INIT_ON)?"ON":"OFF");
                printf("    Current Status : %6s\n", pValueString);
                printf(" Last Changed Time : %20s\n", pBuff);
            }
            break;
            
         case    3:               
            {
                FTE_DO_setValue(nID, switch_on);
                if (permanent)
                {
                    FTE_DO_setPermanent(nID);
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
