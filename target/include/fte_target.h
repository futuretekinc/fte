#ifndef __FTE_TARGET_H__
#define __FTE_TARGET_H__

#include <mqx.h>
#include <bsp.h>
#include <assert.h>
#include <shell.h>
#include <timer.h>
#include <event.h>
#include <user_config.h>
#include <stdlib.h>

#include "fte_type.h"
#include "fte_assert.h"
#include "fte_debug.h"
#include "fte_task.h"
#include "fte_sys.h"
#include "fte_timer.h"

#define VERSION(A,B,C,D)                ((((A) & 0xFF) << 24) | (((B) & 0xFF) << 16) | (((C) & 0xFF) << 8) | ((D) & 0xFF))

#define FTE_STATE_POWER_UP              0x0001
#define FTE_STATE_INITIALIZED           0x0002
#define FTE_STATE_CONNECTED             0x0004
#define FTE_STATE_UNSTABLED             0x0008
#define FTE_STATE_WARNING               0x0100
#define FTE_STATE_ALERT                 0x0200

#define FTE_MANUFACTURER                "FutureTek,Inc."
#define FTE_HW_VERSION                  VERSION(1,1,1,2)
#define FTE_SW_VERSION                  VERSION(0x01,0x33,0x53,0xca)
#define FTE_COPYRIGHT                   "Copyright (c) 2010 FutureTek,Inc.\n"
#define FTE_MODEL_LENGTH                16
#define FTE_DID_SIZE                    32
#define FTE_OID_SIZE                    32
#define FTE_EPID_SIZE                   8
#define FTE_MAC_SIZE                    6

#define FTE_LOCATION_MAX_LEN            32
#define FTE_MTD_MAX_COUNT               12

#define FTE_LOG_BOOT_TIME_MAX_COUNT     5


/******************************************************************************
 * System Configuration
 ******************************************************************************/
#define FTE_SYS_KEEP_ALIVE_TIME         300
#define FTE_SYS_KEEP_ALIVE_TIME_MIN     (60)
#define FTE_SYS_KEEP_ALIVE_TIME_MAX     ((60 * 60) * 24)

#define FTE_SYS_LIVE_CHECK_INTERVAL     100
     
#define FTE_SYS_AUTO_SAVE_INTERVAL      5000
/******************************************************************************
 * Network Configuration
 ******************************************************************************/
#define FTE_NET_INIT_WAITING_TIME       10000   //  10 s
#define FTE_NET_STATE_CHECK_INTERVAL    500     //  500 ms

#define FTE_NET_PHY_ADDR                0

#define FTE_NET_OUI_0                   0x00
#define FTE_NET_OUI_1                   0x40
#define FTE_NET_OUI_2                   0x5c
     
/* IP address macros */
#define FTE_NET_DEFAULT_IP              IPADDR(192,168,1,100)
#define FTE_NET_DEFAULT_NETMASK         IPADDR(255,255,255,0) 
#define FTE_NET_DEFAULT_GATEWAY_IP      IPADDR(192,168,1,1) 

#define FTE_NET_SMNG_BUFF_SIZE          512
#define FTE_NET_SMNG_STACK              (FTE_TASK_DEFAULT_STACK * 2 + FTE_NET_SMNG_BUFF_SIZE)
#define FTE_NET_SMNG_PRIO               9
#define FTE_NET_SMNG_PORT               1234

/* MODBUS/TCP Configuration */
#define FTE_NET_MBTCP_BUFF_SIZE         512
#define FTE_NET_MBTCP_STACK             (FTE_TASK_DEFAULT_STACK * 2 + FTE_NET_SMNG_BUFF_SIZE)
#define FTE_NET_MBTCP_PRIO              9
#define FTE_NET_MBTCP_PORT              502
#define FTE_NET_MBTCP_MAX_SESSION       1
     
/* SNMP Configuration */
#define FTE_NET_SNMP_TRAP_V1            0
#define FTE_NET_SNMP_TRAP_V2            1

#define FTE_NET_SNMP_MIB1213            1
#define FTE_NET_SNMP_MIBMQX             0

#define FTE_NET_SNMP_NAME               "snmp"
#define FTE_NET_SNMP_PRIO               9
#define FTE_NET_SNMP_STACK              (FTE_TASK_DEFAULT_STACK * 4)

