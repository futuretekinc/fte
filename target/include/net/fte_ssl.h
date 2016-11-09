#ifndef __FTE_SSL_H__
#define __FTE_SSL_H__

#include <cyassl/ssl.h>
 
#define FTE_SSL_CIPHER_NAME_MAX_LEN 63
typedef enum
{
    FTE_SSL_METHOD_SSLV3    = 0,
    FTE_SSL_METHOD_TLSV1    = 1,
    FTE_SSL_METHOD_TLSV1_1  = 2,
    FTE_SSL_METHOD_TLSV1_2  = 3
}   FTE_SSL_METHOD, _PTR_ FTE_SSL_METHOD_PTR;

typedef struct FTE_SSL_CONFIG_STRUCT
{
    FTE_BOOL        bEnabled;
    FTE_SSL_METHOD  nMethod;
    FTE_CHAR        pCipher[FTE_SSL_CIPHER_NAME_MAX_LEN+1];
}   FTE_SSL_CONFIG, _PTR_ FTE_SSL_CONFIG_PTR;

typedef struct  FTE_SSL_CONTEXT_STRUCT
{
    CYASSL_CTX*     pxCTX;
    CYASSL*         pxSSL;
}   FTE_SSL_CONTEXT, _PTR_ FTE_SSL_CONTEXT_PTR;

FTE_SSL_CONTEXT_PTR FTE_SSL_create(FTE_SSL_CONFIG_PTR pConfig);
FTE_RET     FTE_SSL_destroy(FTE_SSL_CONTEXT_PTR pxCTX);

FTE_RET     FTE_SSL_connect(FTE_SSL_CONTEXT_PTR pCTX, FTE_INT32   nSocketID);
FTE_RET     FTE_SSL_disconnect(FTE_SSL_CONTEXT_PTR pxCTX);
FTE_INT32   FTE_SSL_send(FTE_SSL_CONTEXT_PTR pCTX, FTE_VOID const _PTR_ pMsg, FTE_INT32    nMsgLen);
FTE_INT32   FTE_SSL_recv(FTE_SSL_CONTEXT_PTR pCTX, FTE_VOID_PTR pBuff, FTE_INT32    nBuffLen);

#endif
