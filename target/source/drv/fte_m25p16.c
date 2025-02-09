#include "fte_target.h"
#include "fte_spi.h"
#include "fte_m25p16.h"

#define FTE_M25P16_CMD_WRITE_STATUS     0x01
#define FTE_M25P16_CMD_WRITE_DISABLE    0x04
#define FTE_M25P16_CMD_READ_STATUS      0x05
#define FTE_M25P16_CMD_WRITE_ENABLE     0x06
#define FTE_M25P16_CMD_SECTOR_ERASE     0xD8
#define FTE_M25P16_READ_IDENT           0x9F

static FTE_M25P16_PTR _pHead = NULL;

FTE_M25P16_PTR FTE_M25P16_get(FTE_UINT32 nID)
{
    FTE_M25P16_PTR    pM25P16;
    
    pM25P16 = _pHead;
    while(pM25P16 != NULL)
    {
        if (pM25P16->pConfig->nID == nID)
        {
            return  pM25P16;
        }
        
        pM25P16 = pM25P16->pNext;
    }
    
    return  NULL;
}

FTE_M25P16_PTR FTE_M25P16_get_first(void)
{
    return  _pHead;
}

FTE_M25P16_PTR FTE_M25P16_getNext
(
    FTE_M25P16_PTR  pObj
)
{
    return  pObj->pNext;
}

