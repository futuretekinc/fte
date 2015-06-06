/**************************************************************************
* 
* Copyright 2005-2010 by Andrey Butok. Freescale Semiconductor, Inc.
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
* @file fnet_tftp.h
*
* @author Andrey Butok
*
* @date Feb-8-2010
*
* @version 0.1.3.0
*
* @brief TFTP common API.
*
***************************************************************************/

#ifndef _FNET_TFTP_H_

#define _FNET_TFTP_H_

#include "fnet_config.h"

/*! @cond */
#if FNET_CFG_TFTP_CLN || FNET_CFG_TFTP_SRV
/*! @endcond */

#include "fnet.h"



/*! @addtogroup fnet_tftp 
* The user application can use the Trivial File Transfer Protocol (TFTP) client and server to transfer files 
* between machines on different networks implementing UDP.
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief Maximum data size transferred in one data block.
 * @showinitializer
 *
 * The single data transfer is from zero to 512 bytes long. If it is 512 bytes
 * long, the transferred data block is not the last block of data. If it is from 
 * zero to 511 bytes long, it signals the end of transfer.
 ******************************************************************************/
#define FNET_TFTP_DATA_SIZE_MAX         (512)

/**************************************************************************/ /*!
 * @brief The TFTP request type. It defines a TFTP service behavior, 
 * if it will read or write a file from/to a TFTP server or client.
 ******************************************************************************/
typedef enum
{
    FNET_TFTP_REQUEST_READ,   /**< @brief TFTP read request (PRQ). The TFTP client 
                               * reads a file from the TFTP server. 
                               */ 
    FNET_TFTP_REQUEST_WRITE   /**< @brief TFTP write request (WRQ). The TFTP client 
                               * writes a file to the TFTP server. 
                               */
} fnet_tftp_request_t;


/**************************************************************************/ /*!
 * @brief TFTP error codes indicating the nature of the error according 
 * to RFC 1350.
 ******************************************************************************/
typedef enum
{
    FNET_TFTP_ERROR_NOT_DEFINED         = 0,    /**< @brief Not defined, see error 
                                                 * message (if any).   
                                                 */ 
    FNET_TFTP_ERROR_FILE_NOT_FOUND      = 1,    /**< @brief File not found.
                                                 */ 
    FNET_TFTP_ERROR_ACCESS_VIOLATION    = 2,    /**< @brief Access violation.
                                                 */ 
    FNET_TFTP_ERROR_DISK_FULL           = 3,    /**< @brief Disk full or allocation 
                                                 * exceeded.
                                                 */ 
    FNET_TFTP_ERROR_ILLEGAL_OPERATION   = 4,    /**< @brief Illegal TFTP operation.
                                                 */ 
    FNET_TFTP_ERROR_UNKNOWN_TID         = 5,    /**< @brief Unknown transfer ID.
                                                 */ 
    FNET_TFTP_ERROR_FILE_ALREADY_EXISTS = 6,    /**< @brief File already exists.
                                                 */ 
    FNET_TFTP_ERROR_NO_SUCH_USER        = 7     /**< @brief No such user.
                                                 */                                                                                                                                                                                                                                                                         
} fnet_tftp_error_t; 



/*! @} */

/*! @cond */
#endif
/*! @endcond */

#endif
