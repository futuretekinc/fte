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
* @file fnet_fs_root.h
*
* @author Andrey Butok
*
* @date Oct-14-2009
*
* @version 0.0.5.0
*
* @brief Private. Root FS API.
*
***************************************************************************/

#ifndef _FNET_FS_ROOT_H_

#define _FNET_FS_ROOT_H_

#include "fnet_config.h"

#if FNET_CFG_FS

/* Root FS name string */
#define FNET_FS_ROOT_NAME   "root"

void fnet_fs_root_register( );
void fnet_fs_root_unregister( );


#endif /* FNET_CFG_FS */

#endif
