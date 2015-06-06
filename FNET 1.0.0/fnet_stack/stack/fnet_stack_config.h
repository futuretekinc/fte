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
* @file fnet_stack_config.h
*
* @author Andrey Butok
*
* @date Jun-2-2011
*
* @version 0.1.31.0
*
* @brief Main TCP/IP stack default configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_STACK_CONFIG_H_

#define _FNET_STACK_CONFIG_H_

/*! @addtogroup fnet_stack_config */
/*! @{ */


/****************************************************************************
 * Supported Data-link Interfaces.
 ****************************************************************************/

/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH
 * @brief    Ethernet interface:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_ETH
/*! @endcond */
#define FNET_CFG_ETH                    (1)
/*! @cond */
#endif
/*! @endcond */


/*! @cond */
#if FNET_CFG_ETH
/*! @endcond */
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH_IP_ADDR
 * @brief    Defines the default IP address for the Ethernet interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_address().
 * @showinitializer
 ******************************************************************************/
    /*! @cond */
    #ifndef FNET_CFG_ETH_IP_ADDR
    /*! @endcond */
    #define FNET_CFG_ETH_IP_ADDR        (FNET_GEN_ADDR(192, 168, 0, 100))
    /*! @cond */
    #endif
    /*! @endcond */
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH_IP_MASK
 * @brief    Defines the default IP Subnetmask for the Ethernet interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_netmask().
 * @showinitializer
 ******************************************************************************/
    /*! @cond */
    #ifndef FNET_CFG_ETH_IP_MASK
    /*! @endcond */
    #define FNET_CFG_ETH_IP_MASK        (FNET_GEN_ADDR(255, 255, 255, 0))
    /*! @cond */
    #endif
    /*! @endcond */
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH_IP_GW
 * @brief    Defines the default Gateway IP address for the Ethernet interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_gateway().
 * @showinitializer 
 ******************************************************************************/
    /*! @cond */
    #ifndef FNET_CFG_ETH_IP_GW
    /*! @endcond */
    #define FNET_CFG_ETH_IP_GW          (FNET_GEN_ADDR(192, 168, 0, 1))
    /*! @cond */
    #endif
    /*! @endcond */
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH_MAC_ADDR
 * @brief    Defines the default MAC address of the Ethernet interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_hw_addr().
 * @showinitializer
 ******************************************************************************/
    /*! @cond */
    #ifndef FNET_CFG_ETH_MAC_ADDR
    /*! @endcond */
    #define FNET_CFG_ETH_MAC_ADDR       ("00:04:9F:" __TIME__)
    /*! @cond */
    #endif
    /*! @endcond */
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH_MTU
 * @brief    Defines the Maximum Transmission Unit for the Ethernet interface.
 *           The largest value is 1500. The Internet Minimum MTU is 576.
 * @showinitializer 
 ******************************************************************************/
    /*! @cond */
    #ifndef FNET_CFG_ETH_MTU
    /*! @endcond */
    #define FNET_CFG_ETH_MTU            (1500)
    /*! @cond */
    #endif
    /*! @endcond */
    
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOP
 * @brief    Loopback interface:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_LOOP
/*! @endcond */
#define FNET_CFG_LOOP                   (1)
/*! @cond */
#endif
/*! @endcond */

#if FNET_CFG_LOOP
/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOP_IP_ADDR
 * @brief    Defines the IP address for the Loopback interface.
 *           By default it is set to 127.0.0.1.
 *           At runtime, it can be changed by @ref fnet_netif_set_address().
 * @showinitializer 
 ******************************************************************************/
    /*! @cond */
    #ifndef FNET_CFG_LOOP_IP_ADDR
    /*! @endcond */
    #define FNET_CFG_LOOP_IP_ADDR       (FNET_GEN_ADDR(127, 0, 0, 1))
    /*! @cond */
    #endif
    /*! @endcond */
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOP_MTU
 * @brief    Defines the Maximum Transmission Unit for the Loopback interface.
 *           By default, it is set to 1576.
 * @showinitializer 
 ******************************************************************************/
    /*! @cond */
    #ifndef FNET_CFG_LOOP_MTU
    /*! @endcond */
    #define FNET_CFG_LOOP_MTU           (1576)
    /*! @cond */
    #endif
    /*! @endcond */
    
