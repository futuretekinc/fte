#ifndef __FTE_MBTCP_H__
#define __FTE_MBTCP_H__

#include "fte_type.h"
#include "fte_modbus.h"
#include <rtcs.h>

#define FTE_MBTCP_FRAME_OFFSET_TID  0
#define FTE_MBTCP_FRAME_OFFSET_PID  2
#define FTE_MBTCP_FRAME_OFFSET_LEN  4
#define FTE_MBTCP_FRAME_OFFSET_UID  6
#define FTE_MBTCP_FRAME_OFFSET_PDU  7

#define FTE_MBTCP_PROTOCOL_ID       0   /* 0 = Modbus Protocol */

typedef struct
{
    FTE_UINT16              usPort;
    FTE_UINT32              ulMaxSessions;
    FTE_MODBUS_SLAVE_CONFIG xMBSlave;
}   FTE_MBTCP_SLAVE_CFG, _PTR_ FTE_MBTCP_SLAVE_CFG_PTR;

typedef struct FTE_MBTCP_SLAVE_SESSION_STRUCT
{
    FTE_TASK_ID             hTask;
    FTE_UINT32              hSock;
    LWSEM_STRUCT            xLWSEM;
    FTE_UINT8               pBuff[FTE_MODBUS_PDU_LEN + 7];
    
    FTE_MODBUS_SLAVE_PTR    pMBSlave;
    struct  FTE_MBTCP_SLAVE_STRUCT _PTR_ pParent;
    
    sockaddr_in             xRemoteAddr;
    
    FTE_TIME                xCreationTime;
    FTE_TIME                xLastTime;
} FTE_MBTCP_SLAVE_SESSION, _PTR_ FTE_MBTCP_SLAVE_SESSION_PTR;

typedef struct  FTE_MBTCP_SLAVE_STRUCT
{
    FTE_MBTCP_SLAVE_CFG         xConfig;
    FTE_UINT32                  hSock;
    FTE_MBTCP_SLAVE_SESSION_PTR pSessions;
    FTE_UINT32                  ulSessions;
    FTE_MODBUS_SLAVE            xMBSlave;
    LWSEM_STRUCT                xLWSEM;
}   FTE_MBTCP_SLAVE, _PTR_ FTE_MBTCP_SLAVE_PTR;

FTE_RET     FTE_MBTCP_SLAVE_init
(
    FTE_MBTCP_SLAVE_CFG_PTR pConfig
);

FTE_RET FTE_MBTCP_SLAVE_receiveFrame
(
    FTE_MBTCP_SLAVE_SESSION_PTR pSession,
    FTE_UINT8_PTR               pBuff,
    FTE_UINT32                  ulBuffSize,
    FTE_UINT32_PTR              pulLen
);

FTE_RET FTE_MBTCP_SLAVE_sendFrame
(
    FTE_MBTCP_SLAVE_SESSION_PTR pSession,
    FTE_UINT8_PTR               pBuff,
    FTE_UINT32                  ulBuffLen
);

void FTE_MBTCP_mainTask(FTE_UINT32  xParam);
void FTE_MBTCP_sessionTask(FTE_UINT32  xParam);

FTE_RET     FTE_MBTCP_getDiscoveryMessage(FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffSize);

FTE_INT32   FTE_MBTCP_SLAVE_SHELL_cmd(FTE_INT32   nArgc, FTE_CHAR_PTR pArgv[] );

#endif
