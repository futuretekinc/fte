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
* @file fnet_http_config.h
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.0.18.0
*
* @brief FNET HTTP Server configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_HTTP_CONFIG_H_

#define _FNET_HTTP_CONFIG_H_

#if !FNET_CFG_FS
    #error The HTTP server uses the File System Interface. Please enable the FNET_CFG_FS in the user configuration. 
    #undef FNET_CFG_HTTP
#endif

/*! @addtogroup fnet_services_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_MAX
 * @brief   Maximum number of the HTTP Servers that can be run simultaneously.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_HTTP_MAX
/*! @endcond */

#define FNET_CFG_HTTP_MAX               (1)

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_SSI
 * @brief   HTTP Server SSI (Server Side Includes) support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/ 
/*! @cond */
#ifndef FNET_CFG_HTTP_SSI
/*! @endcond */

#define FNET_CFG_HTTP_SSI               (1) 

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_CGI
 * @brief   HTTP Server CGI (Common Gateway Interface) support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_HTTP_CGI
/*! @endcond */

#define FNET_CFG_HTTP_CGI               (1) 

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_PORT
 * @brief   Default HTTP port number (in network byte order).@n
 *          It can be changed during the HTTP server initialization by the 
 *          @ref fnet_http_init() function.
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_HTTP_PORT
/*! @endcond */

#define FNET_CFG_HTTP_PORT              (FNET_HTONS(80))

/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_REQUEST_SIZE_MAX
 * @brief   Maximum size of an incoming request.@n 
 *          Also it defines the maximum number of bytes to use for internal 
 *          buffering (parsing, receive and transmit buffering).
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_HTTP_REQUEST_SIZE_MAX
/*! @endcond */

#define FNET_CFG_HTTP_REQUEST_SIZE_MAX  (300) 

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_VERSION_MAJOR
 * @brief   Hypertext Transfer Protocol HTTP version 1.x support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled (HTTP/0.9).
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_HTTP_VERSION_MAJOR
/*! @endcond */

#define FNET_CFG_HTTP_VERSION_MAJOR     (0) 

/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_AUTHENTICATION_BASIC
 * @brief   The HTTP/1.x Basic Authentification Scheme (RFC2617) support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_HTTP_AUTHENTICATION_BASIC
/*! @endcond */

#define FNET_CFG_HTTP_AUTHENTICATION_BASIC  (0) 

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_POST
 * @brief   The HTTP/1.x POST method support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_HTTP_POST
/*! @endcond */

#define FNET_CFG_HTTP_POST                  (0) 

/*! @cond */
#endif
/*! @endcond */


/*! @cond */
#if FNET_CFG_HTTP_AUTHENTICATION_BASIC || FNET_CFG_HTTP_POST 
/*! @endcond */
    /* Push HTTP/1.0*/
/*! @cond */
    #undef FNET_CFG_HTTP_VERSION_MAJOR
    #define FNET_CFG_HTTP_VERSION_MAJOR     (1)
/*! @endcond */    
/*! @cond */
#endif
/*! @endcond */


/*! @} */

#endif
