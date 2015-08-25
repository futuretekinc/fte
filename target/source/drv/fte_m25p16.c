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

FTE_M25P16_PTR FTE_M25P16_get(uint_32 nID)
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

FTE_M25P16_PTR FTE_M25P16_getNext(FTE_M25P16_PTR pObj)
{
    return  pObj->pNext;
}
_mqx_uint   FTE_M25P16_create(FTE_M25P16_CONFIG_PTR pConfig)
{
    FTE_M25P16_PTR    pM25P16;
    
    pM25P16 = (FTE_M25P16_PTR)FTE_MEM_allocZero(sizeof(FTE_M25P16));
    if (pM25P16 == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }

    pM25P16->pNext    = _pHead;
    pM25P16->pConfig  = pConfig;
        
    _pHead = pM25P16;
    
    return  MQX_OK;
}

_mqx_uint   FTE_M25P16_attach(FTE_M25P16_PTR pM25P16, uint_32 nParent)
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
        
    if (FTE_LWGPIO_attach(pLWGPIO_Hold, pM25P16->pConfig->nID) != MQX_OK)
    {
        goto error2;
    }

    if (FTE_LWGPIO_attach(pLWGPIO_WP, pM25P16->pConfig->nID) != MQX_OK)
    {
        goto error2;
    }
    
    if (FTE_SPI_attach(pSPI, pM25P16->pConfig->nID) != MQX_OK)
    {
        goto error2;
    }

    FTE_LWGPIO_setValue(pLWGPIO_WP, FALSE);
    FTE_LWGPIO_setValue(pLWGPIO_Hold, FALSE);
    
    pM25P16->pSPI   = pSPI;
    pM25P16->pHold  = pLWGPIO_Hold;
    pM25P16->pWP    = pLWGPIO_WP;        
        
    return  MQX_OK;
    
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
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_M25P16_detach(FTE_M25P16_PTR pM25P16, uint_32 nParentID)
{
    ASSERT(pM25P16 != NULL);
    
    FTE_SPI_detach(pM25P16->pSPI);
    pM25P16->pSPI = NULL;
    FTE_LWGPIO_detach(pM25P16->pHold);
    pM25P16->pHold = NULL;
    FTE_LWGPIO_detach(pM25P16->pWP);
    pM25P16->pWP = NULL;

    return  MQX_OK;
}

