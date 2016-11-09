#ifndef __FTE_S2LORA_H__
#define __FTE_S2LORA_H__   1

#include "target.h"
#include "board.h"
#include "radio.h"
#include "fte_lorawan_config.h"
#include "fte_list.h"
#include "LoRaMac.h"

#define FTE_S2LORA_BUFFER_SIZE            256

#define S2LORA_DEV_EUI_LENGTH   8
#define S2LORA_APP_EUI_LENGTH   16
#define S2LORA_APP_KEY_LENGTH   16

typedef uint_32 FTE_S2LORA_ID, _PTR_ FTE_S2LORA_ID_PTR;

typedef enum
{
    FTE_S2LORA_STATE_DISCONNECTED,
    FTE_S2LORA_STATE_CONNECTED,
    FTE_S2LORA_STATE_RUN
}   FTE_S2LORA_STATE, _PTR_ FTE_S2LORA_STATE_PTR;

typedef struct
{
    uint_8                  bPort;
    uint_8                  bSize;
    uint_8                  pBuffer[FTE_S2LORA_BUFFER_SIZE];
}   FTE_S2LORA_FRAME, _PTR_ FTE_S2LORA_FRAME_PTR;

typedef struct
{
    uint_8                  pDevEui[S2LORA_DEV_EUI_LENGTH];        // Device IEEE EUI
    uint_8                  pAppEui[S2LORA_APP_EUI_LENGTH];        // Application IEEE EUI
    uint_8                  pAppKey[S2LORA_APP_KEY_LENGTH];
    LoRaMacConfig_t         xMAC;

    bool                    bOverTheAirActivation;
    uint_32                 ulOverTheAirActivationDutyCycle;

}   FTE_S2LORA_CONFIG, _PTR_ FTE_S2LORA_CONFIG_PTR;

typedef struct
{
    FTE_S2LORA_STATE       xState;
    FTE_S2LORA_CONFIG      xConfig;
    LoRaMac_t               *pMac;
    
    FTE_S2LORA_FRAME_PTR   pSendFrame;
    uint_32                 ulBufferSize;
    uint_8                  pBuffer[FTE_S2LORA_BUFFER_SIZE];
    
    int_32                  nRSSI;
    int_32                  nSNR;
    uint_32                 ulRxPktCount;
    uint_32                 ulTxPktCount;
    
    bool                    bIsNetworkJoined;    
    bool                    bTxNextPacket;
    bool                    bTxDone;
    
    TimerEvent_t            xJoinReqTimer;
    FTE_LIST                xTxPktList;
} FTE_S2LORA, _PTR_ FTE_S2LORA_PTR;

_mqx_uint   FTE_S2LORA_init( void *pConfig);
_mqx_uint   FTE_S2LORA_send(void *pBuff, uint_32 ulLen);
uint_32     FTE_S2LORA_recv(void *pBuff, uint_32 ulBuffSize);

_mqx_uint   FTE_S2LORA_requestJoin(FTE_S2LORA_PTR pLoRaWan);

FTE_INT32   FTE_S2LORA_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);

#endif