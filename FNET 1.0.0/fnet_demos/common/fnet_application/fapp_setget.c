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
* @file fapp_setget.c
*
* @author Andrey Butok
*
* @date May-20-2011
*
* @version 0.1.17.0
*
* @brief FNET Application Shell Set/Get routines.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_tftp.h"
#include "fapp_config.h"
#include "fapp_setget.h"
#include "fapp_params_prv.h"
#include "fnet.h"

#if FAPP_CFG_SETGET_CMD

/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_SET_OPT_FORMAT (" %-8s: %s")
#define FAPP_GET_OPT_FORMAT (" %-8s: ")
#define FAPP_GET_SOPT_FORMAT ("%s")
#define FAPP_GET_DOPT_FORMAT ("%d")
#define FAPP_GET_XOPT_FORMAT ("0x%x")


/************************************************************************
*     Function Prototypes
*************************************************************************/
void fapp_set_cmd_ip(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_ip(fnet_shell_desc_t desc);
void fapp_set_cmd_gateway(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_gateway(fnet_shell_desc_t desc);
void fapp_set_cmd_netmask(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_netmask(fnet_shell_desc_t desc);
void fapp_set_cmd_mac(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_mac(fnet_shell_desc_t desc);
void fapp_set_cmd_boot(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_boot(fnet_shell_desc_t desc);
void fapp_set_cmd_bootscript(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_bootscript(fnet_shell_desc_t desc);
void fapp_set_cmd_bootdelay(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_bootdelay(fnet_shell_desc_t desc);
void fapp_set_cmd_tftp(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_tftp(fnet_shell_desc_t desc);
void fapp_set_cmd_image(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_image(fnet_shell_desc_t desc);
void fapp_set_cmd_image_type(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_image_type(fnet_shell_desc_t desc);
void fapp_set_cmd_go(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_go(fnet_shell_desc_t desc);
void fapp_set_cmd_raw(fnet_shell_desc_t desc, char *value );
void fapp_get_cmd_raw(fnet_shell_desc_t desc);

/************************************************************************
*     The set/show parameterr's entry control data structure definition.
*************************************************************************/
typedef struct
{
    char *option;
    void (*set)(fnet_shell_desc_t desc, char *value);
    void (*get)(fnet_shell_desc_t desc);
    char *syntax;
} 
fapp_setget_cmd_t;

/************************************************************************
*    The table of the set/show parameterr's. 
*************************************************************************/
static const fapp_setget_cmd_t fapp_setget_cmd_table [] =
{
#if FAPP_CFG_SETGET_CMD_IP
    { "ip", fapp_set_cmd_ip, fapp_get_cmd_ip, "<board IP address>" },
#endif
#if FAPP_CFG_SETGET_CMD_NETMASK    
    { "netmask", fapp_set_cmd_netmask, fapp_get_cmd_netmask, "<netmask IP address>" },
#endif
#if FAPP_CFG_SETGET_CMD_GATEWAY
    { "gateway", fapp_set_cmd_gateway, fapp_get_cmd_gateway, "<gateway IP address>" },
#endif
#if FAPP_CFG_SETGET_CMD_MAC    
    { "mac", fapp_set_cmd_mac, fapp_get_cmd_mac, "<ethernet address>" },
#endif

/* Bootloader set/get parameters. */
#if FAPP_CFG_SETGET_CMD_BOOT
    { "boot", fapp_set_cmd_boot, fapp_get_cmd_boot, "<stop|go|script>" },
#endif
#if FAPP_CFG_SETGET_CMD_DELAY    
    { "delay", fapp_set_cmd_bootdelay, fapp_get_cmd_bootdelay, "<seconds>" },
#endif
#if FAPP_CFG_SETGET_CMD_SCRIPT    
    { "script", fapp_set_cmd_bootscript, fapp_get_cmd_bootscript, "<command script>" },    
#endif
#if FAPP_CFG_SETGET_CMD_RAW    
    { "raw", fapp_set_cmd_raw, fapp_get_cmd_raw,"0x<address>" }, 
#endif

/* TFTP set/get parameters */
#if FAPP_CFG_SETGET_CMD_TFTP    
    { "tftp", fapp_set_cmd_tftp, fapp_get_cmd_tftp, "<TFTP server IP address>" },   
#endif
#if FAPP_CFG_SETGET_CMD_IMAGE    
    { "image", fapp_set_cmd_image, fapp_get_cmd_image,"<Image-file name to load with TFTP>" },  
#endif
#if FAPP_CFG_SETGET_CMD_TYPE    
    { "type", fapp_set_cmd_image_type, fapp_get_cmd_image_type,"<srec|bin|raw>" },     
#endif
#if FAPP_CFG_SETGET_CMD_GO    
    { "go", fapp_set_cmd_go, fapp_get_cmd_go,"0x<entry point address>" }, 
#endif 
  
};

#define FAPP_SET_CMD_NUM (sizeof(fapp_setget_cmd_table)/sizeof(fapp_setget_cmd_t))

/************************************************************************
* NAME: fapp_set_ip
*
* DESCRIPTION: Set IP address function.
************************************************************************/
static void fapp_set_ip(fnet_shell_desc_t desc, char *value, void (*set_ip)( fnet_netif_desc_t netif_desc, fnet_ip_addr_t ipaddr ) )
{
    fnet_ip_addr_t addr;

    if(inet_aton(value, (struct in_addr *) &addr) == FNET_OK)
    {
        set_ip(fapp_default_netif, addr);
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, value);
    };
}

/************************************************************************
* NAME: fapp_get_ip
*
* DESCRIPTION: Print IP address.
************************************************************************/
static void fapp_get_ip(fnet_shell_desc_t desc, fnet_ip_addr_t (*get_address)( fnet_netif_desc_t netif_desc ))
{
    char ip_str[16];
    struct in_addr addr;
    addr.s_addr = get_address(fapp_default_netif);
    inet_ntoa(addr, ip_str);
    fnet_shell_println(desc, FAPP_GET_SOPT_FORMAT, ip_str);
}

/************************************************************************
* NAME: fapp_set_cmd_ip
*
* DESCRIPTION: Set IP address for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_IP
static void fapp_set_cmd_ip(fnet_shell_desc_t desc, char *value )
{
    fapp_set_ip(desc, value, fnet_netif_set_address );
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_ip
*
* DESCRIPTION: Gets IP address for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_IP
static void fapp_get_cmd_ip(fnet_shell_desc_t desc)
{
    fapp_get_ip(desc, fnet_netif_get_address); 
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_gateway
*
* DESCRIPTION: Sets gateway for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_GATEWAY
static void fapp_set_cmd_gateway(fnet_shell_desc_t desc, char *value )
{
    fapp_set_ip(desc, value, fnet_netif_set_gateway );
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_gateway
*
* DESCRIPTION: Gets gateway for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_GATEWAY
static void fapp_get_cmd_gateway(fnet_shell_desc_t desc)
{
    fapp_get_ip(desc, fnet_netif_get_gateway); 
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_netmask
*
* DESCRIPTION: Sets netmask for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_NETMASK
static void fapp_set_cmd_netmask(fnet_shell_desc_t desc, char *value )
{
    fapp_set_ip(desc, value, fnet_netif_set_netmask);
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_netmask
*
* DESCRIPTION: Gets netmask for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_NETMASK
static void fapp_get_cmd_netmask(fnet_shell_desc_t desc)
{
    fapp_get_ip(desc, fnet_netif_get_netmask); 
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_mac
*
* DESCRIPTION: Sets MAC address for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_MAC 
static void fapp_set_cmd_mac(fnet_shell_desc_t desc, char *value)
{
    fnet_mac_addr_t macaddr;

    if((fnet_str_to_mac(value, macaddr) != FNET_OK) ||
            (fnet_netif_set_hw_addr(fapp_default_netif, macaddr, sizeof(fnet_mac_addr_t)) != FNET_OK))
    {
            fnet_shell_println(desc, FAPP_PARAM_ERR, value);
    }
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_mac
*
* DESCRIPTION: Gets MAC address for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_MAC 
static void fapp_get_cmd_mac(fnet_shell_desc_t desc)
{
    char mac_str[18];
    fnet_mac_addr_t macaddr;

    fnet_netif_get_hw_addr(fapp_default_netif, macaddr, sizeof(fnet_mac_addr_t));
    fnet_mac_to_str(macaddr, mac_str);

    fnet_shell_println(desc, FAPP_GET_SOPT_FORMAT, mac_str);
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_bootscript
*
* DESCRIPTION: Sets boot-command script .
************************************************************************/
#if FAPP_CFG_SETGET_CMD_SCRIPT 
static void fapp_set_cmd_bootscript(fnet_shell_desc_t desc, char *value )
{
    (void)desc;
    fnet_strncpy( fapp_params_boot_config.script, value, FAPP_PARAMS_BOOT_SCRIPT_SIZE );
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_bootscript
*
* DESCRIPTION: Gets boot-command script .
************************************************************************/
#if FAPP_CFG_SETGET_CMD_SCRIPT 
static void fapp_get_cmd_bootscript(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_GET_SOPT_FORMAT, fapp_params_boot_config.script);
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_boot
*
* DESCRIPTION: Sets boot mode.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_BOOT
static void fapp_set_cmd_boot(fnet_shell_desc_t desc, char *value )
{
    const struct boot_mode *mode = fapp_boot_mode_by_name (value);

    if(mode == 0)
        fnet_shell_println(desc, FAPP_PARAM_ERR, value);
    else
        fapp_params_boot_config.mode = mode->index;    

}
#endif
/************************************************************************
* NAME: fapp_get_cmd_boot
*
* DESCRIPTION: Gets boot mode.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_BOOT
static void fapp_get_cmd_boot(fnet_shell_desc_t desc)
{
    const struct boot_mode *mode = fapp_boot_mode_by_index(fapp_params_boot_config.mode);
    fnet_shell_println(desc, FAPP_GET_SOPT_FORMAT, mode->name);        
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_bootdelay
*
* DESCRIPTION: Sets boot delay.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_DELAY 
static void fapp_set_cmd_bootdelay(fnet_shell_desc_t desc, char *value )
{
    unsigned long delay;
    char *p = 0;

    delay = fnet_strtoul(value,&p,0);
    if ((delay == 0) && (p == value))
        fnet_shell_println(desc, FAPP_PARAM_ERR, value); /* Print error mesage. */
    else
        fapp_params_boot_config.delay = delay;     
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_bootdelay
*
* DESCRIPTION: Gets boot delay.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_DELAY
static void fapp_get_cmd_bootdelay(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_GET_DOPT_FORMAT, fapp_params_boot_config.delay);
}
#endif

/************************************************************************
* NAME: fapp_set_cmd_tftp
*
* DESCRIPTION: Set IP address for default network interface.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_TFTP
static void fapp_set_cmd_tftp(fnet_shell_desc_t desc, char *value )
{
    fnet_ip_addr_t addr;

    if(inet_aton(value, (struct in_addr *) &addr) == FNET_OK)
    {
        fapp_params_tftp_config.server = addr;
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, value); /* Print error. */
    }
  
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_tftp
*
* DESCRIPTION: 
************************************************************************/
#if FAPP_CFG_SETGET_CMD_TFTP
static void fapp_get_cmd_tftp(fnet_shell_desc_t desc)
{
    char ip_str[16];
    struct in_addr addr;
    addr.s_addr = fapp_params_tftp_config.server;
    inet_ntoa(addr, ip_str);
    fnet_shell_println(desc, FAPP_GET_SOPT_FORMAT, ip_str);
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_image
*
* DESCRIPTION: 
************************************************************************/
#if FAPP_CFG_SETGET_CMD_IMAGE 
static void fapp_set_cmd_image(fnet_shell_desc_t desc, char *value )
{
    (void)desc;
    fnet_strncpy( fapp_params_tftp_config.file_name, value, FAPP_PARAMS_TFTP_FILE_NAME_SIZE );
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_image
*
* DESCRIPTION: 
************************************************************************/
#if FAPP_CFG_SETGET_CMD_IMAGE 
static void fapp_get_cmd_image(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_GET_SOPT_FORMAT,fapp_params_tftp_config.file_name);
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_image_type
*
* DESCRIPTION:
************************************************************************/
#if FAPP_CFG_SETGET_CMD_TYPE
static void fapp_set_cmd_image_type(fnet_shell_desc_t desc, char *value )
{
    struct image_type *type = fapp_tftp_image_type_by_name (value);

    if(type == 0)
        fnet_shell_println(desc, FAPP_PARAM_ERR, value);
    else
        fapp_params_tftp_config.file_type = type->index;
        
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_image_type
*
* DESCRIPTION:
************************************************************************/
#if FAPP_CFG_SETGET_CMD_TYPE
static void fapp_get_cmd_image_type(fnet_shell_desc_t desc)
{
    struct image_type *type = fapp_tftp_image_type_by_index(fapp_params_tftp_config.file_type);
    fnet_shell_println(desc, FAPP_GET_SOPT_FORMAT, type->name);
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_go
*
* DESCRIPTION: Sets 'go' entry point.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_GO 
static void fapp_set_cmd_go(fnet_shell_desc_t desc, char *value )
{
    unsigned long address;
    char *p = 0;

    address = fnet_strtoul(value,&p,16);
    if ((address == 0) && (p == value))
        fnet_shell_println(desc, FAPP_PARAM_ERR, value); /* Print error mesage. */
    else
        fapp_params_boot_config.go_address = address;     
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_go
*
* DESCRIPTION: Gets 'go' entry point.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_GO 
static void fapp_get_cmd_go(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_GET_XOPT_FORMAT, fapp_params_boot_config.go_address);
}
#endif
/************************************************************************
* NAME: fapp_set_cmd_raw
*
* DESCRIPTION: Sets the default download address for raw binary files.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_RAW 
static void fapp_set_cmd_raw(fnet_shell_desc_t desc, char *value )
{
    unsigned long address;
    char *p = 0;

    address = fnet_strtoul(value,&p,16);
    if ((address == 0) && (p == value))
        fnet_shell_println(desc, FAPP_PARAM_ERR, value); /* Print error mesage. */
    else
        fapp_params_tftp_config.file_raw_address = address;     
}
#endif
/************************************************************************
* NAME: fapp_get_cmd_raw
*
* DESCRIPTION: Gets the default download address for raw binary files.
************************************************************************/
#if FAPP_CFG_SETGET_CMD_RAW 
static void fapp_get_cmd_raw(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, FAPP_GET_XOPT_FORMAT, fapp_params_tftp_config.file_raw_address);
}
#endif
/************************************************************************
* NAME: fapp_set_cmd
*
* DESCRIPTION: Sets system options.
************************************************************************/
void fapp_set_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    int index;

    if(argc == 1) /* Print all set options with syntax description. */
    {
        fnet_shell_println(desc, "Valid 'set' options:");

        for (index = 0; index < FAPP_SET_CMD_NUM; ++index)
        {
            fnet_shell_println(desc, FAPP_SET_OPT_FORMAT, fapp_setget_cmd_table[index].option, fapp_setget_cmd_table[index].syntax);
        }
    }
    else if(argc != 3)
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, "");
    }
    else /* Set parameter */
    {
        for (index = 0; index < FAPP_SET_CMD_NUM; index++)
        {
            if(fnet_strcasecmp(fapp_setget_cmd_table[index].option, argv[1]) == 0)
            {
                fapp_setget_cmd_table[index].set(desc, argv[2]);
                /* Print the result value. */
                fnet_shell_printf(desc, FAPP_GET_OPT_FORMAT, fapp_setget_cmd_table[index].option);
                fapp_setget_cmd_table[index].get(desc);
                return;
            }
        }

        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}

/************************************************************************
* NAME: fapp_show
*
* DESCRIPTION: Shows all system settings.
************************************************************************/
void fapp_get_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    int index;

    if(argc == 1) /* Print all prameters. */
    {
        for (index = 0; index < FAPP_SET_CMD_NUM; ++index)
        {
            fnet_shell_printf(desc, FAPP_GET_OPT_FORMAT, fapp_setget_cmd_table[index].option);
            fapp_setget_cmd_table[index].get(desc);
        }
    }
    else /* Print one parameter. */
    {
       
        for (index = 0; index < FAPP_SET_CMD_NUM; index++)
        {
            if(fnet_strcasecmp(fapp_setget_cmd_table[index].option, argv[1]) == 0)
            {
                fnet_shell_printf(desc, FAPP_GET_OPT_FORMAT, fapp_setget_cmd_table[index].option);
                fapp_setget_cmd_table[index].get(desc);
                return;
            }
        }

        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
    
}

#endif



