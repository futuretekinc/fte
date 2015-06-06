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
* @file fapp_config.h
*
* @author Andrey Butok
*
* @date May-11-2011
*
* @version 0.1.20.0
*
* @brief FNET Demo Application Configuration.
*
***************************************************************************/
#ifndef _FAPP_CONFIG_H_

#define _FAPP_CONFIG_H_
#include "fnet_config.h"
#include "fapp_user_config.h"

/**************************************************************************/ /*!
 * @def      FTE_BL_CFG_NAME
 * @brief    Name of the application. 
 * @showinitializer
 ******************************************************************************/
#ifndef FTE_BL_CFG_NAME
    #define FTE_BL_CFG_NAME         "FTE Application"       
#endif

/**************************************************************************/ /*!
 * @def      FTE_BL_CFG_SHELL_PROMPT
 * @brief    Prompt message used by the application shell. 
 * @showinitializer
 ******************************************************************************/
#ifndef FTE_BL_CFG_SHELL_PROMPT
    #define FTE_BL_CFG_SHELL_PROMPT "FNET> "
#endif    

/**************************************************************************/ /*!
 * @def      FTE_BL_CFG_PARAMS_READ_FLASH
 * @brief    Reading of the configuration parameters from the Flash memory during
 *           the application bootup:
 *               - @c 1 = is enabled. @n
 *                        During bootup, the application read 
 *                        and use the most recently stored customer 
 *                        configuration data from the flash.
 *               - @c 0 = is disabled. @n
 *                        During bootup, the application uses 
 *                        compile-time parameters by default. 
 * @showinitializer
 ******************************************************************************/
#ifndef FTE_BL_CFG_PARAMS_READ_FLASH
    #define FTE_BL_CFG_PARAMS_READ_FLASH    (0)
#endif

/**************************************************************************/ /*!
 * @def      FTE_BL_CFG_PARAMS_REWRITE_FLASH
 * @brief    Rewriting of the configuration parameters in the Flash memory 
 *           duiring flashing of the application:
 *               - @c 1 = is enabled. @n
 *                        It will allocated the default parameter structure 
 *                        which  will rewrite the configurated parameters 
 *                        present in the Flash memory during flashing of 
 *                        the application.
 *               - @c 0 = is disabled. @n
 *                        The configuration parameters that present in the 
 *                        flash memory will stay untouchable during flashing. 
 * @showinitializer
 ******************************************************************************/
#ifndef FTE_BL_CFG_PARAMS_REWRITE_FLASH
    #define FTE_BL_CFG_PARAMS_REWRITE_FLASH (0)
#endif

/**************************************************************************/ /*!
 * @def      FTE_BL_CFG_BOOTLOADER
 * @brief    Bootloader:
 *               - @c 1 = is enabled. @n
 *                        The application startup behaviour is defined 
 *                        by the @ref fapp_params_boot_mode_t mode.
 *               - @c 0 = is disabled. @n
 * @showinitializer
 ******************************************************************************/
#ifndef FTE_BL_CFG_BOOTLOADER
    #define FTE_BL_CFG_BOOTLOADER               (0)
#endif

/* CFM protection is enabled.
 * cfm.c have to be updated aklso.
 */
#ifndef FTE_BL_CFG_CFM_PROTECTION
    #define FTE_BL_CFG_CFM_PROTECTION           (0)
#endif

/* Startup script. */
#ifndef FTE_BL_CFG_STARTUP_SCRIPT_ENABLED
    #define FTE_BL_CFG_STARTUP_SCRIPT_ENABLED   (0)
#endif

#ifndef FTE_BL_CFG_STARTUP_SCRIPT
    #define FTE_BL_CFG_STARTUP_SCRIPT           ""
#endif

/* Preinstall FNET ISR into vector table.*/
#ifndef FTE_BL_CFG_PREINSTALL_INTERRUPTS
    #define FTE_BL_CFG_PREINSTALL_INTERRUPTS      (0)
#endif

#ifndef FTE_BL_CFG_PARAMS_DEVICE_ID
    #define FTE_BL_CFG_PARAMS_DEVICE_ID         (0)
#endif

#ifndef FTE_BL_CFG_PARAMS_DEVICE_NAME
#if FTE_BL
    #error "Undefined device name!\n"
