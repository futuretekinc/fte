#include "fte_target.h"
#include <lwtimer.h>
#include "sys/fte_sys_timer.h"

typedef struct
{
    LWTIMER_STRUCT      xLWTimer; 
    int_32              nInterval;
    int_32              nStartDelay;
    int_32              nRemain;
    LWTIMER_ISR_FPTR    fCallback;
    pointer             pParams;
}   FTE_TIMER, _PTR_ FTE_TIMER_PTR;

LWTIMER_PERIOD_STRUCT _hPeriodicTimer;

void print_time(void)
{ 
    TIME_STRUCT     xTime;
    DATE_STRUCT     xDate;
   
    _time_get(&xTime);
    _time_to_date (&xTime, &xDate);
    printf("%04d-%02d-%02d %02d:%02d:%02d:%03d", xDate.YEAR, xDate.MONTH, xDate.DAY, xDate.HOUR, xDate.MINUTE, xDate.SECOND, xDate.MILLISEC);
}

void check_tick(pointer a)
{
    print_time();
    printf(" - TICK\n");
}

void light_on(pointer a)
{
    print_time();
    printf(" - ON\n");
}

void light_off(pointer a)
{
    print_time();
    printf(" - OFF\n");
}

void _fte_timer_isr(pointer pParams);

_mqx_uint   fte_timer_init(uint_32 nBaseInterval)
{
    return  _lwtimer_create_periodic_queue(&_hPeriodicTimer, _time_get_ticks_per_sec() * nBaseInterval / 1000, _time_get_ticks_per_sec());
}


uint_32 fte_timer_add(uint_32 nInterval, uint_32 nStartDelay, LWTIMER_ISR_FPTR fCallback, pointer pParams)
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
    
    _lwtimer_add_timer_to_queue(&_hPeriodicTimer, &pTimer->xLWTimer, pTimer->nStartDelay, _fte_timer_isr, pTimer);
    
    return  (uint_32)pTimer;
}

_mqx_uint   fte_timer_cancel(uint_32 hTimer)
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

void _fte_timer_isr(pointer pParams)
{
    FTE_TIMER_PTR   pTimer = (FTE_TIMER_PTR)pParams;
    
    if (--pTimer->nRemain <= 0)
    {
        pTimer->nRemain = pTimer->nInterval;
        pTimer->fCallback(pTimer->pParams);
    }
}
