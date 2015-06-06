#include "fte_target.h"
#include "fte_log.h"
#include "fte_time.h"
#include "fte_value.h"
 
static  uint_32         _nLogHead = 0;
static  uint_32         _nLogTail = 0;
static  uint_32         _nLogs = 0;
static  uint_32         _nMaxLogs = 0;
static  FTE_LOG_PTR     _pLogs = NULL;

_mqx_uint   FTE_LOG_init(uint_32 nCount)
{
    _pLogs = (FTE_LOG_PTR)FTE_MEM_allocZero(sizeof(FTE_LOG) * nCount);
    if (_pLogs == NULL)
    {
        return  MQX_ERROR;
    }    
    
    _nMaxLogs = nCount;
    
    return  MQX_OK;
}

_mqx_uint   FTE_LOG_add(FTE_OBJECT_ID   nOID, FTE_VALUE_PTR pValue)
{
    ASSERT(pValue != NULL);
    
    if (_nMaxLogs <= 0)
    {
        return  MQX_ERROR;
    }
    
    _pLogs[_nLogTail].nID     = nOID;
    FTE_VALUE_copy(&_pLogs[_nLogTail].xValue, pValue);
    _nLogTail = (_nLogTail + 1) % _nMaxLogs;
    
    if (++_nLogs >= _nMaxLogs)
    {
        _nLogHead = (_nLogHead + 1) % _nMaxLogs;
        _nLogs = _nMaxLogs - 1;
    }
    
    return  MQX_OK;
}

int         fte_log_count(void)
{
    return  _nLogs;
}

FTE_LOG_PTR FTE_LOG_getAt(uint_32 nIndex)
{
    if ((_nLogs <= 0) || (nIndex >= _nLogs))
    {
        return  NULL;
    }
        
    nIndex = (_nLogHead + nIndex) % _nMaxLogs;    
    return  &_pLogs[nIndex];
}

int_32          FTE_LOG_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
                int_32 i;
                
                for(i =  _nLogs - 1 ; i >= 0; i--)
                {
                    char    pValueString[32];
                    
                    uint_32 index = (_nLogHead + i) % _nMaxLogs;
                
                    printf("%4d : %s %08x %s\n", 
                           _nLogs - i, 
                           FTE_VALUE_printTimeStamp(&_pLogs[index].xValue), 
                           _pLogs[index].nID, 
                           FTE_VALUE_toString(&_pLogs[index].xValue, pValueString, sizeof(pValueString)));
                }
                
                
            }
            break;
            
        default:
            print_usage = TRUE;
        }
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<cmd>]\n", argv[0]);
        }
        else
        {
            printf ("%s [<cmd>]\n", argv[0]);
        }
    }
    return   return_code;
}
