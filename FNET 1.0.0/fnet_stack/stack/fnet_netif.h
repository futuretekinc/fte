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
* @file fnet_netif.h
*
* @author Andrey Butok
*
* @date Feb-4-2011
*
* @version 0.1.8.0
*
* @brief FNET Network interface API.
*
***************************************************************************/

#ifndef _FNET_NETIF_H_

#define _FNET_NETIF_H_

#include "fnet_config.h"
#include "fnet_ip.h"

/*! @addtogroup fnet_netif
* The Network Interface API allows an application to control 
* various interface parameters, such as the IP address, the gateway address, 
* the subnet mask, and others.
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief  Network interface types.
 ******************************************************************************/
typedef enum
{
    FNET_NETIF_TYPE_OTHER,      /**< @brief Unspecified interface. 
                                 */
    FNET_NETIF_TYPE_ETHERNET,   /**< @brief Ethernet interface
                                 */
    FNET_NETIF_TYPE_LOOPBACK    /**< @brief Loopback interface.
                                 */
} fnet_netif_type_t;

/**************************************************************************/ /*!
 * @brief  Network interface statistics, used by the @ref fnet_netif_get_statistics().
 ******************************************************************************/
struct fnet_netif_statistics
{
    unsigned long tx_packet; /**< @brief Tx packet count.
                              */
    unsigned long rx_packet; /**< @brief Rx packet count.
                              */
};

/**************************************************************************/ /*!
 * @brief The maximum length of a network interface name.
 ******************************************************************************/
#define FNET_NETIF_NAMELEN  (5)

/**************************************************************************/ /*!
 * @brief Network interface descriptor.
 ******************************************************************************/
typedef void *fnet_netif_desc_t;


/***************************************************************************/ /*!
 *
 * @brief    Looks for a network interface according to the specified name.
 *
 *
 * @param name       The name string of a network interface.@n Maximum length of
 *                   the interface name is defined by the @ref FNET_NETIF_NAMELEN.
 *
 * @return This function returns:
 *   - Network interface descriptor that matches the @c name parameter.
 *   - @c 0 if there is no match.
 *
 * @see fnet_netif_find_address()
 *
 ******************************************************************************
 *
 * This function scans the global network interface list looking for a network 
 * interface matching the @c name parameter (for example "eth0", "loop").
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_find_name( char *name );

/***************************************************************************/ /*!
 *
 * @brief    Looks for a network interface according to the specified IP address.
 *
 *
 * @param addr       The IP address of a network interface.
 *
 * @return This function returns:
 *   - Network interface descriptor that matches the @c addr parameter.
 *   - @c 0 if there is no match.
 *
 * @see fnet_netif_find_name()
 *
 ******************************************************************************
 *
 * This function scans the global network interface list looking for a network 
 * interface matching the @c addr parameter.
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_find_address( fnet_ip_addr_t addr );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a name of the specified network interface.
 *
 *
 * @param netif          Network interface descriptor.
 *
 * @param name           String buffer that receives a name of the 
 *                       interface @c netif_desc.
 *
 * @param name_size      Size of the @c name buffer. @n Maximum length of
 *                       the interface name is defined by the @ref FNET_NETIF_NAMELEN.
 *
 * @see fnet_netif_find_name()
 *
 ******************************************************************************
 *
 * This function retrieves the name of the specified network interface 
 * @c netif_desc and stores it in the @c name (for example "eth0", "loop").
 *
 ******************************************************************************/
void fnet_netif_get_name( fnet_netif_desc_t netif, char *name, unsigned char name_size );

/***************************************************************************/ /*!
 *
 * @brief    Assigns the default network interface.
 *
 *
 * @param netif     Network interface descriptor to be assigned as default.
 *
 * @see fnet_netif_get_default()
 *
 ******************************************************************************
 *
 * This function sets the @c netif as the default network interface.@n
 * @n
 * By default, during the FNET stack initialization, the default network interface is
 * assigned to the Ethernet.
 *
 ******************************************************************************/
