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
* @file fnet_ip.c
*
* @author Andrey Butok
*
* @date May-31-2011
*
* @version 0.1.21.0
*
* @brief IP protocol implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_ip_prv.h"
#include "fnet_icmp.h"
#include "fnet_checksum.h"
#include "fnet_timer_prv.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_isr.h"
#include "fnet_netbuf.h"
#include "fnet_netif_prv.h"
#include "fnet_prot.h"
#include "fnet_stdlib.h"

#if FNET_CFG_IP_FRAGMENTATION

static fnet_ip_frag_list_t *ip_frag_list_head;

static fnet_timer_desc_t ip_timer_ptr;

#endif

#define FNET_IP_QUEUE_COUNT_MAX (90*1024) /* Maximum size of IP input queue.*/ //TBD

typedef struct                            /* IP input queue.*/
{
    fnet_netbuf_t *head;                  /* Pointer to the queue head.*/
    unsigned long count;                  /* Number of data in buffer.*/
} fnet_ip_queue_t;

static fnet_ip_queue_t ip_queue;

/************************************************************************
*     Function Prototypes
*************************************************************************/
fnet_netbuf_t *fnet_ip_options_insert( fnet_netbuf_t *options, fnet_netbuf_t *nb );

#if FNET_CFG_IP_FRAGMENTATION

int fnet_ip_options_copy( fnet_ip_header_t *ipheader, fnet_ip_header_t *new_ipheader );
fnet_netbuf_t *fnet_ip_reassembly( fnet_netbuf_t ** nb_ptr );
void fnet_ip_frag_list_add( fnet_ip_frag_list_t ** head, fnet_ip_frag_list_t *fl );
void fnet_ip_frag_list_del( fnet_ip_frag_list_t ** head, fnet_ip_frag_list_t *fl );
void fnet_ip_frag_add( fnet_ip_frag_header_t ** head, fnet_ip_frag_header_t *frag,
                       fnet_ip_frag_header_t *frag_prev );
void fnet_ip_frag_del( fnet_ip_frag_header_t ** head, fnet_ip_frag_header_t *frag );
void fnet_ip_frag_list_free( fnet_ip_frag_list_t *list );
void fnet_ip_timer( void );

#endif

int fnet_ip_options_compile( fnet_socket_t *sk, char *optval, int optlen );
int fnet_ip_options_do( fnet_netif_t *netif, fnet_netbuf_t ** nb_ptr );

int fnet_ip_queue_append( fnet_ip_queue_t *queue, fnet_netif_t *netif, 
                            fnet_netbuf_t *nb );
fnet_netbuf_t *fnet_ip_queue_read( fnet_ip_queue_t *queue, fnet_netif_t ** netif );
void fnet_ip_input_low( void );

#if FNET_CFG_DEBUG_TRACE_IP
void fnet_ip_trace(char *str, fnet_ip_header_t *ip_hdr);
#else
#define fnet_ip_trace(str, ip_hdr)
#endif

