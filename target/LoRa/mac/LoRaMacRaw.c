/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdlib.h>
#include <math.h>
#include "board.h"
#include "utilities.h"
#include "radio.h"

#include "fte_lorawan.h"
#include "LoRaMacCrypto.h"
#include "LoRaMac.h"

/*!
 * Statistics
 */
static  LoRaMac_t   LoRaMac = 
{
    .Signature  =   LORAMAC_SIGNATURE,       
};

/*!
 * Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void * obj);

/*!
 * Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( void * obj, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void * obj);

/*!
 * Function executed on Radio Rx error event
 */
static void OnRadioRxError( void * obj);

/*!
 * Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void * obj);

/*!
 * Function executed on Resend Frame timer event.
 */
static void OnMacStateCheckTimerEvent( void * );

/*!
 * Function executed on duty cycle delayed Tx  timer event
 */
static void OnTxDelayedTimerEvent( void * );

/*!
 * Function executed on channel check timer event
 */
static void OnChannelCheckTimerEvent( void *params );

/*!
 * Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void * );

/*!
 * Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void * );

/*!
 * Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void * );

/*!
 * Searches and set the next random available channel
 *
 * \retval status  Function status [0: OK, 1: Unable to find a free channel]
 */
static uint8_t LoRaMacSetNextChannel( LoRaMac_t *pLoRaMac )
{
    ASSERT(pLoRaMac != NULL);

    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t channelNext = pLoRaMac->Status.Channel;
    uint8_t nbEnabledChannels = 0;
    uint8_t enabledChannels[LORA_MAX_NB_CHANNELS];
    uint64_t curTime = TimerGetCurrentTime( );

    
    memset( enabledChannels, 0, LORA_MAX_NB_CHANNELS );

    // Update Aggregated duty cycle
    if( pLoRaMac->Status.AggregatedTimeOff < ( curTime - pLoRaMac->Status.AggregatedLastTxDoneTime ) )
    {
        pLoRaMac->Status.AggregatedTimeOff = 0;
    }

    // Update bands Time OFF
    uint64_t minTime = 0xFFFFFFFFFFFFFFFF;
    for( i = 0; i < LORA_MAX_NB_BANDS; i++ )
    {
        if( pLoRaMac->Status.DutyCycleOn == true )
        {
            if( pLoRaMac->Config.Bands[i].TimeOff < ( curTime - pLoRaMac->Config.Bands[i].LastTxDoneTime ) )
            {
                pLoRaMac->Config.Bands[i].TimeOff = 0;
            }
            if( pLoRaMac->Config.Bands[i].TimeOff != 0 )
            {
                minTime = MIN( pLoRaMac->Config.Bands[i].TimeOff, minTime );
            }
        }
        else
        {
            minTime = 0;
            pLoRaMac->Config.Bands[i].TimeOff = 0;
        }
    }

    // Search how many channels are enabled
    for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
    {
        if( ( ( 1 << i ) & pLoRaMac->Config.ChannelsMask ) != 0 )
        {
            if( pLoRaMac->Config.Channels[i].Frequency == 0 )
            { // Check if the channel is enabled
                continue;
            }
            if( ( ( pLoRaMac->Config.Channels[i].DrRange.Fields.Min <= pLoRaMac->Status.ChannelsDatarate ) &&
                  ( pLoRaMac->Status.ChannelsDatarate <= pLoRaMac->Config.Channels[i].DrRange.Fields.Max ) ) == false )
            { // Check if the current channel selection supports the given datarate
                continue;
            }
            if( pLoRaMac->Config.Bands[pLoRaMac->Config.Channels[i].Band].TimeOff > 0 )
            { // Check if the band is available for transmission
                continue;
            }
            if( pLoRaMac->Status.AggregatedTimeOff > 0 )
            { // Check if there is time available for transmission
                continue;
            }
            enabledChannels[nbEnabledChannels++] = i;
        }
    }
    if( nbEnabledChannels > 0 )
    {
        for( i = 0, j = randr( 0, nbEnabledChannels - 1 ); i < LORA_MAX_NB_CHANNELS; i++ )
        {
            channelNext = enabledChannels[j];
            j = ( j + 1 ) % nbEnabledChannels;

            if( Radio.IsChannelFree( MODEM_LORA, pLoRaMac->Config.Channels[channelNext].Frequency, RSSI_FREE_TH ) == true )
            {
                // Free channel found
                pLoRaMac->Status.Channel = channelNext;
               
                pLoRaMac->Status.State &= ~MAC_CHANNEL_CHECK;
                TimerStop( &pLoRaMac->Status.ChannelCheckTimer );
                return 0;
            }
        }
    }
    // No free channel found.
    // Check again
    if( ( pLoRaMac->Status.State & MAC_CHANNEL_CHECK ) == 0 )
    {
        TimerSetValue( &pLoRaMac->Status.ChannelCheckTimer, minTime );
        TimerStart( &pLoRaMac->Status.ChannelCheckTimer );
        pLoRaMac->Status.State |= MAC_CHANNEL_CHECK;
    }
    return 1;
}

/*
 * TODO: Add documentation
 */
void OnChannelCheckTimerEvent( void * params )
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)params;
    
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.State &= ~MAC_CHANNEL_CHECK;
    if( LoRaMacSetNextChannel( pLoRaMac ) == 0 )
    {
        if( ( pLoRaMac->Status.State & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
        {
           LoRaMacSendFrameOnChannel( pLoRaMac, pLoRaMac->Config.Channels[pLoRaMac->Status.Channel] );
        }
    }
}

/*!
 * Adds a new MAC command to be sent.
 *
 * \Remark MAC layer internal function
 *
 * \param [in] cmd MAC command to be added
 *                 [MOTE_MAC_LINK_CHECK_REQ,
 *                  MOTE_MAC_LINK_ADR_ANS,
 *                  MOTE_MAC_DUTY_CYCLE_ANS,
 *                  MOTE_MAC_RX2_PARAM_SET_ANS,
 *                  MOTE_MAC_DEV_STATUS_ANS
 *                  MOTE_MAC_NEW_CHANNEL_ANS]
 * \param [in] p1  1st parameter ( optional depends on the command )
 * \param [in] p2  2nd parameter ( optional depends on the command )
 *
 * \retval status  Function status [0: OK, 1: Unknown command, 2: Buffer full]
 */
static uint8_t AddMacCommand( LoRaMac_t *pLoRaMac, uint8_t cmd, uint8_t p1, uint8_t p2 )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.MacCommandsBuffer[pLoRaMac->Status.MacCommandsBufferIndex++] = cmd;
    switch( cmd )
    {
        case MOTE_MAC_LINK_CHECK_REQ:
            // No payload for this command
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            // Margin
            pLoRaMac->Status.MacCommandsBuffer[pLoRaMac->Status.MacCommandsBufferIndex++] = p1;
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            // No payload for this answer
            break;
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
            // Status: Datarate ACK, Channel ACK
            pLoRaMac->Status.MacCommandsBuffer[pLoRaMac->Status.MacCommandsBufferIndex++] = p1;
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            // 1st byte Battery
            // 2nd byte Margin
            pLoRaMac->Status.MacCommandsBuffer[pLoRaMac->Status.MacCommandsBufferIndex++] = p1;
            pLoRaMac->Status.MacCommandsBuffer[pLoRaMac->Status.MacCommandsBufferIndex++] = p2;
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            // Status: Datarate range OK, Channel frequency OK
            pLoRaMac->Status.MacCommandsBuffer[pLoRaMac->Status.MacCommandsBufferIndex++] = p1;
            break;
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            // No payload for this answer
            break;
        default:
            return 1;
    }
    if( pLoRaMac->Status.MacCommandsBufferIndex <= 15 )
    {
        pLoRaMac->Status.MacCommandsInNextTx = true;
        return 0;
    }
    else
    {
        return 2;
    }
}

// TODO: Add Documentation
static void LoRaMacNotify( LoRaMac_t *pLoRaMac, LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    ASSERT(pLoRaMac != NULL);
    
    if(pLoRaMac->Events.MacEvent != NULL)
    {
        pLoRaMac->Events.MacEvent( flags, info, pLoRaMac->Events.pParams );
    }
    flags->Value = 0;
}

