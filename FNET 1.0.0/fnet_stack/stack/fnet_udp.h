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
* and the GNU Lesser General Public Licensalong with this program.
* If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_udp.h
*
* @author Andrey Butok
*
* @date Mar-1-2011
*
* @version 0.1.9.0
*
* @brief Private. UDP protocol definitions.
*
***************************************************************************/

#ifndef _FNET_UDP_H_

#define _FNET_UDP_H_

#if FNET_CFG_UDP

#include "fnet.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"

/************************************************************************
*     UDP definitions
*************************************************************************/
#define FNET_UDP_TTL          (64)                       /* Default TTL.*/
#define FNET_UDP_DF           (0)                        /* DF flag.*/
#define FNET_UDP_TX_BUF_MAX   (FNET_CFG_SOCKET_UDP_TX_BUF_SIZE) /* Default maximum size for send socket buffer.*/
#define FNET_UDP_RX_BUF_MAX   (FNET_CFG_SOCKET_UDP_RX_BUF_SIZE) /* Default maximum size for receive socket buffer.*/

/************************************************************************
*     Global Data Structures
*************************************************************************/

extern struct fnet_prot_if fnet_udp_prot_if;

/* Structure of UDP header.*/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned short source_port;      /* Source port number.*/
    unsigned short destination_port; /* Destination port number.*/
    unsigned short length;           /* Length.*/
    unsigned short checksum;         /* Checksum.*/
} fnet_udp_header_t;
FNET_COMP_PACKED_END

/************************************************************************
*     Function Prototypes
*************************************************************************/

void fnet_udp_release( void );
int fnet_udp_output( fnet_netif_t *netif,    fnet_ip_addr_t src_ip,    unsigned short src_port,
                     fnet_ip_addr_t dest_ip, unsigned short dest_port, fnet_socket_option_t *sockoption,
                     fnet_netbuf_t *nb );
int fnet_udp_input( fnet_netif_t *netif, fnet_ip_addr_t src_ip, fnet_ip_addr_t dest_ip, fnet_netbuf_t *nb,
                    fnet_netbuf_t *options );

#endif

#endif
