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
* @file fnet_eth_prv.h
*
* @author Andrey Butok
*
* @date Apr-20-2011
*
* @version 0.1.10.0
*
* @brief Private. Ethernet platform independent API functions.
*
***************************************************************************/

#ifndef _FNET_ETHERNET_PRV_H_

#define _FNET_ETHERNET_PRV_H_

#include "fnet_config.h"

#if FNET_CFG_ETH

#include "fnet_eth.h"
#include "fnet_arp.h"


/************************************************************************
*     Definitions
*************************************************************************/
// Ethernet Frame Types
#define FNET_ETH_TYPE_IP        (0x0800)
#define FNET_ETH_TYPE_ARP       (0x0806)

#define FNET_ETH_HDR_SIZE       (14)    /* Size of Ethernet header.*/
#define FNET_ETH_CRC_SIZE       (4)     /* Size of Ethernet CRC.*/


/**************************************************************************/ /*!
* @internal
* @brief    Ethernet Frame header.
******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    fnet_mac_addr_t destination_addr;   /**< 48-bit destination address.*/
    fnet_mac_addr_t source_addr;        /**< 48-bit source address.*/
    unsigned short type;                /**< 16-bit type field.*/
} fnet_eth_header_t;
FNET_COMP_PACKED_END

/************************************************************************
*     Global Data Structures
*************************************************************************/
extern const fnet_mac_addr_t fnet_eth_null_addr;
extern const fnet_mac_addr_t fnet_eth_broadcast;
extern fnet_netif_t fnet_eth0_if;

#define FNET_ETH_IF (&fnet_eth0_if)

/************************************************************************
*     Function Prototypes
*************************************************************************/
fnet_arp_if_t *fnet_eth_arp_if( fnet_netif_t *netif );
void fnet_eth_output_low( fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr,
                          fnet_netbuf_t *nb );

#if FNET_CFG_DEBUG_TRACE_ETH
void fnet_eth_trace(char *str, fnet_eth_header_t *eth_hdr);
#else
#define fnet_eth_trace(str, eth_hdr)
#endif                          

#endif

#endif
