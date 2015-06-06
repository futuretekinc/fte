#include "fapp.h"
#include "fapp_prv.h"

#if FTE_BL_CFG_SETGET_CMD
#include "fapp_setget.h"
#endif

#include "fapp_mem.h"

#if FTE_BL_CFG_DHCP_CMD && FNET_CFG_DHCP

#include "fapp_dhcp.h"

#endif

#if (FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP)|| (FAPP_CFG_EXP_CMD && FNET_CFG_FS)

#include "fapp_http.h"
#include "fapp_fs.h"

#endif

#if FTE_BL_CFG_TFTP_CMD || FTE_BL_CFG_TFTPUP_CMD || FTE_BL_CFG_TFTPS_CMD

#include "fapp_tftp.h"

#endif

#if FTE_BL_CFG_TELNET_CMD && FNET_CFG_TELNET

#include "fapp_telnet.h"

#endif

#if FAPP_CFG_BENCH_CMD

#include "fapp_bench.h"

#endif

#include "fnet_netbuf.h"
#include "fapp_diag.h"

/************************************************************************
*     Definitions.
*************************************************************************/
const char FTE_BL_DELIMITER_STR[] = "************************************************";
const char FTE_BL_CANCELLED_STR[] = "\nCancelled";
const char FTE_BL_TOCANCEL_STR[] = "Press [Ctr+C] to cancel.";

/* Error mesages */
const char FTE_BL_PARAM_ERR[] = "Error: Invalid paremeter \'%s\'";
const char FTE_BL_NET_ERR[]   = "Error: Network Interface is not configurated!\n";
const char FTE_BL_INIT_ERR[]  = "Error: %s initialization is failed!";



const char FTE_BL_SHELL_INFO_FORMAT_VER[]  = " %-16s : %d.%d.%d.%d";
const char FTE_BL_SHELL_INFO_FORMAT_S[]  = " %-16s : %s";
const char FTE_BL_SHELL_INFO_FORMAT_D[]  = " %-16s : %u";

const char FTE_BL_SHELL_INFO_ENABLED[]  = "enabled";
const char FTE_BL_SHELL_INFO_DISABLED[]  = "disabled";

/* Service release command */
const char FTE_BL_COMMAND_RELEASE [] = "release";

const char FTE_BL_GO_STR[] = "go 0x%08X";
const char FTE_BL_SAVE_STR[] = "Parameters saved";
const char FTE_BL_SAVE_FAILED_STR[] = "Parameters saving failed!";
const char FTE_BL_BOOT_STR[] = "Press any key to stop (%s): %3d";

const char FTE_BL_STATIC_PARAMS_LOAD_STR[] = "\n\nStatic parameters loaded from Flash.\n";
const char FTE_BL_DYNAMIC_PARAMS_LOAD_STR[] = "\n\nDynamic parameters loaded from Flash.\n";

const char FTE_BL_DUP_IP_WARN[] = "\n%s: %s has IP address conflict with another system on the network!\n";


fnet_netif_desc_t fapp_default_netif; /* Default netif. */

/************************************************************************
*     Function Prototypes
*************************************************************************/
void FTE_BL_SHELL_init( fnet_shell_desc_t desc );
static void FTE_BL_bootModeGo(fnet_shell_desc_t desc);
static void FTE_BL_bootModeScript(fnet_shell_desc_t desc);
static void FTE_BL_bootModeRun(fnet_shell_desc_t desc);
static void FTE_BL_LED_init(void);

#if FAPP_CFG_REINIT_CMD 
int fapp_reinit_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
#endif

int fapp_clr_ret_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );

