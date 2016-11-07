#ifndef __FTE_TASK_H__
#define __FTE_TASK_H__

typedef enum 
{
    FTE_TASK_TYPE_MQX,
    FTE_TASK_TYPE_RTCS
}   FTE_TASK_TYPE, _PTR_ FTE_TASK_TYPE_PTR;

typedef struct
{
    _task_id        xID;
    FTE_TASK_TYPE   xType;
    union
    {
        struct
        {
            FTE_UINT32 ulID;
        }   xRTCS;
    }   xParams;
    
}   FTE_TASK_INFO, _PTR_ FTE_TASK_INFO_PTR;


FTE_RET     FTE_TASK_create(_processor_number xProcessorID, FTE_RET nTemplateIndex, FTE_UINT32 ulParameter);
FTE_RET     FTE_TASK_append(FTE_TASK_TYPE   xType, _task_id        xTaskID);
FTE_RET     FTE_TASK_remove(_task_id        xTaskID);
FTE_INT32   FTE_TASK_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );

#endif