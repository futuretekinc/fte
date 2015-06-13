#ifndef __FTE_NET_H__
#define __FTE_NET_H__

#include <ipcfg.h>
#include "fte_smng.h"
#include "fte_http.h"
#include "fte_snmp.h"
#include "fte_mqtt.h"
#include "fte_telnetd.h"
#include "fte_ssl.h"

typedef enum    _fte_net_type
{
    FTE_NET_TYPE_STATIC = 0,
    FTE_NET_TYPE_DHCP   = 1
}   FTE_NET_TYPE;

typedef struct _fte_net_config_struct
{
    FTE_NET_TYPE                nType;
    IPCFG_IP_ADDRESS_DATA       xIPData;

#if FTE_SNMPD_SUPPORTED
    FTE_SNMP_CFG                xSNMP;
#endif
#if FTE_HTTPD_SUPPORTED
    FTE_HTTP_CFG                xHTTP;
#endif
#if FTE_MQTT_SUPPORTED
    FTE_MQTT_CFG                xMQTT;
#endif
#if FTE_TELNETD_SUPPORTED
    FTE_TELNETD_CFG             xTelnetd;
#endif
    
}   FTE_NET_CFG, _PTR_ FTE_NET_CFG_PTR;

int_32  FTE_NET_init(FTE_NET_CFG_PTR pConfig);
int_32  FTE_NET_getMACAddress(_enet_address address);

uint_32 FTE_NET_bind(void);
uint_32 FTE_NET_unbind(void);

int_32  FTE_NET_SERVER_count(void);
uint_32 FTE_NET_SERVER_getAt(uint_32 idx);
boolean FTE_NET_SERVER_isExist(_ip_address ip);

void    FTE_NET_printStats(void);

int_32  FTE_PHY_SHELL_cmd(int_32 argc, char_ptr argv[] );
int_32  FTE_NET_SHELL_cmd(int_32 argc, char_ptr argv[] );

#include "fte_mqtt.h"
#include "fte_telnetd.h"
#endif
