#include "fte_target.h"
#include "fte_buff.h"


FTE_FBM_PTR FTE_FBM_create
(
    FTE_UINT32  ulUnit, 
    FTE_UINT32  ulMaxCount
)
{
    FTE_FBM_PTR pFBM = NULL;
    
    pFBM = (FTE_FBM_PTR)FTE_MEM_allocZero(sizeof(FTE_FBM));
    if (pFBM == NULL)
    {
        goto error;
    }
    
    for(int i = 0 ; i < ulMaxCount ; i++)
    {
        FTE_FBM_BUFF_PTR pBuff = (FTE_FBM_BUFF_PTR)FTE_MEM_allocZero(sizeof(FTE_FBM_BUFF) + ulUnit);    
        if (pBuff == NULL)
        {
            goto error;
        }
        
        FTE_LIST_pushBack(&pFBM->xFreeList, pBuff);
    }
    
    pFBM->ulUnit = ulUnit;
    pFBM->ulMaxCount = ulMaxCount;
    
    FTE_SYS_LOCK_init(&pFBM->xLock, 1);
   
    return  pFBM;
    
error:
    if (pFBM != NULL)
    {
        while(FTE_LIST_count(&pFBM->xAllocList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xAllocList, (pointer *)&pBuff) == FTE_RET_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }

        while(FTE_LIST_count(&pFBM->xFreeList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xFreeList, (pointer *)&pBuff) == FTE_RET_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }
        
        FTE_MEM_free(pFBM);
    }
    
    return  NULL;
}

FTE_RET   FTE_FBM_destroy
(
    FTE_FBM_PTR     pFBM
)
{
    if (pFBM != NULL)
    {
        while(FTE_LIST_count(&pFBM->xAllocList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xAllocList, (pointer *)&pBuff) == FTE_RET_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }

        while(FTE_LIST_count(&pFBM->xFreeList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xFreeList, (pointer *)&pBuff) == FTE_RET_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }
        
        FTE_MEM_free(pFBM);
    }
    
    return  FTE_RET_OK;
}

FTE_FBM_BUFF_PTR    FTE_FBM_alloc
(
    FTE_FBM_PTR     pFBM, 
    FTE_UINT32      ulSize
)
{
    FTE_FBM_BUFF_PTR pAlloc = NULL;
    
    if ((pFBM != NULL) && (FTE_LIST_count(&pFBM->xFreeList) != 0))
    {
        FTE_FBM_BUFF_PTR    pBuff;
        
        FTE_SYS_LOCK_enable(&pFBM->xLock);
        
        if (FTE_LIST_popFront(&pFBM->xFreeList, (pointer *)&pBuff) == FTE_RET_OK)
        {
            FTE_LIST_pushBack(&pFBM->xAllocList, pBuff);
            pBuff->ulSize = ulSize;
            
            pAlloc = pBuff;
        }
        FTE_SYS_LOCK_disable(&pFBM->xLock);
    }
    
    return  pAlloc;
}

void    FTE_FBM_free
(
    FTE_FBM_PTR     pFBM, 
    FTE_FBM_BUFF_PTR pBlock
)
{
    if (pFBM != NULL)
    {
        FTE_SYS_LOCK_enable(&pFBM->xLock);
        
        if (FTE_LIST_remove(&pFBM->xAllocList, pBlock) == FTE_RET_OK)
        {
            FTE_LIST_pushBack(&pFBM->xFreeList, pBlock);
        }
        
        FTE_SYS_LOCK_disable(&pFBM->xLock);
    }
}
