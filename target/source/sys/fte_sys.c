#include "fte_target.h"
#include "fte_config.h"
#include "fte_time.h"
#include "sys/fte_sys.h"
#include "fte_sys_bl.h"
#if FTE_V2    
static void     _sys_power_in_int_cb(void *params);
#endif
static void     _sys_sw_detect_int_cb(void *params);
static void     _sys_sw_pushed(boolean bPowerCtrl);

void            _FTE_SYS_factoryResetPushed(boolean bPowerCtrl);
static void     _FTE_SYS_INT_CB_factoryReset(void *params);


static  MQX_TICK_STRUCT             _xSWPushStartTick;
static  boolean                     _bShutdown = 0;
static  uint_32                     _nState = 0;
static  FTE_SYS_STATE_CHANGE_FPTR   _f_cb;

void    FTE_SYS_powerUp(void)
{
    _nState |= FTE_STATE_POWER_UP;
    if (_f_cb != NULL)
    {
        _f_cb(0); 
    }
} 

void    FTE_SYS_STATE_initialized(void)
{
    _nState |= FTE_STATE_INITIALIZED;
    if (_f_cb != NULL)
    {
        _f_cb(0);
    }
}

void    FTE_SYS_STATE_connected(void)
{
    _nState |= FTE_STATE_CONNECTED;
    if (_f_cb != NULL)
    {
        _f_cb(0);
    }
}

void    FTE_SYS_STATE_disconnected(void)
{
    _nState &= ~FTE_STATE_CONNECTED;
    if (_f_cb != NULL)
    {
        _f_cb(0);
    }
}

_mqx_uint   fte_sys_lock_create(FTE_SYS_LOCK_PTR _PTR_ ppKey)
{
    *ppKey = (FTE_SYS_LOCK_PTR)FTE_MEM_alloc(sizeof(FTE_SYS_LOCK));
    if (*ppKey == NULL)
    {
        return  MQX_ERROR;
    }
    
    if (_lwsem_create(&(*ppKey)->xSemaphore, 1) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

_mqx_uint   fte_sys_lock_enable(FTE_SYS_LOCK_PTR pKey)
{
    ASSERT(pKey != NULL);
    
  //  _int_disable();
    _lwsem_wait(&pKey->xSemaphore);
    
    return  MQX_OK;
}

_mqx_uint   fte_sys_lock_disable(FTE_SYS_LOCK_PTR pKey)
{
    ASSERT(pKey != NULL);
    
    _lwsem_post(&pKey->xSemaphore);
 //   _int_enable();
    
    return  MQX_OK;
}

_mqx_uint   fte_sys_lock_destroy(FTE_SYS_LOCK_PTR pKey)
{
    if (pKey == NULL)
    {
        return  MQX_INVALID_LWSEM;
    }
    
    _lwsem_destroy(&pKey->xSemaphore);
    
    FTE_MEM_free(pKey);
    
    return  MQX_OK;
}

uint_32 FTE_SYS_STATE_get(void)
{
    return  _nState;
}

void    FTE_SYS_STATE_setChangeCB(FTE_SYS_STATE_CHANGE_FPTR fCallback)
{
    _f_cb = fCallback;
}

_mqx_uint FTE_SYS_DEVICE_resetInit(void)
{
    FTE_GPIO_PTR    pResetGPIO = FTE_GPIO_get(FTE_DEV_GPIO_RESET);
    if (pResetGPIO == NULL)
    {
        return MQX_ERROR;
    }
    
    if (FTE_GPIO_attach(pResetGPIO, FTE_DEV_TYPE_ROOT) != MQX_OK)
    {
        return  MQX_ERROR;
    }

    return  FTE_SYS_DEVICE_reset();    
}


_mqx_uint  FTE_SYS_DEVICE_reset(void)
{
    FTE_GPIO_PTR    pResetGPIO = FTE_GPIO_get(FTE_DEV_GPIO_RESET);
    if (pResetGPIO == NULL)
    {
        return MQX_ERROR;
    }
    
    FTE_GPIO_setValue(pResetGPIO, FALSE);
    FTE_GPIO_setValue(pResetGPIO, TRUE);
    
    return  MQX_OK;
}

_mqx_uint FTE_SYS_powerHoldInit(void)
{
    FTE_GPIO_PTR    pPowerHoldGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_HOLD);
    if (pPowerHoldGPIO == NULL)
    {
        return MQX_ERROR;
    }
    
#if !FTE_V2
    FTE_GPIO_PTR    pPowerCtrlGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_CTRL);
    if (pPowerCtrlGPIO == NULL)
    {
        return MQX_ERROR;
    }
#endif
    
    if (FTE_GPIO_attach(pPowerHoldGPIO,    FTE_DEV_TYPE_ROOT) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
#if !FTE_V2
    if (FTE_GPIO_attach(pPowerCtrlGPIO,    FTE_DEV_TYPE_ROOT) != MQX_OK)
    {
        return  MQX_ERROR;
    }
#endif
    return  FTE_SYS_powerHold(TRUE);    
}

_mqx_uint  FTE_SYS_powerHold(boolean bHoldOn)
{
    FTE_GPIO_PTR    pPowerHoldGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_HOLD);
    if (pPowerHoldGPIO == NULL)
    {
        return MQX_ERROR;
    }
    
#if FTE_V2    
    FTE_GPIO_setValue(pPowerHoldGPIO, bHoldOn);
#else
    FTE_GPIO_PTR    pPowerCtrlGPIO = FTE_GPIO_get(FTE_DEV_GPIO_POWER_CTRL);
    if (pPowerCtrlGPIO == NULL)
    {
        return MQX_ERROR;
    }
    
    FTE_GPIO_setValue(pPowerCtrlGPIO, FALSE);
    FTE_GPIO_setValue(pPowerHoldGPIO, bHoldOn);
    FTE_GPIO_setValue(pPowerCtrlGPIO, TRUE);
    FTE_GPIO_setValue(pPowerCtrlGPIO, FALSE);
    FTE_GPIO_setValue(pPowerHoldGPIO, FALSE);
#endif
    printf("Power Hold ON\n");
    return  MQX_OK;
}

