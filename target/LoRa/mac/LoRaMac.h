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
#ifndef __LORAMAC_H__
#define __LORAMAC_H__

// Includes board dependent definitions such as channels frequencies
#include "LoRaMac-board.h"
#if 0
#if defined(__CC_ARM) || defined(__GNUC__)
#define PACKED                                      __attribute__( ( __packed__ ) )
#elif defined( __ICCARM__ )
#define PACKED                                      __packed
#else
    #warning Not supported compiler type
#endif
#else
#define PACKED
#endif
/*!
 * Beacon interval in ms
 */
#define BEACON_INTERVAL                             128000000

/*!
 * Class A&B receive delay in ms
 */
#define RECEIVE_DELAY1                              1000000
#define RECEIVE_DELAY2                              2000000

/*!
 * Join accept receive delay in ms
 */
#define JOIN_ACCEPT_DELAY1                          5000000
#define JOIN_ACCEPT_DELAY2                          6000000

/*!
 * Class A&B maximum receive window delay in ms
 */
#define MAX_RX_WINDOW                               3000000

/*!
 * Maximum allowed gap for the FCNT field
 */
#define MAX_FCNT_GAP                                16384

/*!
 * ADR acknowledgement counter limit
 */
#define ADR_ACK_LIMIT                               64

/*!
 * Number of ADR acknowledgement requests before returning to default datarate
 */
#define ADR_ACK_DELAY                               32

/*!
 * Number of seconds after the start of the second reception window without
 * receiving an acknowledge.
 * AckTimeout = ACK_TIMEOUT + Random( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT                                 2000000

/*!
 * Random number of seconds after the start of the second reception window without
 * receiving an acknowledge
 * AckTimeout = ACK_TIMEOUT + Random( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT_RND                             1000000

/*!
 * Check the Mac layer state every MAC_STATE_CHECK_TIMEOUT
 */
#define MAC_STATE_CHECK_TIMEOUT                     100000

/*!
 * Maximum number of times the MAC layer tries to get an acknowledge.
 */
#define MAX_ACK_RETRIES                             8

/*!
 * RSSI free threshold
 */
#define RSSI_FREE_TH                                ( int8_t )( -90 ) // [dBm]

/*!
 * Frame direction definition
 */
#define UP_LINK                                     0
#define DOWN_LINK                                   1

/*!
 * Sets the length of the LoRaMAC footer field.
 * Mainly indicates the MIC field length
 */
#define LORAMAC_MFR_LEN                             4

/*!
 * Syncword for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/*!
 * Syncword for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34

/*!
 * Maximum PHY layer payload size
 */
#define LORA_MAC_PHY_MAXPAYLOAD                     256


/*!
 * LoRaWAN devices classes definition
 */
typedef enum
{
    CLASS_A,
    CLASS_B,
    CLASS_C,
}DeviceClass_t;

/*!
 * LoRaMAC channels parameters definition
 */
typedef union
{
    int8_t Value;
    struct
    {
        int8_t Min : 4;
        int8_t Max : 4;
    }PACKED Fields;
}PACKED DrRange_t;

typedef struct
{
    uint16_t DCycle;
    int8_t TxMaxPower;
    uint64_t LastTxDoneTime;
    uint64_t TimeOff;
}PACKED Band_t;

typedef struct
{
    uint32_t Frequency; // Hz
    DrRange_t DrRange;  // Max datarate [0: SF12, 1: SF11, 2: SF10, 3: SF9, 4: SF8, 5: SF7, 6: SF7, 7: FSK]
                        // Min datarate [0: SF12, 1: SF11, 2: SF10, 3: SF9, 4: SF8, 5: SF7, 6: SF7, 7: FSK]
    uint8_t Band;       // Band index
}PACKED ChannelParams_t;

typedef struct
{
    uint32_t Frequency; // Hz
    uint8_t  Datarate;  // [0: SF12, 1: SF11, 2: SF10, 3: SF9, 4: SF8, 5: SF7, 6: SF7, 7: FSK]
}PACKED Rx2ChannelParams_t;

#pragma pack(push,1)
typedef struct MulticastParams_s
{
    uint32_t Address;
    uint8_t NwkSKey[16];
    uint8_t AppSKey[16];
    uint32_t DownLinkCounter;
    struct MulticastParams_s *Next;
} MulticastParams_t;
#pragma pack(pop)

