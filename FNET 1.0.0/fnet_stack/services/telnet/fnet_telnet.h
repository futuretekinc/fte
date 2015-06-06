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
**********************************************************************/
/*!
*
* @file fnet_telnet.h
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.16.0
*
* @brief FNET Telnet Server API.
*
***************************************************************************/

#ifndef _FNET_TELNET_H_

#define _FNET_TELNET_H_

#include "fnet_config.h"

/*! @cond */
#if FNET_CFG_TELNET
/*! @endcond */

#include "fnet.h"
#include "fnet_poll.h"


/*! @addtogroup fnet_telnet
* The Telnet server provides a simple command-line interface for a 
* remote host via a virtual terminal connection. @n
* Current version of the Telnet server supports maximum of one simultaneously 
* connected Telnet client.@n
* @n
* After the FNET Telnet server is initialized by calling the @ref fnet_telnet_init() 
* function, the user application should call the main service polling function  
* @ref fnet_poll() periodically in background. @n
*
* For example:
* @code
*
*...
*
* const struct fnet_shell fapp_telnet_shell =
* {
*    fapp_telnet_cmd_table,                                 
*    (sizeof(fapp_telnet_cmd_table) / sizeof(struct fnet_shell_command)), 
*    "TELNET>",                                
*    fapp_shell_init,                                      
* };
*
*...
*
* void main()
* {
*    struct fnet_telnet_params params;
* 
*    ...
*    
*    params.ip_address = FNET_HTONL(INADDR_ANY);             
*    params.port = FNET_HTONS(0);       //Default port number.
*    params.shell= &fapp_telnet_shell;
*    params.cmd_line_buffer = fapp_telnet_cmd_line_buffer;
*    params.cmd_line_buffer_size = sizeof(fapp_telnet_cmd_line_buffer)
*
*    // Init Telnet server.
*    fapp_telnet_desc = fnet_http_init(&params);
*    if(fapp_telnet_desc != FNET_ERR)
*    {
*        fnet_printf("\n FNET Telnet server started.\n");
*        while(1)
*        {
*           // Do something.
*           ...
*           fnet_poll();
*        }
*    }
*    else
*    {
*        fnet_printf("\n FNET Telnet server initialization is failed.\n");
*    }
*
* }
* @endcode
* For Telnet usage example, refer to FNET demo application source code.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_TELNET
* - @ref FNET_CFG_TELNET_MAX   
* - @ref FNET_CFG_TELNET_PORT 
* - @ref FNET_CFG_TELNET_SHELL_ECHO
* - @ref FNET_CFG_TELNET_SOCKET_BUF_SIZE 
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief Input parameters for @ref fnet_telnet_init().
 ******************************************************************************/
struct fnet_telnet_params
{
    fnet_ip_addr_t ip_address;  /**< @brief Server IP address (in network byte order). @n
                                 *   It may be set to the @ref INADDR_ANY, so the server will listen 
                                 *   to all current network interfaces. 
                                 */
    unsigned short port;        /**< @brief Server port number (in network byte order). @n
                                 *   In case the parameter is set to @c 0 
                                 *   the server port number will be assigned to
                                 *   the default port number defined by the @ref FNET_CFG_TELNET_PORT.
                                 */
    const struct fnet_shell *shell;     /**< @brief Root-shell structure. */
    char *cmd_line_buffer;              /**< @brief Command-line buffer. */
    unsigned int cmd_line_buffer_size;  /**< @brief Size of the command-line buffer. 
                                         * It defines the maximum length of the 
                                         * entered input-command line. */
 
                                 
};

/**************************************************************************/ /*!
 * @brief Telnet server descriptor.
 * @see fnet_telnet_init()
 ******************************************************************************/
typedef long fnet_telnet_desc_t;

/**************************************************************************/ /*!
 * @brief Telnet server states.@n
 * Used mainly for debugging purposes.
 * @see fnet_telnet_state()
 ******************************************************************************/
typedef enum
{
    FNET_TELNET_STATE_DISABLED = 0,     /**< @brief Telnet server service is 
                                         * not initialized.
                                         */
    FNET_TELNET_STATE_LISTENING,        /**< @brief Telnet server is listening 
                                         * for client socket.
                                         */                                         
    FNET_TELNET_STATE_RECEIVING,        /**< @brief Ready to receive data from a Telnet client. */
    FNET_TELNET_STATE_IAC,              /**< @brief Received IAC symbol. */
    FNET_TELNET_STATE_DONT ,            /**< @brief Prepare to send DON'T. */
    FNET_TELNET_STATE_WONT ,            /**< @brief Prepare to send WON'T. */
    FNET_TELNET_STATE_SKIP ,            /**< @brief Ignore next received character.*/
    FNET_TELNET_STATE_CLOSING           /**< @brief Closing Telnet session.*/                               
} fnet_telnet_state_t;

/***************************************************************************/ /*!
 *
 * @brief    Initializes the Telnet Server service.
 *
 * @param params     Initialization parameters defined by @ref fnet_telnet_params.
 *
 * @return This function returns:
 *   - Telnet server descriptor if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_telnet_release()
 *
 ******************************************************************************
 *
 * This function initializes the Telnet server service. It allocates all
 * resources needed, and registers the Telnet server service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function  @ref fnet_poll() periodically to run the Telnet server in background.@n
 * The Telnet service executes parsing of user-entered commands received via 
 * the Telnet protocol, and calls user-defined command functions, 
 * if the parsing was successful.
 *
 ******************************************************************************/
fnet_telnet_desc_t fnet_telnet_init( struct fnet_telnet_params * params);

/***************************************************************************/ /*!
 *
 * @brief    Releases the Telnet Server service.
 *
 * @param desc     Telnet server descriptor to be unregistered.
 *
 * @see fnet_telnet_init()
 *
 ******************************************************************************
 *
 * This function releases the Telnet Server assigned to the @c desc 
 * descriptor.@n 
 * It releases all occupied resources, and unregisters the Telnet service from 
 * the polling list.
 *
 ******************************************************************************/
void fnet_telnet_release(fnet_telnet_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Closes the currently active session of the Telnet Server.
 *
 * @param desc     Telnet server descriptor
 *
 ******************************************************************************
 *
 * This function closes the current Telnet session.@n
 * It can be used in a Telnet user-command to close the current 
 * session. This is the alternative to closure of the Telnet-client terminal applicatioin.
 *
 ******************************************************************************/
void fnet_telnet_close_session(fnet_telnet_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current state of the Telnet Server service.
 *
 * @param desc     Telnet server descriptor
 *
 * @return This function returns the current state of the Telnet Server service.
 *   The state is defined by @ref fnet_telnet_state_t.
 *
 ******************************************************************************
 *
 * This function returns the current state of the Telnet Server service.
 * If the state is @ref FNET_TELNET_STATE_DISABLED the Telnet Server service
 * is not initialized, or is released.
 *
 ******************************************************************************/
fnet_telnet_state_t fnet_telnet_state(fnet_telnet_desc_t desc);

/*! @} */

/*! @cond */
#endif
/*! @endcond */

#endif
