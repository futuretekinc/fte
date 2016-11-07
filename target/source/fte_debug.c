#include "fte_target.h"
#include "fte_time.h"

#if FTE_DEBUG
 
static char     _pBuff[1024];
static FTE_UINT32  _ulTraceModule = 0;

static MQX_FILE_PTR _debugOut = 0;

FTE_RET    FTE_DEBUG_setHandle(void)
{
    pointer   hStdout;
    
    hStdout = _io_get_handle(IO_STDOUT);
    if (hStdout != 0)
    {
        _debugOut = hStdout;
    }

    return  MQX_OK;
}

void    FTE_DEBUG_init(void)
{
    _debugOut = stdout;
}

FTE_RET    FTE_DEBUG_trace
(
    FTE_UINT32      ulModule, 
    const char _PTR_ fmt_ptr, 
    ... 
)
{
    if ((_ulTraceModule & ulModule) == ulModule)
    {
       va_list      ap;
       FTE_UINT32      ulLen = 0;
#if 0
        TIME_STRUCT  xTime;
       char         pTimeBuff[64];
       
        _time_get(&xTime);
        FTE_TIME_toStr(&xTime, pTimeBuff, sizeof(pTimeBuff));   
        ulLen = sprintf(&_pBuff[ulLen], "[%s] ", pTimeBuff);
#endif   
        va_start(ap, fmt_ptr);
        _io_vsnprintf(&_pBuff[ulLen], sizeof(_pBuff) - ulLen,  (char _PTR_)fmt_ptr, ap );
        va_end(ap);
       
       return   fprintf(_debugOut, "%s", _pBuff);
    }
    
    return  0;
}

FTE_RET    FTE_DEBUG_traceOn
(
    FTE_UINT32  ulModule
)
{
    _ulTraceModule |= ulModule;
    
    return  0;
}

FTE_RET    FTE_DEBUG_traceOff
(
    FTE_UINT32  ulModule
)
{
    _ulTraceModule &= ~ulModule;
    
    return  0;
}

FTE_BOOL FTE_DEBUG_isTraceOn
(
    FTE_UINT32  ulModule
)
{
    return  (_ulTraceModule & ulModule) == ulModule;
}

FTE_RET    FTE_DEBUG_error
(
    const char _PTR_ pFuncName, 
    FTE_INT32   nLine, 
    const char _PTR_ fmt_ptr, 
    ... 
)
{
   va_list      ap;
   FTE_UINT32      ulLen = 0;
   TIME_STRUCT  xTime;
   char         pTimeBuff[64];
   
    _time_get(&xTime);
    FTE_TIME_toStr(&xTime, pTimeBuff, sizeof(pTimeBuff));   
    ulLen = sprintf(&_pBuff[ulLen], "[%s] ", pTimeBuff);
    
    va_start(ap, fmt_ptr);
    _io_vsnprintf(&_pBuff[ulLen], sizeof(_pBuff) - ulLen,  (char _PTR_)fmt_ptr, ap );
    va_end(ap);
   
    return   fprintf(_debugOut, "%s[%d] : %s\n", pFuncName, nLine, _pBuff);
}

FTE_RET    FTE_DEBUG_dump
(
    const char _PTR_ pFuncName, 
    FTE_INT32       nLine, 
    FTE_CHAR_PTR    pTitle, 
    FTE_VOID_PTR    pBuff, 
    FTE_UINT32      ulSize, 
    FTE_UINT32      ulWrapLen
)
{
    FTE_UINT32  i;
    FTE_CHAR    pIndent[32];
    
    if (pTitle != NULL)
    {
        fprintf(_debugOut, "%s : ", pTitle);
        memset(pIndent, ' ', strlen(pTitle) + 3);
        pIndent[strlen(pTitle) + 3] = 0;
    }
    else
    {
        pIndent[0] = 0;
    }
    
    for(i = 0 ; i < ulSize ; i++)
    {
        fprintf(_debugOut, "%02x ", ((uint_8_ptr)pBuff)[i]);
        if ((ulWrapLen != 0) && ((i+1) % ulWrapLen == 0))
        {
            fprintf(_debugOut, "\n");
            if ((i+1) != ulSize)
            {
                fprintf(_debugOut, "%s", pIndent);
            }
        }
    }
    
    if ((ulWrapLen == 0) || ((i+1) % ulWrapLen != 0))
    {
        fprintf(_debugOut, "\n");
    }
    
    return  MQX_OK;
}

