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
#include "shell.h"
#include "sh_prv.h"
#include "io.h"
#include "fte_db.h"

uint_32     FTE_SHELL_getPasswd(MQX_FILE_PTR pFile, char_ptr pPasswd, uint_32 ulMaxLen, uint_32 ulTimeout);
int_32      FTE_SHELL_cmdPasswd(int_32 nArgc, char_ptr pArgv[]);
int_32      FTE_SHELL_cmdGet(int_32 nArgc, char_ptr pArgv[]);
int_32      FTE_SHELL_cmdSet(int_32 nArgc, char_ptr pArgv[]);
int_32      FTE_SHELL_main(const SHELL_COMMAND_STRUCT   pShellCmds[], FTE_SHELL_CONFIG_PTR pConfig);
_mqx_int    FTE_SHELL_fgetc(MQX_FILE_PTR pFile, _mqx_int_ptr pChar, uint_32 ulTimeout);
_mqx_int    FTE_SHELL_fgets(MQX_FILE_PTR pFile, char_ptr pTTYLine, _mqx_int nSize, uint_32 ulTimeout);

const SHELL_COMMAND_STRUCT pSHELLCommands[] = 
{
   /* RTCS commands */ 

#if FTE_AD7785_SUPPORTED
    { "ad7785",      FTE_AD7785_SHELL_cmd},
#endif
    { "bl",         FTE_SYS_BL_cmd},    
    { "cert",       FTE_CFG_CERT_SHELL_cmd},    
    { "config",     FTE_CFG_SHELL_cmd},
#if FTE_1WIRE_SUPPORTED
    { "1wire",      FTE_1WIRE_SHELL_cmd},
#endif
    { "date",       FTE_TIME_SHELL_cmd},
    { "db",         FTE_DB_SHELL_cmd},
#if FTE_DO_SUPPORTED
    { "do",         FTE_DO_SHELL_cmd},
#endif
#if FTE_DS18B20_SUPPORTED
    { "ds",         FTE_DS18B20_SHELL_cmd},
#endif
    { "event",      FTE_EVENT_shell_cmd},
    { "exit",       Shell_exit},
    { "get",        FTE_SHELL_cmdGet},
#if FTE_TASCON_HEM12_06M_SUPPORTED
    { "hem12",      FTE_TASCON_HEM12_SHELL_cmd},
#endif
#if FTE_I2C_SUPPORTED
    { "i2c",        FTE_I2C_SHELL_cmd },
#endif
    { "ipconfig",   Shell_ipconfig },    
    { "ifconfig",   FTE_NET_SHELL_cmd },
#if FTE_LOG_SUPPORTED
    { "log",        FTE_LOG_SHELL_cmd},
#endif
#if FTE_MCP23S08_SUPPORTED
    {"mcp23s08",    FTE_MCP23S08_SHELL_cmd},
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
    { "rl",          FTE_RL_SHELL_cmd},
#endif
    { "set",        FTE_SHELL_cmdSet},
#if FTE_SNMPD_SUPPORTED
    { "snmp",       FTE_SNMPD_SHELL_cmd},
#endif
#if FTE_SSD1305_SUPPORTED
    { "lcd",        FTE_SSD1305_SHELL_cmd},
#endif
    { "save",       FTE_CFG_SHELL_cmdSave},
#if FTE_SHT_SUPPORTED
    { "sht",        FTE_SHT_SHELL_cmd},
#endif
#if FTE_SPI_SUPPORTED
    { "spi",        FTE_SPI_SHELL_cmd},
#endif
    { "shutdown",   FTE_SYS_SHUTDOWN_SHELL_cmd},
    { "sys",        FTE_SYS_SHELL_cmd},
    { "task",       FTE_TASK_SHELL_cmd},
#if FTE_DEBUG
    { "trace",      FTE_TRACE_SHELL_cmd},
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
    _mqx_int    nResult;
    _mqx_int    nCh;
    FTE_SHELL_CONFIG_PTR    pShellConfig = FTE_CFG_SHELL_get();
    char    pUserID[FTE_SHELL_USER_ID_LENGTH+1];
    char    pPasswd[FTE_SHELL_PASSWD_LENGTH+1];
    /* Run the shell */
 
    if (FTE_SHELL_fgetc(stdin, &nCh, pShellConfig->ulTimeout) != MQX_OK)
    {
        return  MQX_OK;
    }

    fprintf(stdout, "\n\nWalcome to %s!\n", FTE_MODEL);
    fflush(stdout);
    while(1)
    {
        memset(pUserID, 0, sizeof(pUserID));
        memset(pPasswd, 0, sizeof(pPasswd));
        
        fprintf(stdout, "ID : ");    
        fflush(stdout);
        nResult = FTE_SHELL_fgets(stdin, pUserID, FTE_SHELL_USER_ID_LENGTH, pShellConfig->ulTimeout);
        if (nResult != MQX_OK)
        {
            break;
        }
        fprintf(stdout, "passwd : ");    
        fflush(stdout);
        nResult = FTE_SHELL_getPasswd(stdin, pPasswd, FTE_SHELL_PASSWD_LENGTH, pShellConfig->ulTimeout);
        if (nResult != MQX_OK)
        {
            break;
        }
        
        if((strcmp(pShellConfig->pUserID, pUserID) != 0) ||
           (strcmp(pShellConfig->pPasswd, pPasswd) != 0))
        {
            fprintf(stdout, "Invalid id or passwd!\n");
            fflush(stdout);
        }
        else
        {
            FTE_SHELL_main(pSHELLCommands, pShellConfig);
            break;
        }
    }
    
    fprintf(stdout, "Bye!\n");
    fflush(stdout);
    
    return  MQX_OK;
}