/************************************************************************
*     The table of the main shell commands.
*************************************************************************/
const struct fnet_shell_command fapp_cmd_table [] =
{
    { FNET_SHELL_CMD_TYPE_NORMAL, "help", 0, 0, (void *)fapp_help_cmd,"Display this help message", ""},

#if FTE_BL_CFG_SETGET_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "set", 0, 2, (void *)fapp_set_cmd,    "Set parameter", "[<parameter> <value>]"},
    { FNET_SHELL_CMD_TYPE_NORMAL, "get", 0, 1, (void *)fapp_get_cmd,    "Get parameters", "[<parameter>]" },
#endif    
#if FTE_BL_CFG_INFO_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "info", 0, 0, (void *)fapp_info_cmd,  "Show detailed status", ""},
#endif
#if FTE_BL_CFG_DHCP_CMD && FNET_CFG_DHCP
    { FNET_SHELL_CMD_TYPE_NORMAL, "dhcp", 0, 1, (void *)fapp_dhcp_cmd,  "Start DHCP client", "[release]"},
#endif
#if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP
    { FNET_SHELL_CMD_TYPE_NORMAL, "http", 0, 1, (void *)fapp_http_cmd,  "Start HTTP Server", "[release]"},
#endif
#if FAPP_CFG_EXP_CMD && FNET_CFG_FS
    { FNET_SHELL_CMD_TYPE_SHELL,  "exp", 0, 1, (void *)&fapp_fs_shell,  "File Explorer submenu...", ""},
#endif    
#if FTE_BL_CFG_TFTP_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "tftp", 0, 3, (void *)fapp_tftp_cmd,  "TFTP firmware loader", "[<image name>[<server ip>[<type>]]]"},
#endif
#if FTE_BL_CFG_TFTPUP_CMD 
    { FNET_SHELL_CMD_TYPE_NORMAL, "tftpup", 0, 3, (void *)fapp_tftp_cmd,  "TFTP firmware uploader", "[<image name>[<server ip>[<type>]]]"},
#endif  
#if FTE_BL_CFG_TFTPS_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "tftps", 0, 1, (void *)fapp_tftps_cmd,  "TFTP firmware server", "[release]"},
#endif
#if FTE_BL_CFG_TELNET_CMD && FNET_CFG_TELNET
    { FNET_SHELL_CMD_TYPE_NORMAL, "telnet", 0, 1, (void *)fapp_telnet_cmd,    "Start Telnet Server", "[release]"},
#endif 
#if FTE_BL_CFG_MEM_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "mem", 0, 0, (void *)fapp_mem_cmd,    "Show memory map", ""},
#endif  
#if FTE_BL_CFG_ERASE_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "erase", 1, 2, (void *)fapp_mem_erase_cmd,    "Erase flash memory", "all|[0x<erase address> <bytes>]"},
#endif 
#if FTE_BL_CFG_SAVE_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "save", 0, 0, (void *)fapp_save_cmd,  "Save parameters to the FLASH", ""},
#endif 
#if FTE_BL_CFG_GO_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "go", 0, 1, (void *)fapp_go_cmd,      "Start application at address", "[0x<address>]"},
#endif 
#if FTE_BL_CFG_RESET_CMD
    { FNET_SHELL_CMD_TYPE_NORMAL, "reset", 0, 0, (void *)fapp_reset_cmd,    "Reset the board", ""},
#endif    
#if FAPP_CFG_BENCH_CMD    
    { FNET_SHELL_CMD_TYPE_NORMAL, "benchrx", 0, 1, (void *)fapp_benchrx_cmd,    "Receiver Benchmark", "[tcp|udp]"},
    { FNET_SHELL_CMD_TYPE_NORMAL, "benchtx", 1, 5, (void *)fapp_benchtx_cmd,    "Transmitter Benchmark", "<receiver IP>[tcp|udp[<message size>\r\n\t[<number of messages>[<number of iterations>]]]"},
#endif
#if FAPP_CFG_REINIT_CMD   /* Used to test FNET release/init only. */
    { FNET_SHELL_CMD_TYPE_NORMAL, "reinit", 0, 0, (void *)fapp_reinit_cmd,    "Reinit application.", ""},
#endif
    { FNET_SHELL_CMD_TYPE_NORMAL, "clr_ret", 0, 0, (void *)fapp_clr_ret_cmd,    "Clear command result.", ""},
    { FNET_SHELL_CMD_TYPE_NORMAL, "diag", 0, 1, (void *)fapp_diag_cmd,    "Firmware diagnostics", "[fw]"},
    { FNET_SHELL_CMD_TYPE_END, 0, 0, 0, 0, 0, 0}  
};

