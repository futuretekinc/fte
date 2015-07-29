#ifndef _FTE_UCM_H__
#define _FTE_UCM_H__

#include "fte_list.h"


typedef struct _FTE_UCM_CONFIG_STRUCT
{
    uint_32         nID;
    uint_32         nUCSID;
} FTE_UCM_CONFIG, _PTR_ FTE_UCM_CONFIG_PTR;

typedef FTE_UCM_CONFIG const _PTR_ FTE_UCM_CONFIG_CONST_PTR;

typedef struct  _FTE_UART_CMD_MASTER_STRUCT
{
    FTE_UCM_CONFIG_CONST_PTR    pConfig;
    uint_32                     nParent;
}   FTE_UCM, _PTR_ FTE_UCM_PTR;

_mqx_uint       FTE_UCM_create(FTE_UCM_CONFIG_CONST_PTR pConfig);
_mqx_uint       FTE_UCM_attach(FTE_UCM_PTR pUCM, uint_32 nParent);
_mqx_uint       FTE_UCM_detach(FTE_UCM_PTR pUCM, uint_32 nParent);

FTE_UCM_PTR     FTE_UCM_get(uint_32 xID);
FTE_UCM_PTR     FTE_UCM_getFirst(FTE_UCS_PTR pUCS);
FTE_UCM_PTR     FTE_UCM_getNext(FTE_UCS_PTR pUCS);

_mqx_uint       FTE_UCM_setDuplexMode(FTE_UCM_PTR pUCM, boolean bFullDuplex);
_mqx_uint       FTE_UCM_getDuplexMode(FTE_UCM_PTR pUCM, boolean *pFullDuplex);


_mqx_uint       FTE_UCM_clear(FTE_UCM_PTR pUCM);
_mqx_uint       FTE_UCM_read(FTE_UCM_PTR pUCM, uint_8_ptr pBuff, uint_32 nBuffLen, uint_32_ptr pReadLen);
_mqx_uint       FTE_UCM_write(FTE_UCM_PTR pUCM, uint_8_ptr pData, uint_32 nDataLen, uint_32_ptr pWrittenLen);


int_32          FTE_UCM_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
