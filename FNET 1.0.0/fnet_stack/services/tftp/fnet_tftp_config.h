/**************************************************************************
* 
* Copyright 2005-2010 by Andrey Butok. Freescale Semiconductor, Inc.
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
* @file fnet_tftp_config.h
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.0.8.0
*
* @brief TFTP services configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_TFTP_CONFIG_H_

#define _FNET_TFTP_CONFIG_H_

/*! @addtogroup fnet_services_config */
/*! @{ */


/****************************************************************************** 
 *              TFTP-client service config parameters
 ******************************************************************************/

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_CLN_PORT
 * @brief   TFTP server port number (in network byte order) used by TFTP-client service.
 * @showinitializer 
 ******************************************************************************/ 
/*! @cond */
#ifndef FNET_CFG_TFTP_CLN_PORT
/*! @endcond */

#define FNET_CFG_TFTP_CLN_PORT                  (FNET_HTONS(69))

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_CLN_TIMEOUT
 * @brief   Timeout for TFTP server response in seconds. @n
 *          If no response from a TFTP server is received during this timeout,
 *          the TFTP-client service is released automatically.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_TFTP_CLN_TIMEOUT
/*! @endcond */

#define FNET_CFG_TFTP_CLN_TIMEOUT               (10) /*sec*/

/*! @cond */
#endif
/*! @endcond */




/****************************************************************************** 
 *              TFTP-server service config parameters
 ******************************************************************************/

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_PORT
 * @brief   Default TFTP server port number (in network byte order) used by TFTP-server service.@n
 *          It can be changed during the TFTP server initialization by the 
 *          @ref fnet_tftp_srv_init() function.
 * @showinitializer 
 ******************************************************************************/ 
/*! @cond */
#ifndef FNET_CFG_TFTP_SRV_PORT
/*! @endcond */

#define FNET_CFG_TFTP_SRV_PORT                  (FNET_HTONS(69))

/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_MAX
 * @brief   Maximum number of the TFTP Servers that can be run simultaneously.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_TFTP_SRV_MAX
/*! @endcond */

#define FNET_CFG_TFTP_SRV_MAX                   (1)

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_TIMEOUT
 * @brief   Default timeout for TFTP client response in seconds. @n
 *          If no response from a TFTP client is received during this timeout,
 *          the last packet is retransmitted to the TFTP client automatically.@n
 *          It can be changed during the TFTP server initialization by the 
 *          @ref fnet_tftp_srv_init() function. 
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_TFTP_SRV_TIMEOUT
/*! @endcond */

#define FNET_CFG_TFTP_SRV_TIMEOUT               (3) /*sec*/

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_RETRANSMIT_MAX
 * @brief   Default maximum number of retransmissions. @n
 *          If no response from a TFTP client is received
 *          till maximum retransmission number is reached, 
 *          the TFTP server cancels the data transfer.@n
 *          It can be changed during the TFTP server initialization by the 
 *          @ref fnet_tftp_srv_init() function. 
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_TFTP_SRV_RETRANSMIT_MAX
/*! @endcond */

#define FNET_CFG_TFTP_SRV_RETRANSMIT_MAX        (4) 

/*! @cond */
#endif
/*! @endcond */


/*! @} */


#endif
