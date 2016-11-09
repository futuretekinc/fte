#ifndef __FTE_SMNG_H__
#define __FTE_SMNG_H__

typedef struct
{
    FTE_UINT32  pulHideClass[16];
}   FTE_SMNGD_CFG, _PTR_ FTE_SMNGD_CFG_PTR;

FTE_RET     FTE_SMNGD_init(FTE_VOID_PTR pParams);
FTE_RET     FTE_SMNG_getDiscoveryMessage(FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffSize);

FTE_INT32   FTE_SMNGD_SHELL_cmd(FTE_INT32   nArgc, FTE_CHAR_PTR pArgv[] );

#endif
