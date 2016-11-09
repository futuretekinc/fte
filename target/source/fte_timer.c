#include "fte_target.h"
#include "fte_timer.h"

/*!
 * Timers list head FTE_VOID_PTR
 */
static FTE_TIMER_EVENT_PTR pTimerListHead = NULL;

/*!
 * \brief Adds or replace the head timer of the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be become the new head
 * \param [IN]  ulRemainingTime Remaining time of the pPrevious head to be replaced
 */
static void FTE_TIMER_insertNewHeadTimer( FTE_TIMER_EVENT_PTR obj, FTE_UINT32 ulRemainingTime );

/*!
 * \brief Adds a timer to the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be added to the list
 * \param [IN]  ulRemainingTime Remaining time of the running head after which the object may be added
 */
static void FTE_TIMER_insertTimer( FTE_TIMER_EVENT_PTR obj, FTE_UINT32 ulRemainingTime );

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
static 
FTE_BOOL FTE_TIMER_exists
(
   FTE_TIMER_EVENT_PTR obj 
);

/*!
 * \brief Read the timer value of the pCurrentrently running timer
 *
 * \retval value pCurrentrent timer value
 */
FTE_UINT32 FTE_TIMER_getValue( void );


FTE_RET     FTE_TIMER_cancel
(
    FTE_TIMER_ID_PTR    pTimerID
)
{
    ASSERT(pTimerID != NULL);
    
    FTE_RET xRet = FTE_RET_OK;
    
    if (*pTimerID != 0)
    {
        xRet = _timer_cancel(*pTimerID);
        if (xRet == FTE_RET_OK)
        {
            *pTimerID = 0;
        }
    }
    
    return  xRet;
}

FTE_RET FTE_TIMER_startOneshotAtTicks
(
    TIMER_NOTIFICATION_TICK_FPTR    fTimeout, 
    FTE_VOID_PTR        pData,
    FTE_UINT32          ulMode,
    MQX_TICK_STRUCT_PTR pTicks,
    FTE_TIMER_ID_PTR    pTimerID
)
{
    ASSERT((fTimeout != NULL) && (pTicks != NULL) && (pTimerID != NULL));
    
    *pTimerID = _timer_start_oneshot_at_ticks(fTimeout, pData, ulMode, pTicks);
    
    if (*pTimerID == 0)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    return  FTE_RET_OK;
}
    
FTE_RET FTE_TIMER_startOneshotAfter
(
    TIMER_NOTIFICATION_TIME_FPTR    fTimeout, 
    FTE_VOID_PTR        pData,
    FTE_UINT32          ulMilliSecs,
    FTE_TIMER_ID_PTR    pTimerID
)
{
    *pTimerID = _timer_start_oneshot_after(fTimeout, pData, TIMER_ELAPSED_TIME_MODE, ulMilliSecs);
    
    if (*pTimerID == 0)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    return  FTE_RET_OK;
}
    
void FTE_TIMER_init( FTE_TIMER_EVENT_PTR obj, void ( *callback )( void ) )
{
    obj->ulTimestamp = 0;
    obj->ulReloadValue = 0;
    obj->bIsRunning = FALSE;
    obj->fCallback = callback;
    obj->Next = NULL;
}

void FTE_TIMER_start( FTE_TIMER_EVENT_PTR pEvent )
{
    FTE_UINT32 ulElapsedTime = 0;
    FTE_UINT32 ulRemainingTime = 0;

    //__disable_irq( );

    if( ( pEvent == NULL ) || ( FTE_TIMER_exists( pEvent ) == TRUE ) )
    {
        //__enable_irq( );
        return;
    }


    DEBUG("FTE_TIMER_start\n");
    pEvent->ulTimestamp = pEvent->ulReloadValue;
    pEvent->bIsRunning = FALSE;

    if( pTimerListHead == NULL )
    {
        FTE_TIMER_insertNewHeadTimer( pEvent, pEvent->ulTimestamp );
    }
    else 
    {
        if( pTimerListHead->bIsRunning == TRUE )
        {
            ulElapsedTime = FTE_TIMER_getValue( );
            if( ulElapsedTime > pTimerListHead->ulTimestamp )
            {
                ulElapsedTime = pTimerListHead->ulTimestamp; // sepCurrentity but should never ocpCurrent
            }
            ulRemainingTime = pTimerListHead->ulTimestamp - ulElapsedTime;
            _timer_cancel(pEvent->xID);
            pEvent->xID = 0;
        }
        else
        {
            ulRemainingTime = pTimerListHead->ulTimestamp;
        }
    
        if( pEvent->ulTimestamp < ulRemainingTime )
        {
            FTE_TIMER_insertNewHeadTimer( pEvent, ulRemainingTime );
        }
        else
        {
             FTE_TIMER_insertTimer( pEvent, ulRemainingTime );
        }
    }
    //__enable_irq( );
}