/*!
 * LoRaMAC frame types
 */
typedef enum
{
    FRAME_TYPE_JOIN_REQ              = 0x00,
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    FRAME_TYPE_RFU                   = 0x06,
    FRAME_TYPE_PROPRIETARY           = 0x07,
}PACKED LoRaMacFrameType_t;

/*!
 * LoRaMAC mote MAC commands
 */
typedef enum
{
    MOTE_MAC_LINK_CHECK_REQ          = 0x02,
    MOTE_MAC_LINK_ADR_ANS            = 0x03,
    MOTE_MAC_DUTY_CYCLE_ANS          = 0x04,
    MOTE_MAC_RX_PARAM_SETUP_ANS      = 0x05,
    MOTE_MAC_DEV_STATUS_ANS          = 0x06,
    MOTE_MAC_NEW_CHANNEL_ANS         = 0x07,
    MOTE_MAC_RX_TIMING_SETUP_ANS     = 0x08,
}PACKED LoRaMacMoteCmd_t;

/*!
 * LoRaMAC server MAC commands
 */
typedef enum
{
    SRV_MAC_LINK_CHECK_ANS           = 0x02,
    SRV_MAC_LINK_ADR_REQ             = 0x03,
    SRV_MAC_DUTY_CYCLE_REQ           = 0x04,
    SRV_MAC_RX_PARAM_SETUP_REQ       = 0x05,
    SRV_MAC_DEV_STATUS_REQ           = 0x06,
    SRV_MAC_NEW_CHANNEL_REQ          = 0x07,
    SRV_MAC_RX_TIMING_SETUP_REQ      = 0x08,
}PACKED LoRaMacSrvCmd_t;

/*!
 * LoRaMAC Battery level indicator
 */
typedef enum
{
    BAT_LEVEL_EXT_SRC                = 0x00,
    BAT_LEVEL_EMPTY                  = 0x01,
    BAT_LEVEL_FULL                   = 0xFE,
    BAT_LEVEL_NO_MEASURE             = 0xFF,
}PACKED LoRaMacBatteryLevel_t;


/*!
 * LoRaMac internal states
 */
typedef enum LoRaMacState_e
{
    MAC_IDLE          = 0x00000000,
    MAC_TX_RUNNING    = 0x00000001,
    MAC_RX            = 0x00000002,
    MAC_ACK_REQ       = 0x00000004,
    MAC_ACK_RETRY     = 0x00000008,
    MAC_CHANNEL_CHECK = 0x00000010,
} LoRaMacState_t;

/*!
 * LoRaMAC header field definition
 */
typedef union
{
    uint8_t Value;
    struct
    {
        uint8_t Major           : 2;
        uint8_t RFU             : 3;
        uint8_t MType           : 3;
    }PACKED Bits;
}PACKED LoRaMacHeader_t;

/*!
 * LoRaMAC frame header field definition
 */
typedef union
{
    uint8_t Value;
    struct
    {
        uint8_t FOptsLen        : 4;
        uint8_t FPending        : 1;
        uint8_t Ack             : 1;
        uint8_t AdrAckReq       : 1;
        uint8_t Adr             : 1;
    }PACKED Bits;
}PACKED LoRaMacFrameCtrl_t;

/*!
 * LoRaMAC event flags
 */
typedef union
{
    uint8_t Value;
    struct
    {
        uint8_t Tx              : 1;
        uint8_t Rx              : 1;
        uint8_t RxData          : 1;
        uint8_t Multicast       : 1;
        uint8_t RxSlot          : 2;
        uint8_t LinkCheck       : 1;
        uint8_t JoinAccept      : 1;
    }PACKED Bits;
}PACKED LoRaMacEventFlags_t;

typedef enum
{
    LORAMAC_EVENT_INFO_STATUS_OK = 0,
    LORAMAC_EVENT_INFO_STATUS_ERROR,
    LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT,
    LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT,
    LORAMAC_EVENT_INFO_STATUS_RX2_ERROR,
    LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL,
    LORAMAC_EVENT_INFO_STATUS_DOWNLINK_FAIL,
    LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL,
    LORAMAC_EVENT_INFO_STATUS_MIC_FAIL,
}PACKED LoRaMacEventInfoStatus_t;

