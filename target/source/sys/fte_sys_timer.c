#include "fte_target.h"
#include <lwtimer.h>
#include "sys/fte_sys_timer.h"

typedef struct
{
    LWTIMER_STRUCT      xLWTimer; 
    FTE_INT32           nInterval;
    FTE_INT32           nStartDelay;
    FTE_INT32           nRemain;
    LWTIMER_ISR_FPTR    fCallback;
    FTE_VOID_PTR        pParams;
}   FTE_TIMER, _PTR_ FTE_TIMER_PTR;

static 
void FTE_TIMER_ISR(FTE_VOID_PTR pParams);


LWTIMER_PERIOD_STRUCT _hPeriodicTimer;

void print_time(void)
{ 
    TIME_STRUCT     xTime;
    DATE_STRUCT     xDate;
   
    _time_get(&xTime);
    _time_to_date (&xTime, &xDate);
    printf("%04d-%02d-%02d %02d:%02d:%02d:%03d", xDate.YEAR, xDate.MONTH, xDate.DAY, xDate.HOUR, xDate.MINUTE, xDate.SECOND, xDate.MILLISEC);
}

void check_tick
(
    FTE_VOID_PTR    a
)
{
    print_time();
    printf(" - TICK\n");
}

void light_on
(
    FTE_VOID_PTR    a
)
{
    print_time();
    printf(" - ON\n");
}

void light_off
(
    FTE_VOID_PTR    a
)
{
    print_time();
    printf(" - OFF\n");
}

FTE_RET   fte_timer_init
(
    FTE_UINT32  nBaseInterval
)
{
    return  _lwtimer_create_periodic_queue(&_hPeriodicTimer, _time_get_ticks_per_sec() * nBaseInterval / 1000, _time_get_ticks_per_sec());
}


FTE_UINT32 fte_timer_add
(
    FTE_UINT32  nInterval, 
    FTE_UINT32  nStartDelay, 
    LWTIMER_ISR_FPTR    fCallback, 
    FTE_VOID_PTR    pParams
)
{
    FTE_TIMER_PTR   pTimer = (FTE_TIMER_PTR)FTE_MEM_allocZero(sizeof(FTE_TIMER));
    if (pTimer == NULL)
    {
        return  0;
    }
 
    pTimer->nInterval   = nInterval;
    pTimer->nStartDelay = _time_get_ticks_per_sec() * nStartDelay / 1000;
    pTimer->nRemain     = nInterval;
    pTimer->fCallback   = fCallback;
    pTimer->pParams     = pParams;
    
    _lwtimer_add_timer_to_queue(&_hPeriodicTimer, &pTimer->xLWTimer, pTimer->nStartDelay, FTE_TIMER_ISR, pTimer);
    
    return  (FTE_UINT32)pTimer;
}

FTE_RET   fte_timer_cancel
(
    FTE_UINT32  hTimer
)
{
    FTE_TIMER_PTR pTimer = (FTE_TIMER_PTR)hTimer;
    
    if (pTimer == NULL)
    {
        return  MQX_ERROR;
    }

    _lwtimer_cancel_timer(&pTimer->xLWTimer);
    
    FTE_MEM_free(pTimer);
    
    return  MQX_OK;
}

void FTE_TIMER_ISR
(
    FTE_VOID_PTR    pParams
)
{
    FTE_TIMER_PTR   pTimer = (FTE_TIMER_PTR)pParams;
    
    if (--pTimer->nRemain <= 0)
    {
        pTimer->nRemain = pTimer->nInterval;
        pTimer->fCallback(pTimer->pParams);
    }
}
