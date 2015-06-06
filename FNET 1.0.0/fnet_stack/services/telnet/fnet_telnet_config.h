/**************************************************************************
* 
* Copyright 2005-2009 by Andrey Butok. Freescale Semiconductor, Inc.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License 
* Version 3 or later (the "GPL"), or the GNU Lesser General Public 
* License Version 3 or later (the "LGPL").
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
* @file fnet_telnet_config.h
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.0.8.0
*
* @brief FNET Telnet Server configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_TELNET_CONFIG_H_

#define _FNET_TELNET_CONFIG_H_


/** @addtogroup fnet_services_config */
/** @{ */

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_MAX
 * @brief   Maximum number of the Telnet Servers that can be run simultaneously.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_TELNET_MAX
/*! @endcond */

#define FNET_CFG_TELNET_MAX             (1)

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_PORT
 * @brief   Default Telnet port number (in network byte order).@n
 *          It can be changed during the Telnet server initialization by the 
 *          @ref fnet_telnet_init() function.
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_TELNET_PORT
/*! @endcond */

#define FNET_CFG_TELNET_PORT            (FNET_HTONS(23))

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_SHELL_ECHO
 * @brief    Echo in the Tenet shell:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled (default value).
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_TELNET_SHELL_ECHO
/*! @endcond */

#define FNET_CFG_TELNET_SHELL_ECHO      (0)

/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_SOCKET_BUF_SIZE
 * @brief    Size of the socket RX & TX buffer used by the Telnet server.
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_TELNET_SOCKET_BUF_SIZE
/*! @endcond */

#define FNET_CFG_TELNET_SOCKET_BUF_SIZE (60)

/*! @cond */
#endif
/*! @endcond */



/** @} */

#endif
