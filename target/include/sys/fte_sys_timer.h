#ifndef _FTE_SYS_TIMER_H__
#define _FTE_SYS_TIMER_H__

_mqx_uint   fte_timer_init(uint_32 nBaseInterval);
uint_32     fte_timer_add(uint_32 nInterval, uint_32 nStartDelay, LWTIMER_ISR_FPTR fCallback, pointer pParams);
_mqx_uint   fte_timer_cancel(uint_32 hTimer);


#endif
