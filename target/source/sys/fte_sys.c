#include "fte_target.h"
#include "fte_config.h"
#include "fte_time.h"
#include "sys/fte_sys.h"
#include "fte_sys_bl.h"
#if FTE_V2    
static void     _sys_power_in_int_cb(FTE_VOID_PTR params);
#endif
static void     _sys_sw_detect_int_cb(FTE_VOID_PTR params);
static void     _sys_sw_pushed(FTE_BOOL bPowerCtrl);

void            _FTE_SYS_factoryResetPushed(FTE_BOOL bPowerCtrl);
static void     _FTE_SYS_INT_CB_factoryReset(FTE_VOID_PTR params);


static  MQX_TICK_STRUCT             _xSWPushStartTick;
static  FTE_BOOL                    _bShutdown = 0;
static  FTE_UINT32                     _nState = 0;
static  FTE_BOOL                    _bLiveCheck = FALSE;
static  FTE_SYS_STATE_CHANGE_FPTR   _FTE_SYS_CB_stateChanged = NULL;

void    FTE_SYS_powerUp(void)
{
    _nState |= FTE_STATE_POWER_UP;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0); 
    }
} 

void    FTE_SYS_STATE_initialized(void)
{
    _nState |= FTE_STATE_INITIALIZED;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0);
    }
}

void    FTE_SYS_STATE_connected(void)
{
    _nState |= FTE_STATE_CONNECTED;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0);
    }
}

void    FTE_SYS_STATE_disconnected(void)
{
    _nState &= ~FTE_STATE_CONNECTED;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0);
    }
}

void    FTE_SYS_STATE_factoryResetRequested(void)
{
    _nState |= FTE_STATE_WARNING;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0);
    }
}

void    FTE_SYS_STATE_factoryResetCanceled(void)
{
    _nState &= ~FTE_STATE_WARNING;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0);
    }
}

void    FTE_SYS_STATE_factoryResetFinished(void)
{
    _nState |= FTE_STATE_ALERT;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0);
    }
}

void    FTE_SYS_STATE_setAlert(void)
{
    _nState |= FTE_STATE_ALERT;
    if (_FTE_SYS_CB_stateChanged != NULL)
    {
        _FTE_SYS_CB_stateChanged(0);
    }
}

