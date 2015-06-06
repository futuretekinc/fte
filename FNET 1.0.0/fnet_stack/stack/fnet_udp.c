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
* @file fnet_udp.c
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.14.0
*
* @brief UDP protocol implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_udp.h"
#include "fnet_ip_prv.h"
#include "fnet_timer.h"
#include "fnet_stdlib.h"

#include "fnet_isr.h"
#include "fnet_checksum.h"
#include "fnet_prot.h"

#if FNET_CFG_UDP

/************************************************************************
*     Function Prototypes
*************************************************************************/

int fnet_udp_attach( fnet_socket_t *sk );

int fnet_udp_detach( fnet_socket_t *sk );
int fnet_udp_connect( fnet_socket_t *sk, fnet_ip_addr_t addr, unsigned short port );
int fnet_udp_snd( fnet_socket_t *sk, char *buf, int len, int flags, fnet_ip_addr_t addr,
                  unsigned short port );
int fnet_udp_rcv( fnet_socket_t *sk, char *buf, int len, int flags, fnet_ip_addr_t *addr,
                  unsigned short *port );
void fnet_udp_control_input( fnet_prot_notify_t command, fnet_ip_header_t *ip_hdr );
int fnet_udp_shutdown( fnet_socket_t *sk, int how );

#if FNET_CFG_DEBUG_TRACE_UDP
void fnet_udp_trace(char *str, fnet_udp_header_t *udp_hdr);
#else
#define fnet_udp_trace(str, udp_hdr)
#endif

/************************************************************************
*     Global Data Structures
*************************************************************************/
fnet_prot_if_t fnet_udp_prot_if =
{
  0,                      /* Poinnter to the next protocol interface structure.*/
  AF_INET,                /* Address domain family.*/
  SOCK_DGRAM,             /* Socket type used for.*/
  FNET_IP_PROTOCOL_UDP,   /* Protocol number.*/
  0,                      /* Flag that protocol is connection oriented.*/
  0,                      /* Pointer to the head of the protocol's socket list.*/
  0,                      /* Protocol initialization function.*/
  fnet_udp_release,       /* Protocol release function.*/
  fnet_udp_input,         /* Protocol input function.*/
  fnet_udp_control_input, /* Protocol input control function.*/
  fnet_udp_attach,        /* Protocol "attach" function.*/
  fnet_udp_detach,        /* Protocol "detach" function.*/
  fnet_udp_connect,       /* Protocol "connect" function.*/
  0,                      /* Protocol "accept" function.*/
  fnet_udp_rcv,           /* Protocol "receive" function.*/
  fnet_udp_snd,           /* Protocol "send" function.*/
  fnet_udp_shutdown,      /* Protocol "shutdown" function.*/
  fnet_ip_setsockopt,     /* Protocol "setsockopt" function.*/
  fnet_ip_getsockopt,     /* Protocol "getsockopt" function.*/
  0,                      /* Protocol "listen" function.*/
  0                       /* protocol drain function.*/
};

/************************************************************************
* NAME: fnet_udp_release
*
* DESCRIPTION: This function releases all sockets associated 
*              with UDP protocol. 
*************************************************************************/
static void fnet_udp_release( void )
{
    while(fnet_udp_prot_if.head)
      fnet_socket_release(&fnet_udp_prot_if.head, fnet_udp_prot_if.head);
}

