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
* @file fnet_isr.h
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.14.0
*
* @brief Private. Interrupt dispatcher API.
*
***************************************************************************/

#ifndef _FNET_ISR_H_

#define _FNET_ISR_H_

#include "fnet_config.h"

/* Interrupt dispatcher functions.*/
unsigned long fnet_isr_set_vector( unsigned int vector_number, void (*handler_top)( void ),
                                   void (*handler_bottom)( void ), unsigned int priority );

void fnet_isr_reset_vector( unsigned int vector_number, void (*handler)( void ) );
void fnet_isr_generate_irq( unsigned int vector_number );
void fnet_isr_lock( void );
void fnet_isr_unlock( void );
void fnet_isr_init();


void fnet_isr_handler( int vector_number );
unsigned long fnet_cpu_isr_install(unsigned int vector_number, unsigned int priority);
void fnet_cpu_isr_uninstall( unsigned int vector_number, unsigned long old_handler );

#define FNET_ISR_SW_VECTOR_NUMBER (192+0x40) /* Defines first number of the SW Interrupt handler. */

#endif
