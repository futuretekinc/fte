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
* @file fnet_fec.c
*
* @author Andrey Butok
*
* @date May-31-2011
*
* @version 0.1.3.0
*
* @brief FEC module driver implementation.
*
***************************************************************************/

#include "fnet_config.h"
#if (FNET_MCF || FNET_MK) && FNET_CFG_ETH

#include "fnet_fec.h"


FNET_COMP_PACKED_BEGIN
/* FEC buffers structure. */
typedef struct 
{   
    fnet_uint8 tx_buf_desc[(FNET_FEC_TX_BUF_NUM * sizeof(fnet_fec_buf_desc_t)) + (FNET_FEC_BUF_DESC_DIV-1)];
    fnet_uint8 rx_buf_desc[(FNET_FEC_RX_BUF_NUM * sizeof(fnet_fec_buf_desc_t)) + (FNET_FEC_BUF_DESC_DIV-1)];
    fnet_uint8 tx_buf[FNET_FEC_TX_BUF_NUM][FNET_FEC_BUF_SIZE + (FNET_FEC_TX_BUF_DIV-1)];
    fnet_uint8 rx_buf[FNET_FEC_RX_BUF_NUM][FNET_FEC_BUF_SIZE + (FNET_FEC_RX_BUF_DIV-1)]; 
}
fnet_fec_buf_memory_t;
FNET_COMP_PACKED_END

#define FNET_FEC_ALIGN_DIV(div, x)     (((fnet_uint32)(x) + ((fnet_uint32)(div)-1))  & (~((fnet_uint32)(div)-1)))

/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_fec_init(fnet_netif_t *netif);
void fnet_fec_release(fnet_netif_t *netif);
void fnet_fec_input(fnet_netif_t *netif);
void fnet_fec_output(fnet_netif_t *netif, fnet_ip_addr_t dest_ip_addr, fnet_netbuf_t* nb, int do_not_route);
void fnet_fec_rx_buf_next( fnet_fec_if_t *ethif);

/* FEC rx frame interrup handler. */
static void fnet_fec_isr_rx_handler_top(void);
static void fnet_fec_isr_rx_handler_bottom(void);

void fnet_fec_phy_discover_addr (fnet_fec_if_t *ethif);
void fnet_fec_mii_print_regs(fnet_fec_if_t *ethif);



/************************************************************************
*     Global Data Structures
*************************************************************************/
/*Default MAC address.*/
const static fnet_mac_addr_t fnet_eth_default_addr = {0x00,0x11,0x22,0x33,0x44,0x55};

/* Ethernet specific control data structure.*/
static fnet_fec_if_t fnet_eth0_net_if;



/* FEC buffer structure. */
static fnet_fec_buf_memory_t fnet_fec_buf_memory;

static unsigned long fnet_fec_old_isr = 0;

#define FNET_FEC_TIMER_PERIOD (4000) /*ms*/
/************************************************************************
* NAME: fnet_fec_timer
*
* DESCRIPTION: 
*************************************************************************/
static void fnet_fec_timer()
{
    static int connection_flag = 0;

    if(fnet_netif_connected(&fnet_eth0_if) != connection_flag) /* Is any change in connection. */
    {
        if(connection_flag == 0)  /* Connected. */
        {
            fnet_fec_set_addr_notify(&fnet_eth0_if);
        }
            
        connection_flag ^= 1;
    }
}

