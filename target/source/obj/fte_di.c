#include "fte_target.h"
#include "fte_net.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_value.h"
#include "sys/fte_sys_timer.h"

static  _mqx_uint   _FTE_DI_init(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _FTE_DI_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _FTE_DI_stop(FTE_OBJECT_PTR pObj);
static  void        _di_int(pointer);

void _FTE_DI_ISR(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);

static  FTE_LIST                _xObjList = { 0, NULL, NULL}; 
static  FTE_OBJECT_ACTION       _xAction = 
{
    .f_init             =   _FTE_DI_init,
    .f_run              =   _FTE_DI_run,
    .f_stop             =   _FTE_DI_stop
};

_mqx_uint FTE_DI_attach(FTE_OBJECT_PTR pObj)
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
    
    if (FTE_GPIO_attach(pStatus->pGPIO, pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }

    
    if (_FTE_DI_init(pObj) != MQX_OK)
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

_mqx_uint FTE_DI_detach (FTE_OBJECT_PTR pObj)
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

FTE_OBJECT_PTR _di_get_object(FTE_OBJECT_ID nID)
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

uint_32     FTE_DI_count(void)
{
    return  FTE_LIST_count(&_xObjList);
}

_mqx_uint   FTE_DI_getValue(FTE_OBJECT_ID  nID, uint_32_ptr pValue)
{
    ASSERT(pValue != NULL);
    
    FTE_OBJECT_PTR pObj = _di_get_object(nID);
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }

    *pValue = pObj->pStatus->pValue->xData.bValue;
        
    return  MQX_OK;    
}

boolean     FTE_DI_isActive(FTE_OBJECT_ID  nID)
{
    FTE_OBJECT_PTR pObj = _di_get_object(nID);
    if (pObj == NULL)
    {
        return  FALSE;
    }

    return  pObj->pStatus->pValue->xData.bValue;    
}

_mqx_uint   FTE_DI_INT_lock(FTE_OBJECT_ID  nID)
{
    FTE_OBJECT_PTR pObj = _di_get_object(nID);
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    return  FTE_GPIO_INT_setEnable(((FTE_DI_STATUS_PTR)pObj->pStatus)->pGPIO, FALSE);    
}

_mqx_uint   FTE_DI_INT_unlock(FTE_OBJECT_ID  nID)
{
    FTE_OBJECT_PTR pObj = _di_get_object(nID);
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    if (pObj->pStatus->pValue->xData.bValue == TRUE)
    {
        FTE_GPIO_INT_setPolarity(((FTE_DI_STATUS_PTR)pObj->pStatus)->pGPIO, FTE_LWGPIO_INT_LOW);
    }
    else
    {
        FTE_GPIO_INT_setPolarity(((FTE_DI_STATUS_PTR)pObj->pStatus)->pGPIO, FTE_LWGPIO_INT_HIGH);
    }
    
    return  FTE_GPIO_INT_setEnable(((FTE_DI_STATUS_PTR)pObj->pStatus)->pGPIO, TRUE);
}

_mqx_uint   fte_di_int_global_lock(void)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            FTE_DI_INT_lock(((FTE_DI_CONFIG_PTR)pObj->pConfig)->xCommon.nID);
        }
    }
    
    return  MQX_OK;
}

_mqx_uint   fte_di_int_global_unlock(void)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            FTE_DI_INT_unlock(((FTE_DI_CONFIG_PTR)pObj->pConfig)->xCommon.nID);
        }
    }
    
    return  MQX_OK;
}

/******************************************************************************/
_mqx_uint   _FTE_DI_init(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    uint_32             nValue;
    FTE_DI_CONFIG_PTR   pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
    FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;
    
    if (FTE_GPIO_getValue(pStatus->pGPIO, &nValue) != MQX_OK)
    {
        return  MQX_ERROR;
    }

    FTE_VALUE_setDIO(pStatus->xCommon.pValue, nValue);

    if (pConfig->nLED != 0)
    {
        FTE_LED_setValue(pConfig->nLED, pStatus->xCommon.pValue->xData.bValue);
    }
    
    return  MQX_OK;
}

_mqx_uint   _FTE_DI_run(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
        
    FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;

    FTE_GPIO_setISR(pStatus->pGPIO, _di_int, 0);
    FTE_GPIO_INT_init(pStatus->pGPIO, 3, 0, TRUE);

    return  MQX_OK;
}

static _mqx_uint    _FTE_DI_stop(FTE_OBJECT_PTR pObj)
{
    FTE_DI_STATUS_PTR   pStatus = (FTE_DI_STATUS_PTR)pObj->pStatus;

    FTE_GPIO_INT_init(pStatus->pGPIO, 3, 0, FALSE);
    FTE_GPIO_setISR(pStatus->pGPIO, 0, 0);
    
    return  MQX_OK;
}
 
_mqx_uint   FTE_DI_setPolarity(FTE_OBJECT_PTR pObj, boolean bActiveHI)
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
    
    return  MQX_OK;
}

/*ISR*-----------------------------------------------------
*
* Task Name    : di_int
* Comments     :
* Digital Input Signal ISR
*END*-----------------------------------------------------*/
void _di_int(void *params)
{
    FTE_OBJECT_PTR      pObj;
    FTE_LIST_ITERATOR   xIter;
    
    if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
    {
        while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
        {
            boolean             value = FALSE;
            boolean             flag;
            FTE_DI_CONFIG_PTR   pConfig;
            FTE_DI_STATUS_PTR   pStatus;
            
            pConfig = (FTE_DI_CONFIG_PTR)pObj->pConfig;
            
            if (FTE_FLAG_IS_SET(pConfig->xCommon.xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
            {
                pStatus  = (FTE_DI_STATUS_PTR)pObj->pStatus;
                
                 FTE_GPIO_INT_getFlag(pStatus->pGPIO, &flag);
                
                if (1)
                {       
                    FTE_GPIO_getValue(pStatus->pGPIO, &value);
                    
                    if (pStatus->xCommon.pValue->xData.bValue != value)
                    {
                        FTE_VALUE_setDIO(pStatus->xCommon.pValue, value);
                        
                        if (pConfig->nLED != 0)
                        {
                            FTE_LED_setValue(pConfig->nLED, value);
                        }

                        FTE_OBJ_wasUpdated(pObj);
                    }
                        
                    FTE_GPIO_INT_setEnable(pStatus->pGPIO, FALSE);
                    FTE_GPIO_INT_clrFlag(pStatus->pGPIO);
                }
            }        
        }
    }
}

void _FTE_DI_ISR(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    _di_int(NULL);    
    fte_di_int_global_unlock();
}

int_32      FTE_DI_SHELL_cmd(int_32 nArgc, char_ptr pArgv[])
{ /* Body */
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
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
                
                if (FTE_LIST_ITER_init(&_xObjList, &xIter) == MQX_OK)
                {
                    while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
                    {
                        char    pValueString[32];
                        
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
                uint_32 nID;
                FTE_OBJECT_PTR  pObj = NULL;
                
                if (!Shell_parse_number(pArgv[1], &nID))
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
                    char    pValueString[32];

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
