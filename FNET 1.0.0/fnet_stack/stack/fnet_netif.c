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
* @file fnet_netif.c
*
* @author Andrey Butok
*
* @date May-16-2011
*
* @version 0.1.11.0
*
* @brief FNET Network interface implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_ip_prv.h"
#include "fnet_error.h"
#include "fnet_netif_prv.h"
#include "fnet_arp.h"
#include "fnet_eth_prv.h"
#include "fnet_loop.h"
#include "fnet_stdlib.h"
#include "fnet.h"
#include "fnet_isr.h"


/************************************************************************
*     Global Data Structures
*************************************************************************/

fnet_netif_t *fnet_netif_list;           /* The list of network interfaces. */

static fnet_netif_t *fnet_netif_default; /* Default net_if. */

/* Duplicated IP event handler.*/
static fnet_netif_dupip_handler_t fnet_netif_dupip_handler;


/************************************************************************
* NAME: fnet_netif_init
*
* DESCRIPTION: Initialization of all supported interfaces.
*************************************************************************/
int fnet_netif_init( void )
{
    fnet_netif_t *net_if_ptr;

    fnet_isr_lock();

    fnet_netif_list = fnet_netif_default = 0;

#if FNET_CFG_ETH

    fnet_netif_add(FNET_ETH_IF);

#endif
#if FNET_CFG_LOOP

    fnet_netif_add(FNET_LOOP_IF);

#endif

    fnet_netif_set_default(FNET_DEFAULT_IF);

    for (net_if_ptr = fnet_netif_list; net_if_ptr; net_if_ptr = net_if_ptr->next)
    {
        if(net_if_ptr->init)
            if(net_if_ptr->init(net_if_ptr) != FNET_OK)
            {
                fnet_netif_del(net_if_ptr);
                return (FNET_ERR);
            }
    }

#if FNET_CFG_ETH

    fnet_netif_set_address(FNET_ETH_IF, FNET_CFG_ETH_IP_ADDR);
    fnet_netif_set_netmask(FNET_ETH_IF, (unsigned long)FNET_CFG_ETH_IP_MASK);
    fnet_netif_set_gateway(FNET_ETH_IF, FNET_CFG_ETH_IP_GW);

#endif
#if FNET_CFG_LOOP

    fnet_netif_set_address(FNET_LOOP_IF, FNET_CFG_LOOP_IP_ADDR);

#endif

    fnet_isr_unlock();
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_netif_release
*
* DESCRIPTION: Releases all installed interfaces.
*************************************************************************/
void fnet_netif_release( void )
{
    fnet_netif_t *net_if_ptr;
    
    fnet_netif_dupip_handler_init(0); /* Reset dupip handler.*/
    
    for (net_if_ptr = fnet_netif_list; net_if_ptr; net_if_ptr = net_if_ptr->next)
    {
        if(net_if_ptr->release)
            net_if_ptr->release(net_if_ptr);

        fnet_netif_del(net_if_ptr);
    }

    fnet_netif_list = fnet_netif_default = 0;
}

/************************************************************************
* NAME: fnet_netif_drain
*
* DESCRIPTION: This function calls "drain" functions of all currently 
*              installed network interfaces. 
*************************************************************************/
void fnet_netif_drain( void )
{
    fnet_netif_t *net_if_ptr;

    fnet_isr_lock();

    for (net_if_ptr = fnet_netif_list; net_if_ptr; net_if_ptr = net_if_ptr->next)
    {
        if(net_if_ptr->drain)
            net_if_ptr->drain(net_if_ptr);
    }

    fnet_isr_unlock();
}


/************************************************************************
* NAME: net_if_find
*
* DESCRIPTION: Returns a network interface given its name.
*************************************************************************/
fnet_netif_desc_t fnet_netif_find_name( char *name )
{
    fnet_netif_t *netif;
    fnet_netif_desc_t result = (fnet_netif_desc_t)0;

    fnet_os_mutex_lock();

    if(name)
        for (netif = fnet_netif_list; netif != 0; netif = netif->next)
        {
            if(fnet_strncmp(name, netif->name, FNET_NETIF_NAMELEN) == 0)
            {
                result = (fnet_netif_desc_t)netif;
                break;
            }
        }

    fnet_os_mutex_unlock();
    return result;
}


/************************************************************************
* NAME: net_if_find
*
* DESCRIPTION: Returns a network interface given its address.
*************************************************************************/
fnet_netif_desc_t fnet_netif_find_address( fnet_ip_addr_t addr )
{
    fnet_netif_t *netif;
    fnet_netif_desc_t result = (fnet_netif_desc_t)0;

    fnet_os_mutex_lock();

    for (netif = fnet_netif_list; netif != 0; netif = netif->next)
    {
        if(addr == netif->addr.address)
        {
            result = (fnet_netif_desc_t)netif;
            break;
        }
    }

    fnet_os_mutex_unlock();
    return result;
}


/************************************************************************
* NAME: fnet_netif_add
*
* DESCRIPTION: This function installs a network interface.
*************************************************************************/
void fnet_netif_add( fnet_netif_t *netif )
{
    if(netif)
    {
        fnet_os_mutex_lock();
        netif->next = fnet_netif_list;

        if(netif->next != 0)
            netif->next->prev = netif;

        netif->prev = 0;
        fnet_netif_list = netif;
        fnet_os_mutex_unlock();
    }
}

/************************************************************************
* NAME: fnet_netif_del
*
* DESCRIPTION: This function releases a network interface.
*************************************************************************/
void fnet_netif_del( fnet_netif_t *netif )
{
    if(netif)
    {
        fnet_os_mutex_lock();

        if(netif->prev == 0)
            fnet_netif_list = netif->next;
        else
            netif->prev->next = netif->next;

        if(netif->next != 0)
            netif->next->prev = netif->prev;

        fnet_os_mutex_unlock();
    }
}

/************************************************************************
* NAME: fnet_netif_set_default
*
* DESCRIPTION: This function sets the default network interface.
*************************************************************************/
void fnet_netif_set_default( fnet_netif_desc_t netif_desc )
{
    if(netif_desc)
    {
        fnet_os_mutex_lock();
        fnet_netif_default = netif_desc;
        fnet_os_mutex_unlock();
    }
}

/************************************************************************
* NAME: fnet_netif_set_address
*
* DESCRIPTION: This function sets the IP address.
*************************************************************************/
void fnet_netif_set_address( fnet_netif_desc_t netif_desc, fnet_ip_addr_t ipaddr )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    fnet_os_mutex_lock();

    if(netif_desc)
    {
        netif->addr.address = ipaddr; /* the IP address */
        netif->addr.is_automatic = 0; /* Adress is set manually. */

        if(FNET_IP_CLASS_A(netif->addr.address))
        {
            if(netif->addr.subnetmask == 0)
                netif->addr.subnetmask = FNET_IP_CLASS_A_NET;

            netif->addr.netmask = FNET_IP_CLASS_A_NET;
        }
        else
        {
            if(FNET_IP_CLASS_B(netif->addr.address))
            {
                if(netif->addr.subnetmask == 0)
                    netif->addr.subnetmask = FNET_IP_CLASS_B_NET;

                netif->addr.netmask = FNET_IP_CLASS_B_NET;
            }
            else
            {
                if(FNET_IP_CLASS_C(netif->addr.address))
                {
                    if(netif->addr.subnetmask == 0)
                        netif->addr.subnetmask = FNET_IP_CLASS_C_NET;

                    netif->addr.netmask = FNET_IP_CLASS_C_NET;
                }
                /* else: Is not supported */
            }
        }

        netif->addr.net = netif->addr.address & netif->addr.netmask;             // network address
        netif->addr.subnet = netif->addr.address & netif->addr.subnetmask;       // network and subnet address

        netif->addr.netbroadcast = netif->addr.address | (~netif->addr.netmask); // network broadcast address
        netif->addr.subnetbroadcast = netif->addr.address
                                          | (~netif->addr.subnetmask);           // subnet broadcast address

        if(netif->set_addr_notify)
            netif->set_addr_notify(netif);
    }

    fnet_os_mutex_unlock();

}

