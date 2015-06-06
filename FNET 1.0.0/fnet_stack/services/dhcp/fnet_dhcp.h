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
* @file fnet_dhcp.h
*
* @author Andrey Butok
*
* @date Feb-4-2011
*
* @version 0.1.11.0
*
* @brief DHCP Client API.
*
***************************************************************************/

#ifndef _FNET_DHCP_H_

#define _FNET_DHCP_H_

#include "fnet_config.h"

/*! @cond */
#if FNET_CFG_DHCP
/*! @endcond */

#include "fnet.h"
#include "fnet_poll.h"

/*! @addtogroup fnet_dhcp 
* The user application can use the DHCP client service to retrieve various network 
* parameters (IP address, gateway address, subnet mask) from a DHCP server.@n
* The DHCP client code keeps the IP address parameters updated by renewing it 
* automatically, without user application intervention.@n
* @n
* After the DHCP client is initialized by calling the fnet_dhcp_init() function,
* the user application should call the main service-polling function  
* @ref fnet_poll() periodically in background. @n
* @n
* The DHCP client will be released automatically if an application sets 
* any of the IP-related parameters directly by the @ref fnet_netif_set_address(), 
* @ref fnet_netif_set_netmask(), or @ref fnet_netif_set_gateway() calls. @n
* @n
* For example:
* @code
* ...
*    fnet_netif_desc_t netif;
*    struct fnet_dhcp_params dhcp_params;
*
*    // Get current net interface.
*    if((netif = fnet_netif_get_default()) == 0)
*    {
*        fnet_printf("ERROR: Network Interface is not configurated!");
*    }
*    else
*    {
*        fnet_memset(&dhcp_params, 0, sizeof(struct fnet_dhcp_params));
*
*        // Enable DHCP client.
*        if(fnet_dhcp_init(netif, &dhcp_params) != FNET_ERR)
*        {
*            // Register DHCP event handlers.
*            fnet_dhcp_newip_handler_init(fapp_dhcp_newip_handler, shl);
*            fnet_dhcp_discover_handler_init(fapp_dhcp_discover_handler, shl);
*        }
*        else
*        {
*            fnet_printf("ERROR: DHCP initialization is failed!");
*        }
*    }
* ...
* @endcode
* @n
* Configuration parameters:
* - @ref FNET_CFG_DHCP 
* - @ref FNET_CFG_DHCP_OPTION_OVERLOAD
*
*/
/*! @{ */



/**************************************************************************/ /*!
 * @def FNET_DHCP_LEASE_INFINITY
 * @brief Infinite lease time.@n
 * The lease-option value, when a client acquires a lease for a network 
 * address for infinite time.
 * @showinitializer
 ******************************************************************************/
#define FNET_DHCP_LEASE_INFINITY    (0xFFFFFFFF)

/**************************************************************************/ /*!
 * @brief Minimum lease-time value in seconds.@n
 * If a client-acquired lease time is less than the @ref FNET_DHCP_LEASE_MIN, 
 * the lease time will be pushed to the @ref FNET_DHCP_LEASE_MIN value.
 ******************************************************************************/
#define FNET_DHCP_LEASE_MIN         (180)






/**************************************************************************/ /*!
 * @brief Initialization parameters for the @ref fnet_dhcp_init() function.
 *
 * The user application may use this structure to suggest a network address and/or 
 * the lease time to the DHCP client service.@n
 ******************************************************************************/
struct fnet_dhcp_params
{
    struct in_addr requested_ip_address;    /**< @brief Suggested IP address.@n
                                             * The client can suggest to the DHCP server
                                             * that a particular IP address value should be 
                                             * assigned to the client.@n
                                             * This parameter is optional and can be set to @c 0.
                                             */
    unsigned long requested_lease_time;     /**< @brief Suggested Lease time in seconds.@n
                                             * The client can suggest to the DHCP server
                                             * that a particular lease time should be 
                                             * assigned to the client IP address.@n
                                             * This parameter is optional and can be set to @c 0.
                                             */
};

/**************************************************************************/ /*!
 * @brief DHCP client states.@n
 * Used mainly for debugging purposes.
 ******************************************************************************/
