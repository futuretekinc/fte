#include "fte_target.h"
#include "fte_time.h"

#if FTE_DEBUG

static char     _pBuff[1024];
static uint_32  _ulTraceModule = 0;

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
       
       return   fprintf(stderr, "%s", _pBuff);
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
   
    return   fprintf(stderr, "%s[%d] : %s\n", pFuncName, nLine, _pBuff);
}


int_32 FTE_DEBUG_shellCmd(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    3:
            {
                if (strcmp(pArgv[0], "trace") == 0)
                {                    
                    if(strcmp(pArgv[2], "on") == 0)
                    {
                        if (strcmp(pArgv[1], "object") == 0)
                        {
                            FTE_DEBUG_traceOn(DEBUG_OBJECT);
                        }
                        else if (strcmp(pArgv[1], "event") == 0)
                        {
                            FTE_DEBUG_traceOn(DEBUG_EVENT);
                        }
                        else if (strcmp(pArgv[1], "mqtt") == 0)
                        {
                            FTE_DEBUG_traceOn(DEBUG_NET_MQTT);
                        }
                        else if (strcmp(pArgv[1], "cgi") == 0)
                        {
                            FTE_DEBUG_traceOn(DEBUG_NET_CGI);
                        }
                        else if (strcmp(pArgv[1], "all") == 0)
                        {
                            FTE_DEBUG_traceOn(DEBUG_ALL);
                        }
                        else
                        {
                            bPrintUsage = TRUE;
                        }
                    }
                    else if(strcmp(pArgv[2], "off") == 0)
                    {
                        if (strcmp(pArgv[1], "object") == 0)
                        {
                            FTE_DEBUG_traceOff(DEBUG_OBJECT);
                        }
                        else if (strcmp(pArgv[1], "event") == 0)
                        {
                            FTE_DEBUG_traceOff(DEBUG_EVENT);
                        }
                        else if (strcmp(pArgv[1], "mqtt") == 0)
                        {
                            FTE_DEBUG_traceOff(DEBUG_NET_MQTT);
                        }
                        else if (strcmp(pArgv[1], "cgi") == 0)
                        {
                            FTE_DEBUG_traceOff(DEBUG_NET_CGI);
                        }
                        else if (strcmp(pArgv[1], "all") == 0)
                        {
                            FTE_DEBUG_traceOff(DEBUG_ALL);
                        }
                        else 
                        {
                            bPrintUsage = TRUE;
                        }
                    }
                    else
                    {
                        bPrintUsage = TRUE;
                    }
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