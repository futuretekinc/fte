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
* @file fapp_dhcp.c
*
* @author Andrey Butok
*
* @date May-31-2011
*
* @version 0.1.24.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_dhcp.h"

#if FTE_BL_CFG_DHCP_CMD && FNET_CFG_DHCP
/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_DHCP_DISCOVER_STR      "Sending DHCP discover..."
#define FAPP_DHCP_NEWIP_STR         " DHCP has updated/renewed parameters:"

/************************************************************************
*     Function Prototypes
*************************************************************************/
#define FAPP_DHCP_COMMAND_REBOOT    "reboot"

static long fapp_dhcp_discover_counter;
static fnet_ip_addr_t fapp_dhcp_ip_old;

/************************************************************************
* NAME: fapp_dhcp_block_exit
*
* DESCRIPTION:
************************************************************************/
static void fapp_dhcp_on_ctrlc(fnet_shell_desc_t desc)
{
    /* Release DHCP. */
    fapp_dhcp_release();
    /* Restore old ip address, as DHCP set it to zero. */
    fnet_netif_set_address( fapp_default_netif, fapp_dhcp_ip_old );    
    fnet_shell_println( desc, FTE_BL_CANCELLED_STR); 
    fnet_shell_ctrlc(desc);
}

/************************************************************************
* NAME: fapp_newip_handler
*
* DESCRIPTION: Event handler on new IP from DHCP client. 
************************************************************************/
static void fapp_dhcp_newip_handler( fnet_netif_desc_t netif, void *shl_desc )
{
    
    fnet_ip_addr_t local_ip;
    fnet_ip_addr_t local_netmask;
    fnet_ip_addr_t local_gateway;
    
    char name[FNET_NETIF_NAMELEN];
    char ip_str[16];
    
    fnet_shell_desc_t desc = (fnet_shell_desc_t) shl_desc;

    fapp_dhcp_discover_counter = -1; /* Infinite for future. */
    
    /* Optionally, unregister DHCP event handlers, just to do not 
     * disturb a user. */
    fnet_dhcp_newip_handler_init(0, 0);
    fnet_dhcp_discover_handler_init(0, 0);
    
    fnet_shell_unblock((fnet_shell_desc_t)shl_desc); /* Unblock the shell. */
   
    /* Print updated parameters info. */
    local_ip = fnet_netif_get_address(netif);
    local_netmask = fnet_netif_get_netmask(netif);
    local_gateway = fnet_netif_get_gateway(netif);
    
    fnet_shell_println( desc, "\n%s", FTE_BL_DELIMITER_STR);
    fnet_shell_println( desc, FAPP_DHCP_NEWIP_STR);
    fnet_shell_println( desc, FTE_BL_DELIMITER_STR);

    fnet_netif_get_name(netif, name, FNET_NETIF_NAMELEN);
    fnet_shell_println( desc, " Interface    : %s", name);
    inet_ntoa(*(struct in_addr *)( &local_ip), ip_str);
    fnet_shell_println( desc, " IP address   : %s", ip_str);
    inet_ntoa(*(struct in_addr *)( &local_netmask), ip_str);
    fnet_shell_println( desc, " Subnet mask  : %s", ip_str);
    inet_ntoa(*(struct in_addr *)( &local_gateway), ip_str);
    fnet_shell_println( desc, " Gateway      : %s\n", ip_str);
}

/************************************************************************
* NAME: fapp_newip_handler
*
* DESCRIPTION: Event handler on new IP from DHCP client. 
************************************************************************/
static void fapp_dhcp_discover_handler( fnet_netif_desc_t netif,void *shl_desc )
{
    fnet_shell_desc_t desc = (fnet_shell_desc_t) shl_desc;
    FNET_COMP_UNUSED_ARG(netif);
    
    if(fapp_dhcp_discover_counter-- == 0)
    {
        fnet_shell_unblock((fnet_shell_desc_t)shl_desc);
        /* Release DHCP. */
        fapp_dhcp_release();
        /* Restore old ip address, as DHCP set it to zero. */
        fnet_netif_set_address( fapp_default_netif, fapp_dhcp_ip_old );
        fnet_shell_set_cmd_ret( desc, -1);    
    }
    else
        fnet_shell_println(desc, FAPP_DHCP_DISCOVER_STR);
}

