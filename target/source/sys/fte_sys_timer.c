#include "fte_target.h"
#include "sys/fte_sys_timer.h"

static 
void FTE_SYS_TIMER_ISR(FTE_VOID_PTR pParams);


LWTIMER_PERIOD_STRUCT _hPeriodicTimer;

FTE_RET   FTE_SYS_TIMER_init
(
    FTE_UINT32  nBaseInterval
)
{
    return  _lwtimer_create_periodic_queue(&_hPeriodicTimer, _time_get_ticks_per_sec() * nBaseInterval / 1000, _time_get_ticks_per_sec());
}


FTE_UINT32 FTE_SYS_TIMER_add
(
    FTE_UINT32      nInterval, 
    FTE_UINT32      nStartDelay, 
    LWTIMER_ISR_FPTR    fCallback, 
    FTE_VOID_PTR    pParams
)
{
    FTE_SYS_TIMER_PTR   pTimer = (FTE_SYS_TIMER_PTR)FTE_MEM_allocZero(sizeof(FTE_SYS_TIMER));
    if (pTimer == NULL)
    {
        return  0;
    }
 
    pTimer->nInterval   = nInterval;
    pTimer->nStartDelay = _time_get_ticks_per_sec() * nStartDelay / 1000;
    pTimer->nRemain     = nInterval;
    pTimer->fCallback   = fCallback;
    pTimer->pParams     = pParams;
    
    _lwtimer_add_timer_to_queue(&_hPeriodicTimer, &pTimer->xLWTimer, pTimer->nStartDelay, FTE_SYS_TIMER_ISR, pTimer);
    
    return  (FTE_UINT32)pTimer;
}

FTE_RET   FTE_SYS_TIMER_cancel
(
    FTE_UINT32  hTimer
)
{
    FTE_SYS_TIMER_PTR pTimer = (FTE_SYS_TIMER_PTR)hTimer;
    
    if (pTimer == NULL)
    {
        return  FTE_RET_ERROR;
    }

    _lwtimer_cancel_timer(&pTimer->xLWTimer);
    
    FTE_MEM_free(pTimer);
    
    return  FTE_RET_OK;
}

void FTE_SYS_TIMER_ISR
(
    FTE_VOID_PTR    pParams
)
{
    FTE_SYS_TIMER_PTR   pTimer = (FTE_SYS_TIMER_PTR)pParams;
    
    if (--pTimer->nRemain <= 0)
    {
        pTimer->nRemain = pTimer->nInterval;
        pTimer->fCallback(pTimer->pParams);
    }
}