#endif
#endif
/************************************************************************
* Default application parameters (allocated in the Flash memory).
*************************************************************************/
/* FNET TCP/IP stack default parameters ROM. */
#ifndef FAPP_CFG_PARAMS_IP_ADDR
    #define FAPP_CFG_PARAMS_IP_ADDR             FNET_CFG_ETH_IP_ADDR    /* Defined by FNET */
#endif

#ifndef FAPP_CFG_PARAMS_IP_MASK
    #define FAPP_CFG_PARAMS_IP_MASK             FNET_CFG_ETH_IP_MASK    /* Defined by FNET */
#endif

#ifndef FAPP_CFG_PARAMS_IP_GW
    #define FAPP_CFG_PARAMS_IP_GW               FNET_CFG_ETH_IP_GW      /* Defined by FNET */
#endif

#ifndef FAPP_CFG_PARAMS_MAC_ADDR
    #define FAPP_CFG_PARAMS_MAC_ADDR            (0) /* Defined by FNET */
#endif

/* Bootloader parameters. */
#ifndef FTE_BL_CFG_PARAMS_BOOT_MODE
    #define FTE_BL_CFG_PARAMS_BOOT_MODE         FAPP_PARAMS_BOOT_MODE_STOP 
#endif

#ifndef FTE_BL_CFG_PARAMS_BOOT_DELAY
    #define FTE_BL_CFG_PARAMS_BOOT_DELAY       (3) /*sec*/
#endif

#ifndef FTE_BL_CFG_PARAMS_BOOT_GO_ADDRESS
    #define FTE_BL_CFG_PARAMS_BOOT_GO_ADDRESS   (0x10421) 
#endif

#ifndef FTE_BL_CFG_PARAMS_BOOT_SCRIPT
    #define FTE_BL_CFG_PARAMS_BOOT_SCRIPT       ""
#endif

/* TFTP parameters */
#ifndef FTE_BL_CFG_PARAMS_TFTP_SERVER
    #define FTE_BL_CFG_PARAMS_TFTP_SERVER       FNET_GEN_ADDR(0, 0, 0, 0)
#endif

#ifndef FTE_BL_CFG_PARAMS_TFTP_FILE_TYPE
    #define FTE_BL_CFG_PARAMS_TFTP_FILE_TYPE    FAPP_PARAMS_TFTP_FILE_TYPE_RAW
#endif

#ifndef FTE_BL_CFG_PARAMS_TFTP_FILE_NAME
    #define FTE_BL_CFG_PARAMS_TFTP_FILE_NAME    FTE_BL_CFG_PARAMS_DEVICE_NAME ".bin"
#endif

#ifndef FTE_BL_CFG_PARAMS_TFTP_FILE_RAW_ADDRESS
    #define FTE_BL_CFG_PARAMS_TFTP_FILE_RAW_ADDRESS    (0xF000) 
#endif



/************************************************************************
*    "info" command.
*************************************************************************/
#ifndef FTE_BL_CFG_INFO_CMD
    #define FTE_BL_CFG_INFO_CMD         (0)
#endif

/************************************************************************
*    "dhcp" command.
*************************************************************************/
#ifndef FTE_BL_CFG_DHCP_CMD
    #define FTE_BL_CFG_DHCP_CMD         (0)
    
    #ifndef FTE_BL_CFG_DHCP_CMD_DISCOVER_MAX
        #define FTE_BL_CFG_DHCP_CMD_DISCOVER_MAX    (-1) /* -1 means infinite. */
    #endif
    
#endif

/************************************************************************
*    "http" command.
*************************************************************************/
#ifndef FAPP_CFG_HTTP_CMD
    #define FAPP_CFG_HTTP_CMD           (0)
#endif

/************************************************************************
*    "exp" command.
*************************************************************************/
#ifndef FAPP_CFG_EXP_CMD
    #define FAPP_CFG_EXP_CMD            (0)
#endif

/************************************************************************
*    "tftp" command.
*************************************************************************/
#ifndef FTE_BL_CFG_TFTP_CMD
    #define FTE_BL_CFG_TFTP_CMD         (0)
#endif

/************************************************************************
*    "tftpup" command.
*************************************************************************/
#ifndef FTE_BL_CFG_TFTPUP_CMD
    #define FTE_BL_CFG_TFTPUP_CMD       (0)
#endif

/************************************************************************
*    "tftps" command.
*************************************************************************/
#ifndef FTE_BL_CFG_TFTPS_CMD
    #define FTE_BL_CFG_TFTPS_CMD        (0)
#endif

