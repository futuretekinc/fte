#ifndef __FTE_SYS_H__
#define __FTE_SYS_H__

#include "fte_type.h"
#include "fte_sys_lock.h"

typedef struct
{
    struct
    {
        FTE_BOOL     bSystemMonitor:1;
    }   xFlags;
    FTE_UINT32         ulAllowFailureCount;
    FTE_UINT32         ulKeepAliveTime;
    FTE_UINT32         ulAutoSaveInterval;
}   FTE_SYS_CONFIG, _PTR_ FTE_SYS_CONFIG_PTR;

typedef void (_CODE_PTR_  FTE_SYS_STATE_CHANGE_FPTR)(pointer);

void        FTE_SYS_powerUp(void);

void        FTE_SYS_STATE_initialized(void);
void        FTE_SYS_STATE_connected(void);
void        FTE_SYS_STATE_disconnected(void);
void        FTE_SYS_STATE_factoryResetRequested(void);
void        FTE_SYS_STATE_factoryResetCanceled(void);
void        FTE_SYS_STATE_factoryResetFinished(void);
void        FTE_SYS_STATE_setAlert(void);

FTE_UINT32     FTE_SYS_STATE_get(void);
void        FTE_SYS_STATE_setChangeCB(FTE_SYS_STATE_CHANGE_FPTR fCallback);

FTE_RET   FTE_SYS_powerHoldInit(void);
FTE_RET   FTE_SYS_powerHold(FTE_BOOL bHoldOn);

void        FTE_SYS_powerStateInit(void);

void        FTE_SYS_SWDetectInit(void);

void        FTE_SYS_factoryResetInit(void);
FTE_BOOL    FTE_SYS_isfactoryResetPushed(void);
void        FTE_SYS_factoryReset(FTE_BOOL bPushed);

FTE_RET     FTE_SYS_DEVICE_resetInit(void);
FTE_RET     FTE_SYS_DEVICE_reset(void);

void        FTE_SYS_LCD_init(void);

FTE_INT32   FTE_SYS_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
FTE_INT32   FTE_SYS_RESET_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
FTE_INT32   FTE_SYS_SHUTDOWN_SHELL_cmd(FTE_INT32 argc, FTE_CHAR_PTR argv[]);

FTE_UINT32  FTE_SYS_getTime(void);


void        FTE_SYS_setUnstable(void);
FTE_BOOL    FTE_SYS_isUnstable(void);

void        FTE_SYS_reset(void);
void        FTE_SYS_liveCheckAndReset(void);

FTE_RET     FTE_SYS_liveCheckStart(void);
FTE_RET     FTE_SYS_liveCheckStop(void);
FTE_BOOL    FTE_SYS_isLiveChecking(void);

FTE_RET     FTE_SYS_getOID(uint_8_ptr pOID);
FTE_CHAR_PTR    FTE_SYS_getOIDString(void);

FTE_RET     FTE_SYS_getMAC(uint_8_ptr pMAC);

#endif
