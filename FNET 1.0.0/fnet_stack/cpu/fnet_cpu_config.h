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
* @file fnet_cpu_config.h
*
* @author Andrey Butok
*
* @date Jun-2-2011
*
* @version 0.1.12.0
*
* @brief Default platform-specific configuration.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_CPU_CONFIG_H_

#define _FNET_CPU_CONFIG_H_

/*! @addtogroup fnet_platform_config  */
/*! @{ */
/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_processor_type 
 * @brief    This is the set of the @c FNET_CFG_CPU_[processor_type] definitions that 
 *           define a currently used processor. @n
 *           Current version of the FNET supports the following processor definitions:
 *            - @c FNET_CFG_CPU_MCF52235  = Used platform is the MCF52235.
 *            - @c FNET_CFG_CPU_MCF52259 = Used platform is the MCF52259.
 *            - @c FNET_CFG_CPU_MCF5282  = Used platform is the MCF5282.
 *            - @c FNET_CFG_CPU_MCF51CN128  = Used platform is the MCF51CN128.
 *            - @c FNET_CFG_CPU_MK60N512  = Used platform is the MK60N512. 
 *            @n @n
 *            Selected processor definition should be only one and must be defined as 1. 
 *            All others may be defined but must have the 0 value.
 * 
 ******************************************************************************/

#define FNET_CFG_CPU_processor_type /* Ignore it. Just only for Doxygen documentation */

/*-----------*/
#ifndef FNET_CFG_CPU_MCF52235
    #define FNET_CFG_CPU_MCF52235   (0)
#endif    
#ifndef FNET_CFG_CPU_MCF52259
    #define FNET_CFG_CPU_MCF52259   (0)
#endif
#ifndef FNET_CFG_CPU_MCF5282
    #define FNET_CFG_CPU_MCF5282    (0)
#endif
#ifndef FNET_CFG_CPU_MCF51CN128
    #define FNET_CFG_CPU_MCF51CN128 (0)
#endif
#ifndef FNET_CFG_CPU_MK60N512
    #define FNET_CFG_CPU_MK60N512   (0)
#endif  

/*-----------*/
#if FNET_CFG_CPU_MCF52235 /* Kirin2 */
    #ifdef FNET_CPU_STR
        #error More than one CPU selected FNET_CPU_XXXX
    #endif
   
    #include "fnet_mcf52235_config.h"
    #define FNET_CPU_STR    "MCF52235"
#endif

#if FNET_CFG_CPU_MCF52259 /* Kirin3 */
    #ifdef FNET_CPU_STR
        #error More than one CPU selected FNET_CPU_XXXX
    #endif
    
    #include "fnet_mcf52259_config.h"
    #define FNET_CPU_STR    "MCF52259"
#endif

#if FNET_CFG_CPU_MCF5282 /* Reindeer */
    #ifdef FNET_CPU_STR
        #error More than one CPU selected FNET_CPU_XXXX
    #endif
    
    #include "fnet_mcf5282_config.h"
    #define FNET_CPU_STR    "MCF5282"
#endif

#if FNET_CFG_CPU_MCF51CN128 /* Lasko */
    #ifdef FNET_CPU_STR
        #error More than one CPU selected FNET_CPU_XXXX
    #endif
    
    #include "fnet_mcf51cn128_config.h"
    #define FNET_CPU_STR    "MCF51CN128"
#endif

#if FNET_CFG_CPU_MK60N512 /* Kinetis */
    #ifdef FNET_CPU_STR
        #error More than one CPU selected FNET_CPU_XXXX
    #endif
    
    #include "fnet_mk60n512_config.h"
    #define FNET_CPU_STR    "MK60N512"
#endif

/*-----------*/
#ifndef FNET_CPU_STR
    #error Select/Define proper CPU FNET_CPU_XXXX !
#endif

/*-----------*/
#ifndef FNET_MCF
  #define FNET_MCF  (0)
#endif

#ifndef FNET_MK
  #define FNET_MK   (0)
#endif

/*-----------*/
#if FNET_MCF
    #include "fnet_mcf_config.h"
#endif

#if FNET_MK
    #include "fnet_mk_config.h"
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_LITTLE_ENDIAN
 * @brief    Byte order is:
 *               - @c 1 = little endian (for ARM).
 *               - @c 0 = big endian (for ColdFire).
 *           @n @n NOTE: User application should not change this parameter. 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_LITTLE_ENDIAN
/*! @endcond */
#define FNET_CFG_CPU_LITTLE_ENDIAN      (0)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_CLOCK_HZ
 * @brief    System frequency in Hz.
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_CLOCK_HZ 
/*! @endcond */
#error Please define  FNET_CFG_CPU_CLOCK_HZ
#define FNET_CFG_CPU_CLOCK_HZ       (xx) /* Just only for Doxygen documentation */
/*! @cond */
#endif

