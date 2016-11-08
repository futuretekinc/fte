#include "fte_target.h"
#include "fte_drv.h"

typedef struct _FTE_DRIVER_DESCRIPT_CONTENT_STRUCT
{
    struct _FTE_DRIVER_DESCRIPT_CONTENT_STRUCT * pNext;
    FTE_DRIVER_DESCRIPT_CONST_PTR               pDescript;
} FTE_DRIVER_DESCRIPT_CONTENT, _PTR_ FTE_DRIVER_DESCRIPT_CONTENT_PTR;

static 
FTE_DRIVER_DESCRIPT_CONTENT_PTR  _pHead  = NULL;

static 
FTE_UINT32                          _nCount = 0;

FTE_RET   FTE_DRV_init
(
    FTE_DRIVER_DESCRIPT_CONST_PTR   pDescript
)
{
    FTE_DRIVER_DESCRIPT_CONTENT_PTR pContent;
    
    pContent = (FTE_DRIVER_DESCRIPT_CONTENT_PTR)FTE_MEM_allocZero(sizeof(FTE_DRIVER_DESCRIPT_CONTENT));
    if (pContent == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    pContent->pNext      = _pHead;
    pContent->pDescript  = pDescript;
    
    _pHead = pContent;
    _nCount++;

    return  FTE_RET_OK;
}

FTE_RET   FTE_DRV_create
(
    FTE_DEV_TYPE    xType, 
    FTE_VOID_PTR    pParams
)
{
    FTE_DRIVER_DESCRIPT_CONTENT_PTR pContent;
    
    pContent = _pHead;
    while(pContent != NULL)
    {
        if (pContent->pDescript->nType == xType)
        {
            return  FTE_RET_OK;
        }
        
        pContent = pContent->pNext;
    }
    
    return  FTE_RET_INVALID_OBJECT;
}