/************************************************************************
* NAME: fnet_udp_output
*
* DESCRIPTION: UDP output function
*************************************************************************/
static int fnet_udp_output( fnet_netif_t *netif,      fnet_ip_addr_t src_ip,
                            unsigned short src_port,  fnet_ip_addr_t dest_ip,
                            unsigned short dest_port, fnet_socket_option_t *sockoption,
                            fnet_netbuf_t *nb )
{
    fnet_netbuf_t *nb_header;
    fnet_udp_header_t *udp_header;
    int error;

    if(netif == 0)
        if((netif = fnet_ip_route(dest_ip)) == 0)
        {
            fnet_netbuf_free_chain(nb); /* No route.*/
            return (FNET_ERR_NETUNREACH);
        }

    /* If source address not specified, use address of outgoing interface.*/
    if(src_ip == INADDR_ANY)
    {
        src_ip = netif->addr.address;
    };

    /* Construct UDP header.*/
    if((nb_header = fnet_netbuf_new(sizeof(fnet_udp_header_t), FNET_NETBUF_TYPE_HEADER,
                                        FNET_NETBUF_MALLOC_NOWAITDRAIN)) == 0)
    {
        fnet_netbuf_free_chain(nb); /* No route.*/
        return (FNET_ERR_NOMEM);
    }

    udp_header = nb_header->data_ptr;

    udp_header->source_port = src_port;             /* Source port number.*/
    udp_header->destination_port = dest_port;       /* Destination port number.*/
    nb = fnet_netbuf_concat(nb_header, nb);
    udp_header->length = fnet_htons((unsigned short)nb->total_length);  /* Length.*/

    udp_header->checksum = 0;                              /* Checksum.*/

#if FNET_CFG_UDP_CHECKSUM
    udp_header->checksum = fnet_checksum_pseudo(nb, src_ip, dest_ip, FNET_IP_PROTOCOL_UDP,
                                               fnet_ntohs(udp_header->length)); //TBD PFI
    if(udp_header->checksum == 0x0000)
        udp_header->checksum = 0xFFFF;
#endif

    error = fnet_ip_output(netif, src_ip, dest_ip, FNET_IP_PROTOCOL_UDP, sockoption->ip_opt.tos,
                               sockoption->ip_opt.ttl, nb, FNET_UDP_DF,
                               sockoption->ip_opt.options, ((sockoption->flags & SO_DONTROUTE) > 0));

    return (error);
}


