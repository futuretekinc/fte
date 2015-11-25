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
    id[0] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[1] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[2] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[3] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[4] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[5] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[6] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[7] = ( ( *( uint32_t* )ID2 ) );
}

uint8_t BoardMeasureBatterieLevel( void )
{
    return 0x1F;
}



int_32  FTE_LORA_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean     print_usage, shorthelp = FALSE;
    int_32      return_code = SHELL_EXIT_SUCCESS;
   
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {  
#if 0
                printf("%16s : %s\n", "Op Mode", FTE_LORA_getOpModeString(SX1276LoRaGetOpMode( )));
                printf("%16s : %d\n", "RF Frequency", SX1276LoRaGetRFFrequency( ));
                printf("%16s : %d\n", "Spreading Factor", SX1276LoRaGetSpreadingFactor( )); // SF6 only operates in implicit header mode.
                printf("%16s : %d\n", "Error Coding", SX1276LoRaGetErrorCoding( ));
                printf("%16s : %d\n", "Packet CRC ON", SX1276LoRaGetPacketCrcOn( ));
                printf("%16s : %d\n", "Bandwidth", SX1276LoRaGetSignalBandwidth( ));
                printf("%16s : %d dBm\n", "RSSI", (int_32)SX1276LoRaGetPacketRssi( ));
                printf("%16s : %d\n", "Rx Packets", SX1276LoRaGetRxPacketCount());
                printf("%16s : %d\n", "Tx Packets", SX1276LoRaGetTxPacketCount());
#endif
            }
            break;
           
        case    3:
            {
#if 0               
                if (strcmp(argv[1], "send") == 0)
                {
                    FTE_LORA_send((uint_8_ptr)argv[2], strlen(argv[2]));
                }
#endif
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
            printf ("%s [<command>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<command>]\n", argv[0]);
            printf("  Commands:\n");
            printf("  Parameters:\n");
        }
    }
    return   return_code;
}