/************************************************************************
* NAME: fnet_fec_init
*
* DESCRIPTION: Ethernet module initialization.
*************************************************************************/
int fnet_fec_init(fnet_netif_t *netif)
{
    fnet_fec_if_t *ethif;
    int i;
    fnet_mac_addr_t macaddr;
    
    netif->if_ptr = &fnet_eth0_net_if; /* Set pointer to FEC-specific data structure. */
    
    if (FNET_OK == fnet_arp_init(netif))
    {
        ethif = netif->if_ptr;
        
        ethif->reg = (volatile fnet_fec_reg_t *)FNET_FEC_BASE_ADDR; /* Set base address of FEC registers.*/
        
        fnet_eth_io_init();
        
        /* Reset FEC.*/
        ethif->reg->ECR = FNET_FEC_ECR_RESET;            /* Reset the Ethernet controller.*/
        while (ethif->reg->ECR & FNET_FEC_ECR_RESET){};  /* Wait for the reset sequence to complete.*/
        
        /* Disable FEC interrupts.*/
        ethif->reg->EIMR = 0;

        /* Clear any pending interrupts.*/
        ethif->reg->EIR = 0xFFFFFFFF;
        
        /* ======== Ethernet buffers initialisation ===============*/
        ethif->tx_buf_desc = (fnet_fec_buf_desc_t *)FNET_FEC_ALIGN_DIV(FNET_FEC_BUF_DESC_DIV, fnet_fec_buf_memory.tx_buf_desc);
        ethif->rx_buf_desc = (fnet_fec_buf_desc_t *)FNET_FEC_ALIGN_DIV(FNET_FEC_BUF_DESC_DIV, fnet_fec_buf_memory.rx_buf_desc);
        
        
        ethif->tx_buf_desc_cur=ethif->tx_buf_desc;
        ethif->rx_buf_desc_cur=ethif->rx_buf_desc;

        /* Initialize Tx descriptor rings.*/
        for (i = 0; i < FNET_FEC_TX_BUF_NUM; i++)
        {
            ethif->tx_buf_desc[i].status = FNET_HTONS(FNET_FEC_TX_BD_L | FNET_FEC_TX_BD_TC);
            ethif->tx_buf_desc[i].length = FNET_HTONS(0);
            
            ethif->tx_buf_desc[i].buf_ptr = (unsigned char *)fnet_htonl(FNET_FEC_ALIGN_DIV(FNET_FEC_TX_BUF_DIV, fnet_fec_buf_memory.tx_buf[i]));
        }

        ethif->tx_buf_desc_num=FNET_FEC_TX_BUF_NUM;

        /* Initialize Rx descriptor rings.*/
        for (i = 0; i < FNET_FEC_RX_BUF_NUM; i++)
        {
            ethif->rx_buf_desc[i].status = FNET_HTONS(FNET_FEC_RX_BD_E);
            ethif->rx_buf_desc[i].length = FNET_HTONS(0);
            
            ethif->rx_buf_desc[i].buf_ptr = (unsigned char *)fnet_htonl(FNET_FEC_ALIGN_DIV(FNET_FEC_RX_BUF_DIV, fnet_fec_buf_memory.rx_buf[i]));
        }

        ethif->rx_buf_desc_num = FNET_FEC_RX_BUF_NUM;

        /* Set the Wrap bit on the last one in the ring.*/
        ethif->tx_buf_desc[FNET_FEC_TX_BUF_NUM - 1].status |= FNET_HTONS(FNET_FEC_TX_BD_W);
        ethif->rx_buf_desc[FNET_FEC_RX_BUF_NUM - 1].status |= FNET_HTONS(FNET_FEC_RX_BD_W);

 
        /*======== END of Ethernet buffers initialisation ========*/

        fnet_fec_old_isr = fnet_isr_set_vector(FNET_FEC_RX_FRAME_VECTOR, fnet_fec_isr_rx_handler_top, fnet_fec_isr_rx_handler_bottom, FNET_FEC_INT_LEVEL);

        /* Reset all multicast (hash table registers).*/
        ethif->reg->GAUR = 0;
        ethif->reg->GALR = 0;

        ethif->reg->IAUR = 0;
        ethif->reg->IALR = 0;

        /* Set Receive Buffer Size.*/
        ethif->reg->EMRBR = FNET_FEC_BUF_SIZE;

        /* Pointer to Receive descriptor ring. */
        /* ERDSR points to the start of the circular receive buffer descriptor 
        queue in external memory. This pointer must be 32-bit aligned; however, 
        it is recommended it be made 128-bit aligned (evenly divisible by 16).*/
        ethif->reg->ERDSR = (unsigned long)ethif->rx_buf_desc;

        /* Pointer to Transmit descriptor ring. */
        /* ETDSR provides a pointer to the start of the circular transmit buffer 
        descriptor queue in external memory.
        This pointer must be 32-bit aligned; however, it is recommended it be 
        made 128-bit aligned (evenly divisible by 16).*/
        ethif->reg->ETDSR = (unsigned long)ethif->tx_buf_desc;

#if FNET_MCF /*MCF*/  //TBD Set it by config. param.         
        /* Set the tranceiver interface to MII mode.*/
        ethif->reg->RCR = FNET_FEC_RCR_MII_MODE | FNET_FEC_RCR_MAX_FL(FNET_FEC_BUF_SIZE);
#endif        
#if FNET_MK /*Kinetis*/ //TBD Set it by config. param.     
        /* Set the tranceiver interface to RMII mode.*/
        ethif->reg->RCR = FNET_FEC_RCR_MII_MODE | FNET_FEC_RCR_MAX_FL(FNET_FEC_BUF_SIZE);
#endif        
        
        /* Full duplex enable.*/
        ethif->reg->TCR = FNET_FEC_TCR_FDEN;


        /*The MII_SPEED field must be programmed with a value to provide
        * an EMDC frequency of less than or equal to 2.5 MHz to be compliant
        * with the IEEE 802.3 MII specification.*/
        ethif->reg->MSCR = ((FNET_FEC_CLOCK_MHZ/5) + ((FNET_FEC_CLOCK_MHZ%5)?1:0))<<1;

        /* Enable interrupts (Receive frame interrupt).*/
        ethif->reg->EIMR = FNET_FEC_EIMR_RXF ;
        
        /* Enable FEC */
        ethif->reg->ECR = FNET_FEC_ECR_ETHER_EN;

        fnet_fec_phy_discover_addr(ethif);
        
        fnet_eth_phy_init(ethif);


#if 0 /* Do not wait for negotiation. */
        {
            unsigned short status;
            if (! fnet_eth_mii_read(ethif, 0x1, &status)) 
                status = 0;
            if (status & 0x0008)
            { /* Has auto-negotiate ability. */
                    do 
                    {
                       if ((status & 0x0020))
                       {
                          /* Negotiated.*/    
                          break;
                       }
                    } 
                    while ( fnet_eth_mii_read(ethif, 0x1, &status) );
             } 

        }
#endif

        /* Indicate that there have been empty receive buffers produced.*/
        ethif->reg->RDAR=FNET_FEC_RDAR_R_DES_ACTIVE;

        /* Check that the MAC address is valid. If good use it, otherwise use the default.*/
        if((fnet_str_to_mac( FNET_CFG_ETH_MAC_ADDR, macaddr) != FNET_OK) ||
            (fnet_fec_set_mac( netif, macaddr, sizeof(fnet_mac_addr_t)) != FNET_OK))
        {
             fnet_fec_set_mac( netif, (unsigned char *) fnet_eth_default_addr, sizeof(fnet_mac_addr_t));
        }

/* Clear Ethernet statistics. */
#if FNET_CFG_CPU_ETH_MIB 
        ethif->reg->MIBC = FNET_FEC_MIBC_MIB_DISABLE; /* Disable MIB */
        ethif->reg->RMON_T_PACKETS = 0;
        ethif->reg->RMON_R_PACKETS = 0;
        ethif->reg->MIBC &= ~FNET_FEC_MIBC_MIB_DISABLE; /* Enable MIB */
#else
        fnet_memset(&ethif->statistics, 0, sizeof(struct fnet_netif_statistics));
#endif         
        
        
        ethif->eth_timer = fnet_timer_new((FNET_FEC_TIMER_PERIOD / FNET_TIMER_PERIOD_MS), 
                        fnet_fec_timer);
 
        return (FNET_OK);
    }
    else
        return(FNET_ERR);
}