/************************************************************************
* NAME: fnet_ip_init
*
* DESCRIPTION: This function makes initialization of the IP layer. 
*************************************************************************/
int fnet_ip_init( void )
{
#if FNET_CFG_IP_FRAGMENTATION

    ip_frag_list_head = 0;
    ip_timer_ptr = fnet_timer_new((FNET_IP_TIMER_PERIOD / FNET_TIMER_PERIOD_MS), fnet_ip_timer);

    if(ip_timer_ptr == 0)
        return (FNET_ERR);

#endif

    /* Install SW Interrupt handler. */
    fnet_isr_set_vector(FNET_IP_SW_VECTOR_NUMBER, 0, fnet_ip_input_low, 0);
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_ip_release
*
* DESCRIPTION: This function makes release of the all resources 
*              allocated for IP layer module.
*************************************************************************/
void fnet_ip_release( void )
{
    fnet_ip_drain();
#if FNET_CFG_IP_FRAGMENTATION

    fnet_timer_free(ip_timer_ptr);
    ip_timer_ptr = 0;

#endif

}
/************************************************************************
* NAME: fnet_ip_route
*
* DESCRIPTION: This function performs IP routing 
*              on an outgoing IP packet. 
*************************************************************************/
fnet_netif_t *fnet_ip_route( fnet_ip_addr_t dest_ip )
{
    fnet_netif_t *netif;

    for (netif = fnet_netif_list; netif != 0; netif = netif->next)
    {
        if((dest_ip &netif->addr.subnetmask) == (netif->addr.address &netif->addr.subnetmask))
            return (netif);
    }

    return (fnet_netif_get_default());
}

/************************************************************************
* NAME: fnet_ip_output
*
* DESCRIPTION: IP output function.
*
* RETURNS: FNET_OK=OK
*          FNET_ERR_NETUNREACH=No route
*          FNET_ERR_MSGSIZE=Size error
*          FNET_ERR_NOMEM=No memory
*************************************************************************/
int fnet_ip_output( fnet_netif_t *netif,    fnet_ip_addr_t src_ip, fnet_ip_addr_t dest_ip,
                    unsigned char protocol, unsigned char tos,     unsigned char ttl,
                    fnet_netbuf_t *nb,      int DF,                fnet_netbuf_t *options,
                    int do_not_route )
{
    static unsigned short ip_id;
    fnet_netbuf_t *nb_header;
    fnet_ip_header_t *ipheader;
    unsigned short total_length;

    if(netif == 0)
        if((netif = fnet_ip_route(dest_ip)) == 0) /* No route */
        {
            fnet_netbuf_free_chain(nb);           /* Discard datagram */
            fnet_netbuf_free_chain(options);      /* Discard options */
            return (FNET_ERR_NETUNREACH);
        }

    /* If source address not specified, use address of outgoing interface */
    if(src_ip == INADDR_ANY)
    {
        src_ip = netif->addr.address;
    };

    if((nb->total_length + sizeof(fnet_ip_header_t)) > FNET_IP_MAX_PACKET)
    {
        fnet_netbuf_free_chain(nb);      /* Discard datagram */
        fnet_netbuf_free_chain(options); /* Discard options */
        return (FNET_ERR_MSGSIZE);
    }

    /* Construct IP header */
    if((nb_header = fnet_netbuf_new(sizeof(fnet_ip_header_t), FNET_NETBUF_TYPE_HEADER,
                                        FNET_NETBUF_MALLOC_NOWAITDRAIN)) == 0)
    {
        fnet_netbuf_free_chain(nb);      /* Discard datagram */
        fnet_netbuf_free_chain(options); /* Discard options */
        return (FNET_ERR_NOMEM);
    }

    ipheader = nb_header->data_ptr;

    FNET_IP_HEADER_SET_VERSION(ipheader, (unsigned char)FNET_IP_VERSION); /* version =4 */
    ipheader->id = fnet_htons(ip_id++);              /* Id */

    ipheader->tos = tos;                 /* Type of service */
    total_length = (unsigned short)(nb->total_length + sizeof(fnet_ip_header_t)); /* total length*/
    FNET_IP_HEADER_SET_HEADER_LENGTH(ipheader, sizeof(fnet_ip_header_t) >> 2);
    ipheader->flags_fragment_offset = 0x0000; /* flags & fragment offset field */

    if(DF)
        ipheader->flags_fragment_offset |= FNET_HTONS(FNET_IP_DF);

    ipheader->ttl = ttl;                 /* time to live */
    ipheader->protocol = protocol;       /* protocol */
    ipheader->source_addr = src_ip;      /* source address */
    ipheader->desination_addr = dest_ip; /* destination address */

    if((options) && ((options = fnet_netbuf_copy(options, 0, FNET_NETBUF_COPYALL, FNET_NETBUF_MALLOC_NOWAIT))
                         != 0))
    {
        nb = fnet_ip_options_insert(options, nb);              /* IP options. */
        /* header length update. */
        FNET_IP_HEADER_SET_HEADER_LENGTH(ipheader, FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) + (options->total_length >> 2)); //TBD place of improvement.
        total_length += options->total_length >> 2;  /* total length update. */
    }

    ipheader->total_length = fnet_htons(total_length);
    
    
    nb = fnet_netbuf_concat(nb_header, nb);

    if(fnet_ntohs(ipheader->total_length) > netif->mtu) /* IP Fragmentation. */  //TBD PFI
    {
#if FNET_CFG_IP_FRAGMENTATION

        int first_frag_length, frag_length; /* The number of data in each fragment. */
        int offset;
        int error = 0;
        fnet_netbuf_t *tmp_nb;
        fnet_netbuf_t *nb_prev;
        fnet_netbuf_t ** nb_next_ptr = &nb->next_chain;
        int new_header_length, header_length = FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) << 2;
        fnet_ip_header_t *new_ipheader;

        frag_length = (int)(netif->mtu - header_length) & ~7; /* rounded down to an 8-byte boundary.*/
        first_frag_length = frag_length;

        if((ipheader->flags_fragment_offset & FNET_HTONS(FNET_IP_DF)) ||   /* The fragmentation is prohibited. */
        (frag_length < 8))                                    /* The MTU is too small.*/
        {
            fnet_netbuf_free_chain(nb);                       /* Discard datagram. */
            return (FNET_ERR_MSGSIZE);
        }

        /* The header (and options) must reside in contiguous area of memory.*/
        if((tmp_nb = fnet_netbuf_pullup(nb, header_length, FNET_NETBUF_MALLOC_NOWAIT)) == 0)
        {
            fnet_netbuf_free_chain(nb); /* Discard datagram. */
            return (FNET_ERR_NOMEM);
        }

        nb = tmp_nb;

        ipheader = nb->data_ptr;

        nb_prev = nb;

        /* Go through the whole data segment after first fragment.*/
        for (offset = (header_length + frag_length); offset < fnet_ntohs(ipheader->total_length); offset += frag_length) //TBD PFI
        {
            fnet_netbuf_t *nb_tmp;

            nb = fnet_netbuf_new(header_length, FNET_NETBUF_TYPE_HEADER,
                                 FNET_NETBUF_MALLOC_NOWAIT); /* Allocate a new header.*/

            if(nb == 0)
            {
                error++;
                goto FRAG_END;
            }

            fnet_memcpy(nb->data_ptr, ipheader, (unsigned int)header_length); /* Copy IP header.*/
            new_ipheader = nb->data_ptr;
            new_header_length = sizeof(fnet_ip_header_t);

            if(header_length > sizeof(fnet_ip_header_t))
            {
                /* The options copy.*/
                new_header_length += fnet_ip_options_copy(ipheader, new_ipheader);
            }

            FNET_IP_HEADER_SET_HEADER_LENGTH(new_ipheader, new_header_length >> 2); 
            new_ipheader->flags_fragment_offset = fnet_htons((unsigned short)((offset - header_length) >> 3));

            if(offset + frag_length >= fnet_ntohs(ipheader->total_length))   //TBD PFI
                frag_length = fnet_ntohs(ipheader->total_length) - offset;   //TBD PFI
            else
                new_ipheader->flags_fragment_offset |= FNET_HTONS(FNET_IP_MF);

            /* Copy the data from the original packet into the fragment.*/
            if((nb_tmp = fnet_netbuf_copy(nb_prev, offset, frag_length, FNET_NETBUF_MALLOC_NOWAIT)) == 0)
            {
                error++;
                fnet_netbuf_free_chain(nb);
                goto FRAG_END;
            }

            nb = fnet_netbuf_concat(nb, nb_tmp);

            new_ipheader->total_length = fnet_htons((unsigned short)nb->total_length);   //TBD PFI

            new_ipheader->checksum = 0;
            new_ipheader->checksum = fnet_checksum(nb, new_header_length); /* IP checksum.*/

            *nb_next_ptr = nb;
            nb_next_ptr = &nb->next_chain;
        }

        /* Update the first fragment.*/
        nb = nb_prev;
        fnet_netbuf_trim(&nb, header_length + first_frag_length - fnet_ntohs(ipheader->total_length)); //TBD PFI
        ipheader->total_length = fnet_htons((unsigned short)nb->total_length);
        ipheader->flags_fragment_offset |= FNET_HTONS(FNET_IP_MF);
        ipheader->checksum = 0;
        ipheader->checksum = fnet_checksum(nb_header, FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) << 2); /* IP checksum*/

