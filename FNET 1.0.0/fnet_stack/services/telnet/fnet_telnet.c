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
* @file fnet_telnet.c
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.27.0
*
* @brief FNET Telnet Server implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_TELNET

#include "fnet_telnet.h"
#include "fnet_timer.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_shell.h"
#include "fnet_poll.h"

/************************************************************************
*     Definitions
************************************************************************/

#if FNET_CFG_DEBUG_TELNET    
    #define FNET_DEBUG_TELNET   FNET_DEBUG
#else
    #define FNET_DEBUG_TELNET(...)
#endif

#define FNET_TELNET_WAIT_SEND_MS        (2000)  /* ms*/

#define FNET_TELNET_TX_BUFFER_SIZE      FNET_CFG_TELNET_SOCKET_BUF_SIZE
#define FNET_TELNET_RX_BUFFER_SIZE      (10)

#if FNET_TELNET_TX_BUFFER_SIZE > 90     /* Check maximum value for TX application/stream buffer (optional check).*/
    #undef FNET_TELNET_TX_BUFFER_SIZE
    #define FNET_TELNET_TX_BUFFER_SIZE  (90)
#endif

#if (FNET_TELNET_TX_BUFFER_SIZE  < 5)   /* Check minimum value for TX application/stream buffer.*/
    error "FNET_TELNET_TX_BUFFER_SIZE must be > 4"
#endif


/* Keepalive probe retransmit limit.*/
#define FNET_TELNET_TCP_KEEPCNT         (2)

/* Keepalive retransmit interval.*/
#define FNET_TELNET_TCP_KEEPINTVL       (4) /*sec*/

/* Time between keepalive probes.*/
#define FNET_TELNET_TCP_KEEPIDLE        (7) /*sec*/


/* RFC:
* All TELNET commands consist of at least a two byte sequence: the
* "Interpret as Command" (IAC) escape character followed by the code
* for the command. The commands dealing with option negotiation are
* three byte sequences, the third byte being the code for the option
* referenced.
*/
/* RFC:
* Since the NVT is what is left when no options are enabled, the DON’T and
* WON’T responses are guaranteed to leave the connection in a state
* which both ends can handle. Thus, all hosts may implement their
* TELNET processes to be totally unaware of options that are not
* supported, simply returning a rejection to (i.e., refusing) any
* option request that cannot be understood.
*
* The Network Virtual Terminal (NVT) is a bi-directional character
* device.
*/


#define FNET_TELNET_CMD_IAC   ((char)255) /* "Interpret as Command" (IAC) escape character followed by the code for the command. */
#define FNET_TELNET_CMD_WILL  ((char)251) /* Indicates the desire to begin performing, or confirmation that
                                     * you are now performing, the indicated option.*/
#define FNET_TELNET_CMD_WONT  ((char)252) /* Indicates the refusal to perform, or continue performing, the
                                     * indicated option. */
#define FNET_TELNET_CMD_DO    ((char)253) /* Indicates the request that the other party perform, or
                                     * confirmation that you are expecting the other party to perform, the
                                     * indicated option. */
#define FNET_TELNET_CMD_DONT  ((char)254) /* Indicates the demand that the other party stop performing,
                                     * or confirmation that you are no longer expecting the other party
                                     * to perform, the indicated option. */


/************************************************************************
*    Telnet interface control structure.
*************************************************************************/

struct fnet_telnet_if
{
    fnet_telnet_state_t state;              /* Current state.*/

    SOCKET socket_listen;                   /* Listening socket.*/
    SOCKET socket_foreign;                  /* Foreign socket.*/
    
    char tx_buffer[FNET_TELNET_TX_BUFFER_SIZE];     /* Transmit liner buffer */
    int tx_buffer_head_index;                       /* TX buffer index (write place).*/
    char rx_buffer[FNET_TELNET_RX_BUFFER_SIZE];    /* Transmit circular buffer */    
    char *rx_buffer_head;                   /* The Tx circular buffer write pointer. */
    char *rx_buffer_tail;                   /* The Tx_ circular buffer read pointer. */
    char *rx_buffer_end;                    /* Pointer to the end of the Tx circular buffer. */
    