LoRaMac_t *LoRaMacInit( LoRaMacConfig_t *pConfig, LoRaMacEvent_t *pEvents )
{
    RadioEvents_t RadioEvents;
    
    memcpy(&LoRaMac.Events, pEvents, sizeof(LoRaMacEvent_t));
    memcpy(&LoRaMac.Config, pConfig, sizeof(LoRaMacConfig_t));

    LoRaMac.EventInfo.TxAckReceived = false;
    LoRaMac.EventInfo.TxNbRetries = 0;
    LoRaMac.EventInfo.TxDatarate = 7;
    LoRaMac.EventInfo.RxPort = 1;
    LoRaMac.EventInfo.RxBuffer = NULL;
    LoRaMac.EventInfo.RxBufferSize = 0;
    LoRaMac.EventInfo.RxRssi = 0;
    LoRaMac.EventInfo.RxSnr = 0;
    LoRaMac.EventInfo.Energy = 0;
    LoRaMac.EventInfo.DemodMargin = 0;
    LoRaMac.EventInfo.NbGateways = 0;
    LoRaMac.EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
  
    LoRaMac.Status.EventFlags.Value = 0;
   
    LoRaMac.Status.UpLinkCounter = 1;
    LoRaMac.Status.DownLinkCounter = 0;
   
    LoRaMac.Status.IsNetworkJoined = false;
    LoRaMac.Status.State = MAC_IDLE;

    LoRaMac.Status.IsUpLinkCounterFixed = true;
    LoRaMac.Status.IsRxWindowsEnabled = true;
    LoRaMac.Status.NodeAckRequested = false;
    LoRaMac.Status.SrvAckRequested = false;

    LoRaMac.Status.AckTimeoutRetry = false;
    
    LoRaMac.Status.ChannelsDatarate = LoRaMac.Config.ChannelsDefaultDatarate;
    LoRaMac.Status.ChannelsNbRepCounter = 0;
   
    LoRaMac.Status.Rx1DrOffset = 0;
    LoRaMac.Status.MaxDCycle = 0;
    LoRaMac.Status.AggregatedDCycle = 1;
    LoRaMac.Status.AggregatedLastTxDoneTime = 0;
    LoRaMac.Status.AggregatedTimeOff = 0;

    LoRaMac.Status.AdrCtrlOn = false;
    LoRaMac.Status.AdrAckCounter = 0;

    LoRaMac.Status.MulticastChannels = NULL;
    LoRaMac.Status.DutyCycleOn = true;

    LoRaMac.Status.ReceiveDelay1 = RECEIVE_DELAY1;
    LoRaMac.Status.ReceiveDelay2 = RECEIVE_DELAY2;
    LoRaMac.Status.JoinAcceptDelay1 = JOIN_ACCEPT_DELAY1;
    LoRaMac.Status.JoinAcceptDelay2 = JOIN_ACCEPT_DELAY2;

    TimerInit( &LoRaMac.Status.MacStateCheckTimer, OnMacStateCheckTimerEvent, (void *)&LoRaMac );
    TimerSetValue( &LoRaMac.Status.MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );

    TimerInit( &LoRaMac.Status.ChannelCheckTimer, OnChannelCheckTimerEvent, (void *)&LoRaMac );
    TimerInit( &LoRaMac.Status.TxDelayedTimer, OnTxDelayedTimerEvent, (void *)&LoRaMac );
    TimerInit( &LoRaMac.Status.RxWindowTimer1, OnRxWindow1TimerEvent, (void *)&LoRaMac );
    TimerInit( &LoRaMac.Status.RxWindowTimer2, OnRxWindow2TimerEvent, (void *)&LoRaMac );
    TimerInit( &LoRaMac.Status.AckTimeoutTimer, OnAckTimeoutTimerEvent, (void *)&LoRaMac );
   
    LoRaMac.Status.MacCommandsInNextTx = false;
    LoRaMac.Status.MacCommandsBufferIndex= 0;

    // Random seed initialization
    srand( RAND_SEED );

    // Initialize Radio driver
    RadioEvents.Params = (void *)&LoRaMac;
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &RadioEvents );


    // Initialize channel index.
    LoRaMac.Status.Channel = LORA_MAX_NB_CHANNELS;

    LoRaMac.Status.AckTimeoutRetries = 1;
    LoRaMac.Status.AckTimeoutRetriesCounter = 1;

    Radio.SetModem( MODEM_LORA );
    if( LoRaMac.Config.PublicNetwork == true )
    {
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }

    if (LoRaMac.Config.DeviceClass == CLASS_C)
    {
        OnRxWindow2TimerEvent(&LoRaMac);
    }
    else
    {
        Radio.Sleep( );
    }
    
    
    return  &LoRaMac;
}

void LoRaMacSetAdrOn( LoRaMac_t *pLoRaMac, bool enable )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.AdrCtrlOn = enable;
}

void LoRaMacInitNwkIds( uint32_t netID, uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey )
{
    LoRaMac.Config.NetID = netID;
    LoRaMac.Config.DevAddr = devAddr;
    memcpy(LoRaMac.Config.NwkSKey, nwkSKey, 16);
    memcpy(LoRaMac.Config.AppSKey, appSKey, 16);
   
    LoRaMac.Status.IsNetworkJoined = true;
}

void LoRaMacMulticastChannelAdd( LoRaMac_t *pLoRaMac, MulticastParams_t *channelParam )
{
    ASSERT(pLoRaMac != NULL);

    // Reset downlink counter
    channelParam->DownLinkCounter = 0;
   
    if( pLoRaMac->Status.MulticastChannels == NULL )
    {
        pLoRaMac->Status.MulticastChannels = channelParam;
    }
    else
    {
        MulticastParams_t *cur = pLoRaMac->Status.MulticastChannels;
        while( cur->Next != NULL )
        {
            cur = cur->Next;
        }
        cur->Next = channelParam;
    }
}

void LoRaMacMulticastChannelRemove( LoRaMac_t *pLoRaMac, MulticastParams_t *channelParam )
{
    ASSERT(pLoRaMac != NULL);
    
    MulticastParams_t *cur = NULL;
   
    // Remove the front element
    if( pLoRaMac->Status.MulticastChannels == channelParam )
    {
        if( pLoRaMac->Status.MulticastChannels != NULL )
        {
            cur = pLoRaMac->Status.MulticastChannels;
            pLoRaMac->Status.MulticastChannels = pLoRaMac->Status.MulticastChannels->Next;
            cur->Next = NULL;
            // Last node in the list
            if( cur == pLoRaMac->Status.MulticastChannels )
            {
                pLoRaMac->Status.MulticastChannels = NULL;
            }
        }
        return;
    }
   
    // Remove last element
    if( channelParam->Next == NULL )
    {
        if( pLoRaMac->Status.MulticastChannels != NULL )
        {
            cur = pLoRaMac->Status.MulticastChannels;
            MulticastParams_t *last = NULL;
            while( cur->Next != NULL )
            {
                last = cur;
                cur = cur->Next;
            }
            if( last != NULL )
            {
                last->Next = NULL;
            }
            // Last node in the list
            if( cur == last )
            {
                pLoRaMac->Status.MulticastChannels = NULL;
            }
        }
        return;
    }
   
    // Remove a middle element
    cur = pLoRaMac->Status.MulticastChannels;
    while( cur != NULL )
    {
        if( cur->Next == channelParam )
        {
            break;
        }
        cur = cur->Next;
    }
    if( cur != NULL )
    {
        MulticastParams_t *tmp = cur ->Next;
        cur->Next = tmp->Next;
        tmp->Next = NULL;
    }
}

uint8_t LoRaMacJoinReq( LoRaMac_t *pLoRaMac, uint8_t *devEui, uint8_t *appEui, uint8_t *appKey )
{
    LoRaMacHeader_t macHdr;

    ASSERT(pLoRaMac != NULL);
    
    memcpy(LoRaMac.Config.DevEui, devEui, 8);
    memcpy(LoRaMac.Config.AppEui, appEui, 16);
    memcpy(LoRaMac.Config.AppKey, appKey, 16);
   
    macHdr.Value = 0;
    macHdr.Bits.MType        = FRAME_TYPE_JOIN_REQ;
   
    pLoRaMac->Status.IsNetworkJoined = false;
    return LoRaMacSend( pLoRaMac, &macHdr, NULL, 0, NULL, 0 );
}

uint8_t LoRaMacLinkCheckReq( LoRaMac_t *pLoRaMac )
{
    ASSERT(pLoRaMac != NULL);
    
    return AddMacCommand( pLoRaMac, MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
}

uint8_t LoRaMacJoinAccept( LoRaMac_t *pLoRaMac, uint32_t devAddr, uint8_t DLSettings, uint8_t RxDelay)
{
    uint32_t mic = 0;
    uint8_t payload[32];
    uint8_t payloadLen = 0;
    uint32_t random;
    LoRaMacHeader_t macHdr;
    
    ASSERT(pLoRaMac != NULL);

    macHdr.Value = 0;
    macHdr.Bits.MType        = FRAME_TYPE_JOIN_ACCEPT;
    
    payload[payloadLen++] = macHdr.Value;
    random = Radio.Random();
    payload[payloadLen++] = random & 0xFF;
    payload[payloadLen++] = (random >> 8) & 0xFF;
    payload[payloadLen++] = (random >> 16) & 0xFF;

    // NetID
    payload[payloadLen++] = pLoRaMac->Config.NetID & 0xFF;
    payload[payloadLen++] = (pLoRaMac->Config.NetID >> 8) & 0xFF;
    payload[payloadLen++] = (pLoRaMac->Config.NetID >> 16) & 0xFF;
    
    // DevAddr
    payload[payloadLen++] = devAddr & 0xFF;
    payload[payloadLen++] = (devAddr >> 8) & 0xFF;
    payload[payloadLen++] = (devAddr >> 16) & 0xFF;
    payload[payloadLen++] = (devAddr >> 24) & 0xFF;
    
    // DLSettings
    payload[payloadLen++] = DLSettings;
    
    // RxDelay 
    payload[payloadLen++] = RxDelay;
    
    LoRaMacJoinComputeMic( payload, payloadLen, pLoRaMac->Config.AppKey, &mic);
    payload[payloadLen++] = mic & 0xFF;
    payload[payloadLen++] = (mic >> 8) & 0xFF;
    payload[payloadLen++] = (mic >> 16)  & 0xFF;
    payload[payloadLen++] = (mic >> 24)  & 0xFF;
    payloadLen += LORAMAC_MFR_LEN;
    
    pLoRaMac->Status.BufferPktLen = 0;
    pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen++] = macHdr.Value;
    LoRaMacJoinEncrypt( payload + 1, payloadLen - 1, pLoRaMac->Config.AppKey, pLoRaMac->Status.Buffer + 1);
    pLoRaMac->Status.BufferPktLen += payloadLen - 1;

    return LoRaMacSendFrameOnChannel( pLoRaMac, pLoRaMac->Config.Channels[pLoRaMac->Status.Channel] );
}