FRAG_END:
        for (nb = nb_prev; nb; nb = nb_prev)    /* Send each fragment.*/
        {
            nb_prev = nb->next_chain;
            nb->next_chain = 0;

            if(error == 0)
            {
                fnet_ip_trace("TX", nb->data_ptr); /* Print IP header. */
                netif->output(netif, dest_ip, nb, do_not_route);
            }
            else
                fnet_netbuf_free_chain(nb);
        }

#else

        fnet_netbuf_free_chain(nb); /* Discard datagram.*/
        return (FNET_ERR_MSGSIZE);

#endif  /* FNET_CFG_IP_FRAGMENTATION */

    }
    else
    {
        ipheader->checksum = 0;
        ipheader->checksum = fnet_checksum(nb_header, FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) << 2); /* IP checksum*/

        netif->output(netif, dest_ip, nb, do_not_route);
    }

    return (FNET_OK);
}


/************************************************************************
* NAME: fnet_ip_input
*
* DESCRIPTION: IP input function.
*************************************************************************/
void fnet_ip_input( fnet_netif_t *netif, fnet_netbuf_t *nb )
{
    if(netif && nb)
    {

        if(fnet_ip_queue_append(&ip_queue, netif, nb) != FNET_OK)
        {
            fnet_netbuf_free_chain(nb);
            return;
        }

        /* Initiate S/W Interrupt*/
        fnet_isr_generate_irq(FNET_IP_SW_VECTOR_NUMBER);
         
    }
}

/************************************************************************
* NAME: fnet_ip_input_low
*
* DESCRIPTION: This function performs handling of incoming datagrams.
*************************************************************************/
void fnet_ip_input_low( void )
{
    fnet_ip_header_t *hdr;
    fnet_netbuf_t *options;
    fnet_netbuf_t *icmp_data;
    fnet_netbuf_t *tmp_nb;
    unsigned char icmp_type;
    unsigned char icmp_code;
    fnet_prot_if_t *protocol;

    fnet_netif_t *netif;
    fnet_netbuf_t *nb;

    fnet_isr_lock();
 
    while((nb = fnet_ip_queue_read(&ip_queue, &netif)) != 0)
    {
        nb->next_chain = 0;

        if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_ip_header_t), FNET_NETBUF_MALLOC_NOWAIT))
               == 0) /* The header must reside in contiguous area of memory. */
        {
            fnet_netbuf_free_chain(nb);
            continue;
        }

        nb = tmp_nb;

        hdr = nb->data_ptr;
        
        fnet_ip_trace("RX", hdr); /* Print IP header. */

        if((nb->total_length >= fnet_ntohs(hdr->total_length))                                  /* Check the amount of data*/
        && (nb->total_length >= sizeof(fnet_ip_header_t)) && (FNET_IP_HEADER_GET_VERSION(hdr) == 4)    /* Check the IP Version*/
        && ((FNET_IP_HEADER_GET_HEADER_LENGTH(hdr) << 2) >= sizeof(fnet_ip_header_t))                  /* Check the IP header length*/
        && (fnet_checksum(nb, (FNET_IP_HEADER_GET_HEADER_LENGTH(hdr) << 2)) == 0)                      /* Checksum*/
        && ((hdr->desination_addr == netif->addr.address)                                     //TBD PFI
                || fnet_ip_addr_is_broadcast(hdr->desination_addr, &(netif->addr))) /* It is final destination*/ //TBD PFI
        )
        {
            if(nb->total_length > fnet_ntohs(hdr->total_length)) //TBD PFI
            {
                fnet_netbuf_trim(&nb, (int)(fnet_ntohs(hdr->total_length)            //TBD PFI
                                                - nb->total_length)); /* logical size and the physical size of the packet should be the same.*/
            }

            /* Options handling.*/
            if(fnet_ip_options_do(netif, &nb))
                continue;


            /* Reassembly.*/
            if(hdr->flags_fragment_offset & ~FNET_HTONS(FNET_IP_DF)) /* the MF bit or fragment offset is nonzero.*/
            {
#if FNET_CFG_IP_FRAGMENTATION

                if((nb = fnet_ip_reassembly(&nb)) == 0)
                    continue;

                hdr = nb->data_ptr;

#else

                fnet_netbuf_free_chain(nb);
                continue;

#endif

            }

            if(nb->total_length > FNET_IP_MAX_PACKET)
            {
                fnet_netbuf_free_chain(nb); /* Discard datagram */
                continue;
            }

            icmp_data = fnet_netbuf_copy(nb, 0, ((FNET_IP_HEADER_GET_HEADER_LENGTH(hdr) << 2) + 8), FNET_NETBUF_MALLOC_NOWAIT);
            icmp_type = 0;
            icmp_code = 0;
            options = fnet_netbuf_copy(nb,
                                       sizeof(fnet_ip_header_t),
                                       (int)((FNET_IP_HEADER_GET_HEADER_LENGTH(hdr) << 2) - sizeof(fnet_ip_header_t)),
                                       FNET_NETBUF_MALLOC_NOWAIT);
            /* Send to upper layers. */
            fnet_netbuf_trim(&nb, (FNET_IP_HEADER_GET_HEADER_LENGTH(hdr) << 2));

            if(hdr->protocol == FNET_IP_PROTOCOL_ICMP) /* ICMP:rx */
            {
                fnet_icmp_input(netif, hdr->source_addr, hdr->desination_addr, nb, options);   //TBD PFI
            }
            else
            {
                if((protocol = fnet_prot_find(AF_INET, 0, hdr->protocol)) != 0)
                {
                     
                    if(protocol->prot_input(netif, hdr->source_addr, hdr->desination_addr, nb, options)  //TBD PFI
                           == FNET_ERR)
                    {
                        icmp_type =
                            FNET_ICMP_UNREACHABLE; /* No match was found, send ICMP destination port unreachable.*/
                        icmp_code = FNET_ICMP_UNREACHABLE_PORT;
                    }
                    /* After that nb may point to wrong place. Do not use it.*/
                    
                }
                else                                   /* No protocol found.*/
                {
                    icmp_type = FNET_ICMP_UNREACHABLE; /* Send ICMP destination protocol unreachable.*/
                    icmp_code = FNET_ICMP_UNREACHABLE_PROTOCOL;

                    fnet_netbuf_free_chain(options);
                    fnet_netbuf_free_chain(nb);
                }
            }

            if(icmp_type)
            {
                fnet_icmp_error(netif, icmp_type, icmp_code, icmp_data);
            }
            else
                fnet_netbuf_free_chain(icmp_data);
        }
        else
        {
            fnet_netbuf_free_chain(nb);
        }
    } //while
    
    fnet_isr_unlock();
}


