#include "fte_target.h"
#include "fte_config.h"
#include "fte_task.h"
#include "fte_log.h"
#include "fte_db.h"
#include "fte_cias.h"
#include "sys/fte_sys.h"
#include "sys/fte_sys_msg.h"
#include "fte_lora.h"
#include <watchdog.h>

/*TASK*-----------------------------------------------------------------
*
* Function Name  : FTE_TASK_main
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
void FTE_TASK_main(uint_32 params)
{
    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xNextTicks;            

    _int_install_unexpected_isr();

    FTE_DEBUG_init();
    
    _timer_create_component(TIMER_DEFAULT_TASK_PRIORITY, 1024);
    FTE_TASK_append(FTE_TASK_TYPE_MQX, 65541);
    
    fte_platform_init(); 

    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());

    
#if FTE_NET_SUPPORTED
#if FTE_TASK_NET
    _task_create(0, FTE_TASK_NET, 0);
#endif
#endif
    
#if FTE_CONSOLE_SUPPORTED
    _task_create(0, FTE_TASK_SHELL, 0);
#endif

    _task_create(0, FTE_TASK_OBJECT_MNGT, 0);
    
#if FTE_LORA_SUPPORTED
    FTE_LORA_init();
#endif
    
    fte_platform_run();
    
    FTE_SYS_LIVE_CHECK_init(FTE_CFG_SYS_getKeepAliveTime());
    if (FTE_CFG_SYS_getSystemMonitor())
    {
        FTE_SYS_LIVE_CHECK_start();
    }
    
   _time_get_elapsed_ticks(&xNextTicks);
   
    while(1)
    {
        int_32  nDiffTime;
        boolean bOverflow;
        
        FTE_CFG_save(FALSE);
        FTE_LOG_save(); 
        
        if (FTE_SYS_LIVE_CHECK_isRun())
        {
            if (FTE_SYS_LIVE_CHECK_isLive() == FALSE)
            {
                printf("The network is unstable.\n");
                FTE_NET_printStats();
                FTE_SYS_setUnstable();
            }
        
            if (FTE_SYS_isUnstable())
            {
                printf("System restart now!\n");
                _time_delay(1000);
                FTE_SYS_reset();
            }
        }
        
        _time_get_elapsed_ticks(&xTicks);        
        do 
        {
            _time_add_msec_to_ticks(&xNextTicks, FTE_SYS_LIVE_CHECK_INTERVAL);
            bOverflow = FALSE;        
            nDiffTime = _time_diff_milliseconds(&xNextTicks, &xTicks, &bOverflow);
            if (bOverflow)
            {
                nDiffTime = FTE_SYS_LIVE_CHECK_INTERVAL;
            }
        } while(nDiffTime <= 0);
        
        _time_delay(nDiffTime);        
    }
}

