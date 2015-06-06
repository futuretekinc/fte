#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"

#if FTE_SRF_SUPPORTED

static  _mqx_uint   _srf_init(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _srf_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _srf_stop(FTE_OBJECT_PTR pObj);
static  void        _srf_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static _mqx_uint    _srf_get(FTE_OBJECT_PTR pObj, uint_32_ptr pValue, TIME_STRUCT *xTimeStamp);
static void         _srf_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static uint_32      _srf_get_update_interval(FTE_OBJECT_PTR pObj);
static _mqx_uint    _srf_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval);

static  FTE_OBJECT_ACTION _Action = 
{
    .f_init         = _srf_init,
    .f_run          = _srf_run,
    .f_stop         = _srf_stop, 
    .f_get          = _srf_get,
    .f_set          = NULL,
    .f_get_update_interval = _srf_get_update_interval,
    .f_set_update_interval = _srf_set_update_interval
};

static  uint_32        _nObjects = 0;
static  FTE_OBJECT_PTR _pHead = NULL;

_mqx_uint   fte_srf_attach(FTE_OBJECT_PTR pObj)
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
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_Action;
    pObj->pStatus = (FTE_OBJECT_STATUS_PTR)pStatus;
    pObj->pSibling = _pHead;
    _nObjects++;
    _pHead = pObj;
   
    _srf_init(pObj);

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
    
}

_mqx_uint fte_srf_detach(FTE_OBJECT_PTR pObj)
{
    boolean bFound = FALSE;
    
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
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

_mqx_uint   _srf_init(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    FTE_SRF_STATUS_PTR   pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
   
    if (pStatus->pFP == NULL)
    {
        uint_32 nValue;
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

    return  MQX_OK;
}

_mqx_uint   _srf_run(FTE_OBJECT_PTR pObj)
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
    _time_add_sec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_srf_restart_convert, pObj, TIMER_KERNEL_TIME_MODE, &xTicks, &xDTicks);    
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, 500);    
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_srf_done, pObj, TIMER_KERNEL_TIME_MODE, &xDTicks);

    return  MQX_OK;
}

_mqx_uint   _srf_stop(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    return  MQX_OK;
    
}

static void _srf_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
    char    _buff[16];
    
    memset(_buff, 0, sizeof(_buff));
    if (fgets(_buff, 16, pStatus->pFP) != 0)
    {
        if (_buff[0] == 'g')
        {
            printf("%s\n", _buff);
        }
    }
    
}


static void _srf_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        _time_init_ticks(&xDTicks, 0);
        _time_add_msec_to_ticks(&xDTicks, 500);    
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_srf_done, pObj, TIMER_KERNEL_TIME_MODE, &xDTicks);
    }
    else
    {
        FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

_mqx_uint    _srf_get(FTE_OBJECT_PTR pObj, uint_32 *pValue, TIME_STRUCT *xTimeStamp)
{
    ASSERT(pObj != NULL && pValue != NULL);
    
    FTE_SRF_STATUS_PTR  pStatus = (FTE_SRF_STATUS_PTR)pObj->pStatus;
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        if (xTimeStamp != NULL)
        {
            *xTimeStamp = pStatus->xTimeStamp;
        }
        
        return  MQX_OK;
    }

    return  MQX_ERROR;
}


int_32  shell_srf(int_32 argc, char_ptr argv[] )
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
                if (strcmp(argv[1], "attach") == 0)
                {
                    uint_32 nValue;
                    
                    if (! Shell_parse_number( argv[2], &nValue))  
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    FTE_OBJECT_PTR          pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_SRF, nValue, FALSE);
                    if (pObj == NULL)
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (! Shell_parse_number( argv[3], &nValue))  
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }                

                }
            }
            break;
        }
    }
    
error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<id>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]\n", argv[0]);
            printf("        id - SRF Index \n");
        }
    }

    return   return_code;
}
            
uint_32      _srf_get_update_interval(FTE_OBJECT_PTR pObj)
{
    FTE_SRF_CONFIG_PTR  pConfig = (FTE_SRF_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

_mqx_uint    _srf_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval)
{
    FTE_SRF_CONFIG_PTR  pConfig = (FTE_SRF_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    fte_config_object_config_save(pObj);
    
    return  MQX_OK;
}


#endif