uint8_t LoRaMacSendRaw( LoRaMac_t *pLoRaMac, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    ASSERT(pLoRaMac != NULL);

    if (fBufferSize > sizeof(pLoRaMac->Status.Buffer))
    {
        return  5;
    }
    
    memcpy(pLoRaMac->Status.Buffer, fBuffer, fBufferSize);
    pLoRaMac->Status.BufferPktLen = fBufferSize;

    if( LoRaMacSetNextChannel( pLoRaMac ) == 0 )
    {
        return  LoRaMacSendFrameOnChannel( pLoRaMac, pLoRaMac->Config.Channels[pLoRaMac->Status.Channel]);
    }
    
    return 5;
}

uint8_t LoRaMacSendFrame( LoRaMac_t *pLoRaMac, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacHeader_t macHdr;

    ASSERT(pLoRaMac != NULL);
    
    macHdr.Value = 0;

    macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    return LoRaMacSend( pLoRaMac, &macHdr, NULL, fPort, fBuffer, fBufferSize );
}

uint8_t LoRaMacSendConfirmedFrame( LoRaMac_t *pLoRaMac, uint8_t fPort, void *fBuffer, uint16_t fBufferSize, uint8_t retries )
{
    LoRaMacHeader_t macHdr;

    ASSERT( pLoRaMac != NULL );
    
    if( pLoRaMac->Status.AdrCtrlOn == false )
    {
        pLoRaMac->Status.ChannelsDatarate = pLoRaMac->Config.ChannelsDefaultDatarate;
    }
    pLoRaMac->Status.AckTimeoutRetries = retries;
    pLoRaMac->Status.AckTimeoutRetriesCounter = 1;
   
    macHdr.Value = 0;

    macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
    return LoRaMacSend( pLoRaMac, &macHdr, NULL, fPort, fBuffer, fBufferSize );
}


uint8_t LoRaMacSend( LoRaMac_t *pLoRaMac, LoRaMacHeader_t *macHdr, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;

    ASSERT(pLoRaMac != NULL);
    
    fCtrl.Value = 0;

    fCtrl.Bits.FOptsLen      = 0;
    fCtrl.Bits.FPending      = 0;
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = pLoRaMac->Status.AdrCtrlOn;

    if( LoRaMacSetNextChannel( pLoRaMac ) == 0 )
    {
        return LoRaMacSendOnChannel( pLoRaMac, pLoRaMac->Config.Channels[pLoRaMac->Status.Channel], macHdr, &fCtrl, fOpts, fPort, fBuffer, fBufferSize );
    }
    return 5;
}

uint8_t LoRaMacPrepareFrame( LoRaMac_t *pLoRaMac, ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
   
    ASSERT((pLoRaMac != NULL) && (macHdr != NULL));
    
    pLoRaMac->Status.BufferPktLen = 0;
   
    pLoRaMac->Status.NodeAckRequested = false;
   
    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    pLoRaMac->Status.Buffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:           
            pLoRaMac->Status.RxWindow1Delay = pLoRaMac->Status.JoinAcceptDelay1 - RADIO_WAKEUP_TIME;
            pLoRaMac->Status.RxWindow2Delay = pLoRaMac->Status.JoinAcceptDelay2 - RADIO_WAKEUP_TIME;

            pLoRaMac->Status.BufferPktLen = pktHeaderLen;
       
            LoRaMacMemCpy( pLoRaMac->Config.AppEui, pLoRaMac->Status.Buffer + pLoRaMac->Status.BufferPktLen, 8 );
            pLoRaMac->Status.BufferPktLen += 8;
            LoRaMacMemCpy( pLoRaMac->Config.DevEui, pLoRaMac->Status.Buffer + pLoRaMac->Status.BufferPktLen, 8 );
            pLoRaMac->Status.BufferPktLen += 8;

            pLoRaMac->Status.DevNonce = Radio.Random( );
           
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen++] = pLoRaMac->Status.DevNonce & 0xFF;
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen++] = ( pLoRaMac->Status.DevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( pLoRaMac->Status.Buffer, pLoRaMac->Status.BufferPktLen & 0xFF, pLoRaMac->Config.AppKey, &mic );
           
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen++] = mic & 0xFF;
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen++] = ( mic >> 8 ) & 0xFF;
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen++] = ( mic >> 16 ) & 0xFF;
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen++] = ( mic >> 24 ) & 0xFF;
           
            break;
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            pLoRaMac->Status.NodeAckRequested = true;
            //Intentional falltrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( pLoRaMac->Status.IsNetworkJoined == false )
            {
                return 2; // No network has been joined yet
            }
           
            pLoRaMac->Status.RxWindow1Delay = pLoRaMac->Status.ReceiveDelay1 - RADIO_WAKEUP_TIME;
            pLoRaMac->Status.RxWindow2Delay = pLoRaMac->Status.ReceiveDelay2 - RADIO_WAKEUP_TIME;

            if( fOpts == NULL )
            {
                fCtrl->Bits.FOptsLen = 0;
            }

            if( pLoRaMac->Status.SrvAckRequested == true )
            {
                pLoRaMac->Status.SrvAckRequested = false;
                fCtrl->Bits.Ack = 1;
            }
           
            if( fCtrl->Bits.Adr == true )
            {
                if( pLoRaMac->Status.ChannelsDatarate == LORAMAC_MIN_DATARATE )
                {
                    pLoRaMac->Status.AdrAckCounter = 0;
                    fCtrl->Bits.AdrAckReq = false;
                }
                else
                {
                    if( pLoRaMac->Status.AdrAckCounter > ADR_ACK_LIMIT )
                    {
                        fCtrl->Bits.AdrAckReq = true;
                    }
                    else
                    {
                        fCtrl->Bits.AdrAckReq = false;
                    }
                    if( pLoRaMac->Status.AdrAckCounter > ( ADR_ACK_LIMIT + ADR_ACK_DELAY ) )
                    {
                        pLoRaMac->Status.AdrAckCounter = 0;
                        if( pLoRaMac->Status.ChannelsDatarate > LORAMAC_MIN_DATARATE )
                        {
                            pLoRaMac->Status.ChannelsDatarate--;
                        }
                        else
                        {
                            // Re-enable default channels 
                            pLoRaMac->Config.ChannelsMask = pLoRaMac->Config.ChannelsMask | LORAMAC_DEFAULT_CHANNEL_MASK;
                        }
                    }
                }
            }
           
            pLoRaMac->Status.Buffer[pktHeaderLen++] = ( pLoRaMac->Config.DevAddr ) & 0xFF;
            pLoRaMac->Status.Buffer[pktHeaderLen++] = ( pLoRaMac->Config.DevAddr >> 8 ) & 0xFF;
            pLoRaMac->Status.Buffer[pktHeaderLen++] = ( pLoRaMac->Config.DevAddr >> 16 ) & 0xFF;
            pLoRaMac->Status.Buffer[pktHeaderLen++] = ( pLoRaMac->Config.DevAddr >> 24 ) & 0xFF;

            pLoRaMac->Status.Buffer[pktHeaderLen++] = fCtrl->Value;

            pLoRaMac->Status.Buffer[pktHeaderLen++] = pLoRaMac->Status.UpLinkCounter & 0xFF;
            pLoRaMac->Status.Buffer[pktHeaderLen++] = ( pLoRaMac->Status.UpLinkCounter >> 8 ) & 0xFF;

            if( fOpts != NULL )
            {
                for( i = 0; i < fCtrl->Bits.FOptsLen; i++ )
                {
                    pLoRaMac->Status.Buffer[pktHeaderLen++] = fOpts[i];
                }
            }
            if( ( pLoRaMac->Status.MacCommandsBufferIndex + fCtrl->Bits.FOptsLen ) <= 15 )
            {
                if( pLoRaMac->Status.MacCommandsInNextTx == true )
                {
                    fCtrl->Bits.FOptsLen += pLoRaMac->Status.MacCommandsBufferIndex;
                   
                    // Update FCtrl field with new value of OptionsLength
                    pLoRaMac->Status.Buffer[0x05] = fCtrl->Value;
                    for( i = 0; i < pLoRaMac->Status.MacCommandsBufferIndex; i++ )
                    {
                        pLoRaMac->Status.Buffer[pktHeaderLen++] = pLoRaMac->Status.MacCommandsBuffer[i];
                    }
                }
                pLoRaMac->Status.MacCommandsInNextTx = false;
                pLoRaMac->Status.MacCommandsBufferIndex = 0;
            }
           
            if( ( pktHeaderLen + fBufferSize ) > LORA_MAC_PHY_MAXPAYLOAD )
            {
                return 3;
            }

            if( fBuffer != NULL )
            {
                pLoRaMac->Status.Buffer[pktHeaderLen] = fPort;
               
                if( fPort == 0 )
                {
                    LoRaMacPayloadEncrypt( fBuffer, fBufferSize, pLoRaMac->Config.NwkSKey, pLoRaMac->Config.DevAddr, UP_LINK, pLoRaMac->Status.UpLinkCounter, pLoRaMac->Status.Payload );
                }
                else
                {
                    LoRaMacPayloadEncrypt( fBuffer, fBufferSize, pLoRaMac->Config.AppSKey, pLoRaMac->Config.DevAddr, UP_LINK, pLoRaMac->Status.UpLinkCounter, pLoRaMac->Status.Payload );
                }
                LoRaMacMemCpy( pLoRaMac->Status.Payload, pLoRaMac->Status.Buffer + pktHeaderLen + 1, fBufferSize );
            }
            pLoRaMac->Status.BufferPktLen = pktHeaderLen + 1 + fBufferSize;

            LoRaMacComputeMic( pLoRaMac->Status.Buffer, pLoRaMac->Status.BufferPktLen, pLoRaMac->Config.NwkSKey, pLoRaMac->Config.DevAddr, UP_LINK, pLoRaMac->Status.UpLinkCounter, &mic );

            if( ( pLoRaMac->Status.BufferPktLen + LORAMAC_MFR_LEN ) > LORA_MAC_PHY_MAXPAYLOAD )
            {
                return 3;
            }
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen + 0] = mic & 0xFF;
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
            pLoRaMac->Status.Buffer[pLoRaMac->Status.BufferPktLen + 3] = ( mic >> 24 ) & 0xFF;
           
            pLoRaMac->Status.BufferPktLen += LORAMAC_MFR_LEN;
            break;
        default:
            return 4;
    }

    return 0;
}

