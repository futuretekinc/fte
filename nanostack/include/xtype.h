#ifndef __XTYPE_H__
#define __XTYPE_H__

#include <mqx.h>
#include "target.h"
#include "lwmsgq.h"
#include "fte_list.h"

typedef LWMSGQ_STRUCT_PTR   xQueueHandle;
typedef _task_id    xTaskHandle;
typedef uint32_t    portTickType;
typedef uint32_t    portBASE_TYPE;
typedef char        portCHAR;

#define vTaskDelay  _time_delay

#define portDISABLE_INTERRUPTS()    
#define portENABLE_INTERRUPTS()

typedef FTE_LIST xList;
#define vListInitialise(x)  FTE_LIST_init(x)

xQueueHandle xQueueCreate(uint32_t, uint32_t);
uint32_t xQueueSend(xQueueHandle, pointer, uint32_t );
uint32_t xQueueSendFromISR(xQueueHandle, pointer, uint32_t );
_mqx_uint xQueueReceive(xQueueHandle, pointer, uint32_t);

#define portTICK_RATE_MS    1
#define RF_DEFAULT_CHANNEL  10

#endif