/* Shell command-line buffer.*/
static char fapp_cmd_line_buffer[FTE_BL_CFG_SHELL_MAX_LINE_LENGTH];

/************************************************************************
*     The main shell control data structure.
*************************************************************************/
const struct fnet_shell fapp_shell =
{
    fapp_cmd_table,         /* cmd_table */
    FTE_BL_CFG_SHELL_PROMPT,  /* prompt_str */
    FTE_BL_SHELL_init,        /* shell_init */
};

/* Boot-mode string matching. */
#define FAPP_PARAMS_BOOT_MODE_STOP_STR          "stop"   /* Stop at the shell prompt */
#define FAPP_PARAMS_BOOT_MODE_GO_STR            "go"     /* Boot from flash.*/
#define FAPP_PARAMS_BOOT_MODE_RUN_STR           "run"    /* Boot from flash.*/
#define FAPP_PARAMS_BOOT_MODE_SCRIPT_STR        "script" /* Start boot-script.*/



/* Boot modes. */
const struct boot_mode boot_modes[] =
{
    {FAPP_PARAMS_BOOT_MODE_STOP,    FAPP_PARAMS_BOOT_MODE_STOP_STR,     0},
    {FAPP_PARAMS_BOOT_MODE_GO,      FAPP_PARAMS_BOOT_MODE_GO_STR,       FTE_BL_bootModeGo},
    {FAPP_PARAMS_BOOT_MODE_SCRIPT,  FAPP_PARAMS_BOOT_MODE_SCRIPT_STR,   FTE_BL_bootModeScript}
    
};

#define BOOT_MODE_COUNT     (sizeof(boot_modes)/sizeof(struct boot_mode))
#define BOOT_MODE_DEFAULT   boot_modes[3]


fnet_shell_desc_t fapp_shell_desc = 0; /* Shell descriptor. */

/************************************************************************
* NAME: FTE_BL_bootModeByIndex
*
* DESCRIPTION:
************************************************************************/
const struct boot_mode *FTE_BL_bootModeByIndex(unsigned long index)
{
    const struct boot_mode *result = &BOOT_MODE_DEFAULT;
    const struct boot_mode *mode = boot_modes;
    while( mode->name )
    {
        if( mode->index == index )
        {
            result = mode;
            break;
        } 
        mode++;  
    }
    
    return result;    
}


/************************************************************************
* NAME: FTE_BL_bootModeByName
*
* DESCRIPTION:
************************************************************************/
const struct boot_mode *FTE_BL_bootModeByName (char *name)
{
    const struct boot_mode *result = 0;
    const struct boot_mode *mode = boot_modes;
    while( mode->name )
    {
        if(fnet_strcmp( mode->name, name ) == 0)
        {
            result = mode;
            break;
        } 
        mode++;  
    }
    
    return result;   
}



/************************************************************************
* NAME: FTE_BL_bootModeGo
*
* DESCRIPTION: 
************************************************************************/
static void FTE_BL_bootModeGo(fnet_shell_desc_t desc)
{
    (void)desc;
    
    fnet_release(); /* Release FNET stack (it's optional).*/
    
    (( void(*)() )xDynamicParams.boot.go_address)();
}

/************************************************************************
* NAME: FTE_BL_bootModeScript
*
* DESCRIPTION: 
************************************************************************/
static void FTE_BL_bootModeScript(fnet_shell_desc_t desc)
{
    fnet_shell_println(desc, "\n%s",xDynamicParams.boot.script);
    fnet_shell_script(desc, xDynamicParams.boot.script);
}

