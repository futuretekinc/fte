#ifndef _FTE_BL_USER_CONFIG_H_
#define _FTE_BL_USER_CONFIG_H_

#include "fapp_model.h"
#define FTE_BL_CFG_LOADER_VERSION           ((1 << 24) | (0 << 16) | (0 << 8) | (0))    
#if FTE_BL
#if FTE_EB1
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_1
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-EB1"
#elif FTE_EB2
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_2
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-EB2"
#elif FTE_EB3
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_3
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-EB3"
#elif FTE_EB4
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_4
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-EB4"
#elif FTE_EB5
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_5
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-EB5"
#elif FTE_EB6
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_6
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-EB6"
#elif FTE_EB7
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_7
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-EB7"
#elif FTE_LB0
    #define FTE_LORA                        1         
    #define FTE_BL_CFG_HARDWARE_VERSION     FTE_BL_MODEL_8
    #define FTE_BL_CFG_PARAMS_DEVICE_NAME   "FTE-LB0"
#else
#error "Target model undefined!"
#endif
#endif

#define FTE_BL_CFG_LOADER_SIZE              (56*1024) /* First 56 KBs of FLASH are reserved by 
                                                        * the Bootloader application.*/
#define FTE_BL_CFG_LOADER_PARAMS_SIZE       (2*1024)
#define FTE_BL_CFG_APP_IMAGE_HEADER_ADDRESS 0x0000F000

#define FTE_BL_CFG_APP_IMAGE_ADDRESS        0x00010000
#define FTE_BL_CFG_APP_IMAGE_SIZE           0x00060000

#define FTE_BL_CFG_APP_DATA_ADDRESS         0x00070000
#define FTE_BL_CFG_APP_DATA_SIZE            0x00010000

#define FTE_BL_CFG_NAME                     "FTE Bootloader" 
#define FTE_BL_CFG_SHELL_PROMPT             "BOOT> "

/* Enable startup boot mode. */
#define FTE_BL_CFG_BOOTLOADER               (1)

#define FTE_BL_CFG_PREINSTALL_INTERRUPTS    (1)

#define FTE_BL_CFG_TELNET_CMD               (1)

/*  "dhcp" command.*/
#define FTE_BL_CFG_DHCP_CMD                 (1)
#define FTE_BL_CFG_DHCP_CMD_DISCOVER_MAX    (10)

/*  "set/get" command.*/
#define FTE_BL_CFG_SETGET_CMD_ID            (1)
#define FTE_BL_CFG_SETGET_CMD_MODEL         (1)
#define FTE_BL_CFG_SETGET_CMD_IP            (1)
#define FTE_BL_CFG_SETGET_CMD_GATEWAY       (1)
#define FTE_BL_CFG_SETGET_CMD_NETMASK       (1)
#define FTE_BL_CFG_SETGET_CMD_MAC           (1)
#define FTE_BL_CFG_SETGET_CMD_BOOT          (1)
#define FTE_BL_CFG_SETGET_CMD_DELAY         (1)
#define FTE_BL_CFG_SETGET_CMD_SCRIPT        (1)
#define FTE_BL_CFG_SETGET_CMD_RAW           (1)
#define FTE_BL_CFG_SETGET_CMD_TFTP          (1)
#define FTE_BL_CFG_SETGET_CMD_IMAGE         (1)
#define FTE_BL_CFG_SETGET_CMD_TYPE          (1)
#define FTE_BL_CFG_SETGET_CMD_GO            (1)

/*  "info" command. */
#define FTE_BL_CFG_INFO_CMD                 (1)

/*  "tftp" command.*/
#define FTE_BL_CFG_TFTP_CMD                 (1)

/*  "tftpup" command.*/
#define FTE_BL_CFG_TFTPUP_CMD               (0)

/*  "tftps" command.*/
#define FTE_BL_CFG_TFTPS_CMD                (0)

/*  "mem" command.*/
#define FTE_BL_CFG_MEM_CMD                  (1)

/*  "erase" command.*/
#define FTE_BL_CFG_ERASE_CMD                (1)

/*  "save" command.*/
#define FTE_BL_CFG_SAVE_CMD                 (1)

/*  "go" command.*/
#define FTE_BL_CFG_GO_CMD                   (1)

/*  "reset" command.*/
#define FTE_BL_CFG_RESET_CMD                (1)

/* Read init parameters from flash.*/
#define FTE_BL_CFG_PARAMS_READ_FLASH        (1)

/* Rewrite init parameters */
#define FTE_BL_CFG_PARAMS_REWRITE_FLASH     (1)

/* CFM protection.*/
#define FTE_BL_CFG_CFM_PROTECTION           (1)

#define FTE_BL_CFG_PARAMS_DEVICE_ID         "00"
/* Default parameters values*/
#define FTE_BL_CFG_PARAMS_TFTP_SERVER       FNET_GEN_ADDR(192, 168, 1, 1)
#define FTE_BL_CFG_PARAMS_BOOT_MODE         FAPP_PARAMS_BOOT_MODE_SCRIPT
#define FTE_BL_CFG_PARAMS_BOOT_SCRIPT       "diag fw; go;! dhcp;! reset;telnet;! reset"

/* Script on TFTP server "WRITE request" received.*/
#define FTE_BL_CFG_TFTPS_ON_WRITE_REQUEST_SCRIPT        "erase all"
/* Script on TFTP server "WRITE request" successfully  completed.*/
#define FTE_BL_CFG_TFTPS_AFTER_WRITE_REQUEST_SCRIPT     "set boot go; save; go"

/* Default startup script.*/
#define FTE_BL_CFG_STARTUP_SCRIPT_ENABLED   (1)
#define FTE_BL_CFG_STARTUP_SCRIPT           "tftps"
#define FTE_BL_CFG_SHELL_MAX_LINE_LENGTH    (128)

#endif

