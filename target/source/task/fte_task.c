#include "fte_target.h"
#include "fte_task_main.h"
#include "fte_task_netd.h"
#include "fte_task_shell.h"
#include "fte_task_object_management.h"
#include "fte_task_timer.h"
#include "fte_cias.h"
#include "fte_task.h"
#include "fte_task_watchdog.h"
#include "fte_lora.h"
#include "fte_mbtcp.h"
#include <rtcs.h>
 
void MAC_15_4_process(FTE_UINT32 params);
void NS_STACK_main(FTE_UINT32 params);
void FTE_LORA_ctrl(FTE_UINT32 params);
void FTE_LORAWAN_process(FTE_UINT32 params);
void FTE_S2LORA_process(FTE_UINT32 ulParams);

/*
** MQX initialization information
*/

const 
TASK_TEMPLATE_STRUCT  MQX_template_list[] = 
{
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_MAIN,      /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_main,          /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_MAIN_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_MAIN_PRIO, /* Priority */
        .TASK_NAME              =   "main",             /* Name */
        .TASK_ATTRIBUTES        =   MQX_AUTO_START_TASK,/* Attributes */
        .CREATION_PARAMETER     =   0,                  /* Param */
        .DEFAULT_TIME_SLICE     =   0                   /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_OBJECT_MNGT,   /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_objectManagement,  /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_OBJECT_MNGT_STACK, /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_OBJECT_MNGT_PRIO,  /* Priority */
        .TASK_NAME              =   "object",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   (FTE_UINT32)0,          /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_WATCHDOG,      /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_watchdog,      /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_WATCHDOG_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_WATCHDOG_PRIO, /* Priority */
        .TASK_NAME              =   "watchdog",             /* Name */
        .TASK_ATTRIBUTES        =   MQX_AUTO_START_TASK,    /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_TIMER,         /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_timer,         /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_TIMER_STACK,   /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_TIMER_PRIO,    /* Priority */
        .TASK_NAME              =   "timer",                /* Name */
        .TASK_ATTRIBUTES        =   MQX_AUTO_START_TASK,    /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#if FTE_TASK_SHELL
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_SHELL,         /* Task Index */
        .TASK_ADDRESS           =   FTE_TASK_shell,         /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_SHELL_STACK,   /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_SHELL_PRIO,    /* Priority */
        .TASK_NAME              =   "shell",                /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
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
        .CREATION_PARAMETER     =   0,                  /* Param */
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
        .CREATION_PARAMETER     =   0,                  /* Param */
        .DEFAULT_TIME_SLICE     =   0                   /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_UCS_TX,    /* Task Index */
        .TASK_ADDRESS           =   FTE_UCS_TASK_send,  /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_UCS_TX_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_UCS_TX_PRIO,/* Priority */
        .TASK_NAME              =   "ucs_tx",           /* Name */
        .TASK_ATTRIBUTES        =   0,                  /* Attributes */
        .CREATION_PARAMETER     =   0,                  /* Param */
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
#if FTE_CIAS_SIOUX_CU_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_CIAS_SIOUX_CU, /* Task Index */
        .TASK_ADDRESS           =   FTE_CIAS_SIOUX_CU_task,    /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_CIAS_SIOUX_CU_STACK,   /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_CIAS_SIOUX_CU_PRIO,    /* Priority */
        .TASK_NAME              =   "SIOUX_CU",             /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if FTE_IOEX_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_IOEX,          /* Task Index */
        .TASK_ADDRESS           =   FTE_IOEX_task,          /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_IOEX_STACK,    /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_IOEX_PRIO,     /* Priority */
        .TASK_NAME              =   "IOEX",                 /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif

#if FTE_MULTI_DIO_NODE_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_DIO_NODE,          /* Task Index */
        .TASK_ADDRESS           =   FTE_DIO_NODE_task,          /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_DIO_NODE_STACK,    /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_DIO_NODE_PRIO,     /* Priority */
        .TASK_NAME              =   "DIO_NODE",                 /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
	
#if FTE_LORAWAN_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_LORA_CTRL,     /* Task Index */
        .TASK_ADDRESS           =   FTE_LORA_ctrl,          /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_LORA_STACK,    /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_LORA_PRIO,     /* Priority */
        .TASK_NAME              =   "LoRaWAN COMM",         /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_LORAWAN,       /* Task Index */
        .TASK_ADDRESS           =   FTE_LORAWAN_process,    /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_LORAWAN_STACK, /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_LORAWAN_PRIO,  /* Priority */
        .TASK_NAME              =   "LoRaWAN",              /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if  FTE_S2LORA_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_S2LORA,        /* Task Index */
        .TASK_ADDRESS           =   FTE_S2LORA_process,     /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_S2LORA_STACK,  /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_S2LORA_PRIO,   /* Priority */
        .TASK_NAME              =   "LoRaWAN",              /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   (FTE_UINT32)0,             /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if FTE_SOHA_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_SOHA,          /* Task Index */
        .TASK_ADDRESS           =   FTE_SOHA_task,          /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_SOHA_STACK,    /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_SOHA_PRIO,     /* Priority */
        .TASK_NAME              =   "soha",                 /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif    
#if FTE_ELT_AQM100_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_ELT,          /* Task Index */
        .TASK_ADDRESS           =   FTE_ELT_task,          /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_ELT_STACK,    /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_ELT_PRIO,     /* Priority */
        .TASK_NAME              =   "elt",                 /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif    
#if FTE_DOTECH_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_DOTECH,        /* Task Index */
        .TASK_ADDRESS           =   FTE_DOTECH_task,        /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_DOTECH_STACK,  /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_DOTECH_PRIO,   /* Priority */
        .TASK_NAME              =   "dotech",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if FTE_TURBOMAX_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_TURBOMAX,      /* Task Index */
        .TASK_ADDRESS           =   FTE_TURBOMAX_task,      /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_TURBOMAX_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_TURBOMAX_PRIO, /* Priority */
        .TASK_NAME              =   "TurboMax",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if FTE_FUTURETEK_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_FUTURETEK,      /* Task Index */
        .TASK_ADDRESS           =   FTE_FUTURETEK_task,      /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_FUTURETEK_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_FUTURETEK_PRIO, /* Priority */
        .TASK_NAME              =   "Futuretek",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_FUTURETEK_MS10,      /* Task Index */
        .TASK_ADDRESS           =   FTE_FUTURETEK_MS10_task,      /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_FUTURETEK_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_FUTURETEK_PRIO, /* Priority */
        .TASK_NAME              =   "Futuretek",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_FUTURETEK_KM21B20,      /* Task Index */
        .TASK_ADDRESS           =   FTE_FUTURETEK_KM21B20_task,      /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_FUTURETEK_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_FUTURETEK_PRIO, /* Priority */
        .TASK_NAME              =   "Futuretek",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_FUTURETEK_JCG06,      /* Task Index */
        .TASK_ADDRESS           =   FTE_FUTURETEK_JCG06_task,      /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_FUTURETEK_JCG06_STACK,/* Stack */
        .TASK_PRIORITY          =   FTE_TASK_FUTURETEK_JCG06_PRIO, /* Priority */
        .TASK_NAME              =   "Futuretek_jcg06",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if FTE_MBTCP_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_MBTCP_MAIN,    /* Task Index */
        .TASK_ADDRESS           =   FTE_MBTCP_mainTask,     /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_MBTCP_MAIN_STACK,  /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_MBTCP_MAIN_PRIO,   /* Priority */
        .TASK_NAME              =   "mbtcp",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
#if FTE_MBTCP_SUPPORTED
    { 
        .TASK_TEMPLATE_INDEX    =   FTE_TASK_MBTCP_SESSION,        /* Task Index */
        .TASK_ADDRESS           =   FTE_MBTCP_sessionTask,        /* Function */
        .TASK_STACKSIZE         =   FTE_TASK_MBTCP_SESSION_STACK,  /* Stack */
        .TASK_PRIORITY          =   FTE_TASK_MBTCP_SESSION_PRIO,   /* Priority */
        .TASK_NAME              =   "mbtcp_session",               /* Name */
        .TASK_ATTRIBUTES        =   0,                      /* Attributes */
        .CREATION_PARAMETER     =   0,                      /* Param */
        .DEFAULT_TIME_SLICE     =   0                       /* Time Slice */
    },
#endif
    { 0 }
};

