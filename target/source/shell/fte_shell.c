#include "tfs.h"
#include "fte_target.h"
#include "fte_config.h"
#include "sys/fte_sys.h"
#include "fte_task.h"
#include "fte_time.h"
#include "fte_log.h"
#include "fte_net.h"
#include "fte_shell.h"
#include <sh_rtcs.h>
#include "fte_sys_bl.h"
#include "fte_object.h"

uint_32  FTE_SHELL_getPasswd(char_ptr pPasswd, uint_32 ulMaxLen);
int_32   FTE_SHELL_cmdPasswd(int_32 nArgc, char_ptr pArgv[]);

const SHELL_COMMAND_STRUCT pSHELLCommands[] = 
{
   /* RTCS commands */ 

#if FTE_AD7785_SUPPORTED
    { "ad7785",      FTE_AD7785_SHELL_cmd},
#endif
    {"bl",  FTE_SYS_BL_cmd},
    { "config",     FTE_CFG_SHELL_cmd},
#if FTE_1WIRE_SUPPORTED
    { "1wire",      FTE_1WIRE_SHELL_cmd},
#endif
    { "date",       FTE_TIME_SHELL_cmd},
#if FTE_DO_SUPPORTED
    {"do",          FTE_DO_SHELL_cmd},
#endif
#if FTE_DS18B20_SUPPORTED
    { "ds",         fte_ds18b20_shell_cmd},
#endif
    { "event",      FTE_EVENT_shell_cmd},
    { "exit",       Shell_exit},
#if FTE_I2C_SUPPORTED
    { "i2c",        fte_i2c_shell_cmd },
#endif
#if FTE_TASCON_HEM12_06M_SUPPORTED
    {"hem12",       FTE_TASCON_HEM12_SHELL_cmd},
#endif
    { "ipconfig",   Shell_ipconfig },    
    { "ifconfig",   FTE_NET_SHELL_cmd },
#if FTE_LOG_SUPPORTED
    { "log",        FTE_LOG_SHELL_cmd},
#endif
#if FTE_MCP23S08_SUPPORTED
    {"mcp23s08",    fte_mcp23s08_shell_cmd},
#endif
    { "mem",        FTE_MEM_SHELL_cmd},
#if FTE_MQTT_SUPPORTED
    { "mqtt",       FTE_MQTT_SHELL_cmd},
#endif
    { "obj",        FTE_OBJ_SHELL_cmd},
    { "passwd",     FTE_SHELL_cmdPasswd},
#if RTCSCFG_ENABLE_ICMP      
    { "ping",        Shell_ping },
#endif
    { "reset",      FTE_SYS_RESET_cmd},
#if FTE_RL_SUPPORTED
    { "rl",          fte_rl_shell_cmd},
#endif
#if FTE_SNMPD_SUPPORTED
    { "snmp",       FTE_SNMPD_SHELL_cmd},
#endif
#if FTE_SSD1305_SUPPORTED
    { "lcd",        fte_ssd1305_shell_cmd},
#endif
    { "save",       FTE_CFG_SHELL_cmdSave},
#if FTE_SSL_SUPPORTED
    { "ssl",        FTE_SSL_SHELL_cmd},
#endif
#if FTE_SPI_SUPPORTED
    { "spi",        fte_spi_shell_cmd},
#endif
    { "shutdown",   FTE_SYS_SHUTDOWN_SHELL_cmd},
    { "sys",        FTE_SYS_SHELL_cmd},
    { "task",       FTE_TASK_SHELL_cmd},
#if FTE_DEBUG
    { "trace",      FTE_DEBUG_shellCmd},
#endif
#if FTE_UCS_SUPPORTED
    { "ucs",        FTE_UCS_SHELL_cmd},
#endif
    { "phy",        FTE_PHY_SHELL_cmd},

    { "help",       Shell_help }, 
    { "?",          Shell_command_list },     
    { NULL,         NULL } 
};

