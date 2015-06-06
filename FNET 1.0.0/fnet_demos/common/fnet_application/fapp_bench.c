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
* @file fapp_bench.c
*
* @author Andrey Butok
*
* @date May-31-2011
*
* @version 0.1.11.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fnet.h"
#include "fnet_shell.h"
#include "fnet_stdlib.h"
#include "fnet_eth.h"
#include "fapp_prv.h"

#include "fapp_bench.h"

#if FAPP_CFG_BENCH_CMD


/************************************************************************
* Benchmark definitions
************************************************************************/
#define FAPP_BENCH_UDP_END_BUFFER_LENGTH        (1) 
#define FAPP_UDP_TIMEOUT_MS                     (8000)

/* Socket Tx&Rx buffer sizes. */
#define FAPP_BENCH_SOCKET_BUF_SIZE      (FAPP_BENCH_PACKET_SIZE_MAX)
#define FAPP_BENCH_BUFFER_SIZE          (FAPP_BENCH_PACKET_SIZE_MAX)


/* Keepalive probe retransmit limit.*/
#define FAPP_BENCH_TCP_KEEPCNT         (2)

/* Keepalive retransmit interval.*/
#define FAPP_BENCH_TCP_KEEPINTVL       (5) /*sec*/

/* Time between keepalive probes.*/
#define FAPP_BENCH_TCP_KEEPIDLE        (5) /*sec*/


#define FAPP_BENCH_COMPLETED_STR    "Test completed."

/************************************************************************
*    Benchmark server control structure.
*************************************************************************/

struct fapp_bench_t
{
    SOCKET socket_listen;                   /* Listening socket.*/
    SOCKET socket_foreign;                  /* Foreign socket.*/
    
    char buffer[FAPP_BENCH_BUFFER_SIZE];    /* Transmit circular buffer */    
 	
 	unsigned long first_time;
 	unsigned long last_time;
 	unsigned long bytes;
}; 

static struct fapp_bench_t fapp_bench;


/**************************************************************************/ /*!
 * TX benchmark parameters
 ******************************************************************************/
struct fapp_bench_tx_params
{
    fnet_shell_desc_t desc;
    fnet_ip_addr_t ip_address;  
    unsigned short port;
    int packet_size;
    int packet_number;
    int iteration_number;
};


/************************************************************************
* NAME: fapp_bench_tcp_rx
*
* DESCRIPTION: Start Benchmark TCP server. 
************************************************************************/
static void fapp_bench_print_results (fnet_shell_desc_t desc)
{
/* Print benchmark results.*/
    unsigned long interval = fnet_timer_get_interval(fapp_bench.first_time, fapp_bench.last_time);
    
    fnet_shell_println(desc, "Results:");
    fnet_shell_println(desc, "\t%d bytes in %d.%d seconds = %d B/sec\n", fapp_bench.bytes, 
        ((interval*FNET_TIMER_PERIOD_MS)/1000),
        ((interval*FNET_TIMER_PERIOD_MS)%1000)/100,
        (interval == 0) ? -1 : (fapp_bench.bytes*(1000/FNET_TIMER_PERIOD_MS))/interval); 
}


