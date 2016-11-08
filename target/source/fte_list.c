#include "fte_target.h"
#include "fte_list.h"
#include "fte_assert.h"

FTE_RET FTE_LIST_init
(
    FTE_LIST_PTR    pList
)
{
    ASSERT(pList != NULL);
 
    FTE_LIST_NODE_PTR   pNode = (FTE_LIST_NODE_PTR)FTE_MEM_allocZero(sizeof(FTE_LIST_NODE));
    if (pNode == NULL)
    {
        return  FTE_RET_INSUFFICIENT_MEMORY;
    }

    pNode->pNext = pNode;
    pNode->pPrev = pNode;
    pList->pHead  = pNode;
    pList->nCount = 0;

    FTE_SYS_LOCK_create(&pList->pLockKey);
    
    return  FTE_RET_OK;
}

FTE_RET    FTE_LIST_final
(
    FTE_LIST_PTR    pList
)
{
    FTE_LIST_NODE_PTR pNode, pNextNode ;
    
    ASSERT(pList != NULL);
    pNode = pList->pHead;
    while(pNode != pList->pHead)
    {
        pNextNode = pNode->pNext;
        FTE_MEM_free(pNode);
        pNode = pNextNode;
    }

    FTE_SYS_LOCK_destroy(pList->pLockKey);
    FTE_MEM_free(pList->pHead);
    pList->pHead = NULL;
    pList->nCount= 0;
    
    return  FTE_RET_OK;
}

FTE_BOOL FTE_LIST_isInitialized
(
    FTE_LIST_PTR    pList
)
{
    if ((pList == NULL) || (pList->pHead == NULL))
    {
        return  FALSE;
    }
    
    return  TRUE;
}

FTE_BOOL    FTE_LIST_isExist
(
    FTE_LIST_PTR    pList, 
    FTE_VOID_PTR    pItem
)
{
    FTE_LIST_NODE_PTR    pNode = pList->pHead;
    if (pNode != NULL)
    {
        pNode = pNode->pNext;
        
        while(pNode != pList->pHead)
        {
            if (pNode->pItem == pItem)
            {
                return  TRUE;
            }
            
            pNode = pNode->pNext;
        }
    }
    
    return  FALSE;
}

FTE_UINT32  FTE_LIST_count
(
    FTE_LIST_PTR    pList
)
{
    ASSERT(pList != NULL);
    
    return  pList->nCount;
}

FTE_RET    FTE_LIST_pushFront
(
    FTE_LIST_PTR    pList, 
    FTE_VOID_PTR    pItem
)
{
    FTE_LIST_NODE_PTR   pNode;
    ASSERT((pList != NULL) && (pItem != NULL));

    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    pNode = (FTE_LIST_NODE_PTR)FTE_MEM_allocZero(sizeof(FTE_LIST_NODE));
    if (pNode == NULL)
    {
        ERROR("Not enough memory!\n");
        FTE_SYS_setUnstable();
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }

    FTE_SYS_LOCK_enable(pList->pLockKey);
    
    pNode->pItem = pItem;
    
    pNode->pPrev = pList->pHead;
    pNode->pNext = pList->pHead->pNext;
        
    pList->pHead->pNext->pPrev = pNode;
    pList->pHead->pNext = pNode;     
    pList->nCount++;
    
    FTE_SYS_LOCK_disable(pList->pLockKey);
    
    return  FTE_RET_OK;
}

FTE_RET    FTE_LIST_pushBack
(
    FTE_LIST_PTR    pList, 
    FTE_VOID_PTR    pItem
)
{
    FTE_LIST_NODE_PTR   pNode;

    ASSERT((pList != NULL) && (pItem != NULL));
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    pNode = (FTE_LIST_NODE_PTR)FTE_MEM_allocZero(sizeof(FTE_LIST_NODE));
    if (pNode == NULL)
    {
        ERROR("Not enough memory!\n");
        FTE_SYS_setUnstable();
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }

    FTE_SYS_LOCK_enable(pList->pLockKey);

    pNode->pItem = pItem;
    
    pNode->pPrev = pList->pHead->pPrev;
    pNode->pNext = pList->pHead;
        
    pList->pHead->pPrev->pNext = pNode;
    pList->pHead->pPrev = pNode;    
    pList->nCount++;
    
    FTE_SYS_LOCK_disable(pList->pLockKey);
  
    return  FTE_RET_OK;
}

