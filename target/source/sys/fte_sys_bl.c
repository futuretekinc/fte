#include "fte_target.h"
#include "fapp_params.h"
#include "fapp_mem.h"
#include "fte_sys_bl.h"

#define IPBYTES(a)            ((a)&0xFF),(((a)>>8)&0xFF),(((a)>> 16)&0xFF),(((a)>>24)&0xFF)
#define IPADDR(a,b,c,d)       (((uFTE_INT32)(a)&0xFF)|(((uFTE_INT32)(b)&0xFF)<<8)|(((uFTE_INT32)(c)&0xFF)<<16)|(((uFTE_INT32)(d)&0xFF)<<24))

static 
FTE_BL_STATIC_PARAMS _PTR_  pBLStaticParams = (FTE_BL_STATIC_PARAMS _PTR_)FTE_BL_STATIC_PARAMS_ADDRESS;
static 
FTE_BL_DYNAMIC_PARAMS _PTR_ pBLDynamicParams = NULL;

FTE_BOOL FTE_SYS_BL_checkStaticParams(void)
{
    if (pBLStaticParams == NULL)
    {
        FTE_BL_STATIC_PARAMS _PTR_ pParams = (FTE_BL_STATIC_PARAMS _PTR_)FTE_BL_STATIC_PARAMS_ADDRESS;

        if ((strcmp(pParams->signature, FTE_BL_PARAMS_SIGNATURE) != 0) || 
            (pParams->ulCRC32 != FTE_CRC32(0, (FTE_UINT8_PTR)pParams, sizeof(FTE_BL_STATIC_PARAMS) - sizeof(unsigned long))))
        {
            return  FALSE;
        }
    }
    
    return  TRUE;
} 

FTE_RET       FTE_SYS_BL_getOID
(
    FTE_UINT8_PTR   pOID
)
{
    ASSERT(pOID != NULL);

    if (FTE_SYS_BL_checkStaticParams() != TRUE)
    {
        return  FTE_RET_ERROR;
    }

    memcpy(pOID, pBLStaticParams->device.id, FTE_OID_SIZE);
    
    return  FTE_RET_OK;   
}


FTE_RET       FTE_SYS_BL_getMAC
(
    FTE_UINT8_PTR   pMAC
)
{
    ASSERT(pMAC != NULL);
    
    if (FTE_SYS_BL_checkStaticParams() != TRUE)
    {
        return  FTE_RET_ERROR;
    }

    memcpy(pMAC, pBLStaticParams->device.mac, FTE_MAC_SIZE);
    
    return  FTE_RET_OK;   
}

FTE_RET   FTE_SYS_BL_startUpgrade(void)
{
    if (FTE_SYS_BL_load() != FTE_RET_OK)
    {
        printf("ERROR : BL not loaded\n");
    }
    
    pBLDynamicParams->boot.mode = FAPP_PARAMS_BOOT_MODE_SCRIPT;
    if (FTE_SYS_BL_save() == FTE_RET_OK)
    {
        FTE_SYS_reset();
    }
  
    return  FTE_RET_OK;
}