uint_32  FTE_SHELL_getPasswd(MQX_FILE_PTR pFile, char_ptr pPasswd, uint_32 ulMaxLen, uint_32 ulTimeout)
{
    boolean     bExit = FALSE;
    _mqx_int    ch;
    uint_32     ulLen = 0;
    _mqx_uint   ulFlags;
    
   ioctl(pFile, IO_IOCTL_SERIAL_GET_FLAGS, &ulFlags);
    ulFlags  &= ~IO_SERIAL_ECHO;
   ioctl(pFile, IO_IOCTL_SERIAL_SET_FLAGS, &ulFlags);
   
    while(!bExit)
    {
        FTE_SHELL_fgetc(pFile, &ch, ulTimeout);
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
        case    IO_EOF:
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
    ioctl(pFile, IO_IOCTL_SERIAL_SET_FLAGS, &ulFlags);

    return  MQX_OK;
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
                FTE_SHELL_getPasswd(stdin, pOldPasswd, FTE_SHELL_PASSWD_LENGTH, pShellConfig->ulTimeout);
                if (strcmp(pShellConfig->pPasswd, pOldPasswd) != 0)
                {
                    goto error;
                }
                
                fprintf(stdout, "\nEnter new password : ");
                FTE_SHELL_getPasswd(stdin, pNewPasswd, FTE_SHELL_PASSWD_LENGTH, pShellConfig->ulTimeout);
                fprintf(stdout, "\nRetype new password : ");
                FTE_SHELL_getPasswd(stdin, pNewPasswdConfirm, FTE_SHELL_PASSWD_LENGTH, pShellConfig->ulTimeout);
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
            fprintf (stdout, "%s\n", pArgv[0]);
        }
        else
        {
            fprintf (stdout, "Usage: %s\n", pArgv[0]);
        }
    }
    return   nReturnCode;

error:
    fprintf(stdout, "\npasswd : Authentication token manipulation error.\n");
    fprintf(stdout, "passwd : password unchanged.\n");

    return  0;
}

int_32  FTE_SHELL_cmdGet(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (bPrintUsage)
    {
        goto print_usage;
    }
    
    switch(nArgc)
    {
    case    1:
        {    
            FTE_SHELL_CONFIG_PTR    pConfig = FTE_CFG_SHELL_get();
            
            printf("%s=%d\n", "timeout", pConfig->ulTimeout);
        }
        break;
                    
    default:
        bPrintUsage = TRUE;
    }
           
print_usage:    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            fprintf (stdout, "%s\n", pArgv[0]);
        }
        else
        {
            fprintf (stdout, "Usage: %s\n", pArgv[0]);
        }
    }
    return   nReturnCode;
}

