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
#include "fte_s2lora.h"
#include "fte_shell.h"

//#define FTE_S2LORA_SUPPORTED   1 
#define FTE_S2LORA_SINGLE_TEST 1
#if FTE_S2LORA_SUPPORTED


/*!
 * Radio events function pointer
 */
static  _mqx_uint   FTE_S2LORA_sendFrame( FTE_S2LORA_PTR pS2LORA, FTE_S2LORA_FRAME_PTR pFrame );
static  void        FTE_S2LORA_processRxFrame( FTE_S2LORA_PTR pS2LORA, LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info );
static  void        FTE_S2LORA_onTxNextPacketTimerEvent( void *obj);
static  void        FTE_S2LORA_onMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info, void *pParams);
static  void        FTE_S2LORA_OnJoinReqTimerEvent( void * pParams);
static  FTE_UINT32     FTE_S2LORA_getNextTxDutyCycleTime(FTE_S2LORA_PTR pS2LORA);

void SX1276OnDio0Irq(void *);
void SX1276OnDio1Irq(void *);
void SX1276OnDio2Irq(void *);
void SX1276OnDio3Irq(void *);

extern FTE_S2LORA_CONFIG   xDefaultConfig;
static FTE_S2LORA_PTR      pS2LORA = NULL;
static FTE_UCS_PTR         pUCS = NULL;

_mqx_uint FTE_S2LORA_init( void *pConfig)
{
    FTE_DEBUG_traceOn(DEBUG_NET_LORA);

    pUCS = (FTE_UCS_PTR)FTE_UCS_get(FTE_DEV_UCS_1);
    if (pUCS != NULL)
    {
        FTE_UCS_attach(pUCS, 0);
    }
    
    if (pConfig == NULL)
    {
        pConfig = (void *)&xDefaultConfig;
    }   
    
    // Target board initialisation
    BoardInitMcu( );
    
    if (pS2LORA == NULL)
    {
        pS2LORA = (FTE_S2LORA_PTR)FTE_MEM_allocZero(sizeof(FTE_S2LORA));
        if (pS2LORA == NULL)
        {
            return  FTE_RET_NOT_ENOUGH_MEMORY;
        }
    }
    
    pS2LORA->xState = FTE_S2LORA_STATE_DISCONNECTED;
    memcpy(&pS2LORA->xConfig, pConfig, sizeof(FTE_S2LORA_CONFIG));
    
    FTE_LIST_init(&pS2LORA->xTxPktList);

    FTE_TASK_create(FTE_TASK_S2LORA, (FTE_UINT32)pS2LORA, NULL);

    return  FTE_RET_OK;
}

