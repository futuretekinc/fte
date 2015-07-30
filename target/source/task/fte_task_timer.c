#include "fte_target.h"
#include "fte_sys.h"
#include "fte_config.h"
#include <watchdog.h>
/*TASK*-----------------------------------------------------------------
*
* Function Name  : FTE_TASK_timer
* Returned Value : void
* Comments       : 
*
*END------------------------------------------------------------------*/
 
extern  void _FTE_DI_ISR(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);

void FTE_TASK_timer(uint_32 params)
{
#if FTE_FACTORY_RESET_SUPPORTED
    static  uint_32 ulFactoryResetPushedTime = 0;
#endif
    MQX_TICK_STRUCT xBaseTick;
    MQX_TICK_STRUCT xCurrentTick;
    int_32          nDiffTime;
    boolean         bOverflow;
    
    _time_get_ticks(&xBaseTick);
    
    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id()); 
    
    while(1)
    {
        _FTE_DI_ISR(0, 0, 0);
      
#if FTE_FACTORY_RESET_SUPPORTED
        if (FTE_SYS_isfactoryResetPushed())
        {
            if (++ulFactoryResetPushedTime > FTE_FACTORY_RESET_DETECT_TIME * 10)
            {
                FTE_CFG_clear();
                _time_delay(100);
                FTE_SYS_reset();
            }
        }
        else
        {
            ulFactoryResetPushedTime = 0;
        }
#endif
        _time_add_msec_to_ticks(&xBaseTick, 100);
        _time_get_ticks(&xCurrentTick);
        
        nDiffTime = _time_diff_milliseconds(&xBaseTick, &xCurrentTick, &bOverflow);
        if ((0 < nDiffTime) && (nDiffTime <= 100))
        {
            _time_delay(nDiffTime);
        }
        else
        {
            _time_get_ticks(&xBaseTick);
            _time_delay(100);            
        }
    }      
}

