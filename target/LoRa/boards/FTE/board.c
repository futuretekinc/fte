/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

void BoardInitMcu( void )
{
    if( McuInitialized == false )
    {
        SX1276IoInit( );

#if( LOW_POWER_MODE_ENABLE )
        TimerSetLowPowerEnable( true );
#else
        TimerSetLowPowerEnable( false );
#endif

        if( TimerGetLowPowerEnable( ) == true )
        {
//            RtcInit( );
        }
        else
        {
//            TimerHwInit( );
        }
        McuInitialized = true;
    }
}

void BoardDeInitMcu( void )
{
    SX1276IoDeInit( );
  
    McuInitialized = false;
}

void BoardGetUniqueId( uint8_t *id )
{
    id[0] = 0x00;
    id[1] = 0x40;
    id[2] = 0x5c;
    id[3] = 0xFF;
    id[4] = 0xFA;
    id[5] = 0x12;
    id[6] = 0x00;
    id[7] = 0x00;
}

uint8_t BoardMeasureBatterieLevel( void )
{
    return 0x1F;
}

