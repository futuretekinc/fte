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
* @file fnet_prot.c
*
* @author Andrey Butok
*
* @date Apr-20-2011
*
* @version 0.1.11.0
*
* @brief Transport protocol interface implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_prot.h"
#include "fnet_tcp.h"
#include "fnet_udp.h"
#include "fnet_ip_prv.h"
#include "fnet_netif_prv.h"
#include "fnet_isr.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"

/************************************************************************
*     Global Data Structures
*************************************************************************/

static fnet_prot_if_t *fnet_prot_if_list; /* List of transport protocol interfaces.*/

/************************************************************************
* NAME: fnet_prot_init
*
* DESCRIPTION: Transport and IP layers initialization. 
*************************************************************************/
int fnet_prot_init( void )
{
    fnet_isr_lock();
    fnet_prot_if_list = 0;

#if FNET_CFG_TCP

    if(fnet_prot_add(&fnet_tcp_prot_if) == FNET_ERR)
        goto ERROR;

#endif

#if FNET_CFG_UDP

    if(fnet_prot_add(&fnet_udp_prot_if) == FNET_ERR)
        goto ERROR;

#endif

    if(fnet_ip_init() == FNET_ERR)
        goto ERROR;

    fnet_isr_unlock();
    return (FNET_OK);
    ERROR:
    fnet_isr_unlock();

    return (FNET_ERR);
}

/************************************************************************
* NAME: fnet_prot_release
*
* DESCRIPTION: Transport and IP layers release.
*************************************************************************/
void fnet_prot_release( void )
{
    fnet_isr_lock();

    while((volatile fnet_prot_if_t *)fnet_prot_if_list != 0)
      fnet_prot_del(fnet_prot_if_list);

    fnet_ip_release();
    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_prot_add
*
* DESCRIPTION: Adds the transport layer protocol to the stack.
*************************************************************************/
int fnet_prot_add( fnet_prot_if_t *prot )
{
    prot->next = fnet_prot_if_list;
    fnet_prot_if_list = prot;

    prot->head = 0;

    if(prot->prot_init)
        if(prot->prot_init() == FNET_ERR)
        {
            fnet_prot_if_list = prot->next;
            return (FNET_ERR);
        }

    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_prot_del
*
* DESCRIPTION: Deletes the transport layer protocol from the stack.
*************************************************************************/
void fnet_prot_del( fnet_prot_if_t *prot )
{
    fnet_prot_if_t *prot_tmp = fnet_prot_if_list;

    if(prot->prot_release)
        prot->prot_release();

    if(fnet_prot_if_list == prot)
    {
        fnet_prot_if_list = prot->next;
    }

    for (prot_tmp = fnet_prot_if_list; prot_tmp != 0; prot_tmp = prot_tmp->next)
    {
        if(prot_tmp->next == prot)
        {
            prot_tmp->next = prot->next;
            break;
        }
    }

    prot->next = 0;
}

/************************************************************************
* NAME: fnet_prot_find
*
* DESCRIPTION: This function looks up a protocol by domain family name, 
*              by type and by protocol number.
*************************************************************************/
fnet_prot_if_t *fnet_prot_find( int domain, int type, int protocol )
{
    fnet_prot_if_t *prot;

    for (prot = fnet_prot_if_list; prot != 0; prot = prot->next)
    {
        if((prot->domain == domain) && 
            ((prot->type == type) && ((protocol == 0) ? 1: (prot->protocol == protocol))
            || ((type == 0) && (prot->protocol == protocol))))
        {
            return (prot);
        }
    }

    return (0);
}

/************************************************************************
* NAME: fnet_prot_drain
*
* DESCRIPTION: Tries to free not critical parts of 
*              dynamic allocated memory in the stack, if possible.
*************************************************************************/
void fnet_prot_drain( void )
{
    fnet_prot_if_t *prot;

#if 0 /* For debug needs.*/

    fnet_println("DRAIN");

#endif

    for (prot = fnet_prot_if_list; prot != 0; prot = prot->next)
    {
        if(prot->prot_drain)
            prot->prot_drain();
    }

    fnet_ip_drain();
    fnet_netif_drain();
}
