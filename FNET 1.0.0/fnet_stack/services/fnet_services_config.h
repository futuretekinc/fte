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
***********************************************************************/ /*!
*
* @file fnet_services_config.h
*
* @author Andrey Butok
*
* @date Jun-2-2011
*
* @version 0.1.22.0
*
* @brief Services default configuration.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_SERVICES_CONFIG_H_

#define _FNET_SERVICES_CONFIG_H_

#include "fnet_user_config.h" 

/*! @addtogroup fnet_services_config */
/*! @{ */

/**************************************************************************/ /*!
 * @showinitializer
 * @def     FNET_CFG_POLL_MAX
 * @brief   Maximum number of registered services in the polling list.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_POLL_MAX

#define FNET_CFG_POLL_MAX   (5)

#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_DHCP
 * @brief    DHCP Client service support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer  
 ******************************************************************************/
#ifndef FNET_CFG_DHCP

#define FNET_CFG_DHCP       (0)

#endif
/*! @cond */
#if FNET_CFG_DHCP 
/*! @endcond */
    #include "fnet_dhcp_config.h"
/*! @cond */    
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_HTTP
 * @brief    HTTP Server service support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_HTTP

#define FNET_CFG_HTTP       (0)

#endif

#if FNET_CFG_HTTP

/* Force FS if HTTP is defined. */
#undef FNET_CFG_FS
#define FNET_CFG_FS        (1)

#endif

#if FNET_CFG_HTTP 
    #include "fnet_http_config.h"
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS
 * @brief    File System Interface support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_FS

#define FNET_CFG_FS         (0) 

#endif

#if FNET_CFG_FS 
    #include "fnet_fs_config.h"
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_TFTP_CLN
 * @brief    TFTP Client support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer  
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_CLN

#define FNET_CFG_TFTP_CLN   (0)

#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_TFTP_SRV
 * @brief    TFTP Server support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer  
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_SRV

#define FNET_CFG_TFTP_SRV   (0)

#endif


#if FNET_CFG_TFTP_CLN || FNET_CFG_TFTP_SRV  
    #include "fnet_tftp_config.h"
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_FLASH
 * @brief    On-chip Flash driver support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer  
 ******************************************************************************/
#ifndef FNET_CFG_FLASH

#define FNET_CFG_FLASH        (0)

#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_TELNET
 * @brief    Telnet server support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer  
 ******************************************************************************/
#ifndef FNET_CFG_TELNET

#define FNET_CFG_TELNET       (0)

#endif

#if FNET_CFG_TELNET 
    #include "fnet_telnet_config.h"
#endif

/* Include Serial Library default configuration. */
#include "fnet_serial_config.h"

/* Include Shell Library default configuration. */
#include "fnet_shell_config.h"


/*! @} */

#endif