int_32  FTE_SHELL_cmdSet(int_32 nArgc, char_ptr pArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (bPrintUsage)
    {
        goto print_usage;
    }
    
    switch(nArgc)
    {
    case    3:
        {    
            if (strcmp(pArgv[1], "timeout") == 0)
            {
                uint_32                 ulTimeout;
                FTE_SHELL_CONFIG_PTR    pConfig = FTE_CFG_SHELL_get();
                    
                if (! Shell_parse_number( pArgv[2], &ulTimeout))  
                {
                    nReturnCode = SHELL_EXIT_ERROR;
                    goto print_usage;
                }

                pConfig->ulTimeout = ulTimeout;
                FTE_CFG_save(TRUE);
            }
        }
        break;
                    
    default:
        bPrintUsage = TRUE;
    }
           
print_usage:    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            fprintf (stdout, "%s <variable> <value>\n", pArgv[0]);
        }
        else
        {
            fprintf (stdout, "Usage: %s <variable> <value>\n", pArgv[0]);
            fprintf (stdout, "  Parameters:\n");
            fprintf (stdout, "    %-10s = Shell environment variable.\n", "<variable>" );
            fprintf (stdout, "    %-10s = value.\n", "<value>" );
        }
    }
    return   nReturnCode;
}

int_32 FTE_SHELL_main
(
    const SHELL_COMMAND_STRUCT  pShellCmds[],
    FTE_SHELL_CONFIG_PTR        pConfig
)
{ /* Body */
    SHELL_CONTEXT_PTR    pShell;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    uint_32              i;

    fprintf(stdout, "\nShell (build: %s)\n", __DATE__);
    fprintf(stdout, "Copyright (c) 2013 FuruteTek,Inc.;\n");
   
   
    pShell = _mem_alloc_zero( sizeof( SHELL_CONTEXT ));
    if (pShell == NULL)  
    {
        return SHELL_EXIT_ERROR;
    }
    _mem_set_type(pShell, MEM_TYPE_SHELL_CONTEXT);
   
    pShell->COMMAND_LIST_PTR=(SHELL_COMMAND_PTR)pShellCmds; 

    if (!pShell->COMMAND_FP)  
    {
        pShell->COMMAND_FP = stdin;
    }

#if SHELLCFG_USES_MFS
   // initialize current fs ptr, fs name and path
    Shell_set_current_filesystem((pointer)pShell,NULL);
#endif //SHELLCFG_USES_MFS
   
    if (pShell->CMD_LINE)
    {
        fprintf(stdout, "shell> %s\n",pShell->CMD_LINE);
    } 
    else 
    {
        fprintf(stdout, "shell> ");
    }
    fflush(stdout);
   
    while (!pShell->EXIT) 
    {      
        if ((!pShell->EXIT) && (pShell->CMD_LINE[0] != '\0'))  
        {
            if (pShell->COMMAND_FP != stdin)  
            {
                fprintf(stdout, "%s\n", pShell->CMD_LINE);
            }
      
            if (pShell->CMD_LINE[0] != '#') 
            {
                if (strcmp(pShell->CMD_LINE, "!") == 0)  
                {
                    strncpy(pShell->CMD_LINE,pShell->HISTORY,sizeof(pShell->CMD_LINE));
                } 
                else if (strcmp(pShell->CMD_LINE, "\340H") == 0)  
                {
                    strncpy(pShell->CMD_LINE,pShell->HISTORY,sizeof(pShell->CMD_LINE));
                } 
                else  
                {
                    strncpy(pShell->HISTORY,pShell->CMD_LINE,sizeof(pShell->HISTORY));
                }
    
                pShell->ARGC = Shell_parse_command_line(pShell->CMD_LINE, pShell->ARGV );
                
                if (pShell->ARGC > 0) 
                {
                    _io_strtolower(pShell->ARGV[0]); 
                    for (i=0;pShellCmds[i].COMMAND != NULL;i++)  
                    {
                        if (strcmp(pShell->ARGV[0], pShellCmds[i].COMMAND) == 0)  
                        {
                            /* return_code = */ (*pShellCmds[i].SHELL_FUNC)(pShell->ARGC, pShell->ARGV);
                            break;   
                        }
                    }
                    if (pShellCmds[i].COMMAND == NULL)  
                    {
                        fprintf(stdout, "Invalid command.  Type 'help' for a list of commands.\n");
                    } 
                }
            }
        }
  
        if (!pShell->EXIT) 
        { 
            fprintf(stdout, "shell> ");
            fflush(stdout);
            
            do 
            {
                _mqx_int    nResult;
                nResult = FTE_SHELL_fgets(pShell->COMMAND_FP, pShell->CMD_LINE, sizeof(pShell->CMD_LINE), pConfig->ulTimeout);
               
               if (nResult != MQX_OK) 
                {
                    if (pShell->COMMAND_FP != stdin)  
                    {
                        fclose(pShell->COMMAND_FP);
                        pShell->COMMAND_FP = stdin;
                        pShell->HISTORY[0]=0;
                        pShell->CMD_LINE[0]=0;
                        fprintf(stdout, "\n");
                    } 
                    else  
                    {
                        pShell->EXIT=TRUE;
                        break;
                    }
                }
            } 
            while ((pShell->CMD_LINE[0] == '\0') && (pShell->COMMAND_FP != stdin)) ; 
        }
    } 
 
    fprintf(stdout, "Terminate.\n");
    fflush(stdout);
    
    _mem_free(pShell);
   
    return return_code;
} /* Endbody */