/************************************************************************
* NAME: fnet_ip_reassembly
*
* DESCRIPTION: This function attempts to assemble a complete datagram.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

fnet_netbuf_t *fnet_ip_reassembly( fnet_netbuf_t ** nb_ptr )
{
    fnet_ip_frag_list_t *frag_list_ptr;
    fnet_ip_frag_header_t *frag_ptr, *cur_frag_ptr;
    fnet_netbuf_t *nb = *nb_ptr;
    fnet_netbuf_t *tmp_nb;
    fnet_ip_header_t *iphdr;
    int i;
    int offset;

    if((tmp_nb = fnet_netbuf_pullup(nb, (int)nb->total_length, FNET_NETBUF_MALLOC_NOWAIT))
           == 0) /* For this algorithm the all datagram must reside in contiguous area of memory.*/
    {
        goto DROP_FRAG;
    }


    *nb_ptr = tmp_nb;
    nb = tmp_nb;
    
    iphdr = nb->data_ptr;

    /* Liner search of the list to locate the appropriate datagram for the current fragment.*/
    for (frag_list_ptr = ip_frag_list_head; frag_list_ptr != 0; frag_list_ptr = frag_list_ptr->next)
    {
        if((frag_list_ptr->id == iphdr->id) && (frag_list_ptr->protocol == iphdr->protocol)
               && (frag_list_ptr->source_addr == iphdr->source_addr)
               && (frag_list_ptr->desination_addr == iphdr->desination_addr))
            break;
    }

    
    cur_frag_ptr = (fnet_ip_frag_header_t *)iphdr;

    /* Exclude the standard IP header and options.*/
    fnet_netbuf_trim(&nb, FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2); //TBD PFI
    cur_frag_ptr->total_length = (unsigned short)(fnet_ntohs(cur_frag_ptr->total_length) - (FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2)); /* Host endian.*/ //TBD PFI


    if(frag_list_ptr == 0)                                                  /* The first fragment of the new datagram.*/
    {
        if((frag_list_ptr = fnet_malloc(sizeof(fnet_ip_frag_list_t))) == 0) /* Create list.*/
            goto DROP_FRAG;

        fnet_ip_frag_list_add(&ip_frag_list_head, frag_list_ptr);

        frag_list_ptr->ttl = FNET_IP_FRAG_TTL;
        frag_list_ptr->id = iphdr->id;
        frag_list_ptr->protocol = iphdr->protocol;
        frag_list_ptr->source_addr = iphdr->source_addr;
        frag_list_ptr->desination_addr = iphdr->desination_addr;
        frag_list_ptr->frag_ptr = 0;
        frag_ptr = 0;

        if(iphdr->flags_fragment_offset & FNET_HTONS(FNET_IP_MF))
            cur_frag_ptr->mf |= 1;

        cur_frag_ptr->offset = (unsigned short)(fnet_ntohs(cur_frag_ptr->offset) * 8); /* Convert offset to bytes (Host endian).*/ //TBD PFI <<3
        
       
        cur_frag_ptr->nb = nb;
    }
    else
    {
        if(iphdr->flags_fragment_offset & FNET_HTONS(FNET_IP_MF))
            cur_frag_ptr->mf |= 1;

        cur_frag_ptr->offset = (unsigned short)(fnet_ntohs(cur_frag_ptr->offset) * 8); /* Convert offset to bytes.*/ //TBD PFI <<3

        cur_frag_ptr->nb = nb;

        /* Find position in reassembly list.*/
        frag_ptr = frag_list_ptr->frag_ptr;
        do
        {
            if(frag_ptr->offset > cur_frag_ptr->offset)
                break;

            frag_ptr = frag_ptr->next;
        } 
        while (frag_ptr != frag_list_ptr->frag_ptr);
        
        /* Trims or discards icoming fragments.*/
        if(frag_ptr != frag_list_ptr->frag_ptr)
        {
            if((i = frag_ptr->prev->offset + frag_ptr->prev->total_length - cur_frag_ptr->prev->offset) != 0)
            {
                if(i > cur_frag_ptr->total_length)
                    goto DROP_FRAG;

                fnet_netbuf_trim(&nb, i);
                cur_frag_ptr->total_length -= i;
                cur_frag_ptr->offset += i;
            }
        }
        
        /* Trims or discards existing fragments.*/
        while((frag_ptr != frag_list_ptr->frag_ptr)
                  && ((cur_frag_ptr->offset + cur_frag_ptr->total_length) > frag_ptr->offset))
        {
            i = (cur_frag_ptr->offset + cur_frag_ptr->total_length) - frag_ptr->offset;

            if(i < frag_ptr->total_length)
            {
                frag_ptr->total_length -= i;
                frag_ptr->offset += i;
                fnet_netbuf_trim(&(frag_ptr->nb), i);
                break;
            }

            frag_ptr = frag_ptr->next;
            fnet_netbuf_free_chain(frag_ptr->prev->nb);
            fnet_ip_frag_del(&frag_list_ptr->frag_ptr, frag_ptr->prev);
        }
    }

    /* Insert fragment to the list.*/
    fnet_ip_frag_add(&(frag_list_ptr->frag_ptr), cur_frag_ptr, frag_ptr->prev);

    offset = 0;
    frag_ptr = frag_list_ptr->frag_ptr;

    do
    {
        if(frag_ptr->offset != offset)
            return (0);

        offset += frag_ptr->total_length;
        frag_ptr = frag_ptr->next;
    } while (frag_ptr != frag_list_ptr->frag_ptr);

    if(frag_ptr->prev->mf & 1)
        return (0);

    /* Reconstruct datagram.*/
    frag_ptr = frag_list_ptr->frag_ptr;
    nb = frag_ptr->nb;
    frag_ptr = frag_ptr->next;

    while(frag_ptr != frag_list_ptr->frag_ptr)
    {
        nb = fnet_netbuf_concat(nb, frag_ptr->nb);

        frag_ptr = frag_ptr->next;
    }

    /* Reconstruct datagram header.*/
    iphdr = (fnet_ip_header_t *)frag_list_ptr->frag_ptr;
    nb->total_length += FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2;
    nb->length += FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2;
    nb->data_ptr = (unsigned char *)nb->data_ptr - (FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2);

    iphdr->total_length = fnet_htons((unsigned short)nb->total_length);

    iphdr->source_addr = frag_list_ptr->source_addr;
    iphdr->desination_addr = frag_list_ptr->desination_addr;
    iphdr->protocol = frag_list_ptr->protocol;
    iphdr->tos &= ~1;

    fnet_ip_frag_list_del(&ip_frag_list_head, frag_list_ptr);
    fnet_free(frag_list_ptr);

    return (nb);