/************************************************************************
* NAME: fnet_netif_set_netmask
*
* DESCRIPTION: This function sets the subnet mask.
*************************************************************************/
void fnet_netif_set_netmask( fnet_netif_desc_t netif_desc, fnet_ip_addr_t netmask )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    if(netif)
    {
        fnet_os_mutex_lock();
        netif->addr.subnetmask = netmask;
        netif->addr.is_automatic = 0;

        netif->addr.subnet = netif->addr.address & netif->addr.subnetmask; // network and subnet address
        netif->addr.subnetbroadcast = netif->addr.address
                                          | (~netif->addr.subnetmask);     // subnet broadcast address
        fnet_os_mutex_unlock();
    }
}

/************************************************************************
* NAME: fnet_netif_set_gateway
*
* DESCRIPTION: This function sets the gateway IP address.
*************************************************************************/
void fnet_netif_set_gateway( fnet_netif_desc_t netif_desc, fnet_ip_addr_t gw )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    if(netif)
    {
        fnet_os_mutex_lock();
        netif->addr.gateway = gw;
        netif->addr.is_automatic = 0;
        fnet_os_mutex_unlock();
    }
}

/************************************************************************
* NAME: fnet_netif_get_default
*
* DESCRIPTION: This function gets the default network interface.
*************************************************************************/
fnet_netif_desc_t fnet_netif_get_default( void )
{
    return (fnet_netif_desc_t)fnet_netif_default;
}

/************************************************************************
* NAME: fnet_netif_get_address
*
* DESCRIPTION: This function returns the IP address of the net interface.
*************************************************************************/
fnet_ip_addr_t fnet_netif_get_address( fnet_netif_desc_t netif_desc )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    return netif ? (netif->addr.address) : 0;
}

