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
* @file fnet_mk_flash.c
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.2.0
*
* @brief Kinetis Flash Memory Module (FTFL) driver.
*
***************************************************************************/
#include "fnet_config.h" 
#include "fnet_cpu.h"
#include "fnet_stdlib.h"

#if FNET_MK && FNET_CFG_CPU_FLASH

/************************************************************************
* NAME: fnet_ftfl_command_lunch_inram
*
* DESCRIPTION: Launch the command. It must be in RAM.
************************************************************************/
/* == Should be in the RAM ==*/
__ramfunc static void fnet_ftfl_command_lunch_inram(void)
{
    FNET_MK_FTFL_FSTAT = FNET_MK_FTFL_FSTAT_CCIF_MASK;
    while (!(FNET_MK_FTFL_FSTAT & FNET_MK_FTFL_FSTAT_CCIF_MASK)) 
    {};
}

/************************************************************************
* NAME: fnet_ftfl_command
*
* DESCRIPTION: FTFL command 
************************************************************************/
static void fnet_ftfl_command( unsigned char command, unsigned long *address, unsigned long data )
{
    fnet_cpu_irq_desc_t irq_desc;
   
    #if 1
        /* This problem exist only in first released version product (mask set: 0M33Z). It is proved. 
        * and is solved in next released version product (mask set: 0N30D). Not able to check it.
        */
        FNET_MK_FMC_PFB0CR &= 0xFFFFFFFE; /* Single entry buffer disable.*/
        FNET_MK_FMC_PFB1CR &= 0xFFFFFFFE; /* Single entry buffer disable. */
        FNET_MK_FMC_PFB0CR &= 0xFFFFFFEF; /* Data Cache disable. */
        FNET_MK_FMC_PFB1CR &= 0xFFFFFFEF; /* Data Cache disable. */
    #endif
      
    irq_desc = fnet_cpu_irq_disable();
    
    /* The CCIF flag must read 1 to verify that any previous command has
    * completed. If CCIF is zero, the previous command execution is still active, a new
    * command write sequence cannot be started, and all writes to the FCCOB registers are
    * ignored.
    */
    while (!(FNET_MK_FTFL_FSTAT & FNET_MK_FTFL_FSTAT_CCIF_MASK))     
    {};
    
    /* Ensure that the ACCERR and FPVIOL bits in the FSTAT register are cleared prior to
    *  starting the command write sequence.
    */
    FNET_MK_FTFL_FSTAT = (FNET_MK_FTFL_FSTAT_ACCERR_MASK | FNET_MK_FTFL_FSTAT_FPVIOL_MASK);           
    
    /* The FCCOB register group uses a big endian addressing convention. */
    
    /* Write a valid command to the FCCOB0 register. */
    FNET_MK_FTFL_FCCOB0 = command;
    
    /* Flash address.*/
    FNET_MK_FTFL_FCCOB1 = (fnet_uint8)(((fnet_uint32)address) >> 16);   /* Flash address [23:16] */
    FNET_MK_FTFL_FCCOB2 = (fnet_uint8)(((fnet_uint32)address) >> 8);    /* Flash address [15:8] */
    FNET_MK_FTFL_FCCOB3 = (fnet_uint8)((fnet_uint32)address);           /* Flash address [7:0] */
    /* Data.*/
    FNET_MK_FTFL_FCCOB4 = (fnet_uint8)(data >> 24);    /* Data Byte 0.*/                       
    FNET_MK_FTFL_FCCOB5 = (fnet_uint8)(data >> 16);    /* Data Byte 1.*/
    FNET_MK_FTFL_FCCOB6 = (fnet_uint8)(data >> 8);     /* Data Byte 2.*/
    FNET_MK_FTFL_FCCOB7 = (fnet_uint8)(data);          /* Data Byte 3.*/

    fnet_ftfl_command_lunch_inram();

    fnet_cpu_irq_enable(irq_desc);
}

/************************************************************************
* NAME: fnet_cpu_flash_erase
*
* DESCRIPTION:
************************************************************************/
void fnet_cpu_flash_erase( void *flash_page_addr)
{
    fnet_ftfl_command(FNET_MK_FNET_FTFL_FCCOB0_CMD_ERASE_SECTOR, flash_page_addr, 0);
}

/************************************************************************
* NAME: fnet_cpu_flash_write
*
* DESCRIPTION:
************************************************************************/
void fnet_cpu_flash_write( unsigned long *dest, unsigned long data)
{
    fnet_ftfl_command(FNET_MK_FNET_FTFL_FCCOB0_CMD_PROGRAM_LONGWORD, dest, data);
}


#endif /* FNET_MK && FNET_CFG_CPU_FLASH */
