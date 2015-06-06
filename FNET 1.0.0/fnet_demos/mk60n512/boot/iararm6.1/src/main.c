/*
 * File:		hello_world.c
 * Purpose:		Main process
 *
 */

#include "common.h"

#include "fapp.h"

/********************************************************************/
void main (void)
{
	char ch;
                
//  	printf("\nHello World!!\n");
#if 0
	while(1)
	{


                if( (ch = fnet_getchar())!= (char)FNET_ERR)
                    fnet_putchar(ch);
               
	} 
        
      #endif   
        /* Init UART */
	fnet_cpu_serial_init(FNET_CFG_CPU_SERIAL_PORT_DEFAULT, 115200);

#if 0    
    //DM
    {
       uint32 timeout;   
      /* Initialize the PIT timer to generate an interrupt every 15s */

        /* Enable the clock to the PIT module */
        SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

        /* Enable the PIT timer module */
        PIT_MCR &= ~PIT_MCR_MDIS_MASK;
        
        /* Calculate the timeout value to get a 5s interval */
        timeout = 5 * (FNET_CPU_CLOCK_KHZ / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1)) * 1000;
        PIT_LDVAL0 = timeout;

        /* Enable the timer and enable interrupts */
        PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK;

        /* Enable the PIT timer interrupt in the NVIC */
        enable_irq(68); 
      
      
    }
#endif    
    

    fnet_cpu_irq_enable(0);
    
    /* Run application */
    fapp_main();

}


//DM
/********************************************************************/
/* PIT0 Interrupt Service Routine
 *
 * Updates the counters in EEPROM
 *
 */
void pit0_isr(void)
{
    uint32 temp;

    fnet_printf("\n\nPIT0 interrupt handler.");

    /* Clear the PIT timer flag */
    PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

    /* Read the load value to restart the timer */
    PIT_LDVAL0;

#if 0
    /* Increment all three counters */

    /* Make sure the EEE is ready. If not wait for the command to complete */
    while(!(FTFL_FCNFG & FTFL_FCNFG_EEERDY_MASK));

    temp = *((uint32 *)(LONGWORD_COUNTER_ADDR));
    *((uint32 *)(LONGWORD_COUNTER_ADDR)) = (uint32) temp + 1;

    /* Make sure the EEE is ready. If not wait for the command to complete */
    while(!(FTFL_FCNFG & FTFL_FCNFG_EEERDY_MASK));

    temp = *((uint16 *)(WORD_COUNTER_ADDR));
    *((uint16 *)(WORD_COUNTER_ADDR)) = (uint16) temp + 1;

    /* Make sure the EEE is ready. If not wait for the command to complete */
    while(!(FTFL_FCNFG & FTFL_FCNFG_EEERDY_MASK));

    temp = *((uint8 *)(BYTE_COUNTER_ADDR));
    *((uint8 *)(BYTE_COUNTER_ADDR)) = (uint8) temp + 1;

    /* Make sure the EEE is ready. If not wait for the command to complete */
    while(!(FTFL_FCNFG & FTFL_FCNFG_EEERDY_MASK));

    /* Display the new counter values */
    printf("\nlongword counter = 0x%08X", *(uint32 *)(LONGWORD_COUNTER_ADDR));
    printf("\nword counter = 0x%04X", *(uint16 *)(WORD_COUNTER_ADDR));
    printf("\nbyte counter = 0x%02X", *(uint8 *)(BYTE_COUNTER_ADDR));
#endif
    return;
}

/********************************************************************/
