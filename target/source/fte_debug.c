#include "fte_target.h"
#include "fte_time.h"

#if FTE_DEBUG

static char     _pBuff[1024];
static uint_32  _ulTraceModule = 0;

static MQX_FILE_PTR _debugOut = 0;

_mqx_int    FTE_DEBUG_setHandle(void)
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

_mqx_int    FTE_DEBUG_trace(uint_32  ulModule, const char _PTR_ fmt_ptr, ... )
{
    if ((_ulTraceModule & ulModule) == ulModule)
    {
       va_list      ap;
       uint_32      ulLen = 0;
       TIME_STRUCT  xTime;
       char         pTimeBuff[64];

        _time_get(&xTime);
        FTE_TIME_toString(&xTime, pTimeBuff, sizeof(pTimeBuff));   
        ulLen = sprintf(&_pBuff[ulLen], "[%s] ", pTimeBuff);
        
        va_start(ap, fmt_ptr);
        _io_vsnprintf(&_pBuff[ulLen], sizeof(_pBuff) - ulLen,  (char _PTR_)fmt_ptr, ap );
        va_end(ap);
       
       return   fprintf(_debugOut, "%s", _pBuff);
    }
    
    return  0;
}

_mqx_int    FTE_DEBUG_traceOn(uint_32 ulModule)
{
    _ulTraceModule |= ulModule;
    
    return  0;
}

_mqx_int    FTE_DEBUG_traceOff(uint_32 ulModule)
{
    _ulTraceModule &= ~ulModule;
    
    return  0;
}

_mqx_int    FTE_DEBUG_error(const char _PTR_ pFuncName, int nLine, const char _PTR_ fmt_ptr, ... )
{
   va_list      ap;
   uint_32      ulLen = 0;
   TIME_STRUCT  xTime;
   char         pTimeBuff[64];
   
    _time_get(&xTime);
    FTE_TIME_toString(&xTime, pTimeBuff, sizeof(pTimeBuff));   
    ulLen = sprintf(&_pBuff[ulLen], "[%s] ", pTimeBuff);
    
    va_start(ap, fmt_ptr);
    _io_vsnprintf(&_pBuff[ulLen], sizeof(_pBuff) - ulLen,  (char _PTR_)fmt_ptr, ap );
    va_end(ap);
   
    return   fprintf(_debugOut, "%s[%d] : %s\n", pFuncName, nLine, _pBuff);
}

typedef struct
{
    char_ptr    pName;
    uint_32     ulType;
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
    {   .pName = "all",     .ulType = DEBUG_ALL},
    {   .pName = NULL,      .ulType = 0}
};

int_32 FTE_TRACE_SHELL_cmd(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
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
                   
                    uint_32 ulType = DEBUG_UNKNOWN;
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
                uint_32 ulType = DEBUG_UNKNOWN;
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