#endif


/*****************************************************************************
 * Default Network addresses.
 *****************************************************************************/
/*! @cond */
#if FNET_CFG_ETH

    #define FNET_DEFAULT_IF             (FNET_ETH_IF)
    #define FNET_DEFAULT_IF_MTU         (FNET_CFG_ETH_MTU)

#elif FNET_CFG_LOOP

    #define FNET_DEFAULT_IF             (FNET_LOOP_IF)
    #define FNET_DEFAULT_IF_MTU         (FNET_CFG_LOOP_MTU)

#else

    #define FNET_DEFAULT_IF             (0)
    #define FNET_DEFAULT_IF_MTU         (0)

#endif
/*! @endcond */

/*****************************************************************************
*     TCP/IP stack features.
******************************************************************************/

/**************************************************************************/ /*!
 * @def      FNET_CFG_TCP
 * @brief    TCP protocol support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.@n
 *           @n
 *           You can disable it to save a substantial amount of code, if
 *           your application needs the UDP only. By default, it is enabled.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_TCP
/*! @endcond */
#define FNET_CFG_TCP                    (1)
/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def      FNET_CFG_TCP_DISCARD_OUT_OF_ORDER
 * @brief    Discarding of TCP segments that are received out of order:
 *               - @c 1 = is enabled (default value).
 *               - @c 0 = is disabled.@n
 *           @n
 *           By default, the discarding is enabled. @n But you may disable it, 
 *           for systems that have a lot of free RAM.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_TCP_DISCARD_OUT_OF_ORDER
/*! @endcond */
#define FNET_CFG_TCP_DISCARD_OUT_OF_ORDER                    (1)
/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def      FNET_CFG_UDP
 * @brief    UDP protocol support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.@n
 *           @n
 *           You can disable it to save some amount of code, if your
 *           application needs the TCP only. By default, it is enabled.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_UDP
/*! @endcond */
#define FNET_CFG_UDP                    (1)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_UDP_CHECKSUM
 * @brief    UDP checksum:
 *               - @c 1 = The UDP checksum will be generated for transmitted
 *                        datagrams and verified on received UDP datagrams.
 *               - @c 0 = The UDP checksum will not be generated for transmitted
 *                        datagrams and won't be verified on received UDP datagrams.
 *           @n@n
 *           You can disable it to speed the UDP applications up.
 *           By default, it is enabled.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_UDP_CHECKSUM
/*! @endcond */
#define FNET_CFG_UDP_CHECKSUM           (1)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_IP_FRAGMENTATION
 * @brief    IP fragmentation:
 *               - @c 1 = is enabled. The IP will attempt
 *                        to reassemble the IP packet fragments and will be able to
 *                        generate fragmented IP packets.
 *               - @c 0 = is disabled. The IP will
 *                        silently discard the fragmented IP packets..
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_IP_FRAGMENTATION
/*! @endcond */
#define FNET_CFG_IP_FRAGMENTATION       (1)
/*! @cond */
#endif
/*! @endcond */

/*****************************************************************************
* 	TCP/IP stack parameters.
******************************************************************************/

/**************************************************************************/ /*!
 * @def      FNET_CFG_HEAP_SIZE
 * @brief    Size of the internal static heap buffer.
 *           It is used only if @ref fnet_init_static() was
 *           called for FNET initialization.
 * @hideinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_HEAP_SIZE
/*! @endcond */
#define FNET_CFG_HEAP_SIZE              (50 * 1024)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_MAX
 * @brief    Maximum number of sockets that can exist at the same time.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_SOCKET_MAX