_mqx_uint   FTE_M25P16_eraseSector(FTE_M25P16_PTR pM25P16, uint_32 ulAddress)
{
    _mqx_uint   ulRet;
    uint_8      pEraseSectorCmd[4];
    uint_32     ulStatus;
    int         nCount;
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    if (FTE_M25P16_writeEnable(pM25P16, TRUE) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pEraseSectorCmd[0] = FTE_M25P16_CMD_SECTOR_ERASE;
    pEraseSectorCmd[1] = ((ulAddress >> 16)& 0xFF);
    pEraseSectorCmd[2] = ((ulAddress >>  8) & 0xFF);
    pEraseSectorCmd[3] = ((ulAddress      )& 0xFF);
  
    ulRet = FTE_SPI_write(pM25P16->pSPI, pEraseSectorCmd, sizeof(pEraseSectorCmd), NULL, 0);
    if (ulRet != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    for(nCount = 0 ; nCount < 10 ; nCount++)
    {
        
        if (FTE_M25P16_readStatus(pM25P16, &ulStatus) != MQX_OK)
        {
            return  MQX_ERROR;
        }
    
        if ((ulStatus & 0x01) == 0)
        {
            break;
        }
        
        _time_delay(100);        
    }
    
    if (FTE_M25P16_writeEnable(pM25P16, FALSE) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    
    if (nCount == 10)
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

boolean FTE_M25P16_isExist(FTE_M25P16_PTR pM25P16)
{
    uint_8      pCmd[1] = { FTE_M25P16_READ_IDENT };
    uint_8      pBuff[20];
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));

    if (FTE_SPI_read(pM25P16->pSPI, pCmd, sizeof(pCmd), pBuff, sizeof(pBuff)) != MQX_OK)
    {
        return  FALSE;
    }
    
    if ((pBuff[0] != 0x20) || (pBuff[1] != 0x20) || (pBuff[2] != 0x15))
    {
        return  FALSE;
    }
    
    return  TRUE;
}

_mqx_uint   FTE_M25P16_read(FTE_M25P16_PTR pM25P16, uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen)
{
    uint_8  pCmd[4];
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    pCmd[0] = 0x03;
    pCmd[1] = ((ulAddress >> 16)& 0xFF);
    pCmd[2] = ((ulAddress >> 8) & 0xFF);
    pCmd[3] = (ulAddress        & 0xFF);
   
    return  FTE_SPI_read(pM25P16->pSPI, pCmd, sizeof(pCmd), pBuff, ulLen);
}

_mqx_uint   FTE_M25P16_write(FTE_M25P16_PTR pM25P16, uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen)
{
    _mqx_uint   ulRet;
    uint_8      pReadStatusCmd[1] = {FTE_M25P16_CMD_READ_STATUS};
    uint_8      pEraseSectorCmd[4];
    uint_8      pStatus[1];
    int         nCount;
    
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    if (FTE_M25P16_writeEnable(pM25P16, TRUE) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pEraseSectorCmd[0] = 0x02;
    pEraseSectorCmd[1] = ((ulAddress >> 16)& 0xFF);
    pEraseSectorCmd[2] = ((ulAddress >>  8)& 0xFF);
    pEraseSectorCmd[3] = ((ulAddress      )& 0xFF);
  
    ulRet = FTE_SPI_write(pM25P16->pSPI, pEraseSectorCmd, sizeof(pEraseSectorCmd), pBuff, ulLen);
    if (ulRet != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    for(nCount = 0 ; nCount < 10 ; nCount++)
    {
        ulRet = FTE_SPI_read(pM25P16->pSPI, pReadStatusCmd, sizeof(pReadStatusCmd), pStatus, sizeof(pStatus));
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
    
    if (FTE_M25P16_writeEnable(pM25P16, FALSE) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    if (nCount == 10)
    {
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

_mqx_uint       FTE_M25P16_writeEnable(FTE_M25P16_PTR pM25P16, boolean bON)
{
    ASSERT((pM25P16 != NULL) && (pM25P16->pSPI != NULL));
    
    if (bON)
    {
        uint_8      pCmd[1] = {FTE_M25P16_CMD_WRITE_ENABLE};
        
        if (FTE_SPI_write(pM25P16->pSPI, pCmd, sizeof(pCmd), NULL, 0) != MQX_OK)
        {
            return  MQX_ERROR;
        }
        
        pM25P16->bWriteEnable = TRUE;
    }
    else
    {
        uint_8      pCmd[1] = {FTE_M25P16_CMD_WRITE_DISABLE};
        
        if (FTE_SPI_write(pM25P16->pSPI, pCmd, sizeof(pCmd), NULL, 0) != MQX_OK)
        {
            return  MQX_ERROR;
        }

        pM25P16->bWriteEnable = FALSE;
    }
    
    return  MQX_OK;
}

boolean         fte_M25P16_isWriteEnabled(FTE_M25P16_PTR pM25P16)
{
    return  pM25P16->bWriteEnable;
}

_mqx_uint       FTE_M25P16_readStatus(FTE_M25P16_PTR pM25P16, uint_32_ptr pulStatus)
{
    uint_8      pBuff[1];
    uint_8      pReadStatusCmd[1] = {FTE_M25P16_CMD_READ_STATUS};

    if (FTE_SPI_read(pM25P16->pSPI, pReadStatusCmd, sizeof(pReadStatusCmd), pBuff, sizeof(pBuff)) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    *pulStatus = pBuff[0];
    
    return  MQX_OK;
}