/************************************************************************
* NAME: FTE_BL_boot
*
* DESCRIPTION: 
************************************************************************/
static inline void FTE_BL_boot(fnet_shell_desc_t desc)
{
/* Bootloader. */
#if FTE_BL_CFG_BOOTLOADER

    /* The bootloader wait some time for a key over a serial connection.*/
    /* The wait time is given by the boot_delay parameter.*/
    int delay;

    const struct boot_mode *mode;

    mode = FTE_BL_bootModeByIndex (xDynamicParams.boot.mode);    
        

    if(mode->handler)
    {
        delay = (int)xDynamicParams.boot.delay;
        
        fnet_shell_printf(desc, FTE_BL_BOOT_STR, mode->name, delay);

        while(--delay >= 0)
        {
            if( fnet_shell_getchar(desc) != FNET_ERR)
                break;
                
            fnet_timer_delay(FNET_TIMER_TICK_IN_SEC); /* 1 sec. delay. */
            fnet_shell_printf(desc, "\b\b\b\b %3d", delay);
        }
       
        fnet_shell_println(desc, "");
       
        if(delay < 0)
        {
            /* Auto-start*/
            mode->handler(desc);
        }
    }
    else
#endif    
    {
        /* Default startup script. */
        #if FTE_BL_CFG_STARTUP_SCRIPT_ENABLED
            fnet_shell_println(desc, "Startup script: %s", FTE_BL_CFG_STARTUP_SCRIPT);
            fnet_shell_script(desc, "dhcp");
        #endif
    }
    
    FNET_COMP_UNUSED_ARG(desc);
}

/************************************************************************
* NAME: fapp_reset_cmd
*
* DESCRIPTION: Reset command reboots the system. 
************************************************************************/
#if FTE_BL_CFG_RESET_CMD
int fapp_reset_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_COMP_UNUSED_ARG(desc);
    FNET_COMP_UNUSED_ARG(argc);
    FNET_COMP_UNUSED_ARG(argv);

    fnet_release(); /* Release FNET stack (it's optional).*/
    
    fnet_cpu_reset();
    
    return  0;
}
#endif

/************************************************************************
* NAME: fapp_dup_ip_handler
*
* DESCRIPTION: IP address is duplicated.
************************************************************************/
static void fapp_dup_ip_handler( fnet_netif_desc_t netif )
{
    char name[FNET_NETIF_NAMELEN];
    char ip_str[16];
    fnet_ip_addr_t addr;
    
    fnet_netif_get_name( netif, name, FNET_NETIF_NAMELEN );
    addr = fnet_netif_get_address( netif );
    inet_ntoa(*(struct in_addr *)( &addr), ip_str);

    fnet_printf(FTE_BL_DUP_IP_WARN, name, ip_str);
}


