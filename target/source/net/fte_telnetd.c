#include "fte_target.h"
#include "fte_net.h"
#include "fte_shell.h"

extern const SHELL_COMMAND_STRUCT pSHELLCommands[];

static 
void FTE_TELNETD_proc(FTE_VOID_PTR  pDummy);

FTE_RET   FTE_TELNETD_init
(   
    FTE_VOID_PTR pParams
)
{
    static  RTCS_TASK _TaskTemplate = 
    {   
        .NAME       = FTE_NET_TELNETD_NAME, 
        .PRIORITY   = FTE_NET_TELNETD_PRIO, 
        .STACKSIZE  = FTE_NET_TELNETD_STACK, 
        .START      = FTE_TELNETD_proc, 
        NULL
    };

    _TaskTemplate.ARG = (pointer)pSHELLCommands;
    
    uint_32 uiRet = TELNETSRV_init(FTE_NET_TELNETD_NAME, FTE_NET_TELNETD_PRIO, FTE_NET_TELNETD_STACK, (RTCS_TASK_PTR)&_TaskTemplate );
    if (uiRet !=  0)  
    {
        printf("Unable to start Telnet Server, error = 0x%x\n",uiRet);
        return  FTE_RET_ERROR;
    }
    FTE_TASK_append(FTE_TASK_TYPE_RTCS, _task_get_id_from_name(FTE_NET_TELNETD_NAME));
    
    printf("Telnet Server Started.\n");

    return  FTE_RET_OK;
}

void FTE_TELNETD_proc
(
    FTE_VOID_PTR    pShellCmds
) 
{  
    FTE_SHELL_proc();
}

