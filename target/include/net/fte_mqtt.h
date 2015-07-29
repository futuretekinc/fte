#ifndef __FTE_MQTT_H__
#define __FTE_MQTT_H__

#include "fte_target.h"
#include <rtcs.h>
#include <libemqtt.h>
#include "fte_list.h"
#include "fte_ssl.h"

#ifndef FTE_MQTT_DEFAULT_PORT   
#define FTE_MQTT_DEFAULT_PORT       1883
#endif

#ifndef FTE_MQTT_DEFAULT_KEEPALIVE
#define FTE_MQTT_DEFAULT_KEEPALIVE  180
#endif

#define FTE_MQTT_CLIENT_ID_LENGTH   64
#define FTE_MQTT_USERNAME_LENGTH    MQTT_CONF_USERNAME_LENGTH
#define FTE_MQTT_PASSWORD_LENGTH    MQTT_CONF_PASSWORD_LENGTH
#define FTE_MQTT_TOPIC_LENGTH       128
#define FTE_MQTT_IP_LENGTH          128
#define FTE_MQTT_RECV_BUFF_SIZE     1024
#define FTE_MQTT_TRANS_COUNT        10

#define FTE_MQTT_RET_OK                             (MQX_OK)
#define FTE_MQTT_RET_ERROR                          (RTCS_ERROR_BASE|0xF00)
#define FTE_MQTT_RET_NOT_CONNECTED                  (RTCS_ERROR_BASE|0xF01)
#define FTE_MQTT_RET_READ_ABOARTED                  (RTCS_ERROR_BASE|0xF02)
#define FTE_MQTT_RET_UNACCEPTABLE_PROTOCOL_VERSION  (RTCS_ERROR_BASE|0xF03)
#define FTE_MQTT_RET_IDENTIFIER_REJECTED            (RTCS_ERROR_BASE|0xF04)
#define FTE_MQTT_RET_SERVER_UNAVALIABLE             (RTCS_ERROR_BASE|0xF05)
#define FTE_MQTT_RET_BAD_USER_NAME_OR_PASSWORD      (RTCS_ERROR_BASE|0xF06)
#define FTE_MQTT_RET_NOT_AUTHORIZED                 (RTCS_ERROR_BASE|0xF07)
#define FTE_MQTT_RET_OPERATION_NOT_PERMITTED        (RTCS_ERROR_BASE|0xF08)
#define FTE_MQTT_RET_USBSCRIBE_ERROR                (RTCS_ERROR_BASE|0xF09)
#define FTE_MQTT_RET_INVALID_TOPIC                  (RTCS_ERROR_BASE|0xF0A)
#define FTE_MQTT_RET_INVALID_PARAMS                 (RTCS_ERROR_BASE|0xF0B)
#define FTE_MQTT_RET_NOT_ENOUGH_MEMORY              (RTCS_ERROR_BASE|0xF0C)
#define FTE_MQTT_RET_TCP_CONN_ABORTED               (RTCS_ERROR_BASE|0xF0D)
#define FTE_MQTT_RET_NOT_INITIALIZED                (RTCS_ERROR_BASE|0xF0E)

#define FTE_MQTT_QOS_0              0
#define FTE_MQTT_QOS_1              1
#define FTE_MQTT_QOS_2              2

#define FTE_MQTT_CMD_REQ            1
#define FTE_MQTT_CMD_INFO           2

#define FTE_MQTT_TARGET_SERVER      1
#define FTE_MQTT_TARGET_GATEWAY     2
#define FTE_MQTT_TARGET_LID         3
#define FTE_MQTT_TARGET_UID         4
#define FTE_MQTT_TARGET_BROADCAST   5

typedef enum
{
    FTE_MQTT_MSG_INVALID  = 0,
    FTE_MQTT_MSG_DEV_INFO,
    FTE_MQTT_MSG_DEV_VALUE,
    FTE_MQTT_MSG_EP_INFO,
    FTE_MQTT_MSG_EP_VALUE,
    FTE_MQTT_MSG_GET_VALUE,
    FTE_MQTT_MSG_SET_VALUE,
}   FTE_MQTT_MSG_TYPE, _PTR_ FTE_MQTT_MSG_TYPE_PTR;

