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

typedef FTE_UINT32 FTE_LORAWAN_ID, _PTR_ FTE_LORAWAN_ID_PTR;

typedef enum
{
    FTE_LORAWAN_STATE_DISCONNECTED,
    FTE_LORAWAN_STATE_CONNECTED,
    FTE_LORAWAN_STATE_RUN
}   FTE_LORAWAN_STATE, _PTR_ FTE_LORAWAN_STATE_PTR;

typedef struct
{
    FTE_UINT8                  bPort;
    FTE_UINT8                  bSize;
    FTE_UINT8                  pBuffer[FTE_LORAWAN_BUFFER_SIZE];
}   FTE_LORAWAN_FRAME, _PTR_ FTE_LORAWAN_FRAME_PTR;

typedef struct
{
    FTE_UINT8                  pDevEui[LORAWAN_DEV_EUI_LENGTH];        // Device IEEE EUI
    FTE_UINT8                  pAppEui[LORAWAN_APP_EUI_LENGTH];        // Application IEEE EUI
    FTE_UINT8                  pAppKey[LORAWAN_APP_KEY_LENGTH];
    LoRaMacConfig_t         xMAC;

    bool                    bOverTheAirActivation;
    FTE_UINT32                 ulOverTheAirActivationDutyCycle;

}   FTE_LORAWAN_CONFIG, _PTR_ FTE_LORAWAN_CONFIG_PTR;

typedef struct
{
    FTE_LORAWAN_STATE       xState;
    FTE_LORAWAN_CONFIG      xConfig;
    LoRaMac_t               *pMac;
    
    FTE_LORAWAN_FRAME_PTR   pSendFrame;
    FTE_UINT32                 ulBufferSize;
    FTE_UINT8                  pBuffer[FTE_LORAWAN_BUFFER_SIZE];
    
    FTE_INT32                  nRSSI;
    FTE_INT32                  nSNR;
    FTE_UINT32                 ulRxPktCount;
    FTE_UINT32                 ulTxPktCount;
    
    bool                    bIsNetworkJoined;    
    bool                    bTxNextPacket;
    bool                    bTxDone;
    
    TimerEvent_t            xJoinReqTimer;
    FTE_LIST                xTxPktList;
} FTE_LORAWAN, _PTR_ FTE_LORAWAN_PTR;

_mqx_uint   FTE_LORAWAN_init( void *pConfig);
_mqx_uint   FTE_LORAWAN_send(void *pBuff, FTE_UINT32 ulLen);
FTE_UINT32     FTE_LORAWAN_recv(void *pBuff, FTE_UINT32 ulBuffSize);

_mqx_uint   FTE_LORAWAN_requestJoin(FTE_LORAWAN_PTR pLoRaWan);

FTE_INT32  FTE_LORAWAN_SHELL_cmd(FTE_INT32 argc, char_ptr argv[]);

#endif