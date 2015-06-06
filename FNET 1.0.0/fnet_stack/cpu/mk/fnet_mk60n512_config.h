/**************************************************************************
* 
* Copyright 2009 by Andrey Butok. Freescale Semiconductor, Inc. 
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_mk60n512_config.h
*
* @date May-31-2011
*
* @version 0.1.3.0
*
* @brief K60N512 specific configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_MK60N512_CONFIG_H_

#define _FNET_MK60N512_CONFIG_H_

#define FNET_MK                             (1)

/* Maximum Transmission Unit */
#ifndef FNET_CFG_ETH_MTU
#define FNET_CFG_ETH_MTU                    (1500)
#endif

/* Size of the internal static heap buffer. */
#ifndef FNET_CFG_HEAP_SIZE
#define FNET_CFG_HEAP_SIZE                  (30 * 1024)
#endif

/* System frequency in Hz. */
#ifndef FNET_CFG_CPU_CLOCK_HZ
#define FNET_CFG_CPU_CLOCK_HZ               (96000000)
#endif

/* Defines the maximum number of incoming frames that may 
 *           be buffered by the Ethernet module.*/
#ifndef FNET_CFG_CPU_ETH_RX_BUFS_MAX
#define FNET_CFG_CPU_ETH_RX_BUFS_MAX        (4)
#endif

/* The platform has Kinetis Flash Memory Module (FTFL).*/
#define FNET_CFG_CPU_FLASH                  (1)

/*/ Smallest logical block which can be erased independently.*/
#define FNET_CFG_CPU_FLASH_PAGE_SIZE        (2*1024)        /* 2KB sector.*/

/* On-chip Flash size.*/
#define FNET_CFG_CPU_FLASH_SIZE             (1024 * 512)    /* 512 KB */

/* SRAM size.*/
#define FNET_CFG_CPU_SRAM_SIZE              (1024 * 128)    /* 128 KB */  



#endif 