typedef enum
{
    FTE_MQTT_METHOD_INVALID,
    FTE_MQTT_METHOD_SET_PROPERTY,
    FTE_MQTT_METHOD_CONTROL_ACTUATOR,
    FTE_MQTT_METHOD_TIME_SYNC,
    FTE_MQTT_METHOD_FTLM_DEVICE_GET,
    FTE_MQTT_METHOD_FTLM_DEVICE_SET,
    FTE_MQTT_METHOD_FTLM_GROUP_GET,
    FTE_MQTT_METHOD_FTLM_GROUP_SET
}   FTE_MQTT_METHOD_TYPE, _PTR_ FTE_MQTT_METHOD_TYPE_PTR;

typedef enum
{
    FTE_MQTT_STATE_UNINITIALIZED = 0,
    FTE_MQTT_STATE_INITIALIZED,
    FTE_MQTT_STATE_CONNECTED,
    FTE_MQTT_STATE_DISCONNECTED
}   FTE_MQTT_STATE, _PTR_ FTE_MQTT_STATE_PTR;

typedef struct
{
    _ip_address     xIPAddress;
    uint_16         usPort;
    uint_32         ulKeepalive;
    
    // Autorization Information
    struct
    {
        uint_32     bEnabled;
        char        pUserName[FTE_MQTT_USERNAME_LENGTH];
        char        pPassword[FTE_MQTT_PASSWORD_LENGTH];
    }   xAuth; 
}   FTE_MQTT_BROKER_CONFIG, _PTR_ FTE_MQTT_BROKER_CONFIG_PTR;

typedef struct
{
    char                    pClientID[FTE_MQTT_CLIENT_ID_LENGTH+1];
    boolean                 bEnable;
    // Broker Information
    FTE_MQTT_BROKER_CONFIG  xBroker;    
    FTE_SSL_CONFIG          xSSL;
    uint_32                 ulPubTimeout;
    
}   FTE_MQTT_CFG, _PTR_ FTE_MQTT_CFG_PTR;

typedef struct
{
    int_32                  nSocketID;
    FTE_MQTT_STATE          xState;
    FTE_MQTT_CFG_PTR        pConfig;
	mqtt_broker_handle_t    xBroker;
    FTE_LIST                xTransList;
    FTE_LIST                xFreeTransList;
    FTE_LIST                xRecvMsgPool;
    FTE_LIST                xSendMsgPool;
    uint_8                  pBuff[FTE_MQTT_RECV_BUFF_SIZE];
    uint_32                 ulBuffSize;
    uint_32                 ulRcvdLen;
    TIME_STRUCT             xTime;
    _timer_id               xPingTimer;
    uint_32                 ulPingTimeout;
    struct
    {
        uint_32                 ulTransTimeout;
    } xStatistics;
    FTE_SSL_CONTEXT_PTR     pxSSL;
}   FTE_MQTT_CONTEXT, _PTR_ FTE_MQTT_CONTEXT_PTR;

typedef struct
{
    uint_16     nMsgID;
    uint_32     nMsgType;    
    uint_32     nQoS;
    TIME_STRUCT xTime;
}   FTE_MQTT_TRANS, _PTR_ FTE_MQTT_TRANS_PTR;

typedef _mqx_uint   (*FTE_MQTT_METHOD_CALLBACK)(void _PTR_ pParams);

typedef struct
{
    FTE_MQTT_METHOD_TYPE        xMethod;
    char_ptr                    pString;
    FTE_MQTT_METHOD_CALLBACK    fCallback;
}   FTE_MQTT_METHOD, _PTR_ FTE_MQTT_METHOD_PTR;

uint_32 FTE_MQTT_load_default(FTE_MQTT_CFG_PTR pConfig);
uint_32 FTE_MQTT_init(FTE_MQTT_CFG_PTR pConfig);

uint_32 FTE_MQTT_publishEPInfo(FTE_OBJECT_ID xEPID, uint_32 nQoS);
uint_32 FTE_MQTT_publishEPValue(FTE_OBJECT_ID xEPID, uint_32 nQoS);

uint_32 FTE_MQTT_TP_publishEPValue(FTE_OBJECT_ID xEPID, uint_32 nQoS);

int_32  FTE_MQTT_SHELL_cmd(int_32 argc, char_ptr argv[]);
#endif
