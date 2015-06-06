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
* @file fnet_socket.c
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.22.0
*
* @brief Socket interface implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_timer_prv.h"
#include "fnet_isr.h"
#include "fnet_stdlib.h"
#include "fnet_prot.h"
#include "fnet_debug.h"
#include "fnet.h"


/************************************************************************
*     Global Data Structures
*************************************************************************/

int fnet_enabled = 0;   /* Flag that the stack is initialized. */

/* Array of sockets descriptors. */
static fnet_socket_t *fnet_socket_desc[FNET_CFG_SOCKET_MAX];

/************************************************************************
*     Function Prototypes
*************************************************************************/
SOCKET fnet_socket_desc_alloc( void );
void fnet_socket_desc_set( SOCKET desc, fnet_socket_t *sock );
void fnet_socket_desc_free( SOCKET desc );
fnet_socket_t *fnet_socket_desc_find( SOCKET desc );

/************************************************************************
* NAME: fnet_socket_init
*
* DESCRIPTION: Initialization of the socket layer.
*************************************************************************/
void fnet_socket_init( void )
{
    fnet_memset(fnet_socket_desc, 0, sizeof(fnet_socket_desc));
}

/************************************************************************
* NAME: fnet_socket_set_error
*
* DESCRIPTION: This function sets socket error. 
*************************************************************************/
void fnet_socket_set_error( fnet_socket_t *sock, int error )
{
    if(sock->options.local_error != FNET_OK)
    {
        error = sock->options.local_error;
        sock->options.local_error = FNET_OK;
    }

    sock->options.error = error;
}

