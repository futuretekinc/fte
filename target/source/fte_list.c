#include "fte_target.h"
#include "fte_list.h"
#include "fte_assert.h"

_mqx_int    FTE_LIST_init(FTE_LIST_PTR pList)
{
    ASSERT(pList != NULL);
 
    FTE_LIST_NODE_PTR   pNode = (FTE_LIST_NODE_PTR)FTE_MEM_allocZero(sizeof(FTE_LIST_NODE));
    if (pNode == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }

    pNode->pNext = pNode;
    pNode->pPrev = pNode;
    pList->pHead  = pNode;
    pList->nCount = 0;

    fte_sys_lock_create(&pList->pLockKey);
    
    return  MQX_OK;
}

_mqx_int    FTE_LIST_final(FTE_LIST_PTR pList)
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

    fte_sys_lock_destroy(pList->pLockKey);
    FTE_MEM_free(pList->pHead);
    pList->pHead = NULL;
    pList->nCount= 0;
    
    return  MQX_OK;
}

boolean FTE_LIST_isInitialized(FTE_LIST_PTR pList)
{
    if ((pList == NULL) || (pList->pHead == NULL))
    {
        return  FALSE;
    }
    
    return  TRUE;
}

boolean     FTE_LIST_isExist(FTE_LIST_PTR pList, pointer pItem)
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

uint_32     FTE_LIST_count(FTE_LIST_PTR pList)
{
    ASSERT(pList != NULL);
    
    return  pList->nCount;
}

_mqx_int    FTE_LIST_pushFront(FTE_LIST_PTR pList, pointer pItem)
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
        return  MQX_OUT_OF_MEMORY;
    }

    fte_sys_lock_enable(pList->pLockKey);
    
    pNode->pItem = pItem;
    
    pNode->pPrev = pList->pHead;
    pNode->pNext = pList->pHead->pNext;
        
    pList->pHead->pNext->pPrev = pNode;
    pList->pHead->pNext = pNode;     
    pList->nCount++;
    
    fte_sys_lock_disable(pList->pLockKey);
    
    return  MQX_OK;
}

_mqx_int    FTE_LIST_pushBack(FTE_LIST_PTR pList, pointer pItem)
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
        return  MQX_OUT_OF_MEMORY;
    }

    fte_sys_lock_enable(pList->pLockKey);

    pNode->pItem = pItem;
    
    pNode->pPrev = pList->pHead->pPrev;
    pNode->pNext = pList->pHead;
        
    pList->pHead->pPrev->pNext = pNode;
    pList->pHead->pPrev = pNode;    
    pList->nCount++;
    
    fte_sys_lock_disable(pList->pLockKey);
  
    return  MQX_OK;
}

_mqx_int    FTE_LIST_pushSort(FTE_LIST_PTR pList, pointer pItem, int (*comparator)(pointer a, pointer b))
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
        return  MQX_OUT_OF_MEMORY;
    }

    fte_sys_lock_enable(pList->pLockKey);

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
    
    fte_sys_lock_disable(pList->pLockKey);
   
    return  MQX_OK;
}

_mqx_int    FTE_LIST_popFront(FTE_LIST_PTR pList, pointer _PTR_ ppItem)
{
    ASSERT(pList != NULL);
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    if (pList->nCount == 0)
    {
        return  MQX_ERROR;
    }

    fte_sys_lock_enable(pList->pLockKey);
    
    FTE_LIST_NODE_PTR   pNode = pList->pHead->pNext;
    
    pNode->pPrev->pNext = pNode->pNext;
    pNode->pNext->pPrev = pNode->pPrev;
    pList->nCount--;
   
    if (ppItem != NULL)
    {
        *ppItem = pNode->pItem;
    }
    
    fte_sys_lock_disable(pList->pLockKey);
    
    FTE_MEM_free(pNode);

    return  MQX_OK;
}

_mqx_int    FTE_LIST_popBack(FTE_LIST_PTR pList, pointer _PTR_ ppItem)
{
    ASSERT(pList != NULL);
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    if (pList->nCount == 0)
    {
        return  MQX_ERROR;
    }
    fte_sys_lock_enable(pList->pLockKey);

    FTE_LIST_NODE_PTR   pNode = pList->pHead->pPrev;
    
    fte_sys_lock_enable(pList->pLockKey);
    pList->pHead->pPrev = pNode->pPrev;
    pNode->pPrev->pNext = pNode->pNext;
    pList->nCount--;

    fte_sys_lock_disable(pList->pLockKey);
    
    if (ppItem != NULL)
    {
        *ppItem = pNode->pItem;
    }

    FTE_MEM_free(pNode);
    
    return  MQX_OK;
}

pointer FTE_LIST_getAt(FTE_LIST_PTR pList, uint_32 ulIndex)
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


_mqx_int    FTE_LIST_remove(FTE_LIST_PTR pList, pointer pItem)
{
    FTE_LIST_NODE_PTR   pNode;

    ASSERT((pList != NULL) && (pItem != NULL));
    
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
        
    fte_sys_lock_enable(pList->pLockKey);
    
    pNode = pList->pHead->pNext;
    
    while(pNode != pList->pHead)
    {
        if (pNode->pItem == pItem)
        {
            pNode->pPrev->pNext = pNode->pNext;
            pNode->pNext->pPrev = pNode->pPrev;            
            pList->nCount--;
             
            fte_sys_lock_disable(pList->pLockKey);
            
            FTE_MEM_free(pNode);
            
            return  MQX_OK;
        }
        
        pNode = pNode->pNext;
    }
    
    fte_sys_lock_disable(pList->pLockKey);
    
    return  MQX_ERROR;
}

_mqx_int    FTE_LIST_ITER_init(FTE_LIST_PTR pList, FTE_LIST_ITERATOR_PTR pIter)
{
    ASSERT((pList != NULL) && (pIter != NULL));
  
    if (FTE_LIST_isInitialized(pList) != TRUE)
    {
        FTE_LIST_init(pList);
    }
    
    pIter->pList = pList;
    pIter->pNode = pList->pHead;
    
    return  MQX_OK;    
}

pointer     FTE_LIST_ITER_getNext(FTE_LIST_ITERATOR_PTR pIter)
{
    ASSERT(pIter != NULL);
    
    if (pIter->pNode->pNext == pIter->pList->pHead)
    {
        return  NULL;
    }
    
    pIter->pNode = pIter->pNode->pNext;
    
    return  pIter->pNode->pItem;
}
