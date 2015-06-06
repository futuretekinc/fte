#ifndef __FTE_AD7785_H__
#define __FTE_AD7785_H__

#define FTE_AD7785_CMD_BIT_WRITE            0x00
#define FTE_AD7785_CMD_BIT_READ             0x40
#define FTE_AD7785_CMD_BIT_RS_MASK          0x38
#define FTE_AD7785_CMD_BIT_RS_SHIFT         3
#define FTE_AD7785_CMD_BIT_CREAD            0x04

#define FTE_AD7785_REG_CMD                  0x00
#define FTE_AD7785_REG_STATUS               0x00
#define FTE_AD7785_REG_MODE                 0x01
#define FTE_AD7785_REG_CONFIG               0x02
#define FTE_AD7785_REG_DATA                 0x03
#define FTE_AD7785_REG_ID                   0x04
#define FTE_AD7785_REG_IOUT                 0x05
#define FTE_AD7785_REG_OFFSET               0x06
#define FTE_AD7785_REG_FULL                 0x07

#define FTE_AD7785_STATUS_RDY_MASK          0x80
#define FTE_AD7785_STATUS_RDY_SHIFT         7
#define FTE_AD7785_STATUS_RDY               0x00

#define FTE_AD7785_STATUS_ERR_MASK          0x40
#define FTE_AD7785_STATUS_ERR_SHIFT         6
#define FTE_AD7785_STATUS_ERR               0x01

#define FTE_AD7785_STATUS_CHANNEL_MASK      0x07
#define FTE_AD7785_STATUS_CHANNEL_SHIFT     0
#define FTE_AD7785_STATUS_CHANNEL_0         0x00
#define FTE_AD7785_STATUS_CHANNEL_1         0x01
#define FTE_AD7785_STATUS_CHANNEL_2         0x02
#define FTE_AD7785_STATUS_CHANNEL_3         0x03
#define FTE_AD7785_STATUS_CHANNEL_4         0x04
#define FTE_AD7785_STATUS_CHANNEL_5         0x05
#define FTE_AD7785_STATUS_CHANNEL_6         0x06
#define FTE_AD7785_STATUS_CHANNEL_7         0x07

#define FTE_AD7785_OP_MODE_MASK             0xE000
#define FTE_AD7785_OP_MODE_SHIFT            13

#define FTE_AD7785_OP_MODE_CONTINUOUS       0x00
#define FTE_AD7785_OP_MODE_SINGLE           0x01
#define FTE_AD7785_OP_MODE_IDLE             0x02
#define FTE_AD7785_OP_MODE_PWR_DOWN         0x03
#define FTE_AD7785_OP_MODE_INTR_ZERO_CALIB  0x04
#define FTE_AD7785_OP_MODE_INTR_FULL_CALIB  0x05
#define FTE_AD7785_OP_MODE_SYS_ZERO_CALIB   0x06
#define FTE_AD7785_OP_MODE_SYS_FULL_CALIB   0x07

#define FTE_AD7785_CLK_MASK                 0x00C0
#define FTE_AD7785_CLK_SHIFT                6

#define FTE_AD7785_CLK_INTR_64K             0x00
#define FTE_AD7785_CLK_INTR_64K_WITH_OUT    0x01
#define FTE_AD7785_CLK_EXTR_64K             0x02
#define FTE_AD7785_CLK_EXTR                 0x03

#define FTE_AD7785_FREQ_MASK                0x000F
#define FTE_AD7785_FREQ_SHIFT               0

#define FTE_AD7785_FREQ_INVALID             0x00
#define FTE_AD7785_FREQ_4MS                 0x01
#define FTE_AD7785_FREQ_8MS                 0x02
#define FTE_AD7785_FREQ_16MS                0x03
#define FTE_AD7785_FREQ_32MS                0x04
#define FTE_AD7785_FREQ_40MS                0x05
#define FTE_AD7785_FREQ_48MS                0x06
#define FTE_AD7785_FREQ_60MS                0x07
#define FTE_AD7785_FREQ_101MS               0x08
#define FTE_AD7785_FREQ_120MS               0x09
#define FTE_AD7785_FREQ_160MS               0x0B
#define FTE_AD7785_FREQ_200MS               0x0C
#define FTE_AD7785_FREQ_240MS               0x0D
#define FTE_AD7785_FREQ_320MS               0x0E
#define FTE_AD7785_FREQ_480MS               0x0F