uint8_t LoRaMacSendFrameOnChannel( LoRaMac_t *pLoRaMac, ChannelParams_t channel )
{
    ASSERT( pLoRaMac != NULL );
    
    pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    pLoRaMac->EventInfo.TxDatarate = pLoRaMac->Config.Datarates[pLoRaMac->Status.ChannelsDatarate];

    Radio.SetChannel( channel.Frequency );

    if( pLoRaMac->Status.ChannelsDatarate == DR_7 )
    { // High Speed FSK channel
        Radio.SetTxConfig(  MODEM_FSK, 
                            pLoRaMac->Config.TxPowers[LoRaMac.Config.ChannelsTxPower], 
                            25e3, 
                            0, 
                            (uint32_t)(pLoRaMac->Config.Datarates[pLoRaMac->Status.ChannelsDatarate] * 1e3), 
                            0, 
                            5, 
                            pLoRaMac->Config.FixedLength, 
                            pLoRaMac->Config.PayloadCRC,
                            pLoRaMac->Config.FrequencyHopOn, 
                            pLoRaMac->Config.HopPeriod, 
                            pLoRaMac->Config.InvertedIQ, 
                            pLoRaMac->Config.TxTimeout);
        pLoRaMac->Status.TxTimeOnAir = Radio.TimeOnAir( MODEM_FSK, pLoRaMac->Status.BufferPktLen );
    }
    else if( pLoRaMac->Status.ChannelsDatarate == DR_6 )
    { // High speed LoRa channel
        Radio.SetTxConfig(  MODEM_LORA, 
                            pLoRaMac->Config.TxPowers[LoRaMac.Config.ChannelsTxPower], 
                            0, 
                            1, 
                            pLoRaMac->Config.Datarates[pLoRaMac->Status.ChannelsDatarate], 
                            1, 
                            8, 
                            pLoRaMac->Config.FixedLength, 
                            pLoRaMac->Config.PayloadCRC,
                            pLoRaMac->Config.FrequencyHopOn, 
                            pLoRaMac->Config.HopPeriod, 
                            pLoRaMac->Config.InvertedIQ, 
                            pLoRaMac->Config.TxTimeout);
        pLoRaMac->Status.TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, pLoRaMac->Status.BufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetTxConfig(  MODEM_LORA, 
                            pLoRaMac->Config.TxPowers[pLoRaMac->Config.ChannelsTxPower], 
                            0, 
                            pLoRaMac->Config.Bandwidth, 
                            pLoRaMac->Config.Datarates[pLoRaMac->Status.ChannelsDatarate], 
                            pLoRaMac->Config.CodingRate, 
                            pLoRaMac->Config.PreambleLength, 
                            pLoRaMac->Config.FixedLength, 
                            pLoRaMac->Config.PayloadCRC,
                            pLoRaMac->Config.FrequencyHopOn, 
                            pLoRaMac->Config.HopPeriod, 
                            pLoRaMac->Config.InvertedIQ, 
                            pLoRaMac->Config.TxTimeout);
        pLoRaMac->Status.TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, pLoRaMac->Status.BufferPktLen );
    }
   
    if( pLoRaMac->Status.MaxDCycle == 255 )
    {
        return 6;
    }
    if( pLoRaMac->Status.MaxDCycle == 0 )
    {
        pLoRaMac->Status.AggregatedTimeOff = 0;
    }
    
    pLoRaMac->Status.State |= MAC_TX_RUNNING;
    // Starts the MAC layer status check timer
    TimerStart( &pLoRaMac->Status.MacStateCheckTimer );
   
    if( MAX( pLoRaMac->Config.Bands[channel.Band].TimeOff, pLoRaMac->Status.AggregatedTimeOff ) > ( TimerGetCurrentTime( ) ) )
    {
        // Schedule transmission
        TimerSetValue( &pLoRaMac->Status.TxDelayedTimer, MAX( pLoRaMac->Config.Bands[channel.Band].TimeOff, pLoRaMac->Status.AggregatedTimeOff ) );
        TimerStart( &pLoRaMac->Status.TxDelayedTimer );
    }
    else
    {
        // Send now
        if (pLoRaMac->Config.Debug.TrafficMonitorOn)
        {
            TRACE(DEBUG_NET_LORA, "Tx[%04d] %4d - ", ++pLoRaMac->Statistics.TxPktCount, pLoRaMac->Status.BufferPktLen );
            if (pLoRaMac->Config.Debug.PacketDumpOn)
            {
                for(int i = 0 ; i < pLoRaMac->Status.BufferPktLen ; i++)
                {
                    if ((i != 0) && (pLoRaMac->Config.Debug.PacketDumpColumnSize > 0) && (i % pLoRaMac->Config.Debug.PacketDumpColumnSize == 0))
                    {
                        printf("\n                ");
                    }
                    printf("%02x ", pLoRaMac->Status.Buffer[i]);
                }
            }
            printf("\n");
        }
        
        Radio.Send( pLoRaMac->Status.Buffer, pLoRaMac->Status.BufferPktLen );
    }
    return 0;
}


void OnTxDelayedTimerEvent( void *params )
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)params;
    
    if (pLoRaMac == NULL)
    {
        return;
    }
    
    if (pLoRaMac->Config.Debug.TrafficMonitorOn)
    {
        TRACE(DEBUG_NET_LORA, "Tx[%04d] %4d - ", ++pLoRaMac->Statistics.TxPktCount, pLoRaMac->Status.BufferPktLen );
        if (pLoRaMac->Config.Debug.PacketDumpOn)
        {
            for(int i = 0 ; i < pLoRaMac->Status.BufferPktLen ; i++)
            {
                if ((i != 0) && (pLoRaMac->Config.Debug.PacketDumpColumnSize > 0) && (i % pLoRaMac->Config.Debug.PacketDumpColumnSize == 0))
                {
                    printf("\n                ");
                }
                printf("%02x ", pLoRaMac->Status.Buffer[i]);
            }
        }
        printf("\n");
    }
    
    Radio.Send( pLoRaMac->Status.Buffer, pLoRaMac->Status.BufferPktLen );
}

