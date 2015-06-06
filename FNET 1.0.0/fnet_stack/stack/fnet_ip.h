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
* @file fnet_ip.h
*
* @author Andrey Butok
*
* @date Jun-2-2011
*
* @version 0.1.12.0
*
* @brief IP protocol API.
*
***************************************************************************/

#ifndef _FNET_IP_H_

#define _FNET_IP_H_

/*! @addtogroup fnet_socket */
/*! @{ */

/************************************************************************
*     For doing byte order conversions between CPU 
*     and the independent "network" order.
*     For Freescale CPUs they just return the variable passed.
*************************************************************************/
/*! @cond */
#if FNET_CFG_CPU_LITTLE_ENDIAN /* Little endian.*/
/*! @endcond */  
/**************************************************************************/ /*!
 * @def FNET_HTONS
 * @param short_var A 16-bit number in host byte order.
 * @hideinitializer
 * @see FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 * @brief Macros which converts the unsigned short integer from host byte order to 
 * network byte order.
 ******************************************************************************/
    #define FNET_HTONS(short_var)   ((((short_var) & 0x00FF) << 8) | (((short_var) & 0xFF00) >> 8))
/**************************************************************************/ /*!
 * @def FNET_NTOHS
 * @param short_var A 16-bit number in network byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl() 
 * @brief Macros which converts the unsigned short integer from network byte order to 
 * host byte order.
 ******************************************************************************/      
    #define FNET_NTOHS(short_var)   FNET_HTONS(short_var) 
/**************************************************************************/ /*!
 * @def FNET_HTONL
 * @param long_var A 32-bit number in host byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl() 
 * @brief Macros which converts the unsigned long integer from host byte order to 
 * network byte order.
 ******************************************************************************/      
    #define FNET_HTONL(long_var)    ((((long_var) & 0x000000FF) << 24) | (((long_var) & 0x0000FF00) << 8) | (((long_var) & 0x00FF0000) >> 8) | (((long_var) & 0xFF000000) >> 24))   
/**************************************************************************/ /*!
 * @def FNET_NTOHL
 * @param long_var A 32-bit number in network byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl() 
 * @brief Macros which converts the unsigned long integer from network byte order to 
 * host byte order.
 ******************************************************************************/     
    #define FNET_NTOHL(long_var)    FNET_HTONL(long_var)  

/***************************************************************************/ /*!
 *
 * @brief    Converts 16-bit value from host to network byte order.
 *
 *
 * @param short_var A 16-bit number in host byte order.
 *
 *
 * @return This function returns the network byte-ordered @c short_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from host byte order to 
 * network byte order.
 *
 ******************************************************************************/
    unsigned short fnet_htons(unsigned short short_var);
 
 #define fnet_ntohs   fnet_htons
/***************************************************************************/ /*!
 *
 * @brief    Converts 16-bit value from network to host byte order.
 *
 *
 * @param short_var A 16-bit number in network byte order.
 *
 *
 * @return This function returns the host byte-ordered @c short_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_htonl(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from network byte order to 
 * host byte order.
 *
 ******************************************************************************/    
     unsigned short fnet_ntohs(unsigned short short_var);
     
/***************************************************************************/ /*!
 *
 * @brief    Converts 32-bit value from host to network byte order.
 *
 *
 * @param long_var A 32-bit number in host byte order.
 *
 *
 * @return This function returns the network byte-ordered @c long_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_ntohs(), fnet_htons(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from host byte order to 
 * network byte order.
 *
 ******************************************************************************/
    unsigned long fnet_htonl(unsigned long long_var);

#define fnet_ntohl    fnet_htonl
/***************************************************************************/ /*!
 *
 * @brief    Converts 32-bit value from network to host byte order.
 *
 *
 * @param long_var A 32-bit number in network byte order.
 *
 *
 * @return This function returns the host byte-ordered @c long_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from network byte order to 
 * host byte order.
 *
 ******************************************************************************/       
     unsigned long fnet_ntohl(unsigned long long_var);

