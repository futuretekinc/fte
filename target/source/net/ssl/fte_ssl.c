#include "fte_target.h"
#include "fte_config.h"
#include "fte_net.h"
#include "sys/fte_sys.h"
#include <tfs.h>
#include <ipcfg.h>
#include <cyassl/ssl.h>

#define FTE_SSL_ERROR(...)    DEBUG(__VA_ARGS__)

#define CLIENT_DEFAULT_VERSION 3

static int      FTE_SSL_nonBlockingConnect(CYASSL* pxSSL);
static void     FTE_SSL_showPeer(CYASSL* pxSSL);

const uint_8 pCACert[] = "-----BEGIN CERTIFICATE-----\n"
"MIICjjCCAfegAwIBAgIJANLaAegLuRIrMA0GCSqGSIb3DQEBBQUAMGAxCzAJBgNV\n"
"BAYTAktSMRMwEQYDVQQIDApTb21lLVN0YXRlMRYwFAYDVQQKDA1GdXR1cmVUZWss\n"
"SW5jMREwDwYDVQQLDAhyZXNlYXJjaDERMA8GA1UEAwwIZnRtLTEwMHMwHhcNMTUw\n"
"NTA4MDYwNzMyWhcNMjUwNTA1MDYwNzMyWjBgMQswCQYDVQQGEwJLUjETMBEGA1UE\n"
"CAwKU29tZS1TdGF0ZTEWMBQGA1UECgwNRnV0dXJlVGVrLEluYzERMA8GA1UECwwI\n"
"cmVzZWFyY2gxETAPBgNVBAMMCGZ0bS0xMDBzMIGfMA0GCSqGSIb3DQEBAQUAA4GN\n"
"ADCBiQKBgQC83ucjnI+l5s+fQmzE8NqBMC7sFeQdfd+D8y8v/YV8Tywe0XNnRvDd\n"
"pIIW40gB8AQbZzWN5tgOB5fSYAzcCmGSo3lyFRTnw84wOMfyoNY/YWXVYpkAHPXr\n"
"0lRv9gx9bDgpCgk424M/lbpLEI8rf8n0iFGzAzNbjCRxS4pWnUSIDwIDAQABo1Aw\n"
"TjAdBgNVHQ4EFgQUE6RiCyjMQMMET/QbTqDY1sjQ9b8wHwYDVR0jBBgwFoAUE6Ri\n"
"CyjMQMMET/QbTqDY1sjQ9b8wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOB\n"
"gQAvjA1vqSYSOErxOx8/agZOV9bkAxvNc3WDYQgkJ5mpapFZvtrcnB6l+Lb3CM6O\n"
"ynfYkZPxQSqlmR2L5dFaufFrGevIzGMD90IxvucXkf6qQ2BRkpdzU71h987XnsB/\n"
"oQN0vrIqEADvDDxOXRAR+QczMg3ZlIUU/N7ppigxSWnbvQ==\n"
"-----END CERTIFICATE-----\n";

static void FTE_SSL_showPeer(CYASSL* pxSSL)
{
    CYASSL_CIPHER* cipher;

    printf("SSL version is %s\n", CyaSSL_get_version(pxSSL));

    cipher = CyaSSL_get_current_cipher(pxSSL);
    printf("SSL cipher suite is %s\n", CyaSSL_CIPHER_get_name(cipher));

  (void)pxSSL;
}

FTE_SSL_CONTEXT_PTR FTE_SSL_init(int nVersion)
{
    FTE_SSL_CONTEXT_PTR     pxCTX   = NULL;
    CYASSL_METHOD*          pxMETHOD= NULL;
    CYASSL_CTX*             pxSSLCTX= NULL;
    
    pxCTX = (FTE_SSL_CONTEXT_PTR)FTE_MEM_allocZero(sizeof(FTE_SSL_CONTEXT));
    if (pxCTX == NULL)
    {
        goto error;
    }
    
    CyaSSL_Init();

#if FTE_SSL_DEBUG
    CyaSSL_Debugging_ON();
#endif
    
    switch (nVersion) 
    {
    case 0: pxMETHOD = CyaSSLv3_client_method();      break;
    case 1: pxMETHOD = CyaTLSv1_client_method();      break;
    case 2: pxMETHOD = CyaTLSv1_1_client_method();    break;
    case 3: pxMETHOD = CyaTLSv1_2_client_method();    break;
    default:
        FTE_SSL_ERROR("Bad SSL version");
        goto error;
    }
   
    pxSSLCTX = CyaSSL_CTX_new(pxMETHOD);
    if (pxSSLCTX == NULL)
    {
        FTE_SSL_ERROR("unable to get CTX");
        goto error;
    }
   
    if (CyaSSL_CTX_load_verify_buffer(pxSSLCTX, pCACert, sizeof(pCACert), SSL_FILETYPE_PEM) != SSL_SUCCESS)
    {
        FTE_SSL_ERROR("can't load buffer ca file");
        goto error;
    }
    
    CyaSSL_CTX_set_cipher_list(pxSSLCTX, "AES256-SHA");


    pxCTX->pxSSL = CyaSSL_new(pxSSLCTX);
    if (pxCTX->pxSSL == NULL)
    {        
        goto error;
    }
    
    return  pxCTX;
    
error:
    if (pxCTX != NULL)
    {
        if (pxCTX->pxSSL != NULL)
        {
            CyaSSL_free(pxCTX->pxSSL);
            pxCTX->pxSSL = NULL;
        }
        
        FTE_MEM_free(pxCTX);
    }
    
    if (pxSSLCTX != NULL)
    {
        CyaSSL_CTX_free(pxSSLCTX);
    }
    return  NULL;
}

