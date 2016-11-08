#ifndef _FTE_SYS_TIMER_H__
#define _FTE_SYS_TIMER_H__

FTE_RET     FTE_SYS_TIMER_init(FTE_UINT32 nBaseInterval);
FTE_UINT32  FTE_SYS_TIMER_add(FTE_UINT32 nInterval, FTE_UINT32 nStartDelay, LWTIMER_ISR_FPTR fCallback, pointer pParams);
FTE_RET     FTE_SYS_TIMER_cancel(FTE_UINT32 hTimer);


#endif