    fnet_poll_desc_t service_descriptor;    /* Descriptor of polling service.*/
    fnet_shell_desc_t shell_descriptor;
    
    struct fnet_shell_params shell_params;
    struct fnet_serial_stream stream;
    
    unsigned long timeout;                  /* Tx timeout.*/
}; 


/* The Telnet interface */ 
static struct fnet_telnet_if telnet_if_list[FNET_CFG_TELNET_MAX];

void fnet_telnet_send(struct fnet_telnet_if *telnet_if);


/************************************************************************
* Buffer functions. 
************************************************************************/
/* Write to Tx liner buffer. */
/* It's posible to write FNET_TELNET_TX_BUFFER_SIZE-1 characters. */
static void tx_buffer_write (struct fnet_telnet_if *telnet_if, char data)
{
   telnet_if->tx_buffer[telnet_if->tx_buffer_head_index] = data;
   telnet_if->tx_buffer_head_index++;
}

/* Free space in Tx Liner buffer. */
static int tx_buffer_free_space(struct fnet_telnet_if *telnet_if)
{
   return(FNET_TELNET_TX_BUFFER_SIZE  - telnet_if->tx_buffer_head_index);   
}


/* Write to Rx circular buffer. */
/* It's posible to write FNET_TELNET_RX_BUFFER_SIZE-1 characters. */
static void rx_buffer_write (struct fnet_telnet_if *telnet_if, char data)
{
   
   *telnet_if->rx_buffer_head = data;
   if(++telnet_if->rx_buffer_head==telnet_if->rx_buffer_end)
      telnet_if->rx_buffer_head=telnet_if->rx_buffer;
}

/* Read from Rx circular buffer. */
static char rx_buffer_read (struct fnet_telnet_if *telnet_if)
{
   char data=*telnet_if->rx_buffer_tail;
   if(++telnet_if->rx_buffer_tail==telnet_if->rx_buffer_end)
      telnet_if->rx_buffer_tail=telnet_if->rx_buffer;
   return data;       
}

/* Free space in Rx circular buffer. */
static int rx_buffer_free_space(struct fnet_telnet_if *telnet_if)
{
   int  space = telnet_if->rx_buffer_tail - telnet_if->rx_buffer_head;
   if (space<=0)
      space += FNET_TELNET_RX_BUFFER_SIZE;    
   
   return (space-1);   
}

/************************************************************************
* NAME: fnet_telnet_putchar
*
* DESCRIPTION: 
************************************************************************/
static void fnet_telnet_putchar(long id, int character)
{
    struct fnet_telnet_if *telnet_if = (struct fnet_telnet_if *)id;
    
    if(telnet_if->state != FNET_TELNET_STATE_CLOSING)
    {
        tx_buffer_write(telnet_if, (char)character);         

        if(tx_buffer_free_space(telnet_if) < 1) /* Buffer is full => flush. */
        {
            fnet_telnet_send(telnet_if);
        }
    }
}

/************************************************************************
* NAME: fnet_telnet_getchar
*
* DESCRIPTION: 
************************************************************************/
static int fnet_telnet_getchar(long id)
{
    struct fnet_telnet_if *telnet_if = (struct fnet_telnet_if *)id;
    
    if(telnet_if->rx_buffer_tail != telnet_if->rx_buffer_head) /* If there is something. */
    {
        return rx_buffer_read (telnet_if);
    }
    else
        return FNET_ERR;
}

/************************************************************************
* NAME: fnet_telnet_flush
*
* DESCRIPTION: 
************************************************************************/
static void fnet_telnet_flush(long id)
{
    struct fnet_telnet_if *telnet_if = (struct fnet_telnet_if *)id;
    
    fnet_telnet_send(telnet_if);
}