/*! @cond */
#else /* Big endian. */
/*! @endcond */
/*! @cond */

    #define FNET_HTONS(short_var)   (short_var) 
   
    #define FNET_NTOHS(short_var)   (short_var)
   
    #define FNET_HTONL(long_var)    (long_var)
   
    #define FNET_NTOHL(long_var)    (long_var)
    
    #define fnet_htons(short_var)   (short_var) /* Convert short from host- to network byte order.*/
    #define fnet_ntohs(short_var)   (short_var) /* Convert short from network - to host byte order.*/
    #define fnet_htonl(long_var)    (long_var)  /* Convert long from host- to network byte order.*/
    #define fnet_ntohl(long_var)    (long_var)  /* Convert long from network - to host byte order.*/
/*! @endcond */    
/*! @cond */    
#endif
/*! @endcond */ 

/**************************************************************************/ /*!
 * @def FNET_GEN_ADDR
 * @param a First octet of an IP address.
 * @param b Second octet of an IP address.
 * @param c Third  octet of an IP address.
 * @param d Fourth  octet of an IP address.
 * @hideinitializer
 * @brief Converts the standard dotted-decimal notation @c a.b.c.d 
 *        to an integer value, suitable for use as an Internet address (in network byte order).
 ******************************************************************************/
#define FNET_GEN_ADDR(a, b, c, d)   (FNET_NTOHL(((unsigned long)((a)&0xFFL)<< 24) + ((unsigned long)((b)&0xFFL)<< 16) + ((unsigned long)((c)&0xFFL)<< 8 ) + (unsigned long)((d)&0xFFL)))

/* Special addresses */

/**************************************************************************/ /*!
 * @brief It means to use any network interface.
 ******************************************************************************/
#define INADDR_ANY       (unsigned long)(0x00000000)

/**************************************************************************/ /*!
 * @brief  Broadcast address. @n
 * It can be used to send the broadcast UDP datagrams over an IP network.
 ******************************************************************************/
#define INADDR_BROADCAST (unsigned long)(0xffffffff)

/**************************************************************************/ /*!
 * @brief Wrong IP address. @n
 * It's used by the @ref inet_addr() function as a return 
 * value.
 ******************************************************************************/
#define INADDR_NONE (FNET_ERR)

/**************************************************************************/ /*!
 * @brief IPv4 address type.
 ******************************************************************************/
typedef unsigned long fnet_ip_addr_t; 

/************************************************************************
*    Definitions for options.
*************************************************************************/
/* The type field is divided into three internal fields:*/
#define IPOPT_COPIED(t)   ((t)&0x80)    /* 1-bit copied flag */
#define IPOPT_CLASS (t)   ((t)&0x60)    /* 2-bit class field */
#define IPOPT_NUMBER(t)   ((t)&0x1f)    /* 5-bit number field */
/* Class field: */
#define IPOPT_CONTROL     (0x00)        /* control */
#define IPOPT_RESERVED1   (0x20)        /* reserved */
#define IPOPT_DEBMEAS     (0x40)        /* debugging and measurement */
#define IPOPT_RESERVED2   (0x60)        /* reserved */
/* Currently defined IP options */
#define IPOPT_EOL         (0)           /* end of option list */
#define IPOPT_NOP         (1)           /* no operation */

#define IPOPT_RR          (7)           /* record  route */
#define IPOPT_TS          (68)          /* timestamp */
#define IPOPT_SECURITY    (130)         /* basic security */
#define IPOPT_LSRR        (131)         /* loose source and record route */
#define IPOPT_SATID       (136)         /* stream id */
#define IPOPT_SSRR        (137)         /* strict source and record route */

#define IPOPT_TYPE        (0)
#define IPOPT_LENGTH      (1)
#define IPOPT_OFFSET      (2)
#define IPOPT_OFFSET_MIN  (4)           /* minimum value of 'offset'*/

/************************************************************************
*    Definitions for IP type of service.
*************************************************************************/
#define IP_TOS_NORMAL      (0x0)
#define IP_TOS_LOWDELAY    (0x10)
#define IP_TOS_THROUGHPUT  (0x08)
#define IP_TOS_RELIABILITY (0x04)

/************************************************************************
*    Timestamp option
*************************************************************************/
#define IPOPT_TS_FLAG_TSONLY     (0)    /* Record timestamps*/
#define IPOPT_TS_FLAG_TSANDADDR  (1)    /* Record addresses and timestamps*/
#define IPOPT_TS_FLAG_TSPRESPEC  (3)    /* Record timestamps only at the prespecified systems*/


/*! @} */

#endif