uint8_t LoRaMacSendOnChannel( LoRaMac_t *pLoRaMac, ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint8_t status = 0;
   
    if( ( pLoRaMac->Status.State & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return 1; // MAC is busy transmitting a previous frame
    }

    status = LoRaMacPrepareFrame( pLoRaMac, channel, macHdr, fCtrl, fOpts, fPort, fBuffer, fBufferSize );
    if( status != 0 )
    {
        return status;
    }

    pLoRaMac->EventInfo.TxNbRetries = 0;
    pLoRaMac->EventInfo.TxAckReceived = false;

    return LoRaMacSendFrameOnChannel( pLoRaMac, channel );
}

static void LoRaMacProcessMacCommands( LoRaMac_t *pLoRaMac, uint8_t *payload, uint8_t macIndex, uint8_t commandsSize )
{
    ASSERT(pLoRaMac != NULL);
    
    while( macIndex < commandsSize )
    {
        // Decode Frame MAC commands
        switch( payload[macIndex++] )
        {
            case SRV_MAC_LINK_CHECK_ANS:
                pLoRaMac->Status.EventFlags.Bits.LinkCheck = 1;
                pLoRaMac->EventInfo.DemodMargin = payload[macIndex++];
                pLoRaMac->EventInfo.NbGateways = payload[macIndex++];
                break;
            case SRV_MAC_LINK_ADR_REQ:
                {
                    uint8_t i;
                    uint8_t status = 0x07;
                    uint16_t chMask = 0;
                    int8_t txPower = 0;
                    int8_t datarate = 0;
                    uint8_t nbRep = 0;
                    uint8_t chMaskCntl = 0;
                   
                    datarate = payload[macIndex++];
                    txPower = datarate & 0x0F;
                    datarate = ( datarate >> 4 ) & 0x0F;

                    if( ( pLoRaMac->Status.AdrCtrlOn == false ) &&
                        ( ( pLoRaMac->Status.ChannelsDatarate != datarate ) || ( pLoRaMac->Config.ChannelsTxPower != txPower ) ) )
                    { // ADR disabled don't handle ADR requests if server tries to change datarate or txpower
                        // Answer the server with fail status
                        // Power ACK     = 0
                        // Data rate ACK = 0
                        // Channel mask  = 0
                        AddMacCommand( pLoRaMac, MOTE_MAC_LINK_ADR_ANS, 0, 0 );
                        break;
                    }
                    chMask = payload[macIndex++];
                    chMask |= payload[macIndex++] << 8;

                    nbRep = payload[macIndex++];
                    chMaskCntl = ( nbRep >> 4 ) & 0x07;
                    nbRep &= 0x0F;
                    if( nbRep == 0 )
                    {
                        nbRep = 1;
                    }
                    if( ( chMaskCntl == 0 ) && ( chMask == 0 ) )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
                        {
                            if( chMaskCntl == 6 )
                            {
                                if( pLoRaMac->Config.Channels[i].Frequency != 0 )
                                {
                                    chMask |= 1 << i;
                                }
                            }
                            else
                            {
                                if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                    ( pLoRaMac->Config.Channels[i].Frequency == 0 ) )
                                {// Trying to enable an undefined channel
                                    status &= 0xFE; // Channel mask KO
                                }
                            }
                        }
                    }
                    if( ( ( datarate < LORAMAC_MIN_DATARATE ) ||
                          ( datarate > LORAMAC_MAX_DATARATE ) ) == true )
                    {
                        status &= 0xFD; // Datarate KO
                    }

                    //
                    // Remark MaxTxPower = 0 and MinTxPower = 5
                    //
                    if( ( ( LORAMAC_MAX_TX_POWER <= txPower ) &&
                          ( txPower <= LORAMAC_MIN_TX_POWER ) ) == false )
                    {
                        status &= 0xFB; // TxPower KO
                    }
                    if( ( status & 0x07 ) == 0x07 )
                    {
                        pLoRaMac->Status.ChannelsDatarate = datarate;
                        pLoRaMac->Config.ChannelsTxPower = txPower;
                        pLoRaMac->Config.ChannelsMask = chMask;
                        pLoRaMac->Config.ChannelsNbRep = nbRep;
                    }
                    AddMacCommand( pLoRaMac, MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                pLoRaMac->Status.MaxDCycle = payload[macIndex++];
                pLoRaMac->Status.AggregatedDCycle = 1 << pLoRaMac->Status.MaxDCycle;
                AddMacCommand( pLoRaMac, MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ:
                {
                    uint8_t status = 0x07;
                    int8_t datarate = 0;
                    int8_t drOffset = 0;
                    uint32_t freq = 0;
               
                    drOffset = payload[macIndex++];
                    datarate = drOffset & 0x0F;
                    drOffset = ( drOffset >> 4 ) & 0x0F;
                   
                    freq = payload[macIndex++];
                    freq |= payload[macIndex++] << 8;
                    freq |= payload[macIndex++] << 16;
                    freq *= 100;
                   
                    if( Radio.CheckRfFrequency( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }
                   
                    if( ( ( datarate < LORAMAC_MIN_DATARATE ) ||
                          ( datarate > LORAMAC_MAX_DATARATE ) ) == true )
                    {
                        status &= 0xFD; // Datarate KO
                    }

                    if( ( ( drOffset < 0 ) || ( drOffset > 5 ) ) == true )
                    {
                        status &= 0xFB; // Rx1DrOffset range KO
                    }
                   
                    if( ( status & 0x07 ) == 0x07 )
                    {
                        pLoRaMac->Config.Rx2Channel.Datarate = datarate;
                        pLoRaMac->Config.Rx2Channel.Frequency = freq;
                        pLoRaMac->Status.Rx1DrOffset = drOffset;
                    }
                    AddMacCommand( pLoRaMac, MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                AddMacCommand( pLoRaMac, MOTE_MAC_DEV_STATUS_ANS, BoardMeasureBatterieLevel( ), pLoRaMac->EventInfo.RxSnr );
                break;
            case SRV_MAC_NEW_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;
                    int8_t channelIndex = 0;
                    ChannelParams_t chParam;
                   
                    channelIndex = payload[macIndex++];
                    chParam.Frequency = payload[macIndex++];
                    chParam.Frequency |= payload[macIndex++] << 8;
                    chParam.Frequency |= payload[macIndex++] << 16;
                    chParam.Frequency *= 100;
                    chParam.DrRange.Value = payload[macIndex++];
                   
                    if( ( channelIndex < 3 ) || ( channelIndex > LORA_MAX_NB_CHANNELS ) )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }
               
                    if( Radio.CheckRfFrequency( chParam.Frequency ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ( chParam.DrRange.Fields.Min > chParam.DrRange.Fields.Max ) ||
                        ( ( ( LORAMAC_MIN_DATARATE <= chParam.DrRange.Fields.Min ) &&
                            ( chParam.DrRange.Fields.Min <= LORAMAC_MAX_DATARATE ) ) == false ) ||
                        ( ( ( LORAMAC_MIN_DATARATE <= chParam.DrRange.Fields.Max ) &&
                            ( chParam.DrRange.Fields.Max <= LORAMAC_MAX_DATARATE ) ) == false ) )
                    {
                        status &= 0xFD; // Datarate range KO
                    }
                    if( ( status & 0x03 ) == 0x03 )
                    {
                        LoRaMacSetChannel( pLoRaMac, channelIndex, chParam );
                    }
                    AddMacCommand( pLoRaMac, MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
                }
                break;
            case SRV_MAC_RX_TIMING_SETUP_REQ:
                {
                    uint8_t delay = payload[macIndex++] & 0x0F;
                   
                    if( delay == 0 )
                    {
                        delay++;
                    }
                    pLoRaMac->Status.ReceiveDelay1 = (uint32_t)(delay * 1e6);
                    pLoRaMac->Status.ReceiveDelay2 = (uint32_t)(pLoRaMac->Status.ReceiveDelay1 + 1e6);
                    AddMacCommand( pLoRaMac, MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
                }
                break;
            default:
                // Unknown command. ABORT MAC commands processing
                return;
        }
    }
}

/*!
 * Function to be executed on Tx Done event
 */
static void OnRadioTxDone( void * obj )
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)obj;
    
    ASSERT(pLoRaMac != NULL);
        
    uint64_t curTime = TimerGetCurrentTime( );
    if( pLoRaMac->Config.DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent(obj);
    }

    // Update Band Time OFF
    pLoRaMac->Config.Bands[pLoRaMac->Config.Channels[pLoRaMac->Status.Channel].Band].LastTxDoneTime = curTime;
    if( pLoRaMac->Status.DutyCycleOn == true )
    {
        pLoRaMac->Config.Bands[pLoRaMac->Config.Channels[pLoRaMac->Status.Channel].Band].TimeOff = pLoRaMac->Status.TxTimeOnAir * pLoRaMac->Config.Bands[pLoRaMac->Config.Channels[pLoRaMac->Status.Channel].Band].DCycle - pLoRaMac->Status.TxTimeOnAir;
    }
    else
    {
        pLoRaMac->Config.Bands[pLoRaMac->Config.Channels[pLoRaMac->Status.Channel].Band].TimeOff = 0;
    }
    // Update Agregated Time OFF
    pLoRaMac->Status.AggregatedLastTxDoneTime = curTime;
    pLoRaMac->Status.AggregatedTimeOff = pLoRaMac->Status.AggregatedTimeOff + ( pLoRaMac->Status.TxTimeOnAir * pLoRaMac->Status.AggregatedDCycle - pLoRaMac->Status.TxTimeOnAir );

    if( pLoRaMac->Status.IsRxWindowsEnabled != true )
    {
        TimerSetValue( &pLoRaMac->Status.RxWindowTimer1, pLoRaMac->Status.RxWindow1Delay );
        TimerStart( &pLoRaMac->Status.RxWindowTimer1 );
        if( pLoRaMac->Config.DeviceClass != CLASS_C )
        {
            TimerSetValue( &pLoRaMac->Status.RxWindowTimer2, pLoRaMac->Status.RxWindow2Delay );
            TimerStart( &pLoRaMac->Status.RxWindowTimer2 );
        }
    }
    else
    {
        pLoRaMac->Status.EventFlags.Bits.Tx = 1;
        pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
    }
   
    if( pLoRaMac->Status.NodeAckRequested == false )
    {
        pLoRaMac->Status.ChannelsNbRepCounter++;
    }
}

/*!
 * Function to be executed on Rx Done event
 */
static void OnRadioRxDone( void *obj, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;

    LoRaMac_t *pLoRaMac = (LoRaMac_t *)obj;
    
    ASSERT(pLoRaMac != NULL);

    if (pLoRaMac->Config.Debug.TrafficMonitorOn)
    {
        TRACE(DEBUG_NET_LORA, "Rx[%04d] %4d - ", ++pLoRaMac->Statistics.RxPktCount, size );
        if (pLoRaMac->Config.Debug.PacketDumpOn)
        {
            for(int i = 0 ; i < size ; i++)
            {
                if ((i != 0) && (pLoRaMac->Config.Debug.PacketDumpColumnSize > 0) && (i % pLoRaMac->Config.Debug.PacketDumpColumnSize == 0))
                {
                    printf("\n                ");
                }
                printf("%02x ", payload[i]);
            }
        }
        printf("\n");
    }
    
    if (pLoRaMac->Config.Bypass)
    {
        pLoRaMac->Status.State |= MAC_RX;
        TimerStart( &pLoRaMac->Status.MacStateCheckTimer );
        
        pLoRaMac->Status.EventFlags.Bits.Rx = 1;
        pLoRaMac->Status.EventFlags.Bits.RxData = 1;
        memcpy(pLoRaMac->Status.RxPayload, payload, size);
        pLoRaMac->EventInfo.RxBuffer = pLoRaMac->Status.RxPayload;
        pLoRaMac->EventInfo.RxBufferSize = size;
        pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
    }
    else
    {
        uint8_t pktHeaderLen = 0;
        uint32_t address = 0;
        uint16_t sequenceCounter = 0;
        int32_t sequence = 0;
        uint8_t appPayloadStartIndex = 0;
        uint8_t port = 0xFF;
        uint8_t frameLen = 0;
        uint32_t mic = 0;
        uint32_t micRx = 0;
       
        MulticastParams_t *curMulticastParams = NULL;
        uint8_t *nwkSKey = pLoRaMac->Config.NwkSKey;
        uint8_t *appSKey = pLoRaMac->Config.AppSKey;
        uint32_t downLinkCounter = 0;
       
        bool isMicOk = false;
        
        if( pLoRaMac->Config.DeviceClass != CLASS_C )
        {
            Radio.Sleep( );
        }
        else
        {
            if( pLoRaMac->Status.EventFlags.Bits.RxSlot == 0 )
            {
                OnRxWindow2TimerEvent(obj);
            }
        }
        TimerStop( &pLoRaMac->Status.RxWindowTimer2 );

        macHdr.Value = payload[pktHeaderLen++];
       
        switch( macHdr.Bits.MType )
        {
            case FRAME_TYPE_JOIN_ACCEPT:
                if( pLoRaMac->Status.IsNetworkJoined == true )
                {
                    break;
                }
                LoRaMacJoinDecrypt( payload + 1, size - 1, pLoRaMac->Config.AppKey, pLoRaMac->Status.RxPayload + 1 );

                pLoRaMac->Status.RxPayload[0] = macHdr.Value;

                LoRaMacJoinComputeMic( pLoRaMac->Status.RxPayload, size - LORAMAC_MFR_LEN, pLoRaMac->Config.AppKey, &mic );
               
                micRx |= pLoRaMac->Status.RxPayload[size - LORAMAC_MFR_LEN];
                micRx |= ( pLoRaMac->Status.RxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
                micRx |= ( pLoRaMac->Status.RxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
                micRx |= ( pLoRaMac->Status.RxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );
               
                if( micRx == mic )
                {
                    pLoRaMac->Status.EventFlags.Bits.Rx = 1;
                    pLoRaMac->EventInfo.RxSnr = snr;
                    pLoRaMac->EventInfo.RxRssi = rssi;

                    LoRaMacJoinComputeSKeys( pLoRaMac->Config.AppKey, pLoRaMac->Status.RxPayload + 1, pLoRaMac->Status.DevNonce, pLoRaMac->Config.NwkSKey, pLoRaMac->Config.AppSKey );

                    pLoRaMac->Config.NetID = pLoRaMac->Status.RxPayload[4];
                    pLoRaMac->Config.NetID |= ( pLoRaMac->Status.RxPayload[5] << 8 );
                    pLoRaMac->Config.NetID |= ( pLoRaMac->Status.RxPayload[6] << 16 );
                   
                    pLoRaMac->Config.DevAddr = pLoRaMac->Status.RxPayload[7];
                    pLoRaMac->Config.DevAddr |= ( pLoRaMac->Status.RxPayload[8] << 8 );
                    pLoRaMac->Config.DevAddr |= ( pLoRaMac->Status.RxPayload[9] << 16 );
                    pLoRaMac->Config.DevAddr |= ( pLoRaMac->Status.RxPayload[10] << 24 );
                   
                    // DLSettings
                    pLoRaMac->Status.Rx1DrOffset = ( pLoRaMac->Status.RxPayload[11] >> 4 ) & 0x07;
                    pLoRaMac->Config.Rx2Channel.Datarate = pLoRaMac->Status.RxPayload[11] & 0x0F;
                   
                    // RxDelay
                    pLoRaMac->Status.ReceiveDelay1 = ( pLoRaMac->Status.RxPayload[12] & 0x0F );
                    if( pLoRaMac->Status.ReceiveDelay1 == 0 )
                    {
                        pLoRaMac->Status.ReceiveDelay1 = 1;
                    }
                    pLoRaMac->Status.ReceiveDelay1 *= 1e6;
                    pLoRaMac->Status.ReceiveDelay2 = (uint32_t)(pLoRaMac->Status.ReceiveDelay1 + 1e6);
                   
                    //CFList
                    if( ( size - 1 ) > 16 )
                    {
                        ChannelParams_t param;
                        param.DrRange.Value = ( DR_5 << 4 ) | DR_0;

                        for( uint8_t i = 3, j = 0; i < ( 5 + 3 ); i++, j += 3 )
                        {
                            param.Frequency = ( pLoRaMac->Status.RxPayload[13 + j] | ( pLoRaMac->Status.RxPayload[14 + j] << 8 ) | ( pLoRaMac->Status.RxPayload[15 + j] << 16 ) ) * 100;
                            LoRaMacSetChannel( pLoRaMac, i, param );
                        }
                    }
                   
                    pLoRaMac->Status.EventFlags.Bits.JoinAccept = 1;
                    pLoRaMac->Status.IsNetworkJoined = true;
                    pLoRaMac->Status.ChannelsDatarate = pLoRaMac->Config.ChannelsDefaultDatarate;
                    pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                }
                else
                {
                    pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
                }

                pLoRaMac->Status.EventFlags.Bits.Tx = 1;
                break;
            case FRAME_TYPE_DATA_CONFIRMED_DOWN:
            case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
                {
                    address = payload[pktHeaderLen++];
                    address |= ( payload[pktHeaderLen++] << 8 );
                    address |= ( payload[pktHeaderLen++] << 16 );
                    address |= ( payload[pktHeaderLen++] << 24 );

                    if( address != pLoRaMac->Config.DevAddr )
                    {
                        curMulticastParams = pLoRaMac->Status.MulticastChannels;
                        while( curMulticastParams != NULL )
                        {
                            if( address == curMulticastParams->Address )
                            {
                                pLoRaMac->Status.EventFlags.Bits.Multicast = 1;
                                nwkSKey = curMulticastParams->NwkSKey;
                                appSKey = curMulticastParams->AppSKey;
                                downLinkCounter = curMulticastParams->DownLinkCounter;
                                break;
                            }
                            curMulticastParams = curMulticastParams->Next;
                        }
                        if( pLoRaMac->Status.EventFlags.Bits.Multicast == 0 )
                        {
                            // We are not the destination of this frame.
                            pLoRaMac->Status.EventFlags.Bits.Tx = 1;
                            pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
                            pLoRaMac->Status.State &= ~MAC_TX_RUNNING;
                            return;
                        }
                    }
                    else
                    {
                        pLoRaMac->Status.EventFlags.Bits.Multicast = 0;
                        nwkSKey = pLoRaMac->Config.NwkSKey;
                        appSKey = pLoRaMac->Config.AppSKey;
                        downLinkCounter = pLoRaMac->Status.DownLinkCounter;
                    }
                   
                    if( pLoRaMac->Config.DeviceClass != CLASS_A )
                    {
                        pLoRaMac->Status.State |= MAC_RX;
                        // Starts the MAC layer status check timer
                        TimerStart( &pLoRaMac->Status.MacStateCheckTimer );
                    }
                    fCtrl.Value = payload[pktHeaderLen++];
                   
                    sequenceCounter |= payload[pktHeaderLen++];
                    sequenceCounter |= payload[pktHeaderLen++] << 8;

                    appPayloadStartIndex = 8 + fCtrl.Bits.FOptsLen;

                    micRx |= payload[size - LORAMAC_MFR_LEN];
                    micRx |= ( payload[size - LORAMAC_MFR_LEN + 1] << 8 );
                    micRx |= ( payload[size - LORAMAC_MFR_LEN + 2] << 16 );
                    micRx |= ( payload[size - LORAMAC_MFR_LEN + 3] << 24 );

                    sequence = ( int32_t )sequenceCounter - ( int32_t )( downLinkCounter & 0xFFFF );
                    if( sequence < 0 )
                    {
                        // sequence reset or roll over happened
                        downLinkCounter = ( downLinkCounter & 0xFFFF0000 ) | ( sequenceCounter + ( uint32_t )0x10000 );
                        LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                        if( micRx == mic )
                        {
                            isMicOk = true;
                        }
                        else
                        {
                            isMicOk = false;
                            // sequence reset
                            if( pLoRaMac->Status.EventFlags.Bits.Multicast == 1 )
                            {
                                curMulticastParams->DownLinkCounter = downLinkCounter = sequenceCounter;
                            }
                            else
                            {
                                pLoRaMac->Status.DownLinkCounter = downLinkCounter = sequenceCounter;
                            }
                            LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                        }
                    }
                    else
                    {
                        downLinkCounter = ( downLinkCounter & 0xFFFF0000 ) | sequenceCounter;
                        LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                    }

                    if( ( isMicOk == true ) ||
                        ( micRx == mic ) )
                    {
                        pLoRaMac->Status.EventFlags.Bits.Rx = 1;
                        pLoRaMac->EventInfo.RxSnr = snr;
                        pLoRaMac->EventInfo.RxRssi = rssi;
                        pLoRaMac->EventInfo.RxBufferSize = 0;
                        pLoRaMac->Status.AdrAckCounter = 0;
                        if( pLoRaMac->Status.EventFlags.Bits.Multicast == 1 )
                        {
                            curMulticastParams->DownLinkCounter = downLinkCounter;
                        }
                        else
                        {
                            pLoRaMac->Status.DownLinkCounter = downLinkCounter;
                        }

                        if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
                        {
                            pLoRaMac->Status.SrvAckRequested = true;
                        }
                        else
                        {
                            pLoRaMac->Status.SrvAckRequested = false;
                        }
                        // Check if the frame is an acknowledgement
                        if( fCtrl.Bits.Ack == 1 )
                        {
                            pLoRaMac->EventInfo.TxAckReceived = true;

                            // Stop the AckTimeout timer as no more retransmissions
                            // are needed.
                            TimerStop( &pLoRaMac->Status.AckTimeoutTimer );
                        }
                        else
                        {
                            pLoRaMac->EventInfo.TxAckReceived = false;
                            if( pLoRaMac->Status.AckTimeoutRetriesCounter > pLoRaMac->Status.AckTimeoutRetries )
                            {
                                // Stop the AckTimeout timer as no more retransmissions
                                // are needed.
                                TimerStop( &pLoRaMac->Status.AckTimeoutTimer );
                            }
                        }
                       
                        if( fCtrl.Bits.FOptsLen > 0 )
                        {
                            // Decode Options field MAC commands
                            LoRaMacProcessMacCommands( pLoRaMac, payload, 8, appPayloadStartIndex );
                        }
                       
                        if( ( ( size - 4 ) - appPayloadStartIndex ) > 0 )
                        {
                            port = payload[appPayloadStartIndex++];
                            frameLen = ( size - 4 ) - appPayloadStartIndex;
                           
                            if( port == 0 )
                            {
                                LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                       frameLen,
                                                       nwkSKey,
                                                       address,
                                                       DOWN_LINK,
                                                       downLinkCounter,
                                                       pLoRaMac->Status.RxPayload );
                               
                                // Decode frame payload MAC commands
                                LoRaMacProcessMacCommands( pLoRaMac, pLoRaMac->Status.RxPayload, 0, frameLen );
                            }
                            else
                            {
                                LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                       frameLen,
                                                       appSKey,
                                                       address,
                                                       DOWN_LINK,
                                                       downLinkCounter,
                                                       pLoRaMac->Status.RxPayload );

                                pLoRaMac->Status.EventFlags.Bits.RxData = 1;
                                pLoRaMac->EventInfo.RxPort = port;
                                pLoRaMac->EventInfo.RxBuffer = pLoRaMac->Status.RxPayload;
                                pLoRaMac->EventInfo.RxBufferSize = frameLen;
                            }
                        }

                        pLoRaMac->Status.EventFlags.Bits.Tx = 1;
                        pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    }
                    else
                    {
                        pLoRaMac->EventInfo.TxAckReceived = false;
                       
                        pLoRaMac->Status.EventFlags.Bits.Tx = 1;
                        pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;
                        pLoRaMac->Status.State &= ~MAC_TX_RUNNING;
                    }
                }
                break;
         
             case FRAME_TYPE_PROPRIETARY:
                //Intentional falltrough
            default:
                pLoRaMac->Status.EventFlags.Bits.Tx = 1;
                pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                pLoRaMac->Status.State &= ~MAC_TX_RUNNING;
                break;
        }
    }
}

/*!
 * Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void * obj)
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)obj;
    
    ASSERT(pLoRaMac != NULL);
        
    TRACE(DEBUG_NET_LORA, "onRadioTxTimeout\n");
    
    if( pLoRaMac->Config.DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent(obj);
    }
   
    pLoRaMac->Status.EventFlags.Bits.Tx = 1;
    pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
}

/*!
 * Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void * obj)
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)obj;
    
    ASSERT(pLoRaMac != NULL);
     
    TRACE(DEBUG_NET_LORA, "onRadioRxTimeout\n");
    if( pLoRaMac->Config.DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    if( pLoRaMac->Status.EventFlags.Bits.RxSlot == 1 )
    {
        pLoRaMac->Status.EventFlags.Bits.Tx = 1;
        pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
    }
}

/*!
 * Function executed on Radio Rx Error event
 */
static void OnRadioRxError( void * obj)
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)obj;
    
    ASSERT(pLoRaMac != NULL);
        
    TRACE(DEBUG_NET_LORA, "onRadioRxError\n");
    if( pLoRaMac->Config.DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    if( pLoRaMac->Status.EventFlags.Bits.RxSlot == 1 )
    {
        pLoRaMac->Status.EventFlags.Bits.Tx = 1;
        pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
    }
}

/*!
 * Initializes and opens the reception window
 *
 * \param [IN] freq window channel frequency
 * \param [IN] datarate window channel datarate
 * \param [IN] bandwidth window channel bandwidth
 * \param [IN] timeout window channel timeout
 */
void LoRaMacRxWindowSetup( LoRaMac_t *pLoRaMac, uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout, bool rxContinuous )
{
    if( Radio.Status( ) == RF_IDLE )
    {
        Radio.SetChannel( freq );
        if( datarate == DR_7 )
        {
            Radio.SetRxConfig( MODEM_FSK, 
                              50e3, 
                              (uint32_t)(pLoRaMac->Config.Datarates[datarate] * 1e3), 
                              0, 
                              83.333e3, 
                              5, 
                              0, 
                              false, 
                              0, 
                              true, 
                              0, 
                              0, 
                              false, 
                              rxContinuous );
        }
        else
        {
            Radio.SetRxConfig( MODEM_LORA, 
                              bandwidth, 
                              pLoRaMac->Config.Datarates[datarate], 
                              pLoRaMac->Config.CodingRate, 
                              0, 
                              pLoRaMac->Config.PreambleLength, 
                              timeout, 
                              false, 
                              0, 
                              false, 
                              0, 
                              0, 
                              false, 
                              rxContinuous );
        }
        if( rxContinuous == false )
        {
            Radio.Rx( pLoRaMac->Config.MaxRxWindow );
        }
        else
        {
            Radio.Rx( 0 ); // Continuous mode
        }
    }
}

/*!
 * Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void *params )
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)params;

    ASSERT(pLoRaMac != NULL);
    
    TRACE(DEBUG_NET_LORA, "OnRxWindow1TimerEvent : NetID = %d\n", pLoRaMac->Config.NetID);
    
    uint16_t symbTimeout = 5; // DR_2, DR_1, DR_0
    int8_t datarate = 0;
    uint32_t bandwidth = 0; // LoRa 125 kHz

    datarate = pLoRaMac->Status.ChannelsDatarate - pLoRaMac->Status.Rx1DrOffset;
    if( datarate < 0 )
    {
        datarate = DR_0;
    }

    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( datarate >= DR_3 )
    { // DR_6, DR_5, DR_4, DR_3
        symbTimeout = 8;
    }
    if( datarate == DR_6 )
    {// LoRa 250 kHz
        bandwidth  = 1;
    }
    pLoRaMac->Status.EventFlags.Bits.RxSlot = 0;
    LoRaMacRxWindowSetup( pLoRaMac, pLoRaMac->Config.Channels[pLoRaMac->Status.Channel].Frequency, datarate, bandwidth, symbTimeout, false );
}

/*!
 * Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void * params)
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)params;
    
    
    ASSERT(pLoRaMac != NULL);

    TRACE(DEBUG_NET_LORA, "OnRxWindow2TimerEvent : NetID = %d\n", pLoRaMac->Config.NetID);
    
    if( pLoRaMac->Status.NodeAckRequested == true )
    {
        TimerSetValue( &pLoRaMac->Status.AckTimeoutTimer, ACK_TIMEOUT + randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
        TimerStart( &pLoRaMac->Status.AckTimeoutTimer );
    }
   
    uint16_t symbTimeout = 5; // DR_2, DR_1, DR_0
    uint32_t bandwidth = 0; // LoRa 125 kHz

    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( pLoRaMac->Config.Rx2Channel.Datarate >= DR_3 )
    { // DR_6, DR_5, DR_4, DR_3
        symbTimeout = 8;
    }
    if( pLoRaMac->Config.Rx2Channel.Datarate == DR_6 )
    {// LoRa 250 kHz
        bandwidth  = 1;
    }

    pLoRaMac->Status.EventFlags.Bits.RxSlot = 1;
    if( pLoRaMac->Config.DeviceClass != CLASS_C )
    {
        LoRaMacRxWindowSetup( pLoRaMac, pLoRaMac->Config.Rx2Channel.Frequency, pLoRaMac->Config.Rx2Channel.Datarate, bandwidth, symbTimeout, false );
    }
    else
    {
        LoRaMacRxWindowSetup( pLoRaMac, pLoRaMac->Config.Rx2Channel.Frequency, pLoRaMac->Config.Rx2Channel.Datarate, bandwidth, symbTimeout, true );
    }
}

/*!
 * Function executed on MacStateCheck timer event
 */
static void OnMacStateCheckTimerEvent( void *params)
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)params;
    
    ASSERT(pLoRaMac != NULL);

    if( pLoRaMac->Status.EventFlags.Bits.Tx == 1 )
    {
        if( pLoRaMac->Status.NodeAckRequested == false )
        {
            if( pLoRaMac->Status.EventFlags.Bits.JoinAccept == true )
            {
                // Join messages aren't repeated automatically
                pLoRaMac->Status.ChannelsNbRepCounter = pLoRaMac->Config.ChannelsNbRep;
                pLoRaMac->Status.UpLinkCounter = 0;
            }
            if( pLoRaMac->Status.ChannelsNbRepCounter >= pLoRaMac->Config.ChannelsNbRep )
            {
                pLoRaMac->Status.ChannelsNbRepCounter = 0;

                pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;

                pLoRaMac->Status.AdrAckCounter++;
                if( pLoRaMac->Status.IsUpLinkCounterFixed == false )
                {
                    pLoRaMac->Status.UpLinkCounter++;
                }

                pLoRaMac->Status.State &= ~MAC_TX_RUNNING;
            }
            else
            {
                pLoRaMac->Status.EventFlags.Bits.Tx = 0;
                // Sends the same frame again
                if( LoRaMacSetNextChannel( pLoRaMac ) == 0 )
                {
                    LoRaMacSendFrameOnChannel( pLoRaMac, pLoRaMac->Config.Channels[pLoRaMac->Status.Channel] );
                }
            }
        }

        if( pLoRaMac->Status.EventFlags.Bits.Rx == 1 )
        {
            if( ( pLoRaMac->EventInfo.TxAckReceived == true ) || ( pLoRaMac->Status.AckTimeoutRetriesCounter > pLoRaMac->Status.AckTimeoutRetries ) )
            {
                pLoRaMac->Status.AckTimeoutRetry = false;
                if( pLoRaMac->Status.IsUpLinkCounterFixed == false )
                {
                    pLoRaMac->Status.UpLinkCounter++;
                }
                pLoRaMac->EventInfo.TxNbRetries = pLoRaMac->Status.AckTimeoutRetriesCounter;
               
                pLoRaMac->Status.State &= ~MAC_TX_RUNNING;
            }
        }
       
        if( ( pLoRaMac->Status.AckTimeoutRetry == true ) && ( ( pLoRaMac->Status.State & MAC_CHANNEL_CHECK ) == 0 ) )
        {
            pLoRaMac->Status.AckTimeoutRetry = false;
            if( ( pLoRaMac->Status.AckTimeoutRetriesCounter < pLoRaMac->Status.AckTimeoutRetries ) && ( pLoRaMac->Status.AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                pLoRaMac->Status.AckTimeoutRetriesCounter++;
               
                if( ( pLoRaMac->Status.AckTimeoutRetriesCounter % 2 ) == 1 )
                {
                    pLoRaMac->Status.ChannelsDatarate = MAX( pLoRaMac->Status.ChannelsDatarate - 1, LORAMAC_MIN_DATARATE );
                }
                pLoRaMac->Status.EventFlags.Bits.Tx = 0;
                // Sends the same frame again
                if( LoRaMacSetNextChannel( pLoRaMac ) == 0 )
                {
                    LoRaMacSendFrameOnChannel( pLoRaMac, pLoRaMac->Config.Channels[pLoRaMac->Status.Channel] );
                }
            }
            else
            {
                // Re-enable default channels LC1, LC2, LC3
                pLoRaMac->Config.ChannelsMask = pLoRaMac->Config.ChannelsMask | LORAMAC_DEFAULT_CHANNEL_MASK;

                pLoRaMac->Status.State &= ~MAC_TX_RUNNING;
               
                pLoRaMac->EventInfo.TxAckReceived = false;
                pLoRaMac->EventInfo.TxNbRetries = pLoRaMac->Status.AckTimeoutRetriesCounter;
                if( pLoRaMac->Status.IsUpLinkCounterFixed == false )
                {
                    pLoRaMac->Status.UpLinkCounter++;
                }
                pLoRaMac->EventInfo.Status = LORAMAC_EVENT_INFO_STATUS_OK;
            }
        }
    }
    // Handle reception for Class B and Class C
    if( ( pLoRaMac->Status.State & MAC_RX ) == MAC_RX )
    {
        pLoRaMac->Status.State &= ~MAC_RX;
    }
    if( pLoRaMac->Status.State == MAC_IDLE )
    {
        LoRaMacNotify( pLoRaMac, &pLoRaMac->Status.EventFlags, &pLoRaMac->EventInfo );
    }
    else
    {
        // Operation not finished restart timer
        TimerStart( &pLoRaMac->Status.MacStateCheckTimer );
    }
}

static void OnAckTimeoutTimerEvent( void * params)
{
    LoRaMac_t *pLoRaMac = (LoRaMac_t *)params;
    
    if (pLoRaMac == NULL)
    {
        return;
    }
    
    pLoRaMac->Status.AckTimeoutRetry = true;
    pLoRaMac->Status.State &= ~MAC_ACK_REQ;
}

/*!
 * ============================================================================
 * = LoRaMac test functions                                                   =
 * ============================================================================
 */

void LoRaMacSetDeviceClass( LoRaMac_t *pLoRaMac, DeviceClass_t deviceClass )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Config.DeviceClass = deviceClass;
}

void LoRaMacSetPublicNetwork( LoRaMac_t *pLoRaMac, bool enable )
{
    pLoRaMac->Config.PublicNetwork = enable;
    Radio.SetModem( MODEM_LORA );
    if( pLoRaMac->Config.PublicNetwork == true )
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

void LoRaMacSetDutyCycleOn( LoRaMac_t *pLoRaMac, bool enable )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.DutyCycleOn = enable;
}

void LoRaMacSetChannel( LoRaMac_t *pLoRaMac, uint8_t id, ChannelParams_t params )
{
    params.Band = 0;
    pLoRaMac->Config.Channels[id] = params;
    // Activate the newly created channel
    pLoRaMac->Config.ChannelsMask |= 1 << id;

    // Check if it is a valid channel
    if( pLoRaMac->Config.Channels[id].Frequency == 0 )
    {
        pLoRaMac->Config.ChannelsMask &= ~( 1 << id );
    }
}

void LoRaMacSetRx2Channel( LoRaMac_t *pLoRaMac, Rx2ChannelParams_t param )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Config.Rx2Channel = param;
}

