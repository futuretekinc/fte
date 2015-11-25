/*
    NanoStack: MCU software and PC tools for IP-based wireless sensor networking.
		
    Copyright (C) 2006-2007 Sensinode Ltd.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

		Address:
		Sensinode Ltd.
		Teknologiantie 6	
		90570 Oulu, Finland

		E-mail:
		info@sensinode.com
*/


/**
 *
 * \file timer.c
 * \brief Period timer API.
 *
 *  Period timer API: platform dependent hooks.
 *  
 */



#include "mqx.h"
#include "xtype.h"

#include <string.h>


static _timer_id   hRFTimer = 0;

extern void rf_timer_callback(_timer_id, pointer, MQX_TICK_STRUCT_PTR);

int8_t timer_rf_launch(uint8_t nTicks)
{
    MQX_TICK_STRUCT xTick;
    
    _time_init_ticks(&xTick, nTicks);
    
    hRFTimer = _timer_start_oneshot_after_ticks(rf_timer_callback, 0, TIMER_ELAPSED_TIME_MODE, &xTick);
    
	return 0;
}

void timer_rf_stop(void)
{
    if (hRFTimer != 0)
    {
        _timer_cancel(hRFTimer);
        hRFTimer = 0;
    }
}
#ifdef HAVE_MAC_15_4
static _timer_id   hMACTimer = 0;

int8_t timer_mac_launch(uint8_t nTicks)
{
    MQX_TICK_STRUCT xTick;
    
    _time_init_ticks(&xTick, nTicks);
    
    hMACTimer = _timer_start_oneshot_after_ticks(rf_timer_callback, 0, TIMER_ELAPSED_TIME_MODE, &xTick);

	return 0;
}

void timer_mac_stop(void)
{
    if (hMACTimer != 0)
    {
        _timer_cancel(hMACTimer);
        hMACTimer = 0;
    }
}
#endif
