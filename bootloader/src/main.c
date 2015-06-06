/*
 * Purpose:		Main process
 *
 */
#include "common.h"
#include "fapp.h"

/********************************************************************/
void main (void)
{
	char ch;

    /* Init UART */
	fnet_cpu_serial_init(FNET_CFG_CPU_SERIAL_PORT_DEFAULT, 115200);
    fnet_cpu_irq_enable(0);
    
    
    FTE_BL_main();
}

/********************************************************************/
