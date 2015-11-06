#include "fte_target.h"
#include "fte_buff.h"


FTE_FBM_PTR FTE_FBM_create(uint_32 ulUnit, uint_32 ulMaxCount)
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
    
    _lwsem_create(&pFBM->xSemaphore, 1);
   
    return  pFBM;
    
error:
    if (pFBM != NULL)
    {
        while(FTE_LIST_count(&pFBM->xAllocList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xAllocList, (pointer *)&pBuff) == MQX_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }

        while(FTE_LIST_count(&pFBM->xFreeList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xFreeList, (pointer *)&pBuff) == MQX_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }
        
        FTE_MEM_free(pFBM);
    }
    
    return  NULL;
}

_mqx_uint   FTE_FBM_destroy(FTE_FBM_PTR pFBM)
{
    if (pFBM != NULL)
    {
        while(FTE_LIST_count(&pFBM->xAllocList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xAllocList, (pointer *)&pBuff) == MQX_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }

        while(FTE_LIST_count(&pFBM->xFreeList) != 0)
        {
            FTE_FBM_BUFF_PTR    pBuff;
            
            if (FTE_LIST_popFront(&pFBM->xFreeList, (pointer *)&pBuff) == MQX_OK)
            {
                FTE_MEM_free(pBuff);
            }
        }
        
        FTE_MEM_free(pFBM);
    }
    
    return  MQX_OK;
}

FTE_FBM_BUFF_PTR    FTE_FBM_alloc(FTE_FBM_PTR pFBM, uint_32 ulSize)
{
    FTE_FBM_BUFF_PTR pAlloc = NULL;
    
    if ((pFBM != NULL) && (FTE_LIST_count(&pFBM->xFreeList) != 0))
    {
        FTE_FBM_BUFF_PTR    pBuff;
        
        _lwsem_wait(&pFBM->xSemaphore);
        
        if (FTE_LIST_popFront(&pFBM->xFreeList, (pointer *)&pBuff) == MQX_OK)
        {
            FTE_LIST_pushBack(&pFBM->xAllocList, pBuff);
            pBuff->ulSize = ulSize;
            
            pAlloc = pBuff;
        }
        _lwsem_post(&pFBM->xSemaphore);
    }
    
    return  pAlloc;
}

void        FTE_FBM_free(FTE_FBM_PTR pFBM, FTE_FBM_BUFF_PTR pBlock)
{
    if (pFBM != NULL)
    {
        _lwsem_wait(&pFBM->xSemaphore);
        
        if (FTE_LIST_remove(&pFBM->xAllocList, pBlock) == MQX_OK)
        {
            FTE_LIST_pushBack(&pFBM->xFreeList, pBlock);
        }
        
        _lwsem_post(&pFBM->xSemaphore);
    }
}