/*!
 * LoRaMAC event information
 */
typedef struct
{
    LoRaMacEventInfoStatus_t Status;
    bool TxAckReceived;
    uint8_t TxNbRetries;
    uint8_t TxDatarate;
    uint8_t RxPort;
    uint8_t *RxBuffer;
    uint8_t RxBufferSize;
    int16_t RxRssi;
    uint8_t RxSnr;
    uint16_t Energy;
    uint8_t DemodMargin;
    uint8_t NbGateways;
}PACKED LoRaMacEventInfo_t;

/*!
 * LoRaMAC events structure
 * Used to notify upper layers of MAC events
 */
typedef struct sLoRaMacEvent
{
    /*!
     * MAC layer event callback prototype.
     *
     * \param [IN] flags Bit field indicating the MAC events occurred
     * \param [IN] info  Details about MAC events occurred
     */
    void ( *MacEvent )( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info, void *params );
    void *pParams;
}PACKED LoRaMacEvent_t;

typedef struct sLoRaMacDebug
{
    bool        TrafficMonitorOn;
    bool        PacketDumpOn;
    uint32_t    PacketDumpColumnSize;
}PACKED LoRaMacDebug_t;

typedef struct sLoRaMacConfig
{
    
    uint8_t             DevEui[8];        // Device IEEE EUI
    uint8_t             AppEui[16];        // Application IEEE EUI
    uint8_t             AppKey[16];
    
    uint32_t            NetID;          //  Network ID ( 3 bytes )
    uint32_t            DevAddr;        // Mote Address 

    uint8_t             NwkSKey[16];    // AES encryption/decryption cipher network session key
    uint8_t             AppSKey[16];    // AES encryption/decryption cipher application session key
    
    DeviceClass_t       DeviceClass;
    bool                PublicNetwork;  //Indicates if the node is connected to a private or public network


/*!
 * Channels Tx output power
 */
    int8_t              ChannelsTxPower;

    uint32_t            Frequency;
    uint32_t            OutputPower;
    uint32_t            Bandwidth;
    uint32_t            SpreadingFactor;
    uint32_t            CodingRate;       
    uint32_t            PreambleLength;   
    uint32_t            RxTimeout;        // us
    uint32_t            PayloadCRC;
    bool                FixedLength;
    bool                FrequencyHopOn;
    uint8_t             HopPeriod;
    bool                InvertedIQ;
    uint32_t            TxTimeout;

    Rx2ChannelParams_t  Rx2Channel;   // LoRaMAC 2nd reception window settings
/*!
 * LoRaMac maximum time a reception window stays open
 */
    uint32_t            MaxRxWindow;
    
/*!
 * Number of uplink messages repetitions [1:15] (unconfirmed messages only)
 */
    uint8_t             ChannelsNbRep; //
    int8_t              ChannelsDefaultDatarate;// Channels defualt datarate

    uint8_t             Datarates[8];
    uint8_t             TxPowers[6];
    uint32_t            DutyCycle;
    uint32_t            DutyCycleRange;
    uint16_t            ChannelsMask;     
    
    Band_t              Bands[LORA_MAX_NB_BANDS];
    ChannelParams_t     Channels[LORA_MAX_NB_CHANNELS];

    bool                Bypass;
    LoRaMacDebug_t      Debug;
    
}PACKED LoRaMacConfig_t;

