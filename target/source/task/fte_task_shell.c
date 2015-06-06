#include "fte_target.h"
#include "fte_shell.h"


/*TASK*-----------------------------------------------------------------
*
* Function Name  : FTE_TASK_shell
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/
void FTE_TASK_shell(uint_32 datas)
{
    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    while(1)
    {
        FTE_SHELL_proc();
    }
}