_mqx_uint   FTE_SHELL_proc(void)
{
    FTE_SHELL_CONFIG_PTR    pShellConfig = FTE_CFG_SHELL_get();
    char    pUserID[FTE_SHELL_USER_ID_LENGTH+1];
    char    pPasswd[FTE_SHELL_PASSWD_LENGTH+1];
    /* Run the shell */
 
    fgetc(stdin);
    
    fprintf(stdout, "\n\nWalcome to %s!\n", FTE_MODEL);
    while(1)
    {
        fprintf(stdout, "ID : ");    
        fgets(pUserID, FTE_SHELL_USER_ID_LENGTH, stdin);
        fprintf(stdout, "passwd : ");    
        FTE_SHELL_getPasswd(pPasswd, FTE_SHELL_PASSWD_LENGTH);
        
        if((strcmp(pShellConfig->pUserID, pUserID) != 0) ||
           (strcmp(pShellConfig->pPasswd, pPasswd) != 0))
        {
            fprintf(stdout, "Invalid id or passwd!\n");
        }
        else
        {
            Shell(pSHELLCommands, NULL);
            break;
        }
    }
    printf("Shell exited, restarting...\n");
    
    return  MQX_OK;
}

uint_32  FTE_SHELL_getPasswd(char_ptr pPasswd, uint_32 ulMaxLen)
{
    boolean     bExit = FALSE;
    char        ch;
    uint_32     ulLen = 0;
    _mqx_uint   ulFlags;
    
   ioctl(stdin, IO_IOCTL_SERIAL_GET_FLAGS, &ulFlags);
    ulFlags  &= ~IO_SERIAL_ECHO;
   ioctl(stdin, IO_IOCTL_SERIAL_SET_FLAGS, &ulFlags);
   
    while(!bExit)
    {
        ch = fgetc(stdin);
        switch(ch)
        {
        case    0x08:
            {
                if (ulLen != 0)
                {
                    ulLen--;
                }
            }
            break;
        case    '\n':
        case    '\r':
            {
                pPasswd[ulLen] = '\0';
                bExit = TRUE;
                break;
            }
        default:
            {
                if (ulLen < ulMaxLen)
                {
                    pPasswd[ulLen++] = ch;
                }
            }
        }
        
    }
    
    ulFlags  |= IO_SERIAL_ECHO;
    ioctl(stdin, IO_IOCTL_SERIAL_SET_FLAGS, &ulFlags);

    return  ulLen;
}

int_32  FTE_SHELL_cmdPasswd(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {    
                FTE_SHELL_CONFIG_PTR    pShellConfig = FTE_CFG_SHELL_get();
                char    pOldPasswd[FTE_SHELL_PASSWD_LENGTH+1];
                char    pNewPasswd[FTE_SHELL_PASSWD_LENGTH+1];
                char    pNewPasswdConfirm[FTE_SHELL_PASSWD_LENGTH+1];

                fprintf(stdout, "Changing password for %s\n", pShellConfig->pUserID);
                fprintf(stdout, "(current) password : ");
                FTE_SHELL_getPasswd(pOldPasswd, FTE_SHELL_PASSWD_LENGTH);
                if (strcmp(pShellConfig->pPasswd, pOldPasswd) != 0)
                {
                    goto error;
                }
                
                fprintf(stdout, "\nEnter new password : ");
                FTE_SHELL_getPasswd(pNewPasswd, FTE_SHELL_PASSWD_LENGTH);
                fprintf(stdout, "\nRetype new password : ");
                FTE_SHELL_getPasswd(pNewPasswdConfirm, FTE_SHELL_PASSWD_LENGTH);
                if (strcmp(pNewPasswd, pNewPasswdConfirm) != 0)
                {
                    fprintf(stdout, "\nSorry, passwords do not match.");
                    goto error;
                }
                          
                fprintf(stdout, "\n");
                memset(pShellConfig->pPasswd, 0, sizeof(pShellConfig->pPasswd));
                strcpy(pShellConfig->pPasswd, pNewPasswd);
                
                FTE_CFG_save(TRUE);
            }
            break;
                        
        default:
            bPrintUsage = TRUE;
        }
    }
           
    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s\n", pArgv[0]);
        }
        else
        {
            printf ("Usage: %s\n", pArgv[0]);
        }
    }
    return   nReturnCode;

error:
    fprintf(stdout, "\npasswd : Authentication token manipulation error.\n");
    fprintf(stdout, "passwd : password unchanged.\n");

    return  0;
}