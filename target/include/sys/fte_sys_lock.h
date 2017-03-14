#ifndef __FTE_SYS_LOCK_H__
#define __FTE_SYS_LOCK_H__

#include "fte_type.h"

typedef struct
{
    LWSEM_STRUCT    xSemaphore;
}   FTE_SYS_LOCK, _PTR_ FTE_SYS_LOCK_PTR;

FTE_RET FTE_SYS_LOCK_create(FTE_SYS_LOCK_PTR _PTR_ pKey);
FTE_RET FTE_SYS_LOCK_destroy(FTE_SYS_LOCK_PTR pKey);

FTE_RET FTE_SYS_LOCK_init(FTE_SYS_LOCK_PTR pKey, FTE_UINT32 ulInit);
FTE_RET FTE_SYS_LOCK_final(FTE_SYS_LOCK_PTR pKey);

FTE_RET FTE_SYS_LOCK_enable(FTE_SYS_LOCK_PTR pKey);
FTE_RET FTE_SYS_LOCK_disable(FTE_SYS_LOCK_PTR pKey);

#endif