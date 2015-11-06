#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include <stdlib.h>
#include <timer.h>
#include "fte_sys.h"
#include "fte_time.h"
#include "fte_debug.h"

_mqx_uint   FTE_TIME_init(void)
{
    _rtc_init(RTC_INIT_FLAG_ENABLE);    
    
    return  MQX_OK;
}

uint_32     FTE_TIME_diff(TIME_STRUCT_PTR pTime1, TIME_STRUCT_PTR pTime2)
{
    MQX_TICK_STRUCT xTick1, xTick2;
    boolean         bOverflow = FALSE;
    int_32          nDiffTime;
    
    _time_to_ticks(pTime1, &xTick1);
    _time_to_ticks(pTime2, &xTick2);
    
    nDiffTime = _time_diff_seconds(&xTick1, &xTick2, &bOverflow);
    
    if (bOverflow)
    {
        nDiffTime = _time_diff_seconds(&xTick2, &xTick1, &bOverflow);
    }
    
    return  nDiffTime;
}

uint_32     FTE_TIME_diffMilliseconds(TIME_STRUCT_PTR pTime1, TIME_STRUCT_PTR pTime2)
{
    MQX_TICK_STRUCT xTick1, xTick2;
    boolean         bOverflow = FALSE;
    int_32          nDiffTime;
    
    _time_to_ticks(pTime1, &xTick1);
    _time_to_ticks(pTime2, &xTick2);
    
    nDiffTime = _time_diff_milliseconds(&xTick1, &xTick2, &bOverflow);
    
    if (bOverflow)
    {
        nDiffTime = _time_diff_milliseconds(&xTick2, &xTick1, &bOverflow);
    }
    
    return  nDiffTime;
}

uint_32 FTE_TIME_toString(TIME_STRUCT *pTime, char_ptr pBuff, uint_32 nBuffLen)
{
    DATE_STRUCT     xDate;
   
    _time_to_date (pTime, &xDate);
    return  snprintf(pBuff, nBuffLen, "%04d-%02d-%02d %02d:%02d:%02d", xDate.YEAR, xDate.MONTH, xDate.DAY, xDate.HOUR, xDate.MINUTE, xDate.SECOND);
}

uint_32 FTE_TIME_toTime(char_ptr pBuff, TIME_STRUCT *pTime)
{
    DATE_STRUCT xDate;
    char_ptr    pEnd, pStart = pBuff;
    
    pEnd = strchr(pStart, '-');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.YEAR = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, '-');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.MONTH = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ' ');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.DAY = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ':');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.HOUR = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ':');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.MINUTE = atoi(pStart);
    pStart = pEnd + 1;
    
    xDate.SECOND = atoi(pStart);

    _time_from_date (&xDate, pTime);
    
    return  MQX_OK;
}

uint_64     FTE_TIME_getMilliSeconds(void)
{
    TIME_STRUCT         xTime;

    _time_get(&xTime);
    
    return  xTime.SECONDS * 1000 + xTime.MILLISECONDS;
}