void FTE_TIMER_insertTimer
(
    FTE_TIMER_EVENT_PTR pEvent, 
    FTE_UINT32          ulRemainingTime 
)
{
    FTE_UINT32 aggregatedTimestamp = 0;      // hold the sum of timestamps 
    FTE_UINT32 aggregatedTimestampNext = 0;  // hold the sum of timestamps up to the next event

    FTE_TIMER_EVENT_PTR pPrev = pTimerListHead;
    FTE_TIMER_EVENT_PTR pCurrent = pTimerListHead->Next;

    if( pCurrent == NULL )
    { // pEvent comes just after the head
        pEvent->ulTimestamp -= ulRemainingTime;
        pPrev->Next = pEvent;
        pEvent->Next = NULL;
    }
    else
    {
        aggregatedTimestamp = ulRemainingTime;
        aggregatedTimestampNext = ulRemainingTime + pCurrent->ulTimestamp;

        while( pPrev != NULL )
        {
            if( aggregatedTimestampNext > pEvent->ulTimestamp )
            {
                pEvent->ulTimestamp -= aggregatedTimestamp;
                if( pCurrent != NULL )
                {
                    pCurrent->ulTimestamp -= pEvent->ulTimestamp;
                }
                pPrev->Next = pEvent;
                pEvent->Next = pCurrent;
                break;
            }
            else
            {
                pPrev = pCurrent;
                pCurrent = pCurrent->Next;
                if( pCurrent == NULL )
                { // pEvent comes at the end of the list
                    aggregatedTimestamp = aggregatedTimestampNext;
                    pEvent->ulTimestamp -= aggregatedTimestamp;
                    pPrev->Next = pEvent;
                    pEvent->Next = NULL;
                    break;
                }
                else
                {
                    aggregatedTimestamp = aggregatedTimestampNext;
                    aggregatedTimestampNext = aggregatedTimestampNext + pCurrent->ulTimestamp;
                }
            }
        }
    }
}

void FTE_TIMER_insertNewHeadTimer
(   FTE_TIMER_EVENT_PTR  pEvent, 
   FTE_UINT32           ulRemainingTime 
)
{
    FTE_TIMER_EVENT_PTR pCurrent = pTimerListHead;

    if( pCurrent != NULL )
    {
        pCurrent->ulTimestamp = ulRemainingTime - pEvent->ulTimestamp;
        pCurrent->bIsRunning = FALSE;
    }

    pEvent->Next = pCurrent;
    pEvent->bIsRunning = TRUE;
    pTimerListHead = pEvent;
    FTE_TIMER_setTimeout( pTimerListHead );
}

void FTE_TIMER_handler
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_UINT32 ulElapsedTime = 0;
 
    DEBUG("FTE_TIMER_hander[%d]\n", xTimerID);
    if( pTimerListHead == NULL )
    {
        return;  // Only necessary when the standard timer is used as a time base
    }

    ulElapsedTime = FTE_TIMER_getValue( );

    FTE_TIMER_EVENT_PTR ulElapsedTimer = NULL;

    if( ulElapsedTime > pTimerListHead->ulTimestamp )
    {
        pTimerListHead->ulTimestamp = 0;
    }
    else
    {
        pTimerListHead->ulTimestamp -= ulElapsedTime;
    }
        
    // save pTimerListHead
    ulElapsedTimer = pTimerListHead;

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
    while( ( ulElapsedTimer != NULL ) && ( ulElapsedTimer->ulTimestamp == 0 ) )
    {
        if( ulElapsedTimer->fCallback != NULL )
        {
            ulElapsedTimer->fCallback( );
        }
        ulElapsedTimer = ulElapsedTimer->Next;
    }

    // start the next pTimerListHead if it exists
    if( pTimerListHead != NULL )
    {    
        pTimerListHead->bIsRunning = TRUE;
        FTE_TIMER_setTimeout( pTimerListHead );
    } 
}

