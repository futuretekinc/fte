#include "fte_target.h"
#include "fte_crc.h"
#include "fte_m25p16.h"
#include "fte_db.h"

#define FTE_DB_TAG              0x5894a3f1

#define FTE_DB_PAGE_SIZE        256
#define FTE_DB_PAGE_PER_SECTOR  256
#define FTE_DB_SECTOR_SIZE      (FTE_DB_PAGE_PER_SECTOR * FTE_DB_PAGE_SIZE)

#define FTE_DB_SECTOR_MAX       31
#define FTE_DB_BLOCK_MAX        30

typedef FTE_UINT32 FTE_DB_SECTOR_TYPE;

#define FTE_DB_SECTOR_TYPE_UNUSED       0xFFFFFFFF
#define FTE_DB_SECTOR_TYPE_OBJECT       0x5AA5F00F
#define FTE_DB_SECTOR_TYPE_LOG          0x5AA5C33C
#define FTE_DB_SECTOR_TYPE_DESTROYED    0x00000000
#define FTE_DB_SECTOR_TYPE_CRASHED      0xF0F0F0F0

typedef struct  FTE_DB_OBJECT_HEAD_STRUCT
{
    FTE_OBJECT_ID       xID;
    uint_8              pUsedFlags[FTE_DB_PAGE_PER_SECTOR / 8];
}   FTE_DB_OBJECT_HEAD, _PTR_ FTE_DB_OBJECT_HEAD_PTR;

typedef struct  FTE_DB_LOG_HEAD_STRUCT
{
    uint_8              pUsedFlags[FTE_DB_PAGE_PER_SECTOR / 8];
}   FTE_DB_LOG_HEAD, _PTR_ FTE_DB_LOG_HEAD_PTR;

typedef struct  FTE_DB_OBJECT_STRUCT
{
    FTE_OBJECT_ID       xID;
    FTE_UINT32             ulCount;
    FTE_UINT32             ulSectorIndex;
}   FTE_DB_OBJECT, _PTR_ FTE_DB_OBJECT_PTR;

typedef struct FTE_DB_SECTOR_HEAD_STRUCT
{
    FTE_DB_SECTOR_TYPE  xType;
    union
    {
        FTE_DB_OBJECT_HEAD  xObject;
        FTE_DB_LOG_HEAD     xLog;
    }   xParams;    
}   FTE_DB_SECTOR_HEAD, _PTR_ FTE_DB_SECTOR_HEAD_PTR;

typedef struct  FTE_DB_OBJECT_SECTOR_STRUCT
{
    FTE_DB_SECTOR_TYPE  xType;
    FTE_OBJECT_ID       xID;
    FTE_UINT32             ulPageGroupIndex;
    FTE_UINT32             ulPageGroup;
    FTE_UINT32             ulSlotGroupIndex;
    FTE_UINT32             ulSlotGroup;
}   FTE_DB_OBJECT_SECTOR, _PTR_ FTE_DB_OBJECT_SECTOR_PTR;

typedef struct  FTE_DB_HEAD_STRUCT
{
    FTE_UINT32         ulCRC;
    FTE_UINT32         ulTag;
    FTE_UINT32         ulID;
}   FTE_DB_HEAD, _PTR_ FTE_DB_HEAD_PTR;

typedef struct  FTE_DB_STRUCT
{
    FTE_UINT32                 ulBaseAddress;
    FTE_UINT32                 ulSectorCount;
    FTE_M25P16_PTR          pM25P16;
    FTE_DB_HEAD             xHead;
    FTE_DB_OBJECT_SECTOR    pSectors[FTE_DB_SECTOR_MAX];
    FTE_UINT32                 ulObjectCount;
    FTE_DB_OBJECT           pObjects[FTE_DB_SECTOR_MAX];
}   FTE_DB_FLASH, _PTR_ FTE_DB_PTR;


static FTE_RET FTE_DB_HEAD_init(FTE_DB_PTR pFlash);

