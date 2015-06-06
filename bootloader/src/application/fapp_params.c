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
* @file fapp_params.c
*
* @author Andrey Butok
*
* @date May-24-2011
*
* @version 0.1.16.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_mem.h"
#include "fapp_params.h"
#include "fapp_diag.h"
#include <stdio.h>
#include "common.h"

#if FNET_CFG_FLASH
#define fapp_params_erase   fnet_flash_erase
#define fapp_params_memcpy  fnet_flash_memcpy
#endif

//#if FNET_CFG_MK_FTFL
//#define fapp_params_erase   fnet_ftfl_erase
//#define fapp_params_memcpy  fnet_ftfl_memcpy
//#endif

#if FTE_BL_CFG_PARAMS_REWRITE_FLASH
    /* Default parameter values.
    * One-time setup done as a part of the manufacturing process/flashing.
    * Into non-volatile storage
    */
    /* Default application parameters (in ROM) set during flashing. */
#if FNET_CFG_COMP_CW
    #pragma define_section fapp_params ".fapp_params" far_absolute RW
    __declspec(fapp_params)
#endif
#if FNET_CFG_COMP_IAR
//    #pragma segment="fapp_params"
//    #pragma location="fapp_params" 
    #if FNET_MK      
        __root
    #endif      
#endif 
#endif /* FTE_BL_CFG_PARAMS_REWRITE_FLASH */


/* Local confiruration parameters.
 * Will be overwritten by parameters from flash if FTE_BL_CFG_PARAMS_READ_FLASH set to 1
 */
const FTE_BL_STATIC_PARAMS xStaticDefaultParams =      
{
    .signature  = FTE_BL_PARAMS_SIGNATURE,
    .index      = 0,
    .device =  
    {
        .version=
        {
            .loader     = FTE_BL_CFG_LOADER_VERSION,
            .hardware   = FTE_BL_CFG_HARDWARE_VERSION
        },
        .id     = FTE_BL_CFG_PARAMS_DEVICE_ID,
        .model  = FTE_BL_CFG_PARAMS_DEVICE_NAME,
        .mac    = FAPP_CFG_PARAMS_MAC_ADDR, /* MAC address */
    },
    .ulCRC32 = 0
};
    
const FTE_BL_DYNAMIC_PARAMS xDynamicDefaultParams =      
{    
    .signature  = FTE_BL_PARAMS_SIGNATURE,
    .index      = 0,
    .fnet =
    {
        .address= FAPP_CFG_PARAMS_IP_ADDR,  /* address */
        .netmask= FAPP_CFG_PARAMS_IP_MASK,  /* netmask */
        .gateway= FAPP_CFG_PARAMS_IP_GW,    /* gateway */
    },
#if FAPP_CFG_PARAMS_BOOT 
    .boot = 
    {
        .mode       = FTE_BL_CFG_PARAMS_BOOT_MODE,   /* boot */
        .delay      = FTE_BL_CFG_PARAMS_BOOT_DELAY, /* boot_delay */
        .go_address = FTE_BL_CFG_PARAMS_BOOT_GO_ADDRESS,    /* go_address */                                      
        .script     = FTE_BL_CFG_PARAMS_BOOT_SCRIPT
            
    },
#endif

#if FAPP_CFG_PARAMS_TFTP 
    .tftp = 
    {
        .server     = FTE_BL_CFG_PARAMS_TFTP_SERVER,    /* tftp_ip */
        .file_type  = FTE_BL_CFG_PARAMS_TFTP_FILE_TYPE, /* image_type */  
        .file_raw_address = FTE_BL_CFG_PARAMS_TFTP_FILE_RAW_ADDRESS,   /* raw_address */ 
        .file_name  = FTE_BL_CFG_PARAMS_TFTP_FILE_NAME, /* image */
    },
    .ulCRC32 = 0
};
#endif

