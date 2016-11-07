#ifndef _FTE_SYS_TIMER_H__
#define _FTE_SYS_TIMER_H__

FTE_RET     fte_timer_init(FTE_UINT32 nBaseInterval);
FTE_UINT32  fte_timer_add(FTE_UINT32 nInterval, FTE_UINT32 nStartDelay, LWTIMER_ISR_FPTR fCallback, pointer pParams);
FTE_RET     fte_timer_cancel(FTE_UINT32 hTimer);


#endif