/************************************************************************
* NAME: fnet_fec_release
*
* DESCRIPTION: Ethernet module release.
*************************************************************************/
void fnet_fec_release(fnet_netif_t *netif)
{
    fnet_fec_if_t * ethif = (fnet_fec_if_t *)netif->if_ptr;

    /* Note: Sometimes I could not communicate with the Ethernet PHY, 
    * all reads come back as 0xFFFF. It appears that the problem is 
    * that we need to disable the MDC clock by writing 0 to MSCR 
    * before disabling the FEC: */
    ethif->reg->MSCR = 0;           /* Disable the MDC clock. */
                                    
    ethif->reg->ECR = 0;            /* Disable the Ethernet FEC. */
    
    ethif->reg->EIMR = 0xFFFFFFFF ; /* Disable FEC interrupts. */

    ethif->reg->EIR = 0xFFFFFFFF;   /* Clear any pending FEC interrupt flags. */
   
    
    fnet_isr_reset_vector(FNET_FEC_RX_FRAME_VECTOR, (void(*)())fnet_fec_old_isr);
    fnet_arp_release(netif);
}

/************************************************************************
* NAME: fnet_fec_input
*
* DESCRIPTION: Ethernet input function.
*************************************************************************/
void fnet_fec_input(fnet_netif_t *netif)
{
    fnet_fec_if_t * ethif = (fnet_fec_if_t *)netif->if_ptr;
    fnet_eth_header_t * ethheader;
    fnet_netbuf_t * nb=0;

    fnet_mac_addr_t local_mac_addr;

    fnet_cpu_cache_invalidate();
	
	/* While buffer ! (empty or rx in progress)*/
    while(!(ethif->rx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_E)) )
    {

#if !FNET_CFG_CPU_ETH_MIB       
        ethif->statistics.rx_packet++;
#endif 
		/* If !(buffer last in frame) */
        if (!(ethif->rx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_L)))
        {
            /* Skip the splitted frame. */
            do /* Keep going until we find the last one. */
            {
                fnet_fec_rx_buf_next(ethif);
            }
            while (!(ethif->rx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_L)));
        }
        else
        {
            /* Eerror handling */
            if (ethif->rx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_LG /* Frame too long.*/
                                                 |FNET_FEC_RX_BD_SH /* Frame too short.*/
                                                 |FNET_FEC_RX_BD_NO /* Frame alignment.*/
                                                 |FNET_FEC_RX_BD_CR /* CRC Error.*/
                                                 |FNET_FEC_RX_BD_OV /* FIFO overrun.*/
                                                 |FNET_FEC_RX_BD_TR /* Frame is truncated.*/
                                                 ))
            {
                 goto NEXT_FRAME;
            }

            /* Point to the ethernet header.*/
            ethheader = (fnet_eth_header_t *)fnet_ntohl((unsigned long)ethif->rx_buf_desc_cur->buf_ptr);
                  
            fnet_fec_get_mac(netif, local_mac_addr, sizeof(fnet_mac_addr_t));
            if(!fnet_memcmp(ethheader->source_addr, local_mac_addr, sizeof(local_mac_addr)))
            {
               goto NEXT_FRAME;
            }
            
            fnet_eth_trace("\nRX", ethheader); /* Print ETH header.*/
                
            nb = fnet_netbuf_from_buf( (void *)((unsigned long)ethheader + sizeof(fnet_eth_header_t)), 
                                        (int)(fnet_ntohs(ethif->rx_buf_desc_cur->length) - sizeof(fnet_eth_header_t)), 
                                        FNET_NETBUF_TYPE_HEADER, FNET_NETBUF_MALLOC_NOWAITDRAIN );
                
            if(nb)
            {
                if(ethif->rx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_BC))    /* Broadcast */
                {
                    nb->flags|=FNET_NETBUF_FLAG_BROADCAST;
                }
               
                if ((ethif->rx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_MC))) /* Multicast */
                {
                    nb->flags|=FNET_NETBUF_FLAG_MULTICAST;
                }
                  
                switch(ethheader->type)
                {
                    case FNET_HTONS(FNET_ETH_TYPE_IP):
                        fnet_ip_input(netif, nb);
                        break;
                    case FNET_HTONS(FNET_ETH_TYPE_ARP):
                        fnet_arp_input(netif, nb);
                        break;
                    default:
                        fnet_netbuf_free_chain(nb); 
                        break;
                }
            }
        }
