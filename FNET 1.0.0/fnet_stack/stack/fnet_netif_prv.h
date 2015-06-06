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
* @file fnet_netif_prv.h
*
* @author Andrey Butok
*
* @date Feb-4-2011
*
* @version 0.1.8.0
*
* @brief Private. FNET Network interface API.
*
***************************************************************************/

#ifndef _FNET_NETIF_PRV_H_

#define _FNET_NETIF_PRV_H_

#include "fnet_config.h"
#include "fnet_netbuf.h"
#include "fnet_netif.h"

/**************************************************************************/ /*!
 * @internal
 * @brief    Interface address structure.
 ******************************************************************************/

typedef struct
{
    fnet_ip_addr_t address;         /**< The IP address.*/
    fnet_ip_addr_t net;             /**< Network address.*/
    fnet_ip_addr_t netmask;         /**< Network mask.*/
    fnet_ip_addr_t subnet;          /**< Network and subnet address.*/
    fnet_ip_addr_t subnetmask;      /**< Network and subnet mask.*/
    fnet_ip_addr_t netbroadcast;    /**< Network broadcast address.*/
    fnet_ip_addr_t subnetbroadcast; /**< Subnet broadcast address.*/
    fnet_ip_addr_t gateway;         /**< Gateway.*/
    unsigned char is_automatic;     /**< 0 if it's set statically/manually.*/
                                    /**< 1 if it's obtained automatically (by DHCP).*/
} fnet_netif_addr_t;


/**************************************************************************/ /*!
 * @internal
 * @brief    Network interface structure.
 ******************************************************************************/
typedef struct fnet_netif
{
    struct fnet_netif *next;                    /* Pointer to the next net_if structure. */
    struct fnet_netif *prev;                    /* Pointer to the previous net_if structure. */
    fnet_netif_type_t type;                     /* Data-link type. */
    char name[FNET_NETIF_NAMELEN];              /* Network interface name (e.g. "eth0", "loop"). */
    fnet_netif_addr_t addr;                     /* The interface address structure. */
    unsigned long mtu;                          /* Maximum transmission unit. */
    void *if_ptr;                               /* Points to specific control data structure of current interface. */

    int (*init)( struct fnet_netif * );         /* Initialization function.*/
    void (*release)( struct fnet_netif * );     /* Shutdown function.*/
    void (*input)( struct fnet_netif * );       /* Receive function.*/
    void (*output)(struct fnet_netif *netif, fnet_ip_addr_t dest_ip_addr, fnet_netbuf_t* nb, int do_not_route); /* Transmit function.*/

    void (*set_addr_notify)( struct fnet_netif * );     /* Address change notification function.*/
    void (*drain)( struct fnet_netif * );               /* Drain function.*/
    int (*get_hw_addr)( struct fnet_netif *netif, unsigned char *hw_addr, unsigned char hw_addr_size );
    int (*set_hw_addr)( struct fnet_netif *netif, unsigned char *hw_addr, unsigned char hw_addr_size );
    int (*is_connected)( struct fnet_netif *netif );
    int (*get_statistics)( struct fnet_netif *netif, struct fnet_netif_statistics *statistics );
} fnet_netif_t;

/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_netif_init( void );
void fnet_netif_release( void );

void fnet_netif_add( fnet_netif_t *netif );
void fnet_netif_del( fnet_netif_t *netif );
void fnet_netif_drain( void );
void fnet_netif_address_set_automatic( fnet_netif_desc_t netif );
void fnet_netif_dupip_handler_signal( fnet_netif_desc_t netif );

/************************************************************************
*     Global Data Structures
*************************************************************************/
extern fnet_netif_t *fnet_netif_list;   /* The list of network interfaces.*/

#endif