DROP_FRAG:
    fnet_netbuf_free_chain(nb);

    return (0);
}

#endif /* FNET_CFG_IP_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip_timer
*
* DESCRIPTION: IP timer function.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

void fnet_ip_timer( void )
{
    fnet_ip_frag_list_t *frag_list_ptr, *tmp_frag_list_ptr;

    fnet_isr_lock();
    frag_list_ptr = ip_frag_list_head;

    while(frag_list_ptr != 0)
    {
        frag_list_ptr->ttl--;

        if(frag_list_ptr->ttl == 0)
        {
            tmp_frag_list_ptr = frag_list_ptr->next;
            fnet_ip_frag_list_free(frag_list_ptr);
            frag_list_ptr = tmp_frag_list_ptr;
        }
        else
            frag_list_ptr = frag_list_ptr->next;
    }

    fnet_isr_unlock();
}

#endif

/************************************************************************
* NAME: fnet_ip_drain
*
* DESCRIPTION: This function tries to free not critical parts 
*              of memory occupied by the IP module.
*************************************************************************/

void fnet_ip_drain( void )
{
    fnet_isr_lock();

#if FNET_CFG_IP_FRAGMENTATION

    while(((volatile fnet_ip_frag_list_t *)ip_frag_list_head) != 0)
    {
        fnet_ip_frag_list_free(ip_frag_list_head);
    }

#endif

    while(((volatile fnet_netbuf_t *)ip_queue.head) != 0)
    {
        fnet_netbuf_del_chain(&ip_queue.head, ip_queue.head);
    }

    ip_queue.count = 0;

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_ip_frag_list_free
*
* DESCRIPTION: This function frees list of datagram fragments.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

void fnet_ip_frag_list_free( fnet_ip_frag_list_t *list )
{
    fnet_netbuf_t *nb;

    fnet_isr_lock();

    if(list)
    {
        while((volatile fnet_ip_frag_header_t *)(list->frag_ptr) != 0)
        {
            nb = list->frag_ptr->nb;
            fnet_ip_frag_del(&(list->frag_ptr), list->frag_ptr);
            fnet_netbuf_free_chain(nb);
        }

        fnet_ip_frag_list_del(&ip_frag_list_head, list);
        fnet_free(list);
    }

    fnet_isr_unlock();
}

#endif /* FNET_CFG_IP_FRAGMENTATION */
/************************************************************************
* NAME: fnet_ip_frag_list_add
*
* DESCRIPTION: Adds frag list to the general frag list.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

void fnet_ip_frag_list_add( fnet_ip_frag_list_t ** head, fnet_ip_frag_list_t *fl )
{
    fl->next = *head;

    if(fl->next != 0)
        fl->next->prev = fl;

    fl->prev = 0;
    *head = fl;
}

#endif /* FNET_CFG_IP_FRAGMENTATION */
/************************************************************************
* NAME: fnet_ip_frag_list_del
*
* DESCRIPTION: Deletes frag list from the general frag list.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

void fnet_ip_frag_list_del( fnet_ip_frag_list_t ** head, fnet_ip_frag_list_t *fl )
{
    if(fl->prev == 0)
        *head=fl->next;
    else
        fl->prev->next = fl->next;

    if(fl->next != 0)
        fl->next->prev = fl->prev;
}

#endif /* FNET_CFG_IP_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip_frag_add
*
* DESCRIPTION: Adds frag to the frag list.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

void fnet_ip_frag_add( fnet_ip_frag_header_t ** head, fnet_ip_frag_header_t *frag,
                       fnet_ip_frag_header_t *frag_prev )
{
    if(frag_prev && ( *head))
    {
        frag->next = frag_prev->next;
        frag->prev = frag_prev;
        frag_prev->next->prev = frag;
        frag_prev->next = frag;

        if((*head)->offset > frag->offset)
        {
            *head = frag;
        }
    }
    else
    {
        frag->next = frag;
        frag->prev = frag;
        *head = frag;
    }
}

#endif /* FNET_CFG_IP_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip_frag_del
*
* DESCRIPTION: Deletes frag from the frag list.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

void fnet_ip_frag_del( fnet_ip_frag_header_t ** head, fnet_ip_frag_header_t *frag )
{
    if(frag->prev == frag)
        *head=0;
    else
    {
        frag->prev->next = frag->next;
        frag->next->prev = frag->prev;

        if(*head == frag)
            *head=frag->next;
    }
}

#endif /* FNET_CFG_IP_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip_options_copy
*
* DESCRIPTION: Copies IP options to frag datagram.
*************************************************************************/
#if FNET_CFG_IP_FRAGMENTATION

int fnet_ip_options_copy( fnet_ip_header_t *ipheader, fnet_ip_header_t *ipheader_new )
{
    unsigned char *option_ptr, *option_ptr_new;
    int cnt, len = 0;

    option_ptr = (unsigned char *)(ipheader + 1);
    option_ptr_new = (unsigned char *)(ipheader_new + 1);

    for (cnt = (int)((FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) << 2) - sizeof(fnet_ip_header_t));
        cnt > 0; cnt -= len, option_ptr += len)
    {
        if(option_ptr[IPOPT_TYPE] == IPOPT_EOL) /* end of option list */
            break;

        if(option_ptr[IPOPT_TYPE] == IPOPT_NOP) /* no operation */
        {
            *option_ptr_new++ = IPOPT_NOP;
            len = 1;
            continue;
        }
        else
            len = option_ptr[IPOPT_LENGTH];

        if(len > cnt)
            len = cnt;

        if(IPOPT_COPIED(option_ptr[IPOPT_TYPE]))
        {
            fnet_memcpy(option_ptr, option_ptr_new, (unsigned int)len);
            option_ptr_new += len;
        }
    }

    while(len & 3)
      option_ptr_new[len++] = IPOPT_EOL;

    return (len);
}