#define FTE_AD7785_CONFIG_VBIOS_MASK        0xC000
#define FTE_AD7785_CONFIG_VBIOS_SHIFT       14

#define FTE_AD7785_CONFIG_VBIAS_NONE        0x00
#define FTE_AD7785_CONFIG_VBIAS_AIN1        0x01
#define FTE_AD7785_CONFIG_VBIAS_AIN2        0x02

#define FTE_AD7785_CONFIG_BO_MASK           0x2000
#define FTE_AD7785_CONFIG_BO_SHIFT          13
#define FTE_AD7785_CONFIG_BO_ENABLE         0x01

#define FTE_AD7785_CONFIG_POL_MASK          0x1000
#define FTE_AD7785_CONFIG_POL_SHIFT         12
#define FTE_AD7785_CONFIG_BIPOLAR_MODE      0x00
#define FTE_AD7785_CONFIG_UNIPOLAR_MODE     0x01

#define FTE_AD7785_CONFIG_BOOST_MASK        0x0800
#define FTE_AD7785_CONFIG_BOOST_SHIFT       11
#define FTE_AD7785_CONFIG_BOOST             0x01

#define FTE_AD7785_CONFIG_GAIN_MASK         0x0700
#define FTE_AD7785_CONFIG_GAIN_SHIFT        8
#define FTE_AD7785_CONFIG_GAIN_1            0x00
#define FTE_AD7785_CONFIG_GAIN_2            0x01
#define FTE_AD7785_CONFIG_GAIN_4            0x02
#define FTE_AD7785_CONFIG_GAIN_8            0x03
#define FTE_AD7785_CONFIG_GAIN_16           0x04
#define FTE_AD7785_CONFIG_GAIN_32           0x05
#define FTE_AD7785_CONFIG_GAIN_64           0x06
#define FTE_AD7785_CONFIG_GAIN_128          0x07

#define FTE_AD7785_CONFIG_REF_MASK          0x0080
#define FTE_AD7785_CONFIG_REF_SHIFT         7
#define FTE_AD7785_CONFIG_REF_EXTR          0x00
#define FTE_AD7785_CONFIG_REF_INTR          0x01

#define FTE_AD7785_CONFIG_BUF_MASK          0x10
#define FTE_AD7785_CONFIG_BUF_SHIFT         4
#define FTE_AD7785_CONFIG_BUF_MODE          0x01

#define FTE_AD7785_CONFIG_CHANNEL_MASK      0x0007
#define FTE_AD7785_CONFIG_CHANNEL_SHIFT     0
#define FTE_AD7785_CONFIG_CHANNEL_1         0x00
#define FTE_AD7785_CONFIG_CHANNEL_2         0x01
#define FTE_AD7785_CONFIG_CHANNEL_3         0x02
#define FTE_AD7785_CONFIG_CHANNEL_ZERO      0x03
#define FTE_AD7785_CONFIG_CHANNEL_TEMP      0x06
#define FTE_AD7785_CONFIG_CHANNEL_AVDD      0x07

#define FTE_AD7785_IOUT_DIR_MASK            0x0C
#define FTE_AD7785_IOUT_DIR_SHIFT           2
#define FTE_AD7785_IOUT_DIR_DIRECT          0x00
#define FTE_AD7785_IOUT_DIR_CROSS           0x01
#define FTE_AD7785_IOUT_DIR_1               0x02
#define FTE_AD7785_IOUT_DIR_2               0x03

#define FTE_AD7785_IOUT_CURRENT_MASK        0x03
#define FTE_AD7785_IOUT_CURRENT_SHIFT       0
#define FTE_AD7785_IOUT_CURRENT_0           0x00
#define FTE_AD7785_IOUT_CURRENT_10UA        0x01
#define FTE_AD7785_IOUT_CURRENT_210UA       0x02
#define FTE_AD7785_IOUT_CURRENT_1MA         0x03



