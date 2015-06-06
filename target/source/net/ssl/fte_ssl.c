#include "fte_target.h"
#include "fte_config.h"
#include "fte_net.h"
#include "sys/fte_sys.h"
#include <tfs.h>
#include <ipcfg.h>
#include <cyassl/ssl.h>

#define HAVE_ANON   1

#define FTE_SSL_ERROR(x)    DEBUG(x)

#define CLIENT_DEFAULT_VERSION 3
#define CLIENT_DTLS_DEFAULT_VERSION (-2)
#define CLIENT_INVALID_VERSION (-99)

static void         FTE_SSL_NonBlockingConnect(CYASSL* ssl);
static unsigned int FTE_SSL_PSK_CB_client(CYASSL *pSSL, const char *pHint, char *pIdentity, unsigned int ulIDMaxLen, unsigned char *pKey, unsigned int ulKeyMaxLen);
const char ourCert[] = "-----BEGIN CERTIFICATE-----\n"
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
char ourKey[] = "-----BEGIN ENCRYPTED PRIVATE KEY-----\n"
"MIICxjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQIV/6+IPtS4AsCAggA\n"
"MBQGCCqGSIb3DQMHBAi9m30XmnsQHwSCAoAUN0O6oGp0DzDxfwAy3s7c39GN9/FP\n"
"gFhfmQSnRFJ8LsMfmDyP99JPC4xZJc247wn07FknmAUTmppsXao3QBUkLTIie4Hv\n"
"rig67N3SsMT3uOC01lqc1S5wXfnO7xQLHh/O+dF5qTMRY8tE0e8ueUurEBO4vfRx\n"
"XF/RGSo8bn6g3h2l081sIBCku5R+4r2PWnq9Aji60NXV/UFZQWpCOpyT3ooADT9T\n"
"Y2PuzNofr87v7CXANoEL2T3IT9VZAPheiE1nJ1gxZeVWTxW0BRB3frVt+sR9ZlAz\n"
"opyMgyFI9xaB2UI/k/WQyCbdr4fbwz+p0yxVDjG+WZsGe6kqHaQftVoKLAhe96Co\n"
"ITBvVpgvXlp/j20UdCpAcJLua7gbth47msDq23jTjI30nz5Pdop7PIMXV1FwTP+i\n"
"hl5Mfsstr7OH7Y5yvpH2eEcukvzQ9vBbTxoW7eM0VL6nIiEFAtxKRSWnymRpuCFo\n"
"A1wNo5E1b/H1sEU+4SnBO7UXKftUNDLgrLh55oIbAoMHOqkn10NC0JwS1nH1F1Pr\n"
"N52e79jY04UTL7M+wzYaY+r/yHuy6klzsd3ES+1ORQmlrpFozWzSnXqR/7QpMlcq\n"
"hFms/7/7HGc7IE32cIfn/JTdqsLvIpxB58OCNhkHLFQ3yjrzc6AxoOVcFLNFLErD\n"
"u30+bvWOjqJDgKz7rfUsUilb05pGoAC96k3f20V7hHTjbg8hT4VXVK6FuBNtqKNb\n"
"LXLR2PonbiFjxH7dQVr7TefyfPD7WuCCqdpD5H2y8b4Fs9YQMJkTasN/bzZlqKO+\n"
"OW/K+lXdEEf4sGxmMtvMMxG5QCARt7nPyE9tiC0gEShzJj2NEBYv8Gmb\n"
"-----END ENCRYPTED PRIVATE KEY-----\n";