NEXT_FRAME: 
        fnet_fec_rx_buf_next(ethif);
   }; /* while */

}

/************************************************************************
* NAME: fnet_fec_rx_buf_next
*
* DESCRIPTION: Goes not the next Rx buffer.
*************************************************************************/
static void fnet_fec_rx_buf_next( fnet_fec_if_t * ethif)
{
   /* Mark the buffer as empty.*/
   ethif->rx_buf_desc_cur->status &= FNET_HTONS(FNET_FEC_RX_BD_W);
   ethif->rx_buf_desc_cur->status |= FNET_HTONS(FNET_FEC_RX_BD_E);
    
   /* Update pointer to next entry.*/
   if (ethif->rx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_W))
      ethif->rx_buf_desc_cur = ethif->rx_buf_desc;
   else
      ethif->rx_buf_desc_cur++;
    
   /* Indicate that there have been empty receive buffers produced.*/
   ethif->reg->RDAR=FNET_FEC_RDAR_R_DES_ACTIVE;
}

/************************************************************************
* NAME: fnet_fec_output
*
* DESCRIPTION: Ethernet output function.
*************************************************************************/
void fnet_fec_output(fnet_netif_t *netif, fnet_ip_addr_t dest_ip_addr, fnet_netbuf_t* nb, int do_not_route)
{
    fnet_mac_addr_t destination_addr; /* 48-bit destination address */
    fnet_mac_addr_t * dest_ptr;

#if FNET_CFG_LOOP
    fnet_netbuf_t* nb_loop;   
#endif
   
    fnet_ip_addr_t query_ip_addr = dest_ip_addr;
   
    fnet_memcpy(destination_addr, fnet_eth_null_addr, sizeof(fnet_mac_addr_t));
   
    /* Anything sent to one of the host's own IP address is sent to the loopback interface.*/
#if FNET_CFG_LOOP
    if(dest_ip_addr == netif->addr.address)
    {
        fnet_loop_output(netif, dest_ip_addr, nb, do_not_route);
        return;
    }
#endif
  
    /* Construct Ethernet header. Start with looking up deciding which
    * MAC address to use as a destination address. Broadcasts and
    * multicasts are special, all other addresses are looked up in the
    * ARP table. */
    if(fnet_ip_addr_is_broadcast (dest_ip_addr, &(netif->addr)))
    {
        /* Datagrams sent to a broadcast address are copied to the loopback interface.*/
#if FNET_CFG_LOOP
        if((nb_loop=fnet_netbuf_copy(nb, 0, FNET_NETBUF_COPYALL, FNET_NETBUF_MALLOC_NOWAITDRAIN))!=0) 
        {
            fnet_loop_output(netif, dest_ip_addr, nb_loop, do_not_route);
        }
#endif 
        
        fnet_memcpy (destination_addr, fnet_eth_broadcast, sizeof(fnet_mac_addr_t));
    }
    else if(FNET_IP_MULTICAST(dest_ip_addr))
    {
        /* Datagrams sent to a multicast address are copied to the loopback interface.*/
#if FNET_CFG_LOOP
        if((nb_loop=fnet_netbuf_copy(nb, 0, FNET_NETBUF_COPYALL, FNET_NETBUF_MALLOC_NOWAITDRAIN))!=0) 
           fnet_loop_output(netif, dest_ip_addr, nb_loop, do_not_route);
#endif
        
        /* Hash IP multicast address to MAC address. */
        destination_addr[0] = 0x01;
        destination_addr[1] = 0x0;
        destination_addr[2] = 0x5e;
        destination_addr[3] = (unsigned char)(FNET_IP_ADDR2(dest_ip_addr)& 0x7f);
        destination_addr[4] = (unsigned char)(FNET_IP_ADDR3(dest_ip_addr));
        destination_addr[5] = (unsigned char)(FNET_IP_ADDR4(dest_ip_addr));
    }
    else
    {
        if(!((do_not_route) || ((dest_ip_addr & netif->addr.subnetmask) == (netif->addr.address & netif->addr.subnetmask))))
        {
            /* Use the default router as the address to send the Ethernet frame to.*/
            query_ip_addr=netif->addr.gateway;
        }
           
        if((dest_ptr = fnet_arp_lookup(netif, query_ip_addr))!=0)
        {
            fnet_memcpy (destination_addr, *dest_ptr, sizeof(fnet_mac_addr_t));
        }
    }
   
    if(fnet_memcmp(destination_addr,fnet_eth_null_addr,sizeof(fnet_mac_addr_t)))
    {
        fnet_eth_output_low(netif, FNET_ETH_TYPE_IP, destination_addr, nb);
    }
    else
    {
        fnet_arp_resolve(netif, query_ip_addr, nb);
    }

}

