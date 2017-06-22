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
#include "fte_cias.h"
#include "fte_lora.h"
#include "fte_dotech.h"
#include "fte_turbomax.h"
#include "fte_lorawan.h"
#include "fte_mbtcp.h"

FTE_UINT32  FTE_SHELL_getPasswd(MQX_FILE_PTR pFile, FTE_CHAR_PTR pPasswd, FTE_UINT32 ulMaxLen, FTE_UINT32 ulTimeout);
FTE_INT32  FTE_SHELL_cmdPasswd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
FTE_INT32  FTE_SHELL_cmdGet(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
FTE_INT32  FTE_SHELL_cmdSet(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
FTE_INT32  FTE_SHELL_main(const SHELL_COMMAND_STRUCT   pShellCmds[], FTE_SHELL_CONFIG_PTR pConfig);
FTE_RET    FTE_SHELL_fgetc(MQX_FILE_PTR pFile, FTE_CHAR_PTR pChar, FTE_UINT32 ulTimeout);
FTE_RET    FTE_SHELL_fgets(MQX_FILE_PTR pFile, FTE_CHAR_PTR pTTYLine, FTE_RET nSize, FTE_UINT32 ulTimeout);

const 
SHELL_COMMAND_STRUCT pSHELLCommands[] = 
{
   /* RTCS commands */ 

#if FTE_AD7785_SUPPORTED
    { "ad7785",      FTE_AD7785_SHELL_cmd},
#endif
    { "bl",         FTE_SYS_BL_cmd},    
    { "cert",       FTE_CFG_CERT_SHELL_cmd},    
#if FTE_CIAS_SIOUX_CU_SUPPORTED
    { "cias",       FTE_CIAS_SIOUX_CU_SHELL_cmd},
#endif
    { "config",     FTE_CFG_SHELL_cmd},
#if FTE_1WIRE_SUPPORTED
    { "1wire",      FTE_1WIRE_SHELL_cmd},
#endif
    { "date",       FTE_TIME_SHELL_cmd},
    { "db",         FTE_DB_SHELL_cmd},
#if FTE_DO_SUPPORTED
    { "do",         FTE_DO_SHELL_cmd},
#endif
#if FTE_MULTI_DIO_NODE_SUPPORTED
	{ "dio",  FTE_DIO_SHELL_cmd},
#endif
#if FTE_DOTECH_SUPPORTED
    { "dotech",     FTE_DOTECH_SHELL_cmd},
#endif
#if FTE_DS18B20_SUPPORTED
    { "ds",         FTE_DS18B20_SHELL_cmd},
#endif
    { "event",      FTE_EVENT_shell_cmd},
    { "exit",       Shell_exit},
    { "get",        FTE_SHELL_cmdGet},
    { "gpio",       FTE_GPIO_SHELL_cmd},

#if FTE_IOEX_SUPPORTED
    {"ioex",    FTE_IOEX_SHELL_cmd},
#endif
#if FTE_TASCON_HEM12_06M_SUPPORTED
    { "hem12",      FTE_TASCON_HEM12_SHELL_cmd},
#endif
#if FTE_I2C_SUPPORTED
    { "i2c",        FTE_I2C_SHELL_cmd },
#endif
    { "ifconfig",   FTE_NET_SHELL_cmd },
#if FTE_LOG_SUPPORTED
    { "log",        FTE_LOG_SHELL_cmd},
#endif

#if FTE_LORAWAN_SUPPORTED
    { "lora",       FTE_LORAWAN_SHELL_cmd},
#endif

#if FTE_MCP23S08_SUPPORTED
    {"mcp23s08",    FTE_MCP23S08_SHELL_cmd},
#endif
#if FTE_MBTCP_SUPPORTED
    { "mbtcps",      FTE_MBTCP_SLAVE_SHELL_cmd},
#endif
#if FTE_MBTCP_MASTER_SUPPORTED
    { "mbtcpm",      FTE_MBTCP_MASTER_SHELL_cmd},
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
#if FTE_SOHA_SUPPORTED
    { "soha",       FTE_SOHA_SHELL_cmd},
#endif
#if FTE_SSD1305_SUPPORTED
    { "lcd",        FTE_SSD1305_SHELL_cmd},
#endif
    { "save",       FTE_CFG_SHELL_cmdSave},
#if FTE_SHT_SUPPORTED
    { "sht",        FTE_SHT_SHELL_cmd},
#endif
    { "smng",       FTE_SMNGD_SHELL_cmd},
#if FTE_SPI_SUPPORTED
    { "spi",        FTE_SPI_SHELL_cmd},
#endif
    { "shutdown",   FTE_SYS_SHUTDOWN_SHELL_cmd},
    { "sys",        FTE_SYS_SHELL_cmd},
    { "task",       FTE_TASK_SHELL_cmd},
#if FTE_TURBOMAX_SUPPORTED
    { "turbomax",      FTE_TURBOMAX_SHELL_cmd},
#endif
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

FTE_RET     FTE_SHELL_proc(void)
{
    FTE_RET     nResult;
    FTE_CHAR    nCh;
    FTE_SHELL_CONFIG_PTR    pShellConfig = FTE_CFG_SHELL_get();
    FTE_CHAR    pUserID[FTE_SHELL_USER_ID_LENGTH+1];
    FTE_CHAR    pPasswd[FTE_SHELL_PASSWD_LENGTH+1];
    /* Run the shell */
 
    if (FTE_SHELL_fgetc(stdin, &nCh, pShellConfig->ulTimeout) != FTE_RET_OK)
    {
        return  FTE_RET_OK;
    }

    fprintf(stdout, "\n\nWelcome to %s!\n", FTE_MODEL);
    fflush(stdout);
    while(1)
    {
        memset(pUserID, 0, sizeof(pUserID));
        memset(pPasswd, 0, sizeof(pPasswd));
        
        fprintf(stdout, "ID : ");    
        fflush(stdout);
        nResult = FTE_SHELL_fgets(stdin, pUserID, FTE_SHELL_USER_ID_LENGTH, pShellConfig->ulTimeout);
        if (nResult != FTE_RET_OK)
        {
            break;
        }
        fprintf(stdout, "passwd : ");    
        fflush(stdout);
        nResult = FTE_SHELL_getPasswd(stdin, pPasswd, FTE_SHELL_PASSWD_LENGTH, pShellConfig->ulTimeout);
        if (nResult != FTE_RET_OK)
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
    
    return  FTE_RET_OK;
}

FTE_UINT32  FTE_SHELL_getPasswd
(
    MQX_FILE_PTR pFile, 
    FTE_CHAR_PTR pPasswd, 
    FTE_UINT32  ulMaxLen, 
    FTE_UINT32  ulTimeout
)
{
    FTE_BOOL     bExit = FALSE;
    FTE_CHAR    ch;
    FTE_UINT32  ulLen = 0;
    FTE_UINT32  ulFlags;
    
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

    return  FTE_RET_OK;
}

FTE_INT32  FTE_SHELL_cmdPasswd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL              bPrintUsage, bShortHelp = FALSE;
    FTE_INT32               nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {    
                FTE_SHELL_CONFIG_PTR    pShellConfig = FTE_CFG_SHELL_get();
                FTE_CHAR    pOldPasswd[FTE_SHELL_PASSWD_LENGTH+1];
                FTE_CHAR    pNewPasswd[FTE_SHELL_PASSWD_LENGTH+1];
                FTE_CHAR    pNewPasswdConfirm[FTE_SHELL_PASSWD_LENGTH+1];

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

FTE_INT32  FTE_SHELL_cmdGet
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL              bPrintUsage, bShortHelp = FALSE;
    FTE_INT32               nReturnCode = SHELL_EXIT_SUCCESS;
    
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

FTE_INT32  FTE_SHELL_cmdSet
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL              bPrintUsage, bShortHelp = FALSE;
    FTE_INT32               nReturnCode = SHELL_EXIT_SUCCESS;
    
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
                FTE_UINT32                 ulTimeout;
                FTE_SHELL_CONFIG_PTR    pConfig = FTE_CFG_SHELL_get();
                    
                if (FTE_strToUINT32( pArgv[2], &ulTimeout) != FTE_RET_OK)  
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

FTE_INT32 FTE_SHELL_main
(
    const SHELL_COMMAND_STRUCT  pShellCmds[],
    FTE_SHELL_CONFIG_PTR        pConfig
)
{ /* Body */
    SHELL_CONTEXT_PTR    pShell;
    FTE_INT32               return_code = SHELL_EXIT_SUCCESS;
    FTE_UINT32              i;

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
                FTE_RET    nResult;
                nResult = FTE_SHELL_fgets(pShell->COMMAND_FP, pShell->CMD_LINE, sizeof(pShell->CMD_LINE), pConfig->ulTimeout);
               
               if (nResult != FTE_RET_OK) 
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


FTE_RET    FTE_SHELL_fgetc
(
    MQX_FILE_PTR    pFile, 
    FTE_CHAR_PTR    pChar, 
    FTE_UINT32      ulTimeout
)
{
    FTE_INT32   nValue;
    
    if (_io_fstatus(pFile) != TRUE)
    {        
        MQX_TICK_STRUCT xStartTick;
        _time_get_elapsed_ticks(&xStartTick);
        
        while(_io_fstatus(pFile) != TRUE)
        {
            MQX_TICK_STRUCT xCurrentTick;
            FTE_BOOL     bOverflow = FALSE;
            
            _time_get_elapsed_ticks(&xCurrentTick);
            
            if ((ulTimeout != 0) && (_time_diff_seconds(&xCurrentTick, &xStartTick, &bOverflow) > ulTimeout))
            {
                return  MQX_ETIMEDOUT;
            }
            
            _time_delay(1);
        }
    }

    nValue = _io_fgetc(pFile);
    
    if (nValue == IO_EOF)
    {
        return  FS_EOF;
    }
    
    *pChar = (FTE_CHAR)nValue;
    
    return  FTE_RET_OK;
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
FTE_RET FTE_SHELL_fgets
(
    MQX_FILE_PTR    pFile,
    FTE_CHAR_PTR    pBuff,
    FTE_RET         nMaxLen,
    FTE_UINT32     ulTimeout
) 
{ /* Body */
    FTE_RET     nResult;
    FTE_CHAR    c;
    FTE_INT32   i;
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
    if (nResult != FTE_RET_OK)
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
            *pBuff++ = (FTE_CHAR    )c;
            ++i;
        } /* Endif */
        
        nResult = FTE_SHELL_fgetc(pFile, &c, ulTimeout);
        if (nResult != FTE_RET_OK) 
        {
            *pBuff = '\0'; /* null terminate the string before returning */
            return nResult;
        } /* Endif */
    } /* Endwhile */

    if (i >= nMaxLen) 
    {
        _io_fungetc((FTE_RET)c, pFile);
    } /* Endif */

    *pBuff = '\0';

    return FTE_RET_OK;
} /* Endbody */


FTE_UINT32  FTE_SHELL_printHexString
(   
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      ulSize, 
    FTE_UINT32      ulColumn
)
{
    FTE_UINT32    ulLen = 0;
    for(FTE_INT32 i = 0 ; i < ulSize ; i++)
    {
        ulLen += fprintf(stdout, "%02x ", pData[i]);
        if ((ulColumn != 0) && ((i+1) % ulColumn == 0))
        {
            ulLen += fprintf(stdout, "\n", pData[i]);
        }
    }
    
    return  ulLen;
}

FTE_UINT32  FTE_SHELL_printNumString
(
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      ulSize, 
    FTE_UINT32      ulColumn
)
{
    FTE_UINT32    ulLen = 0;
    for(FTE_INT32 i = 0 ; i < ulSize ; i++)
    {
        ulLen += fprintf(stdout, "%3d ", pData[i]);
        if ((ulColumn != 0) && ((i+1) % ulColumn == 0))
        {
            ulLen += fprintf(stdout, "\n", pData[i]);
        }
    }
    
    return  ulLen;
}