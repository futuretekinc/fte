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
void FTE_TASK_main
(
    FTE_UINT32  ulParams
)
{
    FTE_RET         xRet;
    FTE_TIME_DELAY  xTimeDelay;
    
    _int_install_unexpected_isr();

    FTE_DEBUG_init();
    
    xRet = _timer_create_component(TIMER_DEFAULT_TASK_PRIORITY, 1024);
    if (xRet == FTE_RET_OK)
    {
        FTE_TASK_append(FTE_TASK_TYPE_MQX, 65541);
    }
    
    FTE_PLATFORM_init(); 

    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());

    
#if FTE_NET_SUPPORTED
#if FTE_TASK_NET
    FTE_TASK_create(FTE_TASK_NET, 0, NULL);
#endif
#endif
    
#if FTE_CONSOLE_SUPPORTED
    FTE_TASK_create(FTE_TASK_SHELL, 0, NULL);
#endif

    FTE_TASK_create(FTE_TASK_OBJECT_MNGT, 0, NULL);
    
#if FTE_LORA_SUPPORTED
    FTE_S2LORA_init( NULL );
#endif

    FTE_PLATFORM_run();
    
    FTE_TIME_DELAY_init(&xTimeDelay, FTE_SYS_LIVE_CHECK_INTERVAL*10);
   
    while(TRUE)
    {
        FTE_BOOL    bPushed = FALSE;
        
        FTE_CFG_save(FALSE);
        FTE_LOG_save(); 
        
        FTE_SYS_liveCheckAndReset();

        FTE_SYS_isfactoryResetPushed(&bPushed);
        FTE_SYS_factoryReset(bPushed);
        
        FTE_TIME_DELAY_waitingAndSetNext(&xTimeDelay);
    }
}

