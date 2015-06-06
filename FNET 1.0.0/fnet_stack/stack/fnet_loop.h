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
* @file fnet_loop.h
*
* @author Andrey Butok
*
* @date Feb-4-2011
*
* @version 0.1.6.0
*
* @brief Private. Loopback driver function definitions, data structures, etc.
*
***************************************************************************/

#ifndef _FNET_LOOP_H_

#define _FNET_LOOP_H_

#include "fnet_config.h"

#if FNET_CFG_LOOP

#include "fnet_netif_prv.h"

#define FNET_LOOP_MTU        (FNET_CFG_LOOP_MTU) /* The loopback MTU.*/

#if (FNET_LOOP_MTU <200)

#undef FNET_LOOP_MTU
#define FNET_LOOP_MTU  (200)

#endif

/************************************************************************
*     Global Data Structures
*************************************************************************/

extern fnet_netif_t fnet_loop_if;

#define FNET_LOOP_IF    (&fnet_loop_if)

/************************************************************************
*     Function Prototypes
*************************************************************************/

void fnet_loop_output( fnet_netif_t *netif, fnet_ip_addr_t dest_ip_addr, fnet_netbuf_t *nb,
                       int do_not_route );

#endif

#endif
