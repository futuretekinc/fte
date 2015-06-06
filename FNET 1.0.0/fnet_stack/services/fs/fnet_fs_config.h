/**************************************************************************
* 
* Copyright 2005-2009 by Andrey Butok. Freescale Semiconductor, Inc.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License 
* Version 3 or later (the "GPL"), or the GNU Lesser General Public 
* License Version 3 or later (the "LGPL").
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
* @file fnet_fs_config.h
*
* @author Andrey Butok
*
* @date Apr-27-2010
*
* @version 0.0.7.0
*
* @brief FNET File System configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_FS_CONFIG_H_

#define _FNET_FS_CONFIG_H_

/** @addtogroup fnet_services_config */
/** @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_MOUNT_MAX
 * @brief    Maximum number of mount points.
 * @showinitializer
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_FS_MOUNT_MAX
/*! @endcond */

#define FNET_CFG_FS_MOUNT_MAX           (2)

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_MOUNT_NAME_MAX
 * @brief    Maximum size of a mount-point name. @n
 *           For example, for mount point named "rom" the name size is 3.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_FS_MOUNT_NAME_MAX
/*! @endcond */

#define FNET_CFG_FS_MOUNT_NAME_MAX      (10)

/*! @cond */
#endif
/*! @endcond */

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_DESC_MAX
 * @brief    Maximum number of file and directory descriptors opened 
 *           simultaneously.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_FS_DESC_MAX
/*! @endcond */

#define FNET_CFG_FS_DESC_MAX            (5)

/*! @cond */
#endif
/*! @endcond */


/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_ROM
 * @brief    FNET ROM File System support:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_FS_ROM
/*! @endcond */

#define FNET_CFG_FS_ROM                 (1) 

/*! @cond */
#endif
/*! @endcond */

/** @} */


#endif
