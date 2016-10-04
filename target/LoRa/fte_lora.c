#include <string.h>
#include "board.h"
#include "utilities.h"
#include "radio.h"
#include "target.h"
#include "LoRaMac.h"

void SX1276OnDio0Irq(void *);
void SX1276OnDio1Irq(void *);
void SX1276OnDio2Irq(void *);
void SX1276OnDio3Irq(void *);

void FTE_LORA_ctrl(uint_32 params)
{
    LoRaMac_t *pMac = (LoRaMac_t *)params;
    
    if (pMac == NULL)
    {
        return;
    }
    
    boolean bPrevValue[4] = {FALSE,FALSE,FALSE,FALSE};
    boolean bValue[4] = {FALSE,FALSE,FALSE,FALSE};
    
    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    while(TRUE)
    {
        FTE_LWGPIO_getValue(SX1276.pDIO0, &bValue[0]);
        if (bPrevValue[0] != bValue[0])
        {
            if (bPrevValue[0] == FALSE)
            {
                SX1276OnDio0Irq(pMac);
            }
            bPrevValue[0] = bValue[0];
        }
        
        FTE_LWGPIO_getValue(SX1276.pDIO1, &bValue[1]);
        if (bPrevValue[1] != bValue[1])
        {
            if (bPrevValue[1] == FALSE)
            {
                SX1276OnDio1Irq(pMac);
            }
            bPrevValue[1] = bValue[1];
        }
        
        FTE_LWGPIO_getValue(SX1276.pDIO2, &bValue[2]);
        if (bPrevValue[2] != bValue[2])
        {
            if (bPrevValue[2] == FALSE)
            {
                SX1276OnDio2Irq(pMac);
            }
            bPrevValue[2] = bValue[2];
        }
        
        FTE_LWGPIO_getValue(SX1276.pDIO3, &bValue[3]);
        if (bPrevValue[3] != bValue[3])
        {
            if (bPrevValue[3] == FALSE)
            {
                SX1276OnDio3Irq(pMac);
            }
            bPrevValue[3] = bValue[3];
        }
        
        _time_delay(0);
    }
    
}
