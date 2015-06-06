#include "fte_target.h"
#include <watchdog.h>
/*TASK*-----------------------------------------------------------------
*
* Function Name  : task_watchdog
* Returned Value : void
* Comments       : 
*
*END------------------------------------------------------------------*/
extern  void __boot(void);

void FTE_TASK_WATCHDOG_error(pointer  td_ptr)
{
    FTE_SYS_reset();
}

void FTE_TASK_watchdog(uint_32 params)
{
    TIME_STRUCT     xTime;
    MQX_TICK_STRUCT xTicks;
     
    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    _watchdog_create_component(BSP_TIMER_INTERRUPT_VECTOR, FTE_TASK_WATCHDOG_error);
    
    xTime.MILLISECONDS  = FTE_TASK_WATCHDOG_TIME % 1000;
    xTime.SECONDS       = FTE_TASK_WATCHDOG_TIME / 1000;
    
    _time_to_ticks(&xTime, &xTicks);
    while(1)
    {
      _watchdog_start_ticks(&xTicks);
      _time_delay(FTE_TASK_WATCHDOG_TIME - 1000);
    }      
}