/************************************************************************
* NAME: fnet_eth_output_low
*
* DESCRIPTION: Ethernet low-level output function.
*************************************************************************/
void fnet_eth_output_low(fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb)
{
    fnet_fec_if_t *ethif = (fnet_fec_if_t *)netif->if_ptr;
    fnet_eth_header_t * ethheader;
 
    if((nb!=0)&&(nb->flags & FNET_NETBUF_FLAG_FIRST)&&(nb->total_length<=netif->mtu)) 
    {
        while(ethif->tx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_TX_BD_R))
        {};
      
        ethheader = (fnet_eth_header_t *)fnet_ntohl((unsigned long)ethif->tx_buf_desc_cur->buf_ptr);
      
        fnet_netbuf_to_buf(nb, 0, FNET_NETBUF_COPYALL, (void *)((unsigned long)ethheader + FNET_ETH_HDR_SIZE));    
      

        fnet_memcpy (ethheader->destination_addr, dest_addr, sizeof(fnet_mac_addr_t));
      
        fnet_fec_get_mac(netif, ethheader->source_addr, sizeof(fnet_mac_addr_t));
      
        ethheader->type=fnet_htons(type);
      
         
        ethif->tx_buf_desc_cur->length = fnet_htons((unsigned short)(FNET_ETH_HDR_SIZE + nb->total_length));
        ethif->tx_buf_desc_cur->status |= FNET_HTONS(FNET_FEC_TX_BD_R); /* Set Frame ready for transmit.*/
      
        /* Update pointer to next entry.*/
        if (ethif->tx_buf_desc_cur->status & FNET_HTONS(FNET_FEC_RX_BD_W))
            ethif->tx_buf_desc_cur = ethif->tx_buf_desc;
        else
            ethif->tx_buf_desc_cur++;
       
        while(ethif->reg->TDAR) /* Workaround.*/
        {};

        ethif->reg->TDAR=1; /* Indicate that there has been a transmit buffer produced.*/

#if !FNET_CFG_CPU_ETH_MIB       
        ethif->statistics.tx_packet++;
#endif      
    }
   
    fnet_netbuf_free_chain(nb);   
}

