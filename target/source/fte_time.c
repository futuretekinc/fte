#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include <stdlib.h>
#include <timer.h>
#include "fte_type.h"
#include "fte_sys.h"
#include "fte_time.h"
#include "fte_sys_timer.h"
#include "fte_debug.h"
#include "fte_assert.h"
#include "fte_utils.h"

FTE_RET FTE_TIME_init
(
    FTE_VOID
)
{
    _rtc_init(RTC_INIT_FLAG_ENABLE);    
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_getCurrent
(   
    FTE_TIME_PTR pTime
)
{
    ASSERT(pTime != NULL);
    
    _time_get(pTime);

    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_diff
(   
    FTE_TIME_PTR    pTime1, 
    FTE_TIME_PTR    pTime2, 
    FTE_INT32_PTR   pDiffSec
)
{
    ASSERT((pTime1 != NULL) && (pTime2 != NULL) && (pDiffSec != NULL));
    
    MQX_TICK_STRUCT xTick1, xTick2;
    FTE_BOOL        bOverflow = FALSE;
    FTE_INT32       nDiffTime;
    
    _time_to_ticks(pTime1, &xTick1);
    _time_to_ticks(pTime2, &xTick2);
    
    nDiffTime = _time_diff_seconds(&xTick1, &xTick2, &bOverflow);
    
    if (bOverflow)
    {
        nDiffTime = _time_diff_seconds(&xTick2, &xTick1, &bOverflow);
    }
    
    *pDiffSec = nDiffTime;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_diffMilliseconds
(
    FTE_TIME_PTR    pTime1, 
    FTE_TIME_PTR    pTime2,
    FTE_INT32_PTR   pDiffMSec
)
{
     ASSERT((pTime1 != NULL) && (pTime2 != NULL) && (pDiffMSec != NULL));
    
    MQX_TICK_STRUCT xTick1, xTick2;
    FTE_BOOL        bOverflow = FALSE;
    FTE_INT32       nDiffTime;
    
    _time_to_ticks(pTime1, &xTick1);
    _time_to_ticks(pTime2, &xTick2);
    
    nDiffTime = _time_diff_milliseconds(&xTick1, &xTick2, &bOverflow);
    
    if (bOverflow)
    {
        nDiffTime = _time_diff_milliseconds(&xTick2, &xTick1, &bOverflow);
    }
    
    *pDiffMSec = nDiffTime;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_toStr
(
    FTE_TIME_PTR    pTime, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      nBuffLen
)
{
    ASSERT((pTime != NULL) && (pBuff != NULL));
    
    DATE_STRUCT     xDate;
   
    _time_to_date (pTime, &xDate);
    
    snprintf(pBuff, nBuffLen, "%04d-%02d-%02d %02d:%02d:%02d", xDate.YEAR, xDate.MONTH, xDate.DAY, xDate.HOUR, xDate.MINUTE, xDate.SECOND);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_fromStr
(
    FTE_TIME_PTR    pTime,
    FTE_CHAR_PTR    pBuff 
)
{
    DATE_STRUCT xDate;
    FTE_CHAR_PTR    pEnd, pStart = pBuff;
    
    pEnd = strchr(pStart, '-');
    if (pEnd == NULL)
    {
        return  FTE_RET_ERROR;
    }
    *pEnd = '\0';
    xDate.YEAR = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, '-');
    if (pEnd == NULL)
    {
        return  FTE_RET_ERROR;
    }
    *pEnd = '\0';
    xDate.MONTH = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ' ');
    if (pEnd == NULL)
    {
        return  FTE_RET_ERROR;
    }
    *pEnd = '\0';
    xDate.DAY = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ':');
    if (pEnd == NULL)
    {
        return  FTE_RET_ERROR;
    }
    *pEnd = '\0';
    xDate.HOUR = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ':');
    if (pEnd == NULL)
    {
        return  FTE_RET_ERROR;
    }
    *pEnd = '\0';
    xDate.MINUTE = atoi(pStart);
    pStart = pEnd + 1;
    
    xDate.SECOND = atoi(pStart);

    _time_from_date (&xDate, pTime);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_getMilliSeconds
(
    FTE_UINT64_PTR  pMSec
)
{
    ASSERT(pMSec != NULL);
    
    TIME_STRUCT         xTime;

    _time_get(&xTime);
    
    *pMSec = xTime.SECONDS * 1000 + xTime.MILLISECONDS;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_DELAY_init
(
    FTE_TIME_DELAY_PTR  pObj, 
    FTE_UINT32          ulDelayMS
)
{
    ASSERT(pObj != NULL);
    
    _time_get_elapsed_ticks(&pObj->xNextTicks);        
    pObj->ulDelayMS = ulDelayMS;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_TIME_DELAY_waitingAndSetNext
(
    FTE_TIME_DELAY_PTR pObj
)
{
    ASSERT(pObj != NULL);
    
    MQX_TICK_STRUCT xCurrentTicks;
    FTE_BOOL    bOverflow = FALSE;
    FTE_INT32   nDiffTime;
    
    ASSERT(pObj != NULL);
    
    _time_get_elapsed_ticks(&xCurrentTicks);        
    nDiffTime = _time_diff_milliseconds(&pObj->xNextTicks, &xCurrentTicks, &bOverflow);
    if ((nDiffTime > 0) && (!bOverflow))
    {
        _time_delay(nDiffTime);
    }
    else
    {
        _time_get_elapsed_ticks(&pObj->xNextTicks);
    }
    
    _time_add_msec_to_ticks(&pObj->xNextTicks, pObj->ulDelayMS);
    
    return  FTE_RET_OK;
}

FTE_INT32   FTE_TIME_SHELL_cmd
(
    FTE_INT32   nArgc, 
    FTE_CHAR_PTR pArgv[]
)
{ /* Body */
    FTE_BOOL    print_usage, shorthelp = FALSE;
    FTE_INT32   nRet = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (nArgc, pArgv, &shorthelp);

    if (!print_usage)
    {
        FTE_UINT32 tmp;
        
        switch(nArgc)
        {
        case    1:
            { 
                TIME_STRUCT xTime;
                FTE_CHAR    pBuff[30];

#if BSPCFG_ENABLE_RTCDEV                
                RTC_TIME_STRUCT         xRTC;
                
                _rtc_get_time (&xRTC);
                _rtc_time_to_mqx_time (&xRTC, &xTime);
#else                
                _time_get (&xTime);
#endif
                FTE_TIME_toStr(&xTime, pBuff, sizeof(pBuff));
                printf("%s\n", pBuff);
            }
            break;
            
        case    2:
            {
                FTE_CHAR        pBuff[15];
                DATE_STRUCT     xDate;
                TIME_STRUCT     xTime;
                
                if (FTE_strToUINT32( pArgv[1], &tmp) != FTE_RET_OK || (strlen(pArgv[1]) != 14))
                {
                    print_usage = TRUE;
                }
                else
                {
                    strcpy(pBuff, pArgv[1]);
                    
                    xDate.MILLISEC= 0;
                    FTE_strToUINT32( &pBuff[12], &tmp);
                    if (tmp > 60)
                    {
                        print_usage = TRUE;
                    }
                    else
                    {
                        xDate.SECOND = tmp;
                        pBuff[12] = 0;
                        FTE_strToUINT32( &pBuff[10], &tmp);
                        if (tmp > 60)
                        {
                            print_usage = TRUE;
                        }
                        else
                        {
                            xDate.MINUTE = tmp;
                            pBuff[10] = 0;
                            FTE_strToUINT32( &pBuff[8], &tmp);
                            if (tmp > 23)
                            {
                                print_usage = TRUE;
                            }
                            else
                            {
                                xDate.HOUR = tmp;
                                pBuff[8] = 0;
                                FTE_strToUINT32( &pBuff[6], &tmp);
                                if (tmp > 31)
                                {
                                    print_usage = TRUE;
                                }
                                else
                                {
                                    xDate.DAY = tmp;
                                    pBuff[6] = 0;
                                    FTE_strToUINT32( &pBuff[4], &tmp);
                                    if (tmp > 12)
                                    {
                                        print_usage = TRUE;
                                    }
                                    else
                                    {
                                        xDate.MONTH = tmp;
                                         pBuff[4] = 0;
                                        FTE_strToUINT32( &pBuff[0], &tmp);
                                        if (tmp < 2010)
                                        {
                                            print_usage = TRUE;
                                        }
                                        else
                                        {
                                            xDate.YEAR = tmp;
                                         
                                            _time_from_date (&xDate, &xTime);
                                            _time_set(&xTime);
                                            //_rtc_init(RTC_INIT_FLAG_CLEAR | RTC_INIT_FLAG_ENABLE);
                                            _rtc_init(RTC_INIT_FLAG_ENABLE);
                                            if( _rtc_sync_with_mqx(FALSE) != FTE_RET_OK )
                                            {
                                                printf("\nError synchronize time!\n");
                                            }
                                            
                                            //FTE_SYS_reset();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }                
            }
            break;
            
        default:
            print_usage = TRUE;
        }
    }
    
    if (print_usage || (nRet !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [yyyymmddHHMMSS]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [yyyymmddHHMMSS]\n", pArgv[0]);
            printf("        yyyy - Year\n");
            printf("        mm   - Numeric month, a number from 1 to 12.\n");
            printf("        dd   - Day, a number from 1 to 31.\n");
            printf("        HH   - Hour, a number from 0 to 23.\n");
            printf("        MM   - Minutes, a number from 0 to 59.\n");
            printf("        SS   - Seconds, a number from 0 to 59.\n");
        }
    }
    return   nRet;
}


