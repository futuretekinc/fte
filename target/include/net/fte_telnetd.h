#ifndef __FTE_TELNETD_H__
#define __FTE_TELNETD_H__

#ifndef FTE_TELNETD_ID_LENGTH
#define FTE_TELNETD_ID_LENGTH       32
#endif

#ifndef FTE_TELNETD_PASSWD_LENGTH
#define FTE_TELNETD_PASSWD_LENGTH   32
#endif

typedef struct
{
    char    pID[FTE_TELNETD_ID_LENGTH];
    char    pPasswd[FTE_TELNETD_PASSWD_LENGTH];
}   FTE_TELNETD_CFG, _PTR_ FTE_TELNETD_CFG_PTR;

_mqx_uint   FTE_TELNETD_init(void _PTR_ pParams);

#endif