typedef struct _FTE_AD7785_CONFIG_STRUCT
{
    uint_32     nID;
    uint_32     xType;
    char_ptr    pName;
    uint_32     xSPI;       // SPI channel id
}   FTE_AD7785_CONFIG, _PTR_ FTE_AD7785_CONFIG_PTR;

typedef FTE_AD7785_CONFIG const _PTR_ FTE_AD7785_CONFIG_CONST_PTR ;

typedef struct  _FTE_AD7785_STRUCT
{
    struct _FTE_AD7785_STRUCT * pNext;
    FTE_AD7785_CONFIG_CONST_PTR pConfig;
    uint_32                     nParent;
    FTE_SPI_PTR                 pSPI;
    uint_32                     nLastValue;
}   FTE_AD7785, _PTR_ FTE_AD7785_PTR;


FTE_AD7785_PTR  FTE_AD7785_get(uint_32 nOID);

_mqx_uint   FTE_AD7785_create(FTE_AD7785_CONFIG_PTR pConfig);
_mqx_uint   FTE_AD7785_attach(FTE_AD7785_PTR pAD7785, uint_32 nParent);
_mqx_uint   FTE_AD7785_detach(FTE_AD7785_PTR pAD7785);

_mqx_uint   FTE_AD7785_getReady(FTE_AD7785_PTR pAD7785, boolean *pReady, boolean *pError);

_mqx_uint   FTE_AD7785_runSingle(FTE_AD7785_PTR pAD7785);

_mqx_uint   FTE_AD7785_getOPMode(FTE_AD7785_PTR pAD7785, uint_32 *pMode);
_mqx_uint   FTE_AD7785_setOPMode(FTE_AD7785_PTR pAD7785, uint_32 nMode);

_mqx_uint   FTE_AD7785_getClock(FTE_AD7785_PTR pAD7785, uint_32 *pClk);
_mqx_uint   FTE_AD7785_setClock(FTE_AD7785_PTR pAD7785, uint_32 nClk);

_mqx_uint   FTE_AD7785_getFreq(FTE_AD7785_PTR pAD7785, uint_32 *pFreq);
_mqx_uint   FTE_AD7785_setFreq(FTE_AD7785_PTR pAD7785, uint_32 nFreq);
uint_32     FTE_AD7785_measurementTime(FTE_AD7785_PTR pAD7785);

_mqx_uint   FTE_AD7785_getRawData(FTE_AD7785_PTR pAD7785, uint_32 *pValue);
_mqx_uint   FTE_AD7785_getScaleData(FTE_AD7785_PTR pAD7785, uint_32 *pValue);

_mqx_uint   FTE_AD7785_getConfig(FTE_AD7785_PTR pAD7785, uint_32 *pConfig);
_mqx_uint   FTE_AD7785_setConfig(FTE_AD7785_PTR pAD7785, uint_32 nConfig);

_mqx_uint   FTE_AD7785_getVBias(FTE_AD7785_PTR pAD7785, uint_32 *pVBias);
_mqx_uint   FTE_AD7785_setVBias(FTE_AD7785_PTR pAD7785, uint_32 nVBias);

_mqx_uint   FTE_AD7785_getGain(FTE_AD7785_PTR pAD7785, uint_32 *pGain);
_mqx_uint   FTE_AD7785_setGain(FTE_AD7785_PTR pAD7785, uint_32 nGain);

_mqx_uint   FTE_AD7785_getChannel(FTE_AD7785_PTR pAD7785, uint_32 *pChannel);
_mqx_uint   FTE_AD7785_setChannel(FTE_AD7785_PTR pAD7785, uint_32 nChannel);

_mqx_uint   FTE_AD7785_getIOut(FTE_AD7785_PTR pAD7785, uint_32 *iout);
_mqx_uint   FTE_AD7785_setIOut(FTE_AD7785_PTR pAD7785, uint_32 iout);

_mqx_uint   FTE_AD7785_getCurrent(FTE_AD7785_PTR pAD7785, uint_32 *current);
_mqx_uint   FTE_AD7785_setCurrent(FTE_AD7785_PTR pAD7785, uint_32 current);

int_32      FTE_AD7785_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
