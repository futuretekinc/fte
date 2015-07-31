#include "fte_target.h"
#include "fte_crc.h"

#define FTE_DB_TAG              0x5894a3f1

#define FTE_DB_PAGE_SIZE        256
#define FTE_DB_PAGE_PER_SECTOR  256
#define FTE_DB_SECTOR_SIZE      (FTE_DB_PAGE_PER_SECTOR * FTE_DB_PAGE_SIZE)

#define FTE_DB_SECTOR_MAX       31
#define FTE_DB_BLOCK_MAX        30

typedef struct  FTE_DB_FLASH_SECTOR_HEAD_STRUCT
{
    FTE_OBJECT_ID   xID;
    uint_8          pUsedFlags[FTE_DB_PAGE_PER_SECTOR / 8];
}   FTE_DB_FLASH_SECTOR_HEAD, _PTR_ FTE_DB_FLASH_SECTOR_HEAD_PTR;

typedef struct  FTE_DB_FLASH_SECTOR_CONTEXT_STRUCT
{
    FTE_DB_FLASH_SECTOR_HEAD    xHead;
    uint_32                     ulCount;
}   FTE_DB_FLASH_SECTOR_CONTEXT, _PTR_ FTE_DB_FLASH_SECTOR_CONTEXT_PTR;

typedef struct  FTE_DB_FLASH_HEAD_STRUCT
{
    uint_32         ulCRC;
    uint_32         ulTag;
    uint_32         ulID;
    struct
    {
        FTE_OBJECT_ID   xID;
        uint_32         ulBlockOffset;
    }   pBlocks[FTE_DB_BLOCK_MAX];
}   FTE_DB_FLASH_HEAD, _PTR_ FTE_DB_FLASH_HEAD_PTR;

typedef struct  FTE_DB_FLASH_STRUCT
{
    uint_32                 ulBaseAddress;
    uint_32                 ulSectorCount;
    
    FTE_DB_FLASH_HEAD       xHead;
    FTE_DB_FLASH_SECTOR_HEAD pSectors[FTE_DB_BLOCK_MAX];
}   FTE_DB_FLASH, _PTR_ FTE_DB_FLASH_PTR;


static _mqx_uint _FTE_DB_FLASH_init(FTE_DB_FLASH_PTR pFlash, uint_32 ulAddress, uint_32 ulSize);
static _mqx_uint _FTE_DB_FLASH_format(FTE_DB_FLASH_PTR pFlash);

static _mqx_uint _FTE_DB_FLASH_readHead(FTE_DB_FLASH_PTR pFlash);
static _mqx_uint _FTE_DB_FLASH_writeHead(FTE_DB_FLASH_PTR pFlash);
static _mqx_uint _FTE_DB_FLASH_eraseSector(uint_32 ulAddress);
static _mqx_uint _FTE_DB_FLASH_read(uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen);
static _mqx_uint _FTE_DB_FLASH_write(uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen);

FTE_SPI_PTR         pSPI = NULL;
FTE_DB_FLASH_PTR    pFlash = NULL;