/************************************************************************
* NAME: fnet_udp_input
*
* DESCRIPTION: UDP input function.
*************************************************************************/
static int fnet_udp_input( fnet_netif_t *netif, fnet_ip_addr_t src_ip, fnet_ip_addr_t dest_ip,
                           fnet_netbuf_t *nb,   fnet_netbuf_t *options )
{
    fnet_udp_header_t *udp_header = nb->data_ptr;
    fnet_socket_t *sock;
    fnet_socket_t *last;
    unsigned short src_port;
    unsigned short dest_port;
    fnet_netbuf_t *nb_tmp;

    if((netif != 0) && (nb != 0) && (nb->total_length >= sizeof(fnet_udp_header_t)))
    {
        
        if((nb_tmp = fnet_netbuf_pullup(nb, sizeof(fnet_udp_header_t), FNET_NETBUF_MALLOC_NOWAIT))
               == 0) /* The header must reside in contiguous area of memory.*/
        {
            goto BAD;
        }
        
        nb = nb_tmp;

        if(nb->total_length >= fnet_ntohs(udp_header->length)) /* Check the amount of data.*/
        {
            if(nb->total_length > fnet_ntohs(udp_header->length))   //TBD PFI
            {
                /* Logical size and the physical size of the packet should be the same.*/
                fnet_netbuf_trim(&nb, (int)(fnet_ntohs(udp_header->length)         //TBD PFI
                                                - nb->total_length)); 
            }

#if FNET_CFG_UDP_CHECKSUM

            if(udp_header->checksum != 0)
            {
                if(fnet_checksum_pseudo(nb, src_ip, dest_ip, FNET_IP_PROTOCOL_UDP, fnet_ntohs(udp_header->length))) //TBD PFI
                    goto BAD;
            }

#endif
            fnet_udp_trace("RX", udp_header); /* Trace UDP header.*/
            
            
            src_port = udp_header->source_port;
            dest_port = udp_header->destination_port;

            fnet_netbuf_trim(&nb, sizeof(fnet_udp_header_t));
          

            /* Demultiplexing.*/
            if(fnet_ip_addr_is_broadcast(dest_ip, &(netif->addr))
                   || FNET_IP_MULTICAST(dest_ip)) /* Demultiplex broadcast & multicast datagrams.*/
            {
                last = 0;
                fnet_netbuf_free_chain(options); /* Discard IP options.*/
                options = 0;

                for (sock = fnet_udp_prot_if.head; sock != 0; sock = sock->next)
                {
                    /* Compare local port number.*/
                    if(sock->local_port != dest_port)
                        continue; /* => ignore.*/

                    /* Compare local address.*/
                    if(sock->local_addr != INADDR_ANY)
                    {
                        if(sock->local_addr != dest_ip)
                            continue;
                    }

                    /* Compare foreign address and port number.*/
                    if(sock->foreign_addr != INADDR_ANY)
                    {
                        if(sock->foreign_addr != src_ip || sock->foreign_port != src_port)
                            continue;
                    }

                    if((last != 0) && (last->receive_buffer.is_shutdown == 0))
                    {

                        if((nb_tmp = fnet_netbuf_copy(nb, 0, FNET_NETBUF_COPYALL, FNET_NETBUF_MALLOC_NOWAIT))
                               != 0)
                        {

                            if(fnet_socket_buffer_append_address(&(last->receive_buffer), nb_tmp, src_ip,
                                                                     src_port, 0) == FNET_ERR)
                            {
                                fnet_netbuf_free_chain(nb_tmp);
                            }
                           
                        }
                     }

                    last = sock;
                }

                if(last == 0)
                    goto BAD;

                if(last->receive_buffer.is_shutdown) /* Is shutdown.*/
                    goto BAD;

                if(fnet_socket_buffer_append_address(&(last->receive_buffer), nb, src_ip, src_port,
                                                         options) == FNET_ERR)
                    goto BAD;
            }
            else /* For unicast datagram.*/
            {
                sock = fnet_socket_lookup(fnet_udp_prot_if.head, dest_ip, dest_port, src_ip, src_port);

                if(sock)
                {
                    if(sock->receive_buffer.is_shutdown) /* Is shutdown.*/
                        goto BAD;

                    if(fnet_socket_buffer_append_address(&(sock->receive_buffer), nb, src_ip, src_port,
                                                             options) == FNET_ERR)
                        goto BAD;
                }
                else
                {
                    fnet_netbuf_free_chain(options);
                    fnet_netbuf_free_chain(nb); /* No match was found, send ICMP destination port unreachable.*/
                    return FNET_ERR;
                }
            }
        }
        else
            goto BAD;
    }
    else
    {
        BAD:
        fnet_netbuf_free_chain(options);

        fnet_netbuf_free_chain(nb);
    }

    return FNET_OK;
}