typedef enum
{
    FNET_DHCP_STATE_DISABLED = 0,   /**< @brief The DHCP client service is not 
                                     * initialized.
                                     */
    FNET_DHCP_STATE_INIT,           /**< @brief The DHCP client service is initialized.
                                     * Sends DHCPDISCOVER message.@n
                                     * Signals the @ref fnet_dhcp_discover_handler_t event.
                                     */
    FNET_DHCP_STATE_SELECTING,      /**< @brief Waits for the DHCPOFFER message.
                                     */
    FNET_DHCP_STATE_REQUESTING,     /**< @brief Sends the DHCPREQUEST message.
                                     * Waits for the DHCPACK.
                                     */
    FNET_DHCP_STATE_BOUND,          /**< @brief The DHCPACK message from 
                                     * the DHCP server arrived.
                                     * The client parameters are set.@n
                                     * Signals the @ref fnet_dhcp_newip_handler_t event.
                                     */
    FNET_DHCP_STATE_RENEWING,       /**< @brief T1 expired. Send the DHCPREQUEST
                                     * to a leasing server.
                                     */
    FNET_DHCP_STATE_REBINDING,      /**< @brief T2 expired. Broadcast the DHCPREQUEST.
                                     */
    FNET_DHCP_STATE_INIT_REBOOT,    /**< @brief The DHCP client service is initialized.
                                     * Sends the DHCPREQUEST message.@n
                                     * Signals the @ref fnet_dhcp_discover_handler_t event.
                                     */
    FNET_DHCP_STATE_REBOOTING,      /**< @brief Sends the DHCPREQUEST message.
                                     * Waits for the DHCPACK.
                                     */
    FNET_DHCP_STATE_RELEASE         /**< @brief Sends the RELEASE message.
                                     * Frees the allocated resources.
                                     */
} fnet_dhcp_state_t;

/**************************************************************************/ /*!
 * @brief DHCP options are retrieved from a DHCP server.
 *
 * This structure is used by the @ref fnet_dhcp_get_options() function.@n
 * User application should never assign retrieved IP parameters directly 
 * to network interface. The DHCP client service does it automatically.
 ******************************************************************************/
struct fnet_dhcp_options
{
    struct in_addr ip_address;      /**< @brief Client IP address assigned by the DHCP server.
                                     */
    struct in_addr server_ip_address; /**< @brief The DHCP server IP address.
                                       */
    struct in_addr netmask;         /**< @brief Subnet Mask.
                                     */
    struct in_addr broadcast;       /**< @brief Broadcast address in use on the client's subnet.
                                     * 
                                     */
    struct in_addr gateway;         /**< @brief The IP address of a router on the client's subnet.
                                     */

    /* Internal and for debug needs: */

    unsigned long t1;               /**< @brief Renewal (T1) Time Value in seconds.@n
                                     * This option specifies the time interval from 
                                     * address assignment, until the client transitions 
                                     * to the RENEWING state.@n
                                     * A user application may ignore this option value. 
                                     * It is used for internal and debug purposes only.
                                     */
    unsigned long t2;               /**< @brief Rebinding (T2) Time Value in seconds.@n
                                     * This option specifies the time interval from 
                                     * address assignment until the client transitions 
                                     * to the REBINDING state.@n
                                     * A user application may ignore this option value. 
                                     * It is used for internal and debug purposes only.
                                     */
    unsigned long lease_time;       /**< @brief The IP Address Lease Time in seconds. @n
                                     * @c t1 < @c t2 < @c lease_time.@n
                                     * By default, @c t1=0.5*lease_time; @c t2=0.875*lease_time.@n
                                     * A user application may ignore this option value. 
                                     * It is used for internal and debug purposes only.
                                     */
    unsigned char message_type;     /**< @brief The DHCP Message Type.@n
                                     * This option is used to convey the type of the 
                                     * last DHCP message.@n
                                     * A user application may ignore this option value. 
                                     * It is used for internal and debug purposes only. 	                                     
                                     */

#if FNET_CFG_DHCP_OPTION_OVERLOAD

    unsigned char overload;         /**< @brief Overload Option.@n
                                     * If this option is present, the DHCP client interprets 
                                     * the specified additional fields after it concludes 
                                     * interpretation of the standard option fields.@n
                                     * A user application may ignore this option value. 
                                     * It is used for internal and debug purposes only.
                                     */

#endif
/* Todo DNS, hostname, domainname */

};


/***************************************************************************/ /*!
 *
 * @brief    Initializes the DHCP client service.
 *
 * @param netif      Network interface descriptor to be used by the DHCP client.
 *
 * @param params     Optional initialization parameters.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_dhcp_release()
 *
 ******************************************************************************
 *
 * This function initializes the DHCP client service. It allocates all
 * resources needed and registers the DHCP service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function @ref fnet_poll() periodically to run the DHCP service routine 
 * in the background.
 *
 ******************************************************************************/
