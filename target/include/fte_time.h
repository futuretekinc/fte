#ifndef __FTE_TIME_H__
#define __FTE_TIME_H__

typedef struct FTE_TIMER_EVENT_STRUCT
{
    uint_32 ulTimestamp;                //! Current timer value
    uint_32 ulReloadValue;              //! Timer delay value
    boolean bIsRunning;                 //! Is the timer currently running
    void    ( *fCallback )( void );     //! Timer IRQ callback function
    struct FTE_TIMER_EVENT_STRUCT *Next;//! Pointer to the next Timer object.
    uint_32 xID;
}   FTE_TIMER_EVENT, _PTR_ FTE_TIMER_EVENT_PTR;

typedef struct  FTE_TIME_DELAY_STRUCT
{
    MQX_TICK_STRUCT xNextTicks;
    uint_32         ulDelayMS;          //! milliseconds
}   FTE_TIME_DELAY, _PTR_ FTE_TIME_DELAY_PTR;

_mqx_uint   FTE_TIME_init(void);

extern  uint_32     FTE_TIME_diff(TIME_STRUCT *time1, TIME_STRUCT *time);
extern  uint_32     FTE_TIME_diffMilliseconds(TIME_STRUCT *time1, TIME_STRUCT *time);
extern  uint_32     FTE_TIME_toString(TIME_STRUCT *time, char_ptr pBuff, uint_32 nBuffLen);
extern  uint_32     FTE_TIME_toTime(char_ptr pBuff, TIME_STRUCT *pTime);

extern  uint_64     FTE_TIME_getMilliSeconds(void);

extern  _mqx_uint   FTE_TIME_DELAY_init(FTE_TIME_DELAY_PTR pObj, uint_32 ulDelayMS);
extern  void        FTE_TIME_DELAY_waitingAndSetNext(FTE_TIME_DELAY_PTR pObj);

extern  int_32      FTE_TIME_SHELL_cmd(int_32 argc, char_ptr argv[] );


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
void FTE_TIMER_setLowPowerEnable( boolean enable );

/*!
 * \brief Initializes the timer object
 *
 * \retval enable [true]RTC timer used, [false]Normal timer used
 */
boolean FTE_TIMER_getLowPowerEnable( void );

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
void FTE_TIMER_setValue( FTE_TIMER_EVENT_PTR pObj, uint_32 value );

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