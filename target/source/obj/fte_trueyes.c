#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
 

#if FTE_TRUEYES_AIRQ_SUPPORTED

static const FTE_IFCE_CONFIG FTE_TRUEYES_AIRQ_TEMPERATURE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0x0001),
        .pName      = "AIRQ-TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(RQ, 0x0001),
    .nRegID     = 1,
    .nInterval  = FTE_TRUEYES_AIRQ_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TRUEYES_AIRQ_HUMIDITY_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0x0001),
        .pName      = "AIRQ-HUMI",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(RQ, 0x0001),
    .nRegID     = 2,
    .nInterval  = FTE_TRUEYES_AIRQ_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_TRUEYES_AIRQ_CO2_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CO2, 0x0001),
        .pName      = "AIRQ-CO2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(RQ, 0x0001),
    .nRegID     = 0,
    .nInterval  = FTE_TRUEYES_AIRQ_DEFAULT_UPDATE_INTERVAL
};

static const FTE_OBJECT_CONFIG_PTR FTE_TRUEYES_AIRQ_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_TRUEYES_AIRQ_TEMPERATURE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TRUEYES_AIRQ_HUMIDITY_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TRUEYES_AIRQ_CO2_defaultConfig
};

FTE_GUS_CONFIG FTE_TRUEYES_AIRQ_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(RQ, 0x0001),
        .pName      = "AIRQ",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
        .ulChild    = sizeof(FTE_TRUEYES_AIRQ_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_TRUEYES_AIRQ_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_TRUEYES_AIRQ, 
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_TRUEYES_AIRQ_INTERVAL
};

FTE_VALUE_TYPE  FTE_TRUEYES_AIRQ_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_HUMIDITY,
    FTE_VALUE_TYPE_DIO
};


const FTE_GUS_MODEL_INFO    FTE_TRUEYES_AIRQ_GUSModelInfo = 
{
    .nModel         = FTE_GUS_MODEL_TRUEYES_AIRQ,
    .pName          = "TRUEYES AIRQ",
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_TRUEYES_AIRQ_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_TRUEYES_AIRQ_DEFAULT_DATABITS,
        .nParity    =   FTE_TRUEYES_AIRQ_DEFAULT_PARITY,
        .nStopBits  =   FTE_TRUEYES_AIRQ_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_TRUEYES_AIRQ_DEFAULT_FULL_DUPLEX
    },
    .nFieldCount    = 4,
    .pValueTypes   = FTE_TRUEYES_AIRQ_valueTypes,
    .f_request      = FTE_AIRQ_request,
    .f_received     = FTE_AIRQ_received
};

FTE_RET   FTE_AIRQ_request(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    uint_8  pCMD[] = { 0x00, 0x03, 0x00, 0x65, 0x00, 0x05, 0x95, 0xd6, 0x00};
    
    pCMD[0] = (uint_8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), FALSE);    

    return  MQX_OK;
}

FTE_RET   FTE_AIRQ_received(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_32     nCO2    = 0;
    uint_32     nTEMP   = 0;
    uint_32     nHUMI   = 0;
    uint_32     nCO     = 0;
    uint_8      pBuff[64];
    uint_32     nLen;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 15)
    {
        return  MQX_ERROR;
    }

    if ((pBuff[0] != 0x01))
    {
        return  MQX_ERROR;
    }
    
    nCO2    = (uint_16)pBuff[3] << 8 | pBuff[4];
    nTEMP   = (uint_16)pBuff[5] << 8 | pBuff[6];
    nHUMI   = (uint_16)pBuff[7] << 8 | pBuff[8];
    nCO     = (uint_16)pBuff[9] << 8 | pBuff[10];

    FTE_VALUE_setPPM(&pStatus->xCommon.pValue[0], nCO2);
    FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[1], nTEMP * 10);
    FTE_VALUE_setHumidity(&pStatus->xCommon.pValue[2], nHUMI * 10);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[3], nCO);

    pStatus->xCommon.xFlags = FTE_FLAG_SET(pStatus->xCommon.xFlags, FTE_OBJ_STATUS_FLAG_VALID);
    
    return  MQX_OK;
}

#endif