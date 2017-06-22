#ifndef __FTE_NET_H__
#define __FTE_NET_H__

#include <ipcfg.h>
#include "fte_smng.h"
#include "fte_http.h"
#include "fte_snmp.h"
#include "fte_mqtt.h"
#include "fte_telnetd.h"
#include "fte_ssl.h"
#include "fte_mbtcp.h"
#include "fte_mbtcp_master.h"

typedef enum    _fte_net_type
{
    FTE_NET_TYPE_STATIC = 0,
    FTE_NET_TYPE_DHCP   = 1
}   FTE_NET_TYPE;

typedef enum    FTE_NET_STATE_ENUM
{
    FTE_NET_STATE_INIT      = IPCFG_STATE_INIT,
    FTE_NET_STATE_UNBIND    = IPCFG_STATE_UNBOUND,
    FTE_NET_STATE_BUSY      = IPCFG_STATE_BUSY,
    FTE_NET_STATE_STATIC_IP = IPCFG_STATE_STATIC_IP,
    FTE_NET_STATE_DHCP_IP   = IPCFG_STATE_DHCP_IP,
    FTE_NET_STATE_AUTO_IP   = IPCFG_STATE_AUTO_IP,
    FTE_NET_STATE_DHCPAUTO_IP=IPCFG_STATE_DHCPAUTO_IP,
    FTE_NET_STATE_BOOT      = IPCFG_STATE_BOOT
}   FTE_NET_STATE, _PTR_ FTE_NET_STATE_PTR;

typedef struct FTE_NET_CONFIG_STRUCT
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
#if FTE_MBTCP_SUPPORTED
    FTE_MBTCP_SLAVE_CFG         xMBTCP;
#endif
#if FTE_MBTCP_MASTER_SUPPORTED
    FTE_UINT32                  nMBTCPMasterCount;
    FTE_MBTCP_MASTER_CFG        xMBTCPMasters[2];
#endif
}   FTE_NET_CFG, _PTR_ FTE_NET_CFG_PTR;

FTE_RET     FTE_NET_init(FTE_NET_CFG_PTR pConfig);
FTE_RET     FTE_NET_getMACAddress(_enet_address address);

FTE_RET     FTE_NET_isActive(FTE_BOOL_PTR pActive);
FTE_RET     FTE_NET_bind(FTE_VOID);
FTE_RET     FTE_NET_unbind(FTE_VOID);

FTE_RET     FTE_NET_STATE_get(FTE_NET_STATE_PTR   pState);
FTE_CHAR const _PTR_    FTE_NET_STATE_print(FTE_NET_STATE xState);

FTE_INT32   FTE_NET_SERVER_count(FTE_VOID);
FTE_UINT32  FTE_NET_SERVER_getAt(FTE_UINT32 ulIndex);
FTE_BOOL    FTE_NET_SERVER_isExist(_ip_address ip);


void        FTE_NET_STATISTICS_print(FTE_VOID);

FTE_RET     FTE_NET_liveCheckInit(FTE_UINT32 ulKeepAliveTime);
FTE_RET     FTE_NET_liveCheckStart(FTE_VOID);
FTE_RET     FTE_NET_liveCheckStop(FTE_VOID);
FTE_BOOL    FTE_NET_isLiveChecking(FTE_VOID);
FTE_RET     FTE_NET_lastLiveCheckTime(MQX_TICK_STRUCT_PTR pTime);
FTE_RET     FTE_NET_liveTouch(FTE_VOID);
FTE_BOOL    FTE_NET_isStable(FTE_VOID);

FTE_INT32   FTE_PHY_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );
FTE_INT32   FTE_NET_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );

#include "fte_mqtt.h"
#include "fte_telnetd.h"
#endif
