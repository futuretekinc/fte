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
* @file fnet_mcf_eth.c
*
* @author Andrey Butok
*
* @date May-31-2011
*
* @version 0.1.29.0
*
* @brief Ethernet driver implementation.
*
***************************************************************************/

#include "fnet_config.h"
#if FNET_MCF && FNET_CFG_ETH
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
#if FNET_CFG_CPU_MCF523X

    FNET_MCF523x_GPIO_PAR_FECI2C = 0xF0; 

#endif

#if FNET_CFG_CPU_MCF52235 /* Kirin2 */

    FNET_MCF5223X_GPIO_PLDPAR = (0
                | FNET_MCF5223X_GPIO_PLDPAR_ACTLED_ACTLED
                | FNET_MCF5223X_GPIO_PLDPAR_LINKLED_LINKLED
                | FNET_MCF5223X_GPIO_PLDPAR_SPDLED_SPDLED
                | FNET_MCF5223X_GPIO_PLDPAR_DUPLED_DUPLED
                | FNET_MCF5223X_GPIO_PLDPAR_COLLED_COLLED
                | FNET_MCF5223X_GPIO_PLDPAR_RXLED_RXLED
                | FNET_MCF5223X_GPIO_PLDPAR_TXLED_TXLED);

#endif

#if FNET_CFG_CPU_MCF52259 /* Kirin3 */

    FNET_MCF5225X_GPIO_PNQPAR = 0x4880; // 0x4880 (for Gamma environment, reported by a customer).
    FNET_MCF5225X_GPIO_PTIPAR = 0xFF;
    FNET_MCF5225X_GPIO_PTJPAR = 0xFF;

#endif

#if FNET_CFG_CPU_MCF5282 /* Reindeer */

    FNET_MCF5282_GPIO_PEHLPAR = 0xC0;
    FNET_MCF5282_GPIO_PASPAR |=0x0f00; /* Initialize PEHLPAR to enable ethernet signals.*/

#endif

#if FNET_CFG_CPU_MCF51CN128 /* Lasko */

  /* Enable MII pins by setting Pin Mux Control Registers: */
  /* Port A */
  FNET_MCF_PTAPF1 = 0x55;    /* MII_RXD2, MII_RXD1, MII_RXD0, MII_RX_DV */  
  FNET_MCF_PTAPF2 = 0x54;    /* MII_MDIO, MII_MDC, MII_RXD3 */ 
  /* Port B */
  FNET_MCF_PTBPF1 = 0x55;    /* MII_TX_EN, MII_TXD0, MII_TXD1, MII_TXD2 */
  FNET_MCF_PTBPF2 = 0x55;    /* MII_RX_CLK, MII_RX_ER, MII_TX_ER, MII_TX_CLK */
  /* Port C */
  FNET_MCF_PTCPF2 = 0x15;    /* MII_TXD3, MII_COL, MII_CRS */
  
  /* Set high output drive strength for output pins: */
  FNET_MCF_PTADS = 0x6;  /* Output Drive Enable for Port A*/ 
  FNET_MCF_PTBDS = 0xF4; /* Output Drive Enable for Port B*/
  FNET_MCF_PTCDS = 0x1;  /* Output Drive Enable for Port C*/  
  
  /* Enable filter. */
  FNET_MCF_PTAIFE = 0x06;
  FNET_MCF_PTBIFE = 0xf4;
  FNET_MCF_PTCIFE = 1;
      
#endif

}