FTE_RET   FTE_SYS_BL_load(void)
{
    MQX_FILE_PTR    fp;
    
    if (pBLDynamicParams == NULL)
    {
        pBLDynamicParams = (FTE_BL_DYNAMIC_PARAMS _PTR_)FTE_MEM_allocZero(sizeof(FTE_BL_DYNAMIC_PARAMS));
        if (pBLDynamicParams == NULL)
        {
            return  FTE_RET_ERROR;
        }
        
        fp = fopen("flashx:bl_dynamic", NULL);
        if (fp != NULL) 
        {
            ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
            
            if (sizeof(FTE_BL_DYNAMIC_PARAMS) != read(fp, (pointer)pBLDynamicParams, sizeof(FTE_BL_DYNAMIC_PARAMS)))
            {
                fprintf(stderr, "\nError reading from the file. Error code: %d", _io_ferror(fp));
                fclose(fp);
                
                goto error;
            }
            fclose(fp);
            
            
        }
    }
    
    return  FTE_RET_OK;
    
error:
    if (pBLDynamicParams != NULL)
    {
        FTE_MEM_free(pBLDynamicParams);
        pBLDynamicParams = NULL;
    }
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_SYS_BL_save(void)
{
    MQX_FILE_PTR    fp;
    
    if (pBLDynamicParams == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    fp = fopen("flashx:bl_dynamic", NULL);
    if (fp != NULL) 
    {
        ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
        
        pBLDynamicParams->ulCRC32 = FTE_CRC32(0, pBLDynamicParams, sizeof(FTE_BL_DYNAMIC_PARAMS) - sizeof(unsigned long));
        if (sizeof(FTE_BL_DYNAMIC_PARAMS) != write(fp, (pointer)pBLDynamicParams, sizeof(FTE_BL_DYNAMIC_PARAMS)))
        {
            fprintf(stderr, "\nError writing to the file. Error code: %d", _io_ferror(fp));
            fclose(fp);
            
            goto error;
        }

        fflush(fp);
        fclose(fp);
    }

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_INT32  FTE_SYS_BL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nReturnCode = SHELL_EXIT_SUCCESS;
    
    if (FTE_SYS_BL_load() != FTE_RET_OK)
    {
        printf("ERROR : BL not loaded\n");
    }
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                printf(" %7s : %s\n",           "ID",           pBLStaticParams->device.id);
                printf(" %7s : %d.%d.%d.%d\n",  "B/L Ver",  
                       (pBLStaticParams->device.version.loader >> 24) & 0xFF,
                       (pBLStaticParams->device.version.loader >> 16) & 0xFF,
                       (pBLStaticParams->device.version.loader >>  8) & 0xFF,
                       (pBLStaticParams->device.version.loader >>  0) & 0xFF);
                printf(" %7s : %d.%d.%d.%d\n",  "H/W Ver", 
                       (pBLStaticParams->device.version.hardware >> 24) & 0xFF,
                       (pBLStaticParams->device.version.hardware >> 16) & 0xFF,
                       (pBLStaticParams->device.version.hardware >>  8) & 0xFF,
                       (pBLStaticParams->device.version.hardware >>  0) & 0xFF);
                       
                printf(" %7s : %d.%d.%d.%d\n",  "ip",        IPBYTES(pBLDynamicParams->fnet.address));
                printf(" %7s : %d.%d.%d.%d\n",  "netmask",   IPBYTES(pBLDynamicParams->fnet.netmask));
                printf(" %7s : %d.%d.%d.%d\n",  "gateway",   IPBYTES(pBLDynamicParams->fnet.gateway));
                switch(pBLDynamicParams->boot.mode)
                {
                case    FAPP_PARAMS_BOOT_MODE_STOP: printf(" %7s : stop\n", "boot");    break;
                case    FAPP_PARAMS_BOOT_MODE_GO:   printf(" %7s : go\n", "boot");      break;
                case FAPP_PARAMS_BOOT_MODE_SCRIPT:  printf(" %7s : script\n", "boot");  break;
                }
                printf(" %7s : %s\n", "script", pBLDynamicParams->boot.script);
                printf(" %7s : 0x%08x\n", "raw", pBLDynamicParams->tftp.file_raw_address);
                printf(" %7s : %d.%d.%d.%d\n", "tftp", IPBYTES(pBLDynamicParams->tftp.server));
                printf(" %7s : %s\n", "image", pBLDynamicParams->tftp.file_name);
                
                switch(pBLDynamicParams->tftp.file_type)
                {
                case    FAPP_PARAMS_TFTP_FILE_TYPE_RAW: printf(" %7s : raw\n", "type");  break;
                case    FAPP_PARAMS_TFTP_FILE_TYPE_BIN: printf(" %7s : bin\n", "type");  break;
                case    FAPP_PARAMS_TFTP_FILE_TYPE_SREC: printf(" %7s : srec\n", "type"); break;
                }
                
                printf(" %7s : 0x%08x\n", "go", pBLDynamicParams->boot.go_address);
            }
            break;     
            
        case    2:
            {
                if (strcmp(pArgv[1], "reset") == 0)
                {
                    pBLDynamicParams->boot.mode = FAPP_PARAMS_BOOT_MODE_SCRIPT;
                    strcpy(pBLDynamicParams->boot.script, "dhcp;! reset;telnet;! reset");
                    
                    if (FTE_SYS_BL_save() == FTE_RET_OK)
                    {
                        FTE_SYS_reset();
                    }
                }
            }
            break;

        case    4:
            {
                if (strcmp(pArgv[1], "set") == 0)
                {
                    if (strcmp(pArgv[2], "script") == 0)
                    {
                        if (strlen(pArgv[3]) >= sizeof(pBLDynamicParams->boot.script))
                        {
                            printf("Invalid parameter\n");
                            break;
                        }
                        pBLDynamicParams->boot.mode = FAPP_PARAMS_BOOT_MODE_SCRIPT;
                        memset(pBLDynamicParams->boot.script, 0, sizeof(pBLDynamicParams->boot.script));
                        strncpy(pBLDynamicParams->boot.script, pArgv[3], sizeof(pBLDynamicParams->boot.script) - 1);
                    }         
                    else if (strcmp(pArgv[2], "image") == 0)
                    {
                        if (strlen(pArgv[3]) >= sizeof(pBLDynamicParams->tftp.file_name))
                        {
                            printf("Invalid parameter\n");
                            break;
                        }
                            
                        memset(pBLDynamicParams->tftp.file_name, 0, sizeof(pBLDynamicParams->tftp.file_name));
                        strncpy(pBLDynamicParams->tftp.file_name, pArgv[3], sizeof(pBLDynamicParams->tftp.file_name) - 1);
                    }
                    else
                    {
                        break;
                    }
                    
                    if (FTE_SYS_BL_save() == FTE_RET_OK)
                    {
                        printf("Bootloader configuration changed!\n");
                    }
                }
            }
            break;
        }
    }
    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<command>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<command>]\n", pArgv[0]);
            printf("  Commands:\n");
            printf("    set <param>\n");
            printf("        Set bootloader parameters\n");
            printf("        'set image <image_name>' to change image name\n");
            printf("        'set script <script>' to change boot script\n");                   
            printf("    reset\n");
            printf("        Change to bootloader mode\n");
        }
    }

    return  0;
}
