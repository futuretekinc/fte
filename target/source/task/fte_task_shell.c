#include "fte_target.h"
#include "fte_shell.h"


/*TASK*-----------------------------------------------------------------
*
* Function Name  : FTE_TASK_shell
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
void FTE_TASK_shell(FTE_UINT32 datas)
{
    while(TRUE)
    {
        FTE_SHELL_proc();
    }
}

