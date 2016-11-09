#ifndef __FTE_TIME_H__
#define __FTE_TIME_H__

typedef struct  FTE_TIME_DELAY_STRUCT
{
    MQX_TICK_STRUCT xNextTicks;
    FTE_UINT32         ulDelayMS;          //! milliseconds
}   FTE_TIME_DELAY, _PTR_ FTE_TIME_DELAY_PTR;

typedef TIME_STRUCT FTE_TIME, _PTR_ FTE_TIME_PTR;

FTE_RET FTE_TIME_init
(
    void
);

FTE_RET FTE_TIME_getCurrent
(   
    FTE_TIME_PTR    pTime
);

FTE_RET FTE_TIME_diff
(
    FTE_TIME_PTR    pTime1, 
    FTE_TIME_PTR    pTime2,
    FTE_INT32_PTR   pDiffSec
);

FTE_RET FTE_TIME_diffMilliseconds
(
    FTE_TIME_PTR    pTime1, 
    FTE_TIME_PTR    pTime2,
    FTE_INT32_PTR   pDiffMSec
);

FTE_RET FTE_TIME_toStr
(
    FTE_TIME_PTR    pTime, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      nBuffLen
);

FTE_RET FTE_TIME_fromStr
(
    FTE_TIME_PTR    pTime,
    FTE_CHAR_PTR    pBuff 
);

FTE_RET FTE_TIME_getMilliSeconds
(
    FTE_UINT64_PTR  pMSec
);

FTE_RET FTE_TIME_DELAY_init
(
    FTE_TIME_DELAY_PTR  pObj, 
    FTE_UINT32          ulDelayMS
);

FTE_RET FTE_TIME_DELAY_waitingAndSetNext
(
    FTE_TIME_DELAY_PTR pObj
);

FTE_INT32   FTE_TIME_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
);




#endif