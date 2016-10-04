#ifndef __FTE_SHELL_H__
#define __FTE_SHELL_H__

#define FTE_SHELL_USER_ID_LENGTH    16
#define FTE_SHELL_PASSWD_LENGTH     32

typedef struct
{
    char    pUserID[FTE_SHELL_USER_ID_LENGTH+1];
    char    pPasswd[FTE_SHELL_PASSWD_LENGTH+1];
    uint_32 ulTimeout;
}   FTE_SHELL_CONFIG, _PTR_ FTE_SHELL_CONFIG_PTR;

_mqx_uint   FTE_SHELL_proc(void);
_mqx_int    FTE_SHELL_printHexString(uint_8 *pData, uint_32 ulSize, uint_32 ulColumn);
_mqx_int    FTE_SHELL_printNumString(uint_8 *pData, uint_32 ulSize, uint_32 ulColumn);

#endif