int_32 FTE_SSL_init(void)
{
    CYASSL_METHOD*  method  = 0;
    CYASSL_CTX*     ctx     = 0;
    CYASSL*         ssl     = 0;
   
    int     version = 3;//CLIENT_INVALID_VERSION;
    int     doDTLS    = 0;
    char*   cipherList = NULL;
    int     fewerPackets  = 0;
    int     usePsk = 0;
    int    useClientCert = 1;
    int    useAnon  = 0;
    int    atomicUser    = 0;
    int    pkCallbacks   = 0;
    
    CyaSSL_Init();

#if defined(DEBUG_CYASSL)
    CyaSSL_Debugging_ON();
#endif

   if (version == CLIENT_INVALID_VERSION) 
   {
        if (doDTLS)
        {
            version = CLIENT_DTLS_DEFAULT_VERSION;
        }
        else
        {
            version = CLIENT_DEFAULT_VERSION;
        }
    }
    else 
    {
        if (doDTLS) 
        {
            if (version == 3)
            {
                version = -2;
            }
            else
            {
                version = -1;
            }
        }
    }
        
    switch (version) 
    {
#ifndef NO_OLD_TLS
    case 0:
        method = CyaSSLv3_client_method();
        break;
            
            
#ifndef NO_TLS
    case 1:
        method = CyaTLSv1_client_method();
        break;

    case 2:
        method = CyaTLSv1_1_client_method();
        break;
#endif /* NO_TLS */
            
#endif  /* NO_OLD_TLS */
            
#ifndef NO_TLS
    case 3:
        method = CyaTLSv1_2_client_method();
        break;
#endif

#ifdef CYASSL_DTLS
    case -1:
        method = CyaDTLSv1_client_method();
        break;

    case -2:
        method = CyaDTLSv1_2_client_method();
        break;
#endif

    default:
        FTE_SSL_ERROR("Bad SSL version");
        goto error;
    }
   
    if (method == NULL)
    {
        FTE_SSL_ERROR("unable to get method");
    }
            
   // CyaSSL_Debugging_ON();

    ctx = CyaSSL_CTX_new(method);
    if (ctx == NULL)
    {
        FTE_SSL_ERROR("unable to get ctx");
    }

    if (cipherList)
        if (CyaSSL_CTX_set_cipher_list(ctx, cipherList) != SSL_SUCCESS)
        {
            FTE_SSL_ERROR("client can't set cipher list 1");
        }

    if (fewerPackets)
    {
        CyaSSL_CTX_set_group_messages(ctx);
    }

    if (usePsk) 
    {
#ifndef NO_PSK
        CyaSSL_CTX_set_psk_client_callback(ctx, FTE_SSL_PSK_CB_client);
        if (cipherList == NULL) 
        {
            const char *defaultCipherList;
            #ifdef HAVE_NULL_CIPHER
                defaultCipherList = "PSK-NULL-SHA256";
            #else
                defaultCipherList = "PSK-AES128-CBC-SHA256";
            #endif
            if (CyaSSL_CTX_set_cipher_list(ctx,defaultCipherList) !=SSL_SUCCESS)
                err_sys("client can't set cipher list 2");
        }
#endif
        //useClientCert = 0;
    }

    if (useAnon) 
    {
#ifdef HAVE_ANON
        if (cipherList == NULL) 
        {
            CyaSSL_CTX_allow_anon_cipher(ctx);
            if (CyaSSL_CTX_set_cipher_list(ctx,"AES256-SHA") != SSL_SUCCESS)
                err_sys("client can't set cipher list 4");
        }
#endif
        //useClientCert = 0;
    }
    
    
#ifdef USER_CA_CB
    CyaSSL_CTX_SetCACb(ctx, CaCb);
#endif

#ifdef VERIFY_CALLBACK
    CyaSSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, myVerify);
#endif
    
    
    if (useClientCert)
    {
        if (CyaSSL_CTX_use_certificate_buffer(ctx, ourCert, sizeof(ourCert),SSL_FILETYPE_PEM) != SSL_SUCCESS)
        {
            err_sys("can't load client cert file, check file and run from"
                    " CyaSSL home dir");
        }

        if (CyaSSL_CTX_use_PrivateKey_buffer(ctx, ourKey, sizeof(ourKey), SSL_FILETYPE_PEM)
                                         != SSL_SUCCESS)
        {
            err_sys("can't load client private key file, check file and run "
                    "from CyaSSL home dir");
        }
    }

    if (!usePsk && !useAnon) 
    {
        if (CyaSSL_CTX_load_verify_buffer(ctx, ourCert, sizeof(ourCert),SSL_FILETYPE_PEM) != SSL_SUCCESS)
        {
                err_sys("can't load ca file, Please run from CyaSSL home dir");
        }
    }
   
    
    {
        int nSocketID = 0;
        struct sockaddr_in socket_address;
        uint_32 ulRet;

        //double start = current_time();
        nSocketID = socket(PF_INET, SOCK_STREAM, 0);

        socket_address.sin_family       = AF_INET;
        socket_address.sin_addr.s_addr  = IPADDR(10,0,1,18);
        socket_address.sin_port         = 8883;

        // Connect the socket
        ulRet = connect(nSocketID, (struct sockaddr*)&socket_address, sizeof(socket_address));
        if((ulRet != RTCS_OK) && (ulRet != RTCSERR_TCP_CONN_RLSD))
        {
            printf("Connection error!\n");
            return ulRet;
        }

        ssl = CyaSSL_new(ctx);
        CyaSSL_set_fd(ssl, nSocketID);
        
        
#ifdef ATOMIC_USER
    if (atomicUser)
        SetupAtomicUser(ctx, ssl);
#endif
#ifdef HAVE_PK_CALLBACKS
    if (pkCallbacks)
        SetupPkCallbacks(ctx, ssl);
#endif
        
        CyaSSL_set_using_nonblock(ssl, 1);
        FTE_SSL_NonBlockingConnect(ssl);
        printf("SSL connect ok, sending GET...\n");
    }
        
    return  RTCS_OK;
    