typedef struct sLoRaMacStatus
{    
    LoRaMacState_t      State;
    
    bool                IsNetworkJoined;
  
    uint8_t             Buffer[LORA_MAC_PHY_MAXPAYLOAD]; // Buffer containing the data to be sent or received.
    uint16_t            BufferPktLen;                   //Length of packet in LoRaMac.Status.Buffer
    
/*!
 * Buffer containing the upper layer data.
 */
    uint8_t             Payload[LORA_MAC_PHY_MAXPAYLOAD];
    uint8_t             RxPayload[LORA_MAC_PHY_MAXPAYLOAD];
    
/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
    uint32_t            UpLinkCounter;

/*!
 * LoRaMAC frame counter. Each time a packet is received the counter is incremented.
 * Only the 16 LSB bits are received
 */
    uint32_t            DownLinkCounter;

/*!
 * IsPacketCounterFixed enables the MIC field tests by fixing the
 * UpLinkCounter value
 */
    bool                IsUpLinkCounterFixed;

/*!
 * Used for test purposes. Disables the opening of the reception windows.
 */
    bool                IsRxWindowsEnabled;
    
/*!
 * Indicates if the AckTimeout timer has expired or not
 */
    bool                AckTimeoutRetry;

/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
    bool                NodeAckRequested;
    
/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
 * if the ACK bit must be set for the next transmission
 */
    bool                SrvAckRequested;

/*!
 * Current channel index
 */
    uint8_t             Channel;


/*!
 * LoRaMac notification event flags
 */ 
    LoRaMacEventFlags_t EventFlags;
    
/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
    uint16_t            DevNonce;

/*!
 * Last transmission time on air
 */
    TimerTime_t         TxTimeOnAir;
    
/*!
 * LoRaMac channel check timer
 */
    TimerEvent_t        ChannelCheckTimer;

/*!
 * LoRaMac duty cycle delayed Tx timer
 */
    TimerEvent_t        TxDelayedTimer;

/*!
 * LoRaMac reception windows timers
 */
    TimerEvent_t        RxWindowTimer1;
    TimerEvent_t        RxWindowTimer2;    

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
    TimerEvent_t        AckTimeoutTimer;

/*!
 * LoRaMac timer used to check the LoRaMac.Status.State (runs every second)
 */
    TimerEvent_t        MacStateCheckTimer;
    
/*!
 * LoRaMac reception windows delay from end of Tx
 */
    uint32_t            ReceiveDelay1;
    uint32_t            ReceiveDelay2;
    uint32_t            JoinAcceptDelay1;
    uint32_t            JoinAcceptDelay2;
    
/*!
 * LoRaMac reception windows delay
 * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
 *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
 */
    uint32_t            RxWindow1Delay;
    uint32_t            RxWindow2Delay;

    
/*!
 * Number of trials to get a frame acknowledged
 */
    uint8_t             AckTimeoutRetries ;

/*!
 * Number of trials to get a frame acknowledged
 */
    uint8_t             AckTimeoutRetriesCounter;

/*!
 * Uplink messages repetitions counter
 */
    uint8_t             ChannelsNbRepCounter;

/*!
 * Channels datarate
 */
    int8_t              ChannelsDatarate;
    
/*!
 * Agregated duty cycle management
 */
    uint16_t            AggregatedDCycle;
    uint64_t            AggregatedLastTxDoneTime;
    uint64_t            AggregatedTimeOff;
    
/*!
 * Mutlicast channels linked list
 */
    MulticastParams_t   *MulticastChannels;
    
/*!
 * LoRaMac ADR control status
 */
    bool                AdrCtrlOn;

/*!
 * Counts the number of missed ADR acknowledgements
 */
    uint32_t            AdrAckCounter;

/*!
 * Datarate offset between uplink and downlink on first window
 */
    uint8_t             Rx1DrOffset;

/*!
 * Maximum duty cycle
 * \remark Possibility to shutdown the device. Everything else not implemented.
 */
    uint8_t             MaxDCycle;


/*!
 * Enables/Disables duty cycle management (Test only)
 */
    bool                DutyCycleOn;

/*!
 * Indicates if the MAC layer wants to send MAC commands
 */
    bool                MacCommandsInNextTx;

/*!
 * Contains the current MacCommandsBuffer index
 */
    uint8_t             MacCommandsBufferIndex;

/*!
 * Buffer containing the MAC layer commands
 */
    uint8_t             MacCommandsBuffer[15];




    
    int32_t     RSSI;
    int32_t     SNR;
}PACKED LoRaMacStatus_t;

typedef struct sLoRaMacStatistics
{
    uint32_t    RxPktCount;
    uint32_t    TxPktCount;
}PACKED LoRaMacStatistics_t;

typedef struct sLoRaMac
{
    uint_32             Signature;
    LoRaMacConfig_t     Config;
    LoRaMacStatus_t     Status;
    LoRaMacStatistics_t Statistics;
    LoRaMacEvent_t      Events;
    LoRaMacEventInfo_t  EventInfo;
}PACKED LoRaMac_t;