/************************************************************************
*    "telnet" command.
*************************************************************************/
#ifndef FTE_BL_CFG_TELNET_CMD 
    #define FTE_BL_CFG_TELNET_CMD       (0)
#endif

#ifndef FAPP_CFG_TELNET_CMD_OWN_SHELL
    #define FAPP_CFG_TELNET_CMD_OWN_SHELL   (0)
#endif

/************************************************************************
*    "mem" command.
*************************************************************************/
#ifndef FTE_BL_CFG_MEM_CMD
    #define FTE_BL_CFG_MEM_CMD          (0)
#endif  

/************************************************************************
*    "erase" command.
*************************************************************************/
#ifndef FTE_BL_CFG_ERASE_CMD
    #define FTE_BL_CFG_ERASE_CMD       (0)
#endif 

/************************************************************************
*    "save" command.
*************************************************************************/
#ifndef FTE_BL_CFG_SAVE_CMD
    #define FTE_BL_CFG_SAVE_CMD         (0)
#endif 

/************************************************************************
*    "go" command.
*************************************************************************/
#ifndef FTE_BL_CFG_GO_CMD
    #define FTE_BL_CFG_GO_CMD           (0)
#endif 

/************************************************************************
*    "reset" command.
*************************************************************************/
#ifndef FTE_BL_CFG_RESET_CMD
    #define FTE_BL_CFG_RESET_CMD        (0)
#endif 

/************************************************************************
*    "bench" command.
*************************************************************************/
#ifndef FAPP_CFG_BENCH_CMD
    #define FAPP_CFG_BENCH_CMD          (0)
#endif 

/************************************************************************
*    "reinit" command. Used to test FNET release/init only.
*************************************************************************/
#ifndef FAPP_CFG_REINIT_CMD
    #define FAPP_CFG_REINIT_CMD         (0)
#endif 

/************************************************************************
*    "test" command. Used to test Telnet sending. For debug needs only
*************************************************************************/
#ifndef FAPP_CFG_TELNET_TEST_CMD
    #define FAPP_CFG_TELNET_TEST_CMD    (0)
#endif 



/************************************************************************
*    "set/get" command.
*************************************************************************/
/* FNET TCP/IP stack set/get parameters. */
#ifndef FTE_BL_CFG_SETGET_CMD_IP
    #define FTE_BL_CFG_SETGET_CMD_IP      (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_GATEWAY
    #define FTE_BL_CFG_SETGET_CMD_GATEWAY (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_NETMASK
    #define FTE_BL_CFG_SETGET_CMD_NETMASK (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_MAC
    #define FTE_BL_CFG_SETGET_CMD_MAC     (0)
#endif

/* Bootloader set/get parameters. */
#ifndef FTE_BL_CFG_SETGET_CMD_BOOT
    #define FTE_BL_CFG_SETGET_CMD_BOOT    (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_DELAY
    #define FTE_BL_CFG_SETGET_CMD_DELAY   (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_SCRIPT
    #define FTE_BL_CFG_SETGET_CMD_SCRIPT  (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_RAW
    #define FTE_BL_CFG_SETGET_CMD_RAW     (0)
#endif

/* TFTP set/get parameters */
#ifndef FTE_BL_CFG_SETGET_CMD_TFTP
    #define FTE_BL_CFG_SETGET_CMD_TFTP    (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_IMAGE
    #define FTE_BL_CFG_SETGET_CMD_IMAGE   (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_TYPE
    #define FTE_BL_CFG_SETGET_CMD_TYPE    (0)
#endif
#ifndef FTE_BL_CFG_SETGET_CMD_GO
    #define FTE_BL_CFG_SETGET_CMD_GO      (0)
#endif


#ifndef FTE_BL_CFG_SETGET_CMD
#define FTE_BL_CFG_SETGET_CMD            (FTE_BL_CFG_SETGET_CMD_IP|FTE_BL_CFG_SETGET_CMD_GATEWAY|\
                                         FTE_BL_CFG_SETGET_CMD_NETMASK|FTE_BL_CFG_SETGET_CMD_MAC|\
                                         FTE_BL_CFG_SETGET_CMD_BOOT|FTE_BL_CFG_SETGET_CMD_DELAY|\
                                         FTE_BL_CFG_SETGET_CMD_SCRIPT|FTE_BL_CFG_SETGET_CMD_RAW|\
                                         FTE_BL_CFG_SETGET_CMD_TFTP|FTE_BL_CFG_SETGET_CMD_IMAGE|\
                                         FTE_BL_CFG_SETGET_CMD_TYPE|FTE_BL_CFG_SETGET_CMD_GO)