/*! @endcond */
#define FNET_CFG_SOCKET_MAX             (10)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_TCP_MSS
 * @brief    The default value of the @ref TCP_MSS option 
 *           (TCP Maximum Segment Size).@n
 * The TCP Maximum Segment Size (MSS) defines the maximum amount 
 * of data that a host is willing to accept in a single TCP segment.@n
 * This Maximum Segment Size (MSS) announcement is sent from the
 * data receiver to the data sender and says "I can accept TCP segments
 * up to size X". The size (X) may be larger or smaller than the
 * default.@n
 * The MSS counts only data octets in the segment, it does not count the
 * TCP header or the IP header.@n
 * This option can be set to:
 *     - @c 0 = This is the default value. The selection of the MSS is 
 *              automatic and is based on the MTU of the outgoing 
 *              interface minus 40 (does not include 
 *              the 20 byte IP header and the 20 byte TCP header).@n
 *              It is done to assist in avoiding of IP fragmentation 
 *              at the endpoints of the TCP connection.
 *     - Non-zero value (up to 64K) = The TCP segment could be as large as 64K 
 *              (the maximum IP datagram size), but it could be fragmented 
 *              at the IP layer in order to be transmitted 
 *              across the network to the receiving host.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_SOCKET_TCP_MSS
/*! @endcond */
#define FNET_CFG_SOCKET_TCP_MSS         (0)
/*! @cond */

#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_TCP_TX_BUF_SIZE
 * @brief    Default maximum size for the TCP send-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_SNDBUF socket option.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_SOCKET_TCP_TX_BUF_SIZE
/*! @endcond */
#define FNET_CFG_SOCKET_TCP_TX_BUF_SIZE (2 * 1024)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_TCP_RX_BUF_SIZE
 * @brief    Default maximum size for the TCP receive-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_RCVBUF socket option.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_SOCKET_TCP_RX_BUF_SIZE
/*! @endcond */
#define FNET_CFG_SOCKET_TCP_RX_BUF_SIZE (2 * 1024)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_UDP_TX_BUF_SIZE
 * @brief    Default maximum size for the UDP send-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_SNDBUF socket option.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_SOCKET_UDP_TX_BUF_SIZE
/*! @endcond */
#define FNET_CFG_SOCKET_UDP_TX_BUF_SIZE (2 * 1024)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_UDP_RX_BUF_SIZE
 * @brief    Default maximum size for the UDP receive-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_RCVBUF socket option.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_SOCKET_UDP_RX_BUF_SIZE
/*! @endcond */
#define FNET_CFG_SOCKET_UDP_RX_BUF_SIZE (2 * 1024)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_IP_MAX_PACKET
 * @brief    Maximum size for the IP datagram, the largest value is 65535.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_IP_MAX_PACKET
/*! @endcond */
#define FNET_CFG_IP_MAX_PACKET          (65535)
/*! @cond */
#endif
/*! @endcond */

/*****************************************************************************
 * Function Overload
 *****************************************************************************/
/*! @cond */

#ifndef FNET_CFG_OVERLOAD_CHECKSUM_LOW
#define FNET_CFG_OVERLOAD_CHECKSUM_LOW      (0)
#endif

#ifndef FNET_CFG_OVERLOAD_MEMCPY
#define FNET_CFG_OVERLOAD_MEMCPY            (0)
#endif

/*! @endcond */

/*! @cond */
/*****************************************************************************
 * DEBUGING INFO OUTPUT
 *****************************************************************************/


/**************************************************************************/ /*!
 * @internal
 * @brief    Debugging output:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @internal
 ******************************************************************************/
#ifndef FNET_CFG_DEBUG
    #define FNET_CFG_DEBUG              (1)
#endif

#ifndef FNET_CFG_DEBUG_TIMER  /* It will be printed to the UART '!' sign every second. */
    #define FNET_CFG_DEBUG_TIMER        (0)
#endif

#ifndef FNET_CFG_DEBUG_HTTP
    #define FNET_CFG_DEBUG_HTTP         (0)
#endif

#ifndef FNET_CFG_DEBUG_DHCP
    #define FNET_CFG_DEBUG_DHCP         (1)
#endif

#ifndef FNET_CFG_DEBUG_ARP
    #define FNET_CFG_DEBUG_ARP          (1)
#endif

#ifndef FNET_CFG_DEBUG_MEMPOOL
    #define FNET_CFG_DEBUG_MEMPOOL      (0)
#endif

#ifndef FNET_CFG_DEBUG_TFTP_CLN
    #define FNET_CFG_DEBUG_TFTP_CLN     (0)
#endif

