#ifndef __FTE_LIST_H__
#define __FTE_LIST_H__

#include "fte_sys_lock.h"

typedef struct  _FTE_LIST_NODE_STRUCT 
{
    struct  _FTE_LIST_NODE_STRUCT _PTR_ pPrev;
    struct  _FTE_LIST_NODE_STRUCT _PTR_ pNext;
    FTE_VOID_PTR    pItem;
    FTE_UINT32      nReserved;
}   FTE_LIST_NODE, _PTR_ FTE_LIST_NODE_PTR;

typedef struct _FTE_LIST_STRUCT
{
    FTE_UINT32          nCount;
    FTE_LIST_NODE_PTR   pHead;
    FTE_SYS_LOCK_PTR    pLockKey;
}   FTE_LIST, _PTR_ FTE_LIST_PTR;

typedef struct  
{
    FTE_LIST_PTR        pList;
    FTE_LIST_NODE_PTR   pNode;
}   FTE_LIST_ITERATOR, _PTR_ FTE_LIST_ITERATOR_PTR;

FTE_RET     FTE_LIST_init(FTE_LIST_PTR pList);
FTE_RET     FTE_LIST_final(FTE_LIST_PTR pList);

FTE_BOOL    FTE_LIST_isInitialized(FTE_LIST_PTR pList);
FTE_BOOL    FTE_LIST_isExist(FTE_LIST_PTR pList, FTE_VOID_PTR pItem);

FTE_UINT32  FTE_LIST_count(FTE_LIST_PTR pList);

FTE_RET     FTE_LIST_pushFront(FTE_LIST_PTR pList, FTE_VOID_PTR pItem);
FTE_RET     FTE_LIST_pushBack(FTE_LIST_PTR pList, FTE_VOID_PTR pItem);
FTE_RET     FTE_LIST_pushSort(FTE_LIST_PTR pList, FTE_VOID_PTR pItem, FTE_INT32 (*comparator)(FTE_VOID_PTR a, FTE_VOID_PTR b));
FTE_RET     FTE_LIST_popFront(FTE_LIST_PTR pList, FTE_VOID_PTR _PTR_ ppItem);
FTE_RET     FTE_LIST_popBack(FTE_LIST_PTR pList, FTE_VOID_PTR _PTR_ ppItem);
FTE_RET     FTE_LIST_remove(FTE_LIST_PTR pList, FTE_VOID_PTR pItem);

FTE_VOID_PTR    FTE_LIST_getAt(FTE_LIST_PTR pList, FTE_UINT32 ulIndex);

FTE_RET    FTE_LIST_ITER_init(FTE_LIST_PTR pList, FTE_LIST_ITERATOR_PTR pIter);
FTE_VOID_PTR     FTE_LIST_ITER_getNext(FTE_LIST_ITERATOR_PTR pIter);
#endif
