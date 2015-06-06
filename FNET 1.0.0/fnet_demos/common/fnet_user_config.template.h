**********************************************************************/ /*!
*
* @file fnet_user_config.template.h
*
* @brief Template of FNET User configuration file.
*               It should be used to change any default configuration parameter.
*
***************************************************************************/
/*! @cond */
#ifndef _FNET_USER_CONFIG_H_
#define _FNET_USER_CONFIG_H_

/*****************************************************************************
* Enable proper compiler support.
******************************************************************************/
#define FNET_CFG_COMP_CW            (0)  
#define FNET_CFG_COMP_IAR           (0)    

/*****************************************************************************
* Processor type.
* Selected processor definition should be only one and must be defined as 1. 
* All others may be defined but must have 0 value.
******************************************************************************/
#define FNET_CFG_CPU_MCF52235       (0)
#define FNET_CFG_CPU_MCF52259       (0)
#define FNET_CFG_CPU_MCF5282        (0)
#define FNET_CFG_CPU_MCF51CN128     (0)
#define FNET_CFG_CPU_MK60N512       (0)

/*****************************************************************************
* IP address for the Ethernet interface. 
* At runtime it can be changed by the fnet_netif_set_address() or 
* by the DHCP client service.
******************************************************************************/
#define FNET_CFG_ETH_IP_ADDR        (FNET_GEN_ADDR(10, 171, 88, 22))

/*****************************************************************************
* IP Subnet mask for the Ethernet interface. 
* At runtime it can be changed by the fnet_netif_set_netmask() or 
* by the DHCP client service.
******************************************************************************/
#define FNET_CFG_ETH_IP_MASK        (FNET_GEN_ADDR(255, 255, 255, 0))

/*****************************************************************************
* Gateway IP address for the Ethernet interface.
* At runtime it can be changed by the fnet_netif_set_gateway() or 
* by the DHCP client service.
******************************************************************************/
#define FNET_CFG_ETH_IP_GW          (FNET_GEN_ADDR(10, 171, 88, 254))

/*****************************************************************************
* Size of the internal static heap buffer. 
* This definition is used only if the fnet_init_static() was 
* used for the FNET initialization.
******************************************************************************/
#define FNET_CFG_HEAP_SIZE          (30 * 1024)

/*****************************************************************************
* TCP protocol support.
* You can disable it to save a substantial amount of code if 
* your application only needs UDP. By default it is enabled.
******************************************************************************/
#define FNET_CFG_TCP                (1)

/*****************************************************************************
* UDP protocol support.
* You can disable it to save a some amount of code if your 
* application only needs TCP. By default it is enabled.
******************************************************************************/
#define FNET_CFG_UDP                (1)

/*****************************************************************************
* UDP checksum.
* If enabled, the UDP checksum will be generated for transmitted 
* datagrams and be verified on received UDP datagrams.
* You can disable it to speedup UDP applications. 
* By default it is enabled.
******************************************************************************/
#define FNET_CFG_UDP_CHECKSUM       (1)

/*****************************************************************************
* IP fragmentation.
* If the IP fragmentation is enabled, the IP will attempt to reassemble IP 
* packet fragments and will able to generate fragmented IP packets.
* If disabled, the IP will  silently discard fragmented IP packets..
******************************************************************************/
#define FNET_CFG_IP_FRAGMENTATION   (1)

/*****************************************************************************
* MTU.
* Defines the Maximum Transmission Unit for the Ethernet interface.
* The largest value is 1500. Recommended range is 600 - 1500
******************************************************************************/
#define FNET_CFG_ETH_MTU            (1500)

/*****************************************************************************
* DHCP Client service support.
******************************************************************************/
#define FNET_CFG_DHCP               (1)

/*****************************************************************************
* HTTP Server service support.
******************************************************************************/
#define FNET_CFG_HTTP               (1)
#define FNET_CFG_HTTP_AUTHENTICATION_BASIC  (1) /* Enable HTTP authentication.*/
#define FNET_CFG_HTTP_POST                  (1) /* Enable HTTP POST-method support.*/

/*****************************************************************************
* Telnet Server service support.
******************************************************************************/
#define FNET_CFG_TELNET             (1)

/*****************************************************************************
* Flash Module driver support.
******************************************************************************/
#define FNET_CFG_FLASH              (1)

#endif
/*! @endcond */
