#include "fte_target.h"

static 
FTE_RET        _FTE_AD7785_init(FTE_AD7785_PTR pAD7785);

static 
FTE_RET        _FTE_AD7785_getReg(FTE_AD7785_PTR pAD7785, FTE_UINT32 reg, FTE_UINT8_PTR data, FTE_UINT32 len);

static 
FTE_RET        _FTE_AD7785_setReg(FTE_AD7785_PTR pAD7785, FTE_UINT32 reg, FTE_UINT8_PTR data, FTE_UINT32 len);

static 
FTE_AD7785_PTR  _pHead      = NULL;
static 
FTE_UINT32      _nAD7785s   = 0;

FTE_AD7785_PTR  FTE_AD7785_get(FTE_UINT32 nOID)
{
    FTE_AD7785_PTR  pAD7785;
    
    pAD7785 = _pHead;
    while(pAD7785 != NULL)
    {
        if (pAD7785->pConfig->nID == nOID)
        {
            return  pAD7785;
        }
        
        pAD7785 = pAD7785->pNext;
    }
    
    return  NULL;
}


FTE_RET   FTE_AD7785_create
(
    FTE_AD7785_CONFIG_PTR   pConfig
)
{
    FTE_AD7785_PTR  pAD7785;
    
    assert(pConfig != NULL);
    
    pAD7785 = (FTE_AD7785_PTR)FTE_MEM_allocZero(sizeof(FTE_AD7785));
    if (pAD7785 == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    pAD7785->pNext  = _pHead;
    pAD7785->pConfig= pConfig;
    
    _pHead  = pAD7785;
    _nAD7785s++;
    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_AD7785_attach
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      nParent
)
{
    ASSERT(pAD7785 != NULL);
    
    pAD7785->pSPI = FTE_SPI_get(pAD7785->pConfig->xSPI);
    if (pAD7785->pSPI == NULL)
    {
        goto error;
    }
    
    if (FTE_SPI_attach(pAD7785->pSPI, pAD7785->pConfig->nID) != FTE_RET_OK)
    {
        goto error;
    }
    
    pAD7785->nParent = nParent;
    
    _FTE_AD7785_init(pAD7785);
    
    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_detach
(
    FTE_AD7785_PTR  pAD7785
)
{
    if (pAD7785 == NULL)
    {
        goto error;
    }
    
    if (FTE_SPI_detach(pAD7785->pSPI) != FTE_RET_OK)
    {
        goto error;
    }
    
    pAD7785->nParent = 0;
    
    return  FTE_RET_OK;
error:
    return  FTE_RET_ERROR;
}

FTE_BOOL     FTE_AD7785_exist
(   
    FTE_UINT32  nID
)
{
    FTE_AD7785_PTR  pAD7785 = FTE_AD7785_get(nID);
    
    return  (pAD7785 != NULL);
}

FTE_RET   FTE_AD7785_getReady
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_BOOL_PTR    pReady, 
    FTE_BOOL_PTR    pError
)
{
    FTE_UINT8          ubBuf = 0;

    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&ubBuf, 1) != FTE_RET_OK)
    {
         goto error;
    }
    if (pReady != NULL)
    {
        *pReady = (((ntohs(&ubBuf) & FTE_AD7785_STATUS_RDY_MASK) >> FTE_AD7785_STATUS_RDY_SHIFT) == FTE_AD7785_STATUS_RDY);
    }
    
    if (pError != NULL)
    {
        *pError = (((ntohs(&ubBuf) & FTE_AD7785_STATUS_ERR_MASK) >> FTE_AD7785_STATUS_ERR_SHIFT) == FTE_AD7785_STATUS_ERR);
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_runSingle
(
    FTE_AD7785_PTR  pAD7785
)
{
    ASSERT(pAD7785 != NULL);

    return  FTE_AD7785_setOPMode(pAD7785, FTE_AD7785_OP_MODE_SINGLE);
}

FTE_RET   FTE_AD7785_getOPMode
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  pMode
)
{
    FTE_UINT16  buf = 0;

    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&buf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    *pMode = (ntohs(&buf) & FTE_AD7785_OP_MODE_MASK) >> FTE_AD7785_OP_MODE_SHIFT;
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setOPMode
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      ulMode
)
{
    FTE_UINT16 usBuf1= 0, usBuf2 = 0;
    
    ASSERT(pAD7785 != NULL);
    
    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    usBuf2 = ntohs(&usBuf1);
    
    usBuf2 = (usBuf2 & ~FTE_AD7785_OP_MODE_MASK) | ((ulMode << FTE_AD7785_OP_MODE_SHIFT) & FTE_AD7785_OP_MODE_MASK);
        
    htons(&usBuf1, usBuf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    
    pAD7785->nLastValue = 0xFFFFFFFF;
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getClock
(   
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  pClk
)
{
    FTE_UINT16 usBuf = 0;
    
    ASSERT(pAD7785 != NULL);
    
    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    *pClk = (ntohs(&usBuf) & FTE_AD7785_CLK_MASK) >> FTE_AD7785_CLK_SHIFT;

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setClock
(   
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      clk
)
{
    FTE_UINT16 usBuf1 = 0, usBuf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    usBuf2 = ntohs(&usBuf1);
    
    usBuf2 = (usBuf2 & ~FTE_AD7785_CLK_MASK) | ((clk << FTE_AD7785_CLK_SHIFT) & FTE_AD7785_CLK_MASK);
        
    htons(&usBuf1, usBuf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getFreq
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  pFreq
)
{
    FTE_UINT16 usBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    *pFreq = (ntohs(&usBuf) & FTE_AD7785_FREQ_MASK) >> FTE_AD7785_FREQ_SHIFT;

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setFreq
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      ulFreq
)
{
    FTE_UINT16 usBuf1 = 0, usBuf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    usBuf2 = ntohs(&usBuf1);
    
    usBuf2 = (usBuf2 & ~FTE_AD7785_FREQ_MASK) | ((ulFreq << FTE_AD7785_FREQ_SHIFT) & FTE_AD7785_FREQ_MASK);
        
    htons(&usBuf1, usBuf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_MODE, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_UINT32     FTE_AD7785_measurementTime
(
    FTE_AD7785_PTR  pAD7785
)
{
#if 0
    static  FTE_UINT32 pMeasureTime[] = 
    {
        0, 
        4 + 10,  //  FTE_AD7785_FREQ_4MS
        8 + 10,  //  FTE_AD7785_FREQ_8MS                 0x02
        16 + 10, //  FTE_AD7785_FREQ_16MS                0x03
        32 + 10, //  FTE_AD7785_FREQ_32MS                0x04
        40 + 10, //  FTE_AD7785_FREQ_40MS                0x05
        48 + 10, //  FTE_AD7785_FREQ_48MS                0x06
        60 + 10, //  FTE_AD7785_FREQ_60MS                0x07
        101 + 10,//  FTE_AD7785_FREQ_101MS               0x08
        120 + 10,//  FTE_AD7785_FREQ_120MS               0x09
        160 + 10,//  FTE_AD7785_FREQ_160MS               0x0B
        200 + 10,//  FTE_AD7785_FREQ_200MS               0x0C
        240 + 10,//  FTE_AD7785_FREQ_240MS               0x0D
        320 + 10,//  FTE_AD7785_FREQ_320MS               0x0E
        480 + 10,//  FTE_AD7785_FREQ_480MS               0x0F        
    };
#endif
    return  500;
}

FTE_RET   FTE_AD7785_getConfig
(   
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  config
)
{
    FTE_UINT16 usBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    *config = ntohs(&usBuf);

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setConfig
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      config
)
{
    FTE_UINT16 usBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    htons(&usBuf, config);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getVBias
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  vbios
)
{
    FTE_UINT16 usBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    *vbios = (ntohs(&usBuf) & FTE_AD7785_CONFIG_VBIOS_MASK) >> FTE_AD7785_CONFIG_VBIOS_SHIFT;

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setVBias
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      vbios
)
{
    FTE_UINT16 usBuf1 = 0, usBuf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    usBuf2 = ntohs(&usBuf1);
    
    usBuf2 = (usBuf2 & ~FTE_AD7785_CONFIG_VBIOS_MASK) | ((vbios << FTE_AD7785_CONFIG_VBIOS_SHIFT) & FTE_AD7785_CONFIG_VBIOS_MASK);
        
    htons(&usBuf1, usBuf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getGain
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  gain
)
{
    FTE_UINT16 usBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    *gain = (ntohs(&usBuf) & FTE_AD7785_CONFIG_GAIN_MASK) >> FTE_AD7785_CONFIG_GAIN_SHIFT;

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setGain
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      gain
)
{
    FTE_UINT16 usBuf1 = 0, usBuf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    usBuf2 = ntohs(&usBuf1);
    
    usBuf2 = (usBuf2 & ~FTE_AD7785_CONFIG_GAIN_MASK) | ((gain << FTE_AD7785_CONFIG_GAIN_SHIFT) & FTE_AD7785_CONFIG_GAIN_MASK);
        
    htons(&usBuf1, usBuf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getChannel
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  channel
)
{
    FTE_UINT16 usBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf, 2) != FTE_RET_OK)
    {
         goto error;
    }
    *channel = (ntohs(&usBuf) & FTE_AD7785_CONFIG_CHANNEL_MASK) >> FTE_AD7785_CONFIG_CHANNEL_SHIFT;

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setChannel
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      channel
)
{
    FTE_UINT16 usBuf1 = 0, usBuf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    usBuf2 = ntohs(&usBuf1);
    
    usBuf2 = (usBuf2 & ~FTE_AD7785_CONFIG_CHANNEL_MASK) | ((channel << FTE_AD7785_CONFIG_CHANNEL_SHIFT) & FTE_AD7785_CONFIG_CHANNEL_MASK);
        
    htons(&usBuf1, usBuf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (FTE_UINT8_PTR)&usBuf1, 2) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getIOut
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  iout
)
{
    FTE_UINT8 bBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (FTE_UINT8_PTR)&bBuf, 1) != FTE_RET_OK)
    {
         goto error;
    }
    *iout = (bBuf & FTE_AD7785_IOUT_DIR_MASK) >> FTE_AD7785_IOUT_DIR_SHIFT;

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setIOut
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      iout
)
{
    FTE_UINT8 bBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (FTE_UINT8_PTR)&bBuf, 1) != FTE_RET_OK)
    {
         goto error;
    }
    
    bBuf = (bBuf & ~FTE_AD7785_IOUT_DIR_MASK) | ((iout << FTE_AD7785_IOUT_DIR_SHIFT) & FTE_AD7785_IOUT_DIR_MASK);
        
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_IOUT, (FTE_UINT8_PTR)&bBuf, 1) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getCurrent
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  current
)
{
    FTE_UINT8 bBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (FTE_UINT8_PTR)&bBuf, 1) != FTE_RET_OK)
    {
         goto error;
    }
    *current = (bBuf & FTE_AD7785_IOUT_CURRENT_MASK) >> FTE_AD7785_IOUT_CURRENT_SHIFT;

    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_setCurrent
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      current
)
{
    FTE_UINT8 bBuf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (FTE_UINT8_PTR)&bBuf, 1) != FTE_RET_OK)
    {
         goto error;
    }
    
    bBuf = (bBuf & ~FTE_AD7785_IOUT_CURRENT_MASK) | ((current << FTE_AD7785_IOUT_CURRENT_SHIFT) & FTE_AD7785_IOUT_CURRENT_MASK);
        
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_IOUT, (FTE_UINT8_PTR)&bBuf, 1) != FTE_RET_OK)
    {
         goto error;
    }
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_AD7785_getRawData
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  pValue
)
{
    FTE_UINT32 nValue;
    FTE_UINT8 pBuf[3] = {0, };
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_DATA, pBuf, 3) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }

    ((FTE_UINT8_PTR)&nValue)[0] = pBuf[2];
    ((FTE_UINT8_PTR)&nValue)[1] = pBuf[1];
    ((FTE_UINT8_PTR)&nValue)[2] = pBuf[0];
    ((FTE_UINT8_PTR)&nValue)[3] = 0;
    
    nValue >>= 4;
    
    *pValue = nValue;

    return  FTE_RET_OK;
}