static FTE_RET FTE_DB_HEAD_read(FTE_DB_PTR pFlash);
static FTE_RET FTE_DB_HEAD_write(FTE_DB_PTR pFlash);
static FTE_RET FTE_DB_dump(FTE_UINT32 ulAddress, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);

static FTE_RET FTE_DB_SECTOR_read(FTE_DB_PTR pFlash, FTE_UINT32 ulIndex, FTE_UINT32 ulOffset, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);
static FTE_RET FTE_DB_SECTOR_write(FTE_DB_PTR pFlash, FTE_UINT32 ulIndex, FTE_UINT32 ulOffset, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);
static FTE_RET FTE_DB_SECTOR_erase(FTE_DB_PTR pFlash, FTE_UINT32 ulIndex);

        FTE_RET FTE_DB_SECTOR_getHead(FTE_DB_PTR pFlash, FTE_UINT32 ulSectorIndex, FTE_DB_SECTOR_HEAD_PTR pSectorHead);
        FTE_RET FTE_DB_SECTOR_setHead(FTE_DB_PTR pFlash, FTE_UINT32 ulSectorIndex, FTE_DB_SECTOR_HEAD_PTR pSectorHead);


FTE_DB_PTR    pFlash = NULL;

FTE_RET   FTE_DB_init
(
    FTE_BOOL    bForce
)
{
    FTE_RET       ulRet;
    FTE_M25P16_PTR  pM25P16;
    
    pM25P16 = FTE_M25P16_get(FTE_DEV_M25P16_0);
    if (pM25P16 == NULL)
    {
        goto error;
    }

    if (FTE_M25P16_attach(pM25P16, 0) != MQX_OK)
    {
        goto error;
    }

    if (FTE_M25P16_isExist(pM25P16) != TRUE)
    {
        goto error;
    }
                             
    pFlash = (FTE_DB_PTR)FTE_MEM_allocZero(sizeof(FTE_DB_FLASH));
    if (pFlash == NULL)
    {
        return  MQX_ERROR;
    }
    
    pFlash->ulBaseAddress   = 0;
    pFlash->ulSectorCount   = FTE_DB_SECTOR_MAX;
    pFlash->pM25P16         = pM25P16;
    
    if (bForce || (FTE_DB_HEAD_read(pFlash) != MQX_OK))
    {
        FTE_DB_HEAD_init(pFlash);
    }
    else
    {
        int i;
        
        for(i = 0 ; i < FTE_DB_SECTOR_MAX ; i++)
        {
            FTE_DB_SECTOR_HEAD  xHead;
            
            ulRet = FTE_DB_SECTOR_read(pFlash, i, 0, (FTE_UINT8_PTR)&xHead, sizeof(xHead));
            if (ulRet != MQX_OK)
            {
                pFlash->pSectors[i].xType = FTE_DB_SECTOR_TYPE_CRASHED;
            }
            else
            {
                switch(xHead.xType)
                {
                case    FTE_DB_SECTOR_TYPE_OBJECT:
                    {
                        int nPageGroupIndex;
                        FTE_UINT32 pPageGroups[16];
                        FTE_UINT32 ulPageGroupState;
                        FTE_UINT32 ulPageIndex;
                        FTE_UINT32 ulSlotGroupState;
                        FTE_UINT32 ulSlotIndex;
                        
                        pFlash->pSectors[i].xType   = xHead.xType;
                        pFlash->pSectors[i].xID     = xHead.xParams.xObject.xID;

                        ulRet = FTE_DB_SECTOR_read(pFlash, i, sizeof(FTE_DB_OBJECT_HEAD), (FTE_UINT8_PTR)pPageGroups, sizeof(pPageGroups));
                        if (ulRet != MQX_OK)
                        {
                            pFlash->pSectors[i].xType = FTE_DB_SECTOR_TYPE_CRASHED;
                            break;
                        }
                        
                        for(nPageGroupIndex = 0 ; nPageGroupIndex < 16 ; nPageGroupIndex++)
                        {
                            if (pPageGroups[nPageGroupIndex] != 0)
                            {
                                pFlash->pSectors[i].ulPageGroupIndex= nPageGroupIndex;
                                pFlash->pSectors[i].ulPageGroup     = pPageGroups[nPageGroupIndex];
                                ulPageGroupState                    = pPageGroups[nPageGroupIndex];
                                break;
                            }
                        }

                        for(ulPageIndex = 0 ; ulPageIndex < 16 ; ulPageIndex++)
                        {
                            if ((ulPageGroupState >> (ulPageIndex*2)) != 0)
                            {
                                break;   
                            }
                        }
                        
                        if (ulPageIndex != 16)
                        {
                            FTE_UINT32 ulOffset;
                            
                            ulOffset = sizeof(FTE_DB_OBJECT_HEAD) + sizeof(FTE_UINT32) * 16 + ulPageIndex * 4;
                            
                            ulRet = FTE_DB_SECTOR_read(pFlash, i, ulOffset, (FTE_UINT8_PTR)&ulSlotGroupState, 4);
                            if (ulRet != MQX_OK)
                            {
                                pFlash->pSectors[i].xType = FTE_DB_SECTOR_TYPE_CRASHED;
                                break;
                            }
                            
                            for(ulSlotIndex = 0 ; ulSlotIndex < 32 ; ulSlotIndex++)
                            {
                                if ((ulSlotGroupState >> ulSlotIndex) != 0)
                                {
                                    break;
                                }
                            }
                            
                            pFlash->pSectors[i].ulSlotGroupIndex = pFlash->pSectors[i].ulPageGroupIndex * 16 + ulPageIndex;
                            pFlash->pSectors[i].ulSlotGroup = ulSlotGroupState;
                        }
                        
                        pFlash->ulObjectCount++;
                    }
                    break;
                    
                case    FTE_DB_SECTOR_TYPE_LOG:
                    {
                    }
                    break;
                }
            }            
        }
     }
    
    return  MQX_OK;
error:

    if (pM25P16 != NULL)
    {
        pM25P16 = NULL;
    }
    
    return  MQX_ERROR;
}