#define LORAMAC_SIGNATURE           0x53215636
#define LC( channelIndex )              ( uint16_t )( 1 << ( channelIndex - 1 ) )

#define LORAMAC_DEFAULT_CHANNEL_MASK    (LC(1))
/*!
 * LoRaMAC layer initialization
 *
 * \param [IN] events        Pointer to a structure defining the LoRaMAC
 *                           callback functions.
 */
LoRaMac_t *LoRaMacInit( LoRaMacConfig_t *config, LoRaMacEvent_t *events );

/*!
 * Enables/Disables the ADR (Adaptive Data Rate)
 *
 * \param [IN] enable [true: ADR ON, false: ADR OFF]
 */
void LoRaMacSetAdrOn( LoRaMac_t *pLoRaMac, bool enable );

/*!
 * Initializes the network IDs. Device address,
 * network session AES128 key and application session AES128 key.
 *
 * \remark To be only used when Over-the-Air activation isn't used.
 *
 * \param [IN] netID   24 bits network identifier
 *                     ( provided by network operator )
 * \param [IN] devAddr 32 bits device address on the network
 *                     (must be unique to the network)
 * \param [IN] nwkSKey Pointer to the network session AES128 key array
 *                     ( 16 bytes )
 * \param [IN] appSKey Pointer to the application session AES128 key array
 *                     ( 16 bytes )
 */
void LoRaMacInitNwkIds( uint32_t netID, uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey );

/*
 * TODO: Add documentation
 */
void LoRaMacMulticastChannelAdd( LoRaMac_t *pLoRaMac, MulticastParams_t *channelParam );

/*
 * TODO: Add documentation
 */
void LoRaMacMulticastChannelRemove( LoRaMac_t *pLoRaMac, MulticastParams_t *channelParam );

/*!
 * Initiates the Over-the-Air activation
 *
 * \param [IN] devEui Pointer to the device EUI array ( 8 bytes )
 * \param [IN] appEui Pointer to the application EUI array ( 8 bytes )
 * \param [IN] appKey Pointer to the application AES128 key array ( 16 bytes )
 *
 * \retval status [0: OK, 1: Tx error, 2: Already joined a network]
 */
uint8_t LoRaMacJoinReq( LoRaMac_t *pLoRaMac, uint8_t *devEui, uint8_t *appEui, uint8_t *appKey );

/*!
 * Sends a LinkCheckReq MAC command on the next uplink frame
 *
 * \retval status Function status [0: OK, 1: Busy]
 */
uint8_t LoRaMacLinkCheckReq( LoRaMac_t *pLoRaMac );

/*!
 * LoRaMAC layer send raw data
 *
 * \param [IN] fPort       MAC payload port (must be > 0)
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 *
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command
 *                          5: Unable to find a free channel
 *                          6: Device switched off]
 */
uint8_t LoRaMacSendRaw( LoRaMac_t *pLoRaMac, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * LoRaMAC layer send frame
 *
 * \param [IN] fPort       MAC payload port (must be > 0)
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 *
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command
 *                          5: Unable to find a free channel
 *                          6: Device switched off]
 */
uint8_t LoRaMacSendFrame( LoRaMac_t *pLoRaMac, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * LoRaMAC layer send frame
 *
 * \param [IN] fPort       MAC payload port (must be > 0)
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \param [IN] fBufferSize MAC data buffer size
 * \param [IN] nbRetries   Number of retries to receive the acknowledgement
 *
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command
 *                          5: Unable to find a free channel
 *                          6: Device switched off]
 */
uint8_t LoRaMacSendConfirmedFrame( LoRaMac_t *pLoRaMac, uint8_t fPort, void *fBuffer, uint16_t fBufferSize, uint8_t nbRetries );

/*!
 * ============================================================================
 * = LoRaMac test functions                                                   =
 * ============================================================================
 */

/*!
 * LoRaMAC layer generic send frame
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command
 *                          5: Unable to find a free channel
 *                          6: Device switched off]
 */