/************************************************************************
* NAME: fnet_socket_list_add
*
* DESCRIPTION: This function adds socket into the queue.
*************************************************************************/
void fnet_socket_list_add( fnet_socket_t ** head, fnet_socket_t *s )
{
    fnet_isr_lock();
    s->next = *head;

    if(s->next != 0)
        s->next->prev = s;

    s->prev = 0;
    *head = s;
    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_socket_list_del
*
* DESCRIPTION: This function removes socket from the queue 
*************************************************************************/
void fnet_socket_list_del( fnet_socket_t ** head, fnet_socket_t *s )
{
    fnet_isr_lock();

    if(s->prev == 0)
      *head=s->next;
    else
        s->prev->next = s->next;

    if(s->next != 0)
        s->next->prev = s->prev;

    fnet_isr_unlock();
}


/************************************************************************
* NAME: fnet_socket_desc_alloc
*
* DESCRIPTION: This function reserves socket descriptor.
*************************************************************************/
SOCKET fnet_socket_desc_alloc( void )
{
    int i;
    int res = FNET_ERR;

    fnet_isr_lock();

    for (i = 0; i < FNET_CFG_SOCKET_MAX; i++) /* Find the empty descriptor.*/
    {
        if(fnet_socket_desc[i] == 0)
        {
            fnet_socket_desc[i] = (fnet_socket_t *)FNET_SOCKET_DESC_RESERVED;
            res = i;
            break;
        }
    }

    fnet_isr_unlock();

    return (res);
}
/************************************************************************
* NAME: fnet_socket_desc_set
*
* DESCRIPTION: This function assigns the socket descriptor to the socket.
*************************************************************************/
void fnet_socket_desc_set( SOCKET desc, fnet_socket_t *sock )
{
    fnet_socket_desc[desc] = sock;
    sock->descriptor = desc;
}

/************************************************************************
* NAME: fnet_socket_desc_free
*
* DESCRIPTION: This function frees the socket descriptor.
*************************************************************************/
void fnet_socket_desc_free( SOCKET desc )
{
    fnet_socket_desc[desc] = 0;
}

/************************************************************************
* NAME: fnet_socket_desc_find
*
* DESCRIPTION: This function looking for socket structure 
*              associated with the socket descriptor.
*************************************************************************/
fnet_socket_t *fnet_socket_desc_find( SOCKET desc )
{
    fnet_socket_t *s = 0;

    if((desc >= 0) && (desc!=SOCKET_INVALID))
    {
        if((desc < FNET_CFG_SOCKET_MAX))
            s = fnet_socket_desc[desc];
    }

    return (s);
}

/************************************************************************
* NAME: fnet_socket_release
*
* DESCRIPTION: This function release all resources allocated for the socket. 
*************************************************************************/
void fnet_socket_release( fnet_socket_t ** head, fnet_socket_t *sock )
{
    fnet_isr_lock();
    fnet_socket_list_del(head, sock);
    fnet_socket_buffer_release(&sock->receive_buffer);
    fnet_socket_buffer_release(&sock->send_buffer);
    fnet_netbuf_free_chain(sock->options.ip_opt.options);
    fnet_free(sock);
    fnet_isr_unlock();
}


/************************************************************************
* NAME: fnet_socket_conflict
*
* DESCRIPTION: Return 1 if there's a socket whose addresses 'confict' 
*              with the supplied addresses.
*************************************************************************/
int fnet_socket_conflict( fnet_socket_t *head,         fnet_ip_addr_t local_addr,   unsigned short local_port,
                          fnet_ip_addr_t foreign_addr, unsigned short foreign_port, int wildcard )
{
    fnet_socket_t *sock = head;

    while(sock != 0)
    {
        if((sock->local_port == local_port)
               && (((local_addr == 0) && (wildcard)) || (sock->local_addr == local_addr))
               && (((foreign_addr == 0) && (wildcard)) || (sock->foreign_addr == foreign_addr))
               && (((foreign_port == 0) && (wildcard)) || (sock->foreign_port == foreign_port)))
            return (1);

        sock = sock->next;
    }

    return (0);
}


/************************************************************************
* NAME: fnet_socket_lookup
*
* DESCRIPTION: This function looks for a socket with the best match 
*              to the local and foreign address parameters.
*************************************************************************/
fnet_socket_t *fnet_socket_lookup( fnet_socket_t *head,       fnet_ip_addr_t local_addr,
                                   unsigned short local_port, fnet_ip_addr_t foreign_addr,
                                   unsigned short foreign_port )
{
    fnet_socket_t *sock;
    fnet_socket_t *match_sock = 0;
    int match_wildcard = 3;
    int wildcard;

    for (sock = head; sock != 0; sock = sock->next)
    {
        /* Compare local port number.*/
        if(sock->local_port != local_port)
            continue; // ignore.

        wildcard = 0;

        /* Compare local address.*/
        if(sock->local_addr != INADDR_ANY)
        {
            if(local_addr == INADDR_ANY)
                wildcard++;

            else if(sock->local_addr != local_addr)
                continue;
        }
        else
        {
            if(local_addr != INADDR_ANY)
                wildcard++;
        }

        /* Compare foreign address and port number.*/
        if(sock->foreign_addr != INADDR_ANY)
        {
            if(foreign_addr == INADDR_ANY)
                wildcard++;

            else if(sock->foreign_addr != foreign_addr || sock->foreign_port != foreign_port)
                continue;
        }
        else
        {
            if(foreign_addr != INADDR_ANY)
                wildcard++;
        }

        if(wildcard < match_wildcard)
        {
            match_sock = sock;

            if((match_wildcard = wildcard) == 0)
                break; /* Exact match is found.*/
        }
    }

    return (match_sock);
}

/************************************************************************
* NAME: fnet_socket_uniqueport
*
* DESCRIPTION: Chose a unique (non-conflicting) local port for the socket
*              list starting at 'head'. The port will always be
*	           FNET_SOCKET_PORT_RESERVED < local_port <= FNET_SOCKET_PORT_USERRESERVED (ephemeral port).
*              In network byte order.
*************************************************************************/
unsigned short fnet_socket_uniqueport( fnet_socket_t *head, fnet_ip_addr_t local_addr )
{
    unsigned short local_port = head->local_port;

    fnet_isr_lock();

    do
    {
        if(++local_port <= FNET_SOCKET_PORT_RESERVED || local_port > FNET_SOCKET_PORT_USERRESERVED)
            local_port = FNET_SOCKET_PORT_RESERVED + 1;
    } 
    while (fnet_socket_conflict(head, local_addr, fnet_htons(local_port), 0, 0, 1)); //TBD PFI

    fnet_isr_unlock();
    return (fnet_htons(local_port)); //TBD PFI
}


/************************************************************************
* NAME: fnet_socket_copy
*
* DESCRIPTION: This function creates new socket structure and fills 
*              its proper fields by values from existing socket 
*************************************************************************/
fnet_socket_t *fnet_socket_copy( fnet_socket_t *sock )
{
    fnet_socket_t *sock_cp;

    if((sock_cp = (fnet_socket_t *)fnet_malloc(sizeof(fnet_socket_t))) != 0)
    {
        fnet_memcpy(sock_cp, sock, sizeof(fnet_socket_t));

        sock_cp->next = 0;
        sock_cp->prev = 0;
        sock_cp->descriptor = FNET_SOCKET_DESC_RESERVED;
        sock_cp->state = SS_UNCONNECTED;
        sock_cp->protocol_control = 0;
        sock_cp->head_con = 0;
        sock_cp->partial_con = 0;
        sock_cp->incoming_con = 0;
        sock_cp->receive_buffer.count = 0;
        sock_cp->receive_buffer.net_buf_chain = 0;
        sock_cp->send_buffer.count = 0;
        sock_cp->send_buffer.net_buf_chain = 0;
        sock_cp->options.ip_opt.options = 0;
        sock_cp->options.error = FNET_OK;
        sock_cp->options.local_error = FNET_OK;
        return (sock_cp);
    }
    else
        return (0);
}
/************************************************************************
* NAME: socket
*
* DESCRIPTION: This function creates a socket and returns 
*              the descriptor to the application.
*************************************************************************/
SOCKET socket( int domain, int type, int protocol )
{
    fnet_prot_if_t *prot;
    fnet_socket_t *sock;
    SOCKET res;
    int error = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR_1;
    }

    res = fnet_socket_desc_alloc();

    if(res == FNET_ERR)
    {
        error = FNET_ERR_NO_DESC; /* No more socket descriptors are available.*/
        goto ERROR_1;
    }

    if((prot = fnet_prot_find(domain, type, protocol)) == 0)
    {
        error = FNET_ERR_PROTONOSUPPORT; /* Protocol not supported.*/
        goto ERROR_2;
    }

    if((sock = (fnet_socket_t *)fnet_malloc(sizeof(fnet_socket_t))) == 0)
    {
        error = FNET_ERR_NOMEM; /* Cannot allocate memory.*/
        goto ERROR_2;
    }

    fnet_memset(sock, 0, sizeof(fnet_socket_t));
    fnet_socket_desc_set(res, sock);
    sock->protocol_interface = prot;
    sock->state = SS_UNCONNECTED;
    fnet_socket_list_add(&prot->head, sock);

    if(prot->prot_attach && (prot->prot_attach(sock) == SOCKET_ERROR))
    {
        fnet_socket_release(&sock->protocol_interface->head, sock);
        error = fnet_error_get();
        goto ERROR_2;
    }

    fnet_os_mutex_unlock();
    return (res);

    ERROR_2:
    fnet_socket_desc_free(res);

    ERROR_1:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_INVALID);
}