error:
    return  RTCS_ERROR;
}

static void FTE_SSL_NonBlockingConnect(CYASSL* ssl)
{
#ifndef CYASSL_CALLBACKS
    int ret = CyaSSL_connect(ssl);
#else
    int ret = CyaSSL_connect_ex(ssl, handShakeCB, timeoutCB, timeout);
#endif
    int error = CyaSSL_get_error(ssl, 0);
    int sockfd = (int)CyaSSL_get_fd(ssl);
    uint_32 select_ret;

    while (ret != SSL_SUCCESS && (error == SSL_ERROR_WANT_READ ||
                                  error == SSL_ERROR_WANT_WRITE)) {
        int currTimeout = 1;

        if (error == SSL_ERROR_WANT_READ)
            printf("... client would read block\n");
        else
            printf("... client would write block\n");

#ifdef CYASSL_DTLS
        currTimeout = CyaSSL_dtls_get_current_timeout(ssl);
#endif
        select_ret = RTCS_selectset(&sockfd, 1, currTimeout * 1000);

        if (select_ret == RTCS_SOCKET_ERROR) 
        {
            #ifndef CYASSL_CALLBACKS
                    ret = CyaSSL_connect(ssl);
            #else
                ret = CyaSSL_connect_ex(ssl,handShakeCB,timeoutCB,timeout);
            #endif
            error = CyaSSL_get_error(ssl, 0);
        }
        else if (select_ret == 0) 
        {
            error = SSL_ERROR_WANT_READ;
        }
        else {
            error = SSL_FATAL_ERROR;
        }
    }
    if (ret != SSL_SUCCESS)
        printf("SSL_connect failed");
}

unsigned int FTE_SSL_PSK_CB_client
(
    CYASSL          *pSSL, 
    const char      *pHint,
    char            *pIdentity, 
    unsigned int    ulIDMaxLen, 
    unsigned char   *pKey,
    unsigned int    ulKeyMaxLen
)
{
    (void)pSSL;
    (void)pHint;
    (void)ulKeyMaxLen;

    /* identity is OpenSSL testing default for openssl s_client, keep same */
    strncpy(pIdentity, "Client_identity", ulIDMaxLen);


    /* test key in hex is 0x1a2b3c4d , in decimal 439,041,101 , we're using
       unsigned binary */
    pKey[0] = 26;
    pKey[1] = 43;
    pKey[2] = 60;
    pKey[3] = 77;

    return 4;   /* length of key in octets or 0 for error */
}

int_32 FTE_SSL_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] )
{
    boolean                 bPrintUsage, bShortHelp = FALSE;
    int_32                  nRet = SHELL_EXIT_SUCCESS;
    uint_32                 ulEnetDevice = BSP_DEFAULT_ENET_DEVICE;
    FTE_NET_CFG             xNetCfg;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            FTE_SSL_init();
        }
        break;
    }
    
        {
            extern  FTE_LIST    _taskList;

            uint_32             ulCount = 0;
            FTE_LIST_ITERATOR   xIter;
            FTE_TASK_INFO_PTR   pTaskInfo;
            
            FTE_LIST_ITER_init(&_taskList, &xIter);
            while((pTaskInfo = (FTE_TASK_INFO_PTR)FTE_LIST_ITER_getNext(&xIter)) != MQX_NULL_TASK_ID)
            {
                TASK_TEMPLATE_STRUCT_PTR pTemplate = _task_get_template_ptr(pTaskInfo->xID);
                if (pTemplate != NULL)
                {
                    printf("%2d : %8d %32s %4d\n", ++ulCount, pTaskInfo->xID, pTemplate->TASK_NAME, _task_free_stack(pTaskInfo->xID));
                }
            }
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

