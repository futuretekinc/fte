#include "fte_target.h"
#include "fte_log.h"
#include "fte_time.h"
#include "fte_value.h"
 
typedef struct _FTE_LOG_POOL_STRUCT
{
    uint_32             ulCRC;
    uint_32             ulTag;
    uint_32             ulID;
    uint_32             ulCount;
    uint_32             ulHead;
    uint_32             ulTail;
    FTE_LOG             pLogs[(0x800 - sizeof(uint_32) * 6) / sizeof(FTE_LOG)];
}   FTE_LOG_POOL, _PTR_ FTE_LOG_POOL_PTR;

#define MAX_LOG_COUNT   ((0x800 - sizeof(uint_32) * 6) / sizeof(FTE_LOG))
#define FTE_LOG_TAG     0x12345678

static  FTE_LOG_POOL        xPool;
static  boolean             bPoolModified = FALSE;
static  char *              pMTDs[] = 
{
    "flashx:data6",
    "flashx:data7"
};

static  LWSEM_STRUCT        xLWSEM;

_mqx_uint   FTE_LOG_init(void)
{
    FTE_LOG_POOL_PTR pPool;
    
    memset(&xPool, 0, sizeof(xPool));

    if (_lwsem_create(&xLWSEM, 1) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pPool = (FTE_LOG_POOL_PTR)FTE_MEM_allocZero(sizeof(FTE_LOG_POOL));
    if (pPool == NULL)
    {
        _lwsem_destroy(&xLWSEM);
        return  MQX_ERROR;
    }
    
    for(int nMTD = 0 ; nMTD < 1 ; nMTD++)
    {
        MQX_FILE_PTR    fp;    
        
        /* Open the flash device */
        fp = fopen(pMTDs[nMTD], NULL);
        if (fp == NULL) 
        {
            continue;
        }

        ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
        read(fp, (char_ptr)pPool, sizeof(FTE_LOG_POOL));
        fclose(fp);
        
        if ((pPool->ulCRC != fte_crc32(0, (char_ptr)&pPool->ulTag, sizeof(FTE_LOG_POOL) - sizeof(uint_32))) ||
            (pPool->ulTag != FTE_LOG_TAG) || 
            (pPool->ulID > (uint_32)MAX_INT_32))
        {
            continue;
        }
            
        if (pPool->ulID > xPool.ulID)
        {
            memcpy(&xPool, pPool, sizeof(FTE_LOG_POOL));
        }
    }       
    
    if (xPool.ulTag != FTE_LOG_TAG)
    {   
        bPoolModified = TRUE;
        FTE_LOG_save();
    
    }
    
    FTE_MEM_free(pPool);
    
    return  MQX_OK;
}

_mqx_uint   FTE_LOG_save(void)
{
    MQX_FILE_PTR    fp;    
        
    if (!bPoolModified)
    {
        return  MQX_OK;
    }
    
    if (_lwsem_wait(&xLWSEM) != MQX_OK)
    {  
        DEBUG("\n_xLWSEM_wait failed");
    }

    xPool.ulID++;
    xPool.ulTag = FTE_LOG_TAG;        
    xPool.ulCRC = fte_crc32(0, (pointer)&xPool.ulTag, sizeof(FTE_LOG_POOL) - sizeof(uint_32));   
    
    int nMTD = xPool.ulID & 0x01;

    /* Open the flash device */
    fp = fopen(pMTDs[nMTD], NULL);
    if (fp != NULL) 
    {
        ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
        
        if (sizeof(FTE_LOG_POOL) != write(fp, (pointer)&xPool, sizeof(FTE_LOG_POOL)))
        {
            fprintf(stderr, "\nError writing to the file. Error code: %d", _io_ferror(fp));
        }

        fflush(fp);
        fclose(fp);
    }
    
    if (_lwsem_post(&xLWSEM) != MQX_OK)
    {
        DEBUG("\n_xLWSEM_post failed");
    }

    return  MQX_OK;
}
_mqx_uint   FTE_LOG_addSystem(FTE_LOG_SYSTEM_MESSAGE xMsg)
{
    if (_lwsem_wait(&xLWSEM) != MQX_OK)
    {  
        DEBUG("\n_xLWSEM_wait failed");
    }
    
    xPool.pLogs[xPool.ulTail].xType         = FTE_LOG_TYPE_SYSTEM;
    _time_get(&xPool.pLogs[xPool.ulTail].xTimeStamp);
    xPool.pLogs[xPool.ulTail].xParam.xSystem.xMsg = xMsg;
    
    xPool.ulTail = (xPool.ulTail + 1) % MAX_LOG_COUNT;
    
    if (++xPool.ulCount >= MAX_LOG_COUNT)
    {
        xPool.ulHead = (xPool.ulHead + 1) % MAX_LOG_COUNT;
        xPool.ulCount = MAX_LOG_COUNT - 1;
    }
    
    bPoolModified = TRUE;
    
    if (_lwsem_post(&xLWSEM) != MQX_OK)
    {
        DEBUG("\n_xLWSEM_post failed");
    }

    return  MQX_OK;
}

_mqx_uint   FTE_LOG_addEvent(FTE_OBJECT_ID   nOID, uint_32 ulLevel, FTE_VALUE_PTR pValue)
{
    ASSERT(pValue != NULL);
  
    if (_lwsem_wait(&xLWSEM) != MQX_OK)
    {  
        DEBUG("\n_xLWSEM_wait failed");
    }

    xPool.pLogs[xPool.ulTail].xType         = FTE_LOG_TYPE_EVENT;
    _time_get(&xPool.pLogs[xPool.ulTail].xTimeStamp);
    xPool.pLogs[xPool.ulTail].xParam.xEvent.ulEPID  = nOID;
    xPool.pLogs[xPool.ulTail].xParam.xEvent.ulLevel = ulLevel;
    xPool.pLogs[xPool.ulTail].xParam.xEvent.xValue.ulType = pValue->xType;
    xPool.pLogs[xPool.ulTail].xParam.xEvent.xValue.ulValue= pValue->xData.ulValue;
    
    xPool.ulTail = (xPool.ulTail + 1) % MAX_LOG_COUNT;
    
    if (++xPool.ulCount >= MAX_LOG_COUNT)
    {
        xPool.ulHead = (xPool.ulHead + 1) % MAX_LOG_COUNT;
        xPool.ulCount = MAX_LOG_COUNT - 1;
    }
    
    bPoolModified = TRUE;

    if (_lwsem_post(&xLWSEM) != MQX_OK)
    {
        DEBUG("\n_xLWSEM_post failed");
    }

    return  MQX_OK;
}

uint_32 FTE_LOG_del(uint_32 ulCount)
{
    uint_32 ulDeletedCount = 0;
    
    if (_lwsem_wait(&xLWSEM) != MQX_OK)
    {  
        DEBUG("\n_xLWSEM_wait failed");
    }

    if (ulCount >= xPool.ulCount)
    {
        ulDeletedCount = xPool.ulCount;
        xPool.ulHead = xPool.ulTail;
        xPool.ulCount = 0;
    }
    else
    {
        ulDeletedCount = ulCount;
        xPool.ulHead = (xPool.ulHead + ulCount) % MAX_LOG_COUNT;
        xPool.ulCount -= ulCount;
    }
    
    if (_lwsem_post(&xLWSEM) != MQX_OK)
    {
        DEBUG("\n_xLWSEM_post failed");
    }

    return  ulDeletedCount;
}

int         FTE_LOG_count(void)
{
    return  xPool.ulCount;
}

FTE_LOG_PTR FTE_LOG_getAt(uint_32 nIndex)
{
    if ((xPool.ulCount <= 0) || (nIndex >= xPool.ulCount))
    {
        return  NULL;
    }
        
    nIndex = (xPool.ulHead + nIndex) % MAX_LOG_COUNT;    
    return  &xPool.pLogs[nIndex];
}

static char_ptr _pSystemMessages[] = 
{
    "system start!"
};

char_ptr    FTE_LOG_getSystemMessageString(FTE_LOG_SYSTEM_MESSAGE xMsg)
{
    if (xMsg < FTE_LOG_SYSTEM_MESSAGE_END)
    {
        return  _pSystemMessages[xMsg];
    }
    
    return  "";
}

int_32          FTE_LOG_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
                int_32 i;
                
                for(i =  xPool.ulCount - 1 ; i >= 0; i--)
                {
                    char    pTimeString[64];
                    
                    uint_32 index = (xPool.ulHead + i) % MAX_LOG_COUNT;
                
                    switch(xPool.pLogs[index].xType)
                    {
                    case    FTE_LOG_TYPE_SYSTEM:
                        {
                            FTE_TIME_toString(&xPool.pLogs[index].xTimeStamp, pTimeString, sizeof(pTimeString));
                            printf("%4d : %s %s\n", 
                                   xPool.ulCount - i, 
                                   pTimeString, 
                                   FTE_LOG_getSystemMessageString(xPool.pLogs[index].xParam.xSystem.xMsg));
                        }
                        break;
                        
                    case    FTE_LOG_TYPE_EVENT:
                        {
                            FTE_TIME_toString(&xPool.pLogs[index].xTimeStamp, pTimeString, sizeof(pTimeString));
                            printf("%4d : %s %08x\n", 
                                   xPool.ulCount - i, 
                                   pTimeString, 
                                   xPool.pLogs[index].xParam.xEvent.ulEPID);
                        }
                        break;
                    }
                }
            }
            break;
            
        case    3:
            {
                if (strcmp(argv[1], "del") == 0)
                {
                    uint_32 ulCount;
                    
                    if (! Shell_parse_number( argv[2], &ulCount))  
                    {
                        return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    ulCount = FTE_LOG_del(ulCount);
                    
                    printf("%lu logs have been deleted..\n", ulCount);
                }
            }
            break;
            
        default:
            print_usage = TRUE;
        }
    }

error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<Commands>]\n", argv[0]);
        }
        else
        {
            printf ("Usage: %s [<Commands>]\n", argv[0]);
            printf ("  Commands :\n");
            printf ("    del <count>\n");
            printf ("        Delete the oldest n logs.\n");
            printf ("  Parameters:\n");
            printf ("    %-10s = Log count.\n", "<count>");
        }
    }
    return   return_code;
}
