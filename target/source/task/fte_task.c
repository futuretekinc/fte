#include "fte_target.h"
#include "fte_task_main.h"
#include "fte_task_netd.h"
#include "fte_task_shell.h"
#include "fte_task_object_management.h"
#include "fte_task_timer.h"
#include "fte_task.h"
#include "fte_task_watchdog.h"
#include <rtcs.h>

/*
** MQX initialization information
*/

const TASK_TEMPLATE_STRUCT  MQX_template_list[] = 
{
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_MAIN,      /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_main,          /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_MAIN_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_MAIN_PRIO, /* Priority */
        .TASK_NAME              =   "main",             /* Name */
        .TASK_ATTRIBUTES        =   MQX_AUTO_START_TASK,/* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,         /* Param */
        .DEFAULT_TIME_SLICE     =   0                   /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_OBJECT_MNGT,       /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_objectManagement,  /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_OBJECT_MNGT_STACK, /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_OBJECT_MNGT_PRIO,  /* Priority */
        .TASK_NAME              =   "object management",        /* Name */
        .TASK_ATTRIBUTES        =   0,                          /* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,                 /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_WATCHDOG,      /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_watchdog,      /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_WATCHDOG_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_WATCHDOG_PRIO, /* Priority */
        .TASK_NAME              =   "watchdog",             /* Name */
        .TASK_ATTRIBUTES        =   MQX_AUTO_START_TASK,    /* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,             /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_TIMER,         /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_timer,         /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_TIMER_STACK,   /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_TIMER_PRIO,    /* Priority */
        .TASK_NAME              =   "timer",                /* Name */
        .TASK_ATTRIBUTES        =   MQX_AUTO_START_TASK,    /* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,             /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#if FTE_TASK_SHELL
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_SHELL,         /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_shell,         /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_SHELL_STACK,   /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_SHELL_PRIO,    /* Priority */
        .TASK_NAME              =   "SHELL",                /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,             /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if FTE_TASK_NET
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_NET,       /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_net,       /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_NET_STACK, /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_NET_PRIO,  /* Priority */
        .TASK_NAME              =   "net",              /* Name */
        .TASK_ATTRIBUTES        =   0,                  /* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,         /* Param */
        .DEFAULT_TIME_SLICE     =   0                   /* Time Slice */
    },
#endif
#if FTE_UCS_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_UCS_RX,    /* Task Index */
        .TASK_ADDRESS           =   FTE_UCS_TASK_recv,  /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_UCS_RX_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_UCS_RX_PRIO,/* Priority */
        .TASK_NAME              =   "ucs_rx",           /* Name */
        .TASK_ATTRIBUTES        =   0,                  /* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,         /* Param */
        .DEFAULT_TIME_SLICE     =   0                   /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_UCS_TX,    /* Task Index */
        .TASK_ADDRESS           =   FTE_UCS_TASK_send,  /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_UCS_TX_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_UCS_TX_PRIO,/* Priority */
        .TASK_NAME              =   "ucs_tx",           /* Name */
        .TASK_ATTRIBUTES        =   0,                  /* Attributes */
        .CREATION_PARAMETER     =   (uint_32)0,         /* Param */
        .DEFAULT_TIME_SLICE     =   0                   /* Time Slice */
    },
#endif
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_EVENT,         /* Task Index */
        .TASK_ADDRESS           =   FTE_EVENT_task,         /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_EVENT_STACK,   /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_EVENT_PRIO,    /* Priority */
        .TASK_NAME              =   "event",                /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },

    { 0 }
};

FTE_LIST    _taskList = 
{
    .nCount     = 0,
    .pHead      = NULL,
    .pLockKey   = NULL
};

_mqx_uint   FTE_TASK_create(_processor_number xProcessorID, _mqx_uint nTemplateIndex, uint_32 ulParameter)
{
    _task_id            xTaskID;
    FTE_TASK_INFO_PTR   pTaskInfo;
    
    xTaskID = _task_create(xProcessorID, nTemplateIndex, ulParameter);

    if (xTaskID == MQX_NULL_TASK_ID)
    {
        for(int i = 0  ; MQX_template_list[nTemplateIndex].TASK_TEMPLATE_INDEX != 0 ; i++)
        {
            if (MQX_template_list[nTemplateIndex].TASK_TEMPLATE_INDEX == nTemplateIndex)
            {
                ERROR("\nCould not create %s\n", MQX_template_list[nTemplateIndex].TASK_NAME);
                return  MQX_ERROR;
            }
        }
        
        ERROR("\nCould not create unknown[%d]\n", nTemplateIndex);
        return  MQX_ERROR;
    }
        
    pTaskInfo = (FTE_TASK_INFO_PTR)FTE_MEM_allocZero(sizeof(FTE_TASK_INFO));
    if (pTaskInfo != NULL)
    {
        pTaskInfo->xID  = xTaskID;
        pTaskInfo->xType= FTE_TASK_TYPE_MQX;
        
        FTE_LIST_pushBack(&_taskList, (pointer)pTaskInfo);    
    }
    
    return  MQX_OK;
}

_mqx_uint   FTE_TASK_append
(
    FTE_TASK_TYPE   xType,
    _task_id        xTaskID
)
{
    FTE_TASK_INFO_PTR   pTaskInfo;
        
    pTaskInfo = (FTE_TASK_INFO_PTR)FTE_MEM_allocZero(sizeof(FTE_TASK_INFO));
    if (pTaskInfo == NULL)
    {
        return  MQX_ERROR;
    }
    
    pTaskInfo->xID  = xTaskID;
    pTaskInfo->xType= xType;
        
    FTE_LIST_pushBack(&_taskList, (pointer)pTaskInfo);    
    
    return  MQX_OK;
}

int_32 FTE_TASK_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] )
{ 
    boolean                 bPrintUsage, bShortHelp = FALSE;
    int_32                  nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
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
//            TASK_TEMPLATE_STRUCT_PTR pTemplate = _task_get_template_ptr(65541);
//            printf("%2d : %8d %32s %4d\n", ++ulCount, pTaskInfo->xID, pTemplate->TASK_NAME, _task_free_stack(pTaskInfo->xID));
        }
        break;
        
    default:
        {
            bPrintUsage = TRUE;
        }
    }

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