FTE_RET   FTE_AD7785_getScaleData
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32_PTR  pValue
)
{
    FTE_RET   nRet;
    
    if (pAD7785->nLastValue == 0xFFFFFFFF)
    {
        FTE_UINT32     nValue = 0;
        
        nRet = FTE_AD7785_getRawData(pAD7785, &nValue);
        if (nRet != FTE_RET_OK)
        {
            return  nRet;
        }
        
        pAD7785->nLastValue = nValue;
    }
    
    *pValue = (FTE_UINT32)((float)pAD7785->nLastValue / 0x000FFFFF * 1170000) ;        
    
    return  FTE_RET_OK;
}

/******************************************************************************
 * Internal Static Functions
 ******************************************************************************/
static 
FTE_RET _FTE_AD7785_init
(
    FTE_AD7785_PTR  pAD7785
)
{
    FTE_UINT32 nValue, nNewValue;
        
    FTE_AD7785_setClock(pAD7785, FTE_AD7785_CLK_INTR_64K);
    FTE_AD7785_setFreq(pAD7785, FTE_AD7785_FREQ_120MS);

    FTE_AD7785_getConfig(pAD7785, &nValue);
    
    nNewValue = nValue | (FTE_AD7785_CONFIG_UNIPOLAR_MODE << FTE_AD7785_CONFIG_POL_SHIFT);
    nNewValue = (nNewValue & ~FTE_AD7785_CONFIG_GAIN_MASK) | (FTE_AD7785_CONFIG_GAIN_1 << FTE_AD7785_CONFIG_GAIN_SHIFT);
    nNewValue = (nNewValue & ~FTE_AD7785_CONFIG_REF_MASK) | (FTE_AD7785_CONFIG_REF_INTR << FTE_AD7785_CONFIG_REF_SHIFT);
    nNewValue = (nNewValue & ~FTE_AD7785_CONFIG_BUF_MASK) | (FTE_AD7785_CONFIG_BUF_MODE << FTE_AD7785_CONFIG_BUF_SHIFT);
    nNewValue = (nNewValue & ~FTE_AD7785_CONFIG_CHANNEL_MASK) | (FTE_AD7785_CONFIG_CHANNEL_2 << FTE_AD7785_CONFIG_CHANNEL_SHIFT);
    
    FTE_AD7785_setConfig(pAD7785, nNewValue);
    FTE_AD7785_setIOut(pAD7785, FTE_AD7785_IOUT_DIR_DIRECT);
    FTE_AD7785_setCurrent(pAD7785, FTE_AD7785_IOUT_CURRENT_0);    
    
    
    return  FTE_RET_OK;
}