/************************************************************************
* NAME: fapp_dhcp_release
*
* DESCRIPTION: Releases DHCP client.
*************************************************************************/
void fapp_dhcp_release()
{
    fnet_dhcp_release();
}

/************************************************************************
* NAME: fapp_dhcp_cmd
*
* DESCRIPTION: Enable DHCP client. 
************************************************************************/
int fapp_dhcp_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_dhcp_params dhcp_params;
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *)desc;
    
    if(argc == 1    /* By default is "init".*/
#if 0 /* DHCP reboot feature not used too much. */
    || fnet_strcasecmp(&FAPP_DHCP_COMMAND_REBOOT[0], argv[1]) == 0
#endif    
    ) /* [reboot] */
    {
               
        fnet_memset(&dhcp_params, 0, sizeof(struct fnet_dhcp_params));

        fapp_dhcp_discover_counter = FTE_BL_CFG_DHCP_CMD_DISCOVER_MAX; /* reset counter.*/
        
#if 0 /* DHCP reboot feature not used too much. */
        if(fnet_strcasecmp(&FAPP_DHCP_COMMAND_REBOOT[0], argv[1]) == 0) /* [reboot] */
            dhcp_params.requested_ip_address.s_addr = fnet_netif_get_address(fapp_default_netif);
#endif            

        fapp_dhcp_ip_old = fnet_netif_get_address( fapp_default_netif ); /* Save ip to restore if cancelled. */
        
        /* Enable DHCP client */
        if(fnet_dhcp_init(fapp_default_netif, &dhcp_params) != FNET_ERR)
        {
            /* Register DHCP event handlers. */
            fnet_dhcp_newip_handler_init(fapp_dhcp_newip_handler, (void *)desc);
            fnet_dhcp_discover_handler_init(fapp_dhcp_discover_handler, (void *)desc);
            
            fnet_shell_println(desc, FTE_BL_TOCANCEL_STR);
            fnet_shell_block(desc, fapp_dhcp_on_ctrlc); /* Block shell. */
        }
        else
        {
            fnet_shell_println(desc, FTE_BL_INIT_ERR, "DHCP");
            return  -1;
        }
    }
    else if(argc == 2 && fnet_strcasecmp(&FTE_BL_COMMAND_RELEASE[0], argv[1]) == 0) /* [release] */
    {
        fapp_dhcp_release();
    }
    else
    {
        fnet_shell_println(desc, FTE_BL_PARAM_ERR, argv[1]);
        return  -1;
    }
    
    return  0;
}

/************************************************************************
* NAME: fapp_dhcp_info
*
* DESCRIPTION:
*************************************************************************/
void fapp_dhcp_info(fnet_shell_desc_t desc)
{
    char ip_str[16];
    int dhcp_enabled = (fnet_dhcp_state() != FNET_DHCP_STATE_DISABLED);
    int address_automatic = fnet_netif_address_automatic(fapp_default_netif);
    
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "DHCP client", dhcp_enabled ? FTE_BL_SHELL_INFO_ENABLED: FTE_BL_SHELL_INFO_DISABLED);

    if(dhcp_enabled && address_automatic)
    {
        struct fnet_dhcp_options options;
        fnet_dhcp_get_options(&options);

        inet_ntoa(*(struct in_addr *)( &options.server_ip_address), ip_str);
        
        fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "DHCP server", ip_str);
        
        fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_D, "Lease time", fnet_ntohl(options.lease_time));
    }
}

#endif














