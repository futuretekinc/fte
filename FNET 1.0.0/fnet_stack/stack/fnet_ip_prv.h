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
* @file fnet_ip_prv.h
*
* @author Andrey Butok
*
* @date May-20-2011
*
* @version 0.1.12.0
*
* @brief Private. IP protocol API.
*
***************************************************************************/

#ifndef _FNET_IP_PRV_H_

#define _FNET_IP_PRV_H_

#include "fnet.h"

#include "fnet_ip.h"
#include "fnet_netif.h"
#include "fnet_netif_prv.h"

/************************************************************************
*    Definitions of five different classes.
*************************************************************************/
#define FNET_IP_CLASS_A(i)  (((fnet_ip_addr_t)(i) & FNET_HTONL(0x80000000)) == 0)
#define FNET_IP_CLASS_A_NET FNET_HTONL(0xff000000)

#define FNET_IP_CLASS_B(i)  (((fnet_ip_addr_t)(i) & FNET_HTONL(0xc0000000)) == FNET_HTONL(0x80000000))
#define FNET_IP_CLASS_B_NET FNET_HTONL(0xffff0000)

#define FNET_IP_CLASS_C(i)  (((fnet_ip_addr_t)(i) & FNET_HTONL(0xe0000000)) == FNET_HTONL(0xc0000000))
#define FNET_IP_CLASS_C_NET FNET_HTONL(0xffffff00)

#define FNET_IP_CLASS_D(i)  (((fnet_ip_addr_t)(i) & FNET_HTONL(0xf0000000)) == FNET_HTONL(0xe0000000))
#define FNET_IP_CLASS_D_NET FNET_HTONL(0xf0000000)
#define FNET_IP_MULTICAST(i)    FNET_IP_CLASS_D(i)

#define FNET_IP_CLASS_E(i)  (((fnet_ip_addr_t)(i) & FNET_HTONL(0xf0000000)) == FNET_HTONL(0xf0000000))
#define FNET_IP_EXPERIMENTAL(i) FNET_IP_CLASS_E(i)
#define FNET_IP_BADCLASS(i) FNET_IP_CLASS_E(i)

#define FNET_IP_ADDR1(ipaddr) ((unsigned char)(fnet_ntohl(ipaddr) >> 24) & 0xff)
#define FNET_IP_ADDR2(ipaddr) ((unsigned char)(fnet_ntohl(ipaddr) >> 16) & 0xff)
#define FNET_IP_ADDR3(ipaddr) ((unsigned char)(fnet_ntohl(ipaddr) >> 8) & 0xff)
#define FNET_IP_ADDR4(ipaddr) ((unsigned char)(fnet_ntohl(ipaddr)) & 0xff)

#define FNET_IP_MAX_PACKET     (FNET_CFG_IP_MAX_PACKET)
#if (FNET_IP_MAX_PACKET >65535)

#undef FNET_IP_MAX_PACKET
#define FNET_IP_MAX_PACKET (65535)

#endif

#if (FNET_IP_MAX_PACKET < 200)

#undef FNET_IP_MAX_PACKET
#define FNET_IP_MAX_PACKET      (200)

#endif

#define FNET_IP_MAX_OPTIONS     (40) /* Maximum option field length */

/************************************************************************
*    Internet implementation parameters.
*************************************************************************/
#define FNET_IP_VERSION         (4)   /* IP version */
#define FNET_IP_TTL_MAX         (255) /* maximum time to live */
#define FNET_IP_TTL_DEFAULT     (64)  /* default ttl, from RFC 1340 */

/************************************************************************
*    Supported protocols.
*************************************************************************/
#define FNET_IP_PROTOCOL_ICMP   (1)
#define FNET_IP_PROTOCOL_UDP    (17)
#define FNET_IP_PROTOCOL_TCP    (6)

#define FNET_IP_DF              (0x4000)    /* dont fragment flag */
#define FNET_IP_MF              (0x2000)    /* more fragments flag */
#define FNET_IP_FLAG_MASK       (0xE000)    /* mask for fragmenting bits */
#define FNET_IP_OFFSET_MASK     (0x1fff)    /* mask for fragmenting bits */

#define FNET_IP_TIMER_PERIOD        (500)
#define FNET_IP_FRAG_TTL            (10000/FNET_IP_TIMER_PERIOD) /* TTL for fragments to complete a datagram (10sec)*/
#define FNET_IP_SW_VECTOR_NUMBER    (FNET_ISR_SW_VECTOR_NUMBER +1)

/************************************************************************
*    Timestamp option
*************************************************************************/
FNET_COMP_PACKED_BEGIN
/**************************************************************************/ /*!
 * @internal
 * @brief    timestamp.
 ******************************************************************************/
typedef struct
{
    unsigned char code;         /**< = IPOPT_TS */
    unsigned char length;       /**< The number of bytes in the option.*/
    unsigned char
        pointer;                /**< The number of bytes from the beginning of this option to the end of timestamps plus one.*/
    unsigned char overflow__flag;   /**< overflow counter & flag */
    union
    {
        unsigned long time[1];
        struct
        {
            fnet_ip_addr_t address;
            unsigned long time[1];
        } record;
    } timestamp;
} fnet_ip_timestamp_t;