/************************************************************************
* NAME: connect
*
* DESCRIPTION: This function establishes a connection to 
*              a specified socket.
*************************************************************************/
int connect( SOCKET s, struct sockaddr *name, int namelen )
{
    fnet_socket_t *sock;
    int error = FNET_OK;
    fnet_ip_addr_t foreign_addr;
    fnet_ip_addr_t local_addr;
    unsigned short foreign_port;
    unsigned short local_port;
    int result;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->state == SS_LISTENING) /* The socket is marked to accept connections (listen).*/
        {
            error = FNET_ERR_OPNOTSUPP; /*  Operation not supported.*/
            goto ERROR_SOCK;
        }

        if(name == 0)
        {
            error = FNET_ERR_DESTADDRREQ; /* Destination address required.*/
            goto ERROR_SOCK;
        }

        /* The protocol is connection oriented and a connection has already been initiated.*/
        if(((sock->state == SS_CONNECTED) && sock->protocol_interface->con_req))
        {
            error = FNET_ERR_ISCONN; /* Socket is already connected.*/
            goto ERROR_SOCK;
        }

        if(((sock->state == SS_CONNECTING) && sock->protocol_interface->con_req))
        {
            error = FNET_ERR_INPROGRESS; /* The action is in progress. */
            goto ERROR_SOCK;
        }

        if(namelen != sizeof(struct sockaddr))
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }

        if(name->sa_family != AF_INET)
        {
            error = FNET_ERR_AFNOSUPPORT; /* Address family is not supported.*/
            goto ERROR_SOCK;
        }

        foreign_addr = ((struct sockaddr_in *)name)->sin_addr.s_addr;

        if(foreign_addr == INADDR_ANY)
        {
            error = FNET_ERR_DESTADDRREQ; /* Destination address required.*/
            goto ERROR_SOCK;
        }

        if((foreign_port = (((struct sockaddr_in *)name)->sin_port)) == 0)
        {
            error = FNET_ERR_ADDRNOTAVAIL; /* Can't assign requested port.*/
            goto ERROR_SOCK;
        }

        if((local_addr = sock->local_addr) == INADDR_ANY)
        {
            fnet_netif_t *netif;

            if((netif = fnet_ip_route(foreign_addr)) == 0)
            {
                error = FNET_ERR_NETUNREACH; /* No route. */
                goto ERROR_SOCK;
            }

            local_addr = netif->addr.address;
        }

        if((local_port = sock->local_port) == 0)
        {
            local_port = fnet_socket_uniqueport(sock->protocol_interface->head,
                                                sock->local_addr); /* Get ephemeral port.*/
        }

        if(fnet_socket_conflict(sock->protocol_interface->head, local_addr, local_port, foreign_addr,
                                foreign_port,                   1))
        {
            error = FNET_ERR_ADDRINUSE; /* Address already in use. */
            goto ERROR_SOCK;
        }

        sock->local_port = local_port;
        sock->local_addr = local_addr;

        /* Start the appropriate protocol connection.*/
        if(sock->protocol_interface->prot_connect)
            result = sock->protocol_interface->prot_connect(sock, foreign_addr, foreign_port);
        else
            result = FNET_OK;
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: bind
*
* DESCRIPTION: This function associates a local address with a socket.
*************************************************************************/
int bind( SOCKET s, const struct sockaddr *name, int namelen )
{
    fnet_socket_t *sock;
    fnet_ip_addr_t local_addr = 0;
    unsigned short local_port = 0;
    int error = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if((sock->local_port == 0) && (sock->local_addr == INADDR_ANY))
        {
            if(name)
            {
                if(namelen != sizeof(struct sockaddr))
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }

                if(name->sa_family != AF_INET)
                {
                    error = FNET_ERR_AFNOSUPPORT; /* Address family is not supported.*/
                    goto ERROR_SOCK;
                }

                local_addr = ((struct sockaddr_in *)name)->sin_addr.s_addr;
                local_port = (((struct sockaddr_in *)name)->sin_port);

                if((local_addr != INADDR_ANY) && (!FNET_IP_MULTICAST(local_addr))
                       && (fnet_netif_find_address(local_addr) == 0))
                {
                    error =
                        FNET_ERR_ADDRNOTAVAIL; /* The specified address is not a valid address for this system.*/
                    goto ERROR_SOCK;
                }

                if(!FNET_IP_MULTICAST(local_addr)) //Is not multicast.
                {
                    if((local_port != 0)
                           && fnet_socket_conflict(sock->protocol_interface->head, local_addr, local_port,
                                                       0, 0, 0))
                    {
                        error = FNET_ERR_ADDRINUSE; /* Address already in use. */
                        goto ERROR_SOCK;
                    }
                }

                sock->local_addr = local_addr;
            }

            if(local_port == 0)
            {
                local_port = fnet_socket_uniqueport(sock->protocol_interface->head,
                                                    local_addr); /* Get ephemeral port.*/
            }

            sock->local_port = local_port;

            fnet_socket_buffer_release(&sock->receive_buffer);
            fnet_socket_buffer_release(&sock->send_buffer);
        }
        else
        {
            error = FNET_ERR_INVAL; /* The socket is already bound to an address.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (FNET_OK);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: closesocket
*
* DESCRIPTION: This function closes an existing socket.
*************************************************************************/
int closesocket( SOCKET s )
{
    fnet_socket_t *sock;
    int result = FNET_OK;
    int error;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->protocol_interface->prot_detach)
            result = sock->protocol_interface->prot_detach(sock);

        if(result == FNET_OK)
            fnet_socket_desc_free(s);
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);
    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: shutdown
