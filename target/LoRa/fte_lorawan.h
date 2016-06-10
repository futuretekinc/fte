#ifndef __FTE_LORAWAN_H__
#define __FTE_LORAWAN_H__   1

#include "target.h"
#include "board.h"
#include "radio.h"
#include "fte_lorawan_config.h"
#include "fte_list.h"
#include "LoRaMac.h"

#define LORAWAN_DEV_EUI_LENGTH   8
#define LORAWAN_APP_EUI_LENGTH   16
#define LORAWAN_APP_KEY_LENGTH   16

typedef uint_32 FTE_LORAWAN_ID, _PTR_ FTE_LORAWAN_ID_PTR;

typedef enum
{
    FTE_LORAWAN_STATE_DISCONNECTED,
    FTE_LORAWAN_STATE_CONNECTED,
    FTE_LORAWAN_STATE_RUN
}   FTE_LORAWAN_STATE, _PTR_ FTE_LORAWAN_STATE_PTR;

typedef struct
{
    uint_8                  bPort;
    uint_8                  bSize;
    uint_8                  pBuffer[FTE_LORAWAN_BUFFER_SIZE];
}   FTE_LORAWAN_FRAME, _PTR_ FTE_LORAWAN_FRAME_PTR;

typedef struct
{
    uint_8                  pDevEui[LORAWAN_DEV_EUI_LENGTH];        // Device IEEE EUI
    uint_8                  pAppEui[LORAWAN_APP_EUI_LENGTH];        // Application IEEE EUI
    uint_8                  pAppKey[LORAWAN_APP_KEY_LENGTH];
    LoRaMacConfig_t         xMAC;

    bool                    bOverTheAirActivation;
    uint_32                 ulOverTheAirActivationDutyCycle;

}   FTE_LORAWAN_CONFIG, _PTR_ FTE_LORAWAN_CONFIG_PTR;

typedef struct
{
    FTE_LORAWAN_STATE       xState;
    FTE_LORAWAN_CONFIG      xConfig;
    LoRaMac_t               *pMac;
    
    FTE_LORAWAN_FRAME_PTR   pSendFrame;
    uint_32                 ulBufferSize;
    uint_8                  pBuffer[FTE_LORAWAN_BUFFER_SIZE];
    
    int_32                  nRSSI;
    int_32                  nSNR;
    uint_32                 ulRxPktCount;
    uint_32                 ulTxPktCount;
    
    bool                    bIsNetworkJoined;    
    bool                    bTxNextPacket;
    bool                    bTxDone;
    
    TimerEvent_t            xJoinReqTimer;
    FTE_LIST                xTxPktList;
} FTE_LORAWAN, _PTR_ FTE_LORAWAN_PTR;

_mqx_uint   FTE_LORAWAN_init( void *pConfig);
_mqx_uint   FTE_LORAWAN_send(void *pBuff, uint_32 ulLen);
uint_32     FTE_LORAWAN_recv(void *pBuff, uint_32 ulBuffSize);

_mqx_uint   FTE_LORAWAN_requestJoin(FTE_LORAWAN_PTR pLoRaWan);

int_32  FTE_LORAWAN_SHELL_cmd(int_32 argc, char_ptr argv[]);

#endif