#endif /* FNET_CFG_IP_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip_options_do
*
* DESCRIPTION: Handles the incoming IP options.
*
* RETURNS: 0=OK. The datagram needs further processing
*          1=The The datagram is discarded because of an error
*************************************************************************/

int fnet_ip_options_do( fnet_netif_t *netif, fnet_netbuf_t ** nb_ptr )
{
    int cnt;
    unsigned char *option_ptr;
    int len;
    unsigned char type = FNET_ICMP_PARAMPROB;
    unsigned char code;
    fnet_ip_timestamp_t *ipts;
    fnet_netbuf_t *tmp_nb;
    fnet_netbuf_t *nb = *nb_ptr;
    fnet_ip_header_t *iphdr = nb->data_ptr;

    if((nb) && (nb->total_length >= (FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2))
           && ((FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2) > sizeof(fnet_ip_header_t)))
    {
        if((tmp_nb = fnet_netbuf_pullup(nb, (FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2), FNET_NETBUF_MALLOC_NOWAIT))
               == 0) /* The header must reside in contiguous area of memory. */
        {
            return (0);
        }

        *nb_ptr = tmp_nb;
        nb = tmp_nb;

        option_ptr = (unsigned char *)iphdr + sizeof(fnet_ip_header_t);

        for (cnt = (int)((FNET_IP_HEADER_GET_HEADER_LENGTH(iphdr) << 2) - sizeof(fnet_ip_header_t));
            cnt > 0; cnt -= len, option_ptr += len)
        {
            if(option_ptr[IPOPT_TYPE] == IPOPT_EOL) /* end of option list */
                break;

            if(option_ptr[IPOPT_TYPE] == IPOPT_NOP) /* no operation */
            {
                len = 1;
                continue;
            }
            else
            {
                len = option_ptr[IPOPT_LENGTH];

                if((len <= IPOPT_LENGTH) || (len > cnt))
                {
                    code = (unsigned char)(&option_ptr[IPOPT_LENGTH] - (unsigned char *)iphdr);
                    goto BAD_OPTION;
                }

                /* Option processing.*/
                switch(option_ptr[IPOPT_TYPE])
                {
                    case IPOPT_TS:         /* Timestamp option*/
                      ipts = (fnet_ip_timestamp_t *)option_ptr;

                      if(ipts->length < 5) /* The minimum size of the timestamp option*/
                      {
                          code = (unsigned char)(&(ipts->length)-(unsigned char *)iphdr);
                          goto BAD_OPTION;
                      }

                      switch(FNET_IP_TIMESTAMP_GET_FLAG(ipts))
                      {
                          case IPOPT_TS_FLAG_TSONLY: /* Timestamp only.*/
                            if((ipts->pointer + sizeof(long)) > ipts->length)
                            {
                                 FNET_IP_TIMESTAMP_SET_OVERFLOW(ipts,FNET_IP_TIMESTAMP_GET_OVERFLOW(ipts)+1);
                                if(FNET_IP_TIMESTAMP_GET_OVERFLOW(ipts) == 0)
                                {
                                    code = (unsigned char)(&(ipts->pointer)+1 - (unsigned char *)iphdr);
                                    goto BAD_OPTION;
                                }
                            }

                            break;

                          case IPOPT_TS_FLAG_TSANDADDR: /* Timestamp and address.*/
                            if((ipts->pointer + sizeof(fnet_ip_addr_t) + sizeof(long)) > ipts->length)
                            {
                                FNET_IP_TIMESTAMP_SET_OVERFLOW(ipts,FNET_IP_TIMESTAMP_GET_OVERFLOW(ipts)+1);
                                if(FNET_IP_TIMESTAMP_GET_OVERFLOW(ipts) == 0)
                                {
                                    code = (unsigned char)(&(ipts->pointer)+1 - (unsigned char *)iphdr);
                                    goto BAD_OPTION;
                                }
                            }

                            ipts->timestamp.record.address = netif->addr.address;
                            ipts->pointer += sizeof(fnet_ip_addr_t);
                            break;

                          case IPOPT_TS_FLAG_TSPRESPEC: /* Timestamp at prespecified address.*/
                            if((ipts->pointer + sizeof(fnet_ip_addr_t) + sizeof(long)) > ipts->length)
                            {
                                FNET_IP_TIMESTAMP_SET_OVERFLOW(ipts,FNET_IP_TIMESTAMP_GET_OVERFLOW(ipts)+1);
                                if(FNET_IP_TIMESTAMP_GET_OVERFLOW(ipts) == 0)
                                {
                                    code = (unsigned char)(&(ipts->pointer)+1 - (unsigned char *)iphdr);
                                    goto BAD_OPTION;
                                }
                            }

                            if(ipts->timestamp.record.address != netif->addr.address)
                                continue;

                            ipts->pointer += sizeof(fnet_ip_addr_t);
                            break;

                          default:
                            code = (unsigned char)(&(ipts->pointer)+1 - (unsigned char *)iphdr);
                            goto BAD_OPTION;
                      }

                      /* A nonstandard time must have the high-order bit of the timestamp field set.*/
                      *(unsigned long *)(option_ptr + ipts->pointer - 1)
                           = fnet_htonl(((fnet_timer_ticks() * FNET_TIMER_PERIOD_MS) % 24 * 60 * 60 * 1000)
                                 || 0x80000000); /* The number of milliseconds since first startup. */

                      ipts->pointer += sizeof(long);

                      break;
                }
            }
        }
    }

    return (0);

    BAD_OPTION:
    fnet_icmp_error(netif, type, code, nb);

    return (1);
}