/************************************************************************
* NAME: FTE_BL_init
*
* DESCRIPTION: FNET Application initialization.
************************************************************************/
static int FTE_BL_init()
{
    static unsigned char stack_heap[FNET_CFG_HEAP_SIZE];
    struct fnet_init_params init_params;
    
    struct fnet_shell_params shell_params;

    FTE_BL_LED_init();
    
    /* Input parameters for FNET stack initialization */
    init_params.netheap_ptr = stack_heap;
    init_params.netheap_size = FNET_CFG_HEAP_SIZE;

    
    /* Add event handler on duplicated IP address */
    fnet_netif_dupip_handler_init (fapp_dup_ip_handler);
            
    /* Init FNET stack */
    if(fnet_init(&init_params) != FNET_ERR)
    {
        if((fapp_default_netif = fnet_netif_get_default()) != 0)
        {
            #if FTE_BL_CFG_PARAMS_READ_FLASH
                /* During bootup, the most recently stored customer configuration data will be read and used to configure the interfaces.*/
                if(fapp_static_params_from_flash() == FNET_OK)
                {
                    fnet_printf(FTE_BL_STATIC_PARAMS_LOAD_STR);
                }
                else
                {
                    fapp_static_params_init();
                }
                
                if(fapp_dynamic_params_from_flash() == FNET_OK)
                {
                    fnet_printf(FTE_BL_DYNAMIC_PARAMS_LOAD_STR);
                }
                else
                {
                    fapp_dynamic_params_init();
                }
            #endif
            
            #if (FAPP_CFG_EXP_CMD && FNET_CFG_FS) || (FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP)   
                fapp_fs_mount(); /* Init FS and mount FS Image. */
            #endif   
            
            /* Init main shell. */
            shell_params.shell = &fapp_shell;
            shell_params.cmd_line_buffer = fapp_cmd_line_buffer;
            shell_params.cmd_line_buffer_size = sizeof(fapp_cmd_line_buffer);
            shell_params.stream = FNET_SERIAL_STREAM_DEFAULT;
            shell_params.echo = 1;
            
            if((fapp_shell_desc = fnet_shell_init(&shell_params)) != FNET_ERR)
            {
                  
                FTE_BL_boot(fapp_shell_desc);
 
            }
            else
            {
                fnet_printf(FTE_BL_INIT_ERR, "Shell");
                return  -1;
            }
        }
        else
        {
            fnet_printf(FTE_BL_NET_ERR);
            return  -1;
        }
    }
    else
    {
        fnet_printf(FTE_BL_INIT_ERR, "FNET");
        return  -1;
    }


    return  0;
}

/************************************************************************
* NAME: FTE_BL_init
*
* DESCRIPTION: FNET Application release.
************************************************************************/
static void FTE_BL_release(fnet_shell_desc_t desc)
{
    #if FTE_BL_CFG_DHCP_CMD && FNET_CFG_DHCP
    fapp_dhcp_release();        /* Release DHCP client. */
    #endif
    
    #if FTE_BL_CFG_TELNET_CMD && FNET_CFG_TELNET    /* Release TELNET server. */   
    fapp_telnet_release();
    #endif
 
    #if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP      /* Release HTTP server. */ 
    fapp_http_release();
    #endif    
 
    #if FTE_BL_CFG_TFTPS_CMD /* Release TFTP server. */
    fapp_tftps_release();
    #endif   

    #if (FAPP_CFG_EXP_CMD && FNET_CFG_FS) || (FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP)   
    fapp_fs_unmount();          /* Unmount and release FS. */
    #endif 
    
    fnet_shell_release(desc);   /* Release shell. */
    
    fnet_release();             /* Release the FNET stack.*/
}

/************************************************************************
* NAME: FTE_BL_main
*
* DESCRIPTION: main() of the shell demo.
************************************************************************/
void FTE_BL_main()
{
    FTE_BL_init();
    
    /* Polling services.*/
    while(1)
    {
       fnet_poll();
    }
}

void FTE_BL_LED_init(void)
{
    FNET_MK_PORT_MemMapPtr  pctl;
    unsigned long           *pDirReg;
    unsigned long           *pOutputReg;

#if FTE_BL_MODEL_NB_VERSION(FTE_BL_CFG_HARDWARE_VERSION) != FTE_BL_MODEL_NETWORK_BOARD_V1
    pDirReg = (unsigned long   *)0x400FF054;
    pOutputReg = (unsigned long   *)0x400FF040;
    pctl = (FNET_MK_PORT_MemMapPtr)FNET_MK_PORTB_BASE_PTR;    
    pctl->PCR[5]  = 0x00000100;     /* PTB5, GPIO */
    *pDirReg = *pDirReg | (0x1 << 5);
    *pOutputReg = *pOutputReg & ~ (0x1 << 5);
#endif
    
    pDirReg = (unsigned long   *)0x400FF094;
    pOutputReg = (unsigned long   *)0x400FF080;
    pctl = (FNET_MK_PORT_MemMapPtr)FNET_MK_PORTC_BASE_PTR;    
    pctl->PCR[12]  = 0x00000100;     /* PTC12, GPIO */
    *pDirReg = *pDirReg | (0x1 << 12);
    *pOutputReg = *pOutputReg & ~(0x1 << 12);
}

