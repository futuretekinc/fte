#ifndef __FTE_FBM_H__
#define __FTE_FBM_H__

typedef struct  FTE_FBM_STRUCT
{
    FTE_UINT32      ulUnit;
    FTE_UINT32      ulMaxCount;
    FTE_LIST        xAllocList; 
    FTE_LIST        xFreeList;
    FTE_SYS_LOCK    xLock;
}   FTE_FBM, _PTR_ FTE_FBM_PTR;

typedef struct  
{
    FTE_UINT32  ulSize;
    FTE_CHAR    pBuff[];
}   FTE_FBM_BUFF, _PTR_ FTE_FBM_BUFF_PTR;

FTE_FBM_PTR FTE_FBM_create(FTE_UINT32 ulUnit, FTE_UINT32 ulMaxCount);
FTE_RET FTE_FBM_destroy(FTE_FBM_PTR pFBM);

FTE_FBM_BUFF_PTR    FTE_FBM_alloc(FTE_FBM_PTR pFBM, FTE_UINT32 ulSize);
void                FTE_FBM_free(FTE_FBM_PTR pFBM, FTE_FBM_BUFF_PTR pBlock);

#endif