FTE_RET   FTE_DB_OBJ_create
(
    FTE_OBJECT_ID   xID
)
{
    FTE_DB_SECTOR_TYPE  xSectorType;
    int i, nSectorIndex = -1, nObjectIndex = -1;
    
    for(i = 0 ; i < pFlash->ulObjectCount ; i++)
    {
        if (pFlash->pObjects[i].xID == xID)
        {
            return  MQX_ALREADY_EXISTS;
        }
    }
    
    for(i = 0 ; i < pFlash->ulSectorCount ; i++)
    {
        if (pFlash->pSectors[i].xType == FTE_DB_SECTOR_TYPE_UNUSED)
        {
            xSectorType = FTE_DB_SECTOR_TYPE_UNUSED;
            nSectorIndex = i;
            break;
        }
    }
    
    if (nSectorIndex < 0)
    {
        for(i = 0 ; i < pFlash->ulSectorCount ; i++)
        {
            if (pFlash->pSectors[i].xType == FTE_DB_SECTOR_TYPE_DESTROYED)
            {
                xSectorType = FTE_DB_SECTOR_TYPE_DESTROYED;
                nSectorIndex = i;
                break;
            }
        }
        
        if (nSectorIndex < 0)
        {
            return  MQX_NOT_ENOUGH_MEMORY;
        }
    }
    
    for(i = 0 ; i < sizeof(pFlash->pObjects) / sizeof(FTE_DB_OBJECT) ; i++)
    {
        if (pFlash->pObjects[i].xID == 0)
        {
            nObjectIndex = i;
            break;
        }
    }
    
    if (nObjectIndex < 0)
    {
        return  MQX_NOT_ENOUGH_MEMORY;
    }
    
    FTE_DB_SECTOR_HEAD  xSectorHead;
    
    xSectorHead.xType = FTE_DB_SECTOR_TYPE_OBJECT;
    xSectorHead.xParams.xObject.xID = xID;

    if (xSectorType == FTE_DB_SECTOR_TYPE_DESTROYED)
    {
        FTE_DB_SECTOR_erase(pFlash, nSectorIndex);
    }

    FTE_DB_SECTOR_write(pFlash, nSectorIndex, 0, (FTE_UINT8_PTR)&xSectorHead, sizeof(xSectorHead)); 
    
    pFlash->pObjects[i].xID             = xID;
    pFlash->pObjects[i].ulSectorIndex   = nSectorIndex;
    pFlash->pObjects[i].ulCount         = 0;
    
    pFlash->ulObjectCount++;
    
    return  MQX_ERROR;
}

