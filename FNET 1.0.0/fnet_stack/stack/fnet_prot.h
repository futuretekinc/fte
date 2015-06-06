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
* @file fnet_prot.h
*
* @author Andrey Butok
*
* @date Feb-4-2011
*
* @version 0.1.7.0
*
* @brief Private. Transport protocol interface definitions.
*
***************************************************************************/

#ifndef _FNET_PROT_H_

#define _FNET_PROT_H_

#include "fnet_netbuf.h"
#include "fnet_netif.h"
#include "fnet_netif_prv.h"

#include "fnet_tcp.h"
#include "fnet_ip_prv.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"

/************************************************************************
*    Protocol notify commands.
*************************************************************************/

typedef enum
{
    FNET_PROT_NOTIFY_QUENCH,           /* Some one said to slow down.*/
    FNET_PROT_NOTIFY_MSGSIZE,          /* Message size forced drop.*/
    FNET_PROT_NOTIFY_UNREACH_HOST,     /* No route to host.*/
    FNET_PROT_NOTIFY_UNREACH_PROTOCOL, /* Dst says bad protocol.*/
    FNET_PROT_NOTIFY_UNREACH_PORT,     /* Bad port #.*/
    FNET_PROT_NOTIFY_UNREACH_SRCFAIL,  /* Source route failed.*/
    FNET_PROT_NOTIFY_UNREACH_NET,      /* No route to network.*/
    FNET_PROT_NOTIFY_TIMXCEED_INTRANS, /* Packet time-to-live expired in transit.*/
    FNET_PROT_NOTIFY_TIMXCEED_REASS,   /* Reassembly time-to-leave expired.*/
    FNET_PROT_NOTIFY_PARAMPROB         /* Header incorrect.*/
} fnet_prot_notify_t;

/************************************************************************
*    Protocol interface control structure.
*************************************************************************/
typedef struct fnet_prot_if
{
    struct fnet_prot_if *next;                                                                                                         // Poinnter to the next protocol interface structure.
    int domain;                                                                                                                        // Address domain family.
    int type;                                                                                                                          // Socket type used for.
    int protocol;                                                                                                                      // Protocol number.
    int con_req;                                                                                                                       // Flag that protocol is connection oriented.

    fnet_socket_t *head;                                                                                                               // Pointer to the head of the protocol's socket list.

    int (*prot_init)( void );                                                                                                          // Protocol initialization function.
    void (*prot_release)( void );                                                                                                      // Protocol release function.

    int (*prot_input)(fnet_netif_t *netif, fnet_ip_addr_t src_ip, fnet_ip_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t * options ); // Protocol input function.

    void(*prot_control_input)(fnet_prot_notify_t command, fnet_ip_header_t * ip_hdr);                                                  // Protocol input control function. 

    /* User requests.*/
    int (*prot_attach)(fnet_socket_t *sk);                                                                         // Protocol "attach" function. 
    int (*prot_detach)(fnet_socket_t *sk);                                                                         // Protocol "detach" function.
    int (*prot_connect)(fnet_socket_t *sk, fnet_ip_addr_t addr, unsigned short port);                              // Protocol "connect" function.

    fnet_socket_t *( *prot_accept)(fnet_socket_t * sk);                                                                         // Protocol "accept" function.
    int (*prot_rcv)(fnet_socket_t *sk, char *buf, int len, int flags, fnet_ip_addr_t *addr, unsigned short *port); // Protocol "receive" function.
    int (*prot_snd)(fnet_socket_t *sk, char *buf, int len, int flags, fnet_ip_addr_t addr, unsigned short port);   // Protocol "send" function.		        
    int (*prot_shutdown)(fnet_socket_t *sk, int how);                                                              // Protocol "shutdown" function.
    int (*prot_setsockopt)(fnet_socket_t *sk, int level, int optname, char *optval, int optlen);                   // Protocol "setsockopt" function.
    int (*prot_getsockopt)(fnet_socket_t *sk, int level, int optname, char *optval, int *optlen);                  // Protocol "getsockopt" function.  	 
    int (*prot_listen)(fnet_socket_t *sk, int backlog);                                                            // Protocol "listen" function.

    void (*prot_drain)( void );                                                                                   // protocol drain function. 
} fnet_prot_if_t;

/************************************************************************
*     Function Prototypes.
*************************************************************************/
int fnet_prot_init( void );
void fnet_prot_release( void );
int fnet_prot_add( fnet_prot_if_t *prot );
void fnet_prot_del( fnet_prot_if_t *prot );
fnet_prot_if_t *fnet_prot_find( int domain, int type, int protocol );
void fnet_prot_drain( void );

#endif
