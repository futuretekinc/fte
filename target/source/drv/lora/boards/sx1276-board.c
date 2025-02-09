/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX1276 driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "sx1276-board.h"

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    SX1276Init,
    SX1276GetStatus,
    SX1276SetModem,
    SX1276SetChannel,
    SX1276IsChannelFree,
    SX1276Random,
    SX1276SetRxConfig,
    SX1276SetTxConfig,
    SX1276CheckRfFrequency,
    SX1276GetTimeOnAir,
    SX1276Send,
    SX1276SetSleep,
    SX1276SetStby,
    SX1276SetRx,
    SX1276StartCad,
    SX1276ReadRssi,
    SX1276Write,
    SX1276Read,
    SX1276WriteBuffer,
    SX1276ReadBuffer
};

/*!
 * Antenna switch GPIO pins objects
 */
FTE_LWGPIO_PTR pAntSwitchLf = 0;
FTE_LWGPIO_PTR pAntSwitchHf = 0;

void SX1276IoInit( void )
{
    SX1276.pSPI = FTE_SPI_get(FTE_DEV_SPI_0_0);
    
    if (SX1276.pSPI != NULL)
    {
        FTE_SPI_attach(SX1276.pSPI, 0);
    }
    
    SX1276.pDIO0 = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO0);
    if (SX1276.pDIO0 != NULL)
    {
        FTE_LWGPIO_attach(SX1276.pDIO0, 0);
    }
    
    SX1276.pDIO1 = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO1);
    if (SX1276.pDIO1 != NULL)
    {
        FTE_LWGPIO_attach(SX1276.pDIO1, 0);
    }
    
    SX1276.pDIO2 = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO2);
    if (SX1276.pDIO2 != NULL)
    {
        FTE_LWGPIO_attach(SX1276.pDIO2, 0);
    }
    
    SX1276.pDIO3 = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO3);
    if (SX1276.pDIO3 != NULL)
    {
        FTE_LWGPIO_attach(SX1276.pDIO3, 0);
    }
    
    SX1276.pDIO4 = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO4);
    if (SX1276.pDIO4 != NULL)
    {
        FTE_LWGPIO_attach(SX1276.pDIO4, 0);
    }
    
    SX1276.pDIO5 = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_DIO5);
    if (SX1276.pDIO5 != NULL)
    {
        FTE_LWGPIO_attach(SX1276.pDIO5, 0);
    }

    pAntSwitchHf = FTE_LWGPIO_get(FTE_DEV_LWGPIO_LORA_CTX);
    if (pAntSwitchHf  != NULL)
    {
        FTE_LWGPIO_attach(pAntSwitchHf, 0);
    }
}

void SX1276IoIrqInit( DioIrqHandler **irqHandlers )
{
}

void SX1276IoDeInit( void )
{
}

uint8_t SX1276GetPaSelect( uint32_t channel )
{
    if( channel > RF_MID_BAND_THRESH )
    {
        return RF_PACONFIG_PASELECT_PABOOST;
    }
    else
    {
        return RF_PACONFIG_PASELECT_RFO;
    }
}

void SX1276SetAntSwLowPower( bool status )
{
    if( RadioIsActive != status )
    {
        RadioIsActive = status;
   
        if( status == false )
        {
            SX1276AntSwInit( );
        }
        else
        {
            SX1276AntSwDeInit( );
        }
    }
}

void SX1276AntSwInit( void )
{
   
}

void SX1276AntSwDeInit( void )
{
}

void SX1276SetAntSw( uint8_t rxTx )
{
    if( SX1276.RxTx == rxTx )
    {
        return;
    }

    SX1276.RxTx = rxTx;

    if( rxTx != 0 ) // 1: TX, 0: RX
    {
        if (pAntSwitchLf != 0)
        {
            FTE_LWGPIO_setValue(pAntSwitchLf, FALSE);
        }
        if (pAntSwitchHf != 0)
        {
            FTE_LWGPIO_setValue(pAntSwitchHf, TRUE);
        }
    }
    else
    {
        if (pAntSwitchLf != 0)
        {
            FTE_LWGPIO_setValue(pAntSwitchLf, TRUE);
        }
        if (pAntSwitchHf != 0)
        {
            FTE_LWGPIO_setValue(pAntSwitchHf, FALSE);
        }
    }
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}