FTE_RET FTE_SYS_LOCK_create
(
    FTE_SYS_LOCK_PTR _PTR_ ppKey
)
{
    *ppKey = (FTE_SYS_LOCK_PTR)FTE_MEM_alloc(sizeof(FTE_SYS_LOCK));
    if (*ppKey == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    if (_lwsem_create(&(*ppKey)->xSemaphore, 1) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SYS_LOCK_destroy
(
    FTE_SYS_LOCK_PTR pKey
)
{
    if (pKey == NULL)
    {
        return  MQX_INVALID_LWSEM;
    }
    
    _lwsem_destroy(&pKey->xSemaphore);
    
    FTE_MEM_free(pKey);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_SYS_LOCK_init
(
    FTE_SYS_LOCK_PTR    pKey,
    FTE_UINT32          ulInit
)
{
    ASSERT(pKey != NULL);
    
    if (_lwsem_create(&pKey->xSemaphore, ulInit) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SYS_LOCK_final
(
    FTE_SYS_LOCK_PTR pKey
)
{
    if (pKey == NULL)
    {
        return  MQX_INVALID_LWSEM;
    }
    
    _lwsem_destroy(&pKey->xSemaphore);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SYS_LOCK_enable
(
    FTE_SYS_LOCK_PTR pKey
)
{
    ASSERT(pKey != NULL);
    
  //  _int_disable();
    _lwsem_wait(&pKey->xSemaphore);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SYS_LOCK_disable
(
    FTE_SYS_LOCK_PTR    pKey
)
{
    ASSERT(pKey != NULL);
    
    _lwsem_post(&pKey->xSemaphore);
 //   _int_enable();
    
    return  FTE_RET_OK;
}


FTE_RET FTE_SYS_STATE_get(FTE_UINT32_PTR pState)
{
    ASSERT(pState != NULL);
    
    *pState = _nState;
    
    return  FTE_RET_OK;
}

void    FTE_SYS_STATE_setChangeCB
(
    FTE_SYS_STATE_CHANGE_FPTR fCallback
)
{
    _FTE_SYS_CB_stateChanged = fCallback;
}

FTE_RET FTE_SYS_DEVICE_resetInit(void)
{
    FTE_GPIO_PTR    pResetGPIO = FTE_GPIO_get(FTE_DEV_GPIO_RESET);
    if (pResetGPIO == NULL)
    {
        return FTE_RET_ERROR;
    }
    
    if (FTE_GPIO_attach(pResetGPIO, FTE_DEV_TYPE_ROOT) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }

    return  FTE_SYS_DEVICE_reset();    
}

FTE_RET   FTE_SYS_liveCheckStart(void)
{

    FTE_NET_liveCheckStart();

    _bLiveCheck = TRUE;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SYS_liveCheckStop(void)
{
    
    FTE_NET_liveCheckStop();
    
    _bLiveCheck = FALSE;
    
    return  FTE_RET_OK;
}

FTE_BOOL    FTE_SYS_isLiveChecking(void)
{
    return  _bLiveCheck;
}


FTE_RET  FTE_SYS_DEVICE_reset(void)
{
    FTE_GPIO_PTR    pResetGPIO = FTE_GPIO_get(FTE_DEV_GPIO_RESET);
    if (pResetGPIO == NULL)
    {
        return FTE_RET_ERROR;
    }
    
    FTE_GPIO_setValue(pResetGPIO, FALSE);
    FTE_GPIO_setValue(pResetGPIO, TRUE);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_SYS_powerHoldInit(void)
{
    FTE_GPIO_PTR    pPowerHoldGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_HOLD);
    if (pPowerHoldGPIO == NULL)
    {
        return FTE_RET_ERROR;
    }
    
#if !FTE_V2
    FTE_GPIO_PTR    pPowerCtrlGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_CTRL);
    if (pPowerCtrlGPIO == NULL)
    {
        return FTE_RET_ERROR;
    }
#endif
    
    if (FTE_GPIO_attach(pPowerHoldGPIO,    FTE_DEV_TYPE_ROOT) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
#if !FTE_V2
    if (FTE_GPIO_attach(pPowerCtrlGPIO,    FTE_DEV_TYPE_ROOT) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
#endif
    return  FTE_SYS_powerHold(TRUE);    
}

FTE_RET  FTE_SYS_powerHold
(
    FTE_BOOL    bHoldOn
)
{
    FTE_GPIO_PTR    pPowerHoldGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_HOLD);
    if (pPowerHoldGPIO == NULL)
    {
        return FTE_RET_ERROR;
    }
    
#if FTE_V2    
    FTE_GPIO_setValue(pPowerHoldGPIO, bHoldOn);
#else
    FTE_GPIO_PTR    pPowerCtrlGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_CTRL);
    if (pPowerCtrlGPIO == NULL)
    {
        return FTE_RET_ERROR;
    }
    
    FTE_GPIO_setValue(pPowerCtrlGPIO, FALSE);
    FTE_GPIO_setValue(pPowerHoldGPIO, bHoldOn);
    FTE_GPIO_setValue(pPowerCtrlGPIO, TRUE);
    FTE_GPIO_setValue(pPowerCtrlGPIO, FALSE);
    FTE_GPIO_setValue(pPowerHoldGPIO, FALSE);
#endif
    printf("Power Hold ON\n");
    return  FTE_RET_OK;
}

void FTE_SYS_powerStateInit(void)
{
#if FTE_V2    
    FTE_BOOL bValue;
    FTE_GPIO_PTR    pGPIO_PowerIn = FTE_GPIO_get(FTE_DEV_GPIO_POWER_IN);

    FTE_GPIO_attach(pGPIO_PowerIn, FTE_DEV_TYPE_ROOT);
    FTE_GPIO_setISR(pGPIO_PowerIn, _sys_power_in_int_cb, 0);
    FTE_GPIO_INT_init(pGPIO_PowerIn, 3, 0, TRUE);
    FTE_GPIO_getValue(pGPIO_PowerIn, &bValue);
    
    if (bValue)
    {
        FTE_GPIO_INT_setPolarity(pGPIO_PowerIn, FTE_LWGPIO_INT_LOW);        
    }
    else
    {
        FTE_GPIO_INT_setPolarity(pGPIO_PowerIn, FTE_LWGPIO_INT_HIGH);
    }

    FTE_GPIO_INT_setEnable(pGPIO_PowerIn, TRUE);
#endif
}

#if FTE_V2    
void _sys_power_in_int_cb
(
    FTE_VOID_PTR  params
)
{
    FTE_BOOL bValue;
    FTE_GPIO_PTR    pGPIO_PowerIn = FTE_GPIO_get(FTE_DEV_GPIO_POWER_IN);
    
    FTE_GPIO_INT_setEnable(pGPIO_PowerIn, FALSE);
    FTE_GPIO_getValue(pGPIO_PowerIn, &bValue);
    
    if (bValue)
    {
        FTE_GPIO_INT_setPolarity(pGPIO_PowerIn, FTE_LWGPIO_INT_LOW);
    }
    else
    {
        FTE_GPIO_INT_setPolarity(pGPIO_PowerIn, FTE_LWGPIO_INT_HIGH);
    }
    FTE_GPIO_INT_setEnable(pGPIO_PowerIn, TRUE);
}
#endif

void    FTE_SYS_SWDetectInit(void)
{
    FTE_BOOL bValue;
    FTE_GPIO_PTR    pGPIO_SW_Detect = FTE_GPIO_get(FTE_DEV_GPIO_SW_DETECT);

    FTE_GPIO_attach(pGPIO_SW_Detect, FTE_DEV_TYPE_ROOT);
    
    FTE_GPIO_setISR(pGPIO_SW_Detect, _sys_sw_detect_int_cb, 0);
    FTE_GPIO_INT_init(pGPIO_SW_Detect, 3, 0, TRUE);
    FTE_GPIO_getValue(pGPIO_SW_Detect, &bValue);
    
    if (bValue)
    {
        printf("Pulled\n");
        _sys_sw_pushed(FALSE);
        FTE_GPIO_INT_setPolarity(pGPIO_SW_Detect, FTE_LWGPIO_INT_LOW);        
    }
    else
    {
        printf("Pushed\n");
        FTE_GPIO_INT_setPolarity(pGPIO_SW_Detect, FTE_LWGPIO_INT_HIGH);
    }
    
    FTE_GPIO_INT_setEnable(pGPIO_SW_Detect, TRUE);
}

void _sys_sw_detect_int_cb
(
    FTE_VOID_PTR    params
)
{
    FTE_BOOL bValue;
    FTE_GPIO_PTR    pGPIO_SW_Detect = FTE_GPIO_get(FTE_DEV_GPIO_SW_DETECT);
    
    FTE_GPIO_INT_setEnable(pGPIO_SW_Detect, FALSE);
    FTE_GPIO_getValue(pGPIO_SW_Detect, &bValue);
    
    if (!bValue)
    {
        if (_bShutdown)
        {
            FTE_BOOL         bOverflow;
            MQX_TICK_STRUCT xTicks;
            
            _time_get_elapsed_ticks(&xTicks);
            
            if (_time_diff_milliseconds(&xTicks, &_xSWPushStartTick, &bOverflow) >= 1000)
            {
                FTE_SYS_powerHold(FALSE);
            }
        }
        
        FTE_GPIO_INT_setPolarity(pGPIO_SW_Detect, FTE_LWGPIO_INT_HIGH);
    }
    else
    {
        _sys_sw_pushed(TRUE);
        FTE_GPIO_INT_setPolarity(pGPIO_SW_Detect, FTE_LWGPIO_INT_LOW);
    }
    FTE_GPIO_INT_setEnable(pGPIO_SW_Detect, TRUE);
    
}

void     _sys_sw_pushed(FTE_BOOL bPowerCtrl)
{
    if (bPowerCtrl)
    {
        _bShutdown = TRUE;
    }
}

static 
FTE_BOOL _bFactoryResetPushed = FALSE;


void    FTE_SYS_factoryResetInit(void)
{
    FTE_GPIO_PTR    pGPIO_factoryReset = FTE_GPIO_get(FTE_DEV_GPIO_FACTORY_RESET);

    if (pGPIO_factoryReset != NULL)
    {
        FTE_GPIO_attach(pGPIO_factoryReset, FTE_DEV_TYPE_ROOT);
        
        FTE_GPIO_setISR(pGPIO_factoryReset, _FTE_SYS_INT_CB_factoryReset, 0);
        FTE_GPIO_INT_init(pGPIO_factoryReset, 3, 0, TRUE);
        
        _FTE_SYS_INT_CB_factoryReset(NULL);
    }
}

FTE_RET FTE_SYS_isfactoryResetPushed(FTE_BOOL_PTR pPushed)
{
    ASSERT(pPushed != NULL);
    
    *pPushed = _bFactoryResetPushed;
    
    return  FTE_RET_OK;
}

void _FTE_SYS_INT_CB_factoryReset
(
    FTE_VOID_PTR    params
)
{
    FTE_BOOL bValue;
    FTE_GPIO_PTR    pGPIO_factoryReset = FTE_GPIO_get(FTE_DEV_GPIO_FACTORY_RESET);

    FTE_GPIO_INT_setEnable(pGPIO_factoryReset, FALSE);
    FTE_GPIO_getValue(pGPIO_factoryReset, &bValue);
    
    if (bValue == TRUE)
    {
        FTE_GPIO_INT_setPolarity(pGPIO_factoryReset, FTE_LWGPIO_INT_LOW);
        _bFactoryResetPushed = TRUE;
    }
    else
    {
        FTE_GPIO_INT_setPolarity(pGPIO_factoryReset, FTE_LWGPIO_INT_HIGH);
        _bFactoryResetPushed = FALSE;
    }
    
    FTE_GPIO_INT_setEnable(pGPIO_factoryReset, TRUE);

}

static 
FTE_UINT32  _hFactoryResetTimer = 0;

void _FTE_SYS_CB_factoryReset
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    if (xTimerID == _hFactoryResetTimer)
    {
        printf("Factory Reset!\n");
        FTE_CFG_clear();
        _time_delay(500);
        FTE_SYS_STATE_factoryResetFinished();
        _time_delay(500);
        FTE_SYS_reset();
        
    }
}

void     FTE_SYS_factoryReset(FTE_BOOL bPushed)
{
    MQX_TICK_STRUCT xTicks;

    if (bPushed)
    {
        if (_hFactoryResetTimer == 0)
        {
            FTE_SYS_STATE_factoryResetRequested();
            
            _time_init_ticks(&xTicks, 0);
            _time_add_msec_to_ticks(&xTicks, FTE_FACTORY_RESET_DETECT_TIME);

            _hFactoryResetTimer = _timer_start_oneshot_after_ticks(_FTE_SYS_CB_factoryReset, 0, TIMER_ELAPSED_TIME_MODE, &xTicks);
        }        
    }
    else
    {
        if (_hFactoryResetTimer != 0)
        {
            FTE_SYS_STATE_factoryResetCanceled();
            _timer_cancel(_hFactoryResetTimer);
            _hFactoryResetTimer = 0;
        }        
    }
}


FTE_INT32  FTE_SYS_SHELL_cmd
(
    FTE_INT32   nArgc, 
    FTE_CHAR_PTR pArgv[]
)
{
    FTE_BOOL              bPrintUsage, bShortHelp = FALSE;
    FTE_INT32               nReturnCode = SHELL_EXIT_SUCCESS;

    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                MQX_TICK_STRUCT     xLastCheckTime;
                MQX_TICK_STRUCT     xTempTick;
                FTE_BOOL             bOverflow = FALSE;
                FTE_UINT32             ulDiffTime;
                
                printf("<Network Live Information>\n");
                printf("%16s : %s\n", "State", FTE_NET_isLiveChecking()?"RUNNING":"STOP");
                printf("%16s : %4d seconds\n", "Keepalive", FTE_CFG_SYS_getKeepAliveTime());
                FTE_NET_lastLiveCheckTime(&xLastCheckTime);
                _time_get_elapsed_ticks(&xTempTick);
                ulDiffTime = _time_diff_seconds(&xTempTick, &xLastCheckTime, &bOverflow);
                printf("%16s : %4d seconds\n",   "Elapsed Time", ulDiffTime);
                
                printf("<Device Live Information>\n");
                printf("%16s : %4d\n", "Max Allowed Retry", FTE_OBJ_CHECK_FAILURE_COUNT_MAX);
                printf("%16s : %4d\n", "Allowed Retry", FTE_CFG_SYS_getAllowedFailureCount());
            }
            break;
            
        case    2:
            {
                if (strcmp(pArgv[1], "keepalive") == 0)
                {
                    printf("%-16s : %d seconds\n", "Keep alive time", FTE_CFG_SYS_getKeepAliveTime());
                }
            }
            break;
            
        case    3:
            {
                if (strcmp(pArgv[1], "keepalive") == 0)
                {
                    FTE_UINT32  ulKeepAliveTime = 0;
                    
                    if (FTE_strToUINT32(pArgv[2], &ulKeepAliveTime) != FTE_RET_OK)
                    {
                        if (ulKeepAliveTime > FTE_SYS_KEEP_ALIVE_TIME_MAX)
                        {
                            bPrintUsage = TRUE;
                        }
                        else
                        {
                            printf("Keep alive time changed : %d -> %d seconds\n", FTE_CFG_SYS_getKeepAliveTime(), ulKeepAliveTime);
                            FTE_CFG_SYS_setKeepAliveTime(ulKeepAliveTime);
                        }
                    }
                }
                else if (strcmp(pArgv[1], "monitor") == 0)
                {
                    if (strcmp(pArgv[2], "start") == 0)
                    {
                        FTE_CFG_SYS_setSystemMonitor(TRUE);
                        FTE_NET_liveCheckStart();
                    }
                    else if (strcmp(pArgv[2], "stop") == 0)
                    {
                        FTE_CFG_SYS_setSystemMonitor(FALSE);
                        FTE_NET_liveCheckStop();
                    }
                    else
                    {
                        bPrintUsage = TRUE;
                    }
                }
                else
                {
                    bPrintUsage = TRUE;
                }
            }
        }
    }
    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<commands>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<commands>]\n", pArgv[0]);
            printf("  Commands : \n");
            printf("    keepalive [<seconds>]\n");
            printf("        get/set keepalive\n");            
            printf("    monitor [start|stop]\n");
            printf("        auto system check\n");            
        }
    }

    return  0;
}


FTE_INT32  FTE_SYS_RESET_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL              bPrintUsage, bShortHelp = FALSE;
    FTE_INT32               nReturnCode = SHELL_EXIT_SUCCESS;

    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_SYS_reset();
            }
            break;            
        }
    }
    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s\n", pArgv[0]);
        }
    }

    return  0;
}

