#ifndef __FTE_SYS_LOCK_H__
#define __FTE_SYS_LOCK_H__

typedef struct
{
    LWSEM_STRUCT    xSemaphore;
}   FTE_SYS_LOCK, _PTR_ FTE_SYS_LOCK_PTR;

_mqx_uint   fte_sys_lock_create(FTE_SYS_LOCK_PTR _PTR_ pKey);
_mqx_uint   fte_sys_lock_enable(FTE_SYS_LOCK_PTR pKey);
_mqx_uint   fte_sys_lock_disable(FTE_SYS_LOCK_PTR pKey);
_mqx_uint   fte_sys_lock_destroy(FTE_SYS_LOCK_PTR pKey);

#endif