void FTE_SYS_powerStateInit(void)
{
#if FTE_V2    
    boolean bValue;
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
void _sys_power_in_int_cb(void *params)
{
    boolean bValue;
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
    boolean bValue;
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

void _sys_sw_detect_int_cb(void *params)
{
    boolean bValue;
    FTE_GPIO_PTR    pGPIO_SW_Detect = FTE_GPIO_get(FTE_DEV_GPIO_SW_DETECT);
    
    FTE_GPIO_INT_setEnable(pGPIO_SW_Detect, FALSE);
    FTE_GPIO_getValue(pGPIO_SW_Detect, &bValue);
    
    if (!bValue)
    {
        if (_bShutdown)
        {
            boolean         bOverflow;
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

void     _sys_sw_pushed(boolean bPowerCtrl)
{
    if (bPowerCtrl)
    {
        _bShutdown = TRUE;
    }
}

static  boolean _bFactoryResetPushed = FALSE;


void    FTE_SYS_factoryResetInit(void)
{
    boolean bValue;
    FTE_GPIO_PTR    pGPIO_factoryReset = FTE_GPIO_get(FTE_DEV_GPIO_FACTORY_RESET);

    FTE_GPIO_attach(pGPIO_factoryReset, FTE_DEV_TYPE_ROOT);
    
    FTE_GPIO_setISR(pGPIO_factoryReset, _FTE_SYS_INT_CB_factoryReset, 0);
    FTE_GPIO_INT_init(pGPIO_factoryReset, 3, 0, TRUE);
    FTE_GPIO_getValue(pGPIO_factoryReset, &bValue);
    
    if (bValue)
    {
        _FTE_SYS_factoryResetPushed(TRUE);
        FTE_GPIO_INT_setPolarity(pGPIO_factoryReset, FTE_LWGPIO_INT_LOW);        
    }
    else
    {
        _FTE_SYS_factoryResetPushed(FALSE);
        FTE_GPIO_INT_setPolarity(pGPIO_factoryReset, FTE_LWGPIO_INT_HIGH);
    }
    
    FTE_GPIO_INT_setEnable(pGPIO_factoryReset, TRUE);
}

boolean FTE_SYS_isfactoryResetPushed(void)
{
    return  _bFactoryResetPushed;
}

#if 1
static  uint_32     _hFactoryResetTimer = 0;
#endif

void _FTE_SYS_INT_CB_factoryReset(void *params)
{
    boolean bValue;
    FTE_GPIO_PTR    pGPIO_factoryReset = FTE_GPIO_get(FTE_DEV_GPIO_FACTORY_RESET);
    
    FTE_GPIO_INT_setEnable(pGPIO_factoryReset, FALSE);
    FTE_GPIO_getValue(pGPIO_factoryReset, &bValue);
    
    if (bValue != _bFactoryResetPushed)
    {
        if (!bValue)
        {
    #if 0
            if (_hFactoryResetTimer != 0)
            {
                _timer_cancel(_hFactoryResetTimer);
                _hFactoryResetTimer = 0;
            }        
    #endif
            _FTE_SYS_factoryResetPushed(FALSE);
            FTE_GPIO_INT_setPolarity(pGPIO_factoryReset, FTE_LWGPIO_INT_HIGH);
        }
        else
        {
            _FTE_SYS_factoryResetPushed(TRUE);
            FTE_GPIO_INT_setPolarity(pGPIO_factoryReset, FTE_LWGPIO_INT_LOW);
        }
    }
    FTE_GPIO_INT_setEnable(pGPIO_factoryReset, TRUE);
    
}

#if 1
void _FTE_SYS_CB_factoryReset(_timer_id nID, pointer pData, MQX_TICK_STRUCT_PTR pTick)
{
    if (nID == _hFactoryResetTimer)
    {
        printf("Factory Reset!\n");
    }
}
#endif

void     _FTE_SYS_factoryResetPushed(boolean bPushed)
{
#if 0
    MQX_TICK_STRUCT xTicks;

    if (_hFactoryResetTimer != 0)
    {
        _timer_cancel(_hFactoryResetTimer);
        _hFactoryResetTimer = 0;
    }        
    
    _time_init_ticks(&xTicks, 0);
    _time_add_msec_to_ticks(&xTicks, FTE_FACTORY_RESET_DETECT_TIME);

    _hFactoryResetTimer = _timer_start_oneshot_after_ticks(_FTE_SYS_CB_factoryReset, 0, TIMER_ELAPSED_TIME_MODE, &xTicks);
#endif
    _bFactoryResetPushed = bPushed;
    
}

int_32  FTE_SYS_SHELL_cmd(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;

    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                MQX_TICK_STRUCT_PTR pLastCheckTime;
                MQX_TICK_STRUCT     xCurrentTime;
                TIME_STRUCT         xTime;
                char                pBuff[64];
                boolean             bOverflow = FALSE;
                uint_32             ulDiffTime;
                
                printf("<Live Check>\n");
                printf("%16s : %s\n", "State", FTE_SYS_LIVE_CHECK_isRun()?"RUNNING":"STOP");
                printf("%16s : %d seconds\n", "Keepalive", FTE_CFG_SYS_getKeepAliveTime());
                pLastCheckTime = FTE_SYS_LIVE_CHECK_lastCheckTime();
                _ticks_to_time(pLastCheckTime, &xTime);
                FTE_TIME_toString(&xTime, pBuff, sizeof(pBuff));
                printf("%16s : %s\n", "Touch Time", pBuff);
                
                _time_get_elapsed_ticks(&xCurrentTime);
                ulDiffTime = _time_diff_seconds(&xCurrentTime, pLastCheckTime, &bOverflow);
                printf("%16s : %d seconds\n",   "Live Time", ulDiffTime);
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
                    uint_32  ulKeepAliveTime = 0;
                    
                    if (!Shell_parse_uint_32(pArgv[2], &ulKeepAliveTime))
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
                        if (!FTE_SYS_LIVE_CHECK_isRun())
                        {
                            FTE_SYS_LIVE_CHECK_start();
                        }
                    }
                    else if (strcmp(pArgv[2], "stop") == 0)
                    {
                        FTE_CFG_SYS_setSystemMonitor(FALSE);
                        if (FTE_SYS_LIVE_CHECK_isRun())
                        {
                            FTE_SYS_LIVE_CHECK_stop();
                        }
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


int_32  FTE_SYS_RESET_cmd(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;

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

int_32  FTE_SYS_SHUTDOWN_SHELL_cmd(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
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

void    fte_sys_lcd_init(void)
{
    FTE_SSD1305_PTR    pSSD1305 = FTE_SSD1305_get(FTE_DEV_SSD1305_0);

    FTE_SSD1305_attach(pSSD1305, FTE_DEV_TYPE_ROOT);
    
}


uint_32     FTE_SYS_getTime(void)
{
    TIME_STRUCT     xTime;
                
    _time_get(&xTime);

    return  xTime.SECONDS;
}

static boolean  bSystemIsUnstable = FALSE;

void FTE_SYS_setUnstable(void)
{
    bSystemIsUnstable = TRUE;
}

boolean FTE_SYS_isUnstable(void)
{
    return  bSystemIsUnstable;
}


void FTE_SYS_reset(void)
{
    SCB_AIRCR = (SCB_AIRCR_VECTKEY(0x05fa) | SCB_AIRCR_SYSRESETREQ_MASK);
}


_mqx_uint   FTE_SYS_getOID(uint_8_ptr pOID)
{
    return  FTE_SYS_BL_getOID(pOID);
}

char_ptr    FTE_SYS_getOIDString(void)
{
    static  uint_8 pOIDString[FTE_OID_SIZE+1] = {'\0', };
    
    if (pOIDString[0] == '\0')
    {
        FTE_SYS_getOID(pOIDString);
    }
    
    return  (char_ptr)pOIDString;
}

_mqx_uint   FTE_SYS_getMAC(uint_8_ptr pMAC)
{
    return  FTE_SYS_BL_getMAC(pMAC);
}


static  MQX_TICK_STRUCT _xLastCheckTime;
static  boolean         _bLiveCheckRunning = FALSE;
static  uint_32         _ulKeepAliveTime = FTE_SYS_KEEP_ALIVE_TIME;

_mqx_uint   FTE_SYS_LIVE_CHECK_init(uint_32 ulKeepAliveTime)
{
    if ((ulKeepAliveTime >= FTE_SYS_KEEP_ALIVE_TIME_MIN) && (ulKeepAliveTime <= FTE_SYS_KEEP_ALIVE_TIME_MAX))
    {
        _ulKeepAliveTime = ulKeepAliveTime;
        return  MQX_OK;
    }
    
    return  MQX_ERROR;
} 

_mqx_uint   FTE_SYS_LIVE_CHECK_start(void)
{
    _time_get_elapsed_ticks(&_xLastCheckTime);
    _bLiveCheckRunning = TRUE;

    return  MQX_OK;
}

_mqx_uint   FTE_SYS_LIVE_CHECK_touch(void)
{
    if (_bLiveCheckRunning)
    {    
        _time_get_elapsed_ticks(&_xLastCheckTime);
    }

    return  MQX_OK;
}

_mqx_uint   FTE_SYS_LIVE_CHECK_stop(void)
{
    _bLiveCheckRunning = FALSE;
    
    return  MQX_OK;
}

boolean     FTE_SYS_LIVE_CHECK_isRun(void)
{
    return  _bLiveCheckRunning;
}

MQX_TICK_STRUCT_PTR FTE_SYS_LIVE_CHECK_lastCheckTime(void)
{
    return  &_xLastCheckTime;
}

boolean     FTE_SYS_LIVE_CHECK_isLive(void)
{
    if (_bLiveCheckRunning)
    {
        MQX_TICK_STRUCT xCurrentTime;
        boolean         bOverflow = FALSE;
        uint_32         ulDiffTime;
        
        _time_get_elapsed_ticks(&xCurrentTime);
        
        ulDiffTime = _time_diff_seconds(&xCurrentTime, &_xLastCheckTime, &bOverflow);
        if ( (ulDiffTime> _ulKeepAliveTime) || bOverflow)
        {
            return  FALSE;
        }
    }
    
    return  TRUE;
}
