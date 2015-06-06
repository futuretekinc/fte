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
* @file fnet_os_config.h
*
* @author Andrey Butok
*
* @date May-16-2011
*
* @version 0.1.3.0
*
* @brief Default OS-specific configuration. @n
* Experental. Not supported.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/
#ifndef _FNET_OS_CONFIG_H_

#define _FNET_OS_CONFIG_H_

/*! @addtogroup fnet_platform_config  */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS
 * @brief     Operation System support:
 *               - @c 1 = is enabled. The OS type is defined by the @ref FNET_CFG_OS_operation_system_type.
 *               - @c 0 = is disabled. It is used bare-metal FNET stack.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_OS
/*! @endcond */
	#define FNET_CFG_OS    (0)
/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def      FNET_CFG_OS_operation_system_type 
 * @brief    This is the set of the @c FNET_CFG_OS_[operation_system_type] definitions that 
 *           define a currently used operation system. @n
 *           Current version of the FNET supports the following OS definitions:
 *            - @c FNET_CFG_OS_UCOSII  = Used OS is the uCOS-II.
 *            @n @n
 *            Selected OS definition should be only one and must be defined as 1. 
 *            All others may be defined but must have the 0 value.
 *
 ******************************************************************************/
#define FNET_CFG_OS_operation_system_type /* Ignore it. Just only for Doxygen documentation */

#if FNET_CFG_OS

	/*-----------*/
	#ifndef FNET_CFG_OS_UCOSII
		#define FNET_CFG_OS_UCOSII   (0)
	#endif    

	/*-----------*/
	#if FNET_CFG_OS_UCOSII /* uCOS-II */
		#ifdef FNET_OS_STR
			#error More than one OS selected FNET_OS_XXXX
		#endif
	   
		#include "fnet_ucosII_config.h"
		#define FNET_OS_STR    "uCOS-II"
	#endif


    /*-----------*/
    #if FNET_CFG_OS_UCOSII
        #include "fnet_ucosII_config.h"
    #endif

#endif /* FNET_CFG_OS*/

/*-----------*/
#ifndef FNET_OS_STR
    #define FNET_OS_STR    "NONE"
    
    #undef  FNET_CFG_OS_UCOSII
    #define FNET_CFG_OS_UCOSII      (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS_MUTEX
 * @brief     Mutex support:
 *               - @c 1 = is enabled. 
 *               - @c 0 = is disabled. 
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_OS_MUTEX
/*! @endcond */
	#define FNET_CFG_OS_MUTEX    (0)
/*! @cond */
#endif
/*! @endcond */


/*! @} */

#endif