/************************************************************************
* NAME: fapp_info_print
*
* DESCRIPTION: Desplay detailed information about the stack.
************************************************************************/
void fapp_info_print( fnet_shell_desc_t desc )
{
    fnet_ip_addr_t local_ip;
    fnet_ip_addr_t local_netmask;
    fnet_ip_addr_t local_gateway;
    fnet_mac_addr_t macaddr;
    char name[FNET_NETIF_NAMELEN];
    char ip_str[16 + sizeof(" (obtained automatically)")];
    char mac_str[18];
    int address_automatic;
    struct fnet_netif_statistics statistics;

    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "Device ID", xStaticParams.device.id);
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_VER, "B/L Version", 
                       (xStaticParams.device.version.loader >> 24) & 0xFF,
                       (xStaticParams.device.version.loader >> 16) & 0xFF,
                       (xStaticParams.device.version.loader >>  8) & 0xFF,
                       (xStaticParams.device.version.loader >>  0) & 0xFF);
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_VER, "H/W Version", 
                       (xStaticParams.device.version.hardware >> 24) & 0xFF,
                       (xStaticParams.device.version.hardware >> 16) & 0xFF,
                       (xStaticParams.device.version.hardware >>  8) & 0xFF,
                       (xStaticParams.device.version.hardware >>  0) & 0xFF);
    
    local_ip = fnet_netif_get_address(fapp_default_netif);
    local_netmask = fnet_netif_get_netmask(fapp_default_netif);
    local_gateway = fnet_netif_get_gateway(fapp_default_netif);
    fnet_netif_get_hw_addr(fapp_default_netif, macaddr, sizeof(fnet_mac_addr_t));

    fnet_netif_get_name(fapp_default_netif, name, FNET_NETIF_NAMELEN);
    
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "Interface", name);
    
    
    address_automatic = fnet_netif_address_automatic(fapp_default_netif);

    inet_ntoa(*(struct in_addr *)( &local_ip), ip_str);
   
    fnet_sprintf(ip_str,"%s (%s)", ip_str, address_automatic ? "obtained automatically" : "set manually"); 
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "IP address", ip_str);
      
    inet_ntoa(*(struct in_addr *)( &local_netmask), ip_str);
    
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "Subnet mask", ip_str);
    
    inet_ntoa(*(struct in_addr *)( &local_gateway), ip_str);
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "Gateway", ip_str);

    fnet_mac_to_str(macaddr, mac_str);
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "MAC address", mac_str);
    
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_S, "Link status", fnet_netif_connected(fapp_default_netif) ? "connected" : "unconnected");

    if(fnet_netif_get_statistics(fapp_default_netif, &statistics) == FNET_OK)
    {
        fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_D, "TX Packets", statistics.tx_packet);
        fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_D, "RX Packets", statistics.rx_packet);
    }
    
    fnet_shell_println(desc, FTE_BL_SHELL_INFO_FORMAT_D, "Free Heap", fnet_free_mem_status());

#if FAPP_CFG_HTTP_CMD && FNET_CFG_HTTP

    fapp_http_info(desc);

#endif

#if FTE_BL_CFG_DHCP_CMD && FNET_CFG_DHCP
    
    fapp_dhcp_info(desc);
    
#endif

#if FTE_BL_CFG_TELNET_CMD && FNET_CFG_TELNET

    fapp_telnet_info(desc);
    
#endif   

#if FTE_BL_CFG_TFTPS_CMD

    fapp_tftps_info(desc);
    
#endif 


}