#define FTE_NET_SNMP_TRAP_SPEC          3
#define FTE_NET_SNMP_TRAP_COUNT         5
#define FTE_NET_SNMP_COMMUNITY_LENGTH   32
     
/* Use this define to tell example if only one server should be used for all interfaces */
#define FTE_NET_HTTP_MAX_SESSION        6
#define FTE_NET_HTTP_NAME               "http"
#define FTE_NET_HTTP_STACK              (FTE_TASK_DEFAULT_STACK * 4)
#define FTE_NET_HTTP_CGI_BUFF_SIZE      1536

#define FTE_NET_HTTP_INET_AF            AF_INET         
#define FTE_NET_HTTP_SCOPE_ID           0 /* For any IF. */

#define FTE_NET_MQTT_NAME               "mqtt"
#define FTE_NET_MQTT_STACK              (FTE_TASK_DEFAULT_STACK * 20)
#define FTE_NET_MQTT_SENDER_NAME        "mqtt_sender"
#define FTE_NET_MQTT_SENDER_STACK       (FTE_TASK_DEFAULT_STACK * 2)
#define FTE_NET_MQTT_RECEIVER_NAME      "mqtt_receiver"
#define FTE_NET_MQTT_RECEIVER_STACK     (FTE_TASK_DEFAULT_STACK * 3)
#define FTE_NET_MQTT_PRIO               9
#define FTE_NET_MQTT_PORT               8883
#define FTE_NET_MQTT_BROKER             IPADDR(10, 0, 1, 18)
#define FTE_NET_MQTT_KEEPALIVE          60
#define FTE_NET_MQTT_PUB_TIMEOUT        60
#define FTE_NET_MQTT_WITH_SSL           TRUE
#define FTE_NET_MQTT_SSL_METHOD         FTE_SSL_METHOD_TLSV1_2
     
#define FTE_NET_TELNETD_NAME            "telnet"
#define FTE_NET_TELNETD_PRIO            9
#define FTE_NET_TELNETD_STACK           (FTE_TASK_DEFAULT_STACK * 4)

/******************************************************************************
 * Object Management Configuration
 ******************************************************************************/
#define FTE_OBJ_CHECK_FAILURE_COUNT_MAX 64
#define FTE_OBJ_ALLOW_FAILURE_COUNT     32
#define FTE_OBJ_EVENT_CHECK_INTERVAL    200 /* milliseconds */
#define FTE_OBJ_LIVE_CHECK_INTERVAL     60  /* seconds */


typedef struct _PRODUCT_DESC_STRUCT
{
    FTE_CHAR_PTR    pModel;
    FTE_CHAR_PTR    pManufacturer;
    struct
    {
        FTE_UINT32  hw; 
        FTE_UINT32  sw;
    } xVersion;
}   FTE_PRODUCT_DESC, _PTR_ FTE_PRODUCT_DESC_PTR;

extern  const SHELL_COMMAND_STRUCT shell_commands[];

FTE_RET     FTE_PLATFORM_init(void);
FTE_RET     FTE_PLATFORM_run(void);

#ifndef FTE_SHELL_TIMEOUT
    #define FTE_SHELL_TIMEOUT               60  /* 60 secs */
#endif


#include "fte_drv.h"
#include "fte_object.h"
#include "fte_utils.h"
#include "fte_debug.h"
#include "fte_mem.h"
#include "fte_sys.h"

#if defined(FTE_ES)
#include "fte_es.h"
#elif defined(FTE_ES1)
#include "fte_es1.h"
#elif defined(FTE_ES2)
#include "fte_es2.h"
#elif defined(FTE_ES3)
#include "fte_es3.h"
#elif defined(FTE_ES4)
#include "fte_es4.h"
#elif defined(FTE_ES5)
#include "fte_es5.h"
#elif defined(FTE_ES7)
#include "fte_es7.h"
#elif defined(FTE_ES8)
#include "fte_es8.h"
#elif defined(FTE_ES9)
#include "fte_es9.h"
#elif defined(FTE_MST)
#include "fte_mst.h"
#elif defined(FTE_ES10)
#include "fte_es10.h"
#elif defined(FTE_ES11)
#include "fte_es11.h"
#elif defined(FTE_ES12)
#include "fte_es12.h"
#elif defined(FTE_ES13)
#include "fte_es13.h"
#elif defined(FTE_ES14)
#include "fte_es14.h"
#elif defined(FTE_ES15)
#include "fte_es15.h"
#elif defined(FTE_ES16)
#include "fte_es16.h"
#elif defined(FTE_ES17)
#include "fte_es17.h"
#elif defined(FTE_ES18)
#include "fte_es18.h"
#elif defined(FTE_ES19)
#include "fte_es19.h"
#elif defined(FTE_ES20)
#include "fte_es20.h"
#elif defined(FTE_ES21)
#include "fte_es21.h"
#elif defined(FTE_ES22)
#include "fte_es22.h"
#elif defined(FTE_EH1)
#include "fte_eh1.h"
#elif defined(FTE_EH3)
#include "fte_eh3.h"
#elif defined(FTE_MN1)
#include "fte_mn1.h"
#endif