#ifndef FNET_CFG_DEBUG_TFTP_SRV
    #define FNET_CFG_DEBUG_TFTP_SRV     (0)
#endif

#ifndef FNET_CFG_DEBUG_STACK
    #define FNET_CFG_DEBUG_STACK        (0)
#endif

#ifndef FNET_CFG_DEBUG_TELNET
    #define FNET_CFG_DEBUG_TELNET       (0)
#endif

#ifndef FNET_CFG_DEBUG_SHELL
    #define FNET_CFG_DEBUG_SHELL        (0)
#endif

#ifndef FNET_CFG_DEBUG_BENCH
    #define FNET_CFG_DEBUG_BENCH        (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE
    #define FNET_CFG_DEBUG_TRACE        (1)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_IP
    #define FNET_CFG_DEBUG_TRACE_IP     (1)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_IP
    #define FNET_CFG_DEBUG_TRACE_ICMP   (1)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_ETH
    #define FNET_CFG_DEBUG_TRACE_ETH    (1)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_ARP
    #define FNET_CFG_DEBUG_TRACE_ARP    (1)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_UDP
    #define FNET_CFG_DEBUG_TRACE_UDP    (1)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_TCP
    #define FNET_CFG_DEBUG_TRACE_TCP    (1)
#endif
 

#if !FNET_CFG_DEBUG /* Clear all debuging flags. */
    #undef  FNET_CFG_DEBUG_TIMER
    #define FNET_CFG_DEBUG_TIMER        (0)
    #undef  FNET_CFG_DEBUG_HTTP
    #define FNET_CFG_DEBUG_HTTP         (0)
    #undef  FNET_CFG_DEBUG_DHCP
    #define FNET_CFG_DEBUG_DHCP         (0)        
    #undef  FNET_CFG_DEBUG_TELNET
    #define FNET_CFG_DEBUG_TELNET       (0)     
    #undef  FNET_CFG_DEBUG_ARP
    #define FNET_CFG_DEBUG_ARP          (0)    
    #undef  FNET_CFG_DEBUG_MEMPOOL
    #define FNET_CFG_DEBUG_MEMPOOL      (0) 
    #undef  FNET_CFG_DEBUG_TFTP_CLN
    #define FNET_CFG_DEBUG_TFTP_CLN     (0)          
    #undef  FNET_CFG_DEBUG_TFTP_SRV
    #define FNET_CFG_DEBUG_TFTP_SRV     (0)    
    #undef  FNET_CFG_DEBUG_STACK
    #define FNET_CFG_DEBUG_STACK        (0) 
    #undef  FNET_CFG_DEBUG_SHELL
    #define FNET_CFG_DEBUG_SHELL        (0)          
    #undef  FNET_CFG_DEBUG_BENCH
    #define FNET_CFG_DEBUG_BENCH        (0)
    #undef  FNET_CFG_DEBUG_TRACE
    #define FNET_CFG_DEBUG_TRACE        (0)    
#endif

#if !FNET_CFG_DEBUG_TRACE /* Clear all trace flags. */
    #undef  FNET_CFG_DEBUG_TRACE_IP
    #define FNET_CFG_DEBUG_TRACE_IP     (0)
    #undef  FNET_CFG_DEBUG_TRACE_ICMP
    #define FNET_CFG_DEBUG_TRACE_ICMP   (0)    
    #undef  FNET_CFG_DEBUG_TRACE_ETH
    #define FNET_CFG_DEBUG_TRACE_ETH    (0)    
    #undef  FNET_CFG_DEBUG_TRACE_ARP
    #define FNET_CFG_DEBUG_TRACE_ARP    (0)      
    #undef  FNET_CFG_DEBUG_TRACE_UDP
    #define FNET_CFG_DEBUG_TRACE_UDP    (0)    
    #undef  FNET_CFG_DEBUG_TRACE_TCP
    #define FNET_CFG_DEBUG_TRACE_TCP    (0)    
#endif


/**************************************************************************/ /*!
 * @internal
 ******************************************************************************/
#ifndef FNET_OS
    #define FNET_OS                     (0)
#endif

/*! @endcond */

/*! @} */

#endif
