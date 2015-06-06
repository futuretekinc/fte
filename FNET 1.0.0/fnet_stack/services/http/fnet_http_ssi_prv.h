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
**********************************************************************/
/*!
*
* @file fnet_http_ssi_prv.h
*
* @author Andrey Butok
*
* @date Oct-7-2010
*
* @version 0.1.6.0
*
* @brief Private. FNET HTTP Server SSI API.
*
***************************************************************************/

#ifndef _FNET_HTTP_SSI_PRV_H_

#define _FNET_HTTP_SSI_PRV_H_

#include "fnet_config.h"


#if FNET_CFG_HTTP && FNET_CFG_HTTP_SSI


#include "fnet.h"

/* SSI statemachine state. */
typedef enum
{
    FNET_HTTP_SSI_WAIT_HEAD = 0,
    FNET_HTTP_SSI_WAIT_TAIL,
    FNET_HTTP_SSI_INCLUDING
}
fnet_http_ssi_state_t;

/* SSI private control structure. */
struct fnet_http_ssi_if
{
    const struct fnet_http_ssi *ssi_table; /* Pointer to the SSI table.*/
    fnet_http_ssi_send_t send;    /* Pointer to the respond callback.*/
    fnet_http_ssi_state_t state;        /* State. */
};

extern const struct fnet_http_file_handler fnet_http_ssi_handler; /* SSI file handler. */


#endif


#endif
