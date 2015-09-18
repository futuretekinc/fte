#ifndef __FTE_SYS_H__
#define __FTE_SYS_H__

#include "fte_sys_lock.h"

typedef struct
{
    struct
    {
        boolean     bSystemMonitor:1;
    }   xFlags;
    uint_32         ulAllowFailureCount;
    uint_32         ulKeepAliveTime;
    uint_32         ulAutoSaveInterval;
}   FTE_SYS_CONFIG, _PTR_ FTE_SYS_CONFIG_PTR;

typedef void (_CODE_PTR_  FTE_SYS_STATE_CHANGE_FPTR)(pointer);

void        FTE_SYS_powerUp(void);

void        FTE_SYS_STATE_initialized(void);
void        FTE_SYS_STATE_connected(void);
void        FTE_SYS_STATE_disconnected(void);

uint_32     FTE_SYS_STATE_get(void);
void        FTE_SYS_STATE_setChangeCB(FTE_SYS_STATE_CHANGE_FPTR fCallback);

_mqx_uint   FTE_SYS_powerHoldInit(void);
_mqx_uint   FTE_SYS_powerHold(boolean bHoldOn);

void        FTE_SYS_powerStateInit(void);

void        FTE_SYS_SWDetectInit(void);

void        FTE_SYS_factoryResetInit(void);
boolean     FTE_SYS_isfactoryResetPushed(void);

_mqx_uint   FTE_SYS_DEVICE_resetInit(void);
_mqx_uint   FTE_SYS_DEVICE_reset(void);

void        FTE_SYS_LCD_init(void);

int_32      FTE_SYS_SHELL_cmd(int_32 nArgc, char_ptr pArgv[]);
int_32      FTE_SYS_RESET_cmd(int_32 nArgc, char_ptr pArgv[]);
int_32      FTE_SYS_SHUTDOWN_SHELL_cmd(int_32 argc, char_ptr argv[]);

uint_32     FTE_SYS_getTime(void);

_mqx_uint   FTE_SYS_LIVE_CHECK_init(uint_32 ulKeepAliveTime);
_mqx_uint   FTE_SYS_LIVE_CHECK_touch(void);
_mqx_uint   FTE_SYS_LIVE_CHECK_start(void);
_mqx_uint   FTE_SYS_LIVE_CHECK_stop(void);
boolean     FTE_SYS_LIVE_CHECK_isRun(void);
boolean     FTE_SYS_LIVE_CHECK_isLive(void);
MQX_TICK_STRUCT_PTR FTE_SYS_LIVE_CHECK_lastCheckTime(void);

void        FTE_SYS_reset(void);
void        FTE_SYS_setUnstable(void);
boolean     FTE_SYS_isUnstable(void);

_mqx_uint   FTE_SYS_getOID(uint_8_ptr pOID);
char_ptr    FTE_SYS_getOIDString(void);

_mqx_uint   FTE_SYS_getMAC(uint_8_ptr pMAC);

#endif
