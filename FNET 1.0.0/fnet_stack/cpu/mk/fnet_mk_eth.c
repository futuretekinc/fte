/**************************************************************************
*
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License 
* Version 3 or later (the "GPL"), or the GNU Lesser General Public 
* License Version 3 or later (the "LGPL").
*
* As a special exception, the copyright holders of the FNET project give you
* permission to link the FNET sources with independent modules to produce an
* executable, regardless of the license terms of these independent modules,
* and to copy and distribute the resulting executable under terms of your 
* choice, provided that you also meet, for each linked independent module,
* the terms and conditions of the license of that module.
* An independent module is a module which is not derived from or based 
* on this library. 
* If you modify the FNET sources, you may extend this exception 
* to your version of the FNET sources, but you are not obligated 
* to do so. If you do not wish to do so, delete this
* exception statement from your version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* and the GNU Lesser General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_mk_eth.c
*
* @author Andrey Butok
*
* @date May-31-2011
*
* @version 0.1.5.0
*
* @brief Ethernet driver interafce.
*
***************************************************************************/

#include "fnet_config.h"
#if FNET_MK && FNET_CFG_ETH

#include "fnet_fec.h"


/************************************************************************
* Ethernet interface structure.
*************************************************************************/
fnet_netif_t fnet_eth0_if =
{
   0,                           /* pointer to the next net_if structure.*/
   0,                           /* pointer to the previous net_if structure.*/
   FNET_NETIF_TYPE_ETHERNET,    /* data-link type.*/
   "eth0",                      /* network interface name.*/
   {0,0,0,0,0,0,0,0},           /* the interface address structure.*/
   FNET_CFG_ETH_MTU,            /* maximum transmission unit.*/
   0,                           /* points to interface specific data structure.*/
   fnet_fec_init,               /* initialization function.*/
   fnet_fec_release,            /* shutdown function.*/
   fnet_fec_input,              /* receive function.*/
   fnet_fec_output,             /* transmit function.*/
   fnet_fec_set_addr_notify,    /* address change notification function.*/
   fnet_fec_drain,              /* drain function.*/
   fnet_fec_get_mac,
   fnet_fec_set_mac,
   fnet_fec_is_connected,
   fnet_fec_get_statistics
};

/************************************************************************
* NAME: fnet_eth_io_init
*
* DESCRIPTION: Ethernet IO initialization.
*************************************************************************/
void fnet_eth_io_init() 
{
#if FNET_CFG_CPU_MK60N512
  
    FNET_MK_PORT_MemMapPtr pctl;
    FNET_MK_SIM_MemMapPtr  sim  = (FNET_MK_SIM_MemMapPtr)FNET_MK_SIM_BASE_PTR;
    unsigned long   *pDirReg = (unsigned long   *)0x400FF054;
    unsigned long   *pOutputReg = (unsigned long   *)0x400FF040;
#if 1   
    pctl = (FNET_MK_PORT_MemMapPtr)FNET_MK_PORTB_BASE_PTR;    
    pctl->PCR[3]  = 0x00000100;     /* PTB3, GPIO */
    *pDirReg = *pDirReg | (0x1 << 3);
    *pOutputReg = *pOutputReg & ~(0x1 << 3);
    fnet_timer_delay(1);
    *pOutputReg = *pOutputReg | (0x1 << 3);
    fnet_timer_delay(1);
#endif

    pctl = (FNET_MK_PORT_MemMapPtr)FNET_MK_PORTA_BASE_PTR;    
    pctl->PCR[5]  = 0x00000400;     /* PTA5, MII0_RXER      */
    pctl->PCR[9]  = 0x00000400;     /* PTA9, MII0_RXD3      */
    pctl->PCR[10] = 0x00000400;     /* PTA10, MII0_RXD2      */
    pctl->PCR[11] = 0x00000400;     /* PTA10, MII0_RXCLK     */
    pctl->PCR[12] = 0x00000400;     /* PTA12, RMII0_RXD1/MII0_RXD1      */
    pctl->PCR[13] = 0x00000400;     /* PTA13, RMII0_RXD0/MII0_RXD0      */
    pctl->PCR[14] = 0x00000400;     /* PTA14, RMII0_CRS_DV/MII0_RXDV    */
    pctl->PCR[15] = 0x00000400;     /* PTA15, RMII0_TXEN/MII0_TXEN      */
    pctl->PCR[16] = 0x00000400;     /* PTA16, RMII0_TXD0/MII0_TXD0      */
    pctl->PCR[17] = 0x00000400;     /* PTA17, RMII0_TXD1/MII0_TXD1      */
    
    pctl->PCR[24] = 0x00000400;     /* PTA17, MII0_TXD2      */
    pctl->PCR[25] = 0x00000400;     /* PTA17, MII0_TXCLK     */
    pctl->PCR[26] = 0x00000400;     /* PTA17, MII0_TXD3      */
    pctl->PCR[27] = 0x00000400;     /* PTA17, MII0_CRS       */
    pctl->PCR[29] = 0x00000400;     /* PTA17, MII0_COL      */
    
    pctl = (FNET_MK_PORT_MemMapPtr)FNET_MK_PORTB_BASE_PTR;
    pctl->PCR[0] = FNET_MK_PORT_PCR_MUX(4) | FNET_MK_PORT_PCR_ODE_MASK; /* PTB0, RMII0_MDIO/MII0_MDIO   */
    pctl->PCR[1] = FNET_MK_PORT_PCR_MUX(4);                     /* PTB1, RMII0_MDC/MII0_MDC     */
    
    /* Enable clock for ENET module */
    sim->SCGC2 |= FNET_MK_SIM_SCGC2_ENET_MASK;
    
    /*FSL: allow concurrent access to MPU controller. Example: ENET uDMA to SRAM, otherwise bus error*/
    FNET_MK_MPU_CESR = 0;  /* MPU is disabled. All accesses from all bus masters are allowed.*/

    
#endif  

}



/************************************************************************
* NAME: fnet_eth_phy_init
*
* DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
*************************************************************************/
void fnet_eth_phy_init(fnet_fec_if_t *ethif) 
{
    FNET_COMP_UNUSED_ARG(ethif);
}



#endif /* FNET_MK && FNET_CFG_ETH */