FTE_BL_STATIC_PARAMS xStaticParams;
FTE_BL_DYNAMIC_PARAMS xDynamicParams;
/************************************************************************
* NAME: fapp_static_params_to_flash
*
* DESCRIPTION: Save static configuration parameters to the flash.
************************************************************************/
#if FTE_BL_CFG_SAVE_CMD
int fapp_static_params_to_flash() 
{
 
    FTE_BL_NET_PARAMS       fnet_params;
    FTE_BL_STATIC_PARAMS*   fapp_static_params  = (FTE_BL_STATIC_PARAMS *)FTE_BL_STATIC_PARAMS_ADDRESS;
    
    
    /* Erase one paage allocated for configuration parameters.*/
    fapp_params_erase( (void *)(fapp_static_params), sizeof(FTE_BL_STATIC_PARAMS));
    
    /* Simple check if erased. */
    if( fnet_memcmp((void *)(fapp_static_params), FTE_BL_PARAMS_SIGNATURE, sizeof(FTE_BL_PARAMS_SIGNATURE)) ==0 )
    {
        return  FNET_ERR;
    }
    
    
    /* Write FNET parameters to the flash.*/
    fnet_netif_get_hw_addr(fapp_default_netif, xStaticParams.device.mac, sizeof(fnet_mac_addr_t));
    
    xStaticParams.ulCRC32 = fapp_crc32(0, &xStaticParams, sizeof(xStaticParams) - sizeof(unsigned long));
    
    fapp_params_memcpy( (void *)fapp_static_params, &xStaticParams, sizeof(FTE_BL_STATIC_PARAMS)  );
        
    /* Simple check if it was written. */
    if( fnet_memcmp((void *)(fapp_static_params), FTE_BL_PARAMS_SIGNATURE, sizeof(FTE_BL_PARAMS_SIGNATURE)) == 0 )
    {
        return FNET_OK;
    }
    
    return  FNET_ERR;
}
#endif

/************************************************************************
* NAME: fapp_static_params_init
*
* DESCRIPTION: Init static configuration parameters.
************************************************************************/
#if FTE_BL_CFG_PARAMS_READ_FLASH 
int fapp_static_params_init()
{
    unsigned long   ulTemp;
    
    memcpy(&xStaticParams, &xStaticDefaultParams, sizeof(FTE_BL_STATIC_PARAMS));
    
    sprintf((char *)&xStaticParams.device.id[0],  "%08x",(unsigned long)SIM_UIDH_REG(SIM_BASE_PTR));
    sprintf((char *)&xStaticParams.device.id[8],  "%08x",(unsigned long)SIM_UIDMH_REG(SIM_BASE_PTR));
    sprintf((char *)&xStaticParams.device.id[16], "%08x",(unsigned long)SIM_UIDML_REG(SIM_BASE_PTR));
    sprintf((char *)&xStaticParams.device.id[24], "%08x",(unsigned long)SIM_UIDL_REG(SIM_BASE_PTR));
    
    ulTemp  = (unsigned long)SIM_UIDMH_REG(SIM_BASE_PTR);
    ulTemp += (unsigned long)SIM_UIDML_REG(SIM_BASE_PTR);
    ulTemp += (unsigned long)SIM_UIDL_REG(SIM_BASE_PTR);
    
    xStaticParams.device.mac[0] = FTE_CFG_NET_OUI_0;
    xStaticParams.device.mac[1] = FTE_CFG_NET_OUI_1;
    xStaticParams.device.mac[2] = FTE_CFG_NET_OUI_2;
    
    xStaticParams.device.mac[3] = ((unsigned char *)&ulTemp)[0];
    xStaticParams.device.mac[4] = ((unsigned char *)&ulTemp)[1];
    xStaticParams.device.mac[5] = ((unsigned char *)&ulTemp)[2] + 100;
    
    fnet_netif_set_hw_addr(fapp_default_netif, xStaticParams.device.mac, sizeof(fnet_mac_addr_t));
    
    return FNET_OK;
}
#endif


/************************************************************************
* NAME: fapp_static_params_from_flash
*
* DESCRIPTION: Load static configuration parameters from flash.
************************************************************************/
#if FTE_BL_CFG_PARAMS_READ_FLASH 
int fapp_static_params_from_flash()
{
    FTE_BL_STATIC_PARAMS    *fnet_static_params = (FTE_BL_STATIC_PARAMS *)FTE_BL_STATIC_PARAMS_ADDRESS;
    
    /* Check signature. */
    if((fnet_strncmp( fnet_static_params->signature, FTE_BL_PARAMS_SIGNATURE, FTE_BL_PARAMS_SIGNATURE_SIZE ) !=0 ) ||
        (fnet_static_params->ulCRC32 != fapp_crc32(0, fnet_static_params, sizeof(FTE_BL_STATIC_PARAMS) - sizeof(unsigned long))))
    {
        return  FNET_ERR;
    }

    fnet_memcpy(&xStaticParams, fnet_static_params, sizeof(FTE_BL_STATIC_PARAMS));

    fnet_netif_set_hw_addr(fapp_default_netif, xStaticParams.device.mac, sizeof(fnet_mac_addr_t));
    
    return FNET_OK;
}
#endif

