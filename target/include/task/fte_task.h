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
            uint_32 ulID;
        }   xRTCS;
    }   xParams;
    
}   FTE_TASK_INFO, _PTR_ FTE_TASK_INFO_PTR;


_mqx_uint   FTE_TASK_create(_processor_number xProcessorID, _mqx_uint nTemplateIndex, uint_32 ulParameter);
_mqx_uint   FTE_TASK_append(FTE_TASK_TYPE   xType, _task_id        xTaskID);
_mqx_uint   FTE_TASK_remove(_task_id        xTaskID);
int_32      FTE_TASK_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] );

#endif