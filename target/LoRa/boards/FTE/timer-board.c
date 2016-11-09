/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: MCU RTC timer and low power modes management

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <math.h>
#include "board.h"
#include "timer-board.h"
#include "fte_sys_timer.h"

/*!
 * Hardware Time base in us
 */
#define HW_TIMER_TIME_BASE                              10000 //us

/*!
 * Hardware Timer tick counter
 */
TimerTime_t TimerTickCounter = 1;

/*!
 * Saved value of the Tick counter at the start of the next event
 */
static TimerTime_t TimerTickCounterContext = 0;

/*!
 * Value trigging the IRQ
 */
volatile TimerTime_t TimeoutCntValue = 0;

/*!
 * Increment the Hardware Timer tick counter
 */
void TimerIncrementTickCounter( void );

/*!
 * Counter used for the Delay operations
 */
volatile uint32_t TimerDelayCounter = 0;

/*!
 * Retunr the value of the counter used for a Delay
 */
uint32_t TimerHwGetDelayValue( void );

/*!
 * Increment the value of TimerDelayCounter
 */
void TimerIncrementDelayCounter( void );

void TIM2_IRQHandler( FTE_TIMER_ID xTimerID, FTE_VOID_PTR pParams, FTE_UINT32 a, FTE_UINT32  b);

FTE_TIMER_ID _xHWTimer;


void TimerHwInit( void )
{  
}

void TimerHwDeInit( void )
{
}

uint32_t TimerHwGetMinimumTimeout( void )
{
    return (uint32_t)( ceil( 2 * HW_TIMER_TIME_BASE ) );
}

void TimerHwStart( uint32_t val )
{
    if (_xHWTimer == 0)
    {
        _xHWTimer = _timer_start_periodic_every(TIM2_IRQHandler, 0, TIMER_ELAPSED_TIME_MODE, 10);  
    }
   
    TimerTickCounterContext = TimerHwGetTimerValue( );

    if( val <= HW_TIMER_TIME_BASE + 1 )
    {
        TimeoutCntValue = TimerTickCounterContext + 1;
    }
    else
    {
        TimeoutCntValue = TimerTickCounterContext + ( ( val - 1 ) / HW_TIMER_TIME_BASE );
    }
}

void TimerHwStop( void )
{
    _timer_cancel(_xHWTimer);
    _xHWTimer = 0;
}

void TimerHwDelayMs( uint32_t delay )
{
    _time_delay(delay);
}

TimerTime_t TimerHwGetElapsedTime( void )
{
     return( ( ( TimerHwGetTimerValue( ) - TimerTickCounterContext ) + 1 )  * HW_TIMER_TIME_BASE );
}

TimerTime_t TimerHwGetTimerValue( void )
{
    TimerTime_t val = 0;

    __disable_irq( );

    val = TimerTickCounter;

    __enable_irq( );

    return( val );
}

TimerTime_t TimerHwGetTime( void )
{

    return TimerHwGetTimerValue( ) * HW_TIMER_TIME_BASE;
}

uint32_t TimerHwGetDelayValue( void )
{
    uint32_t val = 0;

    __disable_irq( );

    val = TimerDelayCounter;

    __enable_irq( );

    return( val );
}

void TimerIncrementTickCounter( void )
{
    __disable_irq( );

    TimerTickCounter++;

    __enable_irq( );
}

/*!
 * Timer IRQ handler
 */
void TIM2_IRQHandler( FTE_TIMER_ID xTimerID, FTE_VOID_PTR pParams, FTE_UINT32 a, FTE_UINT32  b)
{
    TimerIncrementTickCounter( );

    if( TimerTickCounter == TimeoutCntValue )
    {
        TimerIrqHandler( );
    }
}

/*!
 * Timer IRQ handler
 */
void TIM3_IRQHandler( void )
{
}

void TimerHwEnterLowPowerStopMode( void )
{
#ifndef USE_DEBUGGER
    __WFI( );
#endif
}