int_32  FTE_TIME_SHELL_cmd(int_32 argc, char_ptr argv[] )
{ /* Body */
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        uint_32 tmp;
        
        switch(argc)
        {
        case    1:
            { 
                TIME_STRUCT             xTime;
                char                    pBuff[30];

#if BSPCFG_ENABLE_RTCDEV                
                RTC_TIME_STRUCT         xRTC;
                
                _rtc_get_time (&xRTC);
                _rtc_time_to_mqx_time (&xRTC, &xTime);
#else                
                _time_get (&xTime);
#endif
                FTE_TIME_toString(&xTime, pBuff, sizeof(pBuff));
                printf("%s\n", pBuff);
            }
            break;
            
        case    2:
            {
                char            buff[15];
                DATE_STRUCT     xDate;
                TIME_STRUCT     xTime;
                
                if (!Shell_parse_number( argv[1], &tmp) || (strlen(argv[1]) != 14))
                {
                    print_usage = TRUE;
                }
                else
                {
                    strcpy(buff, argv[1]);
                    
                    xDate.MILLISEC= 0;
                    Shell_parse_number( &buff[12], &tmp);
                    if (tmp > 60)
                    {
                        print_usage = TRUE;
                    }
                    else
                    {
                        xDate.SECOND = tmp;
                        buff[12] = 0;
                        Shell_parse_number( &buff[10], &tmp);
                        if (tmp > 60)
                        {
                            print_usage = TRUE;
                        }
                        else
                        {
                            xDate.MINUTE = tmp;
                            buff[10] = 0;
                            Shell_parse_number( &buff[8], &tmp);
                            if (tmp > 23)
                            {
                                print_usage = TRUE;
                            }
                            else
                            {
                                xDate.HOUR = tmp;
                                buff[8] = 0;
                                Shell_parse_number( &buff[6], &tmp);
                                if (tmp > 31)
                                {
                                    print_usage = TRUE;
                                }
                                else
                                {
                                    xDate.DAY = tmp;
                                    buff[6] = 0;
                                    Shell_parse_number( &buff[4], &tmp);
                                    if (tmp > 12)
                                    {
                                        print_usage = TRUE;
                                    }
                                    else
                                    {
                                        xDate.MONTH = tmp;
                                         buff[4] = 0;
                                        Shell_parse_number( &buff[0], &tmp);
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
                                            if( _rtc_sync_with_mqx(FALSE) != MQX_OK )
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
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [yyyymmddHHMMSS]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [yyyymmddHHMMSS]\n", argv[0]);
            printf("        yyyy - Year\n");
            printf("        mm   - Numeric month, a number from 1 to 12.\n");
            printf("        dd   - Day, a number from 1 to 31.\n");
            printf("        HH   - Hour, a number from 0 to 23.\n");
            printf("        MM   - Minutes, a number from 0 to 59.\n");
            printf("        SS   - Seconds, a number from 0 to 59.\n");
        }
    }
    return   return_code;
}


/*!
 * Timers list head pointer
 */
static FTE_TIMER_EVENT_PTR pTimerListHead = NULL;

/*!
 * \brief Adds or replace the head timer of the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be become the new head
 * \param [IN]  remainingTime Remaining time of the previous head to be replaced
 */
static void FTE_TIMER_insertNewHeadTimer( FTE_TIMER_EVENT_PTR obj, uint_32 remainingTime );

/*!
 * \brief Adds a timer to the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be added to the list
 * \param [IN]  remainingTime Remaining time of the running head after which the object may be added
 */
static void FTE_TIMER_insertTimer( FTE_TIMER_EVENT_PTR obj, uint_32 remainingTime );

/*!
 * \brief Sets a timeout with the duration "timestamp"
 * 
 * \param [IN] timestamp Delay duration
 */
static void FTE_TIMER_setTimeout( FTE_TIMER_EVENT_PTR obj );

/*!
 * \brief Check if the Object to be added is not already in the list
 * 
 * \param [IN] timestamp Delay duration
 * \retval TRUE (the object is already in the list) or FALSE  
 */
static boolean FTE_TIMER_exists( FTE_TIMER_EVENT_PTR obj );

/*!
 * \brief Read the timer value of the currently running timer
 *
 * \retval value current timer value
 */
uint_32 FTE_TIMER_getValue( void );

void FTE_TIMER_init( FTE_TIMER_EVENT_PTR obj, void ( *callback )( void ) )
{
    obj->ulTimestamp = 0;
    obj->ulReloadValue = 0;
    obj->bIsRunning = FALSE;
    obj->fCallback = callback;
    obj->Next = NULL;
}

void FTE_TIMER_start( FTE_TIMER_EVENT_PTR obj )
{
    uint_32 elapsedTime = 0;
    uint_32 remainingTime = 0;

    //__disable_irq( );

    if( ( obj == NULL ) || ( FTE_TIMER_exists( obj ) == TRUE ) )
    {
        //__enable_irq( );
        return;
    }


    DEBUG("FTE_TIMER_start\n");
    obj->ulTimestamp = obj->ulReloadValue;
    obj->bIsRunning = FALSE;

    if( pTimerListHead == NULL )
    {
        FTE_TIMER_insertNewHeadTimer( obj, obj->ulTimestamp );
    }
    else 
    {
        if( pTimerListHead->bIsRunning == TRUE )
        {
            elapsedTime = FTE_TIMER_getValue( );
            if( elapsedTime > pTimerListHead->ulTimestamp )
            {
                elapsedTime = pTimerListHead->ulTimestamp; // security but should never occur
            }
            remainingTime = pTimerListHead->ulTimestamp - elapsedTime;
            _timer_cancel(obj->xID);
            obj->xID = 0;
        }
        else
        {
            remainingTime = pTimerListHead->ulTimestamp;
        }
    
        if( obj->ulTimestamp < remainingTime )
        {
            FTE_TIMER_insertNewHeadTimer( obj, remainingTime );
        }
        else
        {
             FTE_TIMER_insertTimer( obj, remainingTime );
        }
    }
    //__enable_irq( );
}

static void FTE_TIMER_insertTimer( FTE_TIMER_EVENT_PTR obj, uint_32 remainingTime )
{
    uint_32 aggregatedTimestamp = 0;      // hold the sum of timestamps 
    uint_32 aggregatedTimestampNext = 0;  // hold the sum of timestamps up to the next event

    FTE_TIMER_EVENT_PTR prev = pTimerListHead;
    FTE_TIMER_EVENT_PTR cur = pTimerListHead->Next;

    if( cur == NULL )
    { // obj comes just after the head
        obj->ulTimestamp -= remainingTime;
        prev->Next = obj;
        obj->Next = NULL;
    }
    else
    {
        aggregatedTimestamp = remainingTime;
        aggregatedTimestampNext = remainingTime + cur->ulTimestamp;

        while( prev != NULL )
        {
            if( aggregatedTimestampNext > obj->ulTimestamp )
            {
                obj->ulTimestamp -= aggregatedTimestamp;
                if( cur != NULL )
                {
                    cur->ulTimestamp -= obj->ulTimestamp;
                }
                prev->Next = obj;
                obj->Next = cur;
                break;
            }
            else
            {
                prev = cur;
                cur = cur->Next;
                if( cur == NULL )
                { // obj comes at the end of the list
                    aggregatedTimestamp = aggregatedTimestampNext;
                    obj->ulTimestamp -= aggregatedTimestamp;
                    prev->Next = obj;
                    obj->Next = NULL;
                    break;
                }
                else
                {
                    aggregatedTimestamp = aggregatedTimestampNext;
                    aggregatedTimestampNext = aggregatedTimestampNext + cur->ulTimestamp;
                }
            }
        }
    }
}

static void FTE_TIMER_insertNewHeadTimer( FTE_TIMER_EVENT_PTR obj, uint_32 remainingTime )
{
    FTE_TIMER_EVENT_PTR cur = pTimerListHead;

    if( cur != NULL )
    {
        cur->ulTimestamp = remainingTime - obj->ulTimestamp;
        cur->bIsRunning = FALSE;
    }

    obj->Next = cur;
    obj->bIsRunning = TRUE;
    pTimerListHead = obj;
    FTE_TIMER_setTimeout( pTimerListHead );
}

void FTE_TIMER_handler(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    uint_32 elapsedTime = 0;
 
    DEBUG("FTE_TIMER_hander[%d]\n", id);
    if( pTimerListHead == NULL )
    {
        return;  // Only necessary when the standard timer is used as a time base
    }

    elapsedTime = FTE_TIMER_getValue( );

    FTE_TIMER_EVENT_PTR elapsedTimer = NULL;

    if( elapsedTime > pTimerListHead->ulTimestamp )
    {
        pTimerListHead->ulTimestamp = 0;
    }
    else
    {
        pTimerListHead->ulTimestamp -= elapsedTime;
    }
        
    // save pTimerListHead
    elapsedTimer = pTimerListHead;

    // remove all the expired object from the list
    while( ( pTimerListHead != NULL ) && ( pTimerListHead->ulTimestamp == 0 ) )
    {         
        if( pTimerListHead->Next != NULL )
        {
            pTimerListHead = pTimerListHead->Next;
        }
        else
        {
            pTimerListHead = NULL;
        }
    }

    // execute the callbacks of all the expired objects
    // this is to avoid potential issues between the callback and the object list
    while( ( elapsedTimer != NULL ) && ( elapsedTimer->ulTimestamp == 0 ) )
    {
        if( elapsedTimer->fCallback != NULL )
        {
            elapsedTimer->fCallback( );
        }
        elapsedTimer = elapsedTimer->Next;
    }

    // start the next pTimerListHead if it exists
    if( pTimerListHead != NULL )
    {    
        pTimerListHead->bIsRunning = TRUE;
        FTE_TIMER_setTimeout( pTimerListHead );
    } 
}

void FTE_TIMER_stop( FTE_TIMER_EVENT_PTR obj ) 
{
    //__disable_irq( );

    uint_32 elapsedTime = 0;
    uint_32 remainingTime = 0;

    FTE_TIMER_EVENT_PTR prev = pTimerListHead;
    FTE_TIMER_EVENT_PTR cur = pTimerListHead;

    // List is empty or the Obj to stop does not exist 
    if( ( pTimerListHead == NULL ) || ( obj == NULL ) )
    {
        //__enable_irq( );
        return;
    }

    if( pTimerListHead == obj ) // Stop the Head                                    
    {
        if( pTimerListHead->bIsRunning == TRUE ) // The head is already running 
        {
            elapsedTime = FTE_TIMER_getValue( );
            if( elapsedTime > obj->ulTimestamp )
            {
                elapsedTime = obj->ulTimestamp;
            }
        
            remainingTime = obj->ulTimestamp - elapsedTime;
        
            if( pTimerListHead->Next != NULL )
            {
                pTimerListHead->bIsRunning = FALSE;
                pTimerListHead = pTimerListHead->Next;
                pTimerListHead->ulTimestamp += remainingTime;
                pTimerListHead->bIsRunning = TRUE;
                FTE_TIMER_setTimeout( pTimerListHead );
            }
            else
            {
                pTimerListHead = NULL;
            }
        }
        else // Stop the head before it is started
        {     
            if( pTimerListHead->Next != NULL )     
            {
                remainingTime = obj->ulTimestamp;
                pTimerListHead = pTimerListHead->Next;
                pTimerListHead->ulTimestamp += remainingTime;
            }
            else
            {
                pTimerListHead = NULL;
            }
        }
    }
    else // Stop an object within the list
    {    
        remainingTime = obj->ulTimestamp;
        
        while( cur != NULL )
        {
            if( cur == obj )
            {
                if( cur->Next != NULL )
                {
                    cur = cur->Next;
                    prev->Next = cur;
                    cur->ulTimestamp += remainingTime;
                }
                else
                {
                    cur = NULL;
                    prev->Next = cur;
                }
                break;
            }
            else
            {
                prev = cur;
                cur = cur->Next;
            }
        }   
    }
    //__enable_irq( );
}    
    
static boolean FTE_TIMER_exists( FTE_TIMER_EVENT_PTR obj )
{
    FTE_TIMER_EVENT_PTR cur = pTimerListHead;

    while( cur != NULL )
    {
        if( cur == obj )
        {
            return TRUE;
        }
        cur = cur->Next;
    }
    return FALSE;
}

void FTE_TIMER_reset( FTE_TIMER_EVENT_PTR obj )
{
    FTE_TIMER_stop( obj );
    FTE_TIMER_start( obj );
}

void FTE_TIMER_setValue( FTE_TIMER_EVENT_PTR pObj, uint_32 ulValue)
{
    uint_32 ulMinValue = 0;

    FTE_TIMER_stop( pObj );

    ulMinValue = 1000;
    
    if( ulValue < ulMinValue )
    {
        ulValue = ulMinValue;
    }

    pObj->ulTimestamp = ulValue;
    pObj->ulReloadValue = ulValue;
}

uint_32 FTE_TIMER_getValue( void )
{
    TIME_STRUCT xTime;
    
    _time_get_elapsed(&xTime);
    
    return (FTE_TIMER_TIME)xTime.SECONDS * 1000000 + xTime.MILLISECONDS*1000;
}

FTE_TIMER_TIME FTE_TIMER_getCurrentTime( void )
{
    TIME_STRUCT xTime;
    
    _time_get(&xTime);
    
    return (FTE_TIMER_TIME)xTime.SECONDS * 1000000 + xTime.MILLISECONDS*1000;
}

static void FTE_TIMER_setTimeout( FTE_TIMER_EVENT_PTR obj )
{
    MQX_TICK_STRUCT     xTicks;            
    _timer_id           xID;
    _time_init_ticks(&xTicks, _time_get_ticks_per_sec() * obj->ulTimestamp / 1000000);    
    obj->xID = _timer_start_oneshot_after_ticks(FTE_TIMER_handler, NULL, TIMER_ELAPSED_TIME_MODE, &xTicks);
    DEBUG("setTimeout[%d]\n", xID);
}