/************************************************************************
* NAME: fapp_bench_tcp_rx
*
* DESCRIPTION: Start Benchmark TCP server. 
************************************************************************/
static void fapp_bench_tcp_rx (fnet_shell_desc_t desc)
{
	struct sockaddr_in local_addr;
	int received;
	char ip_str[16];
	fnet_ip_addr_t local_ip;

    struct linger linger_option ={1, /*l_onoff*/
                                        4  /*l_linger*/};
    unsigned long bufsize_option = FAPP_BENCH_SOCKET_BUF_SIZE;
    int keepalive_option = 1;
    int keepcnt_option = FAPP_BENCH_TCP_KEEPCNT;
    int keepintvl_option = FAPP_BENCH_TCP_KEEPINTVL;
    int keepidle_option = FAPP_BENCH_TCP_KEEPIDLE;

	struct sockaddr_in foreign_addr;
    int addr_len;
	int is_first = 1;
	int exit_flag = 0;
	
    
    fapp_bench.socket_foreign = SOCKET_INVALID;
    
	/* Create listen socket */
    if((fapp_bench.socket_listen = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG("BENCH: Socket creation error.");
        goto ERROR_1;
    }
    
    /* Bind socket.*/
    fnet_memset(&local_addr, 0, sizeof(local_addr));
    
    local_addr.sin_port = FAPP_BENCH_PORT;    
    local_addr.sin_family = AF_INET;
    
    if(bind(fapp_bench.socket_listen, (struct sockaddr *)(&local_addr), sizeof(local_addr)) == SOCKET_ERROR)
    {
        FNET_DEBUG("BENCH: Socket bind error.");
        goto ERROR_2;
    }

    /* Set Socket options. */
    if( /* Setup linger option. */
        (setsockopt (fapp_bench.socket_listen, SOL_SOCKET, SO_LINGER, (char *)&linger_option, sizeof(linger_option)) == SOCKET_ERROR) ||
        /* Set socket buffer size. */
        (setsockopt(fapp_bench.socket_listen, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
        (setsockopt(fapp_bench.socket_listen, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
        /* Enable keepalive_option option. */
        (setsockopt (fapp_bench.socket_listen, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive_option, sizeof(keepalive_option)) == SOCKET_ERROR) ||
        /* Keepalive probe retransmit limit. */
        (setsockopt (fapp_bench.socket_listen, IPPROTO_TCP, TCP_KEEPCNT, (char *)&keepcnt_option, sizeof(keepcnt_option)) == SOCKET_ERROR) ||
        /* Keepalive retransmit interval.*/
        (setsockopt (fapp_bench.socket_listen, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&keepintvl_option, sizeof(keepintvl_option)) == SOCKET_ERROR) ||
        /* Time between keepalive probes.*/
        (setsockopt (fapp_bench.socket_listen, IPPROTO_TCP, TCP_KEEPIDLE, (char *)&keepidle_option, sizeof(keepidle_option)) == SOCKET_ERROR)
    )
	{
    	FNET_DEBUG("BENCH: Socket setsockopt error.\n");
        goto ERROR_2;		
	}
    
    /* Listen. */
    if(listen(fapp_bench.socket_listen, 1) == SOCKET_ERROR)
    {
        FNET_DEBUG("BENCH: Socket listen error.\n");
        goto ERROR_2;
    }
    
    /* ------ Start test.----------- */
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    fnet_shell_println(desc, " TCP RX Test" );
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    local_ip = fnet_netif_get_address(fapp_default_netif);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Local IP addr.", inet_ntoa(*(struct in_addr *)( &local_ip), ip_str) );
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Local port", FNET_NTOHS(FAPP_BENCH_PORT));
    fnet_shell_println(desc, FAPP_TOCANCEL_STR);
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    
    while(exit_flag == 0)
    {
        fnet_shell_println(desc, "Waiting.");
        
        fapp_bench.bytes = 0;
        if(fapp_bench.socket_foreign != SOCKET_INVALID)
        {
            closesocket(fapp_bench.socket_foreign);
            fapp_bench.socket_foreign = SOCKET_INVALID;
        }
        
        while((fapp_bench.socket_foreign == SOCKET_INVALID) && (exit_flag == 0))
        {
            /*Accept*/
            addr_len = sizeof(foreign_addr);
            fapp_bench.socket_foreign = accept(fapp_bench.socket_listen, (struct sockaddr *) &foreign_addr, &addr_len);
            
   
            exit_flag = fnet_shell_ctrlc (desc);
                    
            if(fapp_bench.socket_foreign != SOCKET_INVALID)
            {
   
                fnet_shell_println(desc,"Receiving from %s:%d", inet_ntoa(foreign_addr.sin_addr, ip_str), fnet_ntohs(foreign_addr.sin_port));
                          
                fapp_bench.first_time = fnet_timer_ticks();

                while(1) /* Receiving data.*/
                {
                    received = recv(fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), FAPP_BENCH_BUFFER_SIZE, 0);
                    
                    if ((received == SOCKET_ERROR) || exit_flag)
                    {              
                        fapp_bench.last_time = fnet_timer_ticks();
                        
                        /* Print benchmark results.*/
                        fapp_bench_print_results (desc);           			
                        break;
                    }
                    else
                    {
                        fapp_bench.bytes += received;
                    }
                    
                    exit_flag = fnet_shell_ctrlc (desc); /* Check [Ctrl+c]*/
                }
            }
        }
    }

ERROR_3:
    closesocket(fapp_bench.socket_foreign);
    
ERROR_2:
    closesocket(fapp_bench.socket_listen);

ERROR_1:
 
    fnet_shell_println(desc, FAPP_BENCH_COMPLETED_STR);

}


/************************************************************************
* NAME: fapp_bench_udp_rx
*
* DESCRIPTION: Start Benchmark UDP server. 
************************************************************************/
static void fapp_bench_udp_rx (fnet_shell_desc_t desc)
{
	struct sockaddr_in local_addr;
	const unsigned long bufsize_option = FAPP_BENCH_SOCKET_BUF_SIZE;
	fnet_ip_addr_t local_ip;
	int received;
	char ip_str[16];

	struct sockaddr_in addr;
    int addr_len;
	int is_first = 1;
	int exit_flag = 0;
	

	/* Create listen socket */
    if((fapp_bench.socket_listen = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG("BENCH: Socket creation error.\n");
        goto ERROR_1;
    }
    
    /*Bind.*/
    fnet_memset(&local_addr, 0, sizeof(local_addr));
    
    local_addr.sin_port = FAPP_BENCH_PORT;    
    local_addr.sin_family = AF_INET;
    
    if(bind(fapp_bench.socket_listen, (struct sockaddr *)(&local_addr), sizeof(local_addr)) == SOCKET_ERROR)
    {
        FNET_DEBUG("BENCH: Socket bind error.\n");
        goto ERROR_2;
    }


	/* Set socket options. */    
	if( 
		/* Set socket buffer size. */
		(setsockopt(fapp_bench.socket_listen, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
		(setsockopt(fapp_bench.socket_listen, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) 
	)
	{
    	FNET_DEBUG("BENCH: Socket setsockopt error.\n");
        goto ERROR_2;		
	}
 
     /* ------ Start test.----------- */
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    fnet_shell_println(desc, " UDP RX Test" );
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    local_ip = fnet_netif_get_address(fapp_default_netif);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Local IP addr.", inet_ntoa(*(struct in_addr *)( &local_ip), ip_str) );
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Local port", FNET_NTOHS(FAPP_BENCH_PORT));
    fnet_shell_println(desc, FAPP_TOCANCEL_STR);
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
  
    
    while(exit_flag == 0) /* Main loop */
    {
        fnet_shell_println(desc, "Waiting.");
        
        fapp_bench.bytes = 0;
        addr_len = sizeof(struct sockaddr_in);
        is_first = 1;
        
        while(exit_flag == 0) /* Test loop. */
        {
    		/* Receive data */
            received = recvfrom  (fapp_bench.socket_listen, (char*)(&fapp_bench.buffer[0]), FAPP_BENCH_BUFFER_SIZE, 0,
                (struct sockaddr*)&addr, &addr_len );

            if(received >= FAPP_BENCH_UDP_END_BUFFER_LENGTH)
            {   
      
                /* Reset timeout. */
                fapp_bench.last_time = fnet_timer_ticks();
                
                if(is_first)
                {
                    if( received > FAPP_BENCH_UDP_END_BUFFER_LENGTH )
                    {
                        fnet_shell_println(desc,"Receiving from %s:%d", inet_ntoa(addr.sin_addr, ip_str), fnet_ntohs(addr.sin_port));
                        fapp_bench.first_time = fnet_timer_ticks();
                        is_first = 0;
                    }
                }
                else
                {
                    if(received == FAPP_BENCH_UDP_END_BUFFER_LENGTH ) /* End of transfer. */
                    {
                        /* Print benchmark results.*/
                        fapp_bench_print_results (desc);           		
    					
                        break;    	
                    }
                    else
                        fapp_bench.bytes += received; 
                }
            }
            else
            {
                /* Check error. Check timeout */
                if(received == SOCKET_ERROR)
                {
                    fnet_shell_println(desc, "BENCH: Error (%d).", fnet_error_get());                   
                    break;
                }
                /* Check timeout. */
                if((is_first == 0) &&
                    (fnet_timer_get_interval(fapp_bench.last_time, fnet_timer_ticks()) > (FAPP_UDP_TIMEOUT_MS/FNET_TIMER_PERIOD_MS)))
                {
                    fnet_shell_println(desc, "BENCH: Exit on timeout.");
                    fapp_bench_print_results (desc);            
                    break;
                }
            }
            
            exit_flag = fnet_shell_ctrlc (desc);
        }
    }

ERROR_2:
    closesocket(fapp_bench.socket_listen);

ERROR_1:
 
    fnet_shell_println(desc, FAPP_BENCH_COMPLETED_STR);
}

/************************************************************************
* NAME: fapp_benchrx_cmd
*
* DESCRIPTION: Start RX Benchmark server. 
************************************************************************/

void fapp_benchrx_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    /* TCP */
    if((argc == 1)||(argc == 2 && fnet_strcasecmp("tcp", argv[1]) == 0)) 
    {
        fapp_bench_tcp_rx(desc);
    }
    /* UDP */
    else if(argc == 2 && fnet_strcasecmp("udp", argv[1]) == 0) 
    {
        fapp_bench_udp_rx(desc);
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}


////////////////////////////////////////////// TX //////////////////////////////////////
/************************************************************************
* NAME: fapp_bench_tcp_tx
*
* DESCRIPTION: Start TX TCP Benchmark. 
************************************************************************/
static void fapp_bench_tcp_tx (struct fapp_bench_tx_params *params)
{
	int send_result;
	char ip_str[16];

    const struct linger linger_option ={1, /*l_onoff*/
                                        4  /*l_linger*/};
    const unsigned long bufsize_option = FAPP_BENCH_SOCKET_BUF_SIZE;
    const int keepalive_option = 1;
    const int keepcnt_option = FAPP_BENCH_TCP_KEEPCNT;
    const int keepintvl_option = FAPP_BENCH_TCP_KEEPINTVL;
    const int keepidle_option = FAPP_BENCH_TCP_KEEPIDLE;

	struct sockaddr_in foreign_addr;
	int is_first = 1;
	int exit_flag = 0;

	int sock_err ;
	int option_len;
	fnet_shell_desc_t desc = params->desc;
	fnet_socket_state_t connection_state;
	int packet_size = params->packet_size;
	int cur_packet_number;
	int buffer_offset;
	int iterations = params->iteration_number;
	
	if(packet_size > FAPP_BENCH_BUFFER_SIZE) /* Check max size.*/
	     packet_size = FAPP_BENCH_BUFFER_SIZE;
	
    
    fapp_bench.socket_listen = SOCKET_INVALID;
    
    /* ------ Start test.----------- */
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    fnet_shell_println(desc, " TCP TX Test" );
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Remote IP addr.", inet_ntoa(*(struct in_addr *)( &params->ip_address), ip_str));
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Remote port", fnet_ntohs(params->port));
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Message size", params->packet_size);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Number of messages", params->packet_number);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Number of iterations", params->iteration_number);
    fnet_shell_println(desc, FAPP_TOCANCEL_STR);
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    
    
    while(iterations--)
    {
        /* Create socket */
        if((fapp_bench.socket_foreign = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_INVALID)
        {
            FNET_DEBUG("BENCH: Socket creation error.\n");
            iterations = 0;
            goto ERROR_1;
        }
        
        /* Set Socket options. */
        if( /* Setup linger option. */
            (setsockopt (fapp_bench.socket_foreign, SOL_SOCKET, SO_LINGER, (char *)&linger_option, sizeof(linger_option)) == SOCKET_ERROR) ||
            /* Set socket buffer size. */
            (setsockopt(fapp_bench.socket_foreign, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
            (setsockopt(fapp_bench.socket_foreign, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
            /* Enable keepalive_option option. */
            (setsockopt (fapp_bench.socket_foreign, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive_option, sizeof(keepalive_option)) == SOCKET_ERROR) ||
            /* Keepalive probe retransmit limit. */
            (setsockopt (fapp_bench.socket_foreign, IPPROTO_TCP, TCP_KEEPCNT, (char *)&keepcnt_option, sizeof(keepcnt_option)) == SOCKET_ERROR) ||
            /* Keepalive retransmit interval.*/
            (setsockopt (fapp_bench.socket_foreign, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&keepintvl_option, sizeof(keepintvl_option)) == SOCKET_ERROR) ||
            /* Time between keepalive probes.*/
            (setsockopt (fapp_bench.socket_foreign, IPPROTO_TCP, TCP_KEEPIDLE, (char *)&keepidle_option, sizeof(keepidle_option)) == SOCKET_ERROR)
        )
    	{
        	FNET_DEBUG("BENCH: Socket setsockopt error.\n");
        	iterations = 0;
            goto ERROR_2;		
    	}
    
        /* Connect to the server.*/
        fnet_shell_println(desc,"Connecting.");
        fnet_memset(&foreign_addr, 0, sizeof(foreign_addr));
        foreign_addr.sin_addr.s_addr = params->ip_address;
        foreign_addr.sin_port = params->port;    
        foreign_addr.sin_family = AF_INET;
        
        connect(fapp_bench.socket_foreign, (struct sockaddr *)(&foreign_addr), sizeof(foreign_addr)); 
        
        do
        {
            option_len = sizeof(connection_state); 
            getsockopt(fapp_bench.socket_foreign, SOL_SOCKET, SO_STATE, (char*)&connection_state, &option_len);
        }
        while (connection_state == SS_CONNECTING);
        
        if(connection_state != SS_CONNECTED)
        {
           fnet_shell_println(desc, "Connection failed.");
           iterations = 0;
           goto ERROR_2;
        } 
        
        /* Sending.*/
        fnet_shell_println(desc,"Sending."); 
        fapp_bench.bytes = 0;
        cur_packet_number = 0;
        buffer_offset = 0;
        
        fapp_bench.first_time = fnet_timer_ticks();
        
        while(1)
        {
            send_result = send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[buffer_offset]), (packet_size-buffer_offset), 0);
            fapp_bench.last_time = fnet_timer_ticks();
             
            if ( send_result == SOCKET_ERROR )
            {              
                option_len = sizeof(sock_err); 
                getsockopt(fapp_bench.socket_foreign, SOL_SOCKET, SO_ERROR, (char*)&sock_err, &option_len);
                fnet_shell_println(desc, "Socket error = %d", sock_err);
                
                iterations = 0;
                #if 0
                    /* Print benchmark results.*/
                    fapp_bench_print_results (desc);           			
                #endif
                break;
            }
            else if(send_result)
            {
                fapp_bench.bytes += send_result;
                buffer_offset += send_result;
                
                if(buffer_offset == packet_size)
                {
                    cur_packet_number ++;
                    buffer_offset = 0;
                }
                
                exit_flag = fnet_shell_ctrlc(desc); /* Check [Ctrl+c]*/
                
                if((cur_packet_number >= params->packet_number)|| exit_flag)
                { 
                    if(exit_flag)
                    {
                        fnet_shell_println(desc, "Canceled by [Ctrl+C]!");
                        iterations = 0;
                    }
                    /* Print benchmark results.*/
                    fapp_bench_print_results (desc);
                    break;
                }
            }
        }

    ERROR_2:
        closesocket(fapp_bench.socket_foreign);
    }
ERROR_1:
    fnet_shell_println(desc, FAPP_BENCH_COMPLETED_STR);

}

/************************************************************************
* NAME: fapp_bench_udp_tx
*
* DESCRIPTION: Start TX TCP Benchmark. 
************************************************************************/
static void fapp_bench_udp_tx (struct fapp_bench_tx_params *params)
{
	struct sockaddr_in local_addr;
	int send_result;
	char ip_str[16];

    const struct linger linger_option ={1, /*l_onoff*/
                                        4  /*l_linger*/};
    const unsigned long bufsize_option = FAPP_BENCH_SOCKET_BUF_SIZE;

	struct sockaddr_in foreign_addr;
	int is_first = 1;
	int exit_flag = 0;

	int sock_err ;
	int option_len;
	fnet_shell_desc_t desc = params->desc;
	int packet_size = params->packet_size;
	int cur_packet_number;
	int iterations = params->iteration_number;

	
	if(packet_size > FAPP_BENCH_BUFFER_SIZE) /* Check max size.*/
	     packet_size = FAPP_BENCH_BUFFER_SIZE;
	
    
    fapp_bench.socket_listen = SOCKET_INVALID;
    

    
    /* ------ Start test.----------- */
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    fnet_shell_println(desc, " UDP TX Test" );
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Remote IP addr.", inet_ntoa(*(struct in_addr *)( &params->ip_address), ip_str));
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Remote port", fnet_ntohs(params->port));
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Message size", params->packet_size);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Number of messages", params->packet_number);
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Number of iterations", params->iteration_number);
    fnet_shell_println(desc, FAPP_TOCANCEL_STR);
    fnet_shell_println(desc, FAPP_DELIMITER_STR);
    
    while(iterations--)
    {
        /* Create socket */
        if((fapp_bench.socket_foreign = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_INVALID)
        {
            FNET_DEBUG("BENCH: Socket creation error.\n");
            iterations = 0;
            goto ERROR_1;
        }
        
        
        /* Set Socket options. */
        if( /* Setup linger option. */
            (setsockopt (fapp_bench.socket_foreign, SOL_SOCKET, SO_LINGER, (char *)&linger_option, sizeof(linger_option)) == SOCKET_ERROR) ||
            /* Set socket buffer size. */
            (setsockopt(fapp_bench.socket_foreign, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
            (setsockopt(fapp_bench.socket_foreign, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) 
          )
    	{
        	FNET_DEBUG("BENCH: Socket setsockopt error.\n");
        	iterations = 0;
            goto ERROR_2;		
    	}
        
        
        /* Bind to the server.*/
        fnet_shell_println(desc,"Connecting.");
        fnet_memset(&foreign_addr, 0, sizeof(local_addr));
        foreign_addr.sin_addr.s_addr = params->ip_address;
        foreign_addr.sin_port = params->port;    
        foreign_addr.sin_family = AF_INET;
        
        if(connect(fapp_bench.socket_foreign, (struct sockaddr *)(&foreign_addr), sizeof(foreign_addr))== FNET_ERR) 
        {
           fnet_shell_println(desc, "Conection failed.");
           iterations = 0;
           goto ERROR_2;
        } 
    
        /* Sending.*/
        fnet_shell_println(desc,"Sending."); 
        fapp_bench.bytes = 0;
        cur_packet_number = 0;
        //buffer_offset = 0;
        
        fapp_bench.first_time = fnet_timer_ticks();
        
        while(1)
        {
            send_result = send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), packet_size, 0);
            fapp_bench.last_time = fnet_timer_ticks();
            
            
            if ( send_result == SOCKET_ERROR )
            {              
                option_len = sizeof(sock_err); 
                getsockopt(fapp_bench.socket_foreign, SOL_SOCKET, SO_ERROR, (char*)&sock_err, &option_len);
                fnet_shell_println(desc, "socket_error = %d", sock_err);
               
                iterations = 0;
                
                #if 0
                    /* Print benchmark results.*/
                    fapp_bench_print_results (desc);           		
                #endif
                break;
            }
            else
            {
                fapp_bench.bytes += send_result;
                cur_packet_number ++;
                
                exit_flag = fnet_shell_ctrlc (desc); /* Check [Ctrl+c]*/
                
                if((cur_packet_number >= params->packet_number)|| exit_flag)
                { 
                    if(exit_flag)
                    {
                        fnet_shell_println(desc, "Canceled by [Ctrl+C]!");
                        iterations = 0;
                    }
                  
                    /* Print benchmark results.*/
                    fapp_bench_print_results (desc);
                    break;
                }
            }
        }

        /* Send End mark.*/
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        fnet_timer_delay(10);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);
        send( fapp_bench.socket_foreign, (char*)(&fapp_bench.buffer[0]), 1, 0);

    ERROR_2:
        closesocket(fapp_bench.socket_foreign);
    }

ERROR_1:
    fnet_shell_println(desc, FAPP_BENCH_COMPLETED_STR);

}


/************************************************************************
* NAME: fapp_benchtx_cmd
*
* DESCRIPTION: Start TX Benchmark. 
************************************************************************/
void fapp_benchtx_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fapp_bench_tx_params bench_params;
    
    if( inet_aton(argv[1], (struct in_addr *) &bench_params.ip_address) == FNET_OK) /* TFTP server IP*/
    {
        bench_params.desc = desc;
        bench_params.port = FAPP_BENCH_PORT; //TBD not needed.
        bench_params.packet_size = FAPP_BENCH_TX_PACKET_SIZE_DEFAULT;
        bench_params.packet_number = FAPP_BENCH_TX_PACKET_NUMBER_DEFAULT;
        bench_params.iteration_number = FAPP_BENCH_TX_ITERATION_NUMBER_DEFAULT;
        
        if(argc > 3)
        {
            char *p = 0;
            
            /* Packet size.*/
            bench_params.packet_size = fnet_strtoul(argv[3], &p, 0);
            if ((bench_params.packet_size == 0) || (bench_params.packet_size > FAPP_BENCH_PACKET_SIZE_MAX))
            {
                fnet_shell_println(desc, FAPP_PARAM_ERR, argv[3]); /* Print error mesage. */
                return;
            }
            
            /* Number of packets.*/
            if(argc > 4)
            {
                bench_params.packet_number = fnet_strtoul(argv[4], &p, 0);
                if (bench_params.packet_number == 0)
                {
                    fnet_shell_println(desc, FAPP_PARAM_ERR, argv[4]); /* Print error mesage. */
                    return;
                }
                
                /* Number of iterations.*/
                if(argc > 5)
                {
                    bench_params.iteration_number = fnet_strtoul(argv[5], &p, 0);
                    if ((bench_params.iteration_number < 1) || (bench_params.iteration_number > FAPP_BENCH_TX_ITERATION_NUMBER_MAX) )
                    {
                        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[5]); /* Print error mesage. */
                        return;
                    }
                }
            }
        } 
        
        /* TCP */
        if((argc == 2)||(argc >= 3 && fnet_strcasecmp("tcp", argv[2]) == 0)) 
        {
            fapp_bench_tcp_tx (&bench_params);
        }
        /* UDP */
        else if(argc >= 3 && fnet_strcasecmp("udp", argv[2]) == 0) 
        {
            fapp_bench_udp_tx (&bench_params);
        }
        else
        {
            fnet_shell_println(desc, FAPP_PARAM_ERR, argv[2]);
        }
                
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);   /* Wrong Benchmark Server IP address. */
        return;
    }
}


#endif