FTE_RET    FTE_LIST_pushSort
(
    FTE_LIST_PTR    pList, 
    FTE_VOID_PTR    pItem, 
    FTE_INT32 (*comparator)(FTE_VOID_PTR a, FTE_VOID_PTR b)
)
{
    FTE_LIST_NODE_PTR   pNode;
    FTE_LIST_NODE_PTR   pNextNode;

    ASSERT((pList != NULL) && (pItem != NULL));
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    pNode = (FTE_LIST_NODE_PTR)FTE_MEM_allocZero(sizeof(FTE_LIST_NODE));
    if (pNode == NULL)
    {
        ERROR("Not enough memory!\n");
        FTE_SYS_setUnstable();
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }

    FTE_SYS_LOCK_enable(pList->pLockKey);

    pNode->pItem = pItem;
    
    pNextNode = pList->pHead->pNext;
    while(pNextNode != pList->pHead)
    {
        if (comparator(pItem, pNextNode->pItem) < 0)
        {
            break;
        }
        pNextNode = pNextNode->pNext;
    }
    
    pNode->pPrev = pNextNode->pPrev;
    pNode->pNext = pNextNode;
        
    pNextNode->pPrev->pNext = pNode;
    pNextNode->pPrev = pNode;    
    
    pList->nCount++;
    
    FTE_SYS_LOCK_disable(pList->pLockKey);
   
    return  FTE_RET_OK;
}

FTE_RET    FTE_LIST_popFront
(
    FTE_LIST_PTR    pList, 
    FTE_VOID_PTR _PTR_ ppItem
)
{
    ASSERT(pList != NULL);
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    if (pList->nCount == 0)
    {
        return  FTE_RET_ERROR;
    }

    FTE_SYS_LOCK_enable(pList->pLockKey);
    
    FTE_LIST_NODE_PTR   pNode = pList->pHead->pNext;
    
    pNode->pPrev->pNext = pNode->pNext;
    pNode->pNext->pPrev = pNode->pPrev;
    pList->nCount--;
   
    if (ppItem != NULL)
    {
        *ppItem = pNode->pItem;
    }
    
    FTE_SYS_LOCK_disable(pList->pLockKey);
    
    FTE_MEM_free(pNode);

    return  FTE_RET_OK;
}

FTE_RET    FTE_LIST_popBack
(
    FTE_LIST_PTR    pList, 
    FTE_VOID_PTR _PTR_ ppItem
)
{
    ASSERT(pList != NULL);
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    if (pList->nCount == 0)
    {
        return  FTE_RET_ERROR;
    }
    FTE_SYS_LOCK_enable(pList->pLockKey);

    FTE_LIST_NODE_PTR   pNode = pList->pHead->pPrev;
    
    FTE_SYS_LOCK_enable(pList->pLockKey);
    pList->pHead->pPrev = pNode->pPrev;
    pNode->pPrev->pNext = pNode->pNext;
    pList->nCount--;

    FTE_SYS_LOCK_disable(pList->pLockKey);
    
    if (ppItem != NULL)
    {
        *ppItem = pNode->pItem;
    }

    FTE_MEM_free(pNode);
    
    return  FTE_RET_OK;
}

FTE_VOID_PTR FTE_LIST_getAt
(
    FTE_LIST_PTR    pList, 
    FTE_UINT32      ulIndex
)
{
    ASSERT(pList != NULL);
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    if (pList->nCount <= ulIndex)
    {
        return  NULL;
    }
    
    FTE_LIST_NODE_PTR    pNode = pList->pHead->pNext;
    while((pNode != pList->pHead) && (ulIndex > 0))
    {
        pNode = pNode->pNext;
        ulIndex--;
    }
    
    if (ulIndex != 0)
    {
        return  NULL;
    }
    
    return  pNode->pItem;
}


FTE_RET    FTE_LIST_remove
(
    FTE_LIST_PTR    pList, 
    FTE_VOID_PTR    pItem
)
{
    FTE_LIST_NODE_PTR   pNode;

    ASSERT((pList != NULL) && (pItem != NULL));
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
        
    FTE_SYS_LOCK_enable(pList->pLockKey);
    
    pNode = pList->pHead->pNext;
    
    while(pNode != pList->pHead)
    {
        if (pNode->pItem == pItem)
        {
            pNode->pPrev->pNext = pNode->pNext;
            pNode->pNext->pPrev = pNode->pPrev;            
            pList->nCount--;
             
            FTE_SYS_LOCK_disable(pList->pLockKey);
            
            FTE_MEM_free(pNode);
            
            return  FTE_RET_OK;
        }
        
        pNode = pNode->pNext;
    }
    
    FTE_SYS_LOCK_disable(pList->pLockKey);
    
    return  FTE_RET_ERROR;
}

FTE_RET    FTE_LIST_ITER_init
(
    FTE_LIST_PTR    pList, 
    FTE_LIST_ITERATOR_PTR   pIter
)
{
    ASSERT((pList != NULL) && (pIter != NULL));
  
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    pIter->pList = pList;
    pIter->pNode = pList->pHead;
    
    return  FTE_RET_OK;    
}

FTE_VOID_PTR     FTE_LIST_ITER_getNext
(
    FTE_LIST_ITERATOR_PTR   pIter
)
{
    ASSERT(pIter != NULL);
    
    if (pIter->pNode->pNext == pIter->pList->pHead)
    {
        return  NULL;
    }
    
    pIter->pNode = pIter->pNode->pNext;
    
    return  pIter->pNode->pItem;
}