FTE_RET   FTE_M25P16_create
(
    FTE_M25P16_CONFIG_PTR   pConfig
)
{
    FTE_M25P16_PTR    pM25P16;
    
    pM25P16 = (FTE_M25P16_PTR)FTE_MEM_allocZero(sizeof(FTE_M25P16));
    if (pM25P16 == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }

    pM25P16->pNext    = _pHead;
    pM25P16->pConfig  = pConfig;
        
    _pHead = pM25P16;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_M25P16_attach
(
    FTE_M25P16_PTR  pM25P16, 
    FTE_UINT32      nParent
)
{
    FTE_SPI_PTR     pSPI;
    FTE_LWGPIO_PTR  pLWGPIO_Hold, pLWGPIO_WP;
        
    ASSERT(pM25P16 != NULL);
    
    pSPI = FTE_SPI_get(pM25P16->pConfig->xSPI);
    if (pSPI == NULL)
    {
        goto error1;
    }

    pLWGPIO_Hold = FTE_LWGPIO_get(pM25P16->pConfig->xGPIOHold);
    if (pLWGPIO_Hold == NULL)
    {
        goto error1;
    }
    
    pLWGPIO_WP = FTE_LWGPIO_get(pM25P16->pConfig->xGPIOWP);
    if (pLWGPIO_WP == NULL)
    {
        goto error1;
    }
        
    if (FTE_LWGPIO_attach(pLWGPIO_Hold, pM25P16->pConfig->nID) != FTE_RET_OK)
    {
        goto error2;
    }

    if (FTE_LWGPIO_attach(pLWGPIO_WP, pM25P16->pConfig->nID) != FTE_RET_OK)
    {
        goto error2;
    }
    
    if (FTE_SPI_attach(pSPI, pM25P16->pConfig->nID) != FTE_RET_OK)
    {
        goto error2;
    }

    FTE_LWGPIO_setValue(pLWGPIO_WP, FALSE);
    FTE_LWGPIO_setValue(pLWGPIO_Hold, FALSE);
    
    pM25P16->pSPI   = pSPI;
    pM25P16->pHold  = pLWGPIO_Hold;
    pM25P16->pWP    = pLWGPIO_WP;        
        
    return  FTE_RET_OK;
    
error2:
    
    if (pSPI != NULL)
    {
        FTE_SPI_detach(pSPI);
    }
    
    if (pLWGPIO_Hold != NULL)
    {
        FTE_LWGPIO_detach(pLWGPIO_Hold);
    }
    
    if (pLWGPIO_WP != NULL)
    {
        FTE_LWGPIO_detach(pLWGPIO_WP);
    }

error1:
    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_M25P16_detach
(
    FTE_M25P16_PTR  pM25P16, 
    FTE_UINT32      nParentID
)
{
    ASSERT(pM25P16 != NULL);
    
    FTE_SPI_detach(pM25P16->pSPI);
    pM25P16->pSPI = NULL;
    FTE_LWGPIO_detach(pM25P16->pHold);
    pM25P16->pHold = NULL;
    FTE_LWGPIO_detach(pM25P16->pWP);
    pM25P16->pWP = NULL;

    return  FTE_RET_OK;
}

FTE_RET   FTE_M25P16_eraseSector
(
    FTE_M25P16_PTR  pM25P16, 
    FTE_UINT32      ulAddress
)
{
    FTE_RET   ulRet;
    FTE_UINT8      pEraseSectorCmd[4];
    FTE_UINT32     ulStatus;
    int         nCount;
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    if (FTE_M25P16_writeEnable(pM25P16, TRUE) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    pEraseSectorCmd[0] = FTE_M25P16_CMD_SECTOR_ERASE;
    pEraseSectorCmd[1] = ((ulAddress >> 16)& 0xFF);
    pEraseSectorCmd[2] = ((ulAddress >>  8) & 0xFF);
    pEraseSectorCmd[3] = ((ulAddress      )& 0xFF);
  
    ulRet = FTE_SPI_write(pM25P16->pSPI, pEraseSectorCmd, sizeof(pEraseSectorCmd), NULL, 0);
    if (ulRet != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    for(nCount = 0 ; nCount < 10 ; nCount++)
    {
        
        if (FTE_M25P16_readStatus(pM25P16, &ulStatus) != FTE_RET_OK)
        {
            return  FTE_RET_ERROR;
        }
    
        if ((ulStatus & 0x01) == 0)
        {
            break;
        }
        
        _time_delay(100);        
    }
    
    if (FTE_M25P16_writeEnable(pM25P16, FALSE) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    
    if (nCount == 10)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_BOOL FTE_M25P16_isExist
(
    FTE_M25P16_PTR  pM25P16
)
{
    FTE_UINT8      pCmd[1] = { FTE_M25P16_READ_IDENT };
    FTE_UINT8      pBuff[20];
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));

    if (FTE_SPI_read(pM25P16->pSPI, pCmd, sizeof(pCmd), pBuff, sizeof(pBuff)) != FTE_RET_OK)
    {
        return  FALSE;
    }
    
    if ((pBuff[0] != 0x20) || (pBuff[1] != 0x20) || (pBuff[2] != 0x15))
    {
        return  FALSE;
    }
    
    return  TRUE;
}

FTE_RET   FTE_M25P16_read
(
    FTE_M25P16_PTR  pM25P16, 
    FTE_UINT32      ulAddress, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulLen
)
{
    FTE_UINT8  pCmd[4];
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    pCmd[0] = 0x03;
    pCmd[1] = ((ulAddress >> 16)& 0xFF);
    pCmd[2] = ((ulAddress >> 8) & 0xFF);
    pCmd[3] = (ulAddress        & 0xFF);
   
    return  FTE_SPI_read(pM25P16->pSPI, pCmd, sizeof(pCmd), pBuff, ulLen);
}

FTE_RET   FTE_M25P16_write
(
    FTE_M25P16_PTR  pM25P16, 
    FTE_UINT32      ulAddress, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulLen
)
{
    FTE_RET   ulRet;
    FTE_UINT8      pReadStatusCmd[1] = {FTE_M25P16_CMD_READ_STATUS};
    FTE_UINT8      pEraseSectorCmd[4];
    FTE_UINT8      pStatus[1];
    int         nCount;
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    if (FTE_M25P16_writeEnable(pM25P16, TRUE) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    pEraseSectorCmd[0] = 0x02;
    pEraseSectorCmd[1] = ((ulAddress >> 16)& 0xFF);
    pEraseSectorCmd[2] = ((ulAddress >>  8)& 0xFF);
    pEraseSectorCmd[3] = ((ulAddress      )& 0xFF);
  
    ulRet = FTE_SPI_write(pM25P16->pSPI, pEraseSectorCmd, sizeof(pEraseSectorCmd), pBuff, ulLen);
    if (ulRet != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    for(nCount = 0 ; nCount < 10 ; nCount++)
    {
        ulRet = FTE_SPI_read(pM25P16->pSPI, pReadStatusCmd, sizeof(pReadStatusCmd), pStatus, sizeof(pStatus));
        if (ulRet != FTE_RET_OK)
        {
            return  FTE_RET_ERROR;
        }
    
        if ((pStatus[0] & 0x01) == 0)
        {
            break;
        }
        
        _time_delay(1);        
    }
    
    if (FTE_M25P16_writeEnable(pM25P16, FALSE) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    if (nCount == 10)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET       FTE_M25P16_writeEnable
(
    FTE_M25P16_PTR  pM25P16, 
    FTE_BOOL        bON
)
{
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    if (bON)
    {
        FTE_UINT8      pCmd[1] = {FTE_M25P16_CMD_WRITE_ENABLE};
        
        if (FTE_SPI_write(pM25P16->pSPI, pCmd, sizeof(pCmd), NULL, 0) != FTE_RET_OK)
        {
            return  FTE_RET_ERROR;
        }
        
        pM25P16->bWriteEnable = TRUE;
    }
    else
    {
        FTE_UINT8      pCmd[1] = {FTE_M25P16_CMD_WRITE_DISABLE};
        
        if (FTE_SPI_write(pM25P16->pSPI, pCmd, sizeof(pCmd), NULL, 0) != FTE_RET_OK)
        {
            return  FTE_RET_ERROR;
        }

        pM25P16->bWriteEnable = FALSE;
    }
    
    return  FTE_RET_OK;
}

FTE_BOOL    FTE_M25P16_isWriteEnabled
(
    FTE_M25P16_PTR  pM25P16
)
{
    return  pM25P16->bWriteEnable;
}

FTE_RET FTE_M25P16_readStatus
(
    FTE_M25P16_PTR  pM25P16, 
    FTE_UINT32_PTR  pulStatus
)
{
    FTE_UINT8      pBuff[1];
    FTE_UINT8      pReadStatusCmd[1] = {FTE_M25P16_CMD_READ_STATUS};

    if (FTE_SPI_read(pM25P16->pSPI, pReadStatusCmd, sizeof(pReadStatusCmd), pBuff, sizeof(pBuff)) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    *pulStatus = pBuff[0];
    
    return  FTE_RET_OK;
}
