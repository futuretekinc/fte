#include <mqx.h>
#include "fte_mem.h"
#include "address.h"
#include "xtype.h"
#include "buffer.h"
#include "rf.h"


mac_event_t mac_control(buffer_t **ppbuf)
{
        return  MAC_NONE;
}

xQueueHandle xQueueCreate(uint32_t ulCount, uint32_t ulSize)
{
    FTE_QUEUE_PTR   pQueue = FTE_MEM_allocZero(sizeof(FTE_QUEUE));
    if (pQueue == NULL)
    {
        return  NULL;
    }

    pQueue->ulSize = ulSize;
    FTE_LIST_init(&pQueue->xFreeList);
    FTE_LIST_init(&pQueue->xQueue);
    
    for(int i = 0 ; i < ulCount ; i++)
    {
        void *pData = FTE_MEM_allocZero(ulSize);
        if (pData != NULL)
        {
            FTE_LIST_pushBack(&pQueue->xFreeList, pData);
        }
    }
    
    return  pQueue;
}

uint32_t xQueueSend(xQueueHandle xHandle, pointer pData, uint32_t ulFlag)
{
    pointer pItem;
   
    if (FTE_LIST_popFront(&xHandle->xFreeList, &pItem) == MQX_OK)
    {
        memcpy(pItem, pData, xHandle->ulSize);
        FTE_LIST_pushBack(&xHandle->xQueue, pItem);
        
        return  1;
    }
    
    return  0;
}

uint32_t xQueueSendFromISR(xQueueHandle xHandle, pointer pData, uint32_t ulFlag)
{
    return  xQueueSend(xHandle, pData, ulFlag);
}

FTE_RET xQueueReceive(xQueueHandle xHandle, pointer pBuff, uint32_t ulFlag)
{
    pointer pItem;

    if (FTE_LIST_count(&xHandle->xQueue) == 0)
    {
        return  0;
    }
    
    FTE_LIST_popFront(&xHandle->xQueue, &pItem);
    memcpy(pBuff, pItem, xHandle->ulSize);
    
    FTE_LIST_pushBack(&xHandle->xFreeList, pItem);
    
    return  1;
}


uint32_t random_generate(uint32_t ulSeed)
{
    return  0;
}