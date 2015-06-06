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
* @file fnet_cpu.h
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.19.0
*
* @brief CPU-specific library API. 
*
***************************************************************************/

#ifndef _FNET_CPU_H_

#define _FNET_CPU_H_

#include "fnet_config.h"

#if FNET_MCF    /* ColdFire.*/ 
    #include "fnet_mcf.h"
#endif    

#if FNET_MK     /* Kinetis.*/
    #include "fnet_mk.h"
#endif    

/*! @addtogroup fnet_cpu
* The CPU-specific library provides commonly used platform dependent functions.
* Most of these functions are not used by the FNET, but can be useful for
* an application.
*/
/*! @{ */


/**************************************************************************/ /*!
 * @brief IRQ status descriptor returned by the @ref fnet_cpu_irq_disable() function.
 * Actually it corresponds to the interrupt level mask value.
 * @see fnet_cpu_irq_disable(), fnet_cpu_irq_enable()
 ******************************************************************************/
typedef unsigned long fnet_cpu_irq_desc_t;

/***************************************************************************/ /*!
 *
 * @brief    Initiates software reset.
 *
 ******************************************************************************
 *
 * This function performs software reset and asserts the external reset 
 * (RSTO) pin.
 *
 ******************************************************************************/
void fnet_cpu_reset (void);

/***************************************************************************/ /*!
 *
 * @brief    Disables all interrupts.
 *
 * @return This function returns the current IRQ status defined 
 * by @ref fnet_cpu_irq_desc_t.
 *
 * @see fnet_cpu_irq_enable()
 *
 ******************************************************************************
 *
 * This function disables all interrupts. @n
 * The interrupts can be enabled again by the @ref fnet_cpu_irq_enable() function.
 *
 ******************************************************************************/ 
fnet_cpu_irq_desc_t fnet_cpu_irq_disable(void);

/***************************************************************************/ /*!
 *
 * @brief    Enables interrupts.
 *
 * @param desc     IRQ status descriptor returned by the 
 *                 @ref fnet_cpu_irq_disable() function.@n
 *                 Pass @c 0 value to enable all interrupts.
 *
 * @see fnet_cpu_irq_disable()
 *
 ******************************************************************************
 *
 * This function enables interrupts that were disabled by the 
 * @ref fnet_cpu_irq_disable function. @n
 * The functions can enable all interrupts by passing into it the @c 0 value.
 *
 ******************************************************************************/
void fnet_cpu_irq_enable(fnet_cpu_irq_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Writes character to the serial port.
 *
 * @param port_number     Serial port number.
 *
 * @param character       Character to be written to the serial port.
 *
 * @see fnet_cpu_serial_getchar(), fnet_cpu_serial_init()
 *
 ******************************************************************************
 *
 * This function writes @c character to the serial port defined 
 * by @c port_number. @n
 *
 ******************************************************************************/ 
void fnet_cpu_serial_putchar( long port_number, int character );

/***************************************************************************/ /*!
 *
 * @brief    Reads character from the serial port.
 *
 * @param port_number     Serial port number.
 *
 * @return This function returns:
 *   - character received by the serial port.
 *   - @ref FNET_ERR if no character is available.
 *
 * @see fnet_cpu_serial_putchar(), fnet_cpu_serial_init()
 *
 ******************************************************************************
 *
 * This function reads character from the serial port defined 
 * by @c port_number. @n
 *
 ******************************************************************************/ 
int fnet_cpu_serial_getchar( long port_number );

/***************************************************************************/ /*!
 *
 * @brief    Initializes the serial port.
 *
 * @param port_number     Serial port number.
 * 
 * @param baud_rate       Baud rate to be set to the serial port.
 *
 * @see fnet_cpu_serial_putchar(), fnet_cpu_serial_getchar()
 *
 ******************************************************************************
 *
 * This function executes the  HW initialization of the serial port defined 
 * by the @c port_number.
 *
 ******************************************************************************/
void fnet_cpu_serial_init(long port_number, unsigned long baud_rate);


/***************************************************************************/ /*!
 *
 * @brief    Invalidates CPU-cache memory.
 *
 ******************************************************************************
 *
 * If the CPU has cache memory, this function invalidates it.
 *
 ******************************************************************************/
void fnet_cpu_cache_invalidate();

/***************************************************************************/ /*!
 *
 * @brief    Erases the specified page of the on-chip Flash memory.
 *
 * @param flash_page_addr      Address of the page in the Flash to erase.
 *
 * @see fnet_cpu_flash_write()
 *
 ******************************************************************************
 *
 * This function erases the  whole Flash page pointed by @c flash_page_addr.@n
 * Erase page size is defined by @ref FNET_CFG_CPU_FLASH_PAGE_SIZE.
 *
 ******************************************************************************/
void fnet_cpu_flash_erase(void *flash_page_addr);

/***************************************************************************/ /*!
 *
 * @brief    Writes the specified data to the Flash memory.
 *
 * @param dest            Destination address in the Flash to write to.
 *
 * @param data            32-bit value containing the 
 *                        data to write to the Flash memory.
 *
 * @see fnet_cpu_flash_erase()
 *
 ******************************************************************************
 *
 * This function copies four bytes of @c data
 * to the Flash memory pointed by @c dest.
 *
 ******************************************************************************/
void fnet_cpu_flash_write(unsigned long *dest, unsigned long data);

/***************************************************************************/ /*!
 *
 * @brief    CPU-specific FNET interrupt service routine.
 *
 ******************************************************************************
 *
 * This is CPU-specific ISR which is executed on every FNET interrupt 
 * (from Ethernet and timer module).@n
 * It extracts vector number and calls fnet_isr_handler().
 *
 ******************************************************************************/
void fnet_cpu_isr(void);

/*! @} */

#endif
