#include "fte_target.h"
#include "fte_ucs.h"

static  FTE_LIST    _UCMs = 
{ 
    .nCount = 0,
    .pHead  = NULL
};

_mqx_uint       FTE_UCM_create(FTE_UCM_CONFIG_CONST_PTR pConfig)
{
    FTE_UCS_PTR pUCS = NULL;
    FTE_UCM_PTR pUCM = NULL;
    
    ASSERT(pConfig != NULL);
    
    if (_UCMs.pHead == NULL)
    {
        FTE_LIST_init(&_UCMs);
    }
    
    pUCS = FTE_UCS_get(pConfig->nUCSID);
    if (pUCS == NULL)
    {
        return  MQX_ERROR;
    }
    
    pUCM = (FTE_UCM_PTR)FTE_MEM_allocZero(sizeof(FTE_UCM));
    if (pUCM == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }
    
    pUCM->pConfig   = pConfig;
    pUCM->nParent   = 0;
    
    FTE_LIST_pushBack(&_UCMs, pUCM);
    
    return  MQX_OK;
}

_mqx_uint   FTE_UCM_attach(FTE_UCM_PTR pUCM, uint_32 nParent)
{
    ASSERT(pUCM != NULL);
    if (pUCM == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    FTE_UCS_PTR pUCS = FTE_UCS_get(pUCM->pConfig->nUCSID);
    if (pUCS == NULL)
    {
        return  MQX_ERROR;
    }
    
    if (FTE_UCS_attach(pUCS, pUCM->pConfig->nID) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pUCM->nParent = nParent;
    
    return  MQX_OK;
}

_mqx_uint   FTE_UCM_detach(FTE_UCM_PTR pUCM, uint_32 nParent)
{
    ASSERT(pUCM != NULL);
    if (pUCM == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    FTE_UCS_PTR pUCS = FTE_UCS_get(pUCM->pConfig->nUCSID);
    if (pUCS == NULL)
    {
        return  MQX_ERROR;
    }
    
    FTE_UCS_detach(pUCS, pUCM->pConfig->nID);
    pUCM->nParent = 0;
    
    return  MQX_OK;
}

FTE_UCM_PTR     FTE_UCM_get(uint_32 xID)
{
    pointer     pIter   = NULL;
    FTE_UCM_PTR pUCM    = NULL;
    
    pUCM = (FTE_UCM_PTR)fte_list_get_first(&_UCMs, &pIter);
    while(pUCM != NULL)
    {
        if (pUCM->pConfig->nID == xID)
        {
            return  pUCM;
        }
        
        pUCM = (FTE_UCM_PTR)fte_list_get_next(&_UCMs, &pIter);
    }

    return  NULL;
}

uint_32     FTE_UCM_getParent(FTE_UCM_PTR pUCM)
{
    ASSERT(pUCM != NULL);
    
    return  pUCM->nParent;
}

_mqx_uint       FTE_UCM_clear(FTE_UCM_PTR pUCM)
{
    FTE_UCS_PTR pUCS;
    
    ASSERT(pUCM != NULL);
    if (pUCM == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    pUCS = FTE_UCS_get(pUCM->pConfig->nUCSID);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    FTE_UCS_lock(pUCS);
    
    FTE_UCS_clear(pUCS);
    
    FTE_UCS_unlock(pUCS);
    
    return MQX_OK;
}

_mqx_uint   FTE_UCM_read(FTE_UCM_PTR pUCM, uint_8_ptr pBuff, uint_32 nBuffLen, uint_32_ptr pReadLen)
{
    FTE_UCS_PTR pUCS;
    uint_32 nReadLen;
    
    ASSERT((pUCM != NULL) && (pBuff != NULL));
    if (pUCM == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    pUCS = FTE_UCS_get(pUCM->pConfig->nUCSID);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    FTE_UCS_lock(pUCS);
    
    nReadLen = FTE_UCS_read(pUCS, pBuff, nBuffLen);
    
    FTE_UCS_unlock(pUCS);
    
    if (pReadLen != 0)
    {
        *pReadLen = nReadLen;
    }
    return MQX_OK;
}

_mqx_uint   FTE_UCM_write(FTE_UCM_PTR pUCM, uint_8_ptr pData, uint_32 nDataLen, uint_32_ptr pWrittenLen)
{
    FTE_UCS_PTR pUCS;
    uint_32 nWrittenLen;
    
    ASSERT((pUCM != NULL) && (pData != NULL));
    if (pUCM == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    pUCS = FTE_UCS_get(pUCM->pConfig->nUCSID);
    if (pUCS == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    FTE_UCS_lock(pUCS);
    
    nWrittenLen = FTE_UCS_write(pUCS, pData, nDataLen);
    
    FTE_UCS_unlock(pUCS);
    
    if (pWrittenLen != 0)
    {
        *pWrittenLen = nWrittenLen;
    }
    return MQX_OK;
}