FTE_RET   FTE_DB_OBJ_destroy
(
    FTE_OBJECT_ID   xID
)
{
    int i, j;
    
    for(i = 0 ; i < pFlash->ulObjectCount ; i++)
    {
        if (pFlash->pObjects[i].xID == xID)
        {
            FTE_DB_SECTOR_HEAD  xSectorHead;

            xSectorHead.xType = FTE_DB_SECTOR_TYPE_DESTROYED;
            FTE_DB_SECTOR_write(pFlash, pFlash->pObjects[i].ulSectorIndex, 0, (FTE_UINT8_PTR)&xSectorHead, sizeof(xSectorHead)); 

            for(j = i+1 ; j < pFlash->ulObjectCount ; i++, j++)
            {
                memcpy(&pFlash->pObjects[i], &pFlash->pObjects[j], sizeof(FTE_DB_OBJECT));
            }
            
            pFlash->ulObjectCount--;         
            
            return  MQX_OK;
        }
    }
    
    return  MQX_INVALID_OBJECT;
}

FTE_UINT32  FTE_DB_OBJ_count
(
    FTE_OBJECT_ID   xID
)
{
     int i;
    
    for(i = 0 ; i < pFlash->ulObjectCount ; i++)
    {
        if (pFlash->pObjects[i].xID == xID)
        {
            return  pFlash->pObjects[i].ulCount;
        }
    }
    
    return  0;
}

FTE_RET   FTE_DB_OBJ_appendValue
(
    FTE_OBJECT_ID       xID, 
    FTE_DB_VALUE_PTR    pValue
)
{
    int i;
    
    for(i = 0 ; i < pFlash->ulObjectCount ; i++)
    {
        if (pFlash->pObjects[i].xID == xID)
        {
#if 0
            FTE_UINT32 ulOffet = pFlash->pObjects[i].ulCount * 8;            
            pFlash->pObjects[i].ulCount++;
#endif
            
        }
    }
    
    return  MQX_OK;
}


