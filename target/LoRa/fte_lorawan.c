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
#include "fte_lorawan.h"
#include "fte_shell.h"

//#define FTE_LORAWAN_SUPPORTED   1 
#define FTE_LORAWAN_SINGLE_TEST 1
#if FTE_LORAWAN_SUPPORTED

#undef  __MODULE__
#define __MODULE__  FTE_MODULE_NET_LORA

/*!
 * Radio events function pointer
 */
static  _mqx_uint   FTE_LORAWAN_sendFrame( FTE_LORAWAN_PTR pLoRaWan, FTE_LORAWAN_FRAME_PTR pFrame );
static  void        FTE_LORAWAN_processRxFrame( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info );
static  void        FTE_LORAWAN_onTxNextPacketTimerEvent( void *obj);
static  void        FTE_LORAWAN_onMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info, void *pParams);
static  void        FTE_LORAWAN_OnJoinReqTimerEvent( void * pParams);

static  FTE_UINT32     FTE_LORAWAN_getNextTxDutyCycleTime(void);

extern FTE_LORAWAN_CONFIG   xDefaultConfig;
static FTE_LORAWAN_PTR      pLoRaWan = NULL;

_mqx_uint FTE_LORAWAN_init( void *pConfig)
{
    FTE_DEBUG_traceOn(DEBUG_NET_LORA);

    if (pConfig == NULL)
    {
        pConfig = (void *)&xDefaultConfig;
    }   
    
    // Target board initialisation
    BoardInitMcu( );
    
    if (pLoRaWan == NULL)
    {
        pLoRaWan = (FTE_LORAWAN_PTR)FTE_MEM_allocZero(sizeof(FTE_LORAWAN));
        if (pLoRaWan == NULL)
        {
            return  FTE_RET_NOT_ENOUGH_MEMORY;
        }
    }
    
    pLoRaWan->xState = FTE_LORAWAN_STATE_DISCONNECTED;
    memcpy(&pLoRaWan->xConfig, pConfig, sizeof(FTE_LORAWAN_CONFIG));
    
    FTE_LIST_init(&pLoRaWan->xTxPktList);

    FTE_TASK_create(FTE_TASK_LORAWAN, (FTE_UINT32)pLoRaWan, NULL);

    return  FTE_RET_OK;
}

