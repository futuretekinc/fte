#include <string.h>
#include "board.h"
#include "radio.h"
#include "fte_lorawan_config.h"
#include "fte_lorawan.h"
#include "fte_list.h"
#include "LoRaMac.h"

/**
 * Main application entry point.
 */
const FTE_LORAWAN_CONFIG  xDefaultConfig = 
{
    .pDevEui        =   { 0x00, 0x40, 0x5c, 0x01, 0x02, 0x03, 0x04, 0x05},        // Device IEEE EUI
    .pAppEui        =   { 0x00, },        // Application IEEE EUI
    .pAppKey        =   { 0x00, },
        
    .xMAC = 
    {
        .NetID      =   0x00000001,
        .DevAddr    =   0x12345678,

        .NwkSKey    =   
        {
            0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
            0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
        },
        .AppSKey    =   
        {
            0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
            0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
        },
        .Datarates      =   { 12, 11, 10, 9, 8, 7, 7, 50},
        .TxPowers       =   { 20, 14, 11,  8,  5,  2 },
        
        .DeviceClass    =   CLASS_C,   
        .PublicNetwork  =   false,
        
         .ChannelsTxPower=  LORAMAC_DEFAULT_TX_POWER,
        
        .Frequency      =   FTE_LORAWAN_RF_FREQUENCY,
        .OutputPower    =   FTE_LORAWAN_TX_OUTPUT_POWER,
        .Bandwidth      =   FTE_LORAWAN_BANDWITDH,
        .SpreadingFactor=   FTE_LORAWAN_SF7,
        .CodingRate     =   FTE_LORAWAN_CODING_RATE,       
        .PreambleLength =   FTE_LORAWAN_PREAMBLE_LENGTH,
        .RxTimeout      =   FTE_LORAWAN_RX_TIMEOUT_VALUE,
        .PayloadCRC     =   true,
        .FixedLength    =   false,
        .FrequencyHopOn =   false,
        .HopPeriod      =   0,
        .InvertedIQ     =   false,
        .TxTimeout      =   3000000,
        
        .Rx2Channel     =   
        {
            .Frequency  =   920000000, 
            .Datarate   =   DR_6 
        },
        
        .MaxRxWindow    =   MAX_RX_WINDOW,

        .DutyCycle      =   FTE_LORAWAN_TX_DUTYCYCLE,
        .DutyCycleRange =   FTE_LORAWAN_TX_DUTYCYCLE_RND,

        .ChannelsNbRep   =   1,
        .ChannelsDefaultDatarate = DR_6,
        .ChannelsMask   =   LC(4),

        .Bands          =
        {
            { 1, TX_POWER_14_DBM, 0,  0 } //  100.0 %
        },
        .Channels       =
        {
            { 917000000, { ( ( DR_6 << 4 ) | DR_2 ) }, 0 },
            { 918000000, { ( ( DR_6 << 4 ) | DR_2 ) }, 0 },
            { 919000000, { ( ( DR_6 << 4 ) | DR_2 ) }, 0 },
            { 920000000, { ( ( DR_6 << 4 ) | DR_2 ) }, 0 },
            { 921000000, { ( ( DR_6 << 4 ) | DR_2 ) }, 0 },
            { 922000000, { ( ( DR_6 << 4 ) | DR_2 ) }, 0 },
            { 923000000, { ( ( DR_6 << 4 ) | DR_2 ) }, 0 }
        },
        .Debug =
        {
            .TrafficMonitorOn    = true,
            .PacketDumpOn        = false,
            .PacketDumpColumnSize= 16
        },
        .Bypass = true,
    },
    
    .bOverTheAirActivation  =   true,
    .ulOverTheAirActivationDutyCycle = 10000000,    // 10 [s] value in us
        
};