/************************************************************************
* NAME: fnet_ip_options_insert
*
* DESCRIPTION: Inserts IP options into outgoing datagram.
*************************************************************************/
fnet_netbuf_t *fnet_ip_options_insert( fnet_netbuf_t *options, fnet_netbuf_t *nb )
{
    if((options) && (nb))
    {
        if((options->total_length + nb->total_length + sizeof(fnet_ip_header_t)) > FNET_IP_MAX_PACKET)
            fnet_netbuf_free_chain(options); /* discard options */
        else
        {
            nb = fnet_netbuf_concat(options, nb);
        }
    }

    return (nb);
}
/************************************************************************
* NAME: fnet_ip_options_compile
*
* DESCRIPTION: Handles IP options entered by user.
*************************************************************************/
int fnet_ip_options_compile( fnet_socket_t *sk, char *optval, int optlen )
{
    fnet_netbuf_t *new_option = 0;
    int cnt;
    unsigned char *option_ptr;
    int len;

    if((optval) && (optlen))
    {
        if(optlen > 40 || optlen < 0)
            return (FNET_ERR);
        else
        {
            if((new_option = fnet_netbuf_from_buf(optval, ((optlen + 3) & ~3), FNET_NETBUF_TYPE_OPTION,
                                                      FNET_NETBUF_MALLOC_NOWAIT)) == 0)
            {
                return (FNET_ERR);
            }

            while(optlen & 3)
              ((unsigned char *)(new_option->data_ptr))[optlen++] = IPOPT_EOL;

            option_ptr = new_option->data_ptr;

            for (cnt = (int)new_option->total_length; cnt > 0; cnt -= len, option_ptr += len)
            {
                if(option_ptr[IPOPT_TYPE] == IPOPT_EOL) /* End of option list. */
                    break;

                if(option_ptr[IPOPT_TYPE] == IPOPT_NOP) /* No operation */
                {
                    len = 1;
                    continue;
                }
                else
                {
                    len = option_ptr[IPOPT_LENGTH];

                    if((len <= IPOPT_LENGTH) || (len > cnt) || (option_ptr[IPOPT_OFFSET] < IPOPT_OFFSET_MIN))
                    {
                        fnet_netbuf_free_chain(new_option);
                        return (FNET_ERR);
                    }
                }
            } //for
        }     //else
    }

    if(sk->options.ip_opt.options)
    {
        fnet_netbuf_free_chain(sk->options.ip_opt.options); /* Dell the old options */
        sk->options.ip_opt.options = 0;
    }

    if(new_option)
        sk->options.ip_opt.options = new_option;

    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_ip_setsockopt
*
* DESCRIPTION: This function sets the value of IP socket option. 
*************************************************************************/
int fnet_ip_setsockopt( fnet_socket_t *sock, int level, int optname, char *optval, int optlen )
{
    int error;

    if((level == IPPROTO_IP) && (optval) && (optlen))
    {
        switch(optname)      /* Socket options processing. */
        {
            case IP_OPTIONS: /* Set IP options to be included in outgoing datagrams */
              if(fnet_ip_options_compile(sock, optval, optlen) == FNET_ERR)
              {
                  error = FNET_ERR_INVAL;
                  goto ERROR_SOCK;
              }

            case IP_TOS: /* Set IP TOS for outgoing datagrams. */
              if(optlen != sizeof(int))
              {
                  error = FNET_ERR_INVAL;
                  goto ERROR_SOCK;
              }

              sock->options.ip_opt.tos = (unsigned char) (*((int *)(optval)));
              break;

            case IP_TTL: /* Set IP TTL for outgoing datagrams. */
              if(optlen != sizeof(int))
              {
                  error = FNET_ERR_INVAL;
                  goto ERROR_SOCK;
              }

              sock->options.ip_opt.ttl = (unsigned char) (*((int *)(optval)));
              break;

            default:
              error = FNET_ERR_NOPROTOOPT; /* The option is unknown or unsupported. */
              goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_INVAL; /* Invalid argument.*/
        goto ERROR_SOCK;
    }

    return (FNET_OK);

    ERROR_SOCK:
    sock->options.error = error;

    fnet_error_set(error);
    return (SOCKET_ERROR);
}
/************************************************************************
* NAME: fnet_ip_getsockopt
*
* DESCRIPTION: This function retrieves the current value 
*              of IP socket option.
*************************************************************************/
int fnet_ip_getsockopt( fnet_socket_t *sock, int level, int optname, char *optval, int *optlen )
{
    int error;

    if((level == IPPROTO_IP) && (optval) && (optlen) && ( *optlen))
    {
        switch(optname)      /* Socket options processing. */
        {
            case IP_OPTIONS: /* Set IP options to be included in outgoing datagrams */
              if(sock->options.ip_opt.options)
              {
                  if(*optlen < sock->options.ip_opt.options->total_length)
                  {
                      error = FNET_ERR_INVAL;
                      goto ERROR_SOCK;
                  }

                  fnet_netbuf_to_buf(sock->options.ip_opt.options, 0, FNET_NETBUF_COPYALL, optval);
                  *optlen = (int)sock->options.ip_opt.options->total_length;
              }
              else
              {
                  *optlen = 0;
              }

              break;

            case IP_RECVOPTS:
            {
                fnet_netbuf_t *nb;
                fnet_socket_buffer_addr_t *nb_opt;

                if((nb = sock->receive_buffer.net_buf_chain) != 0)
                {
                    if((nb->type != FNET_NETBUF_TYPE_ADDRESS))
                    {
                        error = FNET_ERR_NOPROTOOPT; /* The option is unknown or unsupported. */
                        goto ERROR_SOCK;
                    }

                    nb_opt = nb->data_ptr;

                    if(nb_opt->ip_options)
                    {
                        if(*optlen < nb_opt->ip_options->total_length)
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        fnet_netbuf_to_buf(nb_opt->ip_options, 0, FNET_NETBUF_COPYALL, optval);
                        *optlen = (int)nb_opt->ip_options->total_length;
                    }
                    else
                    {
                        *optlen = 0;
                    }
                }
                else
                {
                    *optlen = 0;
                }
            }
              break;

            case IP_TOS: /* Set IP TOS for outgoing datagrams.*/
              if(*optlen < sizeof(int))
              {
                  error = FNET_ERR_INVAL;
                  goto ERROR_SOCK;
              }

              *((int*)optval) = sock->options.ip_opt.tos;
              *optlen = sizeof(int);
              break;

            case IP_TTL: /* Set IP TTL for outgoing datagrams.*/
              if(*optlen < sizeof(int))
              {
                  error = FNET_ERR_INVAL;
                  goto ERROR_SOCK;
              }

              *((int*)optval) = (char)sock->options.ip_opt.ttl;
              *optlen = sizeof(int);
              break;

            default:
              error = FNET_ERR_NOPROTOOPT; /* The option is unknown or unsupported.*/
              goto ERROR_SOCK;
        }
    }
    else
    {
        error = FNET_ERR_INVAL; /* Invalid argument.*/
        goto ERROR_SOCK;
    }

    return (FNET_OK);

    ERROR_SOCK:
    sock->options.error = error;

    fnet_error_set(error);
    return (SOCKET_ERROR);
}


/************************************************************************
* NAME: ip_append_queue
*
* DESCRIPTION: Appends IP input queue.
*************************************************************************/
int fnet_ip_queue_append( fnet_ip_queue_t *queue, fnet_netif_t *netif, fnet_netbuf_t *nb )
{
    fnet_netbuf_t *nb_netif;

    fnet_isr_lock();

    if((nb->total_length + queue->count) > FNET_IP_QUEUE_COUNT_MAX)
    {
        fnet_isr_unlock();
        return FNET_ERR;
    }

    if((nb_netif = fnet_netbuf_new(sizeof(fnet_netif_t *), FNET_NETBUF_TYPE_ADDRESS,
                                       FNET_NETBUF_MALLOC_NOWAIT)) == 0)
    {
        fnet_isr_unlock();
        return FNET_ERR;
    }

    nb_netif->data_ptr = (void *)netif;

    queue->count += nb->total_length;

    nb = fnet_netbuf_concat(nb_netif, nb);
    fnet_netbuf_add_chain(&queue->head, nb);
    fnet_isr_unlock();

    return FNET_OK;
}

/************************************************************************
* NAME: ip_read_queue
*
* DESCRIPTION: Reads a IP datagram from IP input queue.
*************************************************************************/
fnet_netbuf_t *fnet_ip_queue_read( fnet_ip_queue_t *queue, fnet_netif_t ** netif )
{
    fnet_netbuf_t *nb;
    fnet_netbuf_t *nb_netif;

    if((nb_netif = queue->head) != 0)
    {
        nb = nb_netif->next;

        *netif = ((fnet_netif_t *)(nb_netif->data_ptr));

        fnet_isr_lock();

        if(nb)
            queue->count -= nb->total_length;

        nb_netif->next = 0;
        fnet_netbuf_del_chain(&queue->head, nb_netif);
        fnet_isr_unlock();
    }
    else
        nb = 0;

    return nb;
}


/************************************************************************
* NAME: fnet_ip_addr_is_broadcast
*
* DESCRIPTION: Is the address is broadcast?
*************************************************************************/
int fnet_ip_addr_is_broadcast( fnet_ip_addr_t addr, fnet_netif_addr_t *netif_addr )
{
    if(addr == INADDR_BROADCAST || addr == INADDR_ANY || /* Limited broadcast */
    addr == netif_addr->netbroadcast ||                  /* Net-directed broadcast */
    addr == netif_addr->subnetbroadcast ||               /* Subnet-directed broadcast */
    addr == netif_addr->subnet)
        return 1;

    return 0;
}


//TBD ???
/*Todo path MTU discovery feature
 *Todo get MTU from a routing table, depending on destination MTU*/
unsigned long fnet_ip_maximum_packet( fnet_ip_addr_t dest_ip ) 
{
    unsigned int result;

#if FNET_CFG_IP_FRAGMENTATION == 0

    {
        fnet_netif_t *netif;

        if((netif = fnet_ip_route(dest_ip)) == 0) /* No route*/
            result = FNET_IP_MAX_PACKET;
        else
            result = netif->mtu;
    }

#else

    FNET_COMP_UNUSED_ARG(dest_ip);

    result = FNET_IP_MAX_PACKET;

#endif

    result = (result - (FNET_IP_MAX_OPTIONS + sizeof(fnet_ip_header_t))) & (~0x3L);

    return result;
}

/************************************************************************
* NAME: fnet_ip_trace
*
* DESCRIPTION: Prints an IP header. For debug needs only.
*************************************************************************/
#if FNET_CFG_DEBUG_TRACE_IP
void fnet_ip_trace(char *str,fnet_ip_header_t *ip_hdr)
{
    char ip_str[16];

    fnet_printf(FNET_SERIAL_ESC_FG_GREEN"%s", str); /* Print app-specific header.*/
    fnet_println("[IP header]"FNET_SERIAL_ESC_FG_BLACK); //TBD Add Timestamp
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(V) %2d |(HL)%2d |(TOS)     0x%02x |(L)                      %5u |",
                    FNET_IP_HEADER_GET_VERSION(ip_hdr),
                    FNET_IP_HEADER_GET_HEADER_LENGTH(ip_hdr),
                    ip_hdr->tos,
                    fnet_ntohs(ip_hdr->total_length));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Id)                     %5u |(F)%u |(Offset)            %4u |",
                    fnet_ntohs(ip_hdr->id),
                    fnet_ntohs(FNET_IP_HEADER_GET_FLAG(ip_hdr))>>15,
                    fnet_ntohs(FNET_IP_HEADER_GET_OFFSET(ip_hdr)));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(TTL)      %3u |(Proto)    "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |(Cheksum)               0x%04x |",
                    ip_hdr->ttl,
                    ip_hdr->protocol,
                    fnet_ntohs(ip_hdr->checksum));      
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Src)                                          "FNET_SERIAL_ESC_FG_BLUE"%15s"FNET_SERIAL_ESC_FG_BLACK" |",
                     inet_ntoa(*(struct in_addr *)(&ip_hdr->source_addr), ip_str));
                     //inet_ntoa(*(struct in_addr *)(&(fnet_ntohs(ip_hdr->source_addr))), ip_str));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");                    
    fnet_println("|(Dest)                                         "FNET_SERIAL_ESC_FG_BLUE"%15s"FNET_SERIAL_ESC_FG_BLACK" |",
                    inet_ntoa(*(struct in_addr *)(&ip_hdr->desination_addr), ip_str));                    
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    
}
#endif /* FNET_CFG_DEBUG_TRACE_IP */

