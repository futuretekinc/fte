#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

FTE_RET   FTE_FTLM_switchCtrl(FTE_OBJECT_PTR pObj, FTE_BOOL bSwitchON);
FTE_RET   FTE_FTLM_countReset(FTE_OBJECT_PTR pObj);

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED0_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 0,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED1_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 1,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED2_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 2,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED3_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED3",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 3,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED4_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED4",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 4,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED5_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED5",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 5,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED6_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED6",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 6,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const
FTE_IFCE_CONFIG FTE_FTLM_LED7_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED7",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 7,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_FTLM_LED8_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0001),
        .pName      = "LED8",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 8,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

static const
FTE_OBJECT_CONFIG_PTR FTE_FTLM_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED0_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED1_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED2_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED3_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED4_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED5_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED6_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED7_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_LED8_defaultConfig
};

FTE_GUS_CONFIG FTE_FTLM_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
        .pName      = "FTLM",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
        .ulChild    = sizeof(FTE_FTLM_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_FTLM_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_FTLM,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_FTLM_DEFAULT_UPDATE_INTERVAL
};

FTE_VALUE_TYPE  FTE_FTLM_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
};

const 
FTE_GUS_MODEL_INFO    FTE_FTLM_GUSModelInfo = 
{
    .nModel         = FTE_GUS_MODEL_FTLM,
    .pName          = "FTLM",
    .nFieldCount    = 9,
    .pValueTypes    = FTE_FTLM_valueTypes,
    .fRequest      = FTE_FTLM_requestData,
    .fReceived     = FTE_FTLM_receiveData,
    .fSet          = FTE_FTLM_set,
};


FTE_RET   FTE_FTLM_requestData
(
    FTE_OBJECT_PTR  pObj
)
{
    /*
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    FTE_UINT8  pCMD[] = { 0x3a, 0x01, 0x03, 0x00, 0x65, 0x00, 0x1b, 0x00, 0x00, 0x0d, 0x0a, 0x00};
    FTE_UINT16 uiCRC;
    
    pCMD[1] = (FTE_UINT8)pConfig->nSensorID;
    uiCRC = FTE_CRC16(&pCMD[1], 6);
    pCMD[7] = (uiCRC     ) & 0xFF;
    pCMD[8] = (uiCRC >> 8) & 0xFF;

    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), TRUE);    
*/
    return  FTE_RET_OK; 
}
 
FTE_RET   FTE_FTLM_receiveData
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_UINT8      pBuff[128];
    FTE_UINT32     nLen;
    FTE_UINT16     uiCRC;
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if ((nLen < 4) || (pBuff[0] != 0x3a))// || (pBuff[nLen - 2] != 0x0d) || (pBuff[nLen - 1] != 0x0a) || (nLen != 4 + pBuff[3] + 4))
    {
        return  FTE_RET_ERROR;
    }
    
    if ((pBuff[1] < 1) || (pBuff[1] > 9))
    {
        return  FTE_RET_ERROR;
    }
    
    uiCRC = FTE_CRC16(&pBuff[1], 3 + pBuff[3]);
    if (uiCRC != (pBuff[4 + pBuff[3]] | ((FTE_UINT16)pBuff[4 + pBuff[3] + 1] << 8)))
    {
        return  FTE_RET_ERROR;
    } 
  
    FTE_UINT32 ulValue = 0;
    
    ulValue = ((FTE_UINT32)pBuff[4 + 1]) | ((FTE_UINT32)pBuff[4 + 3] << 8) | ((FTE_UINT32)pBuff[4 + 5] << 16) ;
    
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[pBuff[1]], ulValue);

    return  FTE_RET_OK;
}

FTE_RET   FTE_FTLM_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32 nIndex, 
    FTE_VALUE_PTR pValue
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_UINT8      pCMD[] = { 0x3a, 0x01, 0x10, 0x00, 0x65, 0x00, 0x3, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x00};
    FTE_UINT8      pBuff[128];
    FTE_UINT32     nLen;
    FTE_UINT16     uiCRC;
    FTE_UINT32     ulValue;
    FTE_UINT32     ulTry = 0;
    if (pStatus->xCommon.nValueCount <= nIndex)
    {
        return  FTE_RET_ERROR;
    }
    
    memset(pBuff, 0, sizeof(pBuff));
   
    FTE_VALUE_getULONG(pValue, &ulValue);
    pCMD[1] = (FTE_UINT8)nIndex+1;
    pCMD[4] = (FTE_UINT8)101;
    pCMD[9] = (FTE_UINT8)(ulValue & 0xFF);
    pCMD[11]= (FTE_UINT8)((ulValue >> 8) & 0xFF);
    pCMD[13]= (FTE_UINT8)((ulValue >> 16) & 0xFF);
    uiCRC = FTE_CRC16(&pCMD[1], 13);
    pCMD[14] = (uiCRC     ) & 0xFF;
    pCMD[15] = (uiCRC >> 8) & 0xFF;

    do
    {
        do 
        {
            ulTry++;
            FTE_UCS_clear(pStatus->pUCS);
            nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 100, 500);
        } 
        while ((ulTry < 3) && ((nLen < 4) || (pBuff[0] != 0x3a) || (pBuff[1] != (nIndex+1))));
        
        uiCRC = FTE_CRC16(&pBuff[1], 6);
    } 
    while((ulTry < 3) && (uiCRC != (pBuff[7] | ((FTE_UINT16)pBuff[8] << 8))));
    
    if (ulTry >= 3)
    {
        DEBUG("Request Timeout[Retry = %d]\n", ulTry);
        return  FTE_RET_ERROR;
    }

    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[nIndex], ulValue);
    pStatus->xCommon.pValue[nIndex].bChanged = TRUE;
    return  FTE_RET_OK;
}



