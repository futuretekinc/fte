#include "fte_target.h"
#include "fte_drv.h"

typedef struct _FTE_DRIVER_DESCRIPT_CONTENT_STRUCT
{
    struct _FTE_DRIVER_DESCRIPT_CONTENT_STRUCT * pNext;
    FTE_DRIVER_DESCRIPT_CONST_PTR               pDescript;
} FTE_DRIVER_DESCRIPT_CONTENT, _PTR_ FTE_DRIVER_DESCRIPT_CONTENT_PTR;

static FTE_DRIVER_DESCRIPT_CONTENT_PTR  _pHead  = NULL;
static uint_32                          _nCount = 0;

_mqx_uint   FTE_DRV_init(FTE_DRIVER_DESCRIPT_CONST_PTR pDescript)
{
    FTE_DRIVER_DESCRIPT_CONTENT_PTR pContent;
    
    pContent = (FTE_DRIVER_DESCRIPT_CONTENT_PTR)FTE_MEM_allocZero(sizeof(FTE_DRIVER_DESCRIPT_CONTENT));
    if (pContent == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }
    
    pContent->pNext      = _pHead;
    pContent->pDescript  = pDescript;
    
    _pHead = pContent;
    _nCount++;

    return  MQX_OK;
}

_mqx_uint   FTE_DRV_create(fte_dev_type xType, void *pParams)
{
    FTE_DRIVER_DESCRIPT_CONTENT_PTR pContent;
    
    pContent = _pHead;
    while(pContent != NULL)
    {
        if (pContent->pDescript->nType == xType)
        {
            return  MQX_OK;
        }
        
        pContent = pContent->pNext;
    }
    
    return  MQX_INVALID_DEVICE;
}