#define FNET_CPU_CLOCK_KHZ       (FNET_CFG_CPU_CLOCK_HZ/1000)     
#define FNET_CPU_CLOCK_MHZ       (FNET_CFG_CPU_CLOCK_HZ/1000000)  
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_SERIAL_PORT_DEFAULT
 * @brief    Defines the default serial port number.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_SERIAL_PORT_DEFAULT
/*! @endcond */
#define FNET_CFG_CPU_SERIAL_PORT_DEFAULT    (0)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_VECTOR_TABLE
 * @brief    Vector table address.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef __FNET_ASM_CODE
#ifndef FNET_CFG_CPU_VECTOR_TABLE
/* The following symbol should be defined in the linker file */
extern unsigned long __VECTOR_RAM [];
/*! @endcond */
#define FNET_CFG_CPU_VECTOR_TABLE           __VECTOR_RAM
/*! @cond */
#endif
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_TIMER_NUMBER
 * @brief    Timer number used by the FNET. It can range from 0 to 3.
 *           @n @n NOTE: It's ignored for MCF V1.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_TIMER_NUMBER
/*! @endcond */
#define FNET_CFG_CPU_TIMER_NUMBER           (3)
/*! @cond */
#endif
/*! @endcond */

#if (FNET_CFG_CPU_TIMER_NUMBER<0)||(FNET_CFG_CPU_TIMER_NUMBER>3)
    #error FNET_CFG_CPU_TIMER_NUMBER must be from 0 to 3.
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_TIMER_VECTOR_NUMBER
 * @brief    Vector number of the timer interrupt.
 *           @n @n NOTE: User application should not change this parameter. 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_TIMER_VECTOR_NUMBER
/*! @endcond */
#error FNET_CFG_CPU_TIMER_VECTOR_NUMBER is not defined.
/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_TIMER_VECTOR_PRIORITY
 * @brief    Default Interrupt priority level for a timer used by the FNET.
 *           It can range from 1 to 7. The higher the value, the greater 
 *           the priority of the corresponding interrupt.
 *           @n @n NOTE: It's ignored for MCF V1.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_TIMER_VECTOR_PRIORITY
/*! @endcond */
#define FNET_CFG_CPU_TIMER_VECTOR_PRIORITY        (3)
/*! @cond */
#endif
/*! @endcond */

#if (FNET_CFG_CPU_TIMER_VECTOR_PRIORITY<1)||(FNET_CFG_CPU_TIMER_VECTOR_PRIORITY>7)
    #error FNET_CFG_CPU_TIMER_VECTOR_PRIORITY must be from 1 to 7.
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_ETH_VECTOR_NUMBER
 * @brief    Vector number of the Ethernet Receive Frame interrupt.
 *           @n @n NOTE: User application should not change this parameter. 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_ETH_VECTOR_NUMBER
/*! @endcond */
#error FNET_CFG_CPU_ETH_VECTOR_NUMBER is not defined.
/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_ETH_VECTOR_PRIORITY
 * @brief    Default Interrupt priority level for the Ethernet module. 
 *           It can range from 1 to 7. The higher the value, the greater 
 *           the priority of the corresponding interrupt. 
 *           @n @n NOTE: It's ignored for MCF V1.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_ETH_VECTOR_PRIORITY
/*! @endcond */
#define FNET_CFG_CPU_ETH_VECTOR_PRIORITY          (2)
/*! @cond */
#endif
/*! @endcond */

#if (FNET_CFG_CPU_ETH_VECTOR_PRIORITY<1)||(FNET_CFG_CPU_ETH_VECTOR_PRIORITY>7)
    #error FNET_CFG_CPU_ETH_VECTOR_PRIORITY must be from 1 to 7.
#endif


/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_ETH_TX_BUFS_MAX
 * @brief    Defines the maximum number of outgoing frames that may 
 *           be buffered by the Ethernet module.
 *           As a result  
 *           ((@ref FNET_CFG_ETH_MTU+18) * @ref FNET_CFG_CPU_ETH_TX_BUFS_MAX) 
 *           bytes will be allocated.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_ETH_TX_BUFS_MAX
/*! @endcond */
#define FNET_CFG_CPU_ETH_TX_BUFS_MAX        (2)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_ETH_RX_BUFS_MAX
 * @brief    Defines the maximum number of incoming frames that may 
 *           be buffered by the Ethernet module.
 *           As a result 
 *           ((@ref FNET_CFG_ETH_MTU+18) * @ref FNET_CFG_CPU_ETH_RX_BUFS_MAX) 
 *           bytes will be allocated.
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_ETH_RX_BUFS_MAX
/*! @endcond */
#define FNET_CFG_CPU_ETH_RX_BUFS_MAX        (2)
/*! @cond */
#endif
/*! @endcond */