/************************************************************************
* NAME: fapp_dynamic_params_init
*
* DESCRIPTION: Init dynamic configuration parameters.
************************************************************************/
#if FTE_BL_CFG_PARAMS_READ_FLASH 
int fapp_dynamic_params_init()
{
    memcpy(&xDynamicParams, &xDynamicDefaultParams, sizeof(FTE_BL_DYNAMIC_PARAMS));
    
    return FNET_OK;
}
#endif

/************************************************************************
* NAME: fapp_dynamic_params_from_flash
*
* DESCRIPTION: Load dynamic configuration parameters from flash.
************************************************************************/
#if FTE_BL_CFG_PARAMS_READ_FLASH 
int fapp_dynamic_params_from_flash()
{
    FTE_BL_DYNAMIC_PARAMS   *fnet_dynamic_params = (FTE_BL_DYNAMIC_PARAMS *)FTE_BL_DYNAMIC_PARAMS_ADDRESS;
    int                     result = FNET_OK;
    
    /* Check signature. */
    if((fnet_strncmp( fnet_dynamic_params->signature, FTE_BL_PARAMS_SIGNATURE, FTE_BL_PARAMS_SIGNATURE_SIZE) !=0 ) ||
        (fnet_dynamic_params->ulCRC32 != fapp_crc32(0, fnet_dynamic_params, sizeof(FTE_BL_DYNAMIC_PARAMS) - sizeof(unsigned long))))
    {
        return  FNET_ERR;
    }
    
    /* FNET stack parameters. */
    fnet_netif_set_address(fapp_default_netif, fnet_dynamic_params->fnet.address); 
    fnet_netif_set_gateway(fapp_default_netif, fnet_dynamic_params->fnet.gateway);       
    fnet_netif_set_netmask(fapp_default_netif, fnet_dynamic_params->fnet.netmask);
    
    fnet_memcpy(&xDynamicParams, fnet_dynamic_params, sizeof(FTE_BL_DYNAMIC_PARAMS));
    
    return FNET_OK;
}
#endif

/************************************************************************
* NAME: fapp_dynamic_params_to_flash
*
* DESCRIPTION: Save dynamic configuration parameters to the flash.
************************************************************************/
#if FTE_BL_CFG_SAVE_CMD
int fapp_dynamic_params_to_flash() 
{
 
    FTE_BL_NET_PARAMS       fnet_params;
    FTE_BL_DYNAMIC_PARAMS*  fapp_dynamic_params = (FTE_BL_DYNAMIC_PARAMS *)FTE_BL_DYNAMIC_PARAMS_ADDRESS;
    fnet_ip_addr_t          ip_address;
    
    /* Save IP address only if it was allocated manually/statically. */
    if(fnet_netif_address_automatic(fapp_default_netif) != 0)
    {
        ip_address = fapp_dynamic_params->fnet.address;              /* Preserve the old value.*/
    }
    else
    {
        ip_address = fnet_netif_get_address(fapp_default_netif);    /* Take the current value. */
    }
    
    
    /* Erase one paage allocated for configuration parameters.*/
    fapp_params_erase( (void *)(fapp_dynamic_params), sizeof(FTE_BL_DYNAMIC_PARAMS));
    
    /* Simple check if erased. */
    if( fnet_memcmp((void *)(fapp_dynamic_params), FTE_BL_PARAMS_SIGNATURE, sizeof(FTE_BL_PARAMS_SIGNATURE)) ==0 )
    {
        return  FNET_ERR;
    }
   
    /* Write FNET parameters to the flash.*/
    xDynamicParams.fnet.address = ip_address;
    xDynamicParams.fnet.netmask = fnet_netif_get_netmask(fapp_default_netif);
    xDynamicParams.fnet.gateway = fnet_netif_get_gateway(fapp_default_netif);
    
    xDynamicParams.ulCRC32 = fapp_crc32(0, &xDynamicParams, sizeof(xDynamicParams) - sizeof(unsigned long));
    
    fapp_params_memcpy( (void *)fapp_dynamic_params, &xDynamicParams, sizeof(FTE_BL_DYNAMIC_PARAMS)  );
    
    /* Simple check if it was written. */
    if( fnet_memcmp((void *)(fapp_dynamic_params), FTE_BL_PARAMS_SIGNATURE, sizeof(FTE_BL_PARAMS_SIGNATURE)) == 0 )
    {
        return FNET_OK;
    }
    
    return FNET_ERR;
}
#endif