/************************************************************************
* NAME: fnet_telnet_send
*
* DESCRIPTION: 
************************************************************************/
static void fnet_telnet_send(struct fnet_telnet_if *telnet)
{
    int res;
    int tx_buffer_tail_index = 0;
    
    telnet->timeout = fnet_timer_ticks();
    
    /* Send all data in the budder.*/
    while(tx_buffer_tail_index != telnet->tx_buffer_head_index)
    {
        if((res = send(telnet->socket_foreign, &telnet->tx_buffer[tx_buffer_tail_index], telnet->tx_buffer_head_index - tx_buffer_tail_index, 0)) != SOCKET_ERROR)
        {
            
            if( (res == 0) && (fnet_timer_get_interval(telnet->timeout, fnet_timer_ticks())
                         > (FNET_TELNET_WAIT_SEND_MS / FNET_TIMER_PERIOD_MS)) ) /* Check timeout */
            {
                FNET_DEBUG_TELNET("TELNET:Send timeout.");
                break; /* Time-out. */
            }
            
            /* Update buffer pointers. */
            tx_buffer_tail_index += res;

            /* Reset timeout. */
            telnet->timeout = fnet_timer_ticks();           
        }
        else /* Error.*/
        {
            FNET_DEBUG_TELNET("TELNET:Send error = %d.", res);
            telnet->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
            break; 
        }
    }
    
    /* Reset TX buffer index. */ 
    telnet->tx_buffer_head_index = 0;
    
    if(telnet->state == FNET_TELNET_STATE_RECEIVING)
    {
        struct sockaddr_in foreign_addr;
        int len;
        /* Close all unwanted connections. */
        closesocket(accept(telnet->socket_listen, (struct sockaddr *) &foreign_addr, &len));
    }
}

/************************************************************************
* NAME: fnet_telnet_send_cmd
*
* DESCRIPTION: Wrie command to the TX buffer.
************************************************************************/
static inline void fnet_telnet_send_cmd(struct fnet_telnet_if *telnet_if, char command, char option)
{
    tx_buffer_write(telnet_if, (char)FNET_TELNET_CMD_IAC);
    tx_buffer_write(telnet_if, command);
    tx_buffer_write(telnet_if, option);
    
    /* Send the command.*/
    fnet_telnet_send(telnet_if);
    
    FNET_DEBUG_TELNET("TELNET: Send option = %d", option);
}

