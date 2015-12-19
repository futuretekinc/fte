#ifndef __XTYPE_H__
#define __XTYPE_H__

#include <mqx.h>
#include "fte_target.h"
#include "lwmsgq.h"
#include "fte_list.h"

#define HAVE_DYNAMIC_BUFFERS    1
#define HAVE_MAC_15_4           1

#define SHORT_ADDRESS   0x0102
#define MODULE_RF_802_15_4  1

typedef struct
{
    uint_32     ulSize;
    FTE_LIST    xFreeList;
    FTE_LIST    xQueue;
}   FTE_QUEUE, _PTR_ FTE_QUEUE_PTR;

typedef FTE_QUEUE_PTR   xQueueHandle;
typedef _task_id    xTaskHandle;
#define portTickType    uint32_t   
typedef uint32_t    portBASE_TYPE;
#define portCHAR    char 

#define vTaskDelay  _time_delay

#define portDISABLE_INTERRUPTS()    
#define portENABLE_INTERRUPTS()

#define taskENTER_CRITICAL()
#define taskEXIT_CRITICAL()

typedef FTE_LIST xList;
#define vListInitialise(x)  FTE_LIST_init(x)

xQueueHandle xQueueCreate(uint32_t, uint32_t);
uint32_t xQueueSend(xQueueHandle, pointer, uint32_t );
uint32_t xQueueSendFromISR(xQueueHandle, pointer, uint32_t );
_mqx_uint xQueueReceive(xQueueHandle, pointer, uint32_t);

#define pvPortMalloc(nSize)    FTE_MEM_allocZero(nSize)
#define vPortFree(ptr)          FTE_MEM_free(ptr)
#define portTICK_RATE_MS    1
#define RF_DEFAULT_CHANNEL  10

#define pdFAIL  1

#define RXFIFOCNT   256

#endif