/************************************************************************
* NAME: fapp_info_cmd
*
* DESCRIPTION:
************************************************************************/
#if FTE_BL_CFG_INFO_CMD
int fapp_info_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_COMP_UNUSED_ARG(argc);
    FNET_COMP_UNUSED_ARG(argv);

    fapp_info_print(desc);
    
    return  0;
}
#endif

/************************************************************************
* NAME: FTE_BL_SHELL_init
*
* DESCRIPTION: Main Shell initial function.
************************************************************************/
void FTE_BL_SHELL_init( fnet_shell_desc_t desc )
{
    fnet_shell_println(desc, "\n%s", FTE_BL_DELIMITER_STR);
    fnet_shell_println(desc, " %s", FTE_BL_CFG_NAME);

    /* FNET Header */
    fnet_shell_println(desc, FTE_BL_DELIMITER_STR);
    
    fnet_shell_println(desc, " %s for %s", FNET_DESCRIPTION, FNET_CPU_STR);
    fnet_shell_println(desc, " Version %s", FNET_VERSION);
    fnet_shell_println(desc, " Built %s by %s", FNET_BUILD_DATE, FNET_COMP_STR);
    fnet_shell_println(desc, " %s", FNET_COPYRIGHT);
    fnet_shell_println(desc, " %s", FNET_LICENSE);
    fnet_shell_println(desc, FTE_BL_DELIMITER_STR);
    fapp_info_print(desc);
    fnet_shell_println(desc, "\n Enter 'help' for command list.");
    fnet_shell_println(desc, "%s\n", FTE_BL_DELIMITER_STR);
}

/************************************************************************
* NAME: fapp_go_cmd
*
* DESCRIPTION:  
************************************************************************/
#if FTE_BL_CFG_GO_CMD
int fapp_go_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
	unsigned long address;
	char *p;

	if (argc == 2)
	{
		address = fnet_strtoul(argv[1],&p,16);
		if ((address == 0) && (p == argv[1]))
        {
            fnet_shell_println(desc, FTE_BL_PARAM_ERR, argv[1] );     
            return -1;
        }
    }
    else
        address = xDynamicParams.boot.go_address;
  
    fnet_shell_println(desc, FTE_BL_GO_STR, address);
 
    fnet_release(); /* Release the FNET stack.*/
   
    return  (( int(*)() )address)(); /* Jump. */    
}
#endif

/************************************************************************
* NAME: fapp_save_cmd
*
* DESCRIPTION: Save environment variables to persistent storage. 
************************************************************************/
#if FTE_BL_CFG_SAVE_CMD
int fapp_save_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    (void)argc;
	(void)argv;

    if((fapp_static_params_to_flash() == FNET_OK) && (fapp_dynamic_params_to_flash() == FNET_OK))
    {
        fnet_shell_println(desc, FTE_BL_SAVE_STR);
        return  0;
    }
    else
    {
        fnet_shell_script_release(desc);   /* Critical error. Clear script. */
        fnet_shell_println(desc, FTE_BL_SAVE_FAILED_STR);
        
        return  -1;
    }
}
#endif

/************************************************************************
* NAME: fapp_help_cmd
*
* DESCRIPTION: Display command help. 
************************************************************************/
int fapp_help_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    (void)argc;
	(void)argv;

    fnet_shell_help(desc);
    
    return  0;
}

/************************************************************************
* NAME: fapp_reinit_cmd
*
* DESCRIPTION: Used to test FNET release/init only. 
************************************************************************/
#if FAPP_CFG_REINIT_CMD 
int fapp_reinit_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    (void)argc;
	(void)argv;

    FTE_BL_release(desc); /* FAPP release. */

    FTE_BL_init();        /* FAPP reinit.*/
    
    return  0;
}
#endif

int fapp_clr_ret_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    (void)argc;
	(void)argv;
    
    fnet_shell_clr_ret(desc);
    
    return  0;
}

