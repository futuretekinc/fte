#ifndef __FTE_HTTP_H__
#define __FTE_HTTP_H__

#include <httpsrv.h>

typedef struct
{
    boolean                     bEnable;
    uint_32                     ulAtthRealmCount;
    HTTPSRV_AUTH_REALM_STRUCT   pAuthRealms[4];
}   FTE_HTTP_CFG, _PTR_ FTE_HTTP_CFG_PTR;

uint_32 FTE_HTTPD_init(FTE_HTTP_CFG_PTR pConfig);

#endif
