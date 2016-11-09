#ifndef _FTE_SYS_TIMER_H__
#define _FTE_SYS_TIMER_H__

#include <lwtimer.h>
#include "fte_type.h"

typedef struct
{
    LWTIMER_STRUCT      xLWTimer; 
    FTE_INT32           nInterval;
    FTE_INT32           nStartDelay;
    FTE_INT32           nRemain;
    LWTIMER_ISR_FPTR    fCallback;
    FTE_VOID_PTR        pParams;
}   FTE_SYS_TIMER, _PTR_ FTE_SYS_TIMER_PTR;

FTE_RET     FTE_SYS_TIMER_init(FTE_UINT32 nBaseInterval);
FTE_UINT32  FTE_SYS_TIMER_add(FTE_UINT32 nInterval, FTE_UINT32 nStartDelay, LWTIMER_ISR_FPTR fCallback, pointer pParams);
FTE_RET     FTE_SYS_TIMER_cancel(FTE_UINT32 hTimer);


#endif
