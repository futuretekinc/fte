#include "fte_target.h"
#include "fte_config.h"
#include "fte_net.h"
#include "sys/fte_sys.h"
#include <tfs.h>
#include <ipcfg.h>
#include <cyassl/ssl.h>

#define FTE_SSL_ERR(...)    DEBUG(__VA_ARGS__)
#define FTE_SSL_MSG(...)    TRACE(DEBUG_NET_SSL, __VA_ARGS__)

#define CLIENT_DEFAULT_VERSION 3

FTE_SSL_CONTEXT_PTR FTE_SSL_create
(
    FTE_SSL_CONFIG_PTR pConfig
)
{
    FTE_SSL_CONTEXT_PTR     pxCTX   = NULL;
    CYASSL_METHOD*          pxMETHOD= NULL;
    uchar_ptr               pCACert;
    uint_32                 ulCACertLen = 0;
    ASSERT(pConfig != NULL);
    
    pxCTX = (FTE_SSL_CONTEXT_PTR)FTE_MEM_allocZero(sizeof(FTE_SSL_CONTEXT));
    if (pxCTX == NULL)
    {
        FTE_SSL_ERR("Can not allocate memory.\n");
        goto error;
    }
    
    ulCACertLen = FTE_CFG_CERT_size();
    if (ulCACertLen == 0)
    {
        FTE_SSL_ERR("Certificate does not exist.\n");
        goto error;
    }
    
    pCACert = (uchar_ptr)FTE_MEM_allocZero(ulCACertLen);
    if (pCACert == NULL)
    {
        FTE_SSL_ERR("Can not allocate memory.\n");
        goto error;
    }
    
    if (ulCACertLen != FTE_CFG_CERT_get(pCACert, ulCACertLen))
    {
        FTE_SSL_ERR("Cannot import the certificate. - buffer too small\n");
        goto error;
    }
    
    CyaSSL_Init();

#if FTE_SSL_DEBUG
    CyaSSL_Debugging_ON();
#endif
    
    switch (pConfig->nMethod) 
    {
    case FTE_SSL_METHOD_SSLV3:  pxMETHOD = CyaSSLv3_client_method();      break;
    case FTE_SSL_METHOD_TLSV1:  pxMETHOD = CyaTLSv1_client_method();      break;
    case FTE_SSL_METHOD_TLSV1_1:pxMETHOD = CyaTLSv1_1_client_method();    break;
    case FTE_SSL_METHOD_TLSV1_2:pxMETHOD = CyaTLSv1_2_client_method();    break;
    default:
        FTE_SSL_ERR("Bad SSL version[Version : %d]\n", pConfig->nMethod);
        goto error;
    }
   
    pxCTX->pxCTX = CyaSSL_CTX_new(pxMETHOD);
    if (pxCTX->pxCTX == NULL)
    {
        FTE_SSL_ERR("unable to get CTX\n");
        goto error;
    }
   
    if (CyaSSL_CTX_load_verify_buffer(pxCTX->pxCTX, pCACert, ulCACertLen, SSL_FILETYPE_PEM) != SSL_SUCCESS)
    {
        FTE_SSL_ERR("Failed to verify the certificate.\n");
        goto error;
    }
    
    if (CyaSSL_CTX_set_cipher_list(pxCTX->pxCTX, pConfig->pCipher) != SSL_SUCCESS)
    {
        FTE_SSL_ERR("Failed to set cipher.\n");
        goto error;
    }


    pxCTX->pxSSL = CyaSSL_new(pxCTX->pxCTX);
    if (pxCTX->pxSSL == NULL)
    {        
        FTE_SSL_ERR("Cannot allocate SSL context.\n");
        goto error;
    }
    
    FTE_MEM_free(pCACert);
    
    return  pxCTX;
    
error:
    if (pCACert != NULL)
    {
        FTE_MEM_free(pCACert);
        pCACert = NULL;
    }
    
    if (pxCTX != NULL)
    {
        if (pxCTX->pxSSL != NULL)
        {
            CyaSSL_free(pxCTX->pxSSL);
            pxCTX->pxSSL = NULL;
        }
        
        if (pxCTX->pxCTX != NULL)
        {
            CyaSSL_CTX_free(pxCTX->pxCTX);
        }
        
        FTE_MEM_free(pxCTX);
    }
    
    return  NULL;
}

