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
* @file fnet_timer.c
*
* @author Andrey Butok
*
* @date Apr-20-2011
*
* @version 0.1.9.0
*
* @brief ColdFire specific SW timers implementation.
*
***************************************************************************/

#include "fnet.h"
#include "fnet_timer_prv.h"
#include "fnet_netbuf.h"

/* Queue of the software timers*/

struct fnet_net_timer
{
    struct fnet_net_timer *next; /* Next timer in list.*/
    unsigned long timer_cnt;     /* Timer counter. */
    unsigned long timer_rv;      /* Timer reference value. */
    void (*handler)( void );     /* Timer handler. */
};

static struct fnet_net_timer *fnet_tl_head = 0;

volatile static unsigned long fnet_current_time;

#if FNET_CFG_DEBUG_TIMER    
    #define FNET_DEBUG_TIMER   FNET_DEBUG
#else
    #define FNET_DEBUG_TIMER(...)
#endif

/************************************************************************
* NAME: fnet_timer_init
*
* DESCRIPTION: Starts TCP/IP hardware timer. delay_ms - period of timer (ms)
*************************************************************************/
void fnet_timer_init( unsigned int period_ms )
{
   fnet_current_time = 0;           /* Reset RTC counter. */
   fnet_cpu_timer_init(period_ms);  /* Start HW timer. */
}

/************************************************************************
* NAME: fnet_timer_release
*
* DESCRIPTION: Frees the memory, which was allocated for all
*              TCP/IP timers, and removes hardware timer
*************************************************************************/
void fnet_timer_release( void )
{
    struct fnet_net_timer *tmp_tl;

    fnet_cpu_timer_release();
    
    while(fnet_tl_head != 0)
    {
        tmp_tl = fnet_tl_head->next;

        fnet_free(fnet_tl_head);

        fnet_tl_head = tmp_tl;
    }
}

/************************************************************************
* NAME: fnet_timer_ticks
*
* DESCRIPTION: This function returns current value of the timer in ticks. 
*************************************************************************/
unsigned long fnet_timer_ticks( void )
{
    return fnet_current_time;
}

/************************************************************************
* NAME: fnet_timer_ticks_inc
*
* DESCRIPTION: This function increments current value of the RTC counter. 
*************************************************************************/
void fnet_timer_ticks_inc( void )
{
    fnet_current_time++; 
    
#if FNET_CFG_DEBUG_TIMER
   if((fnet_current_time%10) == 0)    
	    FNET_DEBUG_TIMER("!");
#endif	    
}

/************************************************************************
* NAME: fnet_timer_handler_bottom
*
* DESCRIPTION: Handles timer interrupts 
*              
*************************************************************************/
void fnet_timer_handler_bottom( void )
{
    struct fnet_net_timer *tl;

    tl = fnet_tl_head;

    while(tl)
    {
        if(fnet_timer_get_interval(tl->timer_cnt, fnet_current_time) >= tl->timer_rv)
        {
            tl->timer_cnt = fnet_current_time;

            if(tl->handler)
                tl->handler();
        }

        tl = tl->next;
    }
}


/************************************************************************
* NAME: fnet_timer_new
*
* DESCRIPTION: Creates new software timer with the period 
*              
*************************************************************************/
fnet_timer_desc_t fnet_timer_new( int ref_val, void (*handler)( void ) )
{
    struct fnet_net_timer *tl;

    if((ref_val <= 0) || (handler == 0))
        return (0);

    tl = (struct fnet_net_timer *)fnet_malloc(sizeof(struct fnet_net_timer));

    if(tl == 0)
        return (0);

    tl->next = fnet_tl_head;

    fnet_tl_head = tl;

    tl->timer_cnt = 0;

    tl->timer_rv = (unsigned long)ref_val;

    tl->handler = handler;

    return (fnet_timer_desc_t)tl;
}

/************************************************************************
* NAME: fnet_timer_free
*
* DESCRIPTION: Frees software timer, which is pointed by tl_ptr 
*              
*************************************************************************/
void fnet_timer_free( fnet_timer_desc_t timer )
{
    struct fnet_net_timer *tl = timer;
    struct fnet_net_timer *tl_temp;

    if(tl == 0)
        return;

    if(tl == fnet_tl_head)
        fnet_tl_head = fnet_tl_head->next;
    else
    {
        tl_temp = fnet_tl_head;

        while(tl_temp->next != tl)
          tl_temp = tl_temp->next;

        tl_temp->next = tl->next;
    }

    fnet_free(tl);
}

/************************************************************************
* NAME: fnet_timer_reset_all
*
* DESCRIPTION: Resets all timers' counters
*              
*************************************************************************/
void fnet_timer_reset_all( void )
{
    struct fnet_net_timer *tl;

    tl = fnet_tl_head;

    while(tl != 0)
    {
        tl->timer_cnt = fnet_current_time;
        tl = tl->next;
    }
}



/************************************************************************
* NAME: fnet_timer_get_interval
*
* DESCRIPTION: Calaculates an interval between two moments of time
*              
*************************************************************************/
unsigned long fnet_timer_get_interval( unsigned long start, unsigned long end )
{
    if(start <= end)
        return (end - start);
    else
        return (0xffffffff - start + end + 1);
}

/************************************************************************
* NAME: fnet_timer_delay
*
* DESCRIPTION: Do delay for a given number of timer ticks.
*              
*************************************************************************/
void fnet_timer_delay( unsigned long delay_ticks )
{
    unsigned long start = fnet_current_time;

    while(fnet_timer_get_interval(start, fnet_timer_ticks()) < delay_ticks)
    { };
}

/************************************************************************
* NAME: fnet_timer_ms2ticks
*
* DESCRIPTION: Convert millisecons to timer ticks.
*              
*************************************************************************/
unsigned long fnet_timer_ms2ticks( unsigned long time_ms )
{
    return time_ms / FNET_TIMER_PERIOD_MS;
}
