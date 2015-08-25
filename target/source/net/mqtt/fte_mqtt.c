#include <ctype.h>
#include "fte_target.h"
#include "fte_config.h"
#include "fte_mqtt.h"
#include "sys/fte_sys.h"
#include "fte_time.h"
#include <rtcs.h>
#include <sh_rtcs.h>
#include "nxjson.h"
#include "fte_json.h"
#include "fte_ssl.h"

#if FTE_MQTT_SUPPORTED
#if 0
#define FTE_MQTT_TRACE(...)    TRACE(DEBUG_NET_MQTT, __VA_ARGS__);
#else
#define FTE_MQTT_TRACE(...)    
#endif
#define FTE_MQTT_MSG_DEV_VALUE_STRING   "MSG_DEV_VALUE"
#define FTE_MQTT_MSG_DEV_INFO_STRING    "MSG_DEV_INFO"
#define FTE_MQTT_MSG_EP_VALUE_STRING    "MSG_EP_VALUE"
#define FTE_MQTT_MSG_EP_INFO_STRING     "MSG_EP_INFO"


typedef struct
{
    char_ptr    pTopic;
    char_ptr    pMsg;
    uint_8      nQoS;
    uint_8      pData[];
}   FTE_MQTT_SEND_MSG, _PTR_ FTE_MQTT_SEND_MSG_PTR;

typedef struct
{
    uint_8      nCmd;
    uint_8      nTarget;
    uint_32     ulDataLen;
    uint_8      pData[];
}   FTE_MQTT_MSG, _PTR_ FTE_MQTT_MSG_PTR;

typedef struct
{
    FTE_MQTT_STATE  xState;
    uint_32         (*callback)(FTE_MQTT_CONTEXT_PTR);
}   FTE_MQTT_STATE_CALLBACK, _PTR_ FTE_MQTT_STATE_CALLBACK_PTR;

typedef struct
{
    uint_32     ulMsgID;
    uint_32     (*callback)(FTE_MQTT_CONTEXT_PTR);
}   FTE_MQTT_MSG_CALLBACK, _PTR_ FTE_MQTT_MSG_CALLBACK_PTR;

static uint_32 FTE_MQTT_subscribe(FTE_MQTT_CONTEXT_PTR pCTX, char *pTopic);
static uint_32 FTE_MQTT_publish(FTE_MQTT_CONTEXT_PTR pCTX, uint_32 nQoS, char *pTopic, char *pMsg);

uint_32 FTE_MQTT_publishDevice(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_MSG_TYPE xMsgType, uint_32 nQoS);
uint_32 FTE_MQTT_publishDeviceInfo(FTE_MQTT_CONTEXT_PTR pCTX, uint_32 nQoS);
uint_32 FTE_MQTT_publishDeviceValue(FTE_MQTT_CONTEXT_PTR pCTX, uint_32 nQoS);

static uint_32 FTE_MQTT_publishEP(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_MSG_TYPE xMsgType, FTE_OBJECT_ID xEPID, uint_32 nQoS);


static void     FTE_MQTT_process(pointer pTraps, pointer pCreator);
static void     FTE_MQTT_receiver(pointer pParams, pointer pCreator);
static void     FTE_MQTT_sender(pointer pParams, pointer pCreator);