/************************************************************************
* NAME: fnet_telnet_state_machine
*
* DESCRIPTION: Telnet server state machine.
************************************************************************/
static void fnet_telnet_state_machine( void *telnet_if_p )
{

    struct sockaddr_in foreign_addr;
    int res;
    struct fnet_telnet_if *telnet = (struct fnet_telnet_if *)telnet_if_p;
    char rx_data[1];
    int len;

    do
    {
        switch(telnet->state)
        {
            
            /*---- LISTENING ------------------------------------------------*/
            case FNET_TELNET_STATE_LISTENING:
                
                len = sizeof(foreign_addr);
                telnet->socket_foreign = accept(telnet->socket_listen, (struct sockaddr *) &foreign_addr, &len);
                
                if(telnet->socket_foreign != SOCKET_INVALID)
                {
                    #if FNET_CFG_DEBUG_TELNET
                    {
                        char ip_str[16];
                        inet_ntoa(*(struct in_addr *)(&foreign_addr.sin_addr), ip_str);
                        FNET_DEBUG_TELNET("\nTELNET: New connection: %s; Port: %d.", ip_str, fnet_ntohs(foreign_addr.sin_port));
                    }
                    #endif

                    /* Init Shell. */
                    telnet->shell_descriptor = fnet_shell_init(&telnet->shell_params); 
    
                    if(telnet->shell_descriptor == FNET_ERR)
                    {
                        telnet->shell_descriptor = 0;
                        
                        FNET_DEBUG_TELNET("TELNET: Shell Service registration error.");
                        telnet->state = FNET_TELNET_STATE_CLOSING;   /*=> CLOSING */
                    }
                    else
                    {    
                        /* Reset TX timeout. */
                        telnet->timeout = fnet_timer_ticks();
                        telnet->state = FNET_TELNET_STATE_RECEIVING; /* => WAITING data */
                    }

                }
                break;
            /*---- NORMAL -----------------------------------------------*/
            case FNET_TELNET_STATE_RECEIVING:
                { /* Close all unwanted connections. */
                    struct sockaddr_in foreign_addr;
                    int len;
                    closesocket(accept(telnet->socket_listen, (struct sockaddr *) &foreign_addr, &len));
                }
                
                if(rx_buffer_free_space(telnet)>0) 
                {                
                    res = recv(telnet->socket_foreign, rx_data, 1, 0);
                    if(res == 1)
                    {
                        if(rx_data[0] == FNET_TELNET_CMD_IAC )
                        {
                            telnet->state = FNET_TELNET_STATE_IAC; /*=> Handle IAC */
                        }
                        else
                        {
                            rx_buffer_write (telnet, rx_data[0]);
                        }
                    }
                    else if (res == SOCKET_ERROR)
                    {              
                        telnet->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                    }
                }                
                break;
            /*---- IAC -----------------------------------------------*/    
            case FNET_TELNET_STATE_IAC:
                FNET_DEBUG_TELNET("TELNET: STATE_IAC");
                
                if((res = recv(telnet->socket_foreign, rx_data, 1, 0) )!= SOCKET_ERROR)
                {
                    if(res)
                    {
                        switch(rx_data[0])
                        {
                            case FNET_TELNET_CMD_WILL:
                                telnet->state = FNET_TELNET_STATE_DONT;
                                break;
                            case FNET_TELNET_CMD_DO:
                                telnet->state = FNET_TELNET_STATE_WONT;
                                break;                        
                            case FNET_TELNET_CMD_WONT:
                            case FNET_TELNET_CMD_DONT:
                                telnet->state = FNET_TELNET_STATE_SKIP ;
                                break;   
                            case FNET_TELNET_CMD_IAC:
                                /*
                                the IAC need be doubled to be sent as data, and
                                the other 255 codes may be passed transparently.
                                */
                                rx_buffer_write (telnet, rx_data[0]);
                            default:
                                telnet->state = FNET_TELNET_STATE_RECEIVING; /*=> Ignore commands */ 
                        }
                        
                        /* Reset timeout. */
                        telnet->timeout = fnet_timer_ticks();
                    }
                }
                else
                {              
                    telnet->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                }
                break;
            /*---- DONT & WONT -----------------------------------------------*/     
            case FNET_TELNET_STATE_DONT:
            case FNET_TELNET_STATE_WONT:
                {
                    char command;
                    
                    if(telnet->state == FNET_TELNET_STATE_DONT)
                    {
                        FNET_DEBUG_TELNET("TELNET: STATE_DONT");
                        command = FNET_TELNET_CMD_DONT;
                    }
                    else
                    {
                        FNET_DEBUG_TELNET("TELNET: STATE_WONT");
                        command =  FNET_TELNET_CMD_WONT;
                    }
                     
                    if(tx_buffer_free_space(telnet) >= 3)
    	            {
                        res = recv(telnet->socket_foreign, rx_data, 1, 0);
                        
                        if(res == 1)
                        {
                            /* Send command. */
                            fnet_telnet_send_cmd(telnet, command, rx_data[0]);
                            telnet->state = FNET_TELNET_STATE_RECEIVING; 
                        }
                        else if (res == SOCKET_ERROR)
                        {              
                            telnet->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                        }
                    }
                    else
                    {
                        if(fnet_timer_get_interval(telnet->timeout, fnet_timer_ticks())
                         > ((FNET_TELNET_WAIT_SEND_MS*2) / FNET_TIMER_PERIOD_MS)) /* Check timeout */
                        {
                            telnet->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                        }
                    }
                }
                break;
            /*---- SKIP -----------------------------------------------*/                    
            case FNET_TELNET_STATE_SKIP:
                FNET_DEBUG_TELNET("TELNET: STATE_SKIP");
                 
                res = recv(telnet->socket_foreign, rx_data, 1, 0);
                if(res == 1)
                {
                    telnet->state = FNET_TELNET_STATE_RECEIVING; 
                }
                else if (res == SOCKET_ERROR)
                {              
                    telnet->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                }

                break;
            /*---- CLOSING --------------------------------------------*/
            case FNET_TELNET_STATE_CLOSING:
                FNET_DEBUG_TELNET("TELNET: STATE_CLOSING");
                
                if(telnet->shell_descriptor)
                {
                    fnet_shell_release(telnet->shell_descriptor);
                    telnet->shell_descriptor = 0;
                }
                
                telnet->rx_buffer_head=telnet->rx_buffer;
                telnet->rx_buffer_tail=telnet->rx_buffer; 
              
                closesocket(telnet->socket_foreign);
                telnet->socket_foreign = SOCKET_INVALID;
                
                telnet->state = FNET_TELNET_STATE_LISTENING; /*=> LISTENING */
                break;

        }

    }
    while(telnet->state == FNET_TELNET_STATE_CLOSING);
  
}