/******************************************************************************
 * Task 
 ******************************************************************************/
#define FTE_TASK_MAIN                   1
#define FTE_TASK_WATCHDOG               2
#define FTE_TASK_OBJECT_MNGT            3
#define FTE_TASK_SHELL                  4
#define FTE_TASK_NET                    5
#define FTE_TASK_SMNG                   6
#define FTE_TASK_TIMER                  7
#define FTE_TASK_UCS_RX                 8
#define FTE_TASK_UCS_TX                 9
#define FTE_TASK_EVENT                  10
#define FTE_TASK_CIAS_SIOUX_CU          11
#define FTE_TASK_IOEX                   12
#if FTE_LORA_SUPPORTED
#define FTE_TASK_LORA_CTRL              13
#define FTE_TASK_LORAWAN                14
#define FTE_TASK_S2LORA                 15
#endif
#define FTE_TASK_DOTECH                 17
#define FTE_TASK_SOHA                   18
#define FTE_TASK_ELT                    19

#define FTE_TASK_MBTCP_MAIN             20
#define FTE_TASK_MBTCP_SESSION          21
#define FTE_TASK_DIO_NODE         22

#define FTE_TASK_DEFAULT_STACK          512

#if FTE_TASK_MAIN
    #define FTE_TASK_MAIN_STACK         (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_MAIN_PRIO          9
#endif
     
#if FTE_TASK_WATCHDOG
    #define FTE_TASK_WATCHDOG_STACK     FTE_TASK_DEFAULT_STACK
    #define FTE_TASK_WATCHDOG_PRIO      8
    #define FTE_TASK_WATCHDOG_TIME      5000
#endif
#if FTE_TASK_NET
    #define FTE_TASK_NET_STACK          (FTE_TASK_DEFAULT_STACK * 6)
    #define FTE_TASK_NET_PRIO           9
#endif

#if FTE_TASK_SHELL
    #define FTE_TASK_SHELL_STACK        (FTE_TASK_DEFAULT_STACK * 4)
    #define FTE_TASK_SHELL_PRIO         9
#endif
     
#if FTE_TASK_TIMER
    #define FTE_TASK_TIMER_STACK        FTE_TASK_DEFAULT_STACK * 2
    #define FTE_TASK_TIMER_PRIO         9
#endif

#if FTE_TASK_UCS_RX
    #define FTE_TASK_UCS_RX_STACK       FTE_TASK_DEFAULT_STACK
    #define FTE_TASK_UCS_RX_PRIO        9
#endif

#if FTE_TASK_UCS_TX
    #define FTE_TASK_UCS_TX_STACK       FTE_TASK_DEFAULT_STACK
    #define FTE_TASK_UCS_TX_PRIO        9
#endif

#if FTE_TASK_OBJECT_MNGT
    #define FTE_TASK_OBJECT_MNGT_STACK  (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_OBJECT_MNGT_PRIO   9
#endif
     
#if FTE_TASK_EVENT
    #define FTE_TASK_EVENT_STACK        (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_EVENT_PRIO         9
#endif

#if FTE_TASK_CIAS_SIOUX_CU
    #define FTE_TASK_CIAS_SIOUX_CU_STACK        (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_CIAS_SIOUX_CU_PRIO 9
#endif

#if FTE_TASK_IOEX
    #define FTE_TASK_IOEX_STACK         (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_IOEX_PRIO          9
#endif

#if FTE_TASK_DIO_NODE
	#define FTE_TASK_DIO_NODE_STACK         (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_DIO_NODE_PRIO          9
#endif