/************************************************************************
* NAME: fnet_fec_set_addr_notify
*
* DESCRIPTION: This function issues a gratuitous ARP request.
*************************************************************************/
void fnet_fec_set_addr_notify(fnet_netif_t *netif)
{
   fnet_arp_request(netif,netif->addr.address); /* Gratuitous ARP request.*/
}

/************************************************************************
* NAME: fnet_fec_drain
*
* DESCRIPTION: This function tries to free not critical parts 
*              of memory in the Ethernet driver.
*************************************************************************/
void fnet_fec_drain(fnet_netif_t *netif)
{
   int i;
   fnet_arp_if_t * arpif = fnet_eth_arp_if(netif);
     
   fnet_isr_lock();
   
   /* ARP table drain.*/
   for(i=0;i<FNET_ARP_TABLE_SIZE;i++)
   {
      if(arpif->arp_table[i].hold)
      {
         fnet_netbuf_free_chain(arpif->arp_table[i].hold);
         arpif->arp_table[i].hold=0;
         arpif->arp_table[i].hold_time=0;
      }
   }
  
   fnet_isr_unlock();
}


/************************************************************************
* NAME: fnet_fec_set_mac
*
* DESCRIPTION: This function sets MAC address. 
*************************************************************************/
int fnet_fec_set_mac(fnet_netif_t *netif, unsigned char * hw_addr, unsigned char hw_addr_size)
{
   fnet_fec_if_t *ethif = (fnet_fec_if_t *)netif->if_ptr;
   int result;
   
   /* Set the source address for the controller. */
    if(netif && (netif->type==FNET_NETIF_TYPE_ETHERNET) && ethif && 
        (hw_addr_size == sizeof(fnet_mac_addr_t)) && hw_addr
          && fnet_memcmp(hw_addr,fnet_eth_null_addr,sizeof(fnet_mac_addr_t))&&
            fnet_memcmp(hw_addr,fnet_eth_broadcast,sizeof(fnet_mac_addr_t))&&
            ((hw_addr[0]&0x01)==0x00)) /* Most significant nibble should always be even.*/
    { 
        ethif->reg->PALR = (unsigned long)((hw_addr[0] <<24)|(hw_addr[1] <<16)|(hw_addr[2] <<8)|(hw_addr[3] <<0));
        ethif->reg->PAUR = (unsigned long)(hw_addr[4] <<24)|(hw_addr[5] <<16);
        fnet_arp_request(netif,netif->addr.address); /* Gratuitous ARP request.*/
        result = FNET_OK;
    }
    else
        result = FNET_ERR;
      

   return result;
}

/************************************************************************
* NAME: fnet_fec_get_mac
*
* DESCRIPTION: This function reads MAC address. 
*************************************************************************/
int fnet_fec_get_mac(fnet_netif_t *netif, unsigned char * hw_addr, unsigned char hw_addr_size)
{
    fnet_fec_if_t *ethif = (fnet_fec_if_t *)netif->if_ptr;
    unsigned long tmp;
    int result;
   
    if(netif && (netif->type==FNET_NETIF_TYPE_ETHERNET) && ethif && 
        (hw_addr_size == sizeof(fnet_mac_addr_t)) && hw_addr)
    { 
        tmp=ethif->reg->PALR;
        hw_addr[0]= (unsigned char)(tmp>>24);
        hw_addr[1]= (unsigned char)(tmp>>16);
        hw_addr[2]= (unsigned char)(tmp>>8);
        hw_addr[3]= (unsigned char)tmp;
   
        tmp=ethif->reg->PAUR;
        hw_addr[4]= (unsigned char)(tmp>>24);
        hw_addr[5]= (unsigned char)(tmp>>16);
        result = FNET_OK;
    }
    else
    {
        result = FNET_ERR;
    }

    return result; 
}

/************************************************************************
* NAME: fnet_fec_get_statistics
*
* DESCRIPTION: Returns Ethernet statistics information 
*************************************************************************/
int fnet_fec_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics)
{
    fnet_fec_if_t *ethif = (fnet_fec_if_t *)netif->if_ptr;
    int result;
    
    if(netif && (netif->type==FNET_NETIF_TYPE_ETHERNET))
    {
#if FNET_CFG_CPU_ETH_MIB 
        statistics->tx_packet = ethif->reg->RMON_T_PACKETS; 
        statistics->rx_packet = ethif->reg->RMON_R_PACKETS;
#else 
        *statistics = ethif->statistics;
#endif        
        result = FNET_OK;
    }
    else
    {
        result = FNET_ERR;
    }

    return result;    
}

