#ifndef __FTE_CFG_H__
#define __FTE_CFG_H__

#include "fte_target.h"
#include "fte_net.h"
#include "fte_shell.h"

typedef struct _FTE_CFG_desc_struct
{
    char_ptr                            pMTDs[FTE_MTD_MAX_COUNT];
    FTE_SYS_CONFIG const _PTR_          pSystem;
    FTE_SHELL_CONFIG const _PTR_        pShell;
    FTE_NET_CFG const _PTR_             pNetwork;
    uint_32                             nObjects;
    FTE_OBJECT_CONFIG_PTR const _PTR_   pObjects;
    uint_32                             nEvents;
    FTE_CFG_EVENT_PTR const _PTR_       pEvents;
}   FTE_CFG_DESC, _PTR_ FTE_CFG_DESC_PTR;

_mqx_uint           FTE_CFG_init(FTE_CFG_DESC const *pDESC);
_mqx_uint           FTE_CFG_save(boolean bForce);
_mqx_uint           FTE_CFG_clear(void);

boolean             FTE_CFG_isChanged(void);

_mqx_uint           FTE_CFG_setOID(uint_8_ptr pOID, uint_32 nLen);
_mqx_uint           FTE_CFG_getMAC(_enet_address pMAC);
_mqx_uint           FTE_CFG_setMAC(_enet_address pMAC);

_mqx_uint           FTE_CFG_getLocation(char_ptr pLocation, uint_32 nLen);
_mqx_uint           FTE_CFG_setLocation(char_ptr pLocation, uint_32 nLen);

FTE_NET_CFG_PTR     FTE_CFG_NET_get(void);
_mqx_uint           FTE_CFG_NET_set(FTE_NET_CFG const * pCfgNet);
_mqx_uint           FTE_CFG_NET_copy(FTE_NET_CFG_PTR pCfgNet);

_mqx_uint           FTE_CFG_NET_setIP(_ip_address xIP);
_mqx_uint           FTE_CFG_NET_setNetmask(_ip_address xNetmask);
_mqx_uint           FTE_CFG_NET_setGatewayIP(_ip_address nIP);

uint_32             FTE_CFG_NET_TRAP_count(void);
_ip_address         FTE_CFG_NET_TRAP_getAt(uint_32 ulIndex);
_mqx_uint           FTE_CFG_NET_TRAP_addIP(_ip_address nTrapIP);
_mqx_uint           FTE_CFG_NET_TRAP_delIP(_ip_address nTrapIP);
boolean             FTE_CFG_NET_TRAP_isExist(_ip_address nTrapIP);
_mqx_uint           FTE_CFG_NET_TRAP_clear(void);

FTE_OBJECT_CONFIG_PTR   FTE_CFG_OBJ_alloc(uint_32 nID);
_mqx_uint               FTE_CFG_OBJ_free(uint_32 oid);

FTE_OBJECT_CONFIG_PTR   FTE_CFG_OBJ_create
(
    FTE_OBJECT_CONFIG_PTR pConfig,
    FTE_OBJECT_CONFIG_PTR _PTR_ pChildConfigs,
    uint_32             ulMaxCount,
    uint_32_ptr         pulCount
);
pointer                 FTE_CFG_OBJ_get(uint_32 oid);
pointer                 FTE_CFG_OBJ_getAt(uint_32 oid, uint_32 ulMask, uint_32 ulIdx);

uint_32                 FTE_CFG_OBJ_count(uint_32 ulType, uint_32 ulMask);
pointer                 FTE_CFG_OBJ_getFirst(void);
pointer                 FTE_CFG_OBJ_getNext(void);

_mqx_uint               FTE_CFG_OBJ_save(FTE_OBJECT_PTR pObj);

FTE_CFG_EVENT_PTR       FTE_CFG_EVENT_alloc(uint_32  ulOID);
_mqx_uint               FTE_CFG_EVENT_free(FTE_CFG_EVENT_PTR pConfig);

uint_32                 FTE_CFG_EVENT_count(void);
_mqx_uint               FTE_CFG_EVENT_getAt(uint_32 ulIndex, FTE_CFG_EVENT_PTR _PTR_ ppConfig);

pointer                 FTE_CFG_EVENT_getFirst(void);
pointer                 FTE_CFG_EVENT_getNext(void);

_mqx_uint               FTE_CFG_EXT_init(void);

_mqx_uint               FTE_CFG_CIAS_getExtConfig(void _PTR_ pBuff, uint_32 ulBuffLen);
_mqx_uint               FTE_CFG_CIAS_setExtConfig(void _PTR_ pCIAS, uint_32 ulCIASLen);

_mqx_uint               FTE_CFG_IOEX_getExtConfig(void _PTR_ pBuff, uint_32 ulBuffLen);
_mqx_uint               FTE_CFG_IOEX_setExtConfig(void _PTR_ pCIAS, uint_32 ulIOEXLen);

_mqx_uint               FTE_CFG_DOTECH_getExtConfig(void _PTR_ pBuff, uint_32 ulBuffLen);
_mqx_uint               FTE_CFG_DOTECH_setExtConfig(void _PTR_ pBuff, uint_32 ulBuffLen);

boolean                 FTE_CFG_CERT_valid(void);
uint_32                 FTE_CFG_CERT_size(void);
uint_32                 FTE_CFG_CERT_get(void _PTR_ pBuff, uint_32 ulBuffLen);
_mqx_uint               FTE_CFG_CERT_set(void _PTR_ pCert, uint_32 ulCertLen);

int_32                  FTE_CFG_CERT_SHELL_cmd(int_32 argc, char_ptr argv[]);
                                         
FTE_SYS_CONFIG_PTR      FTE_CFG_SYS_get(void);
_mqx_uint               FTE_CFG_SYS_set(FTE_SYS_CONFIG const *pConfig);

boolean                 FTE_CFG_SYS_getSystemMonitor(void);
_mqx_uint               FTE_CFG_SYS_setSystemMonitor(boolean bEnable);

boolean                 FTE_CFG_SYS_isEnabledLiveCheck(void);
uint_32                 FTE_CFG_SYS_getKeepAliveTime(void);
_mqx_uint               FTE_CFG_SYS_setKeepAliveTime(uint_32 ulTime);

FTE_SHELL_CONFIG_PTR    FTE_CFG_SHELL_get(void);
_mqx_uint               FTE_CFG_SHELL_set(FTE_SHELL_CONFIG const *pConfig);

_mqx_uint               FTE_CFG_DBG_getBootTime(uint_32 nIndex, TIME_STRUCT_PTR pTime);

int_32                  FTE_CFG_SHELL_cmd(int_32 argc, char_ptr argv[]);
int_32                  FTE_CFG_SHELL_cmdSave(int_32 argc, char_ptr argv[]);

#endif
