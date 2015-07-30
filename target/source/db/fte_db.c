#include "fte_target.h"

static _mqx_uint _FTE_DB_FLASH_init(void);

FTE_SPI_PTR pSPI = NULL;

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
    
    _FTE_DB_FLASH_init();
    
    return  MQX_OK;
error:

    if (pSPI != NULL)
    {
        pSPI = NULL;
    }
    
    return  MQX_ERROR;
}

_mqx_uint _FTE_DB_FLASH_init(void)
{
    uint_8  pCmd[1] = { 0x9f};
    uint_8  pBuff[20];
    
    if (pSPI == NULL)
    {
        return  MQX_ERROR;
    }
    
    FTE_SPI_read(pSPI, pCmd, 1, pBuff, 20);
    
    return  MQX_OK;
}