void LoRaMacSetChannelsTxPower( LoRaMac_t *pLoRaMac, int8_t txPower )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Config.ChannelsTxPower = txPower;
}

void LoRaMacSetChannelsDatarate( LoRaMac_t *pLoRaMac, int8_t datarate )
{
    pLoRaMac->Config.ChannelsDefaultDatarate = pLoRaMac->Status.ChannelsDatarate = datarate;
}

void LoRaMacSetChannelsMask( LoRaMac_t *pLoRaMac, uint16_t mask )
{
    pLoRaMac->Config.ChannelsMask = mask;
}

void LoRaMacSetChannelsNbRep( LoRaMac_t *pLoRaMac, uint8_t nbRep )
{
    if( nbRep < 1 )
    {
        nbRep = 1;
    }
    if( nbRep > 15 )
    {
        nbRep = 15;
    }
    pLoRaMac->Config.ChannelsNbRep = nbRep;
}

void LoRaMacSetMaxRxWindow( LoRaMac_t *pLoRaMac, uint32_t delay )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Config.MaxRxWindow = delay;
}

void LoRaMacSetReceiveDelay1( LoRaMac_t *pLoRaMac, uint32_t delay )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.ReceiveDelay1 = delay;
}

void LoRaMacSetReceiveDelay2( LoRaMac_t *pLoRaMac, uint32_t delay )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.ReceiveDelay2 = delay;
}

