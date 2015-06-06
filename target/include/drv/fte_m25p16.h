#ifndef __FTE_M25P16_H__
#define __FTE_M25P16_H__

/* Instruction Set */
#define M25P16_I_WREN      0x06 /* Write Enable */
#define M25P16_I_WRDI      0x04 /* Write Disable */
#define M25P16_I_RDID      0x9F /* Read Identification */
#define M25P16_I_RDSR      0x05 /* Read Status Register */
#define M25P16_I_WRSR      0x01 /* Write Status Register */
#define M25P16_I_READ      0x03 /* Read Data Bytes */
#define M25P16_I_FAST_READ 0x0B /* Read Data Bytes at Higher Speed */
#define M25P16_I_PP        0x02 /* Page Program */
#define M25P16_I_SE        0xD8 /* Sector Erase */
#define M25P16_I_BE        0xC7 /* Bulk Erase */
#define M25P16_I_DP        0xB9 /* Deep Power-down */
#define M25P16_I_RES       0xAB /* Release from Deep Power-down */

/* Dummy Byte - Used in FAST_READ and RES */
#define M25P16_DUMMY_BYTE  0x00

/* Pins */
#define M25P16_PIN_CLOCK   P1_5
#define M25P16_PIN_SER_I   P1_6
#define M25P16_PIN_SER_O   P1_7

/* Status Register Bits */
#define M25P16_SR_SRWD     0x80 /* Status Register Write Disable */
#define M25P16_SR_BP2      0x10 /* Block Protect 2 */
#define M25P16_SR_BP1      0x08 /* Block Protect 1 */
#define M25P16_SR_BP0      0x04 /* Block Protect 0 */
#define M25P16_SR_BP       0x1C /* All Block Protect Bits */
#define M25P16_SR_WEL      0x02 /* Write Enable Latch */
#define M25P16_SR_WIP      0x01 /* Write in Progress */

/* Do we use READ or FAST_READ to read? Fast by default */
#ifdef M25P16_CONF_READ_FAST
#define M25P16_READ_FAST M25P16_CONF_READ_FAST
#else
#define M25P16_READ_FAST 1
#endif
/*---------------------------------------------------------------------------*/
/** \brief Device Identifier
 *
 * Holds the value of the device identifier, returned by the RDID instruction.
 *
 * After a correct RDID, this structure should hold the following values:
 * man_id = 0x20, mem_type = 0x20, mem_size = 0x15, uid_len = 0x10.
 *
 * UID holds optional Customized Factory Data (CFD) content. The CFD bytes are
 * read-only and can be programmed with customers data upon their request.
 * If the customers do not make requests, the devices are shipped with all the
 * CFD bytes programmed to 0x00.
 */
struct m25p16_rdid 
{
    uint8_t  man_id;   /** Manufacturer ID */
    uint8_t  mem_type; /** Memory Type */
    uint8_t  mem_size; /** Memory Size */
    uint8_t  uid_len;  /** Unique ID length */
    uint8_t  uid[16];  /** Unique ID */
};

/*---------------------------------------------------------------------------*/
/**
* \brief Retrieve Block Protect Bits from the status register
*
* This macro returns the software block protect status on the device
* by reading the value of the BP bits ([5:3]) in the Status Register
*/
#define M25P16_BP() (m25p16_rdsr() & M25P16_SR_BP)
/**
* \brief Check for Write in Progress
* \retval 1 Write in progress
* \retval 0 Write not in progress
*
* This macro checks if the device is currently in the middle of a write cycle
* by reading the value of the WIP bit (bit 0) in the Status Register
*/
#define M25P16_WIP() (m25p16_rdsr() & M25P16_SR_WIP)
/**
* \brief Check for Write-Enable
* \retval 1 Write enabled
* \retval 0 Write disabled
*
* This macro checks if the device is ready to accept a write instruction
* by reading the value of the WEL bit (bit 1) in the Status Register
*/
#define M25P16_WEL() (m25p16_rdsr() & M25P16_SR_WEL)
/*---------------------------------------------------------------------------*/

typedef struct
{
    uint_32     nID;
    uint_32     xType;
    char_ptr    pName;
    uint_32     xSPI;       // SPI channel id
}   FTE_M25P16_CONFIG, _PTR_ FTE_M25P16_CONFIG_PTR;

typedef struct  
{
    FTE_M25P16_CONFIG_PTR   pConfig;
    uint_32                 nParent;
    FTE_SPI_PTR             pSPI;
}   FTE_M25P16, _PTR_ FTE_M25P16_PTR;


_mqx_uint   FTE_M25P16_getStatus(FTE_M25P16_PTR pM25P16, uint_32_ptr pulStatus);

#endif /* M25P16_H_ */