void fnet_netif_set_default( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the default network interface.
 *
 *
 * @return   This function returns the descriptor of the default network interface.
 *
 * @see fnet_netif_set_default()
 *
 ******************************************************************************
 *
 * This function returns the descriptor of the default network interface.@n
 * @n
 * By default, during the FNET stack initialization, the default network interface is
 * assigned to the Ethernet.
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_get_default( void );

/***************************************************************************/ /*!
 *
 * @brief    Sets the IP address of the specified network interface.
 *
 *
 * @param netif     Network interface descriptor.
 *
 * @param ipaddr    The IP address of the network interface.
 *
 * @see fnet_netif_get_address()
 *
 ******************************************************************************
 *
 * This function sets the IP address of the @c netif interface to the @c ipaddr value.@n
 * Also, it makes a recalculation of the subnet mask of the interface according 
 * to the @c ipaddr. 
 *
 ******************************************************************************/
void fnet_netif_set_address( fnet_netif_desc_t netif, fnet_ip_addr_t ipaddr );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves an IP address of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the IP address of the @c netif interface.
 *
 * @see fnet_netif_set_address()
 *
 ******************************************************************************
 *
 * This function returns the IP address of the @c netif interface.
 *
 ******************************************************************************/
fnet_ip_addr_t fnet_netif_get_address( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Sets the subnet mask of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param netmask   The subnet mask of the network interface.
 *
 * @see fnet_netif_get_netmask()
 *
 ******************************************************************************
 *
 * This function sets the subnet mask of the @c netif interface 
 * to the @c netmask value. 
 *
 ******************************************************************************/
void fnet_netif_set_netmask( fnet_netif_desc_t netif, fnet_ip_addr_t netmask );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a subnet mask of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the subnet mask of the @c netif 
 *               interface.
 *
 * @see fnet_netif_set_netmask()
 *
 ******************************************************************************
 *
 * This function returns the subnet mask of the @c netif interface.
 *
 ******************************************************************************/
fnet_ip_addr_t fnet_netif_get_netmask( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Sets the gateway IP address of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param gw        The gateway IP address of the network interface.
 *
 * @see fnet_netif_get_gateway()
 *
 ******************************************************************************
 *
 * This function sets the gateway IP address of the @c netif interface 
 * to the @c gw value. 
 *
 ******************************************************************************/
void fnet_netif_set_gateway( fnet_netif_desc_t netif, fnet_ip_addr_t gw );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a gateway IP address of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the gateway IP address of the @c netif 
 *               interface.
 *
 * @see fnet_netif_set_gateway()
 *
 ******************************************************************************
 *
 * This function returns the subnet mask of the @c netif interface.
 *
 ******************************************************************************/
fnet_ip_addr_t fnet_netif_get_gateway( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Sets the hardware address of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param hw_addr        Buffer containing the hardware address 
 *                      (for the @ref FNET_NETIF_TYPE_ETHERNET interface type,
 *                       it contains the MAC address).
 *
 * @param hw_addr_size   Size of the hardware address in the @c hw_addr (for the @ref 
 *                       FNET_NETIF_TYPE_ETHERNET interface type, it 
 *                       equals @c 6).
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_netif_get_hw_addr()
 *
 ******************************************************************************
 *
 * This function sets the hardware address of the @c netif interface 
 * to the @c hw_addr value.@n
 * For the @ref FNET_NETIF_TYPE_ETHERNET interface type, this hardware address is 
 * the MAC address.
 *
 ******************************************************************************/
int fnet_netif_set_hw_addr( fnet_netif_desc_t netif, unsigned char *hw_addr, unsigned char hw_addr_size );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a hardware address of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param hw_addr        Buffer that receives a hardware address  
 *                      (for @ref FNET_NETIF_TYPE_ETHERNET interface type,
 *                       it will contain the MAC address).
 *
 * @param hw_addr_size   Size of the hardware address in the @c hw_addr (for the @ref 
 *                       FNET_NETIF_TYPE_ETHERNET interface type, it 
 *                       equals @c 6).
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_netif_set_hw_addr()
 *
 ******************************************************************************
 *
 * This function reads the hardware address of the @c netif interface 
 * and puts it into the @c hw_addr buffer.@n
 * For the @ref FNET_NETIF_TYPE_ETHERNET interface type, this hardware address is 
 * the MAC address.
 *
 ******************************************************************************/
int fnet_netif_get_hw_addr( fnet_netif_desc_t netif, unsigned char *hw_addr, unsigned char hw_addr_size );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the type of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the type of the @c netif 
 *               interface.@n
 *               The type is defined by the @ref fnet_netif_type_t.
 *
 ******************************************************************************
 *
 * This function returns the type of the @c netif interface that is defined 
 * by the @ref fnet_netif_type_t.
 *
 ******************************************************************************/
fnet_netif_type_t fnet_netif_type( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Determines, if the IP address parameters were obtained 
 *           automatically.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns:
 *   - @c 0 if the address IP parameters were set manually .
 *   - @c 1 if the address IP parameters were obtained automatically.
 *
 * @see fnet_netif_set_address(), fnet_netif_set_gateway()
 *
 ******************************************************************************
 *
 * This function determines if the IP parameters of the @c netif interface 
 * were obtained automatically by the DHCP client or were set manually by the
 * @ref fnet_netif_set_address() and @ref fnet_netif_set_gateway().
 *
 ******************************************************************************/
int fnet_netif_address_automatic( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Determines the link status of the network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns:
 *   - @c 0 if the network link is unconnected.
 *   - @c 1 if the network link is connected.
 *
 ******************************************************************************
 *
 * This function determines if the @c netif interface is marked as connected to 
 * a network or not.
 * The Ethernet network interface gets this status from the PHY. 
 *
 ******************************************************************************/
int fnet_netif_connected( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the network interface statistics.
 *
 * @param netif  Network interface descriptor.
 *
 * @param statistics  Structure that receives the network interface statistics 
 *                    defined by the @ref fnet_netif_statistics structure.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs or the network interface driver does 
 *          not support this statistics.
 *
 ******************************************************************************
 *
 * This function retrieves the network statistics of the @c netif interface 
 * and puts it into the @c statistics defined by the @ref fnet_netif_statistics 
 * structure.
 *
 ******************************************************************************/
int fnet_netif_get_statistics( fnet_netif_desc_t netif, struct fnet_netif_statistics *statistics );

/**************************************************************************/ /*!
 * @brief Event handler callback function prototype, that is 
 * called when there is an IP address conflict with another system 
 * on the network.
 * 
 * @param netif      Network interface descriptor that has duplicated IP 
 *                   address.
 *
 * @see fnet_socket_rx_handler_init()
 ******************************************************************************/
typedef void(*fnet_netif_dupip_handler_t)( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Registers the "duplicated IP address" event handler.
 *
 * @param handler    Pointer to the event-handler function defined by 
 *                   @ref fnet_netif_dupip_handler_t.
 *
 ******************************************************************************
 *
 * This function registers the @c handler callback function for 
 * the "duplicated IP address" event. This event occurs when there is 
 * an IP address conflict with another system on the network. @n
 * To stop the event handling, set @c handler parameter to zero value.
 *
 ******************************************************************************/
void fnet_netif_dupip_handler_init (fnet_netif_dupip_handler_t handler);

/*! @} */

#endif