uint8_t LoRaMacSend( LoRaMac_t *pLoRaMac, LoRaMacHeader_t *macHdr, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

uint8_t LoRaMacJoinAcceptFrame( LoRaMac_t *pLoRaMac, uint32_t devAddr, uint8_t DLSettings, uint8_t RxDelay);

/*!
 * LoRaMAC layer frame buffer initialization.
 *
 * \param [IN] channel     Channel parameters
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          [0: OK, 1: N/A, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command]
 */
uint8_t LoRaMacPrepareFrame( LoRaMac_t *pLoRaMac, ChannelParams_t channel,LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * LoRaMAC layer prepared frame buffer transmission with channel specification
 *
 * \remark LoRaMacPrepareFrame must be called at least once before calling this
 *         function.
 *
 * \param [IN] channel     Channel parameters
 * \retval status          [0: OK, 1: Busy]
 */
uint8_t LoRaMacSendFrameOnChannel( LoRaMac_t *pLoRaMac, ChannelParams_t channel );

/*!
 * LoRaMAC layer generic send frame with channel specification
 *
 * \param [IN] channel     Channel parameters
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command]
 */
uint8_t LoRaMacSendOnChannel( LoRaMac_t *pLoRaMac, ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*
 * TODO: Add documentation
 */
void LoRaMacSetDeviceClass( LoRaMac_t *pLoRaMac, DeviceClass_t deviceClass );

/*
 * TODO: Add documentation
 */
void LoRaMacSetPublicNetwork( LoRaMac_t *pLoRaMac, bool enable );

/*
 * TODO: Add documentation
 */
void LoRaMacSetDutyCycleOn( LoRaMac_t *pLoRaMac, bool enable );

/*
 * TODO: Add documentation
 */
void LoRaMacSetChannel( LoRaMac_t *pLoRaMac, uint8_t id, ChannelParams_t params );

/*
 * TODO: Add documentation
 */
void LoRaMacSetRx2Channel( LoRaMac_t *pLoRaMac, Rx2ChannelParams_t param );

/*
 * TODO: Add documentation
 */
void LoRaMacSetChannelsMask( LoRaMac_t *pLoRaMac, uint16_t mask );

/*
 * TODO: Add documentation
 */
void LoRaMacSetChannelsNbRep( LoRaMac_t *pLoRaMac, uint8_t nbRep );

/*
 * TODO: Add documentation
 */
void LoRaMacSetMaxRxWindow( LoRaMac_t *pLoRaMac, uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetReceiveDelay1( LoRaMac_t *pLoRaMac, uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetReceiveDelay2( LoRaMac_t *pLoRaMac, uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetJoinAcceptDelay1( LoRaMac_t *pLoRaMac, uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetJoinAcceptDelay2( LoRaMac_t *pLoRaMac, uint32_t delay );

/*!
 * Sets channels datarate
 *
 * \param [IN] datarate eu868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
 */
void LoRaMacSetChannelsDatarate( LoRaMac_t *pLoRaMac, int8_t datarate );

/*!
 * Sets channels tx output power
 *
 * \param [IN] txPower [TX_POWER_20_DBM, TX_POWER_14_DBM,
                        TX_POWER_11_DBM, TX_POWER_08_DBM,
                        TX_POWER_05_DBM, TX_POWER_02_DBM]
 */
void LoRaMacSetChannelsTxPower( LoRaMac_t *pLoRaMac, int8_t txPower );

/*!
 * Disables/Enables the reception windows opening
 *
 * \param [IN] enable [true: enable, false: disable]
 */
void LoRaMacTestRxWindowsOn( LoRaMac_t *pLoRaMac, bool enable );

/*
 * TODO: Add documentation
 */
uint32_t LoRaMacGetUpLinkCounter( LoRaMac_t *pLoRaMac );

/*
 * TODO: Add documentation
 */
uint32_t LoRaMacGetDownLinkCounter( LoRaMac_t *pLoRaMac );

/*!
 * Enables the MIC field test
 *
 * \param [IN] txPacketCounter Fixed Tx packet counter value
 */
void LoRaMacSetMicTest( LoRaMac_t *pLoRaMac, uint16_t txPacketCounter );

void LoRaMacTrafficMonitor( LoRaMac_t *pLoRaMac, bool trafficMonitorOn, bool packetDumpOn, uint32_t packetDumpColumnSize);

#endif // __LORAMAC_H__