FTE_RET   FTE_SSL_destroy
(
    FTE_SSL_CONTEXT_PTR     pxCTX
)
{
    ASSERT(pxCTX != NULL);
    
    if (pxCTX->pxSSL != NULL)
    {
        CyaSSL_free(pxCTX->pxSSL);
        pxCTX->pxSSL = NULL;
    }
    
    if (pxCTX->pxCTX != NULL)
    {
        CyaSSL_CTX_free(pxCTX->pxCTX);
        pxCTX->pxCTX = NULL;
    }
        
    FTE_MEM_free(pxCTX);
    
    return  RTCS_OK;
}

FTE_RET   FTE_SSL_connect
(
    FTE_SSL_CONTEXT_PTR     pxCTX, 
    FTE_INT32               nSocketID
)
{
    CyaSSL_set_fd(pxCTX->pxSSL, nSocketID);        
    CyaSSL_set_using_nonblock(pxCTX->pxSSL, TRUE);

#ifndef CYASSL_CALLBACKS
    int nRet = CyaSSL_connect(pxCTX->pxSSL);
#else
    int nRet = CyaSSL_connect_ex(pxCTX->pxSSL, handShakeCB, timeoutCB, timeout);
#endif
    int nError = CyaSSL_get_error(pxCTX->pxSSL, 0);

    while (nRet != SSL_SUCCESS && 
           (nError == SSL_ERROR_WANT_READ || nError == SSL_ERROR_WANT_WRITE)) 
    {
        uint_32 ulSelectRet;

        ulSelectRet = RTCS_selectset(&nSocketID, 1, 2000);
        if (ulSelectRet == RTCS_SOCKET_ERROR) 
        {
#ifndef CYASSL_CALLBACKS
            nRet = CyaSSL_connect(pxCTX->pxSSL);
#else
            nRet = CyaSSL_connect_ex(pxCTX->pxSSL,handShakeCB,timeoutCB,timeout);
#endif
            nError = CyaSSL_get_error(pxCTX->pxSSL, 0);
        }
        else if (ulSelectRet == 0) 
        {
            nError = SSL_ERROR_WANT_READ;
        }
        else 
        {
            nError = SSL_FATAL_ERROR;
        }
    }
    
    if (nRet != SSL_SUCCESS)
    {
        FTE_SSL_ERR("SSL connection failed.");
        CyaSSL_shutdown(pxCTX->pxSSL);
        
        return  RTCS_ERROR;
    }
        
    return  RTCS_OK;
}

FTE_RET   FTE_SSL_disconnect
(
    FTE_SSL_CONTEXT_PTR pxCTX
)
{
    ASSERT(pxCTX != NULL);
    
    CyaSSL_shutdown(pxCTX->pxSSL);
    
    return  RTCS_OK;
}

FTE_INT32   FTE_SSL_send
(
    FTE_SSL_CONTEXT_PTR pxCTX, 
    const FTE_VOID_PTR  pMsg, 
    FTE_INT32           nMsgLen
)
{
    return  CyaSSL_write(pxCTX->pxSSL, pMsg, nMsgLen);
}

FTE_INT32   FTE_SSL_recv
(
    FTE_SSL_CONTEXT_PTR     pxCTX, 
    FTE_VOID_PTR            pBuff, 
    FTE_INT32               nBuffLen
)
{
    return  CyaSSL_read(pxCTX->pxSSL, pBuff, nBuffLen);
}

FTE_INT32   ValidateDate
(
    const FTE_UINT8_PTR pDate, 
    FTE_UINT8           ubFormat, 
    FTE_INT32           nDateType
)
{
    return  1;
}