_mqx_uint   FTE_SSL_connect(FTE_SSL_CONTEXT_PTR pCTX, int nSocketID)
{
    CyaSSL_set_fd(pCTX->pxSSL, nSocketID);
        
    CyaSSL_set_using_nonblock(pCTX->pxSSL, 1);
    FTE_SSL_nonBlockingConnect(pCTX->pxSSL);
        
    return  RTCS_OK;
}

int FTE_SSL_send(FTE_SSL_CONTEXT_PTR pCTX, const void *pMsg, int nMsgLen)
{
    return  CyaSSL_write(pCTX->pxSSL, pMsg, nMsgLen);
}

int FTE_SSL_recv(FTE_SSL_CONTEXT_PTR pCTX, void *pBuff, int nBuffLen)
{
    return  CyaSSL_read(pCTX->pxSSL, pBuff, nBuffLen);
}

static int FTE_SSL_nonBlockingConnect(CYASSL* pxSSL)
{
#ifndef CYASSL_CALLBACKS
    int ret = CyaSSL_connect(pxSSL);
#else
    int ret = CyaSSL_connect_ex(pxSSL, handShakeCB, timeoutCB, timeout);
#endif
    int error = CyaSSL_get_error(pxSSL, 0);
    int sockfd = (int)CyaSSL_get_fd(pxSSL);
    uint_32 select_ret;

    while (ret != SSL_SUCCESS && 
           (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)) 
    {
        int currTimeout = 1;

        if (error == SSL_ERROR_WANT_READ)
        {
            printf("... client would read block\n");
        }
        else
        {
            printf("... client would write block\n");
        }

        select_ret = RTCS_selectset(&sockfd, 1, currTimeout * 1000);
        if (select_ret == RTCS_SOCKET_ERROR) 
        {
#ifndef CYASSL_CALLBACKS
            ret = CyaSSL_connect(pxSSL);
#else
            ret = CyaSSL_connect_ex(pxSSL,handShakeCB,timeoutCB,timeout);
#endif
            error = CyaSSL_get_error(pxSSL, 0);
        }
        else if (select_ret == 0) 
        {
            error = SSL_ERROR_WANT_READ;
        }
        else 
        {
            error = SSL_FATAL_ERROR;
        }
    }
    if (ret != SSL_SUCCESS)
        printf("SSL_connect failed");
    
    return  ret;
}

int_32 FTE_SSL_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] )
{
    boolean                 bPrintUsage, bShortHelp = FALSE;
    int_32                  nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            int nSocketID;
            struct sockaddr_in socket_address;
            
            nSocketID = socket(PF_INET, SOCK_STREAM, 0);

            socket_address.sin_family       = AF_INET;
            socket_address.sin_addr.s_addr  = IPADDR(10,0,1,18);
            socket_address.sin_port         = 8883;

            // Connect the socket
            nRet = connect(nSocketID, (struct sockaddr*)&socket_address, sizeof(socket_address));
            if((nRet != RTCS_OK) && (nRet != RTCSERR_TCP_CONN_RLSD))
            {
                FTE_SSL_ERROR("Connection error!\n");
                goto error;
            }

            FTE_SSL_init(nSocketID);
        }
        break;
    }

error:
    if (bPrintUsage)  
    {
        if (bShortHelp)  
        {
            printf("%s <cmd>\n", pArgv[0]);
        } 
        else  
        {
            printf("Usage: %s <cmd>\n",pArgv[0]);
        }
    }
    
    return  nRet;
}

int ValidateDate(const uint_8* date, uint_8 format, int dateType)
{
    return  1;
}

