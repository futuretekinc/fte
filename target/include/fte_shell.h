#ifndef __FTE_SHELL_H__
#define __FTE_SHELL_H__

#define FTE_SHELL_USER_ID_LENGTH    16
#define FTE_SHELL_PASSWD_LENGTH     32

typedef struct
{
    char    pUserID[FTE_SHELL_USER_ID_LENGTH+1];
    char    pPasswd[FTE_SHELL_PASSWD_LENGTH+1];
}   FTE_SHELL_CONFIG, _PTR_ FTE_SHELL_CONFIG_PTR;

_mqx_uint   FTE_SHELL_proc(void);

#endif