FTE_INT32  FTE_SYS_SHUTDOWN_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL              bPrintUsage, bShortHelp = FALSE;
    FTE_INT32               nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_SYS_powerHold(FALSE);
            }
        default:
            bPrintUsage = TRUE;
        }
    }
    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s\n", pArgv[0]);
        }
    }
    
    return  nReturnCode;
}

void    FTE_LCD_init(void)
{
    FTE_SSD1305_PTR    pSSD1305 = FTE_SSD1305_get(FTE_DEV_SSD1305_0);

    FTE_SSD1305_attach(pSSD1305, FTE_DEV_TYPE_ROOT);
    
}


FTE_UINT32     FTE_SYS_getTime(void)
{
    TIME_STRUCT     xTime;
                
    _time_get(&xTime);

    return  xTime.SECONDS;
}

static 
FTE_BOOL  bSystemIsStable = TRUE;

void FTE_SYS_setUnstable(void)
{
    bSystemIsStable = FALSE;
}

FTE_BOOL FTE_SYS_isStable(void)
{
    return  bSystemIsStable;
}


void FTE_SYS_reset(void)
{
    SCB_AIRCR = (SCB_AIRCR_VECTKEY(0x05fa) | SCB_AIRCR_SYSRESETREQ_MASK);
}


FTE_RET   FTE_SYS_getOID
(
    FTE_UINT8_PTR   pOID
)
{
    return  FTE_SYS_BL_getOID(pOID);
}

FTE_CHAR_PTR    FTE_SYS_getOIDString(void)
{
    static  uint_8 pOIDString[FTE_OID_SIZE+1] = {'\0', };
    
    if (pOIDString[0] == '\0')
    {
        FTE_SYS_getOID(pOIDString);
    }
    
    return  (FTE_CHAR_PTR)pOIDString;
}

FTE_RET   FTE_SYS_getMAC(FTE_UINT8_PTR pMAC)
{
    return  FTE_SYS_BL_getMAC(pMAC);
}


void FTE_SYS_liveCheckAndReset(void)
{
    if (FTE_NET_isLiveChecking())
    {
        if (!FTE_SYS_isStable())
        {
            FTE_SYS_STATE_setAlert();
            printf("System is unstabled!\n");
            printf("System restart now!\n");
            _time_delay(1000);
            FTE_SYS_reset();
        }
    }
}