#define FNET_IP_TIMESTAMP_GET_OVERFLOW(x)           ((x->overflow__flag & 0xF0)>>4)
#define FNET_IP_TIMESTAMP_SET_OVERFLOW(x, overflow) (x->overflow__flag = (unsigned char)((x->overflow__flag & 0x0F)|(((overflow)&0x0F)<<4)))
#define FNET_IP_TIMESTAMP_GET_FLAG(x)               (x->overflow__flag & 0x0F)
#define FNET_IP_TIMESTAMP_SET_FLAG(x, flag)         (x->overflow__flag = (unsigned char)((x->overflow__flag & 0xF0)|((flag)&0x0F)))


/**************************************************************************/ /*!
 * @internal
 * @brief    IP layer socket options.
 ******************************************************************************/
typedef struct
{
    fnet_netbuf_t *options;     /**< IP otions field.*/
    unsigned char ttl;          /**< TTL.*/
    unsigned char tos;          /**< TOS.*/
} fnet_ip_sockopt_t;

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of IP header.
 ******************************************************************************/
typedef struct
{
    unsigned char version__header_length;   /**< version =4 & header length (x4) (>=5)*/     
    unsigned char tos;                  /**< type of service */
    unsigned short total_length;        /**< total length */
    unsigned short id;                  /**< identification */
    unsigned short flags_fragment_offset;   /**< flags & fragment offset field (measured in 8-byte order).*/
    unsigned char ttl;                  /**< time to live */
    unsigned char protocol;             /**< protocol */
    unsigned short checksum;            /**< checksum */
    fnet_ip_addr_t source_addr;         /**< source address */
    fnet_ip_addr_t desination_addr;     /**< destination address */
} fnet_ip_header_t;

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of IP fragment header.
 ******************************************************************************/
typedef struct fnet_ip_frag_header
{
    unsigned char version__header_length;   /**< version =4 & header length (x4) (>=5)*/    
    unsigned char mf;
    unsigned short total_length;        /**< total length (Host endian)*/
    unsigned short id;                  /**< identification*/
    unsigned short offset;              /**< offset field (measured in 8-byte order). (Host endian)*/
    fnet_netbuf_t *nb;
    struct fnet_ip_frag_header *next;   /**< Pointer to the next fragment.*/
    struct fnet_ip_frag_header *prev;   /**< Pointer to the previous fragment.*/
} fnet_ip_frag_header_t;

#define FNET_IP_HEADER_GET_FLAG(x)                      (x->flags_fragment_offset & FNET_HTONS(FNET_IP_FLAG_MASK))
#define FNET_IP_HEADER_GET_OFFSET(x)                    (x->flags_fragment_offset & FNET_HTONS(FNET_IP_OFFSET_MASK))
#define FNET_IP_HEADER_GET_VERSION(x)                   ((x->version__header_length & 0xF0)>>4)
#define FNET_IP_HEADER_SET_VERSION(x, version)          (x->version__header_length = (unsigned char)((x->version__header_length & 0x0F)|(((version)&0x0F)<<4)))
#define FNET_IP_HEADER_GET_HEADER_LENGTH(x)             (x->version__header_length & 0x0F)
#define FNET_IP_HEADER_SET_HEADER_LENGTH(x, length)     (x->version__header_length = (unsigned char)((x->version__header_length & 0xF0)|((length)&0x0F)))

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of the head of each reassembly list.
 ******************************************************************************/
typedef struct fnet_ip_frag_list
{
    struct fnet_ip_frag_list *next;     /**< Pointer to the next reassembly list.*/
    struct fnet_ip_frag_list *prev;     /**< Pointer to the previous reassembly list.*/
    unsigned char ttl;                  /**< TTL for reassembly.*/
    unsigned char protocol;             /**< protocol.*/
    unsigned short id;                  /**< identification.*/
    fnet_ip_addr_t source_addr;         /**< source address.*/
    fnet_ip_addr_t desination_addr;     /**< destination address.*/
    fnet_ip_frag_header_t *frag_ptr;    /**< Pointer to the first fragment of the list.*/
} fnet_ip_frag_list_t;
FNET_COMP_PACKED_END

/************************************************************************
*     Function Prototypes
*************************************************************************/

int fnet_ip_init( void );
void fnet_ip_release( void );

int fnet_ip_addr_is_broadcast( fnet_ip_addr_t addr, fnet_netif_addr_t *net_if_addr );

int fnet_ip_output( fnet_netif_t *netif,    fnet_ip_addr_t src_ip, fnet_ip_addr_t dest_ip,
                    unsigned char protocol, unsigned char tos,     unsigned char ttl,
                    fnet_netbuf_t *nb,      int DF,                fnet_netbuf_t *options,
                    int do_not_route );

void fnet_ip_input( fnet_netif_t *netif, fnet_netbuf_t *nb );
fnet_netif_t *fnet_ip_route( fnet_ip_addr_t dest_ip );
unsigned long fnet_ip_maximum_packet( fnet_ip_addr_t dest_ip );
void fnet_ip_drain( void );

#endif