#if (FNET_CFG_CPU_ETH_TX_BUFS_MAX < 2)
#error FNET_CFG_CPU_ETH_TX_BUFS_MAX is less than 2, minimal required value is 2 - see errata MCF5235
#endif

#if (FNET_CFG_CPU_ETH_RX_BUFS_MAX < 2)
#error FNET_CFG_CPU_ETH_RX_BUFS_MAX is less than 2, minimal required value is 2 - see errata MCF5235
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_ETH_MIB
 * @brief    Ethernet Management Information Base (MIB) Block Counters:
 *               - @c 1 = Current platform has the Ethernet MIB Block.
 *               - @c 0 = Current platform does not have Ethernet MIB Block
 *                        (only for MCF51CNx).
  *              @n @n NOTE: User application should not change this parameter.
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_ETH_MIB
/*! @endcond */
#define FNET_CFG_CPU_ETH_MIB                (1)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_CACHE
 * @brief    Cache invalidation:
 *               - @c 1 = is enabled (for MCF5282).
 *               - @c 0 = is disabled. For platforms that do not have cache.
 *  @n @n NOTE: User application should not change this parameter.
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_CACHE
/*! @endcond */
#define FNET_CFG_CPU_CACHE                  (1)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_CPU_FLASH
 * @brief    On-chip Flash Module:
 *               - @c 1 = Current platform has the On-chip Flash Module 
 *                        (CFM for ColdFire, FTFL for Kinetis).
 *               - @c 0 = Current platform does not have the On-chip Flash Module.
 *              @n @n NOTE: User application should not change this parameter.
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_FLASH
/*! @endcond */
#define FNET_CFG_CPU_FLASH              (0)
/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_CPU_FLASH_ADDRESS
 * @brief   On-chip Flash memory start address. @n
 *          It is not used by the FNET, but can be useful for an application.
 ******************************************************************************/ 
/*! @cond */  
#ifndef FNET_CFG_CPU_FLASH_ADDRESS 
/*! @endcond */   
#define FNET_CFG_CPU_FLASH_ADDRESS      (0x0)
/*! @cond */    
#endif 
/*! @endcond */        

/**************************************************************************/ /*!
 * @def     FNET_CFG_CPU_FLASH_SIZE
 * @brief   On-chip Flash memory size (in bytes). @n
 *          It is not used by the FNET stack, but can be useful for an application.
 *          @n @n NOTE: User application should not change this parameter.
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_CPU_FLASH_SIZE
/*! @endcond */
#define FNET_CFG_CPU_FLASH_SIZE         (0)  
/*! @cond */    
#endif 
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_CPU_FLASH_PAGE_SIZE
 * @brief   Erase-page size of the on-chip Flash memory (in bytes). @n
 *          Flash logical blocks are divided into multiple logical pages that can be
 *          erased separately. @n
 *          It is not possible to read from any flash logical block while the same 
 *          logical block is being erased, programmed, or verified.
 *          @n @n NOTE: User application should not change this parameter.
 ******************************************************************************/    
/*! @cond */
#ifndef FNET_CFG_CPU_FLASH_PAGE_SIZE
/*! @endcond */
#define FNET_CFG_CPU_FLASH_PAGE_SIZE    (0)
/*! @cond */    
#endif 
/*! @endcond */

/**************************************************************************/ /*!
 * @def     FNET_CFG_CPU_SRAM_ADDRESS
 * @brief   On-chip SRAM memory start address. @n
 *          It is not used by the FNET stack, but can be useful for an application.
 ******************************************************************************/ 
/*! @cond */  
#ifndef FNET_CFG_CPU_SRAM_ADDRESS 
/*! @endcond */   
#define FNET_CFG_CPU_SRAM_ADDRESS       (0x0)
/*! @cond */    
#endif 
/*! @endcond */  
          
/**************************************************************************/ /*!
 * @def     FNET_CFG_CPU_SRAM_SIZE
 * @brief   On-chip SRAM memory size (in bytes). @n
 *          It is not used by the FNET stack, but can be useful for an application.
 *          @n @n NOTE: User application should not change this parameter.
 ******************************************************************************/  
/*! @cond */
#ifndef FNET_CFG_CPU_SRAM_SIZE
/*! @endcond */
#define FNET_CFG_CPU_SRAM_SIZE          (0)  
/*! @cond */    
#endif 
/*! @endcond */


/*! @} */

#endif
