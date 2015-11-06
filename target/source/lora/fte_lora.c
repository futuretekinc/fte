#include "fte_target.h"
#include "radio.h"
#include "fte_lora.h"
#include "fte_lora_utils.h"
#include <stdbool.h>
#include "sx1276.h"
#include "sx1276-LoRa.h"
#include "sx1276-LoRaMisc.h"
#include "fte_buff.h"

tRadioDriver*       pRadio = NULL;
FTE_LIST            xRcvdList;
FTE_LIST            xSendList;
FTE_FBM_PTR         pFBM = NULL;

_mqx_uint   FTE_LORA_init(void)
{
    FTE_LIST_init(&xRcvdList);
    FTE_LIST_init(&xSendList);
    
    pFBM = FTE_FBM_create(RF_BUFFER_SIZE_MAX, 8);

    pRadio = RadioDriverInit( );

    pRadio->Init( );
    
    pRadio->StartRx( );
    
    FTE_LORA_init();
    
    _task_create(0, FTE_TASK_LORA_COMM, 0);
    _task_create(0, FTE_TASK_LORA, 0);

    return  MQX_OK;
}

void FTE_LORA_comm(uint_32 params)
{
    uint_8  pBuffer[RF_BUFFER_SIZE_MAX];
    uint_16 usBufferSize = RF_BUFFER_SIZE_MAX;
    uint_32  ulSendCount = 0;
    
    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    
    while(1)
    {
        switch( pRadio->Process( ) )
        {
        case    RF_IDLE:
            {
                if (FTE_LIST_count(&xSendList) == 0)
                {
                    pRadio->StartRx();
                    ulSendCount = 0;
                }
                else
                {
                    FTE_FBM_BUFF_PTR pBlock;
                    if (FTE_LIST_popFront(&xSendList, (pointer *)&pBlock) == MQX_OK)
                    {
                        pRadio->SetTxPacket(pBlock->pBuff, pBlock->ulSize);
                        ulSendCount++;
                        FTE_FBM_free(pFBM, pBlock);
                    }
                }
            }
            break;
             
        case    RF_BUSY:
            {
            }
            break;
            
        case    RF_RX_DONE:
            {
                pRadio->GetRxPacket( pBuffer, ( uint16_t* )&usBufferSize );
                if (usBufferSize != 0)
                {
                    FTE_FBM_BUFF_PTR pBlock = FTE_FBM_alloc(pFBM, usBufferSize);
                    if (pBlock != NULL)
                    {
                        memcpy(pBlock->pBuff, pBuffer, usBufferSize);
                        pBlock->ulSize = usBufferSize;
                        FTE_LIST_pushBack(&xRcvdList, pBlock);
                    }                    
                }
                
                if (FTE_LIST_count(&xSendList) == 0)
                {
                    pRadio->StartRx();
                }
                else
                {
                    FTE_FBM_BUFF_PTR pBlock;
                    if (FTE_LIST_popFront(&xSendList, (pointer *)&pBlock) == MQX_OK)
                    {
                        pRadio->SetTxPacket(pBlock->pBuff, pBlock->ulSize);
                        ulSendCount++;
                        FTE_FBM_free(pFBM, pBlock);
                    }
                }
            }
            break;
            
        case    RF_RX_TIMEOUT:
            {
                ulSendCount = 0;
            }
            break;
            
        case    RF_TX_DONE:
            {
                if ((FTE_LIST_count(&xSendList) == 0) || (ulSendCount > 4))
                {
                    pRadio->StartRx();
                    ulSendCount = 0;
                }
                else
                {
                    FTE_FBM_BUFF_PTR pBlock;
                    if (FTE_LIST_popFront(&xSendList, (pointer *)&pBlock) == MQX_OK)
                    {
                        pRadio->SetTxPacket(pBlock->pBuff, pBlock->ulSize);
                        ulSendCount++;
                        FTE_FBM_free(pFBM, pBlock);
                    }
                }
            }
            break;
            
        case    RF_TX_TIMEOUT:
            {
            }
            break;
            
        case    RF_LEN_ERROR:
            {
            }
            break;
            
        case    RF_CHANNEL_EMPTY:
            {
            }
            break;
            
        case    RF_CHANNEL_ACTIVITY_DETECTED:
            {
            }
            break;
        }

        _time_delay(1);
    }      
}

void FTE_LORA_process(uint_32 params)
{
    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    while(1)
    {
        if (FTE_LIST_count(&xRcvdList) != 0)
        {
            FTE_FBM_BUFF_PTR pBlock;
            if (FTE_LIST_popFront(&xRcvdList, (pointer *)&pBlock) == MQX_OK)
            {
                FTE_FBM_free(pFBM, pBlock);
            }
        }

        _time_delay(1);
    }      
}

int_32  FTE_LORA_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean     print_usage, shorthelp = FALSE;
    int_32      return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {                
                double  dRssi;
                    printf("%16s : %s\n", "Op Mode", FTE_LORA_getOpModeString(SX1276LoRaGetOpMode( )));
                    printf("%16s : %d\n", "RF Frequency", SX1276LoRaGetRFFrequency( ));
                    printf("%16s : %d\n", "Spreading Factor", SX1276LoRaGetSpreadingFactor( )); // SF6 only operates in implicit header mode.
                    printf("%16s : %d\n", "Error Coding", SX1276LoRaGetErrorCoding( ));
                    printf("%16s : %d\n", "Packet CRC ON", SX1276LoRaGetPacketCrcOn( ));
                    printf("%16s : %d\n", "Bandwidth", SX1276LoRaGetSignalBandwidth( ));
                    dRssi = SX1276ReadRssi( );
                    printf("%16s : %d\n", "RSSI", (int_32)dRssi);
            }
            break;
            
        case    2:
            {
            }
            break;
            
        default:
            print_usage = TRUE;
        }
        
    }
    
   
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<command>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<command>]\n", argv[0]);
            printf("  Commands:\n");
            printf("  Parameters:\n");
        }
    }
    return   return_code;
}