FTE_INT32   FTE_DB_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    switch(nArgc)
    {
    case    1:
        {
            int i;
            if (pFlash == NULL)
            {
                printf("DB not initiallized\n");
                break;
            }
            
            printf("%8s : %08x\n",  "Base", pFlash->ulBaseAddress);
            printf("%8s : %d\n",    "ID", pFlash->xHead.ulID);
            printf("%8s : \n",         "Blocks");
            for(i = 0; i < FTE_DB_BLOCK_MAX; i++)
            {
            }
        }
        break;
        
    case    2:
        {
            if (strcmp(pArgv[1], "init") == 0)
            {
                FTE_DB_init(FALSE);
            }
            else if (strcmp(pArgv[1], "force_init") == 0)
            {
                FTE_DB_init(TRUE);
            }
                
        }
        break;
        
    case    3:
        {
            if (strcmp(pArgv[1], "create") == 0)
            {
                FTE_UINT32 xID;
                
                if (Shell_parse_hexnum(pArgv[2], &xID) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                FTE_DB_OBJ_create(xID);
            }
            else if (strcmp(pArgv[1], "destroy") == 0)
            {
                FTE_UINT32 xID;
                
                if (Shell_parse_hexnum(pArgv[2], &xID) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                FTE_DB_OBJ_destroy(xID);
            }
        }
        break;
    case    4:
        {
            if (strcmp(pArgv[1], "add") == 0)
            {
                TIME_STRUCT     xTime;
                FTE_DB_VALUE    xValue;
                FTE_UINT32         xID;
                
                if (Shell_parse_hexnum(pArgv[2], &xID) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (Shell_parse_hexnum(pArgv[3], &xValue.ulValue) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                _time_get(&xTime);
                
                xValue.xTime = xTime.SECONDS;
                
                FTE_DB_OBJ_appendValue(xID, &xValue);
            }
            else if (strcmp(pArgv[1], "destroy") == 0)
            {
                FTE_UINT32 xID;
                
                if (Shell_parse_hexnum(pArgv[2], &xID) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                FTE_DB_OBJ_destroy(xID);
            }
            else if (strcmp(pArgv[1], "dump") == 0)
            {
                FTE_UINT32 ulAddress, ulLen;
                
                if (Shell_parse_hexnum(pArgv[2], &ulAddress) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                if (Shell_parse_number(pArgv[3], &ulLen) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                FTE_UINT8_PTR  pBuff = FTE_MEM_alloc(ulLen);
                
                if (FTE_DB_dump(ulAddress, pBuff, ulLen) == MQX_OK)
                {
                    int i;
                    for(i = 0 ;i < ulLen ; i++)
                    {
                        
                        printf("%02x ", pBuff[i]);
                        
                        if ((i+1) % 16 == 0)
                        {
                            printf("\n");
                        }
                    }
                    printf("\n");
                }
                else
                {
                    printf("FTE_DB_dump error\n");
                }
                FTE_MEM_free(pBuff);
                
            }
        }
        break;
    }
    
    if (bPrintUsage || (nReturnCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<Commands>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<Commands>]\n", pArgv[0]);
        }
    }
    
    return  nReturnCode;
}

FTE_RET FTE_DB_HEAD_init
(
    FTE_DB_PTR  pFlash
)
{
    memset(&pFlash->xHead, 0, sizeof(pFlash->xHead));

    return  FTE_DB_HEAD_write(pFlash);
}

FTE_RET   FTE_DB_HEAD_read
(
    FTE_DB_PTR  pFlash
)
{
    FTE_RET     ulRet;
    FTE_UINT32  ulCRC;
    int         i;
    FTE_DB_HEAD   xHead = {.ulID = 0};
    
    for(i = 0 ; i < FTE_DB_PAGE_PER_SECTOR ; i++)
    {
        FTE_DB_HEAD   xTmpHead;
        
        ulRet = FTE_M25P16_read(pFlash->pM25P16, pFlash->ulBaseAddress + i * FTE_DB_PAGE_SIZE, (FTE_UINT8_PTR)&xTmpHead, sizeof(FTE_DB_HEAD));
        if (ulRet != MQX_OK)
        {
            continue;
        }

        ulCRC = FTE_CRC32(0, &xTmpHead.ulTag, sizeof(xTmpHead) - sizeof(FTE_UINT32));    
        if ((xTmpHead.ulTag != FTE_DB_TAG) || (xTmpHead.ulCRC != ulCRC))
        {
            continue;
        }
                
        if (xHead.ulID < xTmpHead.ulID)
        {
            memcpy(&xHead, &xTmpHead, sizeof(FTE_DB_HEAD));
        }        
    }
    
    if (xHead.ulID == 0)
    {
        return  MQX_ERROR;
    }
    
    memcpy(&pFlash->xHead, &xHead, sizeof(FTE_DB_HEAD));
    
    return  MQX_OK;
}

FTE_RET FTE_DB_HEAD_write
(
    FTE_DB_PTR  pFlash
)
{
    FTE_UINT32 ulPageIndex;

    pFlash->xHead.ulTag = FTE_DB_TAG;
    pFlash->xHead.ulID += 1;
    pFlash->xHead.ulCRC = FTE_CRC32(0, &pFlash->xHead.ulTag, sizeof(pFlash->xHead) - sizeof(FTE_UINT32));        
    
    ulPageIndex = (pFlash->xHead.ulID - 1) % FTE_DB_PAGE_PER_SECTOR;
    if (ulPageIndex == 0)
    {
        FTE_M25P16_eraseSector(pFlash->pM25P16, pFlash->ulBaseAddress);
    }    

    return  FTE_M25P16_write(pFlash->pM25P16, pFlash->ulBaseAddress + ulPageIndex * FTE_DB_PAGE_SIZE, (FTE_UINT8_PTR)&pFlash->xHead, sizeof(FTE_DB_HEAD));
}

FTE_RET FTE_DB_dump
(   
    FTE_UINT32      ulAddress, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulLen
)
{
    if (pFlash == NULL)
    {
        return  MQX_ERROR;
    }

    return  FTE_M25P16_read(pFlash->pM25P16, ulAddress, pBuff, ulLen);
}

FTE_RET FTE_DB_SECTOR_read
(
    FTE_DB_PTR  pFlash, 
    FTE_UINT32  ulIndex, 
    FTE_UINT32  ulOffset, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32  ulLen
)
{
    ASSERT((pFlash != NULL) && (ulIndex < FTE_DB_SECTOR_MAX) && ((ulOffset + ulLen) <= FTE_DB_SECTOR_SIZE) && (pBuff != NULL));
    
    return  FTE_M25P16_read(pFlash->pM25P16, ((ulIndex + 1) * FTE_DB_SECTOR_SIZE) + ulOffset, pBuff, ulLen); 
}

FTE_RET FTE_DB_SECTOR_write
(
    FTE_DB_PTR  pFlash, 
    FTE_UINT32  ulIndex, 
    FTE_UINT32  ulOffset, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32  ulLen
)
{
    ASSERT((pFlash != NULL) && (ulIndex < FTE_DB_SECTOR_MAX) && ((ulOffset + ulLen) <= FTE_DB_SECTOR_SIZE) && (pBuff != NULL));
    
    return  FTE_M25P16_write(pFlash->pM25P16, ((ulIndex + 1) * FTE_DB_SECTOR_SIZE) + ulOffset, pBuff, ulLen); 
}

FTE_RET FTE_DB_SECTOR_erase
(
    FTE_DB_PTR  pFlash, 
    FTE_UINT32  ulIndex
)
{
    ASSERT((pFlash != NULL) && (ulIndex < FTE_DB_SECTOR_MAX));
    
    return  FTE_M25P16_eraseSector(pFlash->pM25P16, ((ulIndex + 1) * FTE_DB_SECTOR_SIZE)); 
}

FTE_RET FTE_DB_SECTOR_getHead
(
    FTE_DB_PTR  pFlash, 
    FTE_UINT32  ulSectorIndex, 
    FTE_DB_SECTOR_HEAD_PTR  pSectorHead
)
{
    FTE_UINT32 ulOffset;
    
    ASSERT((pFlash != NULL) && (ulSectorIndex < FTE_DB_SECTOR_MAX) && (pSectorHead != NULL));
    
    ulOffset = (ulSectorIndex + 1) * FTE_DB_SECTOR_SIZE - FTE_DB_PAGE_SIZE;
    
    return  FTE_M25P16_read(pFlash->pM25P16, ulOffset, (FTE_UINT8_PTR)pSectorHead, sizeof(FTE_DB_SECTOR_HEAD)); 
}

FTE_RET FTE_DB_SECTOR_setHead
(
    FTE_DB_PTR  pFlash, 
    FTE_UINT32  ulSectorIndex, 
    FTE_DB_SECTOR_HEAD_PTR pSectorHead
)
{
    FTE_UINT32 ulOffset;
    
    ASSERT((pFlash != NULL) && (ulSectorIndex < FTE_DB_SECTOR_MAX) && (pSectorHead != NULL));
    
    ulOffset = (ulSectorIndex + 1) * FTE_DB_SECTOR_SIZE - FTE_DB_PAGE_SIZE;
    
    return  FTE_M25P16_write(pFlash->pM25P16, ulOffset, (FTE_UINT8_PTR)pSectorHead, sizeof(FTE_DB_SECTOR_HEAD)); 
}
