#ifndef _FTE_TIMER_H__
#define _FTE_TIMER_H__

#include "fte_type.h"

typedef FTE_UINT32  FTE_TIMER_ID, _PTR_ FTE_TIMER_ID_PTR;

typedef struct FTE_TIMER_EVENT_STRUCT
{
    FTE_UINT32 ulTimestamp;                //! Current timer value
    FTE_UINT32 ulReloadValue;              //! Timer delay value
    FTE_BOOL bIsRunning;                 //! Is the timer currently running
    void    ( *fCallback )( void );     //! Timer IRQ callback function
    struct FTE_TIMER_EVENT_STRUCT _PTR_ Next;//! Pointer to the next Timer object.
    FTE_UINT32 xID;
}   FTE_TIMER_EVENT, _PTR_ FTE_TIMER_EVENT_PTR;


FTE_RET FTE_TIMER_cancel(FTE_TIMER_ID_PTR pTimerID);
FTE_RET FTE_TIMER_startOneshotAtTicks
(
    TIMER_NOTIFICATION_TICK_FPTR    fTimeout, 
    FTE_VOID_PTR        pData,
    FTE_UINT32          ulMode,
    MQX_TICK_STRUCT_PTR pTicks,
    FTE_TIMER_ID_PTR    pTimerID
);

FTE_RET FTE_TIMER_startOneshotAfter
(
    TIMER_NOTIFICATION_TIME_FPTR    fTimeout, 
    FTE_VOID_PTR        pData,
    FTE_UINT32          ulMilliSecs,
    FTE_TIMER_ID_PTR    pTimerID
);

/*!
 * \brief Timer time variable definition
 */
#ifndef FTE_TIMER_TIME
typedef uint_64 FTE_TIMER_TIME;
#endif


/*!
 * \brief Enables/Disables low power timers usage
 *
 * \param [IN] enable [true]RTC timer used, [false]Normal timer used
 */
void FTE_TIMER_setLowPowerEnable( FTE_BOOL enable );

/*!
 * \brief Initializes the timer object
 *
 * \retval enable [true]RTC timer used, [false]Normal timer used
 */
FTE_BOOL FTE_TIMER_getLowPowerEnable( void );

/*!
 * \brief Initializes the timer object
 *
 * \remark TimerSetValue function must be called before starting the timer.
 *         this function initializes timestamp and reload value at 0.
 *
 * \param [IN] obj          Structure containing the timer object parameters
 * \param [IN] callback     Function callback called at the end of the timeout
 */
void FTE_TIMER_init( FTE_TIMER_EVENT_PTR pObj, void ( *callback )( void ) );

/*!
 * Timer IRQ event handler
 */
void FTE_TIMER_irqHandler( void );

/*!
 * \brief Starts and adds the timer object to the list of timer events
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void FTE_TIMER_start( FTE_TIMER_EVENT_PTR pObj );

/*!
 * \brief Stops and removes the timer object from the list of timer events
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void FTE_TIMER_stop( FTE_TIMER_EVENT_PTR pObj );

/*!
 * \brief Resets the timer object
 *
 * \param [IN] obj Structure containing the timer object parameters
 */
void FTE_TIMER_reset( FTE_TIMER_EVENT_PTR pObj );

/*!
 * \brief Set timer new timeout value
 *
 * \param [IN] obj   Structure containing the timer object parameters
 * \param [IN] value New timer timeout value
 */
void FTE_TIMER_setValue( FTE_TIMER_EVENT_PTR pObj, FTE_UINT32 value );

/*!
 * \brief Read the current time
 *
 * \retval time returns current time
 */
FTE_TIMER_TIME  FTE_TIMER_getCurrentTime( void );

/*!
 * \brief Manages the entry into ARM cortex deep-sleep mode
 */
void FTE_TIMER_lowPowerHandler( void );

#endif
