#include "fte_target.h"
#include "fte_config.h"
#include "fte_task.h"
#include "fte_log.h"
#include "fte_db.h"
#include "fte_cias.h"
#include "sys/fte_sys.h"
#include "sys/fte_sys_msg.h"
#include "fte_time.h"
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
    FTE_TIME_DELAY      xTimeDelay;
    
    _int_install_unexpected_isr();

    FTE_DEBUG_init();
    
    _timer_create_component(TIMER_DEFAULT_TASK_PRIORITY, 1024);
    FTE_TASK_append(FTE_TASK_TYPE_MQX, 65541);
    
    FTE_PLATFORM_init(); 

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
    //FTE_LORA_init();
    //MAC_15_4_init( );
    //NS_STACK_init( );
    FTE_LORAWAN_init( TRUE );

#endif

    FTE_PLATFORM_run();
    
    FTE_TIME_DELAY_init(&xTimeDelay, FTE_SYS_LIVE_CHECK_INTERVAL*10);
   
    while(1)
    {
        FTE_CFG_save(FALSE);
        FTE_LOG_save(); 
        
        FTE_SYS_liveCheckAndReset();

        FTE_SYS_factoryReset(FTE_SYS_isfactoryResetPushed());
        
        FTE_TIME_DELAY_waitingAndSetNext(&xTimeDelay);
    }
}

