#ifndef __FTE_LIST_H__
#define __FTE_LIST_H__

#include "fte_sys_lock.h"

typedef struct  _FTE_LIST_NODE_STRUCT 
{
    struct  _FTE_LIST_NODE_STRUCT _PTR_ pPrev;
    struct  _FTE_LIST_NODE_STRUCT _PTR_ pNext;
    pointer                             pItem;
    uint_32                             nReserved;
}   FTE_LIST_NODE, _PTR_ FTE_LIST_NODE_PTR;

typedef struct _FTE_LIST_STRUCT
{
    uint_32             nCount;
    FTE_LIST_NODE_PTR   pHead;
    FTE_SYS_LOCK_PTR    pLockKey;
}   FTE_LIST, _PTR_ FTE_LIST_PTR;

typedef struct  
{
    FTE_LIST_PTR        pList;
    FTE_LIST_NODE_PTR   pNode;
}   FTE_LIST_ITERATOR, _PTR_ FTE_LIST_ITERATOR_PTR;

_mqx_int    FTE_LIST_init(FTE_LIST_PTR pList);
_mqx_int    FTE_LIST_final(FTE_LIST_PTR pList);

boolean     FTE_LIST_isInitialized(FTE_LIST_PTR pList);
boolean     FTE_LIST_isExist(FTE_LIST_PTR pList, pointer pItem);

uint_32     FTE_LIST_count(FTE_LIST_PTR pList);

_mqx_int    FTE_LIST_pushFront(FTE_LIST_PTR pList, pointer pItem);
_mqx_int    FTE_LIST_pushBack(FTE_LIST_PTR pList, pointer pItem);
_mqx_int    FTE_LIST_pushSort(FTE_LIST_PTR pList, pointer pItem, int (*comparator)(pointer a, pointer b));
_mqx_int    FTE_LIST_popFront(FTE_LIST_PTR pList, pointer _PTR_ ppItem);
_mqx_int    FTE_LIST_popBack(FTE_LIST_PTR pList, pointer _PTR_ ppItem);
_mqx_int    FTE_LIST_remove(FTE_LIST_PTR pList, pointer pItem);

pointer     FTE_LIST_getAt(FTE_LIST_PTR pList, uint_32 ulIndex);

_mqx_int    FTE_LIST_ITER_init(FTE_LIST_PTR pList, FTE_LIST_ITERATOR_PTR pIter);
pointer     FTE_LIST_ITER_getNext(FTE_LIST_ITERATOR_PTR pIter);
#endif