/************************************************************************
* NAME: fnet_fec_isr_rx_handler_top
*
* DESCRIPTION: Top Ethernet receive frame interrupt handler. 
*              Clear event flag
*************************************************************************/
static void fnet_fec_isr_rx_handler_top (void) 
{
    fnet_fec_if_t * ethif=(fnet_fec_if_t *)(fnet_eth0_if.if_ptr); 
    
    /* Clear FEC RX Event from the Event Register (by writing 1).*/
    ethif->reg->EIR=FNET_FEC_EIR_RXF;
}
/************************************************************************
* NAME: fnet_fec_isr_rx_handler_bottom
*
* DESCRIPTION: This function implements the Ethernet receive 
*              frame interrupt handler. 
*************************************************************************/
static void fnet_fec_isr_rx_handler_bottom (void) 
{
    fnet_isr_lock();
    
    fnet_eth0_if.input(FNET_ETH_IF);
    
    fnet_isr_unlock();
}


/************************************************************************
* MII Staff 
*************************************************************************/

/************************************************************************
* NAME: fnet_fec_phy_discover_addr
*
* DESCRIPTION: Looking for a valid PHY address.
*************************************************************************/
void fnet_fec_phy_discover_addr (fnet_fec_if_t *ethif)
{ 
    #if FNET_CFG_CPU_MCF52235   //TBD Control it by config. param.     
        
        /* Set phy address */
        ethif->phy_addr = FNET_FEC_PHY_ADDRESS;
    
    #else        
        unsigned short   i;
        fnet_uint16     id;
          
        
        for (i = 0; i < 32; i++) 
        {
            ethif->phy_addr = i;
            fnet_fec_mii_read(ethif, FNET_FEC_MII_REG_IDR1, &id);
          
            if (!(id == 0 || id == 0xffff))
            {
                ethif->phy_addr = (unsigned char)i;
                break;
            }
        } 

        if (id == 0x0141)
        {
            fnet_uint16 value;

            ethif->phy_addr = 8;
            fnet_fec_mii_write(ethif, 0x04, 0x03);
            ethif->phy_addr = 9;
            fnet_fec_mii_write(ethif, 0x04, 0x03);
            ethif->phy_addr = 12;
            fnet_fec_mii_write(ethif, 0x04, 0x03);
        }
        
        ethif->phy_addr = FNET_FEC_PHY_ADDRESS;        
    #endif
} 