#endif


#ifndef FAPP_CFG_PARAMS_BOOT
#define FAPP_CFG_PARAMS_BOOT            (FTE_BL_CFG_SETGET_CMD_BOOT|FTE_BL_CFG_SETGET_CMD_DELAY|\
                                         FTE_BL_CFG_SETGET_CMD_SCRIPT|FTE_BL_CFG_SETGET_CMD_RAW|\
                                         FTE_BL_CFG_SETGET_CMD_GO)
#endif

#ifndef FAPP_CFG_PARAMS_TFTP
#define FAPP_CFG_PARAMS_TFTP            (FTE_BL_CFG_SETGET_CMD_TFTP|FTE_BL_CFG_SETGET_CMD_IMAGE|\
                                         FTE_BL_CFG_SETGET_CMD_TYPE|FTE_BL_CFG_SETGET_CMD_GO|\
                                         FTE_BL_CFG_TFTP_CMD|FTE_BL_CFG_TFTPUP_CMD|FTE_BL_CFG_TFTPS_CMD)
#endif



#ifndef FTE_BL_CFG_TFTPS_ON_WRITE_REQUEST_SCRIPT
    #define FTE_BL_CFG_TFTPS_ON_WRITE_REQUEST_SCRIPT    ""
#endif

#ifndef FTE_BL_CFG_TFTPS_AFTER_WRITE_REQUEST_SCRIPT
    #define FTE_BL_CFG_TFTPS_AFTER_WRITE_REQUEST_SCRIPT ""
#endif


/************************************************************************
*    TFTP data handlers
*************************************************************************/
#ifndef FAPP_CFG_TFTP_RX_RAW
    #define FAPP_CFG_TFTP_RX_RAW            (1)
#endif

#ifndef FAPP_CFG_TFTP_TX_RAW
    #define FAPP_CFG_TFTP_TX_RAW            (1)
#endif

#ifndef FAPP_CFG_TFTP_RX_BIN
    #define FAPP_CFG_TFTP_RX_BIN            (1)
#endif

#ifndef FAPP_CFG_TFTP_TX_BIN
    #define FAPP_CFG_TFTP_TX_BIN            (1)
#endif

#ifndef FAPP_CFG_TFTP_RX_SREC
    #define FAPP_CFG_TFTP_RX_SREC           (1)
#endif

#ifndef FAPP_CFG_TFTP_TX_SREC
    #define FAPP_CFG_TFTP_TX_SREC           (1)
#endif


#if !FTE_BL_CFG_TFTP_CMD && !FTE_BL_CFG_TFTPUP_CMD && !FTE_BL_CFG_TFTPS_CMD
    #undef FAPP_CFG_TFTP_RX_RAW
    #undef FAPP_CFG_TFTP_TX_RAW
    #undef FAPP_CFG_TFTP_RX_BIN
    #undef FAPP_CFG_TFTP_TX_BIN
    #undef FAPP_CFG_TFTP_RX_SREC
    #undef FAPP_CFG_TFTP_TX_SREC
    #define FAPP_CFG_TFTP_RX_RAW            (0)
    #define FAPP_CFG_TFTP_TX_RAW            (0)
    #define FAPP_CFG_TFTP_RX_BIN            (0)
    #define FAPP_CFG_TFTP_TX_BIN            (0)
    #define FAPP_CFG_TFTP_RX_SREC           (0)
    #define FAPP_CFG_TFTP_TX_SREC           (0)
#endif


/************************************************************************/
#ifndef FTE_BL_CFG_SHELL_MAX_LINE_LENGTH
    #define FTE_BL_CFG_SHELL_MAX_LINE_LENGTH    (60)
#endif 

/************************************************************************
*    Memory parameters
*************************************************************************/
/* Start address of the ROM memory, reserved/protected for the application. Used by the bootloader application.*/
#ifndef FTE_BL_CFG_LOADER_ADDRESS
#define FTE_BL_CFG_LOADER_ADDRESS       FNET_CFG_CPU_FLASH_ADDRESS
#endif

/* ROM memory size, reserved/protected for the application. Used by the bootloader application.*/
#ifndef FTE_BL_CFG_LOADER_SIZE          
#define FTE_BL_CFG_LOADER_SIZE         (56*1024) /* 56 KB */
#endif

#endif