_mqx_int    FTE_SHELL_fgetc(MQX_FILE_PTR pFile, _mqx_int_ptr pChar, uint_32 ulTimeout)
{
    if (_io_fstatus(pFile) != TRUE)
    {        
        MQX_TICK_STRUCT xStartTick;
        _time_get_ticks(&xStartTick);
        
        while(_io_fstatus(pFile) != TRUE)
        {
            MQX_TICK_STRUCT xCurrentTick;
            boolean     bOverflow = FALSE;
            
            _time_get_ticks(&xCurrentTick);
            
            if ((ulTimeout != 0) && (_time_diff_seconds(&xCurrentTick, &xStartTick, &bOverflow) > ulTimeout))
            {
                return  MQX_ETIMEDOUT;
            }
            
            _time_delay(1);
        }
    }

    *pChar = _io_fgetc(pFile);
    
    if (*pChar == IO_EOF)
    {
        return  FS_EOF;
    }
    
    return  MQX_OK;
}

/*!
 * \brief Returns the number of characters read into the input line.
 * 
 * The terminating line feed is stripped.
 * 
 * \param[in]     pFile   The stream to read the characters from.
 * \param[in,out] pBuff    Where to store the input characters.
 * \param[in]     nMaxLen The maximum number of characters to store.
 * 
 * \return Number of characters read.
 * \return IO_EOF    
 */ 
_mqx_int FTE_SHELL_fgets
(
    MQX_FILE_PTR pFile,
    char _PTR_   pBuff,
    _mqx_int     nMaxLen,
    uint_32     ulTimeout
) 
{ /* Body */
    _mqx_int    nResult;
    _mqx_int    c;
    _mqx_int    i;
//    _mqx_uint flags;

#if MQX_CHECK_ERRORS
    if (pFile == NULL) 
    {
        *pBuff = '\0';
        return FS_INVALID_HANDLE;
    } /* Endif */
#endif

    if (nMaxLen) 
    {
        nMaxLen--;  /* Need to leave 1 space for the null termination */
    } 
    else 
    {
        nMaxLen = MAX_MQX_INT;  /* Effectively infinite length */
    } /* Endif */

    nResult = FTE_SHELL_fgetc(pFile, &c, ulTimeout);
    if (nResult != MQX_OK)
    {
        *pBuff = '\0';
        return  nResult;
    } /* Endif */
//    flags = pFile->FLAGS;
    i = 0;
    while ( (! ((c == '\n') || (c == '\r'))) && (i < nMaxLen) ) 
    {
//        if ((flags & IO_FLAG_TEXT) && (c == '\b')) 
        if (c == '\b') 
        {
            if ( i ) 
            {
                *--pBuff = ' ';
                --i;
            } /* Endif */
        } 
        else 
        {
            *pBuff++ = (char)c;
            ++i;
        } /* Endif */
        
        nResult = FTE_SHELL_fgetc(pFile, &c, ulTimeout);
        if (nResult != MQX_OK) 
        {
            *pBuff = '\0'; /* null terminate the string before returning */
            return nResult;
        } /* Endif */
    } /* Endwhile */

    if (i >= nMaxLen) 
    {
        _io_fungetc((_mqx_int)c, pFile);
    } /* Endif */

    *pBuff = '\0';

    return MQX_OK;
} /* Endbody */