/************************************************************************
* NAME: fnet_udp_attach
*
* DESCRIPTION: UDP attach function. 
*************************************************************************/
static int fnet_udp_attach( fnet_socket_t *sk )
{
    sk->options.ip_opt.ttl = FNET_UDP_TTL;
    sk->options.ip_opt.tos = 0;
    sk->send_buffer.count_max = FNET_UDP_TX_BUF_MAX;
    sk->receive_buffer.count_max = FNET_UDP_RX_BUF_MAX;
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_detach
*
* DESCRIPTION: UDP close function.
*************************************************************************/
static int fnet_udp_detach( fnet_socket_t *sk )
{
    fnet_isr_lock();
    fnet_socket_release(&fnet_udp_prot_if.head, sk);
    fnet_isr_unlock();
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_shutdown
*
* DESCRIPTION:  UDP shutdown function.
*************************************************************************/
static int fnet_udp_shutdown( fnet_socket_t *sk, int how )
{
    fnet_isr_lock();

    if(how & SD_READ)
    {
        sk->receive_buffer.is_shutdown = 1;
        fnet_socket_buffer_release(&sk->receive_buffer);
    }

    if(how & SD_WRITE)
    {
        sk->send_buffer.is_shutdown = 1;
    }

    fnet_isr_unlock();

    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_connect
*
* DESCRIPTION: UDP connect function.
*************************************************************************/
static int fnet_udp_connect( fnet_socket_t *sk, fnet_ip_addr_t addr, unsigned short port )
{
    fnet_isr_lock();
    sk->foreign_addr = addr; /* Foreign IP address.*/
    sk->foreign_port = port; /* Foreign port.*/
    sk->state = SS_CONNECTED;
    fnet_socket_buffer_release(&sk->receive_buffer);
    fnet_isr_unlock();
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_snd
*
* DESCRIPTION: UDP send function.
*************************************************************************/
static int fnet_udp_snd( fnet_socket_t *sk, char *buf, int len, int flags, fnet_ip_addr_t addr,
                         unsigned short port )
{
    fnet_netbuf_t *nb;
    fnet_netif_t *netif;
    int error = FNET_OK;
    fnet_ip_addr_t foreign_addr;    /* Foreign IP address.*/
    unsigned short foreign_port;    /* Foreign port.*/
    int flags_save;

    if(sk->send_buffer.is_shutdown)
    {
        error = FNET_ERR_SHUTDOWN; /* The socket has been shut down.*/
        goto ERROR;
    }

    if(flags & MSG_OOB)
    {
        error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
        goto ERROR;
    }

    if(len > sk->send_buffer.count_max)
    {
        error = FNET_ERR_MSGSIZE;   /* Message too long. */
        goto ERROR;
    }

    if(addr)
    {
        foreign_addr = addr;
        foreign_port = port;
    }
    else
    {
        foreign_addr = sk->foreign_addr;
        foreign_port = sk->foreign_port;
    }

    if((netif = fnet_ip_route(foreign_addr)) == 0)
    {
        error = FNET_ERR_NETUNREACH; /* No route */
        goto ERROR;
    }

    if((nb = fnet_netbuf_from_buf(buf, len, FNET_NETBUF_TYPE_DATA, FNET_NETBUF_MALLOC_NOWAIT)) == 0)
    {
        error = FNET_ERR_NOMEM;     /* Cannot allocate memory.*/
        goto ERROR;
    }

    if(sk->local_port == 0)
    {
        sk->local_port = fnet_socket_uniqueport(sk->protocol_interface->head,
                                                sk->local_addr); /* Get ephemeral port.*/
    }

    if(flags & MSG_DONTROUTE)
    {
        flags_save = sk->options.flags;
        sk->options.flags |= SO_DONTROUTE;
    }

    error = fnet_udp_output(netif, sk->local_addr, sk->local_port, foreign_addr, foreign_port, &(sk->options),
                            nb);

    if(flags & MSG_DONTROUTE)
    {
        sk->options.flags = flags_save;
    }

    if((error == FNET_OK) && (sk->options.local_error == FNET_OK)) /* We get UDP or ICMP error.*/
    {
        return (len);
    }

    ERROR:
    fnet_socket_set_error(sk, error);

    fnet_error_set(error);
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: fnet_udp_rcv
*
* DESCRIPTION :UDP receive function.
*************************************************************************/
static int fnet_udp_rcv( fnet_socket_t *sk, char *buf, int len, int flags, fnet_ip_addr_t *addr,
                         unsigned short *port )
{
    int error = FNET_OK;
    int length;
#if FNET_CFG_SOCKET_BLOCKING_SUPPORT    
    unsigned long time;
#endif    
    fnet_ip_addr_t foreign_addr = 0;
    unsigned short foreign_port = 0;

    if(sk->receive_buffer.is_shutdown)
    {
        error = FNET_ERR_SHUTDOWN;  /* The socket has been shut down.*/
        goto ERROR;
    }

    if(flags & MSG_OOB)
    {
        error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
        goto ERROR;
    }
#if FNET_CFG_SOCKET_BLOCKING_SUPPORT
    time = fnet_timer_ticks();

    do
    {
#endif    
        if((length = fnet_socket_buffer_read_address(&(sk->receive_buffer), buf,
                     len, &foreign_addr, &foreign_port, ((flags &MSG_PEEK)== 0))) == FNET_ERR)
        {
            /* The message was too large to fit into the specified buffer and was truncated.*/
            error = FNET_ERR_MSGSIZE;
            goto ERROR;
        }
#if FNET_CFG_SOCKET_BLOCKING_SUPPORT 
    } 
    while ((foreign_port == 0) && ((flags &MSG_DONTWAIT) == 0)
                 && ((sk->options.flags &SO_BLOCKING) || (flags &MSG_WAITALL))
                 && (fnet_timer_get_interval(time, fnet_timer_ticks()) < sk->receive_buffer.timeout));
#endif

    if(addr)
        *addr=foreign_addr;

    if(port)
        *port=foreign_port;

    if((error == FNET_OK) && (sk->options.local_error == FNET_OK)) /* We get UDP or ICMP error.*/
    {
        return (length);
    }

    ERROR:
    fnet_socket_set_error(sk, error);

    fnet_error_set(error);
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: fnet_udp_control_input
*
* DESCRIPTION: This function processes the ICMP error.
*************************************************************************/
static void fnet_udp_control_input( fnet_prot_notify_t command, fnet_ip_header_t *ip_header )
{
    fnet_udp_header_t *udp_header;

    fnet_ip_addr_t foreign_addr; /* Foreign IP address.*/
    unsigned short foreign_port; /* Foreign port.*/
    fnet_ip_addr_t local_addr;   /* Local IP address.*/
    unsigned short local_port;   /* Local port.*/
    int error;
    fnet_socket_t *sock;

    if(ip_header)
    {
        udp_header = (fnet_udp_header_t *)((char *)ip_header + (FNET_IP_HEADER_GET_HEADER_LENGTH(ip_header) << 2));
        foreign_addr = ip_header->desination_addr;
        foreign_port = udp_header->destination_port;
        local_addr = ip_header->source_addr;
        local_port = udp_header->source_port;

        if(foreign_addr == INADDR_ANY)
            return;

        switch(command)
        {
            case FNET_PROT_NOTIFY_MSGSIZE: /* Message size forced drop.*/
              error = FNET_ERR_MSGSIZE;
              break;

            case FNET_PROT_NOTIFY_UNREACH_HOST:    /* No route to host.*/
            case FNET_PROT_NOTIFY_UNREACH_NET:     /* No route to network.*/
            case FNET_PROT_NOTIFY_UNREACH_SRCFAIL: /* Source route failed.*/
              error = FNET_ERR_HOSTUNREACH;
              break;

            case FNET_PROT_NOTIFY_UNREACH_PROTOCOL: /* Dst says bad protocol.*/
            case FNET_PROT_NOTIFY_UNREACH_PORT:     /* Bad port #.*/
              error = FNET_ERR_CONNRESET;
              break;

            case FNET_PROT_NOTIFY_PARAMPROB:        /* Header incorrect.*/
              error = FNET_ERR_NOPROTOOPT;          /* Bad protocol option.*/
              break;

            default:
              return;
        }

        for (sock = fnet_udp_prot_if.head; sock != 0; sock = sock->next)
        {
            if((sock->foreign_addr != foreign_addr) || (sock->foreign_port != foreign_port)
                   || (sock->local_port != local_port) || (sock->local_addr != local_addr))
                continue;

            sock->options.local_error = error;
        }
    }
}

/************************************************************************
* NAME: fnet_udp_trace
*
* DESCRIPTION: Prints UDP header. For debugging purposes.
*************************************************************************/
#if FNET_CFG_DEBUG_TRACE_UDP
void fnet_udp_trace(char *str, fnet_udp_header_t *udp_hdr)
{

    fnet_printf(FNET_SERIAL_ESC_FG_GREEN"%s", str); /* Print app-specific header.*/
    fnet_println("[UDP header]"FNET_SERIAL_ESC_FG_BLACK);
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(SrcPort)                  "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |(DestPort)                 "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |",
                    fnet_ntohs(udp_hdr->source_port),
                    fnet_ntohs(udp_hdr->destination_port));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Lenghth)                  "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |(Checksum)              0x%04X |",
                    fnet_ntohs(udp_hdr->length),
                    fnet_ntohs(udp_hdr->checksum));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");    
}
#endif /* FNET_CFG_DEBUG_TRACE_UDP */

#endif 