FTE_LIST    _taskList = 
{
    .nCount     = 0,
    .pHead      = NULL,
    .pLockKey   = NULL
};

FTE_RET   FTE_TASK_create
(
    FTE_INT32           nTemplateIndex, 
    FTE_UINT32          ulParameter,
    FTE_TASK_ID_PTR     pTaskID
)
{
    FTE_TASK_ID         xTaskID;
    
    xTaskID = _task_create(0, nTemplateIndex, ulParameter);

    if (xTaskID == MQX_NULL_TASK_ID)
    {
        for(FTE_INT32 i = 0  ; MQX_template_list[nTemplateIndex].TASK_TEMPLATE_INDEX != 0 ; i++)
        {
            if (MQX_template_list[nTemplateIndex].TASK_TEMPLATE_INDEX == nTemplateIndex)
            {
                ERROR("\nCould not create %s\n", MQX_template_list[nTemplateIndex].TASK_NAME);
                return  FTE_RET_ERROR;
            }
        }
        
        ERROR("\nCould not create unknown[%d]\n", nTemplateIndex);
        return  FTE_RET_ERROR;
    }
       
    FTE_TASK_append(FTE_TASK_TYPE_MQX, xTaskID);

    if (pTaskID != NULL)
    {
        *pTaskID = xTaskID;
    }
    return  FTE_RET_OK;
}

FTE_RET   FTE_TASK_append
(
    FTE_TASK_TYPE   xType,
    FTE_TASK_ID     xTaskID
)
{
    FTE_TASK_INFO_PTR   pTaskInfo;
        
    pTaskInfo = (FTE_TASK_INFO_PTR)FTE_MEM_allocZero(sizeof(FTE_TASK_INFO));
    if (pTaskInfo == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pTaskInfo->xID  = xTaskID;
    pTaskInfo->xType= xType;
        
    FTE_LIST_pushBack(&_taskList, (pointer)pTaskInfo);    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_TASK_remove
(
    FTE_TASK_ID        xTaskID
)
{ 
    for(FTE_INT32 i = 0 ; i < FTE_LIST_count(&_taskList); i++)
    {
        FTE_TASK_INFO_PTR   pTaskInfo = (FTE_TASK_INFO_PTR)FTE_LIST_getAt(&_taskList, i);
        if (pTaskInfo->xID == xTaskID)
        {
            FTE_LIST_remove(&_taskList, pTaskInfo);
            FTE_MEM_free(pTaskInfo);
            
            return  FTE_RET_OK;
        }        
    }

    return  MQX_INVALID_TASK_ID;
}

FTE_INT32 FTE_TASK_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            FTE_UINT32             ulCount = 0;
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