int fnet_dhcp_init( fnet_netif_desc_t netif, struct fnet_dhcp_params *params );

/***************************************************************************/ /*!
 *
 * @brief    Releases the DHCP client service.
 *
 * @see fnet_dhcp_init()
 *
 ******************************************************************************
 *
 * This function releases the DHCP client service. It releases all resources 
 * used by the service, and unregisters it from the polling list.
 *
 ******************************************************************************/
void fnet_dhcp_release();

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current state of the DHCP client service.
 *
 * @return This function returns the current state of the DHCP client service.
 *   The state is defined by the @ref fnet_dhcp_state_t.
 *
 ******************************************************************************
 *
 * This function returns the current state of the DHCP client service.
 * If the state is @ref FNET_DHCP_STATE_DISABLED, the DHCP is not initialized
 * or released.
 *
 ******************************************************************************/
fnet_dhcp_state_t fnet_dhcp_state();

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current DHCP client options retrieved from a 
 *           DHCP server.
 *
 * @param options      DHCP options received from a DHCP server.
 *
 ******************************************************************************
 *
 * This function returns the DHCP @c options that were retrieved 
 * by the DHCP client service from a DHCP server.@n
 * These options are valid after the DHCP client reaches the BOUND state, or
 * after the @ref fnet_dhcp_newip_handler_t event.
 *
 ******************************************************************************/
void fnet_dhcp_get_options( struct fnet_dhcp_options *options );

/**************************************************************************/ /*!
 * @brief DHCP event handler callback function prototype, that is 
 * called when the DHCP client has updated the IP parameters (in BOUND state).
 *
 * @param netif     Network interface descriptor which IP parameters were updated.
 *
 * @param param     User-application specific parameter. It's set during 
 *                  the DHCP service initialization as part of the 
 *                  @ref fnet_dhcp_params.
 *
 * @see fnet_dhcp_params
 ******************************************************************************/
 typedef void(*fnet_dhcp_newip_handler_t)(fnet_netif_desc_t netif, void *param);
 
 /**************************************************************************/ /*!
 * @brief DHCP event handler callback function prototype, that is 
 * called when the DHCP client send the DHCP discover message. @n
 * A user interface program may use it for indication of retransmission attempts.
 *
 * @param netif     Network interface descriptor which IP parameters were updated.
 *
 * @param param     User-application specific parameter. It's set during 
 *                  the DHCP service initialization as part of the 
 *                  @ref fnet_dhcp_params.
 *
 * @see fnet_dhcp_newip_handler_init
 ******************************************************************************/
 typedef void(*fnet_dhcp_discover_handler_t)(fnet_netif_desc_t netif, void *param);

/***************************************************************************/ /*!
 *
 * @brief    Registers the "IP parameters updated" DHCP event handler.
 *
 * @param newip_handler Pointer to the callback function defined by 
 *                      @ref fnet_dhcp_newip_handler_t.
 * @param param         Optional application-specific parameter. @n 
 *                      It's passed to the @c newip_handler callback 
 *                      function as input parameter.
 *
 ******************************************************************************
 *
 * This function registers the @c handler callback function for 
 * the "IP parameters updated" event. This event occurs when 
 * the DHCP client has updated the IP parameters. @n
 * To stop the event handling, set @c handler parameter to zero value.
 *
 ******************************************************************************/
void fnet_dhcp_newip_handler_init (fnet_dhcp_newip_handler_t newip_handler, void *param);

/***************************************************************************/ /*!
 *
 * @brief    Registers the "Discover message sent" DHCP event handler.
 *
 * @param discover_handler  Pointer to the callback function defined by 
 *                          @ref fnet_dhcp_discover_handler_t.
 * @param param             Optional application-specific parameter. @n 
 *                          It's passed to the @c discover_handler callback 
 *                          function as input parameter.
 *
 ******************************************************************************
 *
 * This function registers the @c handler callback function for 
 * the "Discover message sent" event. This event occurs when 
 * the DHCP client send the DHCP discover message. A user interface program 
 * may use it for indication of retransmission attempts. @n
 * To stop the event handling, set @c handler parameter to zero value.
 *
 ******************************************************************************/
void fnet_dhcp_discover_handler_init (fnet_dhcp_discover_handler_t discover_handler, void *param);


/*! @} */

/*! @cond */

#endif
/*! @endcond */

#endif
