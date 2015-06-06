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
* @file fnet_eth.c
*
* @author Andrey Butok
*
* @date Apr-20-2011
*
* @version 0.1.7.0
*
* @brief Ethernet platform independent API functions .
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_eth_prv.h"
#include "fnet_stdlib.h"
#include "fnet.h"

/************************************************************************
*     Global Data Structures
*************************************************************************/

/* Null MAC address */
const fnet_mac_addr_t fnet_eth_null_addr =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Broadcast MAC address */
const fnet_mac_addr_t fnet_eth_broadcast =
{
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/************************************************************************
* NAME: fnet_mac_to_str
*
* DESCRIPTION: Converts MAC address to an null-terminated string.
*************************************************************************/
void fnet_mac_to_str( fnet_mac_addr_t addr, char *str_mac )
{
    unsigned char *p;

    p = (unsigned char *)addr;
    fnet_sprintf(str_mac, "%02X:%02X:%02X:%02X:%02X:%02X", p[0], p[1], p[2],
                        p[3], p[4], p[5]);
}

/************************************************************************
* NAME: fnet_str_to_mac
*
* DESCRIPTION: This function interprets the character string into MAC addr.
*************************************************************************/
int fnet_str_to_mac( char *str_mac, fnet_mac_addr_t addr )
{
    unsigned long val;
    char c;
    unsigned long octet[6], *octetptr = octet;
    int i;

    while(1)
    {
        val = 0;

        while((c = *str_mac) != 0)
        {
            if((c >= '0') && (c <= '9'))
            {
                val = (val * 16) + (c - '0');
                str_mac++;
                continue;
            }
            else if(((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')))
            {
                val = (val << 4) + (c + 10 - (((c >= 'a') && (c <= 'f')) ? 'a' : 'A'));
                str_mac++;
                continue;
            }

            break;
        }

        if(*str_mac == ':')
        {
            *octetptr++ = val;
            str_mac++;

            if(octetptr >= octet + 6)
                goto ERROR;

            continue;
        }

        break;
    }
    /* Check for trailing characters. */
    if(*str_mac && !(*str_mac == ' '))
        goto ERROR;

    *octetptr++ = val;

    if((octetptr - octet) == 6)
    {
        for (i = 0; i <= 6; i++)
          addr[i] = (unsigned char)octet[i];
    }
    else
        goto ERROR;

    return (FNET_OK);
    ERROR:
    return (FNET_ERR);
}

/************************************************************************
* NAME: fnet_eth_trace
*
* DESCRIPTION: Prints an Ethernet header. For debug needs only.
*************************************************************************/
#if FNET_CFG_DEBUG_TRACE_ETH
void fnet_eth_trace(char *str, fnet_eth_header_t *eth_hdr)
{
    char mac_str[18];

    fnet_printf(FNET_SERIAL_ESC_FG_GREEN"%s", str); /* Print app-specific header.*/
    fnet_println("[ETH header]"FNET_SERIAL_ESC_FG_BLACK); 
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+/\\/\\/\\/-+");
    fnet_mac_to_str(eth_hdr->destination_addr, mac_str);
    fnet_println("|(Dest)                                                "FNET_SERIAL_ESC_FG_BLUE"%17s"FNET_SERIAL_ESC_FG_BLACK" |", mac_str);
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+/\\/\\/\\/-+");
    fnet_mac_to_str(eth_hdr->source_addr, mac_str);
    fnet_println("|(Src)                                                 "FNET_SERIAL_ESC_FG_BLUE"%17s"FNET_SERIAL_ESC_FG_BLACK" |", mac_str);
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+/\\/\\/\\/-+");
    fnet_println("|(Type)                  0x%04x |", fnet_ntohs(eth_hdr->type));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
}

#endif /* FNET_CFG_DEBUG_TRACE_ETH */

