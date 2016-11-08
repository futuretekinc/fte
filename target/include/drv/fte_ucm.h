#ifndef _FTE_UCM_H__
#define _FTE_UCM_H__

#include "fte_list.h"


typedef struct _FTE_UCM_CONFIG_STRUCT
{
    FTE_UINT32         nID;
    FTE_UINT32         nUCSID;
} FTE_UCM_CONFIG, _PTR_ FTE_UCM_CONFIG_PTR;

typedef FTE_UCM_CONFIG const _PTR_ FTE_UCM_CONFIG_CONST_PTR;

typedef struct  _FTE_UART_CMD_MASTER_STRUCT
{
    FTE_UCM_CONFIG_CONST_PTR    pConfig;
    FTE_UINT32                     nParent;
}   FTE_UCM, _PTR_ FTE_UCM_PTR;

FTE_RET       FTE_UCM_create(FTE_UCM_CONFIG_CONST_PTR pConfig);
FTE_RET       FTE_UCM_attach(FTE_UCM_PTR pUCM, FTE_UINT32 nParent);
FTE_RET       FTE_UCM_detach(FTE_UCM_PTR pUCM, FTE_UINT32 nParent);

FTE_UCM_PTR     FTE_UCM_get(FTE_UINT32 xID);
FTE_UCM_PTR     FTE_UCM_getFirst(FTE_UCS_PTR pUCS);
FTE_UCM_PTR     FTE_UCM_getNext(FTE_UCS_PTR pUCS);

FTE_RET       FTE_UCM_setDuplexMode(FTE_UCM_PTR pUCM, FTE_BOOL bFullDuplex);
FTE_RET       FTE_UCM_getDuplexMode(FTE_UCM_PTR pUCM, FTE_BOOL_PTR pFullDuplex);


FTE_RET       FTE_UCM_clear(FTE_UCM_PTR pUCM);
FTE_RET       FTE_UCM_read(FTE_UCM_PTR pUCM, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen, FTE_UINT32_PTR pReadLen);
FTE_RET       FTE_UCM_write(FTE_UCM_PTR pUCM, FTE_UINT8_PTR pData, FTE_UINT32 nDataLen, FTE_UINT32_PTR pWrittenLen);


int_32          FTE_UCM_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
