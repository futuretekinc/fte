#include <stdbool.h>
#include "fte_target.h"
#include "fte_lora.h"
#include "fte_lora_utils.h"
#include "fte_buff.h"
#include "fte_debug.h"
#include "sx1276-Hal.h"
#include "sx1276-LoRa.h"
#include "sx1276-LoRaMisc.h"

#include "xtype.h"
#include "buffer.h"
#include "stack.h"

FTE_LIST            xRcvdList;
FTE_LIST            xSendList;
FTE_FBM_PTR         pFBM = NULL;

FTE_RET   FTE_LORA_init(void)
{
    FTE_LIST_init(&xRcvdList);
    FTE_LIST_init(&xSendList);
    
    pFBM = FTE_FBM_create(RF_BUFFER_SIZE_MAX, 8);

    SX1276InitIo( );
    
    SX1276LoRaInit( );
   
    FTE_TASK_create(FTE_TASK_LORAWAN_CTRL, 0, NULL);
    FTE_TASK_create(FTE_TASK_LORA, 0, NULL);

    return  FTE_RET_OK;
}

void FTE_LORA_ctrl(FTE_UINT32 params)
{
    FTE_UINT8      pInternalBuffer[RF_BUFFER_SIZE_MAX];
    FTE_UINT16     usBufferSize = RF_BUFFER_SIZE_MAX;
    
    SX1276LoRaStartRx();   
    while(TRUE)
    {
        FTE_UINT32  ulState;
        FTE_BOOL    bTxON = FALSE;
         
        ulState = SX1276LoRaProcess();
       
        switch( ulState )
        {
        case    RF_RX_DONE:
            {
                SX1276LoRaGetRxPacket( pInternalBuffer, ( uint16_t* )&usBufferSize );
                if (usBufferSize != 0)
                {
#if 1                    
                    FTE_FBM_BUFF_PTR pBlock = FTE_FBM_alloc(pFBM, usBufferSize);
                    if (pBlock != NULL)
                    {
                        memcpy(pBlock->pBuff, pInternalBuffer, usBufferSize);
                        pBlock->ulSize = usBufferSize;
                        FTE_LIST_pushBack(&xRcvdList, pBlock);
                    }                    
#else
                    buffer_t *pBuffer = stack_buffer_allocate(usBufferSize);
                    
                    memcpy(pBuffer->buf, pInternalBuffer, usBufferSize);
                    pBuffer->dir = BUFFER_UP;
                    pBuffer->size = usBufferSize;
                    
			#ifdef HAVE_MAC_15_4
                    mac_push(pBuffer);
					mac_rx_push();
			#else
                    {
                        event_t event;
						event.process = 0;
						event.param = (void *) pBuffer;
				
						xQueueSendFromISR( events, ( void * ) &event, 0);
                    }
			#endif
                    
#endif
                }
            }
        case    RF_IDLE:
            {
                DEBUG("RF_IDLE\n");
             //   bTxON = TRUE;
            }
            break;
            
        case    RF_BUSY:
            {
            }
            break;

        case    RF_RX_TIMEOUT:
            {
                DEBUG("RF_RX_TIMEOUT\n");
                bTxON = TRUE;
            }
            break;

        case    RF_TX_DONE:
            {
                DEBUG("RF_TX_DONE\n");
                bTxON = TRUE;
            }
            break;
            
        case    RF_TX_TIMEOUT:
            {
                DEBUG("RF_TX_TIMEOUT\n");
            }
            break;
            
        case    RF_LEN_ERROR:
            {
                DEBUG("RF_LEN_ERROR\n");
            }
            break;
            
        case    RF_CHANNEL_EMPTY:
            {
                DEBUG("Channel Empty\n");
            }
            break;
            
        case    RF_CHANNEL_ACTIVITY_DETECTED:
            {
                DEBUG("RF_CHANNEL_ACTIVITY_DETECTED\n");
            }
            break;
        }
        
        if (bTxON && (FTE_LIST_count(&xSendList) != 0))
        {
            FTE_FBM_BUFF_PTR pBlock;
            if (FTE_LIST_popFront(&xSendList, (pointer *)&pBlock) == FTE_RET_OK)
            {
                SX1276LoRaSetTxPacket(pBlock->pBuff, pBlock->ulSize);
                FTE_FBM_free(pFBM, pBlock);
            }            
        }
        
        _time_delay(10);
    }      
}

void FTE_LORA_process(FTE_UINT32 params)
{
    while(TRUE)
    {
        if (FTE_LIST_count(&xRcvdList) != 0)
        {
            FTE_FBM_BUFF_PTR pBlock;
            if (FTE_LIST_popFront(&xRcvdList, (pointer *)&pBlock) == FTE_RET_OK)
            {
                pBlock->pBuff[pBlock->ulSize] = 0;
                DEBUG("Packet Received : %s\n", pBlock->pBuff);
                
                FTE_FBM_free(pFBM, pBlock);
            }
        }

        _time_delay(1);
    }      
}

FTE_RET FTE_LORA_send(FTE_UINT8_ptr pData, FTE_UINT32 ulDataSize)
{
    FTE_FBM_BUFF_PTR pBlock = FTE_FBM_alloc(pFBM, ulDataSize);
    if (pBlock == NULL)
    {
        return  MQX_NOT_ENOUGH_MEMORY;
    }
    
    memcpy(pBlock->pBuff, pData, ulDataSize);
    pBlock->ulSize = ulDataSize;
    FTE_LIST_pushBack(&xSendList, pBlock);
    
    return  FTE_RET_OK;
}

FTE_INT32  FTE_LORA_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {                
#if 0
                printf("%16s : %s\n", "Op Mode", FTE_LORA_getOpModeString(SX1276LoRaGetOpMode( )));
                printf("%16s : %d\n", "RF Frequency", SX1276LoRaGetRFFrequency( ));
                printf("%16s : %d\n", "Spreading Factor", SX1276LoRaGetSpreadingFactor( )); // SF6 only operates in implicit header mode.
                printf("%16s : %d\n", "Error Coding", SX1276LoRaGetErrorCoding( ));
                printf("%16s : %d\n", "Packet CRC ON", SX1276LoRaGetPacketCrcOn( ));
                printf("%16s : %d\n", "Bandwidth", SX1276LoRaGetSignalBandwidth( ));
                printf("%16s : %d dBm\n", "RSSI", (FTE_INT32)SX1276LoRaGetPacketRssi( ));
                printf("%16s : %d\n", "Rx Packets", SX1276LoRaGetRxPacketCount());
                printf("%16s : %d\n", "Tx Packets", SX1276LoRaGetTxPacketCount());
#else
                for(int i = 0 ; i < 256 ; i++)
                {
                    FTE_UINT8 bReg;
                   
                   SX1276Read( i & 0xFF, &bReg);
                    printf("%02x ", bReg);
                    if (((i + 1) % 16) == 0)
                    {
                        printf("\n");
                    }
                }
#endif
            }
            break;
            
        case    3:
            {
                if (strcmp(pArgv[1], "send") == 0)
                {
                    FTE_LORA_send((FTE_UINT8_ptr)pArgv[2], strlen(pArgv[2]));
                }
            }
            break;
            
        default:
            bPrintUsage = TRUE;
        }
        
    }
    
   
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<command>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<command>]\n", pArgv[0]);
            printf("  Commands:\n");
            printf("  Parameters:\n");
        }
    }
    return   xRet;
}