_mqx_uint   FTE_DB_init(void)
{
    pSPI = FTE_SPI_get(FTE_DEV_SPI_2_0);
    if (pSPI == NULL)
    {
        goto error;
    }

    if (FTE_SPI_attach(pSPI, 0) != MQX_OK)
    {
        goto error;
    }

    pFlash = (FTE_DB_FLASH_PTR)FTE_MEM_allocZero(sizeof(FTE_DB_FLASH));
    if (pFlash == NULL)
    {
        return  MQX_ERROR;
    }
    
    _FTE_DB_FLASH_init(pFlash, 0, FTE_DB_BLOCK_MAX * FTE_DB_PAGE_PER_SECTOR * FTE_DB_PAGE_SIZE);
    
    return  MQX_OK;
error:

    if (pSPI != NULL)
    {
        pSPI = NULL;
    }
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_DB_OBJ_create(FTE_OBJECT_ID xID)
{
    int i;
    
    for(i = 0 ; i < FTE_DB_BLOCK_MAX ; i++)
    {
        if (pFlash->xHead.pBlocks[i].xID == xID)
        {
            _FTE_DB_FLASH_read(pFlash->xHead.pBlocks[i].ulBlockOffset, (uint_8_ptr)&pFlash->pSectors[i], sizeof(FTE_DB_FLASH_SECTOR_HEAD));

            return  MQX_OK;
        }
    }
    
    for(i = 0 ; i < FTE_DB_BLOCK_MAX ; i++)
    {
        if (pFlash->xHead.pBlocks[i].xID == 0)
        {
            pFlash->xHead.pBlocks[i].xID = xID;
            pFlash->xHead.pBlocks[i].ulBlockOffset = pFlash->ulBaseAddress + (i + 1) * FTE_DB_SECTOR_SIZE;
            
            _FTE_DB_FLASH_eraseSector(pFlash->xHead.pBlocks[i].ulBlockOffset);
            memset(&pFlash->pSectors[i], 0xFF, sizeof(FTE_DB_FLASH_SECTOR_HEAD));
            
            _FTE_DB_FLASH_writeHead(pFlash);
            
            return  MQX_OK;
        }
    }
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_DB_OBJ_destroy(FTE_OBJECT_ID xID)
{
     int i;
    
    for(i = 0 ; i < FTE_DB_BLOCK_MAX ; i++)
    {
        if (pFlash->xHead.pBlocks[i].xID == xID)
        {
            memset(&pFlash->xHead.pBlocks[i], 0, sizeof(pFlash->xHead.pBlocks[i]));

            return  MQX_OK;
        }
    }
    
   return  MQX_OK;
}

uint_32     FTE_DB_OBJ_count(FTE_OBJECT_ID xID)
{
    return  0;
}

int_32      FTE_DB_SHELL_cmd(int_32 nArgc, char_ptr pArgv[])
{
    boolean bPrintUsage, bShortHelp = FALSE;
    int_32  nReturnCode = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    switch(nArgc)
    {
    case    1:
        {
            int i, nCount;
            if (pFlash == NULL)
            {
                printf("DB not initiallized\n");
                break;
            }
            
            printf("%8s : %08x\n",  "Base", pFlash->ulBaseAddress);
            printf("%8s : %d\n",    "ID", pFlash->xHead.ulID);
            printf("%8s : \n",         "Blocks");
            nCount = 0;
            for(i = 0; i < FTE_DB_BLOCK_MAX; i++)
            {
                if (pFlash->xHead.pBlocks[i].xID != 0)
                {
                    printf("    %4d %08x %08x\n", ++nCount, pFlash->xHead.pBlocks[i].xID, pFlash->xHead.pBlocks[i].ulBlockOffset);
                }
            }
        }
        break;
        
    case    2:
        {
            if (strcmp(pArgv[1], "init") == 0)
            {
                FTE_DB_init();
            }
        }
        break;
        
    case    3:
        {
            if (strcmp(pArgv[1], "create") == 0)
            {
                uint_32 xID;
                
                if (Shell_parse_hexnum(pArgv[2], &xID) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                FTE_DB_OBJ_create(xID);
            }
            else if (strcmp(pArgv[1], "destroy") == 0)
            {
                uint_32 xID;
                
                if (Shell_parse_hexnum(pArgv[2], &xID) == FALSE)
                {
                    bPrintUsage = TRUE;
                    break;
                }
                
                FTE_DB_OBJ_destroy(xID);
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

_mqx_uint _FTE_DB_FLASH_init(FTE_DB_FLASH_PTR pFlash, uint_32 ulAddress, uint_32 ulSize)
{
    _mqx_uint   ulRet;
    uint_8      pCmd[1] = { 0x9f};
    uint_8      pBuff[20];
    int         i;
    
    if (pSPI == NULL)
    {
        return  MQX_ERROR;
    }
    
    ulRet = FTE_SPI_read(pSPI, pCmd, 1, pBuff, 20);
    if (ulRet != MQX_OK)
    {
        return  MQX_ERROR;
    }

    pFlash->ulBaseAddress = ulAddress;
    pFlash->ulSectorCount = ulSize / (FTE_DB_PAGE_PER_SECTOR * FTE_DB_PAGE_SIZE);

    ulRet = _FTE_DB_FLASH_readHead(pFlash);
    if (ulRet != MQX_OK)
    {
        _FTE_DB_FLASH_format(pFlash);
    }
    else
    {
        for(i = 0 ; i < FTE_DB_BLOCK_MAX ; i++)
        {
            FTE_DB_FLASH_SECTOR_HEAD    xHead;
            
            ulRet = _FTE_DB_FLASH_read((i+1) * 32 * 1024, (uint_8_ptr)&xHead, sizeof(FTE_DB_FLASH_SECTOR_HEAD)); 
            if (ulRet == MQX_OK)
            {
            }
        }
    }
    
    return  MQX_OK;
}

_mqx_uint _FTE_DB_FLASH_format(FTE_DB_FLASH_PTR pFlash)
{
    memset(&pFlash->xHead, 0, sizeof(pFlash->xHead));
    
    return  _FTE_DB_FLASH_writeHead(pFlash);
}

_mqx_uint   _FTE_DB_FLASH_readHead(FTE_DB_FLASH_PTR pFlash)
{
    _mqx_uint           ulRet;
    uint_32             ulCRC;
    int                 i;
    FTE_DB_FLASH_HEAD   xHead = {.ulID = 0};
    
    for(i = 0 ; i < FTE_DB_PAGE_PER_SECTOR ; i++)
    {
        FTE_DB_FLASH_HEAD   xTmpHead;
        
        ulRet = _FTE_DB_FLASH_read(pFlash->ulBaseAddress + i * FTE_DB_PAGE_SIZE, (uint_8_ptr)&xTmpHead, sizeof(FTE_DB_FLASH_HEAD));
        if (ulRet != MQX_OK)
        {
            continue;
        }

        ulCRC = fte_crc32(0, (void *)&xTmpHead.ulTag, sizeof(xTmpHead) - sizeof(uint_32));    
        if ((xTmpHead.ulTag != FTE_DB_TAG) || (xTmpHead.ulCRC != ulCRC))
        {
            continue;
        }
                
        if (xHead.ulID < xTmpHead.ulID)
        {
            memcpy(&xHead, &xTmpHead, sizeof(FTE_DB_FLASH_HEAD));
        }        
    }
    
    if (xHead.ulID == 0)
    {
        return  MQX_ERROR;
    }
    
    memcpy(&pFlash->xHead, &xHead, sizeof(FTE_DB_FLASH_HEAD));
    
    return  MQX_OK;
}

_mqx_uint   _FTE_DB_FLASH_writeHead(FTE_DB_FLASH_PTR pFlash)
{
    uint_32 ulPageIndex;

    pFlash->xHead.ulTag = FTE_DB_TAG;
    pFlash->xHead.ulID += 1;
    pFlash->xHead.ulCRC = fte_crc32(0, (void *)&pFlash->xHead.ulTag, sizeof(pFlash->xHead) - sizeof(uint_32));        
    
    ulPageIndex = (pFlash->xHead.ulID - 1) % FTE_DB_PAGE_PER_SECTOR;
    if (ulPageIndex == 0)
    {
        _FTE_DB_FLASH_eraseSector(pFlash->ulBaseAddress);
    }    

    return  _FTE_DB_FLASH_write(pFlash->ulBaseAddress + ulPageIndex * FTE_DB_PAGE_SIZE, (uint_8_ptr)&pFlash->xHead, sizeof(FTE_DB_FLASH_HEAD));
}

_mqx_uint   _FTE_DB_FLASH_eraseSector(uint_32 ulAddress)
{
    _mqx_uint   ulRet;
    uint_8      pWriteEnCmd[1] = {0x06};
    uint_8      pWriteDnCmd[1] = {0x04};
    uint_8      pReadStatusCmd[1] = {0x05};
    uint_8      pEraseSectorCmd[4];
    uint_8      pStatus[1];
    int         nCount;
    
    ulRet = FTE_SPI_write(pSPI, pWriteEnCmd, sizeof(pWriteEnCmd), NULL, 0);
    if (ulRet != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pEraseSectorCmd[0] = 0xD8;
    pEraseSectorCmd[1] = (ulAddress        & 0xFF);
    pEraseSectorCmd[2] = ((ulAddress >> 8) & 0xFF);
    pEraseSectorCmd[3] = ((ulAddress >> 16)& 0xFF);
  
    ulRet = FTE_SPI_write(pSPI, pEraseSectorCmd, sizeof(pEraseSectorCmd), NULL, 0);
    if (ulRet != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    for(nCount = 0 ; nCount < 10 ; nCount++)
    {
        ulRet = FTE_SPI_read(pSPI, pReadStatusCmd, sizeof(pReadStatusCmd), pStatus, sizeof(pStatus));
        if (ulRet != MQX_OK)
        {
            return  MQX_ERROR;
        }
    
        if ((pStatus[0] & 0x01) == 0)
        {
            break;
        }
        
        _time_delay(100);        
    }
    
    FTE_SPI_write(pSPI, pWriteDnCmd, sizeof(pWriteDnCmd), NULL, 0);
    
    if (nCount == 10)
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

_mqx_uint   _FTE_DB_FLASH_read(uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen)
{
    uint_8  pCmd[4];
    
    pCmd[0] = 0x03;
    pCmd[1] = (ulAddress        & 0xFF);
    pCmd[2] = ((ulAddress >> 8) & 0xFF);
    pCmd[3] = ((ulAddress >> 16)& 0xFF);
    
    return  FTE_SPI_read(pSPI, pCmd, sizeof(pCmd), pBuff, ulLen);
}

_mqx_uint   _FTE_DB_FLASH_write(uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen)
{
    _mqx_uint   ulRet;
    uint_8      pWriteEnCmd[1] = {0x06};
    uint_8      pWriteDnCmd[1] = {0x04};
    uint_8      pReadStatusCmd[1] = {0x05};
    uint_8      pEraseSectorCmd[4];
    uint_8      pStatus[1];
    int         nCount;
    
    ulRet = FTE_SPI_write(pSPI, pWriteEnCmd, sizeof(pWriteEnCmd), NULL, 0);
    if (ulRet != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pEraseSectorCmd[0] = 0x02;
    pEraseSectorCmd[1] = (ulAddress        & 0xFF);
    pEraseSectorCmd[2] = ((ulAddress >> 8) & 0xFF);
    pEraseSectorCmd[3] = ((ulAddress >> 16)& 0xFF);
  
    ulRet = FTE_SPI_write(pSPI, pEraseSectorCmd, sizeof(pEraseSectorCmd), pBuff, ulLen);
    if (ulRet != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    for(nCount = 0 ; nCount < 10 ; nCount++)
    {
        ulRet = FTE_SPI_read(pSPI, pReadStatusCmd, sizeof(pReadStatusCmd), pStatus, sizeof(pStatus));
        if (ulRet != MQX_OK)
        {
            return  MQX_ERROR;
        }
    
        if ((pStatus[0] & 0x01) == 0)
        {
            break;
        }
        
        _time_delay(1);        
    }
    
    FTE_SPI_write(pSPI, pWriteDnCmd, sizeof(pWriteDnCmd), NULL, 0);
    
    if (nCount == 10)
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