static uint_32  FTE_MQTT_initSocket(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_connect(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_disconnect(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_recvPacket(FTE_MQTT_CONTEXT_PTR pCTX, uint_32  ulTimeout);
static int      FTE_MQTT_closeSocket(FTE_MQTT_CONTEXT_PTR pCTX);

static int      FTE_MQTT_sendPacket(void* pSocketInfo, void const * pBuf, unsigned int ulCount);

static _mqx_uint FTE_MQTT_TRANS_create(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_TRANS_PTR _PTR_ ppTrans);
static _mqx_uint FTE_MQTT_TRANS_destroy(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_TRANS_PTR pTrans);
static _mqx_uint FTE_MQTT_TRANS_push(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_TRANS_PTR pTrans);
static _mqx_uint FTE_MQTT_TRANS_get(FTE_MQTT_CONTEXT_PTR pCTX, uint_16 nMsgID, FTE_MQTT_TRANS_PTR _PTR_ ppTrans);
        _mqx_uint FTE_MQTT_TRANS_checkTimeout(FTE_MQTT_CONTEXT_PTR pCTX, uint_32 ulTimeout);

static uint_32  FTE_MQTT_STATE_CB_uninitialized(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_STATE_CB_initialized(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_STATE_CB_connected(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_STATE_CB_disconnected(FTE_MQTT_CONTEXT_PTR pCTX);

static uint_32  FTE_MQTT_MSG_CB_undefined(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_connectionACK(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_publish(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_publishACK(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_publishReceived(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_publishCompleted(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_subscribe(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_subscribeACK(FTE_MQTT_CONTEXT_PTR pCTX);
static uint_32  FTE_MQTT_MSG_CB_PINGResponse(FTE_MQTT_CONTEXT_PTR pCTX);

static uint_32  FTE_MQTT_MSG_create(uint_8 nCmd, uint_8 nTarget, uint_8_ptr pData, uint_32 ulDataLen, FTE_MQTT_MSG_PTR _PTR_ ppMsg);
static uint_32  FTE_MQTT_MSG_processing(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_MSG_PTR pMsg);
static uint_32  FTE_MQTT_MSG_destroy(FTE_MQTT_MSG_PTR pMsg);

static _mqx_uint    FTE_MQTT_INTERNAL_publish(FTE_MQTT_CONTEXT_PTR pCTX, char *pSubTopic, uint_32 nQoS, char *pMsg);

static uint_32  FTE_MQTT_PING_send(FTE_MQTT_CONTEXT_PTR pCTX);
static void     FTE_MQTT_PING_timeout(_timer_id, pointer, MQX_TICK_STRUCT_PTR);

static char_ptr             FTE_MQTT_MSG_TYPE_string(FTE_MQTT_MSG_TYPE xMsgType);

static FTE_MQTT_METHOD_PTR  FTE_MQTT_METHOD_get(char_ptr pString);

const FTE_MQTT_STATE_CALLBACK _pStateCBs[] =
{
    {   
        .xState     = FTE_MQTT_STATE_UNINITIALIZED,
        .callback   = FTE_MQTT_STATE_CB_uninitialized
    },
    {   
        .xState     = FTE_MQTT_STATE_INITIALIZED,
        .callback   = FTE_MQTT_STATE_CB_initialized
    },
    {   
        .xState     = FTE_MQTT_STATE_CONNECTED,
        .callback   = FTE_MQTT_STATE_CB_connected
    },
    {   
        .xState     = FTE_MQTT_STATE_DISCONNECTED,
        .callback   = FTE_MQTT_STATE_CB_disconnected
    },
    
};
const uint_32   _ulStateCBCount = sizeof(_pStateCBs) / sizeof(FTE_MQTT_STATE_CALLBACK);

const FTE_MQTT_MSG_CALLBACK   _pMsgCBs[] = 
{
    {   .ulMsgID = 0,                   .callback = FTE_MQTT_MSG_CB_undefined       },
    {   .ulMsgID = MQTT_MSG_CONNECT,    .callback = FTE_MQTT_MSG_CB_undefined       },
    {   .ulMsgID = MQTT_MSG_CONNACK,    .callback = FTE_MQTT_MSG_CB_connectionACK   },
    {   .ulMsgID = MQTT_MSG_PUBLISH,    .callback = FTE_MQTT_MSG_CB_publish         },
    {   .ulMsgID = MQTT_MSG_PUBACK,     .callback = FTE_MQTT_MSG_CB_publishACK      },
    {   .ulMsgID = MQTT_MSG_PUBREC,     .callback = FTE_MQTT_MSG_CB_publishReceived },
    {   .ulMsgID = MQTT_MSG_PUBREL,     .callback = FTE_MQTT_MSG_CB_undefined       },
    {   .ulMsgID = MQTT_MSG_PUBCOMP,    .callback = FTE_MQTT_MSG_CB_publishCompleted},
    {   .ulMsgID = MQTT_MSG_SUBSCRIBE,  .callback = FTE_MQTT_MSG_CB_subscribe       },
    {   .ulMsgID = MQTT_MSG_SUBACK,     .callback = FTE_MQTT_MSG_CB_subscribeACK    },
    {   .ulMsgID = MQTT_MSG_UNSUBSCRIBE,.callback = FTE_MQTT_MSG_CB_undefined       },
    {   .ulMsgID = MQTT_MSG_UNSUBACK,   .callback = FTE_MQTT_MSG_CB_undefined       },
    {   .ulMsgID = MQTT_MSG_PINGREQ,    .callback = FTE_MQTT_MSG_CB_undefined       },
    {   .ulMsgID = MQTT_MSG_PINGRESP,   .callback = FTE_MQTT_MSG_CB_PINGResponse    },
    {   .ulMsgID = MQTT_MSG_DISCONNECT, .callback = FTE_MQTT_MSG_CB_undefined       }
};

static FTE_MQTT_METHOD _pThingPlusMethods[];


const uint_32   _ulMsgCBCount = sizeof(_pMsgCBs) / sizeof(FTE_MQTT_MSG_CALLBACK);

static FTE_MQTT_CONTEXT_PTR _pxCTX = NULL;

uint_32 FTE_MQTT_load_default(FTE_MQTT_CFG_PTR pConfig)
{
    _enet_address   xMAC;
    
    FTE_NET_getMACAddress(xMAC);

    snprintf(pConfig->pClientID, FTE_MQTT_CLIENT_ID_LENGTH, "%s/%s", FTE_MODEL, FTE_SYS_getOIDString());
    
    pConfig->xBroker.xIPAddress     = 0;
    pConfig->xBroker.usPort         = FTE_MQTT_DEFAULT_PORT;
    pConfig->xBroker.ulKeepalive    = FTE_MQTT_DEFAULT_KEEPALIVE;
    pConfig->xBroker.xAuth.bEnabled = FALSE;

    return  MQX_OK;
}

uint_32 FTE_MQTT_init(FTE_MQTT_CFG_PTR pConfig)
{
    ASSERT(pConfig != NULL);
    
    TRACE_ON(DEBUG_NET_MQTT);
    
    if (pConfig->bEnable)
    {        
        int i;
        FTE_MQTT_TRANS_PTR  pTrans; 
    
        if (_pxCTX != NULL)
        {
            return  FTE_MQTT_RET_ERROR;
        }
        
        _pxCTX = (FTE_MQTT_CONTEXT_PTR)FTE_MEM_allocZero(sizeof(FTE_MQTT_CONTEXT));
        if (_pxCTX == NULL)
        {
            return  FTE_MQTT_RET_NOT_ENOUGH_MEMORY;
        }
        
        if (strlen(pConfig->pClientID) == 0)
        {
            snprintf(pConfig->pClientID, FTE_MQTT_CLIENT_ID_LENGTH, "%s", FTE_SYS_getOIDString());
        }
        
        FTE_LIST_init(&_pxCTX->xTransList);
        FTE_LIST_init(&_pxCTX->xFreeTransList);
        FTE_LIST_init(&_pxCTX->xRecvMsgPool);
        FTE_LIST_init(&_pxCTX->xSendMsgPool);
        
            
        _pxCTX->nSocketID = 0;
        _pxCTX->xState    = FTE_MQTT_STATE_UNINITIALIZED;
        _pxCTX->pConfig   = pConfig;
        _pxCTX->ulBuffSize= FTE_MQTT_RECV_BUFF_SIZE;
        _pxCTX->xStatistics.ulTransTimeout = 0;
    
        mqtt_init(&_pxCTX->xBroker, pConfig->pClientID);
        if (pConfig->xBroker.xAuth.bEnabled)
        {
            mqtt_init_auth(&_pxCTX->xBroker, pConfig->xBroker.xAuth.pUserName, pConfig->xBroker.xAuth.pPassword);
        }

        for(i = 0 ; i < FTE_MQTT_TRANS_COUNT ; i++)
        {
            pTrans = FTE_MEM_allocZero(sizeof(FTE_MQTT_TRANS));
            if (pTrans != NULL)
            {
                FTE_LIST_pushBack(&_pxCTX->xFreeTransList, pTrans);
            }
        }
            
        RTCS_task_create(FTE_NET_MQTT_NAME, FTE_NET_MQTT_PRIO, FTE_NET_MQTT_STACK, FTE_MQTT_process, _pxCTX);    
        RTCS_task_create(FTE_NET_MQTT_RECEIVER_NAME, FTE_NET_MQTT_PRIO, FTE_NET_MQTT_RECEIVER_STACK, FTE_MQTT_receiver, _pxCTX);    
        RTCS_task_create(FTE_NET_MQTT_SENDER_NAME, FTE_NET_MQTT_PRIO, FTE_NET_MQTT_SENDER_STACK, FTE_MQTT_sender, _pxCTX);    
    }
    
    return  FTE_MQTT_RET_OK;
}    

void FTE_MQTT_process(pointer pParams, pointer pCreator)
{
    FTE_MQTT_CONTEXT_PTR    pCTX = (FTE_MQTT_CONTEXT_PTR)pParams;
    FTE_MQTT_STATE          xOldState = FTE_MQTT_STATE_UNINITIALIZED;

    RTCS_task_resume_creator(pCreator, RTCS_OK);
    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id());

    xOldState = pCTX->xState;
    while(1)
    {
        
        if (pCTX->xState < _ulStateCBCount)
        {   
            _pStateCBs[pCTX->xState].callback(pCTX);
            if (pCTX->xState != xOldState)
            {
                FTE_MQTT_TRACE("STATE CHANGED : %d -> %d\n", xOldState, pCTX->xState);
                xOldState = pCTX->xState;
            }
        }
        
        _time_delay(1000);

    }
}

void FTE_MQTT_receiver(pointer pParams, pointer pCreator)
{
    FTE_MQTT_CONTEXT_PTR    pCTX = (FTE_MQTT_CONTEXT_PTR)pParams;

    RTCS_task_resume_creator(pCreator, RTCS_OK);
    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id());

    while(1)
    {
        if ((FTE_SYS_STATE_get() & FTE_STATE_CONNECTED) == FTE_STATE_CONNECTED)
        {        
            if (FTE_LIST_count(&pCTX->xRecvMsgPool) != 0)
            {
                FTE_MQTT_MSG_PTR    pMsg;
                
                if (FTE_LIST_popFront(&pCTX->xRecvMsgPool, (pointer _PTR_)&pMsg) == MQX_OK)
                {
                    FTE_MQTT_MSG_processing(pCTX, pMsg);
                    FTE_MQTT_MSG_destroy(pMsg);
                }
            }

        }
        _time_delay(1);
    }
}

void FTE_MQTT_sender(pointer pParams, pointer pCreator)
{
    FTE_MQTT_CONTEXT_PTR    pCTX = (FTE_MQTT_CONTEXT_PTR)pParams;

    RTCS_task_resume_creator(pCreator, RTCS_OK);
    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id());

    while(1)
    {
        if ((FTE_SYS_STATE_get() & FTE_STATE_CONNECTED) == FTE_STATE_CONNECTED)
        {        
            if (FTE_LIST_count(&pCTX->xSendMsgPool) != 0)
            {
                FTE_MQTT_SEND_MSG_PTR pMsg;
                    
                if (FTE_LIST_popFront(&pCTX->xSendMsgPool, (pointer _PTR_)&pMsg) == MQX_OK)
                {
                    FTE_MQTT_INTERNAL_publish(pCTX, pMsg->pTopic, pMsg->nQoS, pMsg->pMsg);
                    FTE_MEM_free(pMsg);
                }
            }
        }
        _time_delay(100);
    }
}

uint_32 FTE_MQTT_connect(FTE_MQTT_CONTEXT_PTR pCTX)
{
    uint_32 ulRet;
	struct sockaddr_in socket_address;
    int     nFlag = 1;

    ASSERT(pCTX != NULL);
    
    if (pCTX->xState != FTE_MQTT_STATE_INITIALIZED)
    {
        return  FTE_MQTT_RET_OPERATION_NOT_PERMITTED;
    }

    if (pCTX->nSocketID != 0)
    {
        FTE_MQTT_closeSocket(pCTX);
    }
    
	// Create the socket
    pCTX->nSocketID = socket(PF_INET, SOCK_STREAM, 0);
	if(pCTX->nSocketID == RTCS_SOCKET_ERROR)
    {
		return FTE_MQTT_RET_ERROR;
    }

	// Disable Nagle Algorithm
    ulRet = setsockopt(pCTX->nSocketID, SOL_TCP, OPT_RECEIVE_NOWAIT, (char*)&nFlag, sizeof(nFlag));
	if ( ulRet != RTCS_OK)
    {
        goto error;
    }
    
    // Create the stuff we need to connect
    socket_address.sin_family       = AF_INET;
    socket_address.sin_addr.s_addr  = pCTX->pConfig->xBroker.xIPAddress;
    socket_address.sin_port         = pCTX->pConfig->xBroker.usPort;

    // Connect the socket
    _time_delay(100);
    ulRet = connect(pCTX->nSocketID, (struct sockaddr*)&socket_address, sizeof(socket_address));
    if((ulRet != RTCS_OK) && (ulRet != RTCSERR_TCP_CONN_RLSD))
    {
        goto error;
    }
    
    if (pCTX->pConfig->xSSL.bEnabled)
    {
        pCTX->pxSSL = FTE_SSL_create(&pCTX->pConfig->xSSL);
        if (pCTX->pxSSL == NULL)
        {
            goto error;
        }
        
        ulRet = FTE_SSL_connect(pCTX->pxSSL, pCTX->nSocketID);
        if (ulRet != RTCS_OK)
        {
            goto error;
        }
    }    
    
    // >>>>> CONNECT
    if (mqtt_connect(&pCTX->xBroker) <= 0)
    {    
        goto error;
    }
    
    pCTX->ulPingTimeout = 0;
    pCTX->xPingTimer = 0;
    pCTX->xState = FTE_MQTT_STATE_CONNECTED;
    
    return  FTE_MQTT_RET_OK;
    
error:
    if (pCTX->nSocketID != 0)
    {
        shutdown(pCTX->nSocketID, FLAG_ABORT_CONNECTION);
        pCTX->nSocketID = 0;
    }
    
    if (pCTX->pxSSL != NULL)
    {
        FTE_SSL_destroy(pCTX->pxSSL);
        pCTX->pxSSL = NULL;
    }
    
    return  FTE_MQTT_RET_ERROR;
    
}


_mqx_uint   FTE_MQTT_subscribe(FTE_MQTT_CONTEXT_PTR pCTX, char *pTopic)
{    
    _mqx_uint           ulRet;
    int                 nRet;
    uint_16             nMsgID;
    FTE_MQTT_TRANS_PTR  pTrans;
    
    ASSERT(pCTX != NULL);

    ulRet = FTE_MQTT_TRANS_create(pCTX, &pTrans);
    if (ulRet != MQX_OK)
    {
        return  ulRet;
    }    
    
    nRet = mqtt_subscribe(&pCTX->xBroker, pTopic, &nMsgID);
    if (nRet == 0)
    {
        return  FTE_MQTT_RET_TCP_CONN_ABORTED;
    }
    else if (nRet < 0)
    {
        return  FTE_MQTT_RET_USBSCRIBE_ERROR;
    }

    pTrans->nMsgID  = nMsgID;
    pTrans->nMsgType= MQTT_MSG_SUBSCRIBE;
    pTrans->nQoS    = FTE_MQTT_QOS_1;
    
    FTE_MQTT_TRANS_push(pCTX, pTrans);    
    
    return  FTE_MQTT_RET_OK;
}


uint_32 FTE_MQTT_disconnect(FTE_MQTT_CONTEXT_PTR pCTX)
{
    if (pCTX->xPingTimer != 0)
    {
        _timer_cancel(pCTX->xPingTimer);
        pCTX->xPingTimer = 0;
        pCTX->ulPingTimeout = 0;
    }
    
    mqtt_disconnect(&pCTX->xBroker);
    
    
    if (pCTX->nSocketID != 0)
    {
        shutdown(pCTX->nSocketID, FLAG_ABORT_CONNECTION);
        pCTX->nSocketID = 0;
    }
    
    if (pCTX->pxSSL != NULL)
    {
        FTE_SSL_destroy(pCTX->pxSSL);
        pCTX->pxSSL = NULL;
    }    
    
    pCTX->xState = FTE_MQTT_STATE_DISCONNECTED;
    
    return  FTE_MQTT_RET_OK;
}

int FTE_MQTT_sendPacket(void* pSocketInfo, void const * pBuf, unsigned int ulCount)
{
    FTE_MQTT_CONTEXT_PTR    pCTX = (FTE_MQTT_CONTEXT_PTR)pSocketInfo;
    
    ASSERT((pCTX != NULL) && (pBuf != NULL));
    
    if (pCTX->pConfig->xSSL.bEnabled)
    {
        return  FTE_SSL_send(pCTX->pxSSL, pBuf, ulCount);
    }
    else
    {
        return send(pCTX->nSocketID, (void *)pBuf, ulCount, 0);
    }
}

uint_32 FTE_MQTT_publishDevice(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_MSG_TYPE xMsgType, uint_32 nQoS)
{
    char            pTopic[FTE_MQTT_TOPIC_LENGTH+1];
    char_ptr        pMsg = NULL;
    uint_32         ulMsgLen = 0;
    uint_32         i;
    uint_32         ulCount = 0;
    FTE_JSON_OBJECT_PTR pJSONObject = NULL;
    FTE_JSON_VALUE_PTR  pValue = NULL;
    FTE_JSON_ARRAY_PTR  pEPS = NULL;
    
    ulCount = FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE);
        
    pJSONObject = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(4);
    if (pJSONObject == NULL)
    {
        goto error;
    }
    pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createString(FTE_MQTT_MSG_TYPE_string(xMsgType));
    if (pValue == NULL)
    {
        goto error;
    }
    FTE_JSON_OBJECT_setPair(pJSONObject, FTE_JSON_MSG_TYPE_STRING, pValue);

    pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createString(FTE_SYS_getOIDString());
    if (pValue == NULL)
    {
        goto error;
    }
    FTE_JSON_OBJECT_setPair(pJSONObject, FTE_JSON_OBJ_ID_STRING, pValue);

    pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createNumber(FTE_SYS_getTime());
    if (pValue == NULL)
    {
        goto error;
    }
    FTE_JSON_OBJECT_setPair(pJSONObject, FTE_JSON_OBJ_TIME_STRING, pValue);
    pEPS = (FTE_JSON_ARRAY_PTR)FTE_JSON_VALUE_createArray(ulCount);
    if (pEPS == NULL)
    {
        goto error;
    }
        
    for(i = 0 ; i < ulCount ; i++)
    {
        FTE_OBJECT_PTR pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_UNKNOWN, 0, i, FALSE);
        if (pObj != NULL)
        {
            if (xMsgType == FTE_MQTT_MSG_DEV_VALUE)
            {
                pValue = (FTE_JSON_VALUE_PTR)FTE_OBJ_createJSON(pObj, FTE_OBJ_FIELD_DEV_VALUE);
            }
            else
            {
                pValue = (FTE_JSON_VALUE_PTR)FTE_OBJ_createJSON(pObj, FTE_OBJ_FIELD_DEV_INFO);
            }
            if (pValue!= NULL)
            {
                FTE_JSON_ARRAY_setElement(pEPS, (FTE_JSON_VALUE_PTR)pValue);
            }            
        }
    }   

    FTE_JSON_OBJECT_setPair(pJSONObject, FTE_JSON_DEV_EPS_STRING, (FTE_JSON_VALUE_PTR)pEPS);

    ulMsgLen = FTE_JSON_VALUE_buffSize((FTE_JSON_VALUE_PTR)pJSONObject) + 1;
    
    pMsg = (char_ptr)FTE_MEM_allocZero(ulMsgLen);
    if (pMsg == NULL)
    {
        goto error;
    }
    
    FTE_JSON_VALUE_snprint(pMsg, ulMsgLen, (FTE_JSON_VALUE_PTR)pJSONObject);
    FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pJSONObject);
    
    sprintf(pTopic, "/v/a/g/%s/status", FTE_SYS_getOIDString());                            
    
    FTE_MQTT_publish(pCTX, nQoS, pTopic, pMsg);

    FTE_MEM_free(pMsg);
    
    return  FTE_MQTT_RET_OK;
    
error:
    if (pJSONObject != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pJSONObject);
    }
    
    return  FTE_MQTT_RET_ERROR;
}

uint_32 FTE_MQTT_publishEPInfo(FTE_OBJECT_ID xEPID, uint_32 nQoS)
{
    if (_pxCTX == NULL)
    {
        return  FTE_MQTT_RET_NOT_INITIALIZED;
    }
    
    return  FTE_MQTT_publishEP(_pxCTX, FTE_MQTT_MSG_EP_INFO, xEPID, nQoS);
}

uint_32 FTE_MQTT_publishEPValue(FTE_OBJECT_ID xEPID, uint_32 nQoS)
{
    if (_pxCTX == NULL)
    {
        return  FTE_MQTT_RET_NOT_INITIALIZED;
    }
    
    //return  FTE_MQTT_TP_publishEPValue(xEPID, nQoS);    
    return  FTE_MQTT_publishEP(_pxCTX, FTE_MQTT_MSG_EP_VALUE, xEPID, nQoS);
}

uint_32 FTE_MQTT_publishEP(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_MSG_TYPE xMsgType, FTE_OBJECT_ID xEPID, uint_32 nQoS)
{
    char                pTopic[FTE_MQTT_TOPIC_LENGTH];
    char_ptr            pMsg = NULL;
    uint_32             ulMsgLen;
    FTE_OBJECT_PTR      pObj = NULL;
    FTE_JSON_OBJECT_PTR pJSONObject = NULL;
    FTE_JSON_VALUE_PTR  pValue = NULL;

    pObj = FTE_OBJ_get(xEPID);
    if(pObj == NULL)
    {
        goto error;
    }

    pJSONObject = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(4);
    if (pJSONObject == NULL)
    {
        goto error;
    }
    pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createString("deviceChanged");
    if (pValue == NULL)
    {
        goto error;
    }
    FTE_JSON_OBJECT_setPair(pJSONObject, "method", pValue);

    pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createNumber(FTE_SYS_getTime());
    if (pValue == NULL)
    {
        goto error;
    }
    FTE_JSON_OBJECT_setPair(pJSONObject, FTE_JSON_OBJ_TIME_STRING, pValue);

    if (FTE_MQTT_MSG_EP_VALUE)
    {
        FTE_JSON_VALUE_PTR pObjValue = (FTE_JSON_VALUE_PTR)FTE_OBJ_createJSON(pObj, FTE_OBJ_FIELD_ID | FTE_OBJ_FIELD_VALUE);
        if (pObjValue == NULL)
        {
            goto error;
        }
        
        pValue = FTE_JSON_VALUE_createArray(1);
        if (pValue == NULL)
        {
            FTE_JSON_VALUE_destroy(pObjValue);
            goto error;
        }
        
        FTE_JSON_ARRAY_setElement((FTE_JSON_ARRAY_PTR)pValue, pObjValue);
    }
    else
    {
        pValue = (FTE_JSON_VALUE_PTR)FTE_OBJ_createJSON(pObj, FTE_OBJ_FIELD_EP_INFO);
        if (pValue == NULL)
        {
            goto error;
        }
    }
    FTE_JSON_OBJECT_setPair(pJSONObject, "params", pValue);
    
    ulMsgLen = FTE_JSON_VALUE_buffSize((FTE_JSON_VALUE_PTR)pJSONObject) + 1;
    pMsg = (char_ptr)FTE_MEM_alloc(ulMsgLen);
    if (pMsg == NULL)
    {
        goto error;
    }
    
    FTE_JSON_VALUE_snprint(pMsg, ulMsgLen, (FTE_JSON_VALUE_PTR)pJSONObject);
    FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pJSONObject);

    sprintf(pTopic, "/v/a/g/%s/s/%08x", FTE_SYS_getOIDString(), xEPID);
    
    FTE_MQTT_publish(pCTX, nQoS, pTopic, pMsg);

    FTE_MEM_free(pMsg);
    
    return  FTE_MQTT_RET_OK;
error:
    
    if (pJSONObject != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pJSONObject);
    }
    
    return  FTE_MQTT_RET_ERROR;
}

uint_32 FTE_MQTT_TP_publishEPValue(FTE_OBJECT_ID xEPID, uint_32 nQoS)
{
    char                pTopic[FTE_MQTT_TOPIC_LENGTH];
    char_ptr            pMsg = NULL;
    uint_32             ulMsgLen;
    FTE_OBJECT_PTR      pObj = NULL;
    FTE_JSON_OBJECT_PTR pJSONObject = NULL;
    FTE_JSON_VALUE_PTR  pValue = NULL;

    pObj = FTE_OBJ_get(xEPID);
    if(pObj == NULL)
    {
        goto error;
    }

    pJSONObject = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createArray(2);
    if (pJSONObject == NULL)
    {
        goto error;
    }

    pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createNumber(FTE_SYS_getTime());
    if (pValue == NULL)
    {
        goto error;
    }
    FTE_JSON_ARRAY_setElement((FTE_JSON_ARRAY_PTR)pJSONObject, pValue);

    pValue = (FTE_JSON_VALUE_PTR)FTE_OBJ_createJSON(pObj, FTE_OBJ_FIELD_EP_VALUE);
    if (pValue == NULL)
    {
        goto error;
    }
        
    FTE_JSON_ARRAY_setElement((FTE_JSON_ARRAY_PTR)pJSONObject, pValue);
    
    ulMsgLen = FTE_JSON_VALUE_buffSize((FTE_JSON_VALUE_PTR)pJSONObject) + 1;
    pMsg = (char_ptr)FTE_MEM_alloc(ulMsgLen);
    if (pMsg == NULL)
    {
        goto error;
    }
    
    FTE_JSON_VALUE_snprint(pMsg, ulMsgLen, (FTE_JSON_VALUE_PTR)pJSONObject);
    FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pJSONObject);

    sprintf(pTopic, "/v/a/g/%s/s/%08x", FTE_SYS_getOIDString(), xEPID);
    
    FTE_MQTT_publish(_pxCTX, nQoS, pTopic, pMsg);

    FTE_MEM_free(pMsg);
    
    return  FTE_MQTT_RET_OK;
error:
    
    if (pJSONObject != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pJSONObject);
    }
    
    return  FTE_MQTT_RET_ERROR;
}

uint_32     FTE_MQTT_publish(FTE_MQTT_CONTEXT_PTR pCTX, uint_32 nQoS, char *pTopic, char *pData)
{
    FTE_MQTT_SEND_MSG_PTR   pMsg;
    
    ASSERT (pCTX != NULL);
    
    if (pCTX->xState != FTE_MQTT_STATE_CONNECTED)
    {
        return  FTE_MQTT_RET_NOT_CONNECTED;
    }
    
    pMsg = (FTE_MQTT_SEND_MSG_PTR)FTE_MEM_allocZero(sizeof(FTE_MQTT_SEND_MSG) + strlen(pTopic) + strlen(pData) + 2);
    if (pMsg != NULL)
    {
        
        pMsg->nQoS  = nQoS;
        pMsg->pTopic= (char_ptr)pMsg->pData;
        strcpy(pMsg->pTopic, pTopic);
        
        pMsg->pMsg  = (char_ptr)&pMsg->pData[strlen(pTopic) + 1];                
        strcpy(pMsg->pMsg, pData);
        
        FTE_LIST_pushBack(&pCTX->xSendMsgPool, pMsg);
    }
    return  FTE_MQTT_RET_OK;
}

_mqx_uint     FTE_MQTT_INTERNAL_publish(FTE_MQTT_CONTEXT_PTR pCTX, char *pTopic, uint_32 nQoS, char *pMsg)
{    
    int_32      nRet;
    uint_16     nMsgID;
    
    ASSERT(pCTX != NULL);    
    
    switch(nQoS)
    {
    case    FTE_MQTT_QOS_0:
        {
            nRet = mqtt_publish(&pCTX->xBroker, pTopic, pMsg, 0);
            if (nRet == 0)
            {
                FTE_MQTT_disconnect(pCTX);
                return  FTE_MQTT_RET_TCP_CONN_ABORTED;
            }
        }
        break;
        
    case    FTE_MQTT_QOS_1:
    case    FTE_MQTT_QOS_2:
       {
           FTE_MQTT_TRANS_PTR   pTrans;
           
            if (MQX_OK != FTE_MQTT_TRANS_create(pCTX, &pTrans))
            {
                return  FTE_MQTT_RET_NOT_ENOUGH_MEMORY;
            }    
            
            nRet = mqtt_publish_with_qos(&pCTX->xBroker, pTopic, pMsg, 0, nQoS, &nMsgID);
            
            if (nRet == 0)
            {
                FTE_MQTT_TRANS_destroy(pCTX, pTrans);
                FTE_MQTT_disconnect(pCTX);
                return  FTE_MQTT_RET_TCP_CONN_ABORTED;
            }
            else if (nRet < 0)
            {
                FTE_MQTT_TRANS_destroy(pCTX, pTrans);
                return  FTE_MQTT_RET_ERROR;
            }
            
            pTrans->nMsgID  = nMsgID;
            pTrans->nMsgType= MQTT_MSG_PUBLISH;
            pTrans->nQoS    = nQoS;

            FTE_MQTT_TRANS_push(pCTX, pTrans);
        }
        break;
    }
    
    return  FTE_MQTT_RET_OK;
}

uint_32  FTE_MQTT_initSocket(FTE_MQTT_CONTEXT_PTR pCTX)
{
	// MQTT stuffs
    mqtt_set_alive(&pCTX->xBroker, pCTX->pConfig->xBroker.ulKeepalive);

    pCTX->xBroker.socket_info = (void*)pCTX;
    pCTX->xBroker.send = FTE_MQTT_sendPacket;

    pCTX->xState = FTE_MQTT_STATE_INITIALIZED;
  
	return FTE_MQTT_RET_OK;
}


int FTE_MQTT_closeSocket(FTE_MQTT_CONTEXT_PTR pCTX)
{
    if ((pCTX->xState != FTE_MQTT_STATE_INITIALIZED) && (pCTX->xState != FTE_MQTT_STATE_DISCONNECTED))
    {
        FTE_MQTT_disconnect(pCTX);
    }
    
    pCTX->xState = FTE_MQTT_STATE_UNINITIALIZED;

    return FTE_MQTT_RET_OK;
}

uint_32 FTE_MQTT_recvPacket(FTE_MQTT_CONTEXT_PTR pCTX, uint_32  ulTimeout)
{
	int nRcvdBytes, nPacketLen;
    
	if(ulTimeout > 0)
	{
        if (RTCS_selectset(&pCTX->nSocketID, 1, ulTimeout) == 0)
        {
            return  FTE_MQTT_RET_READ_ABOARTED;
        }
	}

	memset(pCTX->pBuff, 0, pCTX->ulBuffSize);
    pCTX->ulRcvdLen = 0;
    
    if (pCTX->pConfig->xSSL.bEnabled)
    {
        if((nRcvdBytes = FTE_SSL_recv(pCTX->pxSSL, pCTX->pBuff, pCTX->ulBuffSize)) < 2) 
        {
            return FTE_MQTT_RET_READ_ABOARTED;
        }
    }
    else
    {
        if((nRcvdBytes = recv(pCTX->nSocketID, pCTX->pBuff, pCTX->ulBuffSize, 0)) < 2) 
        {
            return FTE_MQTT_RET_READ_ABOARTED;
        }
    }

    pCTX->ulRcvdLen = nRcvdBytes; // Keep tally of total bytes
	
	// now we have the full fixed header in packet_buffer
	// parse it for remaining length and number of bytes
	//nPacketLen = packet_buffer[1] + 2; // Remaining length + fixed header length
	// total packet length = remaining length + byte 1 of fixed header + remaning length part of fixed header
	nPacketLen = mqtt_parse_rem_len(pCTX->pBuff) + mqtt_num_rem_len_bytes(pCTX->pBuff) + 1;

	while(pCTX->ulRcvdLen < nPacketLen) // Reading the packet
	{
        if (pCTX->pConfig->xSSL.bEnabled)
        {
            if((nRcvdBytes = FTE_SSL_recv(NULL, &pCTX->pBuff[pCTX->ulRcvdLen], pCTX->ulBuffSize - pCTX->ulRcvdLen)) <= 0)
            {
                return FTE_MQTT_RET_READ_ABOARTED;
            }
        }
        else
        {
            if((nRcvdBytes = recv(pCTX->nSocketID, &pCTX->pBuff[pCTX->ulRcvdLen], pCTX->ulBuffSize - pCTX->ulRcvdLen, 0)) <= 0)
            {
                return FTE_MQTT_RET_READ_ABOARTED;
            }
        }

		pCTX->ulRcvdLen += nRcvdBytes; // Keep tally of total bytes
	}

	return FTE_MQTT_RET_OK;
}

uint_32 FTE_MQTT_PING_send(FTE_MQTT_CONTEXT_PTR pCTX)
{   
    MQX_TICK_STRUCT     xTicks;            
        
    pCTX->ulPingTimeout++;
    
    _time_get_ticks(&xTicks);
    _time_add_msec_to_ticks(&xTicks, pCTX->ulPingTimeout * 1000);
        
    pCTX->xPingTimer = _timer_start_oneshot_at_ticks(FTE_MQTT_PING_timeout, pCTX, TIMER_KERNEL_TIME_MODE, &xTicks);    

    FTE_MQTT_TRACE("PING SEND : TIMEOUT = %d ms\n", pCTX->ulPingTimeout * 1000);
    
    mqtt_ping(&pCTX->xBroker);
    
    return  FTE_MQTT_RET_OK;        
}

static void   FTE_MQTT_PING_timeout(_timer_id xTimerID, pointer pData, MQX_TICK_STRUCT_PTR pTick)
{
    FTE_MQTT_CONTEXT_PTR pCTX = (FTE_MQTT_CONTEXT_PTR)pData;
    
    if (pCTX->ulPingTimeout < 3)
    {
        FTE_MQTT_PING_send(pCTX);
    }
    else
    {
        pCTX->xPingTimer = 0;
        FTE_MQTT_disconnect(pCTX);
    }
}

/******************************************************************************
 * MQTT State functions
 ******************************************************************************/
static uint_32  FTE_MQTT_STATE_CB_uninitialized(FTE_MQTT_CONTEXT_PTR pCTX)
{
    FTE_MQTT_TRACE("MQTT STATE : Uninitialized\n");
    FTE_MQTT_initSocket(pCTX);
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_STATE_CB_initialized(FTE_MQTT_CONTEXT_PTR pCTX)
{
    FTE_MQTT_TRACE("MQTT STATE : Initialized\n");

    if ((FTE_SYS_STATE_get() & FTE_STATE_CONNECTED) == FTE_STATE_CONNECTED)
    {
        FTE_MQTT_connect(pCTX);
    }
                
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_STATE_CB_connected(FTE_MQTT_CONTEXT_PTR pCTX)
{
    char    pTopic[FTE_MQTT_TOPIC_LENGTH+1];
    char    pMessage[256];
    
    FTE_MQTT_TRACE("MQTT STATE : Connected\n");
                                   
    sprintf(pTopic, "/v/a/g/%s/req", FTE_SYS_getOIDString());
    FTE_MQTT_subscribe(pCTX, pTopic);
    
    sprintf(pTopic, "/v/a/g/%s/info", FTE_SYS_getOIDString());
    sprintf(pMessage, "{\"method\":\"connected\", \"params\":{\"id\":\"%s\"}}", FTE_SYS_getOIDString());
    FTE_MQTT_publish(pCTX, 1, pTopic, pMessage);
    
    while(pCTX->xState == FTE_MQTT_STATE_CONNECTED)
    {
        uint_32 ulRet;
            
        ulRet = FTE_MQTT_recvPacket(pCTX, 1000);
        if (ulRet == RTCS_OK)
        {
            uint_32 nMsgType;
            
            nMsgType = MQTTParseMessageType(pCTX->pBuff) >> 4;   
            if (nMsgType < _ulMsgCBCount)
            {
                _pMsgCBs[nMsgType].callback(pCTX);                                
            }
        }
        
        if ((FTE_SYS_STATE_get() & FTE_STATE_CONNECTED) != FTE_STATE_CONNECTED)
        {
            FTE_MQTT_disconnect(pCTX);
            break;
        }
        
        if (pCTX->ulPingTimeout == 0)
        {
            TIME_STRUCT     xCurrentTime;
            uint_32         ulTimeDiff;

            _time_get(&xCurrentTime);                    
            ulTimeDiff = FTE_TIME_diff(&xCurrentTime, &pCTX->xTime);            
            
             if (pCTX->pConfig->xBroker.ulKeepalive - 5 < ulTimeDiff)
             {
                 FTE_MQTT_PING_send(pCTX);
                _time_get(&pCTX->xTime);                
             }
        }
    }
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_STATE_CB_disconnected(FTE_MQTT_CONTEXT_PTR pCTX)
{
    FTE_MQTT_TRACE("MQTT STATE : Disconnected\n");
    
    FTE_MQTT_closeSocket(pCTX);
    
    return  FTE_MQTT_RET_OK;
}

/******************************************************************************
 * MQTT Command Message functions
 ******************************************************************************/
static uint_32  FTE_MQTT_MSG_CB_undefined(FTE_MQTT_CONTEXT_PTR pCTX)
{
    return  FTE_MQTT_RET_ERROR;
}
                     
static uint_32  FTE_MQTT_MSG_CB_connectionACK(FTE_MQTT_CONTEXT_PTR pCTX)
{
    // <<<<< CONNACK
    
    switch(pCTX->pBuff[3])
    {
    case    0:  break;
    case    1:  return  FTE_MQTT_RET_UNACCEPTABLE_PROTOCOL_VERSION;
    case    2:  return  FTE_MQTT_RET_IDENTIFIER_REJECTED;
    case    3:  return  FTE_MQTT_RET_SERVER_UNAVALIABLE;
    case    4:  return  FTE_MQTT_RET_BAD_USER_NAME_OR_PASSWORD;
    case    5:  return  FTE_MQTT_RET_NOT_AUTHORIZED;
    default:    return  RTCS_ERROR;
    }

    pCTX->xState = FTE_MQTT_STATE_CONNECTED;
    
    FTE_MQTT_TRACE("MQTT Connected!\n");                        
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_MSG_CB_publish(FTE_MQTT_CONTEXT_PTR pCTX)
{
    FTE_MQTT_MSG_PTR    pMsg;
    uint_8              nCmd;
    uint_8              pTopic[FTE_MQTT_TOPIC_LENGTH + 1];
    uint16_t            nTopicLen;
    uint_8 const _PTR_  pData;
    char_ptr            pFields[10];
    uint16_t            nFieldCount;
    uint16_t            nDataLen;    
    
    nTopicLen = mqtt_parse_pub_topic(pCTX->pBuff, pTopic);
    pTopic[nTopicLen] = '\0';

    nDataLen = mqtt_parse_pub_msg_ptr(pCTX->pBuff, &pData);

    nFieldCount = 0;
    pFields[nFieldCount] = strtok((char_ptr)pTopic, "/");
    while((pFields[nFieldCount] != NULL) && (nFieldCount < 10))
    {
        pFields[++nFieldCount] = strtok(NULL, "/");
    }

    if ((nFieldCount < 5) ||
        (strcmp(pFields[0], "v") != 0) ||
        (strcmp(pFields[1], "a") != 0) ||
        (strcmp(pFields[2], "g") != 0) ||
        (strcmp(pFields[3], FTE_SYS_getOIDString()) != 0))
    {
        return  FTE_MQTT_RET_INVALID_TOPIC;
    }

    if (strcmp(pFields[4], "req") == 0)
    {
        nCmd = FTE_MQTT_CMD_REQ;
    }
    else
    {
        return  FTE_MQTT_RET_INVALID_TOPIC;
    }

    if (FTE_MQTT_MSG_create(nCmd, FTE_MQTT_TARGET_UID, (uint_8_ptr)pData, nDataLen, &pMsg) != RTCS_OK)
    {
        return  FTE_MQTT_RET_ERROR;
    }

    FTE_LIST_pushBack(&pCTX->xRecvMsgPool, pMsg);
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_MSG_CB_publishACK(FTE_MQTT_CONTEXT_PTR pCTX)
{
    uint_32             ulRet;
    FTE_MQTT_TRANS_PTR  pTrans;
    uint_32             ulMsgID;
   
   ulMsgID = mqtt_parse_msg_id(pCTX->pBuff);
    
    FTE_MQTT_TRACE("%16s : %3d\n", "PUB ACK", ulMsgID);
    ulRet = FTE_MQTT_TRANS_get(pCTX, ulMsgID, &pTrans);
    if (ulRet != FTE_MQTT_RET_OK)
    {
        DEBUG("%d message id was expected, but it is unknown id !\n", ulMsgID);                    
        return  ulRet;
    }
    
    FTE_MQTT_TRANS_destroy(pCTX, pTrans);
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_MSG_CB_publishReceived(FTE_MQTT_CONTEXT_PTR pCTX)
{
    uint_32             ulRet;
    FTE_MQTT_TRANS_PTR  pTrans;
    uint_32             ulMsgID;
    
    ulMsgID = mqtt_parse_msg_id(pCTX->pBuff);
   
    FTE_MQTT_TRACE("%16s : %3d\n", "PUB RECEIVED", ulMsgID);
    ulRet = FTE_MQTT_TRANS_get(pCTX, ulMsgID, &pTrans);
    if (ulRet != FTE_MQTT_RET_OK)
    {
        DEBUG("%d message id was expected, but it is unknown id !\n", ulMsgID);                    
        return  ulRet;
    }
    
    mqtt_pubrel(&pCTX->xBroker, ulMsgID);
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_MSG_CB_publishCompleted(FTE_MQTT_CONTEXT_PTR pCTX)
{
    uint_32             ulRet;
    FTE_MQTT_TRANS_PTR  pTrans;
    uint_32             ulMsgID;
    
    ulMsgID = mqtt_parse_msg_id(pCTX->pBuff);

    FTE_MQTT_TRACE("%16s : %3d\n", "PUB COMPLETED", ulMsgID);
    
    ulRet = FTE_MQTT_TRANS_get(pCTX, ulMsgID, &pTrans);
    if (ulRet != FTE_MQTT_RET_OK)
    {
        DEBUG("%d message id was expected, but it is unknown id !\n", ulMsgID);                    
        return  ulRet;
    }

    FTE_MQTT_TRANS_destroy(pCTX, pTrans);                
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_MSG_CB_subscribe(FTE_MQTT_CONTEXT_PTR pCTX)
{

    FTE_MQTT_MSG_CB_subscribeACK(pCTX);
        
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_MSG_CB_subscribeACK(FTE_MQTT_CONTEXT_PTR pCTX)
{
    uint_32             ulRet;
    FTE_MQTT_TRANS_PTR  pTrans;
    uint_32             ulMsgID;
    
    ulMsgID = mqtt_parse_msg_id(pCTX->pBuff);

    ulRet = FTE_MQTT_TRANS_get(pCTX, ulMsgID, &pTrans);
    if (ulRet != FTE_MQTT_RET_OK)
    {
        DEBUG("%d message id was expected, but it is unknown id !\n", ulMsgID);                    
        return  ulRet;
    }
    
    FTE_MQTT_TRANS_destroy(pCTX, pTrans);
    
    pCTX->xState = FTE_MQTT_STATE_CONNECTED;
    
    return  FTE_MQTT_RET_OK;
}

static uint_32  FTE_MQTT_MSG_CB_PINGResponse(FTE_MQTT_CONTEXT_PTR pCTX)
{
    _timer_cancel(pCTX->xPingTimer);
    
    pCTX->xPingTimer = 0;   
    pCTX->ulPingTimeout = 0;   
    
    return  FTE_MQTT_RET_OK;
}


/******************************************************************************
 * Internal functions
 ******************************************************************************/
_mqx_uint   FTE_MQTT_TRANS_create(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_TRANS_PTR _PTR_ ppTrans)
{
    _mqx_uint           ulRet;
    FTE_MQTT_TRANS_PTR  pTrans;
        
    FTE_MQTT_TRACE("%s : Free Trans Count : %d\n", __func__, FTE_LIST_count(&pCTX->xFreeTransList));
    if (FTE_LIST_count(&pCTX->xFreeTransList) != 0)
    {
        ulRet = FTE_LIST_popFront(&pCTX->xFreeTransList, (pointer _PTR_)&pTrans);
        if (ulRet != MQX_OK)
        {
            return  FTE_MQTT_RET_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        pTrans = (FTE_MQTT_TRANS_PTR)FTE_MEM_allocZero(sizeof(FTE_MQTT_TRANS));
        if (pTrans == NULL)
        {
            return  FTE_MQTT_RET_NOT_ENOUGH_MEMORY;
        }
    }
    
    _time_get(&pTrans->xTime);

    *ppTrans = pTrans;
    
    return  FTE_MQTT_RET_OK;
}

_mqx_uint   FTE_MQTT_TRANS_destroy(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_TRANS_PTR pTrans)
{
    FTE_LIST_remove(&pCTX->xTransList, pTrans);
    FTE_LIST_pushBack(&pCTX->xFreeTransList, pTrans);
    FTE_MQTT_TRACE("%s : Free Trans Count : %d\n", __func__, FTE_LIST_count(&pCTX->xFreeTransList));
    
    return  FTE_MQTT_RET_OK;
}

_mqx_uint   FTE_MQTT_TRANS_push(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_TRANS_PTR pTrans)
{
    FTE_LIST_pushBack(&pCTX->xTransList, pTrans);
    
    return  FTE_MQTT_RET_OK;
}

uint_32 FTE_MQTT_TRANS_get(FTE_MQTT_CONTEXT_PTR pCTX, uint_16 nMsgID, FTE_MQTT_TRANS_PTR _PTR_ ppTrans)
{
    FTE_MQTT_TRANS_PTR  pTrans;    
    FTE_LIST_ITERATOR   xIter;
 
    FTE_LIST_ITER_init(&pCTX->xTransList, &xIter);
    while((pTrans = FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        if (pTrans->nMsgID == nMsgID)
        {
            *ppTrans = pTrans;
            return  FTE_MQTT_RET_OK;
        }
    }

    return  FTE_MQTT_RET_ERROR;
}

#if 0
_mqx_uint   FTE_MQTT_TRANS_checkTimeout(FTE_MQTT_CONTEXT_PTR pCTX, uint_32 ulTimeout)
{
    FTE_MQTT_TRANS_PTR  pTrans;    
    FTE_LIST_ITERATOR   xIter;
    TIME_STRUCT         xTime;
    FTE_LIST            xRemoveList;
    
    FTE_LIST_init(&xRemoveList);
    
    _time_get(&xTime);
    
    FTE_LIST_ITER_init(&pCTX->xTransList, &xIter);
    while((pTrans = FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        if (FTE_TIME_diff(&xTime, &pTrans->xTime) > ulTimeout)
        {
            FTE_LIST_pushBack(&xRemoveList, pTrans);
        }
    }
    
    FTE_LIST_ITER_init(&xRemoveList, &xIter);
    while((pTrans = FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        FTE_MQTT_TRACE("TRANS TIMEMOUT : %d\n", pTrans->nMsgID);
        FTE_MQTT_TRANS_destroy(pCTX, pTrans);
        pCTX->xStatistics.ulTransTimeout++;
    }

    FTE_LIST_final(&xRemoveList);
    
    return  FTE_MQTT_RET_OK;
}
#endif

uint_32 FTE_MQTT_MSG_create
(
    uint_8      nCmd, 
    uint_8      nTarget, 
    uint_8_ptr  pData, 
    uint_32     ulDataLen, 
    FTE_MQTT_MSG_PTR _PTR_ ppMsg
)
{
    FTE_MQTT_MSG_PTR    pMsg;
    
    pMsg = (FTE_MQTT_MSG_PTR)FTE_MEM_allocZero(sizeof(FTE_MQTT_MSG) + ulDataLen + 1);
    if (pMsg == NULL)
    {
        return  FTE_MQTT_RET_ERROR;
    }
    
    pMsg->nCmd = nCmd;
    pMsg->nTarget = nTarget;
    memcpy(pMsg->pData, pData, ulDataLen);
    pMsg->ulDataLen  = ulDataLen;
    
    *ppMsg = pMsg;
    
    return  FTE_MQTT_RET_OK;
}

uint_32 FTE_MQTT_MSG_processing(FTE_MQTT_CONTEXT_PTR pCTX, FTE_MQTT_MSG_PTR pMsg)
{
    const nx_json* pRoot = NULL;
    
    if (pMsg->nCmd == FTE_MQTT_CMD_REQ)
    {
        TRACE(DEBUG_NET_MQTT, "%s\n", pMsg->pData);
        pRoot = nx_json_parse_utf8((char_ptr)pMsg->pData);            
        if (pRoot == NULL)
        {
            goto error;
        }

        const nx_json* pMethodItem = nx_json_get(pRoot, FTE_JSON_OBJ_METHOD_STRING);
        if (pMethodItem == NULL)
        {
            goto error;
        }
        
        FTE_MQTT_METHOD_PTR pMethod = FTE_MQTT_METHOD_get((char_ptr)pMethodItem->text_value);
        if (pMethod != NULL)
        {
            const nx_json* pParamItem = nx_json_get(pRoot, FTE_JSON_OBJ_PARAMS_STRING);
            if (pParamItem == NULL)
            {
                goto error;
            }
            
            pMethod->fCallback((void _PTR_)pParamItem);
        }
        
        nx_json_free(pRoot);            
    }
    
    return  FTE_MQTT_RET_OK;
    
error:
    
    if (pRoot != NULL)
    {
        nx_json_free(pRoot);            
    }
    
    return  FTE_MQTT_RET_ERROR;
}

uint_32 FTE_MQTT_MSG_destroy(FTE_MQTT_MSG_PTR pMsg)
{
    FTE_MEM_free(pMsg);
    
    return  FTE_MQTT_RET_OK;
}

struct
{
    FTE_MQTT_MSG_TYPE   xType;
    char_ptr            pString;
}   _xMsgTypeString[] =
{
    {   FTE_MQTT_MSG_DEV_INFO,  "MSG_DEV_INFO"  },
    {   FTE_MQTT_MSG_DEV_VALUE, "MSG_DEV_VALUE" },
    {   FTE_MQTT_MSG_EP_INFO,   "MSG_EP_INFO"   },
    {   FTE_MQTT_MSG_EP_VALUE,  "MSG_EP_VALUE"  },
    {   FTE_MQTT_MSG_GET_VALUE, "sensor.get" },
    {   FTE_MQTT_MSG_SET_VALUE, "sensor.set" },
    {   FTE_MQTT_MSG_INVALID,   NULL}
};

/******************************************************************************
 * Thing+ REQ message callback
 ******************************************************************************/
_mqx_uint            FTE_MQTT_METHOD_CB_setProperty(void _PTR_ pParams)
{
    printf("%s called\n", __func__);
    return  MQX_OK;
}

_mqx_uint            FTE_MQTT_METHOD_CB_controlActuator(void _PTR_ pParams)
{
    return  MQX_OK;
}

_mqx_uint            FTE_MQTT_METHOD_CB_timeSync(void _PTR_ pParams)
{
    return  MQX_OK;
}

FTE_MQTT_METHOD _pThingPlusMethods[] =
{
    {   FTE_MQTT_METHOD_SET_PROPERTY,       "setProperty",      FTE_MQTT_METHOD_CB_setProperty},
    {   FTE_MQTT_METHOD_CONTROL_ACTUATOR,   "controlActuator",  FTE_MQTT_METHOD_CB_controlActuator},
    {   FTE_MQTT_METHOD_TIME_SYNC,          "timeSync",         FTE_MQTT_METHOD_CB_timeSync},
    {   FTE_MQTT_METHOD_INVALID,            NULL,               NULL}
};


/******************************************************************************
 * FTLM REQ message callback
 ******************************************************************************/
_mqx_uint            FTE_MQTT_METHOD_FTLM_CB_deviceGet(void _PTR_ pParams)
{
    uint_32         i;
    FTE_OBJECT_PTR  pObj;
    FTE_VALUE_PTR   pValue;
    uint_32         pulValues[9];
    uint_8          pMAC[6];
    char            pTopic[32];
    char            pBuff[512];
    uint_32         ulBuffLen = 0;
    
    pObj = FTE_OBJ_get(0x7f090001);    
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }

    pValue = FTE_VALUE_createULONG(); 
    if (pValue == NULL)
    {
        return  MQX_ERROR;
    }
    
    for(i = 0 ; i < 9 ; i++)
    {
        FTE_OBJ_getValueAt(pObj, i, pValue);
        FTE_VALUE_getULONG(pValue, &pulValues[i]);
    }
    
    FTE_VALUE_destroy(pValue);
    
    ulBuffLen += sprintf(&pBuff[ulBuffLen], "{\"method\":\"deviceGet\",\"params\":[");
    for(i = 0 ; i < 9 ; i++)
    {
        if (i != 0)
        {
            ulBuffLen += sprintf(&pBuff[ulBuffLen], ",");
        }
        ulBuffLen += sprintf(&pBuff[ulBuffLen], "{\"id\":%d,\"value\":%d}",
                             i+1, pulValues[i]);
    }
    ulBuffLen += sprintf(&pBuff[ulBuffLen], "]}");
    FTE_SYS_getMAC( pMAC);

    sprintf(pTopic, "/v/a/g/%02x%02x%02x%02x%02x%02x/resp", pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
    FTE_MQTT_publish(_pxCTX, FTE_MQTT_QOS_1, pTopic, pBuff);
    
    return  MQX_OK;
}

_mqx_uint            FTE_MQTT_METHOD_FTLM_CB_deviceSet(void _PTR_ pParams)
{
    const nx_json   *pItem;
    uint_32         i;
    
    for(i = 0 ; (pItem = nx_json_item((const nx_json* )pParams, i)) != NULL ; i++)
    {
        if (pItem->type == NX_JSON_NULL)
        {
            break;
        }
        
        const nx_json *pID = nx_json_get(pItem, "id");
        const nx_json *pCmd= nx_json_get(pItem, "cmd");
        const nx_json *pLevel = nx_json_get(pItem, "level");
        const nx_json *pTime = nx_json_get(pItem, "time");
        
        if ((pID->type == NX_JSON_NULL) || (pCmd->type == NX_JSON_NULL) || (pLevel->type == NX_JSON_NULL) || (pTime->type == NX_JSON_NULL))
        {
            return  MQX_ERROR;
        }
        
        if ((pID->int_value < 1) || (9 < pID->int_value))
        {
            return  MQX_ERROR;
        }
        
        if ((pCmd->int_value < 0) || (255 < pCmd->int_value))
        {
            return  MQX_ERROR;
        }
        
        if ((pLevel->int_value < 0) || (255 < pLevel->int_value))
        {
            return  MQX_ERROR;
        }
        
        if ((pTime->int_value < 0) || (255 < pTime->int_value))
        {
            return  MQX_ERROR;
        }
        
        FTE_OBJECT_PTR pObj = FTE_OBJ_get(0x0a810900 + pID->int_value);
        if (pObj != NULL)
        {
            uint_32 ulValue = (uint_32)((uint_8)pCmd->int_value & 0xFF) | ((uint_32)((uint_8)pLevel->int_value & 0xFF) << 8) | ((uint_32)((uint_8)pTime->int_value & 0xFF) << 16);
            FTE_VALUE   xValue;
           
            FTE_VALUE_initULONG(&xValue, ulValue);
            FTE_OBJ_setValue(pObj, &xValue);            
        }
    }

    return  MQX_OK;
}

_mqx_uint            FTE_MQTT_METHOD_FTLM_CB_groupGet(void _PTR_ pParams)
{
    printf("%s called\n", __func__);
    return  MQX_OK;
}

_mqx_uint            FTE_MQTT_METHOD_FTLM_CB_groupSet(void _PTR_ pParams)
{
    printf("%s called\n", __func__);
    return  MQX_OK;
}

FTE_MQTT_METHOD _pFTLMMethods[] =
{
    {   FTE_MQTT_METHOD_FTLM_DEVICE_GET, "deviceGet",    FTE_MQTT_METHOD_FTLM_CB_deviceGet},
    {   FTE_MQTT_METHOD_FTLM_DEVICE_SET, "deviceSet",    FTE_MQTT_METHOD_FTLM_CB_deviceSet},
    {   FTE_MQTT_METHOD_FTLM_GROUP_GET,  "groupGet",     FTE_MQTT_METHOD_FTLM_CB_groupGet},
    {   FTE_MQTT_METHOD_FTLM_GROUP_SET,  "groupSet",     FTE_MQTT_METHOD_FTLM_CB_groupSet},
    {   FTE_MQTT_METHOD_INVALID,    NULL,           NULL}
};

FTE_MQTT_METHOD_PTR   FTE_MQTT_METHOD_get(char_ptr pString)
{
    FTE_MQTT_METHOD_PTR pMethod = _pFTLMMethods;
    
    while(pMethod->xMethod != FTE_MQTT_METHOD_INVALID)
    {
        if (strcmp(pMethod->pString, pString) == 0)
        {
            return  pMethod;
        }
        
        pMethod++;
    }
    
    return  NULL;
}

char_ptr   FTE_MQTT_MSG_TYPE_string(FTE_MQTT_MSG_TYPE xMsgType)
{
    uint_32 i;
    
    for(i = 0 ; _xMsgTypeString[i].xType != FTE_MQTT_MSG_INVALID ; i++)
    {
        if (_xMsgTypeString[i].xType == xMsgType)
        {
            return  _xMsgTypeString[i].pString;
        }
    }
    
    return  NULL;
}



/******************************************************************************
 * Shell command
 ******************************************************************************/
int_32  FTE_MQTT_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
                FTE_NET_CFG_PTR     pConfig;
                static char    *pStateString[] =
                {
                    "UNINITIALIZED", "INITIALIZED", "CONNECTED", "DISCONNECTED"
                };
                
                pConfig = FTE_CFG_NET_get();
                printf("%16s : %s\n",           "CLIENT ID",pConfig->xMQTT.pClientID);
                printf("%16s : %s\n",           "STATUS",   pConfig->xMQTT.bEnable?"run":"stop");
                printf("%16s : %d.%d.%d.%d\n",  "HOST",     IPBYTES(pConfig->xMQTT.xBroker.xIPAddress));
                printf("%16s : %d\n",           "PORT",     pConfig->xMQTT.xBroker.usPort);
                printf("%16s : %u\n",           "KEEPALIVE",pConfig->xMQTT.xBroker.ulKeepalive);
                printf("%16s : %s\n",           "AUTHORIZE",pConfig->xMQTT.xBroker.xAuth.bEnabled?"YES":"NO");
                if (pConfig->xMQTT.xBroker.xAuth.bEnabled)
                {
                    printf("%16s : %s\n", "USER NAME", pConfig->xMQTT.xBroker.xAuth.pUserName);
                    printf("%16s : %s\n", "PASSWORD",  pConfig->xMQTT.xBroker.xAuth.pPassword);
                }

                if (pConfig->xMQTT.xSSL.bEnabled)
                {
                    printf("%16s : ENABLED\n","SSL");
                }
                else
                {
                    printf("%16s : DISABLED\n",     "SSL");
                }
                
                if (_pxCTX != NULL)
                {
                    printf("%16s : %s\n",           "STATE",    pStateString[_pxCTX->xState]);
                    printf("%16s : %d\n",           "TRANS COUNT",      FTE_LIST_count(&_pxCTX->xTransList));
                    printf("%16s : %d\n",           "TRANS FREE COUNT", FTE_LIST_count(&_pxCTX->xFreeTransList));
                    printf("%16s : %d\n",           "TRANS TIMEOUT",    _pxCTX->xStatistics.ulTransTimeout);
                }
            }
            break;
            
        case    2:
            {
                FTE_NET_CFG_PTR     pConfig = FTE_CFG_NET_get();
                
                if (strcasecmp(argv[1], "start") == 0)
                {
                    if (pConfig->xMQTT.bEnable == FALSE)
                    {
                        pConfig->xMQTT.bEnable = TRUE;
                        FTE_CFG_save(TRUE);
                    }
                }
                else if (strcasecmp(argv[1], "stop") == 0)
                {
                    if (pConfig->xMQTT.bEnable == TRUE)
                    {
                         pConfig->xMQTT.bEnable = FALSE;
                        FTE_CFG_save(TRUE);
                    }
                }
                else
                {
                    print_usage = TRUE;
                }
            }
            break;

        case    3:
            {
                FTE_NET_CFG_PTR     pConfig = FTE_CFG_NET_get();
                
                if (strcmp(argv[1], "sub") == 0)
                {
                    if (_pxCTX == NULL)
                    {
                        printf("mqtt not initialized\n");
                        break;
                    }
                    
                    FTE_MQTT_subscribe(_pxCTX, argv[2]);
                }
                else if (strcmp(argv[1], "host") == 0)
                {
                    _ip_address xIPAddress;

                     if (! Shell_parse_ip_address (argv[2], &xIPAddress))
                    {
                        printf ("Error! invalid ip address!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    
                    pConfig->xMQTT.xBroker.xIPAddress = xIPAddress;
                    FTE_CFG_save(TRUE);
                }
                else if (strcmp(argv[1], "port") == 0)
                {
                    uint_16 usPort;

                     if (! Shell_parse_uint_16(argv[2], &usPort))
                    {
                        printf ("Error! invalid port number!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    
                    pConfig->xMQTT.xBroker.usPort = usPort;
                    FTE_CFG_save(TRUE);
                }
                else if (strcmp(argv[1], "keepalive") == 0)
                {
                    uint_32 ulKeepalive;

                     if (! Shell_parse_uint_32(argv[2], &ulKeepalive))
                    {
                        printf ("Error! invalid keepalive!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    
                    pConfig->xMQTT.xBroker.ulKeepalive = ulKeepalive;
                    FTE_CFG_save(TRUE);
                }
                else if (strcmp(argv[1], "ssl") == 0)
                {
                     if (! strcmp(argv[2], "enable"))
                     {
                         pConfig->xMQTT.xSSL.bEnabled = TRUE;
                     }
                     else if (!strcmp(argv[2], "disable"))
                     {
                         pConfig->xMQTT.xSSL.bEnabled = FALSE;
                     }
                     else
                    {
                        printf ("Error! invalid value!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    FTE_CFG_save(TRUE);
                }
                else
                {
                    print_usage = TRUE;
                }
                
            }
            break;
            
        case    4:
            {
                if (strcmp(argv[1], "pub") == 0)
                {
                    if (_pxCTX == NULL)
                    {
                        printf("mqtt not initialized\n");
                        break;
                    }
                    
                    FTE_MQTT_publish(_pxCTX, FTE_MQTT_QOS_1, argv[2], argv[3]);
                }
                else
                {
                    print_usage = TRUE;
                }
            }
            break;
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
            printf("    info\n");
            printf("        Show mqtt information.\n");
            printf("    start\n");
            printf("        start mqtt.\n");
            printf("    stop\n");
            printf("        stop mqtt.\n");
            printf("    pub <topic> <message>\n");
            printf("        pulish message.\n");
            printf("    host <ip>\n");
            printf("        mqtt host to connect to.\n");
            printf("    port <port>\n");
            printf("        Set broker port.\n");
            printf("    keepalive <sec>\n");
            printf("        Set keepalive time.\n");
            printf("    ssl [enable|disable]\n");
            printf("        with SSL\n");
            printf("  Parameters:\n");
            printf("    <ip>      = IPv4 address to use.\n");
            printf("    <topic>   = mqtt topic to publish to.\n");
            printf("    <message> = message payload to send.\n");
        }
    }
    return   return_code;
}


#endif