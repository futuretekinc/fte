#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"

#if FTE_GS_DPC_HL_SUPPORTED


#define FTE_GS_DPC_HL_FRAME_FIELD_VERION            0
#define FTE_GS_DPC_HL_FRAME_FIELD_MODEL1            1
#define FTE_GS_DPC_HL_FRAME_FIELD_MODEL2            2
#define FTE_GS_DPC_HL_FRAME_FIELD_RELAY_STATE       3
#define FTE_GS_DPC_HL_FRAME_FIELD_ERROR             4
#define FTE_GS_DPC_HL_FRAME_FIELD_HP                5
#define FTE_GS_DPC_HL_FRAME_FIELD_LP                6
#define FTE_GS_DPC_HL_FRAME_FIELD_HP_TEMP           9
#define FTE_GS_DPC_HL_FRAME_FIELD_LP_TEMP           10

typedef struct
{
    FTE_UINT16 usVersion;
    FTE_UINT16 usModel;
    FTE_UINT16 usID;
    FTE_UINT16 usRelay;
    FTE_UINT16 usError;
    int_16  nHP;
    int_16  nLP;
    FTE_UINT16 usReserved[2];
    int_16  sHPTemp;
    int_16  sLPTemp;
}   FTE_GS_DPC_HL_FRAME, _PTR_ FTE_GS_DPC_HL_FRAME_PTR;



static const FTE_IFCE_CONFIG FTE_GS_DPC_HPS_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "HPS",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HPS_STATE,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_LPS_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "LPS",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LPS_STATE,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_HPC_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "HPC",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HPC_STATE,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_HPS_ERROR_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "HPS ERROR",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HPS_ERROR,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_LPS_ERROR_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "LPS ERROR",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LPS_ERROR,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_HP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_PRESSURE, 0),
        .pName      = "HP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HP,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_LP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_PRESSURE, 0),
        .pName      = "LP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LP,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_HP_TEMP_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "HP TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HP_TEMP,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_IFCE_CONFIG FTE_GS_DPC_LP_TEMP_defaultConfig=
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "lP TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LP_TEMP,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

static const FTE_OBJECT_CONFIG_PTR FTE_GS_DPC_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_HPS_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_LPS_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_HPC_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_HPS_ERROR_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_LPS_ERROR_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_HP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_LP_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_HP_TEMPERATURE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_LP_TEMPERATURE_defaultConfig
};

FTE_GUS_CONFIG FTE_GS_DPC_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
        .pName      = "DPC-HL",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
        .ulChild    = sizeof(FTE_GS_DPC_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_GS_DPC_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_GS_DPC_HL,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL
};

FTE_VALUE_TYPE  FTE_GS_DPC_HL_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       
    FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_PRESSURE,       FTE_VALUE_TYPE_PRESSURE,       FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE
};


const FTE_GUS_MODEL_INFO    FTE_GS_DPC_GUSModelInfo = 
{
    .nModel     = FTE_GUS_MODEL_GS_DPC_HL,
    .pName      = "DPC-HL",
    .nFieldCount= FTE_GS_DPC_HL_INDEX_MAX+1,
    .pValueTypes= FTE_GS_DPC_HL_valueTypes,
    .f_request  = FTE_GS_DPC_HL_request,
    .f_received = FTE_GS_DPC_HL_received,
    .f_set      = FTE_GS_DPC_HL_set
};

FTE_RET   FTE_GS_DPC_HL_request
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    FTE_UINT8  pCMD[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x0f, 0x00};
    
    pCMD[0] = (FTE_UINT8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), FALSE);    

    return  MQX_OK;
}

FTE_RET   FTE_GS_DPC_HL_received
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR      pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GS_DPC_HL_FRAME_PTR pFrame = NULL;
    FTE_UINT8      pBuff[128];
    FTE_UINT32     nLen;
    FTE_UINT16     uiCRC;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 31)
    {
        return  FTE_RET_ERROR; 
    }

    if ((pBuff[0] != 0x01))
    {
        return  FTE_RET_ERROR;
    }
    
    uiCRC = FTE_CRC16(pBuff, 29);
    if (uiCRC != (pBuff[29] | ((FTE_UINT16)pBuff[30] << 8)))
    {
        return  FTE_RET_ERROR;
    }
    
    pFrame = (FTE_GS_DPC_HL_FRAME_PTR)&pBuff[3];
    // Temperature
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_VERSION],       ntohs(&pFrame->usVersion));
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_MODEL0],        pFrame->usModel);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_ID],            ntohs(&pFrame->usID));       
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HPS_STATE],       (pFrame->usRelay & 0x01));
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LPS_STATE],       (pFrame->usRelay >> 1) & 0x01);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HPC_STATE],       (pFrame->usRelay >> 2) & 0x01);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HPS_ERROR],       (pFrame->usError & 0x01));
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LPS_ERROR],       (pFrame->usError >> 1) & 0x01);
    FTE_VALUE_setPressure(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HP],         (short)ntohs(&pFrame->nHP));
    FTE_VALUE_setPressure(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LP],         (short)ntohs(&pFrame->nLP));    
    FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HP_TEMP], (short)ntohs(&pFrame->sHPTemp) * 10);
    FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LP_TEMP], (short)ntohs(&pFrame->sLPTemp) * 10);    
    
    return  MQX_OK;
}

FTE_RET   FTE_GS_DPC_HL_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nIndex, 
    FTE_VALUE_PTR   pValue
)
{
//    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    return  FTE_RET_ERROR;
}

#endif