typedef struct
{
    FTE_CHAR_PTR    pName;
    FTE_UINT32      ulType;
}   FTE_DEBUG_MODULE_TYPE_NAME, _PTR_ FTE_DEBUG_MODULE_TYPE_NAME_PTR;

static FTE_DEBUG_MODULE_TYPE_NAME  _pModuleTypeName[] =
{
    {   .pName = "object",  .ulType = DEBUG_OBJECT  },
    {   .pName = "event",   .ulType = DEBUG_EVENT   },
    {   .pName = "mqtt",    .ulType = DEBUG_NET_MQTT},
    {   .pName = "cgi",     .ulType = DEBUG_NET_CGI },
    {   .pName = "smng",    .ulType = DEBUG_NET_SMNG},
    {   .pName = "snmp",    .ulType = DEBUG_NET_SNMP},
    {   .pName = "ssl",     .ulType = DEBUG_NET_SSL },
    {   .pName = "di",      .ulType = DEBUG_DI },
    {   .pName = "all",     .ulType = DEBUG_ALL},
    {   .pName = NULL,      .ulType = 0}
};

FTE_INT32 FTE_TRACE_SHELL_cmd(FTE_INT32 nArgc, char_ptr pArgv[])
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_DEBUG_MODULE_TYPE_NAME_PTR  pTypeName = _pModuleTypeName;
                while(pTypeName->pName != NULL)
                {
                    if ((_ulTraceModule & pTypeName->ulType) == pTypeName->ulType)
                    {
                        printf("%8s : on\n", pTypeName->pName);
                    }
                    else
                    {
                        printf("%8s : off\n", pTypeName->pName);
                    }
                    
                    pTypeName++;
                }
            }
            break;
            
        case    2:
            {
                if (strcmp(pArgv[1], "out") == 0)
                {
                    FTE_DEBUG_setHandle();
                }
                else
                {
                   
                    FTE_UINT32 ulType = DEBUG_UNKNOWN;
                    FTE_DEBUG_MODULE_TYPE_NAME_PTR  pTypeName = _pModuleTypeName;
                    while(pTypeName->pName != NULL)
                    {
                        if (strcmp(pArgv[1], pTypeName->pName) == 0)
                        {
                            ulType = pTypeName->ulType;
                            break;
                        }
                        pTypeName++;
                    }

                    if (ulType == DEBUG_UNKNOWN)
                    {
                        bPrintUsage = TRUE;
                        break;
                    }
                    
                    if ((_ulTraceModule & ulType) == ulType)
                    {
                        printf("%8s : on\n", pArgv[1]);
                    }
                    else
                    {
                        printf("%8s : off\n", pArgv[1]);
                    }
                }
            }
            break;
                   
        case    3:
            {
                FTE_UINT32 ulType = DEBUG_UNKNOWN;
                FTE_DEBUG_MODULE_TYPE_NAME_PTR  pTypeName = _pModuleTypeName;
                while(pTypeName->pName != NULL)
                {
                    if (strcmp(pArgv[1], pTypeName->pName) == 0)
                    {
                        ulType = pTypeName->ulType;
                        break;
                    }
                    pTypeName++;
                }

                if (ulType == DEBUG_UNKNOWN)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                if(strcmp(pArgv[2], "on") == 0)
                {
                    FTE_DEBUG_traceOn(ulType);
                }
                else if(strcmp(pArgv[2], "off") == 0)
                {
                    FTE_DEBUG_traceOff(ulType);
                }
                else
                {
                    bPrintUsage = TRUE;
                }
            }
            break;
            
        default:
            bPrintUsage = TRUE;
        }
    }
                    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf("%s <cmd> ...\n", pArgv[0]);
        }
        else
        {
            printf ("Usage : %s <target> [ on | off ]\n", pArgv[0]);
        }
    }
    
    
    return  nReturnCode;
}

#endif