_mqx_uint FTE_S2LORA_send(void *pBuff, FTE_UINT32 ulLen)
{
    FTE_S2LORA_FRAME_PTR   pFrame = NULL;
    
    ASSERT(pBuff != NULL);
    
    if ((ulLen == 0) || (ulLen > FTE_S2LORA_BUFFER_SIZE))
    {
        return  FTE_RET_ERROR;
    }

    if (FTE_LIST_count(&pS2LORA->xTxPktList) >= 10)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    pFrame = FTE_MEM_allocZero(sizeof(FTE_S2LORA_FRAME));
    if (pFrame == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    memcpy(pFrame->pBuffer, pBuff, ulLen);
    pFrame->bSize = ulLen;
    
    if (FTE_LIST_pushBack(&pS2LORA->xTxPktList, pFrame) != FTE_RET_OK)
    {
        FTE_MEM_free(pFrame);
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_UINT32     FTE_S2LORA_recv(void *pBuff, FTE_UINT32 ulBuffSize)
{
    return  0;
}


void FTE_S2LORA_processRxFrame( FTE_S2LORA_PTR pS2LORA, LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    TRACE(DEBUG_NET_LORA, "Rx[%04d] %4d - ", ++pS2LORA->ulRxPktCount, info->RxBufferSize );
    for(int i = 0 ; i < info->RxBufferSize ; i++)
    {
        printf("%02x ", info->RxBuffer[i]);
    }
    printf("\n");
    
    FTE_UCS_send(pUCS, info->RxBuffer, info->RxBufferSize, false);
}

_mqx_uint FTE_S2LORA_sendFrame( FTE_S2LORA_PTR pS2LORA, FTE_S2LORA_FRAME_PTR pFrame )
{
    uint8_t sendFrameStatus = 0;

    ASSERT((pS2LORA != NULL) && (pFrame != NULL));
    
    sendFrameStatus = LoRaMacSendRaw( pS2LORA->pMac, pFrame->bPort, pFrame->pBuffer, pFrame->bSize);
    //sendFrameStatus = LoRaMacSendConfirmedFrame( pS2LORA->pMac, bPort, pData, bDataSize, 8 );
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
void FTE_S2LORA_onTxNextPacketTimerEvent( void *obj)
{
    FTE_S2LORA_PTR pS2LORA = (FTE_S2LORA_PTR)obj;
    
    TRACE(DEBUG_NET_LORA, "onTxNextPacketTimer\n");
    if (pS2LORA == NULL)
    {
        return;
    }
    
    pS2LORA->bTxNextPacket = true;
}
/*!
 * \brief Function to be executed on MAC layer event
 */
void FTE_S2LORA_onMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info, void *pParams)
{
    FTE_S2LORA_PTR pS2LORA = (FTE_S2LORA_PTR)pParams;
    
    ASSERT(pS2LORA != NULL);
    
    if( flags->Bits.JoinAccept == 1 )
    {
        TRACE(DEBUG_NET_LORA, "onMacEvent - Join Accept\n");
        if (pS2LORA->xConfig.bOverTheAirActivation)
        {
            TimerStop( &pS2LORA->xJoinReqTimer );
        }

        pS2LORA->bIsNetworkJoined = true;
    }
    
    if( flags->Bits.Tx == 1 )
    {

        if (pS2LORA->pSendFrame != NULL)
        {
            FTE_MEM_free(pS2LORA->pSendFrame);
            pS2LORA->pSendFrame = NULL;
        }
        TRACE(DEBUG_NET_LORA, "onMacEvent - Tx Done\n");
    }

    if( flags->Bits.Rx == 1 )
    {
        TRACE(DEBUG_NET_LORA, "onMacEvent - Rx Done\n");
        if( flags->Bits.RxData == true )
        {
            FTE_S2LORA_processRxFrame( pS2LORA, flags, info );
        }
    }

    // Schedule a new transmission
    pS2LORA->bTxDone = true;    
}

void FTE_S2LORA_process(FTE_UINT32 ulParams)
{
    FTE_S2LORA_PTR         pS2LORA = (FTE_S2LORA_PTR)ulParams;
    TimerEvent_t            bTxNextPacketTimer;
    FTE_UINT8                  pSendBuff[256];
    
    ASSERT(pS2LORA != NULL);

    LoRaMacEvent_t   LoRaMacEvent;
    
    LoRaMacEvent.MacEvent = FTE_S2LORA_onMacEvent;
    LoRaMacEvent.pParams = (void *)pS2LORA;
    
    pS2LORA->pMac = LoRaMacInit( &pS2LORA->xConfig.xMAC, &LoRaMacEvent);
    
    pS2LORA->bIsNetworkJoined = false;

    if (! pS2LORA->xConfig.bOverTheAirActivation )
    {
       // Random seed initialization
        srand( RAND_SEED );
        // Choose a random device address
        // NwkID = 0
        // NwkAddr rand [0, 33554431]
        pS2LORA->xConfig.xMAC.DevAddr = randr( 0, 0x01FFFFFF );
        
        LoRaMacInitNwkIds( 0x000000, pS2LORA->xConfig.xMAC.DevAddr, pS2LORA->xConfig.xMAC.NwkSKey, pS2LORA->xConfig.xMAC.AppSKey );
        pS2LORA->bIsNetworkJoined = true;
    }
    else
    {
        // Sends a JoinReq Command every 5 seconds until the network is joined
        TimerInit( &pS2LORA->xJoinReqTimer, FTE_S2LORA_OnJoinReqTimerEvent, pS2LORA ); 
        TimerSetValue( &pS2LORA->xJoinReqTimer, pS2LORA->xConfig.ulOverTheAirActivationDutyCycle );
    }
    
    pS2LORA->bTxNextPacket = true;
    TimerInit( &bTxNextPacketTimer, FTE_S2LORA_onTxNextPacketTimerEvent, pS2LORA);
    
    LoRaMacSetAdrOn( pS2LORA->pMac, true );
    
    FTE_TASK_create(FTE_TASK_LORA_CTRL, (FTE_UINT32)pS2LORA->pMac, NULL);
    
    while(TRUE)
    {
        FTE_UINT32 ulSerialDataLen = 0;
        FTE_UINT32 ulSendBuffLen = sizeof(pSendBuff);
        
        while(ulSendBuffLen > 0)
        {
            FTE_UINT32 ulLen = FTE_UCS_recv(pUCS, &pSendBuff[ulSerialDataLen], ulSendBuffLen - ulSerialDataLen);
            if (ulLen == 0)
            {
                break;
            }
            
            ulSerialDataLen += ulLen;
            _time_delay(1);
        }

        if (ulSerialDataLen != 0)
        {
            FTE_S2LORA_send(pSendBuff, ulSerialDataLen);
        }
        
        if( pS2LORA->bTxDone == true )
        {
            pS2LORA->bTxDone = false;
            
            // Schedule next packet transmission
            TimerSetValue( &bTxNextPacketTimer, FTE_S2LORA_getNextTxDutyCycleTime(pS2LORA));
            TimerStart( &bTxNextPacketTimer );
        }

        if (pS2LORA->bTxNextPacket)
        {
            if (FTE_LIST_count(&pS2LORA->xTxPktList) != 0)
            {
                if (FTE_LIST_popFront(&pS2LORA->xTxPktList, (void **)&pS2LORA->pSendFrame) == FTE_RET_OK)
                {
                    pS2LORA->bTxNextPacket = false;
                    while (FTE_S2LORA_sendFrame(pS2LORA, pS2LORA->pSendFrame) != FTE_RET_OK)
                    {
                        TimerLowPowerHandler( );
                    }
                }
            }
        }
        
        TimerLowPowerHandler( );
    }

}

FTE_UINT32 FTE_S2LORA_getNextTxDutyCycleTime(FTE_S2LORA_PTR pS2LORA)
{
    ASSERT(pS2LORA != NULL);
    
    return  pS2LORA->xConfig.xMAC.DutyCycle + randr( -pS2LORA->xConfig.xMAC.DutyCycleRange, pS2LORA->xConfig.xMAC.DutyCycleRange );
}

/*!
 * \brief Function executed on JoinReq Timeout event
 */
void FTE_S2LORA_OnJoinReqTimerEvent( void * pParams)
{
    FTE_S2LORA_PTR pS2LORA = (FTE_S2LORA_PTR)pParams;
    
    ASSERT(pS2LORA != NULL);
    
    pS2LORA->bTxNextPacket = true;
#if FTE_S2LORA_SINGLE_TEST    
    pS2LORA->bIsNetworkJoined = true;
#endif
}

FTE_INT32  FTE_S2LORA_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL     bPrintUsage, bShortHelp = FALSE;
    FTE_INT32      xRet = SHELL_EXIT_SUCCESS;
   
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {  
                printf("%16s : %d MHz\n",   "RF Frequency",     pS2LORA->pMac->Config.Frequency / 1000000);
                printf("%16s : SF%d\n",     "Spreading Factor", pS2LORA->pMac->Config.SpreadingFactor); 
                printf("%16s : 4/%d\n",     "Error Coding",     pS2LORA->pMac->Config.CodingRate + 4);
                printf("%16s : %s\n",       "Payload CRC",      pS2LORA->pMac->Config.PayloadCRC?"ON":"OFF");
                printf("%16s : ",           "Bandwidth");
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
                    FTE_SHELL_printHexString(pS2LORA->xConfig.pDevEui, sizeof(pS2LORA->xConfig.pDevEui), 0);
                    printf("\n");
                    
                    printf("%20s : ", "Application EUI");
                    FTE_SHELL_printHexString(pS2LORA->xConfig.pAppEui, sizeof(pS2LORA->xConfig.pAppEui), 0);
                    printf("\n");
                    
                    printf("%20s : ", "Application Key");
                    FTE_SHELL_printHexString(pS2LORA->xConfig.pAppKey, sizeof(pS2LORA->xConfig.pAppKey), 0);
                    printf("\n");

                    printf("%20s : %08x\n", "NetID",       pS2LORA->xConfig.xMAC.NetID);
                    printf("%20s : %08x\n", "DevAddress",   pS2LORA->xConfig.xMAC.NetID);
                    
                    printf("%20s : ", "NwkSKey");
                    FTE_SHELL_printHexString(pS2LORA->xConfig.xMAC.NwkSKey, sizeof(pS2LORA->xConfig.xMAC.NwkSKey), 0);
                    printf("\n");

                    printf("%20s : ", "AppSKey");
                    FTE_SHELL_printHexString(pS2LORA->xConfig.xMAC.AppSKey, sizeof(pS2LORA->xConfig.xMAC.AppSKey), 0);
                    printf("\n");
                    
                    printf("%20s : ", "Data Rate");
                    for(int i = 0 ; i < sizeof(pS2LORA->xConfig.xMAC.Datarates) / sizeof(pS2LORA->xConfig.xMAC.Datarates[0]) ; i++)
                    {
                        printf("%3d ", pS2LORA->xConfig.xMAC.Datarates[i]);
                    }
                    printf("\n");
                    
                    printf("%20s : ", "Tx Powers");
                    for(int i = 0 ; i < sizeof(pS2LORA->xConfig.xMAC.TxPowers) / sizeof(pS2LORA->xConfig.xMAC.TxPowers[0]) ; i++)
                    {
                        printf("%3d ", pS2LORA->xConfig.xMAC.TxPowers[i]);
                    }
                    printf("\n");
                    
                    printf("%20s : CLASS ", "Device Class");
                    switch(pS2LORA->xConfig.xMAC.DeviceClass)
                    {
                    case    CLASS_A: printf("A\n");
                    case    CLASS_B: printf("B\n");
                    case    CLASS_C: printf("C\n");
                    }
                    
                    printf("%20s : %d\n", "Channels Tx Power", pS2LORA->xConfig.xMAC.ChannelsTxPower);
                    printf("%20s : %d\n", "Frequency", pS2LORA->xConfig.xMAC.Frequency);
                    printf("%20s : %d\n", "Output Power", pS2LORA->xConfig.xMAC.OutputPower);
                    printf("%20s : %d\n", "Bandwidth", pS2LORA->xConfig.xMAC.Bandwidth);
                    printf("%20s : %d\n", "Spreading Factor", pS2LORA->xConfig.xMAC.SpreadingFactor);
                    printf("%20s : %d\n", "Coding Rate", pS2LORA->xConfig.xMAC.CodingRate);
                    printf("%20s : %d\n", "Preamble Length", pS2LORA->xConfig.xMAC.PreambleLength);
                    printf("%20s : %d\n", "Payload CRC", pS2LORA->xConfig.xMAC.PayloadCRC);
                    printf("%20s : %d\n", "Fixed Length", pS2LORA->xConfig.xMAC.FixedLength);
                    printf("%20s : %d\n", "Frequency Hop On", pS2LORA->xConfig.xMAC.FrequencyHopOn);
                    printf("%20s : %d\n", "Hop Period", pS2LORA->xConfig.xMAC.HopPeriod);
                    printf("%20s : %d\n", "Inverted IQ", pS2LORA->xConfig.xMAC.InvertedIQ);
                    printf("%20s : %d\n", "Rx Timeout", pS2LORA->xConfig.xMAC.RxTimeout);
                    printf("%20s : %d\n", "Tx Timeout", pS2LORA->xConfig.xMAC.TxTimeout);

                    printf("%20s : %d\n", "Max Rx Window", pS2LORA->xConfig.xMAC.MaxRxWindow);
                    
                    printf("%20s : %d\n", "Duty Cycle", pS2LORA->xConfig.xMAC.DutyCycle);
                    printf("%20s : %d\n", "Duty Cycle Range", pS2LORA->xConfig.xMAC.DutyCycleRange);
        
                    printf("\n%20s\n", "Second Receive Window");
                    printf("%20s : %d\n", "Frequency", pS2LORA->xConfig.xMAC.Rx2Channel.Frequency);
                    printf("%20s : %d\n", "Datarate", pS2LORA->xConfig.xMAC.Rx2Channel.Datarate);

                    printf("\n%20s\n", "Channels");
                    printf("%20s : %d\n", "Nb Rep", pS2LORA->xConfig.xMAC.ChannelsNbRep);
                    printf("%20s : %d\n", "Default Datarate", pS2LORA->xConfig.xMAC.ChannelsDefaultDatarate);
                    printf("%20s : ", "Mask");
                    for(int i = 0 ; i < 16 ; i++)
                    {
                        if (pS2LORA->xConfig.xMAC.ChannelsMask & ( 1 << i ))
                        {
                            printf("%2d ", i+1);
                        }
                    }
                    printf("\n");
                    printf("%20s :\n", "Bands");
                    for(int i = 0 ; i < sizeof(pS2LORA->xConfig.xMAC.Bands) / sizeof(pS2LORA->xConfig.xMAC.Bands[0]); i++)
                    {
                        printf("%20d : %2d, %2d dBm, %d, %d\n", i+1, 
                               pS2LORA->xConfig.xMAC.Bands[i].DCycle,
                               pS2LORA->xConfig.xMAC.Bands[i].TxMaxPower,
                               pS2LORA->xConfig.xMAC.Bands[i].LastTxDoneTime,
                               pS2LORA->xConfig.xMAC.Bands[i].TimeOff);
                    }

                    printf("%20s :\n", "Channels");
                    for(int i = 0 ; i < sizeof(pS2LORA->xConfig.xMAC.Channels) / sizeof(pS2LORA->xConfig.xMAC.Channels[0]); i++)
                    {
                        if (pS2LORA->xConfig.xMAC.Channels[i].Band != 0)
                        {
                            printf("%20d : %9d Hz, %2d, %2d, %d\n", i+1, 
                                   pS2LORA->xConfig.xMAC.Channels[i].Band,
                                   pS2LORA->xConfig.xMAC.Channels[i].DrRange.Fields.Min,
                                   pS2LORA->xConfig.xMAC.Channels[i].DrRange.Fields.Max,
                                   pS2LORA->xConfig.xMAC.Channels[i].Frequency );
                        }
                    }

                    printf("%20s\n", "Over the Air Activation");
                    printf("%20s : %s\n", "Status", pS2LORA->xConfig.bOverTheAirActivation?"ON":"OFF");
                    printf("%20s : %d\n", "Duty Cycle", pS2LORA->xConfig.ulOverTheAirActivationDutyCycle);
                }
            }
            break;
            
        case    3:
            {
                if (strcmp(pArgv[1], "send") == 0)
                {
                    FTE_S2LORA_send(pArgv[2], strlen(pArgv[2]));
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
                            LoRaMacTrafficMonitor( pS2LORA->pMac, true, true, 16);
                        }
                        else if (strcasecmp(pArgv[2], "off") == 0)
                        {
                            LoRaMacTrafficMonitor( pS2LORA->pMac, false, false, 16);
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
                    FTE_strToUINT32(pArgv[2], &ulCount);
                    
                    for(int i = 0; i < ulCount ; i++)
                    {
                        sprintf(pBuff, "IDX : %04d", i+1);
                        FTE_S2LORA_send(pBuff, strlen(pBuff));
                        
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
                    FTE_strToUINT32(pArgv[2], &ulSF);
                    printf("Change SF : SF%d -> ", pS2LORA->pMac->Config.SpreadingFactor);
                    printf("SF%d\n", ulSF);
                    pS2LORA->pMac->Config.SpreadingFactor = ulSF;
                }
                else if (strcasecmp(pArgv[1], "bw") == 0)
                {
                    FTE_UINT32 ulBandwidth;
                    FTE_strToUINT32(pArgv[2], &ulBandwidth);
                    printf("Change BW : BW%d -> ", pS2LORA->pMac->Config.Bandwidth);
                    printf("BW%d\n", ulBandwidth);
                    pS2LORA->pMac->Config.Bandwidth = ulBandwidth;
                }
                else if (strcasecmp(pArgv[1], "crc") == 0)
                {
                    if (strcasecmp(pArgv[2], "on") == 0)
                    {
                        pS2LORA->pMac->Config.PayloadCRC = true;
                    }
                    else if (strcasecmp(pArgv[2], "off") == 0)
                    {
                        pS2LORA->pMac->Config.PayloadCRC = false;
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
                        FTE_UINT8  pDevEUI[8];
                        
                        xRet = FTE_strToHexArray(pArgv[2], pDevEUI, 8, &ulLen);
                        
                        if ((xRet != FTE_RET_OK) || (ulLen != 8))
                        {
                            printf("Invalid parameters\n");
                            bPrintUsage = TRUE;
                        }
                        else
                        {
                            memcpy(pS2LORA->pMac->Config.DevEui, pDevEUI, 8);
                        }
                    }
                }
                else if (strcasecmp(pArgv[1], "appeui") == 0)
                {
                    if (strlen(pArgv[2]) != S2LORA_APP_EUI_LENGTH * 2)
                    {
                        printf("Invalid parameters\n");
                        bPrintUsage = TRUE;
                    }
                    else
                    {
                        FTE_UINT32  ulLen;
                        FTE_UINT8  pAppEUI[S2LORA_APP_EUI_LENGTH];
                        
                        xRet = FTE_strToHexArray(pArgv[2], pAppEUI, S2LORA_APP_EUI_LENGTH, &ulLen);
                        if ((xRet != FTE_RET_OK) || (ulLen != S2LORA_APP_EUI_LENGTH))
                        {
                            printf("Invalid parameters\n");
                            bPrintUsage = TRUE;
                        }
                        else
                        {
                            memcpy(pS2LORA->pMac->Config.AppEui, pAppEUI, S2LORA_APP_EUI_LENGTH);
                        }
                    }
                }
                else if (strcasecmp(pArgv[1], "appkey") == 0)
                {
                    if (strlen(pArgv[2]) != S2LORA_APP_KEY_LENGTH * 2)
                    {
                        printf("Invalid parameters\n");
                        bPrintUsage = TRUE;
                    }
                    else
                    {
                        FTE_UINT32  ulLen;
                        FTE_UINT8  pAppKey[S2LORA_APP_KEY_LENGTH];
                        
                        xRet = FTE_strToHexArray(pArgv[2], pAppKey, S2LORA_APP_KEY_LENGTH, &ulLen);
                       
                        if ((xRet != FTE_RET_OK) || (ulLen != S2LORA_APP_KEY_LENGTH))
                        {
                            printf("Invalid parameters\n");
                            bPrintUsage = TRUE;
                        }
                        else
                        {
                            memcpy(pS2LORA->pMac->Config.AppKey, pAppKey, S2LORA_APP_KEY_LENGTH);
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

