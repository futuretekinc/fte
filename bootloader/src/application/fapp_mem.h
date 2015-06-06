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
* @file fapp_mem.h
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.15.0
*
* @brief FNET Shell Demo API.
*
***************************************************************************/

#ifndef _FTE_BL_MEM_H_

#define _FTE_BL_MEM_H_

#include "fapp_config.h"
#include "fapp.h"

/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_SRAM_ADDRESS               FNET_CFG_CPU_SRAM_ADDRESS
#define FAPP_SRAM_SIZE                  FNET_CFG_CPU_SRAM_SIZE

#define FTE_BL_FLASH_ADDRESS            FNET_CFG_CPU_FLASH_ADDRESS
#define FTE_BL_FLASH_SIZE               FNET_CFG_CPU_FLASH_SIZE
#define FTE_BL_FLASH_PAGE_SIZE          FNET_CFG_CPU_FLASH_PAGE_SIZE

#define FTE_BL_LOADER_ADDRESS           FTE_BL_CFG_LOADER_ADDRESS  /* Actually is the start address of the flash.*/
#define FTE_BL_LOADER_SIZE              FTE_BL_CFG_LOADER_SIZE

#define FTE_BL_STATIC_PARAMS_ADDRESS    (FTE_BL_LOADER_ADDRESS + FTE_BL_LOADER_SIZE)    
#define FTE_BL_STATIC_PARAMS_SIZE       FTE_BL_CFG_LOADER_PARAMS_SIZE
#define FTE_BL_DYNAMIC_PARAMS_ADDRESS   (FTE_BL_LOADER_ADDRESS + FTE_BL_LOADER_SIZE + FTE_BL_CFG_LOADER_PARAMS_SIZE)    
#define FTE_BL_DYNAMIC_PARAMS_SIZE      FTE_BL_CFG_LOADER_PARAMS_SIZE

#ifndef FTE_BL_CFG_APP_IMAGE_ADDRESS
    #err "Undefined application image start address!"
#else
    #define FTE_BL_APP_IMAGE_ADDRESS    FTE_BL_CFG_APP_IMAGE_ADDRESS
#endif
#define FTE_BL_APP_IMAGE_SIZE           FTE_BL_CFG_APP_IMAGE_SIZE

#define FTE_BL_APP_IMAGE_HEADER_SIZE    (FTE_BL_FLASH_PAGE_SIZE)
#define FTE_BL_APP_IMAGE_HEADER_ADDRESS (FTE_BL_APP_IMAGE_ADDRESS - FTE_BL_APP_IMAGE_HEADER_SIZE)    


#define FTE_BL_APP_DATA_ADDRESS         (FTE_BL_APP_IMAGE_ADDRESS + FTE_BL_APP_IMAGE_SIZE) 
#define FTE_BL_APP_DATA_SIZE            FTE_BL_CFG_APP_DATA_SIZE

struct fapp_mem_region
{
    char *description;
    unsigned long address;
    unsigned long size;
    void (*memcpy)( void *dest, const void *src, unsigned n );
    void (*erase)( void *address, unsigned n );
    unsigned long erase_size; /* Logical page size, that can be erased separately. */
};

extern const struct fapp_mem_region fapp_mem_regions[];


#if FTE_BL_CFG_MEM_CMD
int fapp_mem_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
#endif

#if FTE_BL_CFG_ERASE_CMD
int fapp_mem_erase_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
#endif




int fapp_mem_memcpy (fnet_shell_desc_t desc, void *dest, const void *src, unsigned n );
int fapp_mem_region_is_protected( unsigned long start, unsigned long n);
void fapp_mem_erase_all(fnet_shell_desc_t desc);

#endif