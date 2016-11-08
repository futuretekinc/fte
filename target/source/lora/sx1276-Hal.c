/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include <stdint.h>
#include <stdbool.h> 

#include "fte_target.h"
#include "fte_common.h"
#include "fte_spi.h"
#include "sx1276-Hal.h"

static FTE_SPI_PTR pSPI = NULL;
static FTE_LWGPIO_PTR pGPIO[6] = {NULL,};
static FTE_LWGPIO_PTR pCTX = NULL;

void SX1276InitIo( void )
{    
    pSPI = FTE_SPI_get(FTE_DEV_SPI_0_0);
    
    if (pSPI != NULL)
    {
        FTE_SPI_attach(pSPI, 0);
    }
    
    pGPIO[0] = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO0);
    if (pGPIO[0] != NULL)
    {
        FTE_LWGPIO_attach(pGPIO[0], 0);
    }
    
    pGPIO[1] = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO1);
    if (pGPIO[1] != NULL)
    {
        FTE_LWGPIO_attach(pGPIO[1], 0);
    }
    
    pGPIO[2] = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO2);
    if (pGPIO[2] != NULL)
    {
        FTE_LWGPIO_attach(pGPIO[2], 0);
    }
    
    pGPIO[3] = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO3);
    if (pGPIO[3] != NULL)
    {
        FTE_LWGPIO_attach(pGPIO[3], 0);
    }
    
    pGPIO[4] = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO4);
    if (pGPIO[4] != NULL)
    {
        FTE_LWGPIO_attach(pGPIO[4], 0);
    }
    
    pGPIO[5] = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO5);
    if (pGPIO[5] != NULL)
    {
        FTE_LWGPIO_attach(pGPIO[5], 0);
    }
    
    pCTX = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_CTX);
    if (pCTX  != NULL)
    {
        FTE_LWGPIO_attach(pCTX, 0);
    }
    
    
}

void SX1276SetReset( uint8_t state )
{
}

void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

void SX1276Read( uint8_t addr, uint8_t *data )
{ 
    SX1276ReadBuffer( addr, data, 1 );
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t nCmd = (addr | 0x80);
    
    FTE_SPI_write(pSPI, &nCmd, 1, buffer, size);
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t nCmd = (addr & 0x7F);
    
    FTE_SPI_read(pSPI, &nCmd, 1, buffer, size);
}

void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( 0, buffer, size );
}

void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( 0, buffer, size );
}

uint8_t SX1276ReadDio0( void )
{
    FTE_BOOL bValue = 0;
    
    FTE_LWGPIO_getValue(pGPIO[0], &bValue);

    return bValue;
}

 uint8_t SX1276ReadDio1( void )
{
    FTE_BOOL bValue = 0;
    
    FTE_LWGPIO_getValue(pGPIO[1], &bValue);

    return bValue;
}

uint8_t SX1276ReadDio2( void )
{
    FTE_BOOL bValue = 0;
    
    FTE_LWGPIO_getValue(pGPIO[2], &bValue);

    return bValue;
}

uint8_t SX1276ReadDio3( void )
{
    FTE_BOOL bValue = 0;
    
    FTE_LWGPIO_getValue(pGPIO[3], &bValue);

    return bValue;
}

uint8_t SX1276ReadDio4( void )
{
    FTE_BOOL bValue = 0;
    
    FTE_LWGPIO_getValue(pGPIO[4], &bValue);

    return bValue;
}

uint8_t SX1276ReadDio5( void )
{
    FTE_BOOL bValue = 0;
    
    FTE_LWGPIO_getValue(pGPIO[5], &bValue);

    return bValue;
}

void SX1276WriteRxTx( uint8_t txEnable )
{
    if( txEnable != 0 )
    {
        FTE_LWGPIO_setValue(pCTX, TRUE);
    }
    else
    {
        FTE_LWGPIO_setValue(pCTX, FALSE);
    }
}

uint32_t SX1276GetTickCount( void ) 
{
    return  _time_get_hwticks();
}