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
* @file fnet_loop.c
*
* @author Andrey Butok
*
* @date Feb-4-2011
*
* @version 0.1.7.0
*
* @brief Loopback driver implementation.
*
***************************************************************************/
 
#include "fnet_config.h" 
#if FNET_CFG_LOOP

#include "fnet_loop.h"
#include "fnet_ip_prv.h"

/************************************************************************
*     Global Data Structures
*************************************************************************/
fnet_netif_t fnet_loop_if = 
{
		0,                          /* pointer to the next net_if structure.*/
		0,                          /* pointer to the previous net_if structure.*/
		FNET_NETIF_TYPE_LOOPBACK,   /* data-link type.*/
		"loop",                     /* network interface name.*/
		{0,0,0,0,0,0,0,0},          /* the interface address structure.*/
		FNET_LOOP_MTU,              /* maximum transmission unit.*/
		0,                          /* points to interface specific data structure.*/
		0,                          /* initialization function.*/
		0,                          /* shutdown function.*/
		0,                          /* receive function.*/
		fnet_loop_output,           /* transmit function.*/
		0,                          /* address change notification function.*/
		0                           /* drain function.*/
};

/************************************************************************
* NAME: fnet_loop_output
*
* DESCRIPTION: This function just only sends outgoing packets to IP layer.
*************************************************************************/
void fnet_loop_output(fnet_netif_t *netif, fnet_ip_addr_t dest_ip_addr, fnet_netbuf_t* nb, int do_not_route)
{
    FNET_COMP_UNUSED_ARG(dest_ip_addr);
    FNET_COMP_UNUSED_ARG(do_not_route);
   /* MTU check */
    if ((nb->flags & FNET_NETBUF_FLAG_FIRST)&&(nb->total_length<=netif->mtu))
        fnet_ip_input(netif, nb);
    else
        fnet_netbuf_free_chain(nb);
}

#endif
