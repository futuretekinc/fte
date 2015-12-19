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
#include <time.h>
#include "board.h"
#include "rtc.h"
#include "rtc-board.h"

/*!
 * RTC Time base in us
 */
#define RTC_ALARM_TIME_BASE                             122.07

/*!
 * MCU Wake Up Time
 */
#define MCU_WAKE_UP_TIME                                3400

/*!
 * \brief Configure the Rtc hardware
 */
static void RtcSetConfig( void );

/*!
 * \brief Configure the Rtc Alarm
 */
static void RtcSetAlarmConfig( void );

/*!
 * \brief Start the Rtc Alarm (time base 1s)
 */
static void RtcStartWakeUpAlarm( uint32_t timeoutValue );

/*!
 * \brief Read the MCU internal Calendar value
 *
 * \retval Calendar value
 */
static TimerTime_t RtcGetCalendarValue( void );

/*!
 * \brief Clear the RTC flags and Stop all IRQs
 */
static void RtcClearStatus( void );

/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
static bool RtcInitalized = false;

/*!
 * \brief Flag to indicate if the timestamps until the next event is long enough
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the LowPower Mode even if the timestamps until the
 * next event is long enough to allow Low Power mode
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 */
static TimerTime_t RtcTimerContext = 0;


void RtcInit( void )
{
    if( RtcInitalized == false )
    {
        RtcSetConfig( );
        RtcSetAlarmConfig( );
        RtcInitalized = true;
    }
}

static void RtcSetConfig( void )
{
}

static void RtcSetAlarmConfig( void )
{
}

void RtcStopTimer( void )
{
    RtcClearStatus( );
}

uint32_t RtcGetMinimumTimeout( void )
{
    return (uint32_t)( ceil( 3 * RTC_ALARM_TIME_BASE ) );
}

void RtcSetTimeout( uint32_t timeout )
{
    uint32_t timeoutValue = 0;

    timeoutValue = timeout;

    if( timeoutValue < ( 3 * RTC_ALARM_TIME_BASE ) )
    {
        timeoutValue = (uint32_t)(3 * RTC_ALARM_TIME_BASE);
    }
  
    if( timeoutValue < 55000 )
    {
        // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        RtcTimerEventAllowsLowPower = false;
    }
    else
    {
        RtcTimerEventAllowsLowPower = true;
    }

    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        timeoutValue = timeoutValue - MCU_WAKE_UP_TIME;
    }

    RtcStartWakeUpAlarm( timeoutValue );
}


uint32_t RtcGetTimerElapsedTime( void )
{
    TimerTime_t CalendarValue = 0;

    CalendarValue = RtcGetCalendarValue( );

    return( ( uint32_t )( ceil ( ( ( CalendarValue - RtcTimerContext ) + 2 ) * RTC_ALARM_TIME_BASE ) ) );
}

TimerTime_t RtcGetTimerValue( void )
{
    return  RtcGetCalendarValue( );
}

static void RtcClearStatus( void )
{
    _rtc_clear_requests(RTC_RTCISR_ALM | RTC_RTCIENR_ALM);
}

static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
    RTC_TIME_STRUCT xRTCTime;
  
    xRTCTime.seconds = timeoutValue / 1000;
  
    _rtc_set_alarm(&xRTCTime);
}

void RtcEnterLowPowerStopMode( void )
{ 
    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    { 
        // Disable IRQ while the MCU is being deinitialized to prevent race issues
        __disable_irq( );
  
        BoardDeInitMcu( );
  
        __enable_irq( );
    }
}

void RtcRecoverMcuStatus( void )
{
}

/*!
 * \brief RTC IRQ Handler on the RTC Alarm
 */
void RTC_Alarm_IRQHandler( void )
{
    //TimerIrqHandler( );
}

void BlockLowPowerDuringTask( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus( );
    }
    LowPowerDisableDuringTask = status;
}

void RtcDelayMs( uint32_t delay )
{
    _time_delay(delay);
}

TimerTime_t RtcGetCalendarValue( void )
{
    uint_64  ulTimeValue = FTE_TIME_getMilliSeconds();
  
    return  (uint32_t)(ulTimeValue / 1000);
}
