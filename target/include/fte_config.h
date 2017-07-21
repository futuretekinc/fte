#ifndef __FTE_CFG_H__
#define __FTE_CFG_H__

#include "fte_target.h"
#include "fte_net.h"
#include "fte_shell.h"

typedef struct _FTE_CFG_DESC_STRUCT
{
    FTE_CHAR_PTR                        pMTDs[FTE_MTD_MAX_COUNT];
    FTE_SYS_CONFIG const _PTR_          pSystem;
    FTE_SHELL_CONFIG const _PTR_        pShell;
    FTE_NET_CFG const _PTR_             pNetwork;
    FTE_UINT32                          nObjects;
    FTE_OBJECT_CONFIG_PTR const _PTR_   pObjects;
    FTE_UINT32                          nEvents;
    FTE_CFG_EVENT_PTR const _PTR_       pEvents;
}   FTE_CFG_DESC, _PTR_ FTE_CFG_DESC_PTR;

FTE_RET         FTE_CFG_init(FTE_CFG_DESC const *pDESC);
FTE_RET         FTE_CFG_save(FTE_BOOL bForce);
FTE_RET         FTE_CFG_clear(void);
FTE_RET         FTE_CFG_clearObject(void);

FTE_RET         FTE_CFG_isChanged(FTE_BOOL_PTR pChanged);

FTE_RET         FTE_CFG_setOID(FTE_UINT8_PTR pOID, FTE_UINT32 nLen);
FTE_RET         FTE_CFG_getMAC(_enet_address pMAC);
FTE_RET         FTE_CFG_setMAC(_enet_address pMAC);

FTE_RET         FTE_CFG_getLocation(FTE_CHAR_PTR pLocation, FTE_UINT32 nLen);
FTE_RET         FTE_CFG_setLocation(FTE_CHAR_PTR pLocation, FTE_UINT32 nLen);

FTE_RET         FTE_CFG_NET_get(FTE_NET_CFG_PTR _PTR_ ppConfig);
FTE_RET         FTE_CFG_NET_set(FTE_NET_CFG const * pCfgNet);
FTE_RET         FTE_CFG_NET_copy(FTE_NET_CFG_PTR pCfgNet);

FTE_RET         FTE_CFG_NET_setIP(_ip_address xIP);
FTE_RET         FTE_CFG_NET_setNetmask(_ip_address xNetmask);
FTE_RET         FTE_CFG_NET_setGatewayIP(_ip_address nIP);

FTE_UINT32      FTE_CFG_NET_TRAP_count(void);
_ip_address     FTE_CFG_NET_TRAP_getAt(FTE_UINT32 ulIndex);
FTE_RET         FTE_CFG_NET_TRAP_addIP(_ip_address nTrapIP);
FTE_RET         FTE_CFG_NET_TRAP_delIP(_ip_address nTrapIP);
FTE_BOOL        FTE_CFG_NET_TRAP_isExist(_ip_address nTrapIP);
FTE_RET         FTE_CFG_NET_TRAP_clear(void);

FTE_OBJECT_CONFIG_PTR   FTE_CFG_OBJ_alloc(FTE_OBJECT_ID xID);
FTE_RET         FTE_CFG_OBJ_free(FTE_OBJECT_ID xID);

FTE_RET FTE_CFG_OBJ_create
(
    FTE_OBJECT_CONFIG_PTR pBaseConfig,
    FTE_OBJECT_CONFIG_PTR _PTR_ ppNewConfig,
    FTE_OBJECT_CONFIG_PTR _PTR_ ppChildConfigs,
    FTE_UINT32             ulMaxCount,
    FTE_UINT32_PTR         pulCount
);

FTE_VOID_PTR    FTE_CFG_OBJ_get(FTE_OBJECT_ID xID);
FTE_VOID_PTR    FTE_CFG_OBJ_getAt(FTE_OBJECT_ID xID, FTE_UINT32 ulMask, FTE_UINT32 ulIdx);

FTE_UINT32      FTE_CFG_OBJ_count(FTE_UINT32 ulType, FTE_UINT32 ulMask);
FTE_VOID_PTR    FTE_CFG_OBJ_getFirst(void);
FTE_VOID_PTR    FTE_CFG_OBJ_getNext(void);

FTE_RET         FTE_CFG_OBJ_save(FTE_OBJECT_PTR pObj);

FTE_CFG_EVENT_PTR FTE_CFG_EVENT_create(FTE_CFG_EVENT_PTR pConfig);
FTE_CFG_EVENT_PTR   FTE_CFG_EVENT_alloc(FTE_UINT32  ulOID);
FTE_RET         FTE_CFG_EVENT_free(FTE_CFG_EVENT_PTR pConfig);

FTE_UINT32      FTE_CFG_EVENT_count(void);
FTE_RET         FTE_CFG_EVENT_getAt(FTE_UINT32 ulIndex, FTE_CFG_EVENT_PTR _PTR_ ppConfig);

FTE_VOID_PTR    FTE_CFG_EVENT_getFirst(void);
FTE_VOID_PTR    FTE_CFG_EVENT_getNext(void);

FTE_RET         FTE_CFG_EXT_init(void);

FTE_RET         FTE_CFG_CIAS_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen);
FTE_RET         FTE_CFG_CIAS_setExtConfig(FTE_VOID_PTR pCIAS, FTE_UINT32 ulCIASLen);

FTE_RET         FTE_CFG_IOEX_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen);
FTE_RET         FTE_CFG_IOEX_setExtConfig(FTE_VOID_PTR pCIAS, FTE_UINT32 ulIOEXLen);

FTE_RET         FTE_CFG_DIO_NODE_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen);
FTE_RET         FTE_CFG_DIO_NODE_setExtConfig(FTE_VOID_PTR pCIAS, FTE_UINT32 ulIOEXLen);

FTE_RET         FTE_CFG_DOTECH_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen);
FTE_RET         FTE_CFG_DOTECH_setExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen);

FTE_BOOL        FTE_CFG_CERT_valid(void);
FTE_UINT32      FTE_CFG_CERT_size(void);
FTE_RET         FTE_CFG_CERT_get(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen, FTE_UINT32_PTR pLen);
FTE_RET         FTE_CFG_CERT_set(FTE_VOID_PTR pCert, FTE_UINT32 ulCertLen);

FTE_INT32       FTE_CFG_CERT_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
                                         
FTE_RET         FTE_CFG_SYS_get(FTE_SYS_CONFIG_PTR _PTR_ ppConfig);
FTE_RET         FTE_CFG_SYS_set(FTE_SYS_CONFIG const *pConfig);

FTE_BOOL        FTE_CFG_SYS_getSystemMonitor(void);
FTE_RET         FTE_CFG_SYS_setSystemMonitor(FTE_BOOL bEnable);

FTE_UINT32      FTE_CFG_SYS_getAllowedFailureCount(void);

FTE_BOOL        FTE_CFG_SYS_isEnabledLiveCheck(void);
FTE_UINT32      FTE_CFG_SYS_getKeepAliveTime(void);
FTE_RET         FTE_CFG_SYS_setKeepAliveTime(FTE_UINT32 ulTime);

FTE_SHELL_CONFIG_PTR    FTE_CFG_SHELL_get(void);
FTE_RET         FTE_CFG_SHELL_set(FTE_SHELL_CONFIG const *pConfig);

FTE_RET         FTE_CFG_DBG_getBootTime(FTE_UINT32 nIndex, TIME_STRUCT_PTR pTime);

FTE_INT32       FTE_CFG_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
FTE_INT32       FTE_CFG_SHELL_cmdSave(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);

#endif
