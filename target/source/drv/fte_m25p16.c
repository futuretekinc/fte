#include "fte_m25p16.h"

/*---------------------------------------------------------------------------*/
_mqx_uint    FTE_M25P16_writeEnable(FTE_M25P16_PTR pM25P16, boolean bEnable)
{
    uint_8  ubCMD = M25P16_I_WREN;
    uint_32 ulStatus;
    
    if (FTE_SPI_write(pM25P16->pSPI, &ubCMD, 1, NULL, 0) != MQX_OK)
    {
        return  MQX_ERROR;
    }

    if (bEnable)
    {
        while(!(ulStatus & M25P16_SR_WEL))
        {
            _time_delay(1);
            if (FTE_M25P16_getStatus(pM25P16, &ulStatus) != MQX_OK)
            {
                return  MQX_ERROR;
            }
        }
    }
    
    return  MQX_OK;
}

/*---------------------------------------------------------------------------*/
_mqx_uint   FTE_M25P16_getID(FTE_M25P16_PTR pM25P16)
{
    uint_8  ubCMD = M25P16_I_RDID;
    uint_8  pbDeviceID[20];
    
    if (FTE_SPI_read(pM25P16->pSPI, &ubCMD, 1, pbDeviceID, 20) != MQX_OK)
    {
    }
    
    return  MQX_OK;
}
/*---------------------------------------------------------------------------*/
_mqx_uint   FTE_M25P16_getStatus(FTE_M25P16_PTR pM25P16, uint_32_ptr pulStatus)
{
    uint_8  ubCMD = M25P16_I_WREN;
    uint_8  ubStatus = 0;

    if (FTE_SPI_read(pM25P16->pSPI, &ubCMD, 1, &ubStatus, 1) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    *pulStatus = ubStats;

    return return   MQX_OK;
}
/*---------------------------------------------------------------------------*/
void
m25p16_wrsr(uint8_t val)
{
  m25p16_wren(); /* Write Enable */

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  bit_bang_write(M25P16_I_WRSR);
  bit_bang_write(val);
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_read(uint8_t * addr, uint8_t * buff, uint8_t buff_len)
{
  uint8_t i;

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_READ);

#if M25P16_READ_FAST
  bit_bang_write(M25P16_I_FAST_READ);
#else
  bit_bang_write(M25P16_I_READ);
#endif

  /* Write the address, MSB in addr[0], bits [7:5] of the MSB: 'don't care' */
  for(i = 0; i < 3; i++) {
    bit_bang_write(addr[i]);
  }

  /* For FAST_READ, send the dummy byte */
#if M25P16_READ_FAST
  bit_bang_write(M25P16_DUMMY_BYTE);
#endif

  for(i = 0; i < buff_len; i++) {
    buff[i] = ~bit_bang_read();
  }
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_READ);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_pp(uint8_t * addr, uint8_t * buff, uint8_t buff_len)
{
  uint8_t i;

  m25p16_wren(); /* Write Enable */

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  bit_bang_write(M25P16_I_PP);

  /* Write the address, MSB in addr[0] */
  for(i = 0; i < 3; i++) {
    bit_bang_write(addr[i]);
  }

  /* Write the bytes */
  for(i = 0; i < buff_len; i++) {
    bit_bang_write(~buff[i]);
  }
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_se(uint8_t s)
{
  m25p16_wren(); /* Write Enable */

  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  bit_bang_write(M25P16_I_SE);
  bit_bang_write(s);
  bit_bang_write(0x00);
  bit_bang_write(0x00);
  deselect();
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
}
/*---------------------------------------------------------------------------*/
void
m25p16_be()
{
  m25p16_wren(); /* Write Enable */

  select();
  bit_bang_write(M25P16_I_BE);
  deselect();
}
/*---------------------------------------------------------------------------*/
void
m25p16_dp()
{
  select();
  bit_bang_write(M25P16_I_DP);
  deselect();
}
/*---------------------------------------------------------------------------*/
/*
 * Release Deep Power Down. We do NOT read the Electronic Signature
 */
void
m25p16_res()
{
  select();
  bit_bang_write(M25P16_I_RES);
  deselect();
  /* a few usec between RES and standby */
  while(M25P16_WIP());
}
/*---------------------------------------------------------------------------*/
/**
 * Release Deep Power Down. Read and return the Electronic Signature
 * must return 0x14
 *
 * \return The old style Electronic Signature. This must be 0x14
 */
uint8_t
m25p16_res_res()
{
  uint8_t rv;

  select();
  bit_bang_write(M25P16_I_RES);
  bit_bang_write(M25P16_DUMMY_BYTE);
  bit_bang_write(M25P16_DUMMY_BYTE);
  bit_bang_write(M25P16_DUMMY_BYTE);

  rv = bit_bang_read();

  deselect();

  /* a few usec between RES and standby */
  while(M25P16_WIP());
  return rv;
}
/*---------------------------------------------------------------------------*/
