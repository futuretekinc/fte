#ifndef __FTE_FBM_H__
#define __FTE_FBM_H__

typedef struct  FTE_FBM_struct
{
    uint_32         ulUnit;
    uint_32         ulMaxCount;
    FTE_LIST        xAllocList; 
    FTE_LIST        xFreeList;
    LWSEM_STRUCT    xSemaphore;
}   FTE_FBM, _PTR_ FTE_FBM_PTR;

typedef struct  
{
    uint_32 ulSize;
    char    pBuff[];
}   FTE_FBM_BUFF, _PTR_ FTE_FBM_BUFF_PTR;

FTE_FBM_PTR FTE_FBM_create(uint_32 ulUnit, uint_32 ulMaxCount);
_mqx_uint   FTE_FBM_destroy(FTE_FBM_PTR pFBM);

FTE_FBM_BUFF_PTR    FTE_FBM_alloc(FTE_FBM_PTR pFBM, uint_32 ulSize);
void                FTE_FBM_free(FTE_FBM_PTR pFBM, FTE_FBM_BUFF_PTR pBlock);

#endif