_mqx_uint FTE_LORAWAN_send(void *pBuff, FTE_UINT32 ulLen)
{
    FTE_LORAWAN_FRAME_PTR   pFrame = NULL;
    
    ASSERT(pBuff != NULL);
    
    if ((ulLen == 0) || (ulLen > FTE_LORAWAN_BUFFER_SIZE))
    {
        return  FTE_RET_ERROR;
    }

    if (FTE_LIST_count(&pLoRaWan->xTxPktList) >= 10)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    pFrame = FTE_MEM_allocZero(sizeof(FTE_LORAWAN_FRAME));
    if (pFrame == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    memcpy(pFrame->pBuffer, pBuff, ulLen);
    pFrame->bSize = ulLen;
    
    if (FTE_LIST_pushBack(&pLoRaWan->xTxPktList, pFrame) != FTE_RET_OK)
    {
        FTE_MEM_free(pFrame);
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_UINT32     FTE_LORAWAN_recv(void *pBuff, FTE_UINT32 ulBuffSize)
{
    return  0;
}


void FTE_LORAWAN_processRxFrame( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    switch( info->RxPort ) // Check Rx port number
    {
        case 1: // The application LED can be controlled on port 1 or 2
        case 2:
            if( info->RxBufferSize == 1 )
            {
            }
            break;
        default:
            break;
    }
}

_mqx_uint FTE_LORAWAN_sendFrame( FTE_LORAWAN_PTR pLoRaWan, FTE_LORAWAN_FRAME_PTR pFrame )
{
    uint8_t sendFrameStatus = 0;

    ASSERT((pLoRaWan != NULL) && (pFrame != NULL));
    
    sendFrameStatus = LoRaMacSendFrame( pLoRaWan->pMac, pFrame->bPort, pFrame->pBuffer, pFrame->bSize);
    //sendFrameStatus = LoRaMacSendConfirmedFrame( pLoRaWan->pMac, bPort, pData, bDataSize, 8 );
    switch( sendFrameStatus )
    {
    case 5: // NO_FREE_CHANNEL
        // Try again later
        return FTE_RET_ERROR;
    default:
        return FTE_RET_OK;
    }
}

/*!
 * \brief Function executed on bTxNextPacket Timeout event
 */
void FTE_LORAWAN_onTxNextPacketTimerEvent( void *obj)
{
    FTE_LORAWAN_PTR pLoRaWan = (FTE_LORAWAN_PTR)obj;
    
    TRACE("onTxNextPacketTimer\n");
    if (pLoRaWan == NULL)
    {
        return;
    }
    
    pLoRaWan->bTxNextPacket = true;
}
/*!
 * \brief Function to be executed on MAC layer event
 */
void FTE_LORAWAN_onMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info, void *pParams)
{
    FTE_LORAWAN_PTR pLoRaWan = (FTE_LORAWAN_PTR)pParams;
    
    ASSERT(pLoRaWan != NULL);
    
    if( flags->Bits.JoinAccept == 1 )
    {
        TRACE("onMacEvent - Join Accept\n");
        if (pLoRaWan->xConfig.bOverTheAirActivation)
        {
            TimerStop( &pLoRaWan->xJoinReqTimer );
        }

        pLoRaWan->bIsNetworkJoined = true;
    }
    
    if( flags->Bits.Tx == 1 )
    {
        TRACE("onMacEvent - Tx Done\n");
    }

    if( flags->Bits.Rx == 1 )
    {
        TRACE("onMacEvent - Rx Done\n");
        if( flags->Bits.RxData == true )
        {
            FTE_LORAWAN_processRxFrame( flags, info );
        }
    }

    // Schedule a new transmission
    pLoRaWan->bTxDone = true;    
}

void FTE_LORAWAN_process(FTE_UINT32 ulParams)
{
    FTE_LORAWAN_PTR         pLoRaWan = (FTE_LORAWAN_PTR)ulParams;
    TimerEvent_t            bTxNextPacketTimer;
    
    ASSERT(pLoRaWan != NULL);

    LoRaMacEvent_t   LoRaMacEvent;
    
    LoRaMacEvent.MacEvent = FTE_LORAWAN_onMacEvent;
    LoRaMacEvent.pParams = (void *)pLoRaWan;
    
    pLoRaWan->pMac = LoRaMacInit( &pLoRaWan->xConfig.xMAC, &LoRaMacEvent);
    
    pLoRaWan->bIsNetworkJoined = false;

    if (! pLoRaWan->xConfig.bOverTheAirActivation )
    {
       // Random seed initialization
        srand( RAND_SEED );
        // Choose a random device address
        // NwkID = 0
        // NwkAddr rand [0, 33554431]
        pLoRaWan->xConfig.xMAC.DevAddr = randr( 0, 0x01FFFFFF );
        
        LoRaMacInitNwkIds( 0x000000, pLoRaWan->xConfig.xMAC.DevAddr, pLoRaWan->xConfig.xMAC.NwkSKey, pLoRaWan->xConfig.xMAC.AppSKey );
        pLoRaWan->bIsNetworkJoined = true;
    }
    else
    {
        // Sends a JoinReq Command every 5 seconds until the network is joined
        TimerInit( &pLoRaWan->xJoinReqTimer, FTE_LORAWAN_OnJoinReqTimerEvent, pLoRaWan ); 
        TimerSetValue( &pLoRaWan->xJoinReqTimer, pLoRaWan->xConfig.ulOverTheAirActivationDutyCycle );
    }
    
    pLoRaWan->bTxNextPacket = true;
    TimerInit( &bTxNextPacketTimer, FTE_LORAWAN_onTxNextPacketTimerEvent, pLoRaWan);
    
    LoRaMacSetAdrOn( pLoRaWan->pMac, true );
    
    FTE_TASK_create(FTE_TASK_LORA_CTRL, (FTE_UINT32)pLoRaWan->pMac, NULL);
    
    while(TRUE)
    {
#if 0
        while( pLoRaWan->bIsNetworkJoined == false )
        {
            if (pLoRaWan->xConfig.bOverTheAirActivation)
            {
                if( pLoRaWan->bTxNextPacket == true )
                {
                    uint8_t sendFrameStatus;
                    
                    pLoRaWan->bTxNextPacket = false;
                    
                    sendFrameStatus = LoRaMacJoinReq( pLoRaWan->pMac, pLoRaWan->xConfig.xMAC.DevEui, pLoRaWan->xConfig.xMAC.AppEui, pLoRaWan->xConfig.xMAC.AppKey);
                    switch( sendFrameStatus )
                    {
                    case 1: // BUSY
                        break;
                    case 0: // OK
                    case 2: // NO_NETWORK_JOINED
                    case 3: // LENGTH_PORT_ERROR
                    case 4: // MAC_CMD_ERROR
                    case 6: // DEVICE_OFF
                    default:
                        // Relaunch timer for next trial
                        TimerStart( &pLoRaWan->xJoinReqTimer );
                        break;
                    }
                }
                TimerLowPowerHandler( );
            }
        }
#endif

        if( pLoRaWan->bTxDone == true )
        {
            pLoRaWan->bTxDone = false;
            
            if (pLoRaWan->pSendFrame != NULL)
            {
                FTE_MEM_free(pLoRaWan->pSendFrame);
                pLoRaWan->pSendFrame = NULL;
            }
            
            // Schedule next packet transmission
            TimerSetValue( &bTxNextPacketTimer, FTE_LORAWAN_getNextTxDutyCycleTime());
            TimerStart( &bTxNextPacketTimer );
        }

        if (pLoRaWan->bTxNextPacket)
        {
            if (FTE_LIST_count(&pLoRaWan->xTxPktList) != 0)
            {
                if (FTE_LIST_popFront(&pLoRaWan->xTxPktList, (void **)&pLoRaWan->pSendFrame) == FTE_RET_OK)
                {
                    pLoRaWan->bTxNextPacket = false;
                    while (FTE_LORAWAN_sendFrame(pLoRaWan, pLoRaWan->pSendFrame) != FTE_RET_OK)
                    {
                        TimerLowPowerHandler( );
                    }
                }
            }
        }
        
        TimerLowPowerHandler( );
    }

}

FTE_UINT32 FTE_LORAWAN_getNextTxDutyCycleTime(void)
{
    FTE_UINT32 ulTxDutyCycleTime = FTE_LORAWAN_TX_DUTYCYCLE;
    
    if (pLoRaWan != NULL)
    {
        ulTxDutyCycleTime = pLoRaWan->xConfig.xMAC.DutyCycle + randr( -pLoRaWan->xConfig.xMAC.DutyCycleRange, pLoRaWan->xConfig.xMAC.DutyCycleRange );
    }
    
    return  ulTxDutyCycleTime;
}

/*!
 * \brief Function executed on JoinReq Timeout event
 */
void FTE_LORAWAN_OnJoinReqTimerEvent( void * pParams)
{
    FTE_LORAWAN_PTR pLoRaWan = (FTE_LORAWAN_PTR)pParams;
    
    ASSERT(pLoRaWan != NULL);
    
    pLoRaWan->bTxNextPacket = true;
#if FTE_LORAWAN_SINGLE_TEST    
    pLoRaWan->bIsNetworkJoined = true;
#endif
}

FTE_INT32  FTE_LORAWAN_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
   
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {  
                printf("%16s : %d MHz\n",   "RF Frequency",     pLoRaWan->pMac->Config.Frequency / 1000000);
                printf("%16s : SF%d\n",     "Spreading Factor", pLoRaWan->pMac->Config.SpreadingFactor); 
                printf("%16s : 4/%d\n",     "Error Coding",     pLoRaWan->pMac->Config.CodingRate + 4);
                printf("%16s : %s\n",       "Payload CRC",      pLoRaWan->pMac->Config.PayloadCRC?"ON":"OFF");
                printf("%16s : ",           "Bandwidth");
                switch(pLoRaWan->pMac->Config.Bandwidth)
                {
                case    FTE_LORAWAN_BANDWITDH_125KHZ: printf(" 125 kHz\n"); break;
                case    FTE_LORAWAN_BANDWITDH_250KHZ: printf(" 250 kHz\n"); break;
                case    FTE_LORAWAN_BANDWITDH_500KHZ: printf(" 500 kHz\n"); break;
                default:                           printf(" ??? kHz\n"); break;
                };
            }
            break;
        case    2:
            {
                if (strcasecmp(pArgv[1], "regs") == 0)
                {
                    FTE_UINT8  pRegs[64];
                    FTE_UINT32 i;
                    const   FTE_CHAR_PTR pBandwidths[10] = { "7.8", "10.4", "15.6", "20.8", "31.25", "41.7", "62.5", "125", "250", "500"};
                    
                    SX1276ReadBuffer( 0, pRegs, sizeof(pRegs));
                    
                    for(i = 0 ; i < 64 ; i++)
                    {
                        printf("%02x ", pRegs[i]);
                        if ((i+1) % 16 == 0)
                        {
                            printf("\n");
                        }
                    }
                    FTE_UINT32 ulReg = ((FTE_UINT32)pRegs[6] << 16) | ((FTE_UINT32)pRegs[7] << 8) | ((FTE_UINT32)pRegs[8]);
                    FTE_UINT32 ulFrequency = ( uint32_t )( ( double )ulReg * ( double )FREQ_STEP );
                    
                    printf("%20s : %d MHz\n", "Frequency", ulFrequency / 1000000);
                    printf("%20s : %s\n", "Long Range Mode", (pRegs[1] >> 7)?"LoRa":"FSK");
                    printf("%20s : %s\n", "Frequency Mode",((pRegs[1] >> 3) & 0x01)?"Low":"High");
                    printf("%20s : %d\n", "Mode", (pRegs[0x01] & 0x7));
                    
                    printf("%20s : %s\n", "PA Select", (pRegs[0x09] >> 7)?"PA_BOOST":"RFO");
                    FTE_UINT32 ulMaxPower = 108 + 6*((pRegs[0x09] >> 4) & 0x07);
                    printf("%20s : %d.%d dBm\n", "Max Power", ulMaxPower/10, ulMaxPower%10);
                    FTE_UINT32 ulPower = ulMaxPower - (15 - ((pRegs[0x09]) & 0x0F))*10;
                    printf("%20s : %d.%d dBm\n", "Output Power", ulPower / 10, ulPower % 10);
                    printf("%20s : %s kHz\n", "Bandwidth",  pBandwidths[((pRegs[0x1d] >> 4) & 0x0F)] );
                    printf("%20s : 4/%d\n", "Coding Rate", ((pRegs[0x1d] >> 1) & 0x07) + 4);
                    printf("%20s : %s\n", "Implicit Header", (pRegs[0x1d] & 0x01)?"ON":"OFF");
                    printf("%20s : SF%d\n", "Spreading Factor", ((pRegs[0x1E] >> 4) & 0x0F));
                    printf("%20s : %s\n", "Rx Payload CRC On", ((pRegs[0x1E] >> 2) & 0x01)?"Enable":"Disable");
                    printf("%20s : %d\n", "Preamble Length", ((FTE_UINT16)pRegs[0x20] << 8) | pRegs[0x21]);
                    printf("%20s : %d\n", "Payload Length", pRegs[0x13]);
                    printf("%20s : %d\n", "Payload Max Length", pRegs[0x23]);
                    printf("%20s : %s\n", "Data Rate Optimize", ((pRegs[0x26] >> 3) & 0x01)?"ON":"OFF");
                    printf("%20s : %s\n", "Invert IQ", ((pRegs[0x33] >> 6) & 0x01)?"Inverted":"Normal");
                    printf("%20s : %s\n", "Detection Threshold", ((pRegs[0x37] == 0x0A)?"SF7 to SF12":"SF6"));
                }
                else if (strcasecmp(pArgv[1], "config") == 0)
                {
                    printf("%20s : ", "Device EUI");
                    FTE_SHELL_printHexString(pLoRaWan->xConfig.pDevEui, sizeof(pLoRaWan->xConfig.pDevEui), 0);
                    printf("\n");
                    
                    printf("%20s : ", "Application EUI");
                    FTE_SHELL_printHexString(pLoRaWan->xConfig.pAppEui, sizeof(pLoRaWan->xConfig.pAppEui), 0);
                    printf("\n");
                    
                    printf("%20s : ", "Application Key");
                    FTE_SHELL_printHexString(pLoRaWan->xConfig.pAppKey, sizeof(pLoRaWan->xConfig.pAppKey), 0);
                    printf("\n");

                    printf("%20s : %08x\n", "NetID",       pLoRaWan->xConfig.xMAC.NetID);
                    printf("%20s : %08x\n", "DevAddress",   pLoRaWan->xConfig.xMAC.NetID);
                    
                    printf("%20s : ", "NwkSKey");
                    FTE_SHELL_printHexString(pLoRaWan->xConfig.xMAC.NwkSKey, sizeof(pLoRaWan->xConfig.xMAC.NwkSKey), 0);
                    printf("\n");

                    printf("%20s : ", "AppSKey");
                    FTE_SHELL_printHexString(pLoRaWan->xConfig.xMAC.AppSKey, sizeof(pLoRaWan->xConfig.xMAC.AppSKey), 0);
                    printf("\n");
                    
                    printf("%20s : ", "Data Rate");
                    for(int i = 0 ; i < sizeof(pLoRaWan->xConfig.xMAC.Datarates) / sizeof(pLoRaWan->xConfig.xMAC.Datarates[0]) ; i++)
                    {
                        printf("%3d ", pLoRaWan->xConfig.xMAC.Datarates[i]);
                    }
                    printf("\n");
                    
                    printf("%20s : ", "Tx Powers");
                    for(int i = 0 ; i < sizeof(pLoRaWan->xConfig.xMAC.TxPowers) / sizeof(pLoRaWan->xConfig.xMAC.TxPowers[0]) ; i++)
                    {
                        printf("%3d ", pLoRaWan->xConfig.xMAC.TxPowers[i]);
                    }
                    printf("\n");
                    
                    printf("%20s : CLASS ", "Device Class");
                    switch(pLoRaWan->xConfig.xMAC.DeviceClass)
                    {
                    case    CLASS_A: printf("A\n");
                    case    CLASS_B: printf("B\n");
                    case    CLASS_C: printf("C\n");
                    }
                    
                    printf("%20s : %d\n", "Channels Tx Power", pLoRaWan->xConfig.xMAC.ChannelsTxPower);
                    printf("%20s : %d\n", "Frequency", pLoRaWan->xConfig.xMAC.Frequency);
                    printf("%20s : %d\n", "Output Power", pLoRaWan->xConfig.xMAC.OutputPower);
                    printf("%20s : %d\n", "Bandwidth", pLoRaWan->xConfig.xMAC.Bandwidth);
                    printf("%20s : %d\n", "Spreading Factor", pLoRaWan->xConfig.xMAC.SpreadingFactor);
                    printf("%20s : %d\n", "Coding Rate", pLoRaWan->xConfig.xMAC.CodingRate);
                    printf("%20s : %d\n", "Preamble Length", pLoRaWan->xConfig.xMAC.PreambleLength);
                    printf("%20s : %d\n", "Payload CRC", pLoRaWan->xConfig.xMAC.PayloadCRC);
                    printf("%20s : %d\n", "Fixed Length", pLoRaWan->xConfig.xMAC.FixedLength);
                    printf("%20s : %d\n", "Frequency Hop On", pLoRaWan->xConfig.xMAC.FrequencyHopOn);
                    printf("%20s : %d\n", "Hop Period", pLoRaWan->xConfig.xMAC.HopPeriod);
                    printf("%20s : %d\n", "Inverted IQ", pLoRaWan->xConfig.xMAC.InvertedIQ);
                    printf("%20s : %d\n", "Rx Timeout", pLoRaWan->xConfig.xMAC.RxTimeout);
                    printf("%20s : %d\n", "Tx Timeout", pLoRaWan->xConfig.xMAC.TxTimeout);

                    printf("%20s : %d\n", "Max Rx Window", pLoRaWan->xConfig.xMAC.MaxRxWindow);
                    
                    printf("%20s : %d\n", "Duty Cycle", pLoRaWan->xConfig.xMAC.DutyCycle);
                    printf("%20s : %d\n", "Duty Cycle Range", pLoRaWan->xConfig.xMAC.DutyCycleRange);
        
                    printf("\n%20s\n", "Second Receive Window");
                    printf("%20s : %d\n", "Frequency", pLoRaWan->xConfig.xMAC.Rx2Channel.Frequency);
                    printf("%20s : %d\n", "Datarate", pLoRaWan->xConfig.xMAC.Rx2Channel.Datarate);

                    printf("\n%20s\n", "Channels");
                    printf("%20s : %d\n", "Nb Rep", pLoRaWan->xConfig.xMAC.ChannelsNbRep);
                    printf("%20s : %d\n", "Default Datarate", pLoRaWan->xConfig.xMAC.ChannelsDefaultDatarate);
                    printf("%20s : ", "Mask");
                    for(int i = 0 ; i < 16 ; i++)
                    {
                        if (pLoRaWan->xConfig.xMAC.ChannelsMask & ( 1 << i ))
                        {
                            printf("%2d ", i+1);
                        }
                    }
                    printf("\n");
                    printf("%20s :\n", "Bands");
                    for(int i = 0 ; i < sizeof(pLoRaWan->xConfig.xMAC.Bands) / sizeof(pLoRaWan->xConfig.xMAC.Bands[0]); i++)
                    {
                        printf("%20d : %2d, %2d dBm, %d, %d\n", i+1, 
                               pLoRaWan->xConfig.xMAC.Bands[i].DCycle,
                               pLoRaWan->xConfig.xMAC.Bands[i].TxMaxPower,
                               pLoRaWan->xConfig.xMAC.Bands[i].LastTxDoneTime,
                               pLoRaWan->xConfig.xMAC.Bands[i].TimeOff);
                    }

                    printf("%20s :\n", "Channels");
                    for(int i = 0 ; i < sizeof(pLoRaWan->xConfig.xMAC.Channels) / sizeof(pLoRaWan->xConfig.xMAC.Channels[0]); i++)
                    {
                        if (pLoRaWan->xConfig.xMAC.Channels[i].Band != 0)
                        {
                            printf("%20d : %9d Hz, %2d, %2d, %d\n", i+1, 
                                   pLoRaWan->xConfig.xMAC.Channels[i].Band,
                                   pLoRaWan->xConfig.xMAC.Channels[i].DrRange.Fields.Min,
                                   pLoRaWan->xConfig.xMAC.Channels[i].DrRange.Fields.Max,
                                   pLoRaWan->xConfig.xMAC.Channels[i].Frequency );
                        }
                    }

                    printf("%20s\n", "Over the Air Activation");
                    printf("%20s : %s\n", "Status", pLoRaWan->xConfig.bOverTheAirActivation?"ON":"OFF");
                    printf("%20s : %d\n", "Duty Cycle", pLoRaWan->xConfig.ulOverTheAirActivationDutyCycle);
                }
            }
            break;
            
        case    3:
            {
                if (strcmp(pArgv[1], "send") == 0)
                {
                    FTE_LORAWAN_send(pArgv[2], strlen(pArgv[2]));
                }
                else if (strcasecmp(pArgv[1], "trace") == 0)
                {
                    if (strcasecmp(pArgv[2], "on") == 0)
                    {
                        FTE_DEBUG_traceOn(DEBUG_NET_LORA);
                    }
                    else if (strcasecmp(pArgv[2], "off") == 0)
                    {
                        FTE_DEBUG_traceOff(DEBUG_NET_LORA);
                    }
                }
                else if (strcasecmp(pArgv[1], "pktdump") == 0)
                {
                    if (FTE_DEBUG_isTraceOn(DEBUG_NET_LORA))
                    {
                        if (strcasecmp(pArgv[2], "on") == 0)
                        {
                            LoRaMacTrafficMonitor( pLoRaWan->pMac, true, true, 16);
                        }
                        else if (strcasecmp(pArgv[2], "off") == 0)
                        {
                            LoRaMacTrafficMonitor( pLoRaWan->pMac, false, false, 16);
                        }
                    }
                    else
                    {
                        printf("Not configured LoRaWan Trace\n");
                    }
                }
                else if (strcmp(pArgv[1], "send_test") == 0)
                {
                    FTE_CHAR    pBuff[16];
                    FTE_UINT32  ulCount;
                    
                    if (FTE_strToUINT32(pArgv[2], &ulCount) != FTE_RET_OK)
                    {
                        break;
                    }
                    
                    for(int i = 0; i < ulCount ; i++)
                    {
                        sprintf(pBuff, "IDX : %04d", i+1);
                        FTE_LORAWAN_send(pBuff, strlen(pBuff));
                        
                        _time_delay(1000);
                    }
                }
                else if (strcasecmp(pArgv[1], "rr") == 0)
                {
                    FTE_UINT32 ulReg;
                    FTE_strToHex(pArgv[2], &ulReg);
                    
                    FTE_UINT8  bReg = SX1276Read( ulReg);
                    printf("Read : %02x - %02x\n", ulReg, bReg);
                }
                else if (strcasecmp(pArgv[1], "sf") == 0)
                {
                    FTE_UINT32 ulSF;
                    
                    xRet = FTE_strToUINT32(pArgv[2], &ulSF);
                    if (xRet != FTE_RET_OK)
                    {
                        break;
                    }
                    
                    printf("Change SF : SF%d -> ", pLoRaWan->pMac->Config.SpreadingFactor);
                    printf("SF%d\n", ulSF);
                    pLoRaWan->pMac->Config.SpreadingFactor = ulSF;
                }
                else if (strcasecmp(pArgv[1], "bw") == 0)
                {
                    FTE_UINT32 ulBandwidth;
                    xRet = FTE_strToUINT32(pArgv[2], &ulBandwidth);
                    if (xRet != FTE_RET_OK)
                    {
                        break;
                    }
                    
                    printf("Change BW : BW%d -> ", pLoRaWan->pMac->Config.Bandwidth);
                    printf("BW%d\n", ulBandwidth);
                    pLoRaWan->pMac->Config.Bandwidth = ulBandwidth;
                }
                else if (strcasecmp(pArgv[1], "crc") == 0)
                {
                    if (strcasecmp(pArgv[2], "on") == 0)
                    {
                        pLoRaWan->pMac->Config.PayloadCRC = true;
                    }
                    else if (strcasecmp(pArgv[2], "off") == 0)
                    {
                        pLoRaWan->pMac->Config.PayloadCRC = false;
                    }                    
                }
                else if (strcasecmp(pArgv[1], "deveui") == 0)
                {
                    if (strlen(pArgv[2]) != 16)
                    {
                        printf("Invalid parameters\n");
                        bPrintUsage = TRUE;
                    }
                    else
                    {
                        FTE_UINT32  ulLen;
                        FTE_UINT8   pDevEUI[8];
                        
                        xRet = FTE_strToHexArray(pArgv[2], pDevEUI, 8, &ulLen);
                        if (( xRet != FTE_RET_OK) || (ulLen != 8))
                        {
                            printf("Invalid parameters\n");
                            bPrintUsage = TRUE;
                        }
                        else
                        {
                            memcpy(pLoRaWan->pMac->Config.DevEui, pDevEUI, 8);
                        }
                    }
                }
                else if (strcasecmp(pArgv[1], "appeui") == 0)
                {
                    if (strlen(pArgv[2]) != LORAWAN_APP_EUI_LENGTH * 2)
                    {
                        printf("Invalid parameters\n");
                        bPrintUsage = TRUE;
                    }
                    else
                    {
                        FTE_UINT32  ulLen;
                        FTE_UINT8  pAppEUI[LORAWAN_APP_EUI_LENGTH];
                        
                        xRet = FTE_strToHexArray(pArgv[2], pAppEUI, LORAWAN_APP_EUI_LENGTH, &ulLen);
                        if ((xRet != FTE_RET_OK) || (ulLen != LORAWAN_APP_EUI_LENGTH))
                        {
                            printf("Invalid parameters\n");
                            bPrintUsage = TRUE;
                        }
                        else
                        {
                            memcpy(pLoRaWan->pMac->Config.AppEui, pAppEUI, LORAWAN_APP_EUI_LENGTH);
                        }
                    }
                }
                else if (strcasecmp(pArgv[1], "appkey") == 0)
                {
                    if (strlen(pArgv[2]) != LORAWAN_APP_KEY_LENGTH * 2)
                    {
                        printf("Invalid parameters\n");
                        bPrintUsage = TRUE;
                    }
                    else
                    {
                        FTE_UINT32  ulLen;
                        FTE_UINT8  pAppEUI[LORAWAN_APP_KEY_LENGTH];
                        
                        xRet = FTE_strToHexArray(pArgv[2], pAppEUI, LORAWAN_APP_KEY_LENGTH, &ulLen);
                        if ((xRet != FTE_RET_OK) || (ulLen != LORAWAN_APP_KEY_LENGTH))
                        {
                            printf("Invalid parameters\n");
                            bPrintUsage = TRUE;
                        }
                        else
                        {
                            memcpy(pLoRaWan->pMac->Config.AppKey, pAppKey, LORAWAN_APP_KEY_LENGTH);
                        }
                    }
                }
            }
            break;
           
        case    4:
            {
                if (strcasecmp(pArgv[1], "wr") == 0)
                {
                    FTE_UINT32 ulReg, ulValue;
                    
                    FTE_strToHex(pArgv[2], &ulReg);
                    FTE_strToHex(pArgv[3], &ulValue);
                    
                    FTE_UINT8 bTemp = SX1276Read( REG_OPMODE ) ;
                    
                    SX1276Write( REG_OPMODE, ( bTemp & RF_OPMODE_MASK ) | RF_OPMODE_SLEEP);
                    
                    SX1276Write( ulReg, (FTE_UINT8)ulValue);
                    FTE_UINT8  bReg = SX1276Read( ulReg);

                    SX1276Write( REG_OPMODE, bTemp);
                    
                    printf("Write : %02x - %02x - %s\n", ulReg, ulValue, (ulValue == bReg)?"OK":"FAIL");
                }
            }
            break;
            
        default:
            bPrintUsage = TRUE;
        }
       
    }
   
  
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<command>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<command>]\n", pArgv[0]);
            printf("  Commands:\n");
            printf("  Parameters:\n");
        }
    }
    return   xRet;
}

#endif

