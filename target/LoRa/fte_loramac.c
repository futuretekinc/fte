/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"
#include "fte_lorawan_config.h"
#include "fte_lorawan.h"
#include "fte_list.h"
#include "fte_loramac.h"

#if 1 //FTE_LORAMAC_SUPPORTED

#undef  __MODULE__
#define __MODULE__  FTE_MODULE_NET_LORA

typedef struct
{
    FTE_UINT32                 ulSize;
    uint_8                  pBuffer[FTE_LORAMAC_BUFFER_SIZE];
}   FTE_LORAMAC_FRAME, _PTR_ FTE_LORAMAC_FRAME_PTR;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

void FTE_LORAMAC_onTxDone( void * params );
void FTE_LORAMAC_onRxDone( void * params, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void FTE_LORAMAC_onTxTimeout( void * params );
void FTE_LORAMAC_onRxTimeout( void * params );
void FTE_LORAMAC_onRxError( void * params );

void SX1276OnDio0Irq( void *);
void SX1276OnDio1Irq( void *);
void SX1276OnDio2Irq( void *);
void SX1276OnDio3Irq( void *);

FTE_LORAMAC_CONFIG  xDefaultConfig = 
{
    .DevEui     =   { 0x00, 0x40, 0x5c, 0x01, 0x02, 0x03, 0x04, 0x05},        // Device IEEE EUI
    .AppEui     =   NULL,        // Application IEEE EUI
    .AppKey     =   NULL,
    
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
//    .Datarates      =   {12, 11, 10, 9, 8, 7, 7, 50},
    .Datarates      =   {7, 7, 7, 7, 7, 7, 7, 50},
    .TxPowers       =   { 20, 14, 11,  8,  5,  2 },
    
    .xDeviceClass   =   CLASS_C,   
    
    .ulFrequency    =   920000000,
    .OutputPower    =   FTE_LORAWAN_TX_OUTPUT_POWER,
    .xBandwidth     =   FTE_LORA_BANDWITDH_250KHZ,
    .xSF            =   FTE_LORA_SF7,
    .xCR            =   FTE_LORA_CR_4_6,       
    .PreambleLength =   8,
    .RxTimeout      =   FTE_LORAWAN_RX_TIMEOUT_VALUE,
    .bPayloadCRC    =   false,

    
    .DutyCycle      =   FTE_LORAWAN_TX_DUTYCYCLE,
    .DutyCycleRange =   FTE_LORAWAN_TX_DUTYCYCLE_RND,

    .ChannelsMask   =   0x000000001,
};

extern  FTE_LORAMAC_PTR pLoRaMac = NULL;

FTE_LORAMAC_PTR FTE_LORAMAC_init( FTE_LORAMAC_CONFIG_PTR pConfig )
{
    FTE_DEBUG_traceOn(DEBUG_NET_LORA);

    if (pConfig == NULL)
    {
        pConfig = (void *)&xDefaultConfig ;
    }
    
    // Target board initialisation
    BoardInitMcu( );

    if (pLoRaMac == NULL)
    {
        pLoRaMac = (FTE_LORAMAC_PTR)FTE_MEM_allocZero(sizeof(FTE_LORAMAC));
        if (pLoRaMac == NULL)
        {
            return  NULL;
        }
    }
    
    pLoRaMac->pConfig    = pConfig;
    FTE_LIST_init(&pLoRaMac->xTxPktList);

    FTE_TASK_create(FTE_TASK_LORAMAC, (FTE_UINT32)pLoRaMac, NULL);

    return  pLoRaMac;
}

_mqx_uint FTE_LORAMAC_send(FTE_LORAMAC_PTR pLoRaMac, void *pBuff, FTE_UINT32 ulLen)
{
    FTE_LORAMAC_FRAME_PTR   pFrame = NULL;
    
    ASSERT((pLoRaMac != NULL) && (pBuff != NULL));
    
    if ((ulLen == 0) || (ulLen > FTE_LORAMAC_BUFFER_SIZE))
    {
        return  FTE_RET_ERROR;
    }

    if (FTE_LIST_count(&pLoRaMac->xTxPktList) >= 10)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    pFrame = FTE_MEM_allocZero(sizeof(FTE_LORAMAC_FRAME));
    if (pFrame == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    memcpy(pFrame->pBuffer, pBuff, ulLen);
    pFrame->ulSize = ulLen;
    
    if (FTE_LIST_pushBack(&pLoRaMac->xTxPktList, pFrame) != FTE_RET_OK)
    {
        FTE_MEM_free(pFrame);
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

void FTE_LORAMAC_process(FTE_UINT32 ulParams)
{
    pLoRaMac = (FTE_LORAMAC_PTR)ulParams;
    
    ASSERT(pLoRaMac != NULL);
    
    FTE_LORAMAC_CONFIG_PTR  pConfig = pLoRaMac->pConfig;

    // Radio initialization
    RadioEvents.TxDone =    FTE_LORAMAC_onTxDone;
    RadioEvents.RxDone =    FTE_LORAMAC_onRxDone;
    RadioEvents.TxTimeout = FTE_LORAMAC_onTxTimeout;
    RadioEvents.RxTimeout = FTE_LORAMAC_onRxTimeout;
    RadioEvents.RxError =   FTE_LORAMAC_onRxError;

    Radio.Init( &RadioEvents );

    FTE_LORAMAC_setConfig(pConfig);

    Radio.Rx( pConfig->RxTimeout );
    
    FTE_TASK_create(FTE_TASK_LORA_CTRL, (FTE_UINT32)pLoRaMac, NULL);
    
    pLoRaMac->xState = FTE_LORAMAC_STATE_TX;
    
    while( TRUE )
    {
        switch( pLoRaMac->xState )
        {
        case FTE_LORAMAC_STATE_RX:
        case FTE_LORAMAC_STATE_RX_TIMEOUT:
        case FTE_LORAMAC_STATE_RX_ERROR:
            {
                if (FTE_LIST_count(&pLoRaMac->xTxPktList) != 0)
                {
                    FTE_LORAMAC_FRAME_PTR pFrame;
                    
                    if (FTE_LIST_popFront(&pLoRaMac->xTxPktList, (void **)&pFrame) == FTE_RET_OK)
                    {
                        Radio.Send(pFrame->pBuffer, pFrame->ulSize);
                        FTE_MEM_free(pFrame);
                    }
                    else
                    {                
                        Radio.Rx( pLoRaMac->pConfig->RxTimeout );
                        pLoRaMac->xState = FTE_LORAMAC_STATE_LOW_POWER;
                    }
                }
                else
                {                
                    Radio.Rx( pLoRaMac->pConfig->RxTimeout );
                    pLoRaMac->xState = FTE_LORAMAC_STATE_LOW_POWER;
                }
            }
            break;
            
        case FTE_LORAMAC_STATE_TX:
        case FTE_LORAMAC_STATE_TX_TIMEOUT:
            {
                Radio.Rx( pLoRaMac->pConfig->RxTimeout );
                pLoRaMac->xState = FTE_LORAMAC_STATE_LOW_POWER;
            }
            break;
            
        case FTE_LORAMAC_STATE_LOW_POWER:
        default:
            // Set low power
            break;
        }
    
        TimerLowPowerHandler( );
        _time_delay(10);
    
    }
}


void FTE_LORAMAC_ctrl(FTE_UINT32 params)
{
    FTE_BOOL bPrevValue[4] = {FALSE,FALSE,FALSE,FALSE};
    FTE_BOOL bValue[4] = {FALSE,FALSE,FALSE,FALSE};
    
    while(TRUE)
    {
        FTE_LWGPIO_getValue(SX1276.pDIO0, &bValue[0]);
        if (bPrevValue[0] != bValue[0])
        {
            if (bPrevValue[0] == FALSE)
            {
                SX1276OnDio0Irq(0);
            }
            bPrevValue[0] = bValue[0];
        }
        
        FTE_LWGPIO_getValue(SX1276.pDIO1, &bValue[1]);
        if (bPrevValue[1] != bValue[1])
        {
            if (bPrevValue[1] == FALSE)
            {
                SX1276OnDio1Irq(0);
            }
            bPrevValue[1] = bValue[1];
        }
        
        FTE_LWGPIO_getValue(SX1276.pDIO2, &bValue[2]);
        if (bPrevValue[2] != bValue[2])
        {
            if (bPrevValue[2] == FALSE)
            {
                SX1276OnDio2Irq(0);
            }
            bPrevValue[2] = bValue[2];
        }
        
        FTE_LWGPIO_getValue(SX1276.pDIO3, &bValue[3]);
        if (bPrevValue[3] != bValue[3])
        {
            if (bPrevValue[3] == FALSE)
            {
                SX1276OnDio3Irq(0);
            }
            bPrevValue[3] = bValue[3];
        }
        
        _time_delay(0);
    }
    
}

void FTE_LORAMAC_setConfig(FTE_LORAMAC_CONFIG_PTR pConfig)
{
    Radio.Standby();
    
    Radio.SetChannel( pConfig->ulFrequency );

    Radio.SetTxConfig( MODEM_LORA, 
                      pConfig->OutputPower, 
                      0, 
                      pConfig->xBandwidth,
                      pConfig->xSF, 
                      pConfig->xCR,
                      pConfig->PreambleLength, 
                      pConfig->bPayloadCRC,
                      pConfig->bPayloadCRC, 
                      0, 
                      0, 
                      FTE_LORAMAC_IQ_INVERSION_ON, 
                      3000000 );
    
    Radio.SetRxConfig( MODEM_LORA,  
                      pConfig->xBandwidth, 
                      pConfig->xSF,
                      pConfig->xCR,
                      0, 
                      pConfig->PreambleLength,
                      FTE_LORAMAC_SYMBOL_TIMEOUT, 
                      pConfig->bPayloadCRC,
                      0, 
                      pConfig->bPayloadCRC, 
                      0, 
                      0, 
                      FTE_LORAMAC_IQ_INVERSION_ON, 
                      true );

    Radio.Rx(pConfig->RxTimeout);
}

void FTE_LORAMAC_onTxDone( void * params )
{
    Radio.Sleep( );
    pLoRaMac->xState = FTE_LORAMAC_STATE_TX;
}

void FTE_LORAMAC_onRxDone( void * params, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    static FTE_CHAR  pBuff[256];
    
    Radio.Sleep( );
    pLoRaMac->ulBufferSize = size;
    memcpy( pLoRaMac->pBuffer, payload, pLoRaMac->ulBufferSize );
    pLoRaMac->nRSSI = rssi;
    pLoRaMac->nSNR = snr;
    pLoRaMac->ulRxPktCount++;
    pLoRaMac->xState = FTE_LORAMAC_STATE_RX;
    
    pLoRaMac->pBuffer[pLoRaMac->ulBufferSize] = 0;
    
    FTE_UINT32 ulBuffLen = 0;
    for(int i = 0 ; i < 16 && i < pLoRaMac->ulBufferSize ; i++)
    {
        ulBuffLen += sprintf(&pBuff[ulBuffLen], "%02x ", pLoRaMac->pBuffer[i]);
    }
    TRACE( "Rx Done[%3d, %3d], Data[%2d] - %s\n", 
          pLoRaMac->nRSSI, pLoRaMac->nSNR, pLoRaMac->ulBufferSize, pBuff);
}

void FTE_LORAMAC_onTxTimeout( void * params )
{
    Radio.Sleep( );
    pLoRaMac->xState = FTE_LORAMAC_STATE_TX_TIMEOUT;
    TRACE("Tx Timeout\n");
}

void FTE_LORAMAC_onRxTimeout( void * params )
{
    Radio.Sleep( );
    pLoRaMac->xState = FTE_LORAMAC_STATE_RX_TIMEOUT;
    TRACE("Rx Timeout\n");
}

void FTE_LORAMAC_onRxError( void * params )
{
    Radio.Sleep( );
    pLoRaMac->xState = FTE_LORAMAC_STATE_RX_ERROR;
    TRACE("Rx Error\n");
}

FTE_UINT32         FTE_LORAMAC_getFrequency(FTE_LORAMAC_PTR pLoRaMac)
{
    ASSERT(pLoRaMac != NULL);
    
    return  pLoRaMac->pConfig->ulFrequency;
}

FTE_LORA_SF  FTE_LORAMAC_getSpreadingFactor(FTE_LORAMAC_PTR pLoRaMac)
{
    ASSERT(pLoRaMac != NULL);
    
    return  pLoRaMac->pConfig->xSF;
}

FTE_LORA_SF FTE_LORAMAC_setSpreadingFactor(FTE_LORAMAC_PTR pLoRaMac, FTE_LORA_SF xSF)
{
    ASSERT(pLoRaMac != NULL);
    
    return  (pLoRaMac->pConfig->xSF = xSF);
}

FTE_LORA_CR FTE_LORAMAC_getCodingRate(FTE_LORAMAC_PTR pLoRaMac)
{
    ASSERT(pLoRaMac != NULL);
    
    return  pLoRaMac->pConfig->xCR;
}

bool    FTE_LORAMAC_getPayloadCRC(FTE_LORAMAC_PTR pLoRaMac)
{
    ASSERT(pLoRaMac != NULL);
    
    return  pLoRaMac->pConfig->bPayloadCRC;
}

bool    FTE_LORAMAC_setPayloadCRC(FTE_LORAMAC_PTR pLoRaMac, bool bOn)
{
    ASSERT(pLoRaMac != NULL);
    
    return  (pLoRaMac->pConfig->bPayloadCRC = bOn);
}

FTE_LORA_BANDWIDTH FTE_LORAMAC_getBandwidth(FTE_LORAMAC_PTR pLoRaMac)
{
    ASSERT(pLoRaMac != NULL);
    
    return  pLoRaMac->pConfig->xBandwidth;
}

FTE_LORA_BANDWIDTH FTE_LORAMAC_setBandwidth(FTE_LORAMAC_PTR pLoRaMac, FTE_LORA_BANDWIDTH xBandwidth)
{
    ASSERT(pLoRaMac != NULL);
    
    return  (pLoRaMac->pConfig->xBandwidth = xBandwidth);
}
#endif

