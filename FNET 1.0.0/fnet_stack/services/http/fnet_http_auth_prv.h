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
* @file fnet_http_auth_prv.h
*
* @author Andrey Butok
*
* @date Jan-18-2011
*
* @version 0.1.4.0
*
* @brief Private. FNET HTTP Server Authentication API.
*
***************************************************************************/

#ifndef _FNET_HTTP_AUTH_PRV_H_

#define _FNET_HTTP_AUTH_PRV_H_

#include "fnet_config.h"


#if FNET_CFG_HTTP && FNET_CFG_HTTP_AUTHENTICATION_BASIC


#include "fnet.h"
#include "fnet_http_auth.h"

typedef int(*fnet_http_auth_scheme_validate_t)(const struct fnet_http_auth *auth_entry, char * auth_param);
typedef int(*fnet_http_auth_scheme_generate_t)(struct fnet_http_if * http, char *buffer, unsigned int buffer_size);

struct fnet_http_auth_scheme
{
    fnet_http_auth_scheme_t id;
    const char *name;
    fnet_http_auth_scheme_validate_t validate; /* Validate credentials params.*/
    fnet_http_auth_scheme_generate_t generate; /* Generate challenge params.*/
};

void fnet_http_auth_validate_uri(struct fnet_http_if * http);
int fnet_http_auth_validate_credentials(struct fnet_http_if * http, char *credentials);
int fnet_http_auth_generate_challenge(struct fnet_http_if * http, char *buffer, unsigned int buffer_size);

#endif


#endif