static 
FTE_RET   _FTE_AD7785_getReg
(
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      ulReg, 
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      ulLen
)
{
    FTE_UINT8   ubCmd;
    
    assert((pAD7785 != NULL) && (pData != NULL) && (ulLen < 4));
    
    ubCmd = FTE_AD7785_CMD_BIT_READ | ((ulReg << FTE_AD7785_CMD_BIT_RS_SHIFT) & FTE_AD7785_CMD_BIT_RS_MASK);
    if (FTE_RET_OK != FTE_SPI_read(pAD7785->pSPI, &ubCmd, 1, pData, ulLen))
    {
        goto error;
    }
    
    return  FTE_RET_OK;
error:
    return  FTE_RET_ERROR;

}


FTE_RET   _FTE_AD7785_setReg
(   
    FTE_AD7785_PTR  pAD7785, 
    FTE_UINT32      ulReg, 
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      ulLen
)
{
    FTE_UINT8   ubCmd;

    assert((pAD7785 != NULL) && (pData != NULL) && (ulLen < 4));
    
    ubCmd = FTE_AD7785_CMD_BIT_WRITE | ((ulReg << FTE_AD7785_CMD_BIT_RS_SHIFT) & FTE_AD7785_CMD_BIT_RS_MASK);
    if (FTE_RET_OK != FTE_SPI_write(pAD7785->pSPI, &ubCmd, 1, pData, ulLen))
    {
        goto error;
    }
    
    return  FTE_RET_OK;
error:
    return  FTE_RET_ERROR;
}