/************************************************************************
* NAME: fnet_eth_phy_init
*
* DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
*************************************************************************/
void fnet_eth_phy_init(fnet_fec_if_t *ethif) 
{
   
    #if FNET_CFG_CPU_MCF52235 
        #if 1
            fnet_uint16 reg_value;
            /* Enable EPHY module, Enable auto_neg at start-up, Let PHY PLLs be determined by PHY.*/
            FNET_MCF_EPHY_EPHYCTL0 = (fnet_uint8)(FNET_MCF_EPHY_EPHYCTL0_EPHYEN);
     
            /* Start-up Delay for Kirin2 = 350uS */
            fnet_timer_delay(1); /* Delay for 1 timer tick (100ms) */
             
            /* Disable ANE that causes problems with some routers.  Enable full-duplex and 100Mbps */
            fnet_fec_mii_read(ethif, FNET_FEC_MII_REG_CR, &reg_value);
            fnet_fec_mii_write(ethif, FNET_FEC_MII_REG_CR, (fnet_uint16)(reg_value & (~FNET_FEC_MII_REG_CR_ANE)|FNET_FEC_MII_REG_CR_DPLX|FNET_FEC_MII_REG_CR_DATARATE));
        
        #else /* Old version, just in case.*/ 
            fnet_uint16 reg_value;
             
            /* Set phy address */
            FNET_MCF_EPHY_EPHYCTL1 = (fnet_uint8)FNET_MCF_EPHY_EPHYCTL1_PHYADD(ethif->phy_addr);

            /* Disable device, PHY clocks disabled, Enable autonigatiation, Turn on leds.*/
            FNET_MCF_EPHY_EPHYCTL0 = FNET_MCF_EPHY_EPHYCTL0_DIS100 | FNET_MCF_EPHY_EPHYCTL0_DIS10 | FNET_MCF_EPHY_EPHYCTL0_LEDEN ;
           
            /* Enable EPHY module.*/
            FNET_MCF_EPHY_EPHYCTL0 |= FNET_MCF_EPHY_EPHYCTL0_EPHYEN;
          
            /* Reset PHY.*/
            fnet_timer_delay(10);
            fnet_fec_mii_write(ethif, FNET_FEC_MII_REG_CR, FNET_ETH_MII_REG_CR_RESET);
       
            /* SECF128: EPHY Incorrectly Advertises It Can Receive Next Pages
            * Description:The EPHY from reset incorrectly advertises that it can receive next pages
            *    from the link partner. These next pages are most often used to send gigabit
            *    Ethernet ability information between link partners. This device is 10/100 Mbit
            *    only, so there is no need to advertise this capability. In fact if advertised this
            *    additional pages of information must be handled in a special manor not typical
            *    of 10/100 Ethernet drivers.
            * Workaround: The NXTP bit in the auto-negotiate (A/N) advertisement register (4.15) should
            *    be cleared as soon as possible after reset; ideally before enabling
            *    auto-negotiation.
            */
            if (fnet_fec_mii_read(ethif, FNET_FEC_MII_REG_ANAR, &reg_value))
            {
                anar_value &= (~ ( FNET_ETH_MII_REG_ANAR_NEXT_PAGE));/* Turn off next page mode.*/
                fnet_fec_mii_write(ethif, FNET_FEC_MII_REG_ANAR, reg_value); 
            }
        
             /* Turns clocks on. Let PHY PLLs be determined by PHY. */
            FNET_MCF_EPHY_EPHYCTL0 &= (~(FNET_MCF_EPHY_EPHYCTL0_DIS100 | FNET_MCF_EPHY_EPHYCTL0_DIS10)); 
        #endif
    #endif
    
    #if FNET_CFG_CPU_MCF52259
        fnet_uint8 tmp_ptipar;
        fnet_uint8 tmp_ptjpar;
        
        fnet_timer_delay(3);
        /* Workarround for PHY reset */
        tmp_ptipar = FNET_MCF5225X_GPIO_PTIPAR; /* Save current state */
        tmp_ptjpar = FNET_MCF5225X_GPIO_PTJPAR;	
        FNET_MCF5225X_GPIO_PTIPAR = 0x00;		/* Ethernet signals now GPIO*/
        FNET_MCF5225X_GPIO_PTJPAR = 0x00;		
        FNET_MCF5225X_GPIO_DDRTI = 0xFF;		/* GPIO output.*/
        FNET_MCF5225X_GPIO_DDRTJ = 0xFF;		
        FNET_MCF5225X_GPIO_PORTTI = 0x00;		/* Force Ethernet signals low.*/
        FNET_MCF5225X_GPIO_PORTTJ = 0x00;		
        
         /* Reset PHY.*/
        fnet_fec_mii_write(ethif, FNET_FEC_MII_REG_CR, FNET_FEC_MII_REG_CR_RESET);
        fnet_timer_delay(5);

        /* RSTO signal.*/
        FNET_MCF_RCM_RCR |= FNET_MCF_RCM_RCR_FRCRSTOUT;     /* Assert RSTO.*/
        fnet_timer_delay(5); 
        FNET_MCF_RCM_RCR &= ~(FNET_MCF_RCM_RCR_FRCRSTOUT);  /* Negate RSTO.*/
        
        FNET_MCF5225X_GPIO_PTIPAR = tmp_ptipar;		/* Restore Ethernet signals.*/
        FNET_MCF5225X_GPIO_PTJPAR = tmp_ptjpar;	
    #endif
    
    FNET_COMP_UNUSED_ARG(ethif);
   
}


#endif /* FNET_MCF && FNET_CFG_ETH */