/************************************************************************
* NAME: fnet_fec_mii_print_regs
*
* DESCRIPTION: Prints all MII register. Used only for debug needs.
*************************************************************************/
#if 0 /* For debug needs only.*/
static void fnet_fec_mii_print_regs(fnet_eth_if_t *ethif) 
{
    fnet_uint16 reg_value;

    fnet_println(" === MII registers ===");
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_CR, &reg_value);
    fnet_println("\tCR = 0x%04X", reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_SR, &reg_value);
    fnet_println("\tSR = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_IDR1, &reg_value);
    fnet_println("\tIDR1 = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_IDR2, &reg_value);
    fnet_println("\tIDR2 = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_ANAR, &reg_value);
    fnet_println("\tANAR = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_ANLPAR, &reg_value);
    fnet_println("\tANLPAR = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_ANER, &reg_value);
    fnet_println("\tANER = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_ANNPTR, &reg_value);
    fnet_println("\tANNPTR = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_ICR, &reg_value);
    fnet_println("\tICR = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_PSR, &reg_value);
    fnet_println("\tPSR = 0x%04X",reg_value );
    fnet_eth_mii_read(ethif, FNET_FEC_MII_REG_PCR, &reg_value);
    fnet_println("\tPCR = 0x%04X",reg_value );
}
#endif


/************************************************************************
* NAME: fnet_fec_mii_read
*
* DESCRIPTION: Read a value from a PHY's MII register.
* reg_addr < address of the register in the PHY
* data < Pointer to storage for the Data to be read from the PHY register (passed by reference)
* Return 0 on failure, 1 on success
*************************************************************************/ 
int fnet_fec_mii_read(fnet_fec_if_t *ethif, int reg_addr, fnet_uint16 *data) 
{
    int timeout;
    fnet_uint32 eimr;
    int result;

    /* Clear the MII interrupt bit */
     ethif->reg->EIR = FNET_FEC_EIR_MII;

    /* Mask the MII interrupt */
    eimr = ethif->reg->EIMR;
    ethif->reg->EIMR = (unsigned long)(~FNET_FEC_EIMR_MII); 

    /* Kick-off the MII read */
    ethif->reg->MMFR = (fnet_vuint32)(0 
            | FNET_FEC_MMFR_ST_01
            | FNET_FEC_MMFR_OP_READ
            | FNET_FEC_MMFR_PA(ethif->phy_addr)
            | FNET_FEC_MMFR_RA(reg_addr)
            | FNET_FEC_MMFR_TA_10);

    /* Poll for the MII interrupt */
    for (timeout = 0; timeout < FNET_FEC_MII_TIMEOUT; timeout++)
    {
        if(ethif->reg->EIR & FNET_FEC_EIR_MII)
            break;
    }

    /* Clear the MII interrupt bit */
    ethif->reg->EIR = FNET_FEC_EIR_MII;

    /* Restore the EIMR */
    ethif->reg->EIMR = eimr;
    
    if(timeout == FNET_FEC_MII_TIMEOUT)
    {
        result = 0;
    }
    else
    {
        *data = (fnet_uint16)(ethif->reg->MMFR & 0xFFFF);
        result = 1;
    }

    return result;
}

/************************************************************************
* NAME: fnet_fec_mii_write
*
* DESCRIPTION: Write a value to a PHY's MII register.
* reg_addr < address of the register in the PHY
* data < Data to be writen to the PHY register (passed by reference)
* Return 0 on failure (timeout), 1 on success
*************************************************************************/ 
int fnet_fec_mii_write(fnet_fec_if_t *ethif, int reg_addr, fnet_uint16 data) 
{

    int timeout;
    fnet_uint32 eimr;
    int result;

    /* Clear the MII interrupt bit */
    ethif->reg->EIR = FNET_FEC_EIR_MII;

    /* Mask the MII interrupt */
    eimr = ethif->reg->EIMR;
    ethif->reg->EIMR = (unsigned long)(~FNET_FEC_EIMR_MII); 

    /* Kick-off the MII write */
    ethif->reg->MMFR = (fnet_vuint32)(0 
            | FNET_FEC_MMFR_ST_01
            | FNET_FEC_MMFR_OP_WRITE
            | FNET_FEC_MMFR_PA(ethif->phy_addr)
            | FNET_FEC_MMFR_RA(reg_addr)
            | FNET_FEC_MMFR_TA_10
            | (data & 0xffff));

    /* Poll for the MII interrupt */
    for (timeout = 0;  timeout < FNET_FEC_MII_TIMEOUT; timeout++)
    {
        if(ethif->reg->EIR & FNET_FEC_EIR_MII)
            break;
    }

    /* Clear the MII interrupt bit */
    ethif->reg->EIR = FNET_FEC_EIR_MII;

    /* Restore the EIMR */
    ethif->reg->EIMR = eimr;
    
    if(timeout == FNET_FEC_MII_TIMEOUT)
    {
        result = 0;
    }
    else
    {
        result = 1;
    }

    return result;
}


/************************************************************************
* NAME: fnet_fec_is_connected
*
* DESCRIPTION: Link status.
*************************************************************************/
int fnet_fec_is_connected(fnet_netif_t *netif)
{
    fnet_uint16 data;
    fnet_fec_if_t *ethif;
    int res = 0;
    
    ethif = (fnet_fec_if_t *)netif->if_ptr;
    
    /* Some PHY (e.g.DP8340) returns "unconnected" and than "connected" state
     *  just to show that was transition event from one state to another.
     *  As we need only curent state,  read 2 times and returtn 
     *  the current/latest state. 
     */

    fnet_fec_mii_read(ethif, FNET_FEC_MII_REG_SR, &data); 
    
    if (fnet_fec_mii_read(ethif, FNET_FEC_MII_REG_SR, &data))
    {
        res = (data & FNET_FEC_MII_REG_SR_LINK_STATUS) ? 1 : 0;
    }
    
    return res;
}

/************************************************************************
* NAME: fnet_eth_arp_if
*
* DESCRIPTION:
*************************************************************************/
fnet_arp_if_t * fnet_eth_arp_if(fnet_netif_t *netif)
{
    fnet_fec_if_t *ethif = (fnet_fec_if_t *)netif->if_ptr;
    
    return &ethif->arp_if;
}



#endif /* FNET_MK && FNET_CFG_ETH */



