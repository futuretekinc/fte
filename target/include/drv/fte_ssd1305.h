#ifndef __FTE_SSD1305_H__
#define __FTE_SSD1305_H__

#define FTE_SSD1305_MEM_HADDR_MODE  0
#define FTE_SSD1305_MEM_VADDR_MODE  1
#define FTE_SSD1305_MEM_PADDR_MODE  2

#define FTE_SSD1305_DISPLAY_OFF 0
#define FTE_SSD1305_DISPLAY_ON  1
#define FTE_SSD1305_DISPLAY_DIM 2

#define FTE_SSD1305_COLOR_MODE_MONO     0x00
#define FTE_SSD1305_COLOR_MODE_COLOR    0x30
#define FTE_SSD1305_COLOR_MODE_COLOR_PS 0x35

typedef struct _FTE_SSD1305_CONFIG_STRUCT
{
    uint_32     nID;
    uint_32     xType;
    char_ptr    pName;
    uint_32     xGPIOPower;
    uint_32     xI2C;       // I2C channel id
}   FTE_SSD1305_CONFIG, _PTR_ FTE_SSD1305_CONFIG_PTR;

typedef FTE_SSD1305_CONFIG const _PTR_ FTE_SSD1305_CONFIG_CONST_PTR ;

typedef struct  _FTE_SSD1305_STRUCT
{
    struct _FTE_SSD1305_STRUCT *    pNext;
    FTE_SSD1305_CONFIG_CONST_PTR    pConfig;
    uint_32                         nParent;
    FTE_GPIO_PTR                    pGPIOPower;
    FTE_I2C_PTR                     pI2C;
    uint_32                         nLastValue;
}   FTE_SSD1305, _PTR_ FTE_SSD1305_PTR;


_mqx_uint   fte_ssd1305_create(FTE_SSD1305_CONFIG_PTR pConfig);
_mqx_uint   fte_ssd1305_attach(FTE_SSD1305_PTR pSSD1305, uint_32 nParent);
_mqx_uint   fte_ssd1305_detach(FTE_SSD1305_PTR pSSD1305);

FTE_SSD1305_PTR  fte_ssd1305_get(uint_32 nOID);

_mqx_uint   fte_ssd1305_SetMemoryAddressingMode(FTE_SSD1305_PTR pSSD1305, uint_32 ulMode);
_mqx_uint   fte_ssd1305_SetDisplayMode(FTE_SSD1305_PTR pSSD1305, uint_32 xMode);
_mqx_uint   fte_ssd1305_SetInverseMode(FTE_SSD1305_PTR pSSD1305, boolean bInverse);
_mqx_uint   fte_ssd1305_SetStartLine(FTE_SSD1305_PTR pSSD1305, uint_32 ulLine);
_mqx_uint   fte_ssd1305_SetStartPage(FTE_SSD1305_PTR pSSD1305, uint_32 ulPage);
_mqx_uint   fte_ssd1305_SetLowColumn(FTE_SSD1305_PTR pSSD1305, uint_32 ulLine);
_mqx_uint   fte_ssd1305_SetHighColumn(FTE_SSD1305_PTR pSSD1305, uint_32 ulLine);
_mqx_uint   fte_ssd1305_SetColumnAddress(FTE_SSD1305_PTR pSSD1305, uint_32 ulStart, uint_32 ulEnd);
_mqx_uint   fte_ssd1305_SetPageAddress(FTE_SSD1305_PTR pSSD1305, uint_32 ulStart, uint_32 ulEnd);
_mqx_uint   fte_ssd1305_SetOffset(FTE_SSD1305_PTR pSSD1305, uint_32 ulOffset);
_mqx_uint   fte_ssd1305_SetMultiplexRatio(FTE_SSD1305_PTR pSSD1305, uint_32 ulRatio);
_mqx_uint   fte_ssd1305_SetFrameFrequency(FTE_SSD1305_PTR pSSD1305, uint_32 ulFrequency);
_mqx_uint   fte_ssd1305_SetBank0Contrast(FTE_SSD1305_PTR pSSD1305, uint_32 ulContrast);
_mqx_uint   fte_ssd1305_SetColorBrightness(FTE_SSD1305_PTR pSSD1305, uint_32 ulBrightness);
_mqx_uint   fte_ssd1305_SetPrechargePeriod(FTE_SSD1305_PTR pSSD1305, uint_32 ulPeriod);
_mqx_uint   fte_ssd1305_SetOutputScanDirection(FTE_SSD1305_PTR pSSD1305, boolean bNormal);
_mqx_uint   fte_ssd1305_SetCommonPadsHardware(FTE_SSD1305_PTR pSSD1305);
_mqx_uint   fte_ssd1305_SetVCOMDeselectLevel(FTE_SSD1305_PTR pSSD1305, uint_32 ulMode);
_mqx_uint   fte_ssd1305_SetExternalVCC(FTE_SSD1305_PTR pSSD1305, boolean bExternalVCC);
_mqx_uint   fte_ssd1305_EnterReadModifyWrite(FTE_SSD1305_PTR pSSD1305);
_mqx_uint   fte_ssd1305_WriteData(FTE_SSD1305_PTR pSSD1305, uint_32 ulData);
_mqx_uint   fte_ssd1305_ReadData(FTE_SSD1305_PTR pSSD1305, uint_8_ptr puiData);
_mqx_uint   fte_ssd1305_SetAddress(FTE_SSD1305_PTR pSSD1305,uint_32 page, uint_32 lAddr, uint_32 hAddr);
_mqx_uint   fte_ssd1305_SetDisplayEnable(FTE_SSD1305_PTR pSSD1305, boolean bEnable);

int_32  fte_ssd1305_shell_cmd(int_32 argc, char_ptr argv[] );

#endif