/************************************************************************
* NAME: fnet_netif_get_netmask
*
* DESCRIPTION: This function returns the netmask of the net interface.
*************************************************************************/
fnet_ip_addr_t fnet_netif_get_netmask( fnet_netif_desc_t netif_desc )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    return netif ? (netif->addr.subnetmask) : 0;
}

/************************************************************************
* NAME: fnet_netif_get_gateway
*
* DESCRIPTION: This function returns the gateway IP address of 
*              the net interface.
*************************************************************************/
fnet_ip_addr_t fnet_netif_get_gateway( fnet_netif_desc_t netif_desc )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    return netif ? (netif->addr.gateway) : 0;
}

/************************************************************************
* NAME: fnet_netif_get_name
*
* DESCRIPTION: This function returns network interface name (e.g. "eth0", "loop").
*************************************************************************/
void fnet_netif_get_name( fnet_netif_desc_t netif_desc, char *name, unsigned char name_size )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    if(netif)
        fnet_strncpy(name, netif->name, name_size);
}

/************************************************************************
* NAME: fnet_netif_address_automatic
*
* DESCRIPTION: This function returns 0 if the IP address is set 
*              statically/manually, and returns 1 if the IP address is 
*              obtained automatically (by DHCP).
*************************************************************************/
int fnet_netif_address_automatic( fnet_netif_desc_t netif_desc )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;
    return netif ? (netif->addr.is_automatic) : 0;
}

/************************************************************************
* NAME: fnet_netif_address_set_automatic
*
* DESCRIPTION: This function set flag that IP address was 
*              obtained automatically (by DHCP). Called only by DHCP client.
*************************************************************************/
void fnet_netif_address_set_automatic( fnet_netif_desc_t netif_desc )
{
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    if(netif)
        netif->addr.is_automatic = 1;
}

/************************************************************************
* NAME: fnet_netif_get_hw_addr
*
* DESCRIPTION: This function reads HW interface address. 
*              (MAC address in case of Ethernet interface)
*************************************************************************/
int fnet_netif_get_hw_addr( fnet_netif_desc_t netif_desc, unsigned char *hw_addr, unsigned char hw_addr_size )
{
    int result;
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    fnet_os_mutex_lock();

    if(netif && hw_addr && hw_addr_size && netif->get_hw_addr)
        result = netif->get_hw_addr(netif, hw_addr, hw_addr_size);
    else
        result = FNET_ERR;

    fnet_os_mutex_unlock();

    return result;
}

/************************************************************************
* NAME: fnet_netif_set_hw_addr
*
* DESCRIPTION: This function sets HW interface address. 
*              (MAC address in case of Ethernet interface)
*************************************************************************/
int fnet_netif_set_hw_addr( fnet_netif_desc_t netif_desc, unsigned char *hw_addr, unsigned char hw_addr_size )
{
    int result;
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    fnet_os_mutex_lock();

    if(netif && hw_addr && hw_addr_size && netif->get_hw_addr)
        result = netif->set_hw_addr(netif, hw_addr, hw_addr_size);
    else
        result = FNET_ERR;

    fnet_os_mutex_unlock();

    return result;
}

/************************************************************************
* NAME: fnet_netif_type
*
* DESCRIPTION: This function returns type of the network interface.
*************************************************************************/
fnet_netif_type_t fnet_netif_type( fnet_netif_desc_t netif_desc )
{
    fnet_netif_type_t result;
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    if(netif)
        result = netif->type;
    else
        result = FNET_NETIF_TYPE_OTHER;

    return result;
}

/************************************************************************
* NAME: fnet_netif_connected
*
* DESCRIPTION: This function gets physical link status.
*************************************************************************/
int fnet_netif_connected( fnet_netif_desc_t netif_desc )
{
    int result;
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    if(netif && netif->is_connected)
        result = netif->is_connected(netif);
    else
        result = 1; /* Is connected by default; */

    return result;
}

/************************************************************************
* NAME: fnet_get_statistics
*
* DESCRIPTION: This function returns network interface statistics.
*************************************************************************/
int fnet_netif_get_statistics( fnet_netif_desc_t netif_desc, struct fnet_netif_statistics *statistics )
{
    int result;
    fnet_netif_t *netif = (fnet_netif_t *)netif_desc;

    if(netif && statistics && netif->get_statistics)
        result = netif->get_statistics(netif, statistics);
    else
        result = FNET_ERR;

    return result;
}

/************************************************************************
* NAME: fnet_netif_dupip_handler_init
*
* DESCRIPTION:
************************************************************************/
void fnet_netif_dupip_handler_init(fnet_netif_dupip_handler_t handler)
{
    fnet_netif_dupip_handler = handler;
}

/************************************************************************
* NAME: fnet_netif_dupip_handler_signal
*
* DESCRIPTION:
************************************************************************/
void fnet_netif_dupip_handler_signal(fnet_netif_desc_t netif )
{
    if(fnet_netif_dupip_handler)
        fnet_netif_dupip_handler(netif);

}
