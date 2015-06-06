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
* @file fnet_isr.c
*
* @author Andrey Butok
*
* @date May-6-2011
*
* @version 0.1.15.0
*
* @brief Interrupt service dispatcher implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_isr.h"
#include "fnet_timer.h"
#include "fnet_netbuf.h"

/* queue of interrupt descriptors */
typedef struct fnet_isr
{
    struct fnet_isr *next;          /* next isr in chain */
    unsigned int vector_number;     /* vector number */
    void (*handler_top)( void );    /* “Critical handler” it will 
                                     * be called every time on interrupt event, 
                                     * (e.g. user can put here clear flags etc.)*/

    void (*handler_bottom)( void ); /* “Bottom half handler” it will be called after 
                                     *  isr_handler_top() in case NO SW lock 
                                     *  or on SW unlock.*/
    unsigned int pended;            /* indicates interrupt pending */
} fnet_isr_t;

static unsigned long fnet_locked = 0;
static fnet_isr_t *fnet_isr_table = 0;


/************************************************************************
* NAME: fnet_isr_handler
*
* DESCRIPTION: This handler is envoked by fnet_cpu_isr().
*              If fnet_locked == 0 - executes the
*              corresponding handler; else - marks it as pended.
*************************************************************************/
void fnet_isr_handler( int vector_number )
{
    fnet_isr_t *isr_cur;

/* This function operates as follows:
 * Searches through linked list of ISR Descriptor Table entries until
 * match is found with vector_number. Calls handler_top().
 * If local global "fnet_locked" is set, flags this interrupt as "pended" and exits.
 * Else, clears this interrupt's "pended" flag, executes handler_bottom() and exits.
 * NOTE: fnet_locked is incremented by fnet_isr_lock() and
 * decremented by fnet_isr_unlock().
*/

    for (isr_cur = fnet_isr_table; isr_cur; isr_cur = isr_cur->next)
    {
        if(isr_cur->vector_number == vector_number) /* we got it. */
        {
            if(isr_cur->handler_top)
                isr_cur->handler_top();         /* Call "top half" handler; */

            if(fnet_locked)
            {
                isr_cur->pended = 1;
            }
            else
            {
                isr_cur->pended = 0;

                if(isr_cur->handler_bottom)
                    isr_cur->handler_bottom(); /* Call "bottom half" handler;*/
            }

            break;
        }
    }
}

/************************************************************************
* NAME: fnet_isr_set_vector
*
* DESCRIPTION: Sets 'handler' for the interrupt vector with number 
*              'vector_number' at the exception vector queue and interrupt 
*              handler 'fnet_cpu_isr' at the exception vector table
*************************************************************************/
unsigned long fnet_isr_set_vector( unsigned int vector_number, void (*handler_top)( void ),
                                   void (*handler_bottom)( void ), unsigned int priority )
{
    unsigned long old_handler = 0;

    fnet_isr_t *isr_temp;

    old_handler = fnet_cpu_isr_install(vector_number, priority);

    isr_temp = (fnet_isr_t *)fnet_malloc(sizeof(fnet_isr_t));

    if(isr_temp == 0)
        return 0;

    isr_temp->vector_number = vector_number;

    isr_temp->handler_top = (void (*)())handler_top;

    isr_temp->handler_bottom = (void (*)())handler_bottom;

    isr_temp->next = fnet_isr_table;

    isr_temp->pended = 0;

    fnet_isr_table = isr_temp;

    return (old_handler);
}

/************************************************************************
* NAME: fnet_isr_reset_vector
*
* DESCRIPTION: Sets the interrupt handler 'handler' for the interrupt vector 
*              with number 'vector_number' at the exception vector table but 
*              destroys info about old interrupt handler and removes
*              information from 'fnet_isr_table' queue
*************************************************************************/
void fnet_isr_reset_vector( unsigned int vector_number, void (*handler)( void ) )
{
    fnet_isr_t *isr_temp;

    fnet_cpu_isr_uninstall(vector_number, (unsigned long)handler);

    isr_temp = fnet_isr_table;

    while(isr_temp != 0)
    {
        if(isr_temp->vector_number == vector_number)
            break;

        isr_temp = isr_temp->next;
    }

    if(isr_temp == 0)       /* if handler wasn't registered in queue */
        return;

    fnet_free(isr_temp);

    if(fnet_isr_table->vector_number == vector_number)
    {
        fnet_isr_table = fnet_isr_table->next;
        return;
    }
    else
    {
        isr_temp = fnet_isr_table;

        while(isr_temp->next != 0)
        {
            if(isr_temp->next->vector_number == vector_number)
            {
                isr_temp->next = isr_temp->next->next;
                break;
            }

            isr_temp = isr_temp->next;
        }
    }
}


/************************************************************************
* NAME: fnet_isr_lock
*
* DESCRIPTION: After execution of this routine all interrupts will be 
*              pended
*************************************************************************/
void fnet_isr_lock( void )
{
    fnet_locked++;
}

/************************************************************************
* NAME: fnet_isr_unlock
*
* DESCRIPTION: Executes all pending interrupt handlers and 
*              enables hardware interrupts processing
*************************************************************************/
void fnet_isr_unlock( void )
{
    fnet_isr_t *isr_temp;
	
	/* This function operates as follows:
    * If local global "fnet_locked" == 0 then it simply returns.
    * Else, if fnet_locked == 1 (at topmost lock level), 
    * index through ISR Descriptor Table until 1st pended interrupt is found.
    * Clear pended status and call associated handler_bottom().
    * Continue to do this until all pended interrupts have been handled.
    *Always exits by decrementing fnet_locked so as to bump up a lock level.
	*/	

    if(fnet_locked == 1)
    {
        isr_temp = fnet_isr_table;

        while(isr_temp != 0)
        {
            if(isr_temp->pended)
            {
                isr_temp->pended = 0;

                if(isr_temp->handler_bottom)
                    isr_temp->handler_bottom();
            }

            isr_temp = isr_temp->next;
        }
    }

    --fnet_locked;
}

/************************************************************************
* NAME: fnet_isr_generate_irq
*
* DESCRIPTION: This function can generate software or hardware irq. 
*              This irq should be registered in 'fnet_isr_table'. For software 
*              interrupts vector_number >= 192.
*              It will defer call to handler_bottom if interrupts are locked,
*              same as hardware vectored interrupts
*
*************************************************************************/
void fnet_isr_generate_irq( unsigned int vector_number )
{
    fnet_isr_t *isr_temp;

    isr_temp = fnet_isr_table;

    fnet_isr_lock();

    while(isr_temp != 0)
    {
        if(isr_temp->vector_number == vector_number)
        {
            if(isr_temp->handler_top)
                isr_temp->handler_top();

            if(fnet_locked == 1)
            {
                isr_temp->pended = 0;

                if(isr_temp->handler_bottom)
                    isr_temp->handler_bottom();
            }
            else
                isr_temp->pended = 1;

            break;
        }

        isr_temp = isr_temp->next;
    }

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_isr_generate_irq
*
* DESCRIPTION: This function inits ISR module
*
*************************************************************************/
void fnet_isr_init()
{
    fnet_locked = 0;
    fnet_isr_table = 0;
}
