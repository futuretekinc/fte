#ifndef __FTE_SHELL_H__
#define __FTE_SHELL_H__

#define FTE_SHELL_USER_ID_LENGTH    16
#define FTE_SHELL_PASSWD_LENGTH     32

typedef struct
{
    FTE_CHAR    pUserID[FTE_SHELL_USER_ID_LENGTH+1];
    FTE_CHAR    pPasswd[FTE_SHELL_PASSWD_LENGTH+1];
    FTE_UINT32  ulTimeout;
}   FTE_SHELL_CONFIG, _PTR_ FTE_SHELL_CONFIG_PTR;

typedef struct FTE_SHELL_CMD_STRUCT
{   
    FTE_CHAR_PTR    pStrCmd;
    FTE_RET         (*fCmd)(FTE_INT32, FTE_CHAR_PTR []);
    FTE_CHAR_PTR    pUsage;
}   FTE_SHELL_CMD, _PTR_ FTE_SHELL_CMD_PTR;

FTE_RET     FTE_SHELL_proc(void);
FTE_UINT32  FTE_SHELL_printHexString(FTE_UINT8_PTR pData, FTE_UINT32 ulSize, FTE_UINT32 ulColumn);
FTE_UINT32  FTE_SHELL_printNumString(FTE_UINT8_PTR pData, FTE_UINT32 ulSize, FTE_UINT32 ulColumn);

#endif