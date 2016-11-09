#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"

#if FTE_SRF_SUPPORTED

static  
FTE_RET   FTE_SRF_init(FTE_OBJECT_PTR pObj);

static  
FTE_RET   FTE_SRF_run(FTE_OBJECT_PTR pObj);

static  
FTE_RET FTE_SRF_stop(FTE_OBJECT_PTR pObj);

static  
void    FTE_SRF_done(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);

static 
FTE_RET FTE_SRF_get(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pValue, TIME_STRUCT_PTR pTimeStamp);

static 
void    FTE_SRF_restartConvert(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);

static 
FTE_UINT32 FTE_SRF_getUpdateInterval(FTE_OBJECT_PTR pObj);

static 
FTE_RET FTE_SRF_setUpdateInterval(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);

static  FTE_OBJECT_ACTION FTE_SRF_action = 
{
    .f_init         = FTE_SRF_init,
    .f_run          = FTE_SRF_run,
    .f_stop         = FTE_SRF_stop, 
    .f_get          = FTE_SRF_get,
    .f_set          = NULL,
    .f_get_update_interval = FTE_SRF_getUpdateInterval,
    .f_set_update_interval = FTE_SRF_setUpdateInterval
};

static  FTE_UINT32      _nObjects = 0;
static  FTE_OBJECT_PTR  _pHead = NULL;

FTE_RET   fte_srf_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    FTE_SRF_STATUS_PTR   pStatus;
    if (pObj == NULL)
    {
        goto error;
    }

    pStatus = (FTE_SRF_STATUS_PTR)FTE_MEM_allocZero(sizeof(FTE_SRF_STATUS));
    if (pStatus == NULL)
    {
        goto error;
    }
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&FTE_SRF_action;
    pObj->pStatus = (FTE_OBJECT_STATUS_PTR)pStatus;
    pObj->pSibling = _pHead;
    _nObjects++;
    _pHead = pObj;
   
    FTE_SRF_init(pObj);

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
    
}

FTE_RET fte_srf_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_BOOL bFound = FALSE;
    
    if (pObj == NULL)
    {
        goto error;
    }

    if (pObj == _pHead)
    {
        _pHead = pObj->pSibling;
        bFound = TRUE;
    }
    else 
    {
        FTE_OBJECT_PTR pPrev = _pHead;
        
        while(pPrev != NULL)
        {
            if (pPrev->pSibling == pObj)
            {
                pPrev->pSibling = pObj->pSibling;
        bFound = TRUE;
                break;
            }
        }
    }

    if (bFound)
    {
         FTE_MEM_free(pObj->pStatus);
         pObj->pAction = NULL;
         pObj->pStatus = NULL;
         _nObjects--;
    }
    
    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_SRF_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    FTE_SRF_STATUS_PTR   pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
   
    if (pStatus->pFP == NULL)
    {
        FTE_UINT32 nValue;
        pStatus->pFP = fopen("ittyb:", 0);
        
        nValue = 9600;
        ioctl (pStatus->pFP, IO_IOCTL_SERIAL_SET_BAUD, &nValue);
        nValue = 8;
        ioctl (pStatus->pFP, IO_IOCTL_SERIAL_SET_DATA_BITS, &nValue);
        nValue = 1;
        ioctl (pStatus->pFP, IO_IOCTL_SERIAL_SET_PARITY, &nValue);
        nValue = 1;
        ioctl (pStatus->pFP, IO_IOCTL_SERIAL_SET_STOP_BITS, &nValue);
    }

    return  FTE_RET_OK;
}

FTE_RET   FTE_SRF_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;            
    FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
    FTE_SRF_CONFIG_PTR  pConfig = (FTE_SRF_CONFIG_PTR)pObj->pConfig;
    
    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_msec_to_ticks(&xTicks, 1000);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(FTE_SRF_restartConvert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);    
    
    _time_init_ticks(&xDTicks, _time_get_ticks_per_sec());
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(FTE_SRF_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);

    return  FTE_RET_OK;
}

FTE_RET   FTE_SRF_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    return  FTE_RET_OK;
    
}

void FTE_SRF_done
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
    FTE_CHAR    _buff[16];
    
    memset(_buff, 0, sizeof(_buff));
    if (fgets(_buff, 16, pStatus->pFP) != 0)
    {
        if (_buff[0] == 'g')
        {
            printf("%s\n", _buff);
        }
    }
    
}


void FTE_SRF_restartConvert
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        _time_init_ticks(&xDTicks, _time_get_ticks_per_sec());
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(FTE_SRF_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);
    }
    else
    {
        FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

FTE_RET    FTE_SRF_get
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pValue, 
    TIME_STRUCT_PTR pTimeStamp
)
{
    ASSERT(pObj != NULL && pValue != NULL);
    
    FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (pTimeStamp != NULL)
        {
            *pTimeStamp = pStatus->xTimeStamp;
        }
        
        return  FTE_RET_OK;
    }

    return  FTE_RET_ERROR;
}


FTE_INT32  shell_srf
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    { 
        switch(nArgc)
        {
        case    1:
            { 
                int count = FTE_OBJ_count(FTE_OBJ_TYPE_SRF, FALSE);
                for(int i = 0 ; i < count ; i++)
                {
                    FTE_OBJECT_PTR pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_SRF, i, FALSE);
                    if (pObj != NULL)
                    {
                    }
                }
            }
            break;
        case    4:
            {
                if (strcmp(pArgv[1], "attach") == 0)
                {
                    FTE_UINT32 nValue;
                    
                    if (FTE_strToUINT32( pArgv[2], &nValue) != FTE_RET_OK)  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    FTE_OBJECT_PTR          pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_SRF, nValue, FALSE);
                    if (pObj == NULL)
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (FTE_strToUINT32( pArgv[3], &nValue) != FTE_RET_OK)  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }                

                }
            }
            break;
        }
    }
    
error:    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]\n", pArgv[0]);
            printf("        id - SRF Index \n");
        }
    }

    return   xRet;
}
            
FTE_UINT32      FTE_SRF_getUpdateInterval
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_SRF_CONFIG_PTR  pConfig = (FTE_SRF_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

FTE_RET    FTE_SRF_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_SRF_CONFIG_PTR  pConfig = (FTE_SRF_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    fte_config_object_config_save(pObj);
    
    return  FTE_RET_OK;
}


#endif