/************************************************************************
* NAME: fnet_telnet_init
*
* DESCRIPTION: Initialization of the Telnet server.
*************************************************************************/
fnet_telnet_desc_t fnet_telnet_init( struct fnet_telnet_params *params )
{
    struct sockaddr_in local_addr;
    int i;
    struct fnet_telnet_if *telnet_if = 0;
    
    /* Socket options. */
    const struct linger linger_option ={1, /*l_onoff*/
                                     4  /*l_linger*/};
    const unsigned long bufsize_option = FNET_CFG_TELNET_SOCKET_BUF_SIZE;
    const int keepalive_option = 1;
    const int keepcnt_option = FNET_TELNET_TCP_KEEPCNT;
    const int keepintvl_option = FNET_TELNET_TCP_KEEPINTVL;
    const int keepidle_option = FNET_TELNET_TCP_KEEPIDLE;
    
    if(params == 0 )
    {
        FNET_DEBUG_TELNET("TELNET: Wrong init parameters.");
        goto ERROR_1;
    }

    /* Try to find free Telnet server descriptor. */
    for(i=0; i<FNET_CFG_TELNET_MAX; i++)
    {
        if(telnet_if_list[i].state == FNET_TELNET_STATE_DISABLED)
        {
            telnet_if = &telnet_if_list[i];
            break; 
        }
    }
    

    /* No free Telnet server descriptor. */
    if(telnet_if == 0)
    {
        FNET_DEBUG_TELNET("TELNET: No free Telnet Server.");
        goto ERROR_1;
    }

    /* Create listen socket */
    if((telnet_if->socket_listen = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG_TELNET("TELNET: Socket creation error.");
        goto ERROR_1;
    }

    fnet_memset(&local_addr, 0, sizeof(local_addr));
    
    if(params->port)    /* Aply the user-defined port. */
        local_addr.sin_port = params->port;
    else                /* Aply the default port. */
        local_addr.sin_port = FNET_CFG_TELNET_PORT;    

    local_addr.sin_addr.s_addr = params->ip_address;
    local_addr.sin_family = AF_INET;

    if(bind(telnet_if->socket_listen, (struct sockaddr *)(&local_addr), sizeof(local_addr)) == SOCKET_ERROR)
    {
        FNET_DEBUG_TELNET("TELNET: Socket bind error.");
        goto ERROR_2;
    }
    
    /* Set socket options. */    
    if( /* Setup linger option. */
        (setsockopt (telnet_if->socket_listen, SOL_SOCKET, SO_LINGER, (char *)&linger_option, sizeof(linger_option)) == SOCKET_ERROR) ||
         /* Set socket buffer size. */
        (setsockopt(telnet_if->socket_listen, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
        (setsockopt(telnet_if->socket_listen, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
        /* Enable keepalive_option option. */
        (setsockopt (telnet_if->socket_listen, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive_option, sizeof(keepalive_option)) == SOCKET_ERROR) ||
        /* Keepalive probe retransmit limit. */
        (setsockopt (telnet_if->socket_listen, IPPROTO_TCP, TCP_KEEPCNT, (char *)&keepcnt_option, sizeof(keepcnt_option)) == SOCKET_ERROR) ||
        /* Keepalive retransmit interval.*/
        (setsockopt (telnet_if->socket_listen, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&keepintvl_option, sizeof(keepintvl_option)) == SOCKET_ERROR) ||
        /* Time between keepalive probes.*/
        (setsockopt (telnet_if->socket_listen, IPPROTO_TCP, TCP_KEEPIDLE, (char *)&keepidle_option, sizeof(keepidle_option)) == SOCKET_ERROR)
    )
    {
        FNET_DEBUG_TELNET("TELNET: Socket setsockopt() error.");
        goto ERROR_2;
    }
 
    if(listen(telnet_if->socket_listen, 1) == SOCKET_ERROR)
    {
        FNET_DEBUG_TELNET("TELNET: Socket listen error.");
        goto ERROR_2;
    }
    
    /* Register service. */
    telnet_if->service_descriptor = fnet_poll_register(fnet_telnet_state_machine, (void *) telnet_if);
    
    if(telnet_if->service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_TELNET("TELNET: Service registration error.");
        goto ERROR_2;
    }
  
    /* Reset buffer pointers. Move it to init state. */ 
    telnet_if->tx_buffer_head_index = 0;
    telnet_if->rx_buffer_head=telnet_if->rx_buffer;
    telnet_if->rx_buffer_tail=telnet_if->rx_buffer; 
    telnet_if->rx_buffer_end=&telnet_if->rx_buffer[FNET_TELNET_RX_BUFFER_SIZE]; 

    /* Setup stream. */
    telnet_if->stream.id = (long)telnet_if;
    telnet_if->stream.putchar = fnet_telnet_putchar;
    telnet_if->stream.getchar = fnet_telnet_getchar;
    telnet_if->stream.flush = fnet_telnet_flush;
    
    /* Init shell. */
    telnet_if->shell_params.shell = params->shell;
    telnet_if->shell_params.cmd_line_buffer = params->cmd_line_buffer;
    telnet_if->shell_params.cmd_line_buffer_size = params->cmd_line_buffer_size;
    telnet_if->shell_params.stream = &telnet_if->stream;
    telnet_if->shell_params.echo = FNET_CFG_TELNET_SHELL_ECHO;

    telnet_if->socket_foreign = SOCKET_INVALID;
            
    telnet_if->state = FNET_TELNET_STATE_LISTENING;
    
    return (fnet_telnet_desc_t)telnet_if;

ERROR_2:
    closesocket(telnet_if->socket_listen);

ERROR_1:
    return (fnet_telnet_desc_t)FNET_ERR;
}

/************************************************************************
* NAME: fnet_telnet_release
*
* DESCRIPTION: Telnet server release.
************************************************************************/
void fnet_telnet_release(fnet_telnet_desc_t desc)
{
    struct fnet_telnet_if *telnet_if = (struct fnet_telnet_if *) desc;
    
    if(telnet_if && (telnet_if->state != FNET_TELNET_STATE_DISABLED))
    {
        closesocket(telnet_if->socket_foreign);        
        telnet_if->socket_foreign = SOCKET_INVALID;
        closesocket(telnet_if->socket_listen);
        if(telnet_if->shell_descriptor)
        {
            fnet_shell_release(telnet_if->shell_descriptor);
            telnet_if->shell_descriptor = 0;
        }
        fnet_poll_unregister(telnet_if->service_descriptor); /* Delete service.*/
        telnet_if->state = FNET_TELNET_STATE_DISABLED;
    }
}

/************************************************************************
* NAME: fnet_telnet_close_session
*
* DESCRIPTION: Close current Telnet server session.
************************************************************************/
void fnet_telnet_close_session(fnet_telnet_desc_t desc)
{
    struct fnet_telnet_if *telnet_if = (struct fnet_telnet_if *) desc;
    
    if(telnet_if && (telnet_if->state != FNET_TELNET_STATE_DISABLED))
    {
        telnet_if->state = FNET_TELNET_STATE_CLOSING;
    }
}

/************************************************************************
* NAME: fnet_telnet_state
*
* DESCRIPTION: This function returns a current state of the Telnet server.
************************************************************************/
fnet_telnet_state_t fnet_telnet_state(fnet_telnet_desc_t desc)
{
    struct fnet_telnet_if *telnet_if = (struct fnet_telnet_if *) desc;
    fnet_telnet_state_t result;
    
    if(telnet_if)
        result = telnet_if->state;
    else
        result = FNET_TELNET_STATE_DISABLED;
    
    return result;
}



#endif