#if FTE_TASK_DOTECH
    #define FTE_TASK_DOTECH_STACK         (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_DOTECH_PRIO          9
#endif

#if FTE_TASK_LORA
    #define FTE_TASK_LORA_STACK         (FTE_TASK_DEFAULT_STACK * 4)
    #define FTE_TASK_LORA_PRIO          9
#endif

#if FTE_TASK_LORAWAN
    #define FTE_TASK_LORAWAN_STACK      (FTE_TASK_DEFAULT_STACK * 4)
    #define FTE_TASK_LORAWAN_PRIO       9
#endif

#if FTE_TASK_S2LORA
    #define FTE_TASK_S2LORA_STACK       (FTE_TASK_DEFAULT_STACK * 4)
    #define FTE_TASK_S2LORA_PRIO        9
#endif

#if FTE_TASK_SOHA
    #define FTE_TASK_SOHA_STACK         (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_SOHA_PRIO          9
#endif

#if FTE_TASK_ELT
    #define FTE_TASK_ELT_STACK         (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_ELT_PRIO          9
#endif

#if FTE_TASK_MBTCP_MAIN
    #define FTE_TASK_MBTCP_MAIN_STACK   (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_MBTCP_MAIN_PRIO    9
#endif

#if FTE_TASK_MBTCP_SESSION
    #define FTE_TASK_MBTCP_SESSION_STACK         (FTE_TASK_DEFAULT_STACK * 2)
    #define FTE_TASK_MBTCP_SESSION_PRIO          9
#endif

#if FTE_DI_SUPPORTED                    
    #if !FTE_DI_MAX_COUNT            
        #error  This application requires DI count.
    #endif
#endif

#if FTE_DO_SUPPORTED                    
    #if !FTE_DO_MAX_COUNT            
        #error  This application requires DO count.
    #endif
#endif

#if FTE_RL_SUPPORTED                    
    #if !FTE_RL_MAX_COUNT            
        #error  This application requires RL count.
    #endif
#endif

#if FTE_RTD_SUPPORTED
    #if !FTE_AD7785_SUPPORTED            
        #error  This application requires AD7785.
    #endif

    #if !FTE_PT100_MAX_COUNT         
        #error  This application requires PT-100 count.
    #endif

    #if !FTE_RTD_INTERVAL
        #warning    RTD measure interval is not set. 
        #warning    The measurement interval is set to 10 seconds.
        #define FTE_RTD_INTERVAL    10
    #endif

    #define FTE_RTD_HIGH_BOUND   12000
    #define FTE_RTD_LOW_BOUND   -10000

#endif

#if FTE_GAI_SUPPORTED
#endif

#if FTE_DS18B20_SUPPORTED
    #if !FTE_DS18B20_MAX_COUNT       
        #error  This application requires DS18B20 count.
    #endif
#endif

#if FTE_SHT_SUPPORTED
    #if !FTE_SHT_MAX_COUNT           
        #error  This application requires SHT count.
    #endif

    #if !FTE_SHT_DEFAULT_UPDATE_INTERVAL
        #warning    SHT(Humidity) measure interval is not set. 
        #warning    The measurement interval is set to 10 seconds.
        #define FTE_SHT_DEFAULT_UPDATE_INTERVAL 10000
    #endif

#endif

#if FTE_1WIRE_SUPPORTED
    #if !FTE_1WIRE_MAX_DEVICES       
        #error  This application requires 1WIRE device count.
    #endif
#endif

#ifndef FTE_POWER_OFF_DETECT_TIME
    #define FTE_POWER_OFF_DETECT_TIME   1
#endif

#ifndef FTE_FACTORY_RESET_DETECT_TIME
    #define FTE_FACTORY_RESET_DETECT_TIME   5000    /* msecs */
#endif

#ifndef FTE_LOG_MAX_COUNT
    #define FTE_LOG_MAX_COUNT   100
#endif

#define FTE_OBJ_LED_SYS0_STATUS  	MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 100)
#define FTE_OBJ_LED_SYS1_STATUS  	MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 101)
#define FTE_OBJ_TYPE_IOEX_RESET 	MAKE_SYSTEM_ID(FTE_OBJ_TYPE_DO, 100)
#define FTE_OBJ_TYPE_DIO_NODE_RESET MAKE_SYSTEM_ID(FTE_OBJ_TYPE_DO, 101)
#endif

