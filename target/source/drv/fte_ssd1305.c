#include "fte_target.h"
#include "fte_ssd1305.h"

#define GRAM_COLUMN_OFFSET  1

#define SetAddress(page, lowerAddr, higherAddr) \
	FTE_I2C_write_command(page); \
    FTE_I2C_write_command(lowerAddr); \
    FTE_I2C_write_command(higherAddr); 

// Clipping region control
static uint_32  _clipRgn = 0;

// Clipping region borders
static uint_32  _clipLeft;
static uint_32  _clipRight;
static uint_32  _clipTop;
static uint_32  _clipBottom;

// Color
//GFX_COLOR   _color;


static FTE_SSD1305_PTR  _pHead      = NULL;
static uint_32          _nSSD1305s  = 0;

FTE_SSD1305_PTR  FTE_SSD1305_get(uint_32 nOID)
{
    FTE_SSD1305_PTR  pSSD1305;
    
    pSSD1305 = _pHead;
    while(pSSD1305 != NULL)
    {
        if (pSSD1305->pConfig->nID == nOID)
        {
            return  pSSD1305;
        }
        
        pSSD1305 = pSSD1305->pNext;
    }
    
    return  NULL;
}

_mqx_uint   FTE_SSD1305_create(FTE_SSD1305_CONFIG_PTR pConfig)
{
    FTE_SSD1305_PTR  pSSD1305;
    
    assert(pConfig != NULL);
    
    pSSD1305 = (FTE_SSD1305_PTR)FTE_MEM_allocZero(sizeof(FTE_SSD1305));
    if (pSSD1305 == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }
    
    pSSD1305->pNext  = _pHead;
    pSSD1305->pConfig= pConfig;
    
    _pHead  = pSSD1305;
    _nSSD1305s++;
    
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_attach(FTE_SSD1305_PTR pSSD1305, uint_32 nParent)
{
    ASSERT(pSSD1305 != NULL);
    
    pSSD1305->pI2C = FTE_I2C_get(pSSD1305->pConfig->xI2C);
    if (pSSD1305->pI2C == NULL)
    {
        goto error;
    }
    
    pSSD1305->pGPIOPower = FTE_GPIO_get(pSSD1305->pConfig->xGPIOPower);
    if (pSSD1305->pGPIOPower == NULL)
    {
        goto error;
    }
    
    if (FTE_I2C_attach(pSSD1305->pI2C, pSSD1305->pConfig->nID) != MQX_OK)
    {
        goto error;
    }
    
    if (FTE_GPIO_attach(pSSD1305->pGPIOPower, pSSD1305->pConfig->nID) != MQX_OK)
    {
        FTE_I2C_detach(pSSD1305->pI2C);
        goto error;
    }
    
    pSSD1305->nParent = nParent;    
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

_mqx_uint   FTE_SSD1305_detach(FTE_SSD1305_PTR pSSD1305)
{
    if (pSSD1305 == NULL)
    {
        goto error;
    }
    
    if (FTE_I2C_detach(pSSD1305->pI2C) != MQX_OK)
    {
        goto error;
    }
    
    pSSD1305->nParent = 0;
    
    return  MQX_OK;
error:
    return  MQX_ERROR;
}

/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates
*
* Output: none
*
* Side Effects: none
*
* Overview: puts pixel
*
* Note: none
*
********************************************************************/
void FTE_SSD1305_put_pixel(FTE_SSD1305_PTR pSSD1305, uint_32   x, uint_32  y)
{
    uint_8      page, add, lAddr, hAddr;
    uint_8      mask, display;

    // check if point is in clipping region
    if(_clipRgn)
    {
        if(x < _clipLeft)
            return;
        if(x > _clipRight)
            return;
        if(y < _clipTop)
            return;
        if(y > _clipBottom)
            return;
    }

    // Assign a page address
    page = 0xB0 | (y / 8);
    add = x + GRAM_COLUMN_OFFSET; 
    lAddr = 0x0F & add;             // Low address
    hAddr = 0x10 | (add >> 4);      // High address

    // Calculate mask from rows basically do a y%8 and remainder is bit position
    mask = 1 << (y % 8);             // Left shift 1 by bit position

    //    DisplayEnable();
    FTE_SSD1305_SetDisplayEnable(pSSD1305, TRUE);

    // Set the address (sets the page,
    // lower and higher column address pointers)
    FTE_SSD1305_SetAddress(pSSD1305, page, lAddr, hAddr);
    
    // enter read-modify-write so we do not need to re-issue address 
    FTE_SSD1305_EnterReadModifyWrite(pSSD1305);

    FTE_SSD1305_ReadData(pSSD1305, &display);   // Read to initiate Read transaction on PMP and dummy read
                                                // (requirement for data synchronization in the controller)
    FTE_SSD1305_ReadData(pSSD1305, &display);   // Read again as a requirement for data synchronization in the display controller
    FTE_SSD1305_ReadData(pSSD1305, &display);   // Read actual data from from display buffer

#if 0
    if(_color > 0)                  // If non-zero for pixel on
        display |= mask;            // or in mask
    else
#endif
    // If 0 for pixel off
    //display &= ~mask;           // and with inverted mask
    display |= mask;           // and with inverted mask

    // lower and higher column address pointers)
    // restore the byte with manipulated bit
    FTE_SSD1305_WriteData(pSSD1305, display);    

    // exit read modify write, SSD1305 address pointer is set 
    // back to the last address set
    FTE_SSD1305_EnterReadModifyWrite(pSSD1305);
    FTE_SSD1305_SetDisplayEnable(pSSD1305, FALSE);
    
}

_mqx_uint   FTE_SSD1305_SetDisplayMode(FTE_SSD1305_PTR pSSD1305, uint_32 xMode)
{
    uint_8  pData[2] = { 0x00};
    
    switch(xMode)
    {
    case    FTE_SSD1305_DISPLAY_OFF:    pData[1] = 0xAE; break;
    case    FTE_SSD1305_DISPLAY_ON:     pData[1] = 0xAF; break;
    case    FTE_SSD1305_DISPLAY_DIM:    pData[1] = 0xAC; break;
    default:
            return   MQX_ERROR;
    }

    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetInverseMode(FTE_SSD1305_PTR pSSD1305, boolean bInverse)
{
    uint_8  pData[2] = { 0x00};
    
    pData[1] = (bInverse)?0xA7:0xA6;
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetStartLine(FTE_SSD1305_PTR pSSD1305, uint_32 ulLine)
{
    uint_8  pData[2] = { 0x00};
    
    pData[1] = 0x40 | ((uint_8)ulLine & 0x3F);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetMemoryAddressingMode(FTE_SSD1305_PTR pSSD1305, uint_32 ulMode)
{
    uint_8  pData[4] = { 0x00, 0x20, 0x00};
    
    pData[3] = ((uint_8)ulMode & 0x03);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetColumnAddress(FTE_SSD1305_PTR pSSD1305, uint_32 ulStart, uint_32 ulEnd)
{
    uint_8  pData[6] = { 0x00, 0x21, 0x00, 0x00, 0x00, 0x00};
    
    pData[3] = ((uint_8)ulStart);
    pData[5] = ((uint_8)ulEnd);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[4], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetPageAddress(FTE_SSD1305_PTR pSSD1305, uint_32 ulStart, uint_32 ulEnd)
{
    uint_8  pData[6] = { 0x00, 0x22, 0x00, 0x00, 0x00, 0x00};
    
    pData[3] = ((uint_8)ulStart & 0x07);
    pData[5] = ((uint_8)ulStart & ulEnd);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[4], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetOrientation(FTE_SSD1305_PTR pSSD1305, boolean bReserve)
{
    uint_8  pData[2] = { 0x00, 0x00};
    
    if (bReserve)
    {
        pData[1] = 0xA1;
    }
    else
    {
        pData[1] = 0xA0;
    }
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, sizeof(pData));
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetDisplayEnable(FTE_SSD1305_PTR pSSD1305, boolean bEnable)
{
    uint_8  pData[2] = { 0x00, 0x00};
    
    if (bEnable)
    {
        pData[1] = 0xA4;
    }
    else
    {
        pData[1] = 0xA5;
    }
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, sizeof(pData));
    
    return  MQX_OK;
}


_mqx_uint   FTE_SSD1305_SetMultiplexRatio(FTE_SSD1305_PTR pSSD1305, uint_32 ulRatio)
{
    uint_8  pData[4] = { 0x00, 0xA8, 0x00, 0x00};
    
    pData[3] = ((uint_8)ulRatio & 0x3F);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetOffset(FTE_SSD1305_PTR pSSD1305, uint_32 ulOffset)
{
    uint_8  pData[4] = { 0x00, 0xD3, 0x00, 0x00};
    
    pData[3] = ((uint_8)ulOffset & 0x3F);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetStartPage(FTE_SSD1305_PTR pSSD1305, uint_32 ulPage)
{
    uint_8  pData[2] = { 0x00, };
    
    pData[1] = 0xB0 | ((uint_8)ulPage& 0x07);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    
    return  MQX_OK;
}


_mqx_uint   FTE_SSD1305_SetFrameFrequency(FTE_SSD1305_PTR pSSD1305, uint_32 ulFrequency)
{
    uint_8  pData[4] = { 0x00, 0xD5, 0x00, 0x00};
    
    pData[3] = ((uint_8)ulFrequency & 0xFF);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetColorMode(FTE_SSD1305_PTR pSSD1305, uint_32 ulMode)
{
    uint_8  pData[4] = { 0x00, 0xD8, 0x00};
    
    pData[3] = ((uint_8)ulMode & 0x35);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
     FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
   
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetBank0Contrast(FTE_SSD1305_PTR pSSD1305, uint_32 ulContrast)
{
    uint_8  pData[4] = { 0x00, 0x81, 0x00};
    
    pData[3] = ((uint_8)ulContrast & 0xFF);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetColorBrightness(FTE_SSD1305_PTR pSSD1305, uint_32 ulBrightness)
{
    uint_8  pData[4] = { 0x00, 0x82, 0x00};
    
    pData[3] = ((uint_8)ulBrightness & 0xFF);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetPrechargePeriod(FTE_SSD1305_PTR pSSD1305, uint_32 ulPeriod)
{
    uint_8  pData[4] = { 0x00, 0xD9, 0x00};
    
    pData[3] = ((uint_8)ulPeriod & 0xFF);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetCommonPadsHardware(FTE_SSD1305_PTR pSSD1305)
{
    uint_8  pData[4] = { 0x00, 0xDA, 0x00, 0x12};
    
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetOutputScanDirection(FTE_SSD1305_PTR pSSD1305, boolean bNormal)
{
    uint_8  pData[2] = { 0x00, 0x00};
    
    if (bNormal)
    {
        pData[1] = 0xC0;
    }
    else
    {
        pData[1] = 0xC8;
    }
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetVCOMDeselectLevel(FTE_SSD1305_PTR pSSD1305, uint_32 ulMode)
{
    uint_8  pData[4] = { 0x00, 0xDB, 0x00};
    
    pData[3] = ((uint_8)ulMode & 0x3C);
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}


_mqx_uint   FTE_SSD1305_SetExternalVCC(FTE_SSD1305_PTR pSSD1305, boolean bExternalVCC)
{
    uint_8  pData[4] = { 0x00, 0xAD, 0x00};
    
    if (bExternalVCC)
    {
        pData[3] = 0x8E;
    }
    else
    {
        pData[3] = 0x8F;
    }
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    
    return  MQX_OK;
}
_mqx_uint   FTE_SSD1305_EnterReadModifyWrite(FTE_SSD1305_PTR pSSD1305)
{
    uint_8  pData[2] = { 0x00, 0xE0};
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_WriteData(FTE_SSD1305_PTR pSSD1305, uint_32 ulData)
{
    uint_8  pData[2] = { 0x40, };
    
    pData[1] = (uint_8)ulData;
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_ReadData(FTE_SSD1305_PTR pSSD1305, uint_8_ptr puiData)
{
    uint_8  pData[2] = { 0x00, };
    
    FTE_I2C_read(pSSD1305->pI2C, 0x3C, pData, 2);
    
    *puiData = pData[1];
    
    return  MQX_OK;
}

_mqx_uint   FTE_SSD1305_SetAddress(FTE_SSD1305_PTR pSSD1305,uint_32 page, uint_32 lAddr, uint_32 hAddr)
{
    uint_8  pData[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    pData[1] = (uint_8)page;
    pData[1] = (uint_8)lAddr;
    pData[1] = (uint_8)hAddr;
    
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, pData, 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[2], 2);
    FTE_I2C_write(pSSD1305->pI2C, 0x3C, &pData[4], 2);
    
    return  MQX_OK;
}


/******************************************************************************
 * Internal Static Functions
 ******************************************************************************/
_mqx_uint   FTE_SSD1305_init(FTE_SSD1305_PTR pSSD1305)
{
   assert(pSSD1305 != NULL);
   
    FTE_GPIO_setValue(pSSD1305->pGPIOPower, TRUE);
    
        /*Display on*/
        FTE_SSD1305_SetDisplayMode(pSSD1305, FTE_SSD1305_DISPLAY_OFF);

        /*Display Memory Address mode*/
        FTE_SSD1305_SetMemoryAddressingMode(pSSD1305, FTE_SSD1305_MEM_PADDR_MODE); 
        
        FTE_SSD1305_SetColumnAddress(pSSD1305, 0, 0x83);
        FTE_SSD1305_SetPageAddress(pSSD1305, 0, 0x07);
        FTE_SSD1305_SetStartLine(pSSD1305, 0x40);

        /*Display Segment Re_map mode*/
        FTE_SSD1305_SetOrientation(pSSD1305, TRUE);
        FTE_SSD1305_SetDisplayEnable(pSSD1305, TRUE);
        FTE_SSD1305_SetInverseMode(pSSD1305, TRUE);
        
        FTE_SSD1305_SetStartPage(pSSD1305, 0);

        /*Display Multiplex Ratio*/
        FTE_SSD1305_SetMultiplexRatio(pSSD1305, 0x1F);

        /*Display offset*/
        FTE_SSD1305_SetOffset(pSSD1305, 0);
        
        /*Display Frame Frequence*/
        FTE_SSD1305_SetFrameFrequency(pSSD1305, 0x51);

        /*Display Area color mode ON/OFF*/
        FTE_SSD1305_SetColorMode(pSSD1305, FTE_SSD1305_COLOR_MODE_MONO);
        
        /*Display COM Configuration*/
        FTE_SSD1305_SetCommonPadsHardware(pSSD1305);
        
        /*Display Bank0 of Contrast*/
        FTE_SSD1305_SetBank0Contrast(pSSD1305, 0x3F);

        FTE_SSD1305_SetColorBrightness(pSSD1305, 0x00);
        
        /*Display Pre_charge period*/
        FTE_SSD1305_SetPrechargePeriod(pSSD1305, 0x22);

        /*Display VComh Deselect Level*/
        FTE_SSD1305_SetVCOMDeselectLevel(pSSD1305, 0x25);

        
        /*Display Internal DCDC Disable*/
        FTE_SSD1305_SetExternalVCC(pSSD1305, TRUE);
        _time_delay(100);
        
        /*Display ON*/
        FTE_SSD1305_SetDisplayMode(pSSD1305, FTE_SSD1305_DISPLAY_ON);

 
        FTE_SSD1305_SetDisplayEnable(pSSD1305, FALSE);
        
    return  MQX_OK;
}

void FTE_SSD1305_clear(FTE_SSD1305_PTR pSSD1305) 
{
    FTE_SSD1305_SetStartLine(pSSD1305, 0);
}

int_32  FTE_SSD1305_shell_cmd(int_32 argc, char_ptr argv[] )
{ 
    boolean             print_usage, shorthelp = FALSE;
    int_32              return_code = SHELL_EXIT_SUCCESS;
    FTE_SSD1305_PTR     pSSD1305 = (FTE_SSD1305_PTR)FTE_SSD1305_get(FTE_DEV_SSD1305_0);

    if (pSSD1305 == NULL)
    {
        goto error;
    }
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);
    if (print_usage)
    {
        goto error;
    }
    
    switch(argc)
    {
    case    2:
        {
            if (strcmp(argv[1], "init") == 0)
            {
                FTE_SSD1305_init(pSSD1305);

                for(int i = 0 ; i < 63 ; i++)
                {
                    FTE_SSD1305_put_pixel(pSSD1305, i, i);
                }
            }
        }
        break;

    default:
        goto error;
    }
    
error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s \n", argv[0]);
        }
        else
        {
            printf("Usage : %s \n", argv[0]);
            printf("        id       - I2C Channel \n");
            printf("        baudrate - I2C speed \n");
        }
    }

    return   return_code;
}
