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
* @file fnet_dhcp_config.h
*
* @author Andrey Butok
*
* @date Feb-4-2011
*
* @version 0.0.8.0
*
* @brief DHCP Client configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_DHCP_CONFIG_H_

#define _FNET_DHCP_CONFIG_H_

/*! @addtogroup fnet_services_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_DHCP_OPTION_OVERLOAD
 * @brief    DHCP "Option overload" support [RFC 2132 9.3]:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.@n
 * @n
 * It's recomended to disable this option, as it's not used in practice. 
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_DHCP_OPTION_OVERLOAD
/*! @endcond */

#define FNET_CFG_DHCP_OPTION_OVERLOAD   (0)

/*! @cond */
#endif
/*! @endcond */


/*! @} */


#endif