void FTE_TIMER_stop
(
    FTE_TIMER_EVENT_PTR pEvent 
) 
{
    //__disable_irq( );

    FTE_UINT32 ulElapsedTime = 0;
    FTE_UINT32 ulRemainingTime = 0;

    FTE_TIMER_EVENT_PTR pPrev = pTimerListHead;
    FTE_TIMER_EVENT_PTR pCurrent = pTimerListHead;

    // List is empty or the Obj to stop does not exist 
    if( ( pTimerListHead == NULL ) || ( pEvent == NULL ) )
    {
        //__enable_irq( );
        return;
    }

    if( pTimerListHead == pEvent ) // Stop the Head                                    
    {
        if( pTimerListHead->bIsRunning == TRUE ) // The head is already running 
        {
            ulElapsedTime = FTE_TIMER_getValue( );
            if( ulElapsedTime > pEvent->ulTimestamp )
            {
                ulElapsedTime = pEvent->ulTimestamp;
            }
        
            ulRemainingTime = pEvent->ulTimestamp - ulElapsedTime;
        
            if( pTimerListHead->Next != NULL )
            {
                pTimerListHead->bIsRunning = FALSE;
                pTimerListHead = pTimerListHead->Next;
                pTimerListHead->ulTimestamp += ulRemainingTime;
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
                ulRemainingTime = pEvent->ulTimestamp;
                pTimerListHead = pTimerListHead->Next;
                pTimerListHead->ulTimestamp += ulRemainingTime;
            }
            else
            {
                pTimerListHead = NULL;
            }
        }
    }
    else // Stop an object within the list
    {    
        ulRemainingTime = pEvent->ulTimestamp;
        
        while( pCurrent != NULL )
        {
            if( pCurrent == pEvent )
            {
                if( pCurrent->Next != NULL )
                {
                    pCurrent = pCurrent->Next;
                    pPrev->Next = pCurrent;
                    pCurrent->ulTimestamp += ulRemainingTime;
                }
                else
                {
                    pCurrent = NULL;
                    pPrev->Next = pCurrent;
                }
                break;
            }
            else
            {
                pPrev = pCurrent;
                pCurrent = pCurrent->Next;
            }
        }   
    }
    //__enable_irq( );
}    
    
FTE_BOOL FTE_TIMER_exists
(
    FTE_TIMER_EVENT_PTR pEvent 
)
{
    FTE_TIMER_EVENT_PTR pCurrent = pTimerListHead;

    while( pCurrent != NULL )
    {
        if( pCurrent == pEvent )
        {
            return TRUE;
        }
        pCurrent = pCurrent->Next;
    }
    return FALSE;
}

void FTE_TIMER_reset
(
    FTE_TIMER_EVENT_PTR     pEvent 
)
{
    FTE_TIMER_stop( pEvent );
    FTE_TIMER_start( pEvent );
}

void FTE_TIMER_setValue
(   FTE_TIMER_EVENT_PTR  pEvent, 
   FTE_UINT32           ulValue
)
{
    FTE_UINT32 ulMinValue = 0;

    FTE_TIMER_stop( pEvent );

    ulMinValue = 1000;
    
    if( ulValue < ulMinValue )
    {
        ulValue = ulMinValue;
    }

    pEvent->ulTimestamp = ulValue;
    pEvent->ulReloadValue = ulValue;
}

FTE_UINT32 FTE_TIMER_getValue( void )
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

void FTE_TIMER_setTimeout
(   FTE_TIMER_EVENT_PTR pEvent
)
{
    MQX_TICK_STRUCT     xTicks;            

    _time_init_ticks(&xTicks, _time_get_ticks_per_sec() * pEvent->ulTimestamp / 1000000);    
    pEvent->xID = _timer_start_oneshot_after_ticks(FTE_TIMER_handler, NULL, TIMER_ELAPSED_TIME_MODE, &xTicks);
}