FTE_INT32  FTE_AD7785_SHELL_cmd
(   
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;

    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_AD7785_PTR pAD7785;
                
                pAD7785 = _pHead;
                while(pAD7785 != NULL)
                {
                    if (pAD7785->pConfig != NULL)
                    {
                        printf("%08x : %08x\n", pAD7785->pConfig->nID, pAD7785->pConfig->xSPI);
                    }
                    pAD7785 = pAD7785->pNext;
                }        
            }
            break;
            
        case    2:
            {
                FTE_UINT32 nDID;
                
                
                if (!Shell_parse_hexnum(pArgv[1], &nDID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

#if 1
                FTE_AD7785_PTR  pAD7785 = FTE_AD7785_get(nDID);
                if (pAD7785 == NULL)
                {
                    goto error;
                }

                int nFieldSize[9] = { 1, 2, 2, 3, 1, 1, 3, 3};
                for(int i = 0 ; i < 8 ; i++)
                {
                    FTE_UINT8 pRegValue[4] = {0,};
                    
                    if (_FTE_AD7785_getReg(pAD7785, i, pRegValue, nFieldSize[i]) != FTE_RET_OK)
                    {
                        printf("%d : Invalid\n", i);
                    }
                    else
                    {
                        printf("%d : ", i);
                        for(int j = 0 ; j < nFieldSize[i] ; j++)
                        {
                            printf("%02x ", pRegValue[j]);
                        }
                    }
                    
                    printf("\n");
    
                }
#else           
                {
                    FTE_UINT32 nMode, nClk, nFreq, nData, nConfig, nVBias, nGain, nChannel, nIOut, nCurrent;
                    
                    FTE_AD7785_getOPMode(nDID, &nMode);
                    FTE_AD7785_getClock(nDID, &nClk);
                    FTE_AD7785_getFreq(nDID, &nFreq);
                    FTE_AD7785_getData(nDID, &nData);
                    FTE_AD7785_getConfig(nDID, &nConfig);
                    FTE_AD7785_getVBias(nDID, &nVBias);
                    FTE_AD7785_getGain(nDID, &nGain);
                    FTE_AD7785_getChannel(nDID, &nChannel);
                    FTE_AD7785_getIOut(nDID, &nIOut);
                    FTE_AD7785_getCurrent(nDID, &nCurrent);

                    printf("%08s : %08x\n", "Mode", nMode);
                    printf("%08s : %08x\n", "Clock", nClk);
                    printf("%08s : %08x\n", "Frequency", nFreq);
                    printf("%08s : %08x\n", "Data", nData);
                    printf("%08s : %08x\n", "Config", nConfig);
                    printf("%08s : %08x\n", "VBias", nVBias);
                    printf("%08s : %08x\n", "Gain", nGain);
                    printf("%08s : %08x\n", "Channel", nChannel);
                    printf("%08s : %08x\n", "IOut", nIOut);
                    printf("%08s : %08x\n", "Current", nCurrent);
                }
#endif
            }
            break;
            
        case    3:
            {
                FTE_UINT32 nDID;
                
                if (!Shell_parse_hexnum(pArgv[1], &nDID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (strcmp(pArgv[2], "start") == 0)
                {
                
                }
            }
            
        }
    }

error:    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s\n", pArgv[0]);
        }
    }

    return   xRet;
}
        