void LoRaMacSetJoinAcceptDelay1( LoRaMac_t *pLoRaMac, uint32_t delay )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.JoinAcceptDelay1 = delay;
}

void LoRaMacSetJoinAcceptDelay2( LoRaMac_t *pLoRaMac, uint32_t delay )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.JoinAcceptDelay2 = delay;
}

void LoRaMacTestRxWindowsOn( LoRaMac_t *pLoRaMac, bool enable )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.IsRxWindowsEnabled = enable;
}

uint32_t LoRaMacGetUpLinkCounter( LoRaMac_t *pLoRaMac)
{
    ASSERT(pLoRaMac != NULL);
    
    return pLoRaMac->Status.UpLinkCounter;
}

uint32_t LoRaMacGetDownLinkCounter( LoRaMac_t *pLoRaMac )
{
    ASSERT(pLoRaMac != NULL);
    
    return pLoRaMac->Status.DownLinkCounter;
}

void LoRaMacSetMicTest( LoRaMac_t *pLoRaMac, uint16_t upLinkCounter )
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Status.UpLinkCounter = upLinkCounter;
    pLoRaMac->Status.IsUpLinkCounterFixed = true;
}

void LoRaMacTrafficMonitor( LoRaMac_t *pLoRaMac, bool trafficMonitorOn, bool packetDumpOn, uint32_t packetDumpColumnSize)
{
    ASSERT(pLoRaMac != NULL);
    
    pLoRaMac->Config.Debug.TrafficMonitorOn = trafficMonitorOn;
    pLoRaMac->Config.Debug.PacketDumpOn = packetDumpOn;
    pLoRaMac->Config.Debug.PacketDumpColumnSize = packetDumpColumnSize;
}