*
* DESCRIPTION: This function to disable reception, transmission, or both.
*************************************************************************/
int shutdown( SOCKET s, int how )
{
    fnet_socket_t *sock;
    int result = FNET_OK;
    int error;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->protocol_interface && sock->protocol_interface->prot_shutdown)
            result = sock->protocol_interface->prot_shutdown(sock, how);
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);
    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: listen
*
* DESCRIPTION: This function places the socket into the state where 
*              it is listening for an incoming connection.
*************************************************************************/
int listen( SOCKET s, int backlog )
{
    fnet_socket_t *sock;

    int error;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if((sock->state == SS_CONNECTING) || (sock->state == SS_CONNECTED))
        {
            error = FNET_ERR_ISCONN; /* Operation not supported.*/
            goto ERROR_SOCK;
        }

        if(sock->local_port == 0)
        {
            error = FNET_ERR_BOUNDREQ; /* The socket has not been bound.*/
            goto ERROR_SOCK;
        }

        if(backlog <= 0)
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }

        if(sock->protocol_interface && sock->protocol_interface->prot_listen)
        {
            result = sock->protocol_interface->prot_listen(sock, 1);
        }
        else
        {
            error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: accept
*
* DESCRIPTION: This function accepts a connection on a specified socket.
*************************************************************************/
SOCKET accept( SOCKET s, struct sockaddr *addr, int *addrlen )
{
    fnet_socket_t *sock;
    fnet_socket_t *sock_new;
    SOCKET desc;
    int error;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->protocol_interface && sock->protocol_interface->prot_accept)
        {
            if(sock->state != SS_LISTENING)
            {
                error = FNET_ERR_INVAL; /* Invalid argument.*/
                goto ERROR_SOCK;
            }

            if(addr && addrlen)
            {
                if(*addrlen < sizeof(struct sockaddr))
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }
            }

            if((desc = fnet_socket_desc_alloc()) != FNET_ERR)
            {
                fnet_isr_lock();

                if((sock_new = sock->protocol_interface->prot_accept(sock)) == 0)
                {
                    fnet_socket_desc_free(desc);
                    fnet_isr_unlock();
                    error = FNET_ERR_AGAIN;
                    goto ERROR_SOCK;
                };

                fnet_socket_desc_set(desc, sock_new);
                fnet_socket_list_add(&sock->protocol_interface->head, sock_new);
                fnet_isr_unlock();

                if(addr && addrlen)
                {
                    ((struct sockaddr_in *)addr)->sin_addr.s_addr = sock_new->foreign_addr;
                    ((struct sockaddr_in *)addr)->sin_port = sock_new->foreign_port;
                    addr->sa_family = AF_INET;

                    *addrlen = sizeof(struct sockaddr);
                }
            }
            else
            {
                error = FNET_ERR_NO_DESC; /* No more socket descriptors are available.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_OPNOTSUPP; /*  Operation not supported.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (desc);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_INVALID);
}


/************************************************************************
* NAME: send
*
* DESCRIPTION: This function sends data on a connected socket. 
*************************************************************************/
int send( SOCKET s, char *buf, int len, int flags )
{
    fnet_socket_t *sock;
    int error;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->foreign_addr == INADDR_ANY)
        {
            error = FNET_ERR_NOTCONN; /* Socket is not connected.*/
            goto ERROR_SOCK;
        }

        if(buf && (len >= 0))
        {
            if(sock->protocol_interface->prot_snd)
            {
                result = sock->protocol_interface->prot_snd(sock, buf, len, flags, 0, 0);
            }
            else
            {
                error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}


/************************************************************************
* NAME: sendto
*
* DESCRIPTION: This function sends data to a specific destination. 
*************************************************************************/
int sendto( SOCKET s, char *buf, int len, int flags, struct sockaddr *to, int tolen )
{
    fnet_socket_t *sock;
    fnet_ip_addr_t foreign_addr = 0; /* foreign IP address */
    unsigned short foreign_port = 0; /* foreign port */
    int error;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(buf && (len >= 0))
        {
            if(to)
            {
                if(tolen != sizeof(struct sockaddr))
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }

                if(to->sa_family != AF_INET)
                {
                    error = FNET_ERR_AFNOSUPPORT; /* Address family is not supported.*/
                    goto ERROR_SOCK;
                }

                foreign_addr = ((struct sockaddr_in *)to)->sin_addr.s_addr;
                foreign_port = ((struct sockaddr_in *)to)->sin_port;

                if(foreign_addr == INADDR_ANY)
                {
                    error = FNET_ERR_DESTADDRREQ; /* Destination address required.*/
                    goto ERROR_SOCK;
                }
            }
            else
            {
                error = FNET_ERR_DESTADDRREQ; /* Destination address required.*/
                goto ERROR_SOCK;
            }

            if(sock->protocol_interface->prot_snd)
            {
                result = sock->protocol_interface->prot_snd(sock, buf, len, flags, foreign_addr,
                                                            foreign_port);
            }
            else
            {
                error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}


/************************************************************************
* NAME: recv
*
* DESCRIPTION: This function receives data from a connected socket. 
*************************************************************************/
int recv( SOCKET s, char *buf, int len, int flags )
{
    fnet_socket_t *sock;
    int error;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(buf && (len >= 0))
        {

            /* The sockets must be bound before calling recv.*/
            if(sock->local_port == 0)
            {
                error = FNET_ERR_BOUNDREQ; /* The socket has not been bound with bind().*/
                goto ERROR_SOCK;
            }

            if(sock->protocol_interface->prot_rcv)
            {
                result = sock->protocol_interface->prot_rcv(sock, buf, len, flags, 0, 0);
            }
            else
            {
                error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor. */
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: recvfrom
*
* DESCRIPTION: This function reads incoming data of socket and captures 
*              the address from which the data was sent.  
*************************************************************************/
int recvfrom( SOCKET s, char *buf, int len, const int flags, struct sockaddr *from, int *fromlen )
{
    fnet_socket_t *sock;
    int error;
    fnet_ip_addr_t foreign_addr;
    unsigned short foreign_port;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(buf && (len >= 0))
        {

            /* The sockets must be bound before calling recv.*/
            if(sock->local_port == 0)
            {
                error = FNET_ERR_BOUNDREQ; /* The socket has not been bound with bind().*/
                goto ERROR_SOCK;
            }

            if(from && fromlen)
            {
                if(*fromlen < sizeof(struct sockaddr))
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }
            }

            if(sock->protocol_interface->prot_rcv)
            {
                if((result = sock->protocol_interface->prot_rcv(sock, buf, len, flags, 
                             (from && fromlen) ? &foreign_addr : 0, 
                             (from && fromlen) ? &foreign_port : 0))
                       != FNET_ERR)
                {
                    if(from && fromlen)
                    {
                        ((struct sockaddr_in *)from)->sin_addr.s_addr = foreign_addr;
                        ((struct sockaddr_in *)from)->sin_port = foreign_port;
                        from->sa_family = AF_INET;
                    }
                }
            }
            else
            {
                error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: getsockname
*
* DESCRIPTION: This function retrieves the current name 
*              for the specified socket. 
*************************************************************************/
int getsockname( SOCKET s, struct sockaddr *name, int *namelen )
{
    fnet_socket_t *sock;

    int error;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if((name == 0) || (*namelen < sizeof(struct sockaddr)))
        {
            error = FNET_ERR_INVAL;
            goto ERROR_SOCK;
        }

        if(name->sa_family != AF_INET)
        {
            error = FNET_ERR_AFNOSUPPORT; /* Address family is not supported.*/
            goto ERROR_SOCK;
        }

        if(sock->local_port == 0)
        {
            error = FNET_ERR_BOUNDREQ; /* The socket has not been bound with bind().*/
            goto ERROR_SOCK;
        }

        fnet_memset(name, 0, sizeof(struct sockaddr));

        ((struct sockaddr_in *)name)->sin_addr.s_addr = sock->local_addr;
        ((struct sockaddr_in *)name)->sin_port = sock->local_port;
        name->sa_family = AF_INET;
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (FNET_OK);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: getsockname
*
* DESCRIPTION: This function retrieves the name of the peer 
*              connected to the socket
*************************************************************************/
int getpeername( SOCKET s, struct sockaddr *name, int *namelen )
{
    fnet_socket_t *sock;
    int error;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if((name == 0) || (*namelen < sizeof(struct sockaddr)))
        {
            error = FNET_ERR_INVAL;
            goto ERROR_SOCK;
        }

        if(name->sa_family != AF_INET)
        {
            error = FNET_ERR_AFNOSUPPORT; /* Address family is not supported.*/
            goto ERROR_SOCK;
        }

        if(sock->foreign_addr == INADDR_ANY)
        {
            error = FNET_ERR_NOTCONN; /* Socket is not connected.*/
            goto ERROR_SOCK;
        }

        fnet_memset(name, 0, sizeof(struct sockaddr));

        ((struct sockaddr_in *)name)->sin_addr.s_addr = sock->foreign_addr;
        ((struct sockaddr_in *)name)->sin_port = sock->foreign_port;
        name->sa_family = AF_INET;
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (FNET_OK);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}


/************************************************************************
* NAME: setsockopt
*
* DESCRIPTION: This function sets the current value for a socket option 
*              associated with a socket
*************************************************************************/
int setsockopt( SOCKET s, int level, int optname, char *optval, int optlen )
{
    fnet_socket_t *sock;
    int error;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(optval && optlen)
        {
            if(level != SOL_SOCKET)
            {
                if(sock->protocol_interface && sock->protocol_interface->prot_setsockopt)
                    result = sock->protocol_interface->prot_setsockopt(sock, level, optname, optval, optlen);
                else
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }
            }
            else
            {
                switch(optname)     /* Socket options processing.*/
                {
                    case SO_LINGER: /* Linger on close if data present.*/
                      if(optlen != sizeof(struct linger))
                      {
                          error = FNET_ERR_INVAL;
                          goto ERROR_SOCK;
                      }

                      sock->options.linger = ((struct linger *)optval)->l_linger
                                                 * (1000 / FNET_TIMER_PERIOD_MS);

                      if(((struct linger *)optval)->l_onoff)
                          sock->options.flags |= optname;
                      else
                          sock->options.flags &= ~optname;

                      break;

                    case SO_KEEPALIVE: /* Keep connections alive.*/
                    case SO_DONTROUTE: /* Just use interface addresses.*/
                    case SO_OOBINLINE: /* Leave received OOB data in line.*/
#if FNET_CFG_SOCKET_BLOCKING_SUPPORT                     
                    case SO_BLOCKING:  /* Blocking state.*/
#endif                    
                      if(optlen < sizeof(int))
                      {
                          error = FNET_ERR_INVAL;
                          goto ERROR_SOCK;
                      }

                      if(*((int *)optval))
                          sock->options.flags |= optname;
                      else
                          sock->options.flags &= ~optname;

                      break;

                    case SO_SNDBUF: /* Send buffer size.*/
                    case SO_RCVBUF: /* Receive buffer size.*/
                      if((optlen < sizeof(unsigned long)))
                      {
                          error = FNET_ERR_INVAL;
                          goto ERROR_SOCK;
                      }

                      if(optname == SO_SNDBUF)
                          sock->send_buffer.count_max = *((unsigned long *)optval);
                      else
                          sock->receive_buffer.count_max = *((unsigned long *)optval);

                      break;
#if FNET_CFG_SOCKET_BLOCKING_SUPPORT 
                    case SO_SNDTIMEO: /* Send timeout.*/
                    case SO_RCVTIMEO: /* Receive timeout.*/
                        {
                            struct timeval *tv;
                            unsigned long tmp_val;

                            if((optlen < sizeof(struct timeval)))
                            {
                                error = FNET_ERR_INVAL;
                                goto ERROR_SOCK;
                            }

                            tv = (struct timeval *)(optval);

                            tmp_val = (unsigned long)((tv->tv_sec * 1000) / FNET_TIMER_PERIOD_MS + tv->tv_usec
                                                          / (1000 * FNET_TIMER_PERIOD_MS));

                            if(tmp_val > 65535)
                            {
                                error = FNET_ERR_INVAL;
                                goto ERROR_SOCK;
                            }

                            if(optname == SO_RCVTIMEO)
                                sock->receive_buffer.timeout = (unsigned short)tmp_val;
                            else
                                sock->send_buffer.timeout = (unsigned short)tmp_val;
                        }
                        break;
#endif
                    default:
                      error = FNET_ERR_NOPROTOOPT; /* The option is unknown or unsupported. */
                      goto ERROR_SOCK;
                }
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}
/************************************************************************
* NAME: getsockopt
*
* DESCRIPTION: This function retrieves the current value for 
*              a socket option associated with a socket 
*************************************************************************/
int getsockopt( SOCKET s, int level, int optname, char *optval, int *optlen )
{
    fnet_socket_t *sock;
    int error;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR;
    }

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(optval && optlen)
        {
            if(level != SOL_SOCKET)
            {
                if(sock->protocol_interface && sock->protocol_interface->prot_getsockopt)
                    result = sock->protocol_interface->prot_getsockopt(sock, level, optname, optval, optlen);
                else
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }
            }
            else
            {
                switch(optname)     /* Socket options processing.*/
                {
                    case SO_LINGER: /* Linger on close if data present.*/
                        if(*optlen < sizeof(struct linger))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(struct linger);
                        ((struct linger *)optval)->l_onoff
                                = (unsigned short)((sock->options.flags & SO_LINGER) > 0);
                        ((struct linger *)optval)->l_linger
                            = (unsigned short)((sock->options.linger * FNET_TIMER_PERIOD_MS) / 1000);
                        sock->options.linger = ((struct linger *)optval)->l_linger;
                        break;

                    case SO_KEEPALIVE: /* Keep connections alive.*/
                    case SO_DONTROUTE: /* Just use interface addresses.*/
                    case SO_OOBINLINE: /* Leave received OOB data in line.*/
#if FNET_CFG_SOCKET_BLOCKING_SUPPORT 
                    case SO_BLOCKING:  /* Blocking state.*/
#endif                    
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int*)optval) = (int)((sock->options.flags & optname) > 0);
                        break;

                    case SO_ACCEPTCONN: /* Socket is listening. */
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int*)optval) = (int)(sock->state == SS_LISTENING);
                        break;

                    case SO_SNDBUF: /* Send buffer size.*/
                    case SO_RCVBUF: /* Receive buffer size.*/
                        if(*optlen < sizeof(unsigned long))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(unsigned long);

                        if(optname == SO_SNDBUF)
                            *((unsigned long*)optval)=sock->send_buffer.count_max;
                        else
                            *((unsigned long *)optval) = sock->receive_buffer.count_max;
                        break;
                    case SO_STATE: /* State of the socket.*/
                        if(*optlen < sizeof(fnet_socket_state_t))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(fnet_socket_state_t);
                        *((fnet_socket_state_t*)optval) = sock->state;
                        break;

                    case SO_RCVNUM:  /* Use to determine the amount of data pending in the network's input buffer that can be read from socket.*/
                    case SO_SNDNUM: /* Use to determine the amount of data in the network's output buffer.*/
                        if(*optlen < sizeof(unsigned long))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(unsigned long);

                        if(optname == SO_RCVNUM)
                            *((unsigned long*)optval)=sock->receive_buffer.count;
                        else
                            *((unsigned long *)optval) = sock->send_buffer.count;
                        break;
#if FNET_CFG_SOCKET_BLOCKING_SUPPORT 
                    case SO_SNDTIMEO: /* Send timeout.*/
                    case SO_RCVTIMEO: /* Receive timeout.*/
                        {
                            unsigned long tmp_val;

                            if((*optlen < sizeof(struct timeval)))
                            {
                                error = FNET_ERR_INVAL;
                                goto ERROR_SOCK;
                            }

                            tmp_val = (unsigned long)(optname == SO_SNDTIMEO ? sock->send_buffer.timeout
                                                          : sock->receive_buffer.timeout);

                            *optlen = sizeof(struct timeval);

                            ((struct timeval *)optval)->tv_sec = (long)((tmp_val * FNET_TIMER_PERIOD_MS) / 1000);
                            ((struct timeval *)optval)->tv_sec
                                = (long)(((tmp_val * FNET_TIMER_PERIOD_MS) % 1000) * 1000);
                        }
                        break;
#endif
                    case SO_ERROR: /* Socket error.*/
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int *)optval) = sock->options.error;
                        sock->options.error = FNET_OK; /* Reset error.*/
                        break;

                    case SO_TYPE:
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int *)optval) = (sock->protocol_interface ? sock->protocol_interface->type : 0);
                        break;

                    default:
                        error = FNET_ERR_NOPROTOOPT; /* The option is unknown or unsupported. */
                        goto ERROR_SOCK;
                }/* case*/
            }/* else */
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

    ERROR_SOCK:
    fnet_socket_set_error(sock, error);

    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: inet_ntoa
*
* DESCRIPTION:The function converts an (IPv4) Internet network address 
*             into a string in Internet standard dotted format.
*************************************************************************/
char *inet_ntoa( struct in_addr addr, char *res_str )
{
    unsigned char *ptr;

    ptr = (unsigned char *) &addr;
    fnet_sprintf(res_str, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);

    return res_str;
}

/************************************************************************
* NAME: inet_aton
*
* DESCRIPTION:The function converts a string containing an (Ipv4) 
*             Internet Protocol dotted address into a suitable binary 
*             representation of the Internet address.
*************************************************************************/
int inet_aton( char *cp, struct in_addr *addr )
{
    fnet_ip_addr_t val;
    unsigned char base, n;
    char c;
    unsigned long octet[4], *octetptr = octet;

    again:

    /* Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, other=decimal.
     */
    val = 0;

    base = 10;

    if(*cp == '0')
        base = 8, cp++;

    if(*cp == 'x' || *cp == 'X')
        base = 16, cp++;

    while((c = *cp) != 0)
    {
        /* If "c" is digit. */
        if((c >= '0') && (c <= '9'))
        {
            val = (val * base) + (c - '0');
            cp++;
            continue;
        }
        /* If base == 16 and "c" is digit.*/
        else if(base == 16 && (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))))
        {
            val = (val << 4) + (c + 10 - (((c >= 'a') && (c <= 'f')) ? 'a' : 'A'));
            cp++;
            continue;
        }

        break;
    }

    if(*cp == '.')
    {

        /* Internet format:
         *      a.b.c.d
         *      a.b.c   (with c treated as 16-bits)
         *      a.b     (with b treated as 24 bits)
         */
        *octetptr++ = val, cp++;

        if(octetptr >= octet + 4)
            return (FNET_ERR);

        goto again;
    }

    /* Check for trailing characters.*/
    if(*cp && !(*cp == ' '))
        return (FNET_ERR);

    *octetptr++ = val;

    /* Concoct the address according to
     * the number of octet specified.
     */
    n = (unsigned char)(octetptr - octet);

    switch(n)
    {
        case 1: /* a -- 32 bits.*/
          val = octet[0];
          break;

        case 2: /* a.b -- 8.24 bits.*/
          val = (octet[0] << 24) | (octet[1] & 0xffffff);
          break;

        case 3: /* a.b.c -- 8.8.16 bits.*/
          val = (octet[0] << 24) | ((octet[1] & 0xff) << 16) | (octet[2] & 0xffff);
          break;

        case 4: /* a.b.c.d -- 8.8.8.8 bits.*/
          val = (octet[0] << 24) | ((octet[1] & 0xff) << 16) | ((octet[2] & 0xff) << 8) | (octet[3] & 0xff);
          break;

        default:
          return (FNET_ERR);
    }

    addr->s_addr = fnet_htonl(val);

    return (FNET_OK);
}


/************************************************************************
* NAME: inet_addr
*
* DESCRIPTION:The function converts a string containing an (Ipv4) 
*             Internet Protocol dotted address into a suitable binary 
*             representation of the Internet address.
*************************************************************************/
unsigned long inet_addr( char *cp )
{
    struct in_addr addr;

    if(inet_aton(cp, &addr) == FNET_OK)
        return (addr.s_addr);
    else
        return ((unsigned long)INADDR_NONE);
}

/************************************************************************
* NAME: fnet_socket_buffer_release
*
* DESCRIPTION: Discards any buffers in the socket buffer
*************************************************************************/
void fnet_socket_buffer_release( fnet_socket_buffer_t *sb )
{
    fnet_netbuf_t *nb_ptr, *tmp_nb_ptr;

    fnet_isr_lock();

    if(sb && sb->net_buf_chain)
    {
        nb_ptr = sb->net_buf_chain;

        while(nb_ptr != 0)
        {
            if((nb_ptr->type == FNET_NETBUF_TYPE_ADDRESS))
            {
                fnet_netbuf_free_chain(((fnet_socket_buffer_addr_t *)(nb_ptr->data_ptr))->ip_options);
            }

            tmp_nb_ptr = nb_ptr->next_chain;
            fnet_netbuf_free_chain(nb_ptr);
            nb_ptr = tmp_nb_ptr;
        }

        sb->net_buf_chain = 0;
        sb->count = 0;
    }

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_socket_buffer_append_record
*
* DESCRIPTION: Append the record to the end of the socket buffer.
*************************************************************************/
int fnet_socket_buffer_append_record( fnet_socket_buffer_t *sb, fnet_netbuf_t *nb )
{
    fnet_isr_lock();

    if((nb->total_length + sb->count) > sb->count_max)
    {
        fnet_isr_unlock();
        return FNET_ERR;
    }

    sb->net_buf_chain = fnet_netbuf_concat(sb->net_buf_chain, nb);

    sb->count += nb->total_length;
    fnet_isr_unlock();

    return FNET_OK;
}

/************************************************************************
* NAME: fnet_socket_buffer_append_address
*
* DESCRIPTION: Constract net_buf chain  and add it to the queue. 
*              The chain contains the address of the message 
*              and the message data.
*************************************************************************/
int fnet_socket_buffer_append_address( fnet_socket_buffer_t *sb, fnet_netbuf_t *nb, fnet_ip_addr_t addr,
                                       unsigned short port,      fnet_netbuf_t *opt )
{
    fnet_socket_buffer_addr_t *sb_address;
    fnet_netbuf_t *nb_addr;

    fnet_isr_lock();

    if((nb->total_length + sb->count) > sb->count_max)
    {
        fnet_isr_unlock();
        return FNET_ERR;
    }

    if((nb_addr = fnet_netbuf_new(sizeof(fnet_socket_buffer_addr_t), FNET_NETBUF_TYPE_ADDRESS,
                                      FNET_NETBUF_MALLOC_NOWAIT)) == 0)
    {
        fnet_isr_unlock();
        return FNET_ERR;
    }

    sb_address = (fnet_socket_buffer_addr_t *)nb_addr->data_ptr;
    sb_address->addr = addr;
    sb_address->port = port;
    sb_address->ip_options = opt;

    sb->count += nb->total_length;

    nb = fnet_netbuf_concat(nb_addr, nb);
    fnet_netbuf_add_chain(&sb->net_buf_chain, nb);
    fnet_isr_unlock();

    return FNET_OK;
}


/************************************************************************
* NAME: fnet_socket_buffer_read_record
*
* DESCRIPTION: This function reads data from socket buffer and 
*              put this data into application buffer. 
*************************************************************************/
int fnet_socket_buffer_read_record( fnet_socket_buffer_t *sb, char *buf, int len, int remove )
{
    if(sb->net_buf_chain)
    {
        if(len > sb->net_buf_chain->total_length)
            len = (int)sb->net_buf_chain->total_length;

        fnet_netbuf_to_buf(sb->net_buf_chain, 0, len, buf);

        if(remove)
        {
            fnet_isr_lock();
            fnet_netbuf_trim(&sb->net_buf_chain, len);
            sb->count -= len;
            fnet_isr_unlock();
        }
    }
    else
        len = 0;

    return len;
}

/************************************************************************
* NAME: fnet_socket_buffer_read_address
*
* DESCRIPTION:This function reads data from socket buffer and 
*             put this data into application buffer. 
*             And captures the address information from which the data was sent. 
*************************************************************************/
int fnet_socket_buffer_read_address( fnet_socket_buffer_t *sb, char *buf, int len, fnet_ip_addr_t *addr,
                                     unsigned short *port,     int remove )
{
    fnet_netbuf_t *nb, *nb_addr;

    if(((nb_addr = sb->net_buf_chain) != 0) && (nb_addr->type == FNET_NETBUF_TYPE_ADDRESS))
    {
        if((nb = nb_addr->next) != 0)
        {
            if(len > nb->total_length)
                len = (int)nb->total_length;

            fnet_netbuf_to_buf(nb, 0, len, buf);
        }
        else
            len = 0;

        *addr = ((fnet_socket_buffer_addr_t *)(nb_addr->data_ptr))->addr;
        *port = ((fnet_socket_buffer_addr_t *)(nb_addr->data_ptr))->port;

        if(len < (nb_addr->total_length - sizeof(fnet_socket_buffer_addr_t)))
            len = FNET_ERR;

        if(remove)
        {
            fnet_isr_lock();

            if(nb)
                sb->count -= nb->total_length;

            fnet_netbuf_free_chain(((fnet_socket_buffer_addr_t *)(nb_addr->data_ptr))->ip_options);
            fnet_netbuf_del_chain(&sb->net_buf_chain, nb_addr);
            fnet_isr_unlock();
        }
    }
    else
        len = 0;

    return len;
}

#if FNET_CFG_CPU_LITTLE_ENDIAN
unsigned short fnet_htons(unsigned short n)                                /* Convert short from host- to network byte order.*/
{
    unsigned short result = FNET_HTONS(n);
    return result;
}
unsigned long fnet_htonl(unsigned long n)                                  /* Convert long from host- to network byte order.*/
{
    unsigned long result = FNET_HTONL(n);
    return result;    
}
#endif



/************************************************************************
* NAME: fnet_stack_init
*
* DESCRIPTION: TCP/IP Stack initialization.
************************************************************************/
int fnet_stack_init( void )
{
    fnet_isr_init();
   
    fnet_timer_init(FNET_TIMER_PERIOD_MS);
 
    if(fnet_prot_init() == FNET_ERR)
        goto ERROR;
    fnet_socket_init();

    if(fnet_netif_init() == FNET_ERR)
        goto ERROR;

    return (FNET_OK);
ERROR:
    fnet_stack_release();

    return (FNET_ERR);
}

/************************************************************************
* NAME: fnet_stack_release
*
* DESCRIPTION: TCP/IP Stack release.
************************************************************************/
void fnet_stack_release( void )
{
    fnet_netif_release();
    fnet_prot_release();
    fnet_timer_release();
    fnet_mem_release();
}

