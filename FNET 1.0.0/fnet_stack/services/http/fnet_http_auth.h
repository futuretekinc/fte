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
* @file fnet_http_auth.h
*
* @author Andrey Butok
*
* @date Jan-18-2011
*
* @version 0.1.4.0
*
* @brief FNET HTTP Server Authentication API.
*
***************************************************************************/

#ifndef _FNET_HTTP_AUTH_H_

#define _FNET_HTTP_AUTH_H_

#include "fnet_config.h"

/*! @cond */
#if FNET_CFG_HTTP && FNET_CFG_HTTP_AUTHENTICATION_BASIC
/*! @endcond */

#include "fnet.h"
#include "fnet_fs.h"

/*! @addtogroup fnet_http
 @{ */

/**************************************************************************/ /*!
 * @brief Type of Authentication Scheme
 * @see fnet_http_auth
 ******************************************************************************/
typedef enum
{
    FNET_HTTP_AUTH_SCHEME_NONE = 0,     /**< @brief No authentication.
                                         */
    FNET_HTTP_AUTH_SCHEME_BASIC = 1,    /**< @brief Basic Access Authentication Scheme.
                                         */
    FNET_HTTP_AUTH_SCHEME_DIGEST = 2    /**< @brief Digest Access Authentication Scheme (NOT YET IMPLEMENTED).
                                         */                                         
} fnet_http_auth_scheme_t;

 
 /**************************************************************************/ /*!
 * @brief HTTP Authentication table.
 *
 * HTTP server protects specified directories from unauthorized access.
 * Directories that are not registered in this table are accessible by anyone.
 * The last table element must have all fields set to zero as the end-of-table mark.@n
 * @n
 *   With HTTP authentication, the following things occur: 
 *   -# A client requests access to a protected resource.
 *   -# The web server returns a dialog box that requests the user name and password. 
 *   -# The client submits the user name and password to the server.
 *   -# The server validates the credentials and, if successful, returns the requested resource.
 * 
 * @see fnet_http_auth_scheme_t, fnet_http_params
 ******************************************************************************/
struct fnet_http_auth
{
	char *dir_name;	                /**< @brief Name of the directory to protect.*/
	char *userid;                   /**< @brief Required user name to access this directory.*/
    char *password;                 /**< @brief Required password to access this directory.*/
    fnet_http_auth_scheme_t scheme; /**< @brief Used Authentication Scheme.*/							
};

/*! @} */

/*! @cond */
#endif
/*! @endcond */

#endif
