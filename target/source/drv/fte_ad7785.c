#include "fte_target.h"

static _mqx_uint        _FTE_AD7785_init(FTE_AD7785_PTR pAD7785);
static _mqx_uint        _FTE_AD7785_getReg(FTE_AD7785_PTR pAD7785, uint_32 reg, uint_8 *data, uint_32 len);
static _mqx_uint        _FTE_AD7785_setReg(FTE_AD7785_PTR pAD7785, uint_32 reg, uint_8 *data, uint_32 len);

static FTE_AD7785_PTR   _pHead      = NULL;
static uint_32          _nAD7785s   = 0;

FTE_AD7785_PTR  FTE_AD7785_get(uint_32 nOID)
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


_mqx_uint   FTE_AD7785_create(FTE_AD7785_CONFIG_PTR pConfig)
{
    FTE_AD7785_PTR  pAD7785;
    
    assert(pConfig != NULL);
    
    pAD7785 = (FTE_AD7785_PTR)FTE_MEM_allocZero(sizeof(FTE_AD7785));
    if (pAD7785 == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }
    
    pAD7785->pNext  = _pHead;
    pAD7785->pConfig= pConfig;
    
    _pHead  = pAD7785;
    _nAD7785s++;
    
    
    return  MQX_OK;
}

_mqx_uint   FTE_AD7785_attach(FTE_AD7785_PTR pAD7785, uint_32 nParent)
{
    ASSERT(pAD7785 != NULL);
    
    pAD7785->pSPI = FTE_SPI_get(pAD7785->pConfig->xSPI);
    if (pAD7785->pSPI == NULL)
    {
        goto error;
    }
    
    if (FTE_SPI_attach(pAD7785->pSPI, pAD7785->pConfig->nID) != MQX_OK)
    {
        goto error;
    }
    
    pAD7785->nParent = nParent;
    
    _FTE_AD7785_init(pAD7785);
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_detach(FTE_AD7785_PTR pAD7785)
{
    if (pAD7785 == NULL)
    {
        goto error;
    }
    
    if (FTE_SPI_detach(pAD7785->pSPI) != MQX_OK)
    {
        goto error;
    }
    
    pAD7785->nParent = 0;
    
    return  MQX_OK;
error:
    return  MQX_ERROR;
}

boolean     FTE_AD7785_exist(uint_32 nID)
{
    FTE_AD7785_PTR  pAD7785 = FTE_AD7785_get(nID);
    
    return  (pAD7785 != NULL);
}

_mqx_uint   FTE_AD7785_getReady(FTE_AD7785_PTR pAD7785, boolean *pReady, boolean *pError)
{
    uint_8          buf = 0;

    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf, 1) != MQX_OK)
    {
         goto error;
    }
    if (pReady != NULL)
    {
        *pReady = (((ntohs(&buf) & FTE_AD7785_STATUS_RDY_MASK) >> FTE_AD7785_STATUS_RDY_SHIFT) == FTE_AD7785_STATUS_RDY);
    }
    
    if (pError != NULL)
    {
        *pError = (((ntohs(&buf) & FTE_AD7785_STATUS_ERR_MASK) >> FTE_AD7785_STATUS_ERR_SHIFT) == FTE_AD7785_STATUS_ERR);
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_runSingle(FTE_AD7785_PTR pAD7785)
{
    ASSERT(pAD7785 != NULL);

    return  FTE_AD7785_setOPMode(pAD7785, FTE_AD7785_OP_MODE_SINGLE);
}

_mqx_uint   FTE_AD7785_getOPMode(FTE_AD7785_PTR pAD7785, uint_32 *mode)
{
    uint_16         buf = 0;

    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    *mode = (ntohs(&buf) & FTE_AD7785_OP_MODE_MASK) >> FTE_AD7785_OP_MODE_SHIFT;
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_setOPMode(FTE_AD7785_PTR pAD7785, uint_32 mode)
{
    uint_16 buf1, buf2 = 0;
    
    ASSERT(pAD7785 != NULL);
    
    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    buf2 = ntohs(&buf1);
    
    buf2 = (buf2 & ~FTE_AD7785_OP_MODE_MASK) | ((mode << FTE_AD7785_OP_MODE_SHIFT) & FTE_AD7785_OP_MODE_MASK);
        
    htons(&buf1, buf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    
    pAD7785->nLastValue = 0xFFFFFFFF;
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getClock(FTE_AD7785_PTR pAD7785, uint_32 *clk)
{
    uint_16 buf = 0;
    
    ASSERT(pAD7785 != NULL);
    
    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    *clk = (ntohs(&buf) & FTE_AD7785_CLK_MASK) >> FTE_AD7785_CLK_SHIFT;

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_setClock(FTE_AD7785_PTR pAD7785, uint_32 clk)
{
    uint_16 buf1 = 0, buf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    buf2 = ntohs(&buf1);
    
    buf2 = (buf2 & ~FTE_AD7785_CLK_MASK) | ((clk << FTE_AD7785_CLK_SHIFT) & FTE_AD7785_CLK_MASK);
        
    htons(&buf1, buf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getFreq(FTE_AD7785_PTR  pAD7785, uint_32 *freq)
{
    uint_16 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    *freq = (ntohs(&buf) & FTE_AD7785_FREQ_MASK) >> FTE_AD7785_FREQ_SHIFT;

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_setFreq(FTE_AD7785_PTR  pAD7785, uint_32 freq)
{
    uint_16 buf1 = 0, buf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    buf2 = ntohs(&buf1);
    
    buf2 = (buf2 & ~FTE_AD7785_FREQ_MASK) | ((freq << FTE_AD7785_FREQ_SHIFT) & FTE_AD7785_FREQ_MASK);
        
    htons(&buf1, buf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_MODE, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

uint_32     FTE_AD7785_measurementTime(FTE_AD7785_PTR pAD7785)
{
#if 0
    static  uint_32 pMeasureTime[] = 
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

_mqx_uint   FTE_AD7785_getConfig(FTE_AD7785_PTR  pAD7785, uint_32 *config)
{
    uint_16 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    *config = ntohs(&buf);

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_setConfig(FTE_AD7785_PTR  pAD7785, uint_32 config)
{
    uint_16 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    htons(&buf, config);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getVBias(FTE_AD7785_PTR  pAD7785, uint_32 *vbios)
{
    uint_16 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    *vbios = (ntohs(&buf) & FTE_AD7785_CONFIG_VBIOS_MASK) >> FTE_AD7785_CONFIG_VBIOS_SHIFT;

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_setVBias(FTE_AD7785_PTR  pAD7785, uint_32 vbios)
{
    uint_16 buf1 = 0, buf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    buf2 = ntohs(&buf1);
    
    buf2 = (buf2 & ~FTE_AD7785_CONFIG_VBIOS_MASK) | ((vbios << FTE_AD7785_CONFIG_VBIOS_SHIFT) & FTE_AD7785_CONFIG_VBIOS_MASK);
        
    htons(&buf1, buf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getGain(FTE_AD7785_PTR pAD7785, uint_32 *gain)
{
    uint_16 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    *gain = (ntohs(&buf) & FTE_AD7785_CONFIG_GAIN_MASK) >> FTE_AD7785_CONFIG_GAIN_SHIFT;

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_setGain(FTE_AD7785_PTR pAD7785, uint_32 gain)
{
    uint_16 buf1 = 0, buf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    buf2 = ntohs(&buf1);
    
    buf2 = (buf2 & ~FTE_AD7785_CONFIG_GAIN_MASK) | ((gain << FTE_AD7785_CONFIG_GAIN_SHIFT) & FTE_AD7785_CONFIG_GAIN_MASK);
        
    htons(&buf1, buf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getChannel(FTE_AD7785_PTR pAD7785, uint_32 *channel)
{
    uint_16 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf, 2) != MQX_OK)
    {
         goto error;
    }
    *channel = (ntohs(&buf) & FTE_AD7785_CONFIG_CHANNEL_MASK) >> FTE_AD7785_CONFIG_CHANNEL_SHIFT;

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_setChannel(FTE_AD7785_PTR pAD7785, uint_32 channel)
{
    uint_16 buf1 = 0, buf2 = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    buf2 = ntohs(&buf1);
    
    buf2 = (buf2 & ~FTE_AD7785_CONFIG_CHANNEL_MASK) | ((channel << FTE_AD7785_CONFIG_CHANNEL_SHIFT) & FTE_AD7785_CONFIG_CHANNEL_MASK);
        
    htons(&buf1, buf2);
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_CONFIG, (uint_8_ptr)&buf1, 2) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getIOut(FTE_AD7785_PTR pAD7785, uint_32 *iout)
{
    uint_8 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (uint_8_ptr)&buf, 1) != MQX_OK)
    {
         goto error;
    }
    *iout = (buf & FTE_AD7785_IOUT_DIR_MASK) >> FTE_AD7785_IOUT_DIR_SHIFT;

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}
_mqx_uint   FTE_AD7785_setIOut(FTE_AD7785_PTR pAD7785, uint_32 iout)
{
    uint_8 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (uint_8_ptr)&buf, 1) != MQX_OK)
    {
         goto error;
    }
    
    buf = (buf & ~FTE_AD7785_IOUT_DIR_MASK) | ((iout << FTE_AD7785_IOUT_DIR_SHIFT) & FTE_AD7785_IOUT_DIR_MASK);
        
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_IOUT, (uint_8_ptr)&buf, 1) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getCurrent(FTE_AD7785_PTR pAD7785, uint_32 *current)
{
    uint_8 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (uint_8_ptr)&buf, 1) != MQX_OK)
    {
         goto error;
    }
    *current = (buf & FTE_AD7785_IOUT_CURRENT_MASK) >> FTE_AD7785_IOUT_CURRENT_SHIFT;

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}
_mqx_uint   FTE_AD7785_setCurrent(FTE_AD7785_PTR pAD7785, uint_32 current)
{
    uint_8 buf = 0;
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_IOUT, (uint_8_ptr)&buf, 1) != MQX_OK)
    {
         goto error;
    }
    
    buf = (buf & ~FTE_AD7785_IOUT_CURRENT_MASK) | ((current << FTE_AD7785_IOUT_CURRENT_SHIFT) & FTE_AD7785_IOUT_CURRENT_MASK);
        
    if (_FTE_AD7785_setReg(pAD7785, FTE_AD7785_REG_IOUT, (uint_8_ptr)&buf, 1) != MQX_OK)
    {
         goto error;
    }
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_AD7785_getRawData(FTE_AD7785_PTR pAD7785, uint_32 *pValue)
{
    uint_32 nValue;
    uint_8 pBuf[3] = {0, };
    
    ASSERT(pAD7785 != NULL);

    if (_FTE_AD7785_getReg(pAD7785, FTE_AD7785_REG_DATA, pBuf, 3) != MQX_OK)
    {
        return  MQX_ERROR;
    }

    ((uint_8_ptr)&nValue)[0] = pBuf[2];
    ((uint_8_ptr)&nValue)[1] = pBuf[1];
    ((uint_8_ptr)&nValue)[2] = pBuf[0];
    ((uint_8_ptr)&nValue)[3] = 0;
    
    nValue >>= 4;
    
    *pValue = nValue;

    return  MQX_OK;
}

_mqx_uint   FTE_AD7785_getScaleData(FTE_AD7785_PTR pAD7785, uint_32 *pValue)
{
    _mqx_uint   nRet;
    
    if (pAD7785->nLastValue == 0xFFFFFFFF)
    {
        uint_32     nValue = 0;
        
        nRet = FTE_AD7785_getRawData(pAD7785, &nValue);
        if (nRet != MQX_OK)
        {
            return  nRet;
        }
        
        pAD7785->nLastValue = nValue;
    }
    
    *pValue = (uint_32)((float)pAD7785->nLastValue / 0x000FFFFF * 1170000) ;        
    
    return  MQX_OK;
}

/******************************************************************************
 * Internal Static Functions
 ******************************************************************************/
static _mqx_uint        _FTE_AD7785_init(FTE_AD7785_PTR pAD7785)
{
    uint_32 nValue, nNewValue;
        
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
    
    
    return  MQX_OK;
}


static _mqx_uint   _FTE_AD7785_getReg(FTE_AD7785_PTR pAD7785, uint_32 reg, uint_8 *data, uint_32 len)
{
    uint_8  cmd;
    
    assert((pAD7785 != NULL) && (data != NULL) && (len < 4));
    
    cmd = FTE_AD7785_CMD_BIT_READ | ((reg << FTE_AD7785_CMD_BIT_RS_SHIFT) & FTE_AD7785_CMD_BIT_RS_MASK);
    if (MQX_OK != FTE_SPI_read(pAD7785->pSPI, &cmd, 1, data, len))
    {
        goto error;
    }
    
    return  MQX_OK;
error:
    return  MQX_ERROR;

}


_mqx_uint   _FTE_AD7785_setReg(FTE_AD7785_PTR pAD7785, uint_32 reg, uint_8 *data, uint_32 len)
{
    uint_8  cmd;

    assert((pAD7785 != NULL) && (data != NULL) && (len < 4));
    
    cmd = FTE_AD7785_CMD_BIT_WRITE | ((reg << FTE_AD7785_CMD_BIT_RS_SHIFT) & FTE_AD7785_CMD_BIT_RS_MASK);
    if (MQX_OK != FTE_SPI_write(pAD7785->pSPI, &cmd, 1, data, len))
    {
        goto error;
    }
    
    return  MQX_OK;
error:
    return  MQX_ERROR;
}


int_32  FTE_AD7785_SHELL_cmd(int_32 argc, char_ptr argv[] )
{ 
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
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
                uint_32 nDID;
                
                
                if (!Shell_parse_hexnum(argv[1], &nDID))
                {
                    print_usage = TRUE;
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
                    uint_8 pRegValue[4] = {0,};
                    
                    if (_FTE_AD7785_getReg(pAD7785, i, pRegValue, nFieldSize[i]) != MQX_OK)
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
                    uint_32 nMode, nClk, nFreq, nData, nConfig, nVBias, nGain, nChannel, nIOut, nCurrent;
                    
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
                uint_32 nDID;
                
                if (!Shell_parse_hexnum(argv[1], &nDID))
                {
                    print_usage = TRUE;
                    break;
                }

                if (strcmp(argv[2], "start") == 0)
                {
                
                }
            }
            
        }
    }

error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s\n", argv[0]);
        }
        else
        {
            printf("Usage : %s\n", argv[0]);
        }
    }

    return   return_code;
}
        