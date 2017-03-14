#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "fte_tascon.h"
#include "nxjson.h"

#if FTE_TASCON_HEM12_SUPPORTED || FTE_TASCON_HEM12_06M_SUPPORTED

#define FTE_TASCON_PACKET_DEBUG 1

#define FTE_HEM12_FRAME_START               0x69
#define FTE_HEM12_START_CODE                0x69
#define FTE_HEM12_STOP_CODE                 0x96

#define FTE_HEM12_CMD_REQ_READ              0x78
#define FTE_HEM12_CMD_RESP_READ             0xF8

#define FTE_HEM12_RESP_VOLTAGE_FRAME_LEN    28
#define FTE_HEM12_RESP_CURRENT_FRAME_LEN    29
#define FTE_HEM12_RESP_POWER_FRAME_LEN      36

#define FTE_HEM12_RESP_DELAY_TIME           100
#define FTE_HEM12_RESP_WAIT_TIME            1000

typedef struct  FTE_TASCON_DEVICE_STRUCT
{
    _task_id        xTaskID;
    FTE_OBJECT_PTR  pObj;
}   FTE_TASCON_DEVICE, _PTR_ FTE_TASCON_DEVICE_PTR;


#if FTE_TASCON_PACKET_DEBUG
static FTE_BOOL  bDebugON = 0;
#endif

static const 
FTE_IFCE_CONFIG    FTE_TASCON_HEM12_PWR_defaultConfig=
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_POWER, 0x0000),
        .pName      = "POWER(kWh)",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12, 0x0000),
    .nRegID     = 0,
    .nInterval  = FTE_TASCON_HEM12_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_OBJECT_CONFIG_PTR FTE_TASCON_HEM12_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_PWR_defaultConfig,
};

FTE_TASCON_HEM12_CONFIG FTE_TASCON_HEM12_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12, 0),
        .pName      = "HEM12",
        .xFlags     = 0, 
        .ulChild    = sizeof(FTE_TASCON_HEM12_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_TASCON_HEM12_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_TASCON_HEM12_DEFAULT_UPDATE_INTERVAL,
    .pSensorID  = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
};

static const 
FTE_IFCE_CONFIG    FTE_TASCON_HEM12_06M_PWR_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_POWER, 0x0000),
        .pName      = "POWER(kWh)",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0x0000),
    .nRegID     = 0,
    .nInterval  = FTE_TASCON_HEM12_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG    FTE_TASCON_HEM12_06M_VOLTAGE_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VOLTAGE, 0x0000),
        .pName      = "VOLTAGE(V)",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0x0000),
    .nRegID     = 2,
    .nInterval  = FTE_TASCON_HEM12_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG    FTE_TASCON_HEM12_06M_CURRENT_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CURRENT, 0x0000),
        .pName      = "CURRENT(A)",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0x0000),
    .nRegID     = 3,
    .nInterval  = FTE_TASCON_HEM12_DEFAULT_UPDATE_INTERVAL
};


static const 
FTE_OBJECT_CONFIG_PTR FTE_TASCON_HEM12_06M_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_06M_PWR_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_06M_VOLTAGE_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_06M_CURRENT_defaultConfig,
};

FTE_TASCON_HEM12_CONFIG FTE_TASCON_HEM12_06M_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0),
        .pName      = "AGU-HA",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
        .ulChild    = sizeof(FTE_TASCON_HEM12_06M_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_TASCON_HEM12_06M_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12_06M,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_TASCON_HEM12_DEFAULT_UPDATE_INTERVAL,
    .pSensorID  = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
};

const 
FTE_GUS_MODEL_INFO  FTE_TASCON_HEM12_GUSModelInfo = 
{
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12,
    .pName      = "TASCON HEM12",
    .xFlags     = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_TASCON_HEM12_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_TASCON_HEM12_DEFAULT_DATABITS,
        .nParity    =   FTE_TASCON_HEM12_DEFAULT_PARITY,
        .nStopBits  =   FTE_TASCON_HEM12_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_TASCON_HEM12_DEFAULT_FULL_DUPLEX
    },
    .nFieldCount= 1,
    .pValueTypes= FTE_TASCON_HEM12_valueTypes,
    .fAttach   = FTE_TASCON_HEM12_attach,
    .fDetach   = FTE_TASCON_HEM12_detach,
    .fGet      = FTE_TASCON_HEM12_get,
    .fRequest  = FTE_TASCON_HEM12_request,
    .fReceived = FTE_TASCON_HEM12_received,
    .fSetConfig = FTE_TASCON_HEM12_06M_setConfig,
    .fGetConfig = FTE_TASCON_HEM12_06M_getConfig,
};

const FTE_GUS_MODEL_INFO FTE_TASCON_HEM12_06M_GUSModelInfo = 
{
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12_06M,
    .pName      = "TASCON HEM12-06M",
    .xFlags     = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_TASCON_HEM12_06M_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_TASCON_HEM12_06M_DEFAULT_DATABITS,
        .nParity    =   FTE_TASCON_HEM12_06M_DEFAULT_PARITY,
        .nStopBits  =   FTE_TASCON_HEM12_06M_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_TASCON_HEM12_06M_DEFAULT_FULL_DUPLEX
    },
    .nFieldCount= 4,
    .pValueTypes= FTE_TASCON_HEM12_06M_valueTypes,
    .fAttach   = FTE_TASCON_HEM12_attach,
    .fDetach   = FTE_TASCON_HEM12_detach,
    .fGet      = FTE_TASCON_HEM12_get,
    .fRequest  = FTE_TASCON_HEM12_06M_request,
    .fReceived = FTE_TASCON_HEM12_06M_received,
    .fSetConfig = FTE_TASCON_HEM12_06M_setConfig,
    .fGetConfig = FTE_TASCON_HEM12_06M_getConfig,
};


static 
FTE_UINT32           ulDeviceCount=0;

static 
FTE_TASCON_DEVICE    pDevices[FTE_TASCON_MAX];

FTE_RET FTE_TASCON_HEM12_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
    FTE_INT32 i;
    FTE_TASCON_DEVICE_PTR   pDevice = NULL;
    
    for(i = 0 ; i < FTE_TASCON_MAX ; i++)
    {
        if (pDevices[i].pObj == NULL)
        {   
            pDevice = &pDevices[i];
            break;
        }
    }
    
    if (pDevice == NULL)
    {
        return  FTE_RET_OBJECT_FULL;
    }
    
//    xRet =  FTE_TASK_create(FTE_TASK_TASCON, (FTE_UINT32)pObj->pConfig->xCommon.nID, &pDevice->xTaskID);
    //if (xRet != FTE_RET_OK)
//    {
//        return  xRet;
//    }             
                
    pDevice->pObj = pObj;    
    ulDeviceCount++;
                
    return  FTE_RET_OK;
}


FTE_RET   FTE_TASCON_HEM12_attach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_RET xRet;
    FTE_TASCON_HEM12_CONFIG_PTR pConfig;
    FTE_TASCON_HEM12_STATUS_PTR pStatus;
    FTE_UCS_PTR  pUCS;
    
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL) && (pObj->pStatus != NULL));

    pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
    pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;    
    
    pUCS = (FTE_UCS_PTR)FTE_UCS_get(pConfig->nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->nUCSID);
        return  FTE_RET_OBJECT_NOT_FOUND;
    }

    xRet = FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    FTE_UCS_setUART(pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
    pStatus->xGUS.pUCS = pUCS;
    
    FTE_TASCON_HEM12_init(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_TASCON_HEM12_detach
(
    FTE_OBJECT_PTR pObj
)
{
    FTE_TASCON_HEM12_STATUS_PTR  pStatus;
	FTE_TASCON_HEM12_CONFIG_PTR  pConfig;
    ASSERT((pObj != NULL) && (pObj->pStatus != NULL));

    pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;
	pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
	
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
	
	memset(pConfig->pSensorID, 0xAA , sizeof(pConfig->pSensorID)); 
    
    return  FTE_RET_OK;
}


FTE_RET FTE_TASCON_HEM12_get
(
    FTE_OBJECT_PTR  pObject, 
    FTE_UINT32         ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    FTE_TASCON_HEM12_STATUS_PTR pStatus;
    ASSERT ((pObject != NULL) && (pValue != NULL));

    pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObject->pStatus;
    
    if (ulIndex >= pStatus->xGUS.xCommon.nValueCount)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_VALUE_copy(pValue, &pStatus->xGUS.xCommon.pValue[ulIndex]);
}

static 
FTE_UINT8  FTE_TASCON_HEM12_CRC(FTE_UINT8_PTR pData, FTE_UINT32 ulDataLen)
{
    FTE_UINT8  uiCS = 0;
    
    for(FTE_INT32 i = 0 ; i < ulDataLen ; i++)
    {
        uiCS += pData[i];
    }
    
    return  uiCS;
}

static 
FTE_BOOL FTE_TASCON_HEM12_06M_isValidFrame
(
    FTE_UINT8_PTR   pFrame, 
    FTE_UINT32      ulLen
)
{
    if ((ulLen < 12) || (ulLen != (pFrame[9] + 12)) || (pFrame[0] != FTE_HEM12_FRAME_START) || (pFrame[ulLen - 1] != FTE_HEM12_STOP_CODE))
    {
        return  FALSE;
    }    
        
    if (FTE_TASCON_HEM12_CRC(pFrame, ulLen - 2) != pFrame[ulLen - 2])
    {
        return  FALSE;
    }
    
    return  TRUE;
}

static 
FTE_RET  FTE_TASCON_HEM12_06M_setAddress
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pAddress, 
    FTE_UINT32      ulLen
)
{
    FTE_INT32  i;
    FTE_UINT8  pSensorID[6];
    FTE_TASCON_HEM12_CONFIG_PTR  pConfig;

    if ((pObj == NULL) || (ulLen != 12))
    {
        return  FTE_RET_ERROR;
    }
    
    pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;    
    
    memset(pSensorID, 0, sizeof(pSensorID));
    
    for(i = 0 ; i < 12 ; i++)
    {
        FTE_UINT8  nValue;
        
        if ('0' <= pAddress[i] && pAddress[i] <= '9')
        {
            nValue = pAddress[i] - '0';
        } 
        else if ('A' <= pAddress[i] && pAddress[i] <= 'F')
        {
            nValue = pAddress[i] - 'A' + 10;
        }
        else if ('a' <= pAddress[i] && pAddress[i] <= 'f')
        {
            nValue = pAddress[i] - 'a' + 10;
        }
        else
        {
            return  FTE_RET_ERROR;
        }
        
        if (i & 0x01)
        {
            pSensorID[i >> 1] |= (nValue & 0x0F);
        }
        else
        {
            pSensorID[i >> 1] |= (nValue & 0x0F) << 4;
        }
    }
    
    memcpy(pConfig->pSensorID, pSensorID, 6);
    
    return  FTE_RET_OK;
}

FTE_VALUE_TYPE  FTE_TASCON_HEM12_valueTypes[] =
{
    FTE_VALUE_TYPE_PWR_KW
};

FTE_RET   FTE_TASCON_HEM12_FRAME_create
(
    FTE_UINT8_PTR   pAddress, 
    FTE_UINT32      ulType, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulBuffLen, 
    FTE_UINT32_PTR  pulFrameSize
)
{
    switch(ulType)
    {
    case    0:
        {
            FTE_UINT8  ucCRC;
            const FTE_UINT8  pBaseFrame[] = {  FTE_HEM12_FRAME_START, 
                                            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
                                            FTE_HEM12_START_CODE,
                                            0x01, 0x02, 0x47, 0x14,
                                            0xFF, 
                                            FTE_HEM12_STOP_CODE, 0x00 };

            memcpy(pBuff, pBaseFrame, sizeof(pBaseFrame));
            *pulFrameSize = sizeof(pBaseFrame);    
            
            pBuff[1] = pAddress[5];
            pBuff[2] = pAddress[4];
            pBuff[3] = pAddress[3];
            pBuff[4] = pAddress[2];
            pBuff[5] = pAddress[1];
            pBuff[6] = pAddress[0];
                        
            ucCRC = FTE_TASCON_HEM12_CRC(pBuff, 12);
            pBuff[12] = ucCRC;
        }
        break;
        
    case    1:
        {
            const FTE_UINT8  pBaseFrame[] = {  FTE_HEM12_FRAME_START, 
                                            0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 
                                            FTE_HEM12_START_CODE,
                                            0x01, 0x02, 0x67, 0xf3,
                                            0xC5, 
                                            FTE_HEM12_STOP_CODE, 0x00 };

            memcpy(pBuff, pBaseFrame, sizeof(pBaseFrame));
            *pulFrameSize = sizeof(pBaseFrame);    
        }
        break;
    default:
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
} 


FTE_RET   FTE_TASCON_HEM12_request
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_TASCON_HEM12_CONFIG_PTR    pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
    FTE_TASCON_HEM12_STATUS_PTR    pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;
    FTE_UINT8      pReqBuff[32];
    FTE_UINT32     ulReqLen;
    FTE_UINT8      pRcvdBuff[32];
    FTE_UINT32     ulRcvdLen;
    FTE_UINT8      nCS = 0;
    FTE_CHAR_PTR    pHead;
     FTE_UINT32    nPower, nSkip = 0, nValue;    

    FTE_UCS_setBaudrate(pStatus->xGUS.pUCS, FTE_TASCON_HEM12_DEFAULT_BAUDRATE);
        
    pStatus->xGUS.xRet = FTE_RET_OK;
    
    FTE_TASCON_HEM12_FRAME_create(pConfig->pSensorID, 0, pReqBuff, sizeof(pReqBuff), &ulReqLen);    
        
    FTE_UCS_clear(pStatus->xGUS.pUCS);    
    
	FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig); 
    ulRcvdLen = FTE_UCS_sendAndRecv(pStatus->xGUS.pUCS, pReqBuff, ulReqLen, pRcvdBuff, sizeof(pRcvdBuff), FTE_HEM12_RESP_DELAY_TIME, FTE_HEM12_RESP_WAIT_TIME);            
    if (ulRcvdLen == 0)
    {
        FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
        pStatus->xGUS.xRet = FTE_RET_ERROR;

        return  FTE_RET_ERROR;
    }
    
    pHead = (FTE_CHAR_PTR)pRcvdBuff;
    for(FTE_INT32 i = 0 ; i < ulRcvdLen ; i++)
    {
        if (pHead[i] == 0xFE)
        {
            nSkip++;
        }
        else
        {
            break;
        }
    }
    
    pHead += nSkip;
    ulRcvdLen -= nSkip;
    
    if ((ulRcvdLen < 14) || 
        (pHead[0] != FTE_HEM12_FRAME_START) || 
        (pHead[7] != FTE_HEM12_START_CODE) || 
        (ulRcvdLen < (12 + pHead[9])))
    {
        FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
        pStatus->xGUS.xRet = FTE_RET_ERROR;
        
        return  FTE_RET_INVALID_CHECKSUM;
    }
    
    ulRcvdLen = 12 + pHead[9];

    for(FTE_INT32 i = 0 ; i < ulRcvdLen - 2 ; i++)
    {
        nCS += pHead[i];
    }
    
    if ((pHead[ulRcvdLen - 2] != nCS) ||
        (pHead[ulRcvdLen - 1] != FTE_HEM12_STOP_CODE))
    {
        FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
        pStatus->xGUS.xRet = FTE_RET_ERROR;

        return  FTE_RET_INVALID_CHECKSUM;
    }
    
#if 0
    for(FTE_INT32 i = 0 ; i < 6 ; i++)
    {
        if (pConfig->pAddress[i] != pHead[6 - i])
        {
            FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
            pStatus->xGUS.xRet = FTE_RET_ERROR;

            return  FTE_RET_INVALID_PARAMETER;
        }
    }
#endif
    
    nValue = (pHead[15] - 0x33);
    nPower = ((nValue >> 4) * 10) + (nValue & 0x0F);
    nValue = (pHead[14] - 0x33);
    nPower = nPower * 100 + ((nValue >> 4) * 10) + (nValue & 0x0F);
    nValue = (pHead[13] - 0x33);
    nPower = nPower * 100 + ((nValue >> 4) * 10) + (nValue & 0x0F);
    nValue = (pHead[12] - 0x33);
    nPower = nPower * 100 + ((nValue >> 4) * 10) + (nValue & 0x0F);
   
    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[0], nPower * 10);
    
    return  FTE_RET_OK;
}

FTE_RET     FTE_TASCON_HEM12_received
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_TASCON_HEM12_STATUS_PTR  pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;
#if 0
    FTE_UINT32             nPower, nSkip = 0, nValue;    
    FTE_UINT8              pBuff[64];
    FTE_CHAR_PTR            pHead;
    FTE_UINT32             nLen;
    FTE_UINT8              nCS = 0;
    
    nLen = FTE_UCS_recv(pStatus->xGUS.pUCS, pBuff, sizeof(pBuff));
    if (nLen == 0)
    {
        return  FTE_RET_ERROR;
    }
    
    pHead = (FTE_CHAR_PTR)pBuff;
    
    
    for(FTE_INT32 i = 0 ; i < nLen ; i++)
    {
        if (pHead[i] == 0xFE)
        {
            nSkip++;
        }
        else
        {
            break;
        }
    }
    
    pHead += nSkip;
    nLen -= nSkip;
    
    if ((nLen < 14) || 
        (pHead[0] != FTE_HEM12_FRAME_START) || 
        (pHead[7] != FTE_HEM12_START_CODE) || 
        (nLen < (12 + pHead[9])))
    {
        return  FTE_RET_INVALID_CHECKSUM;
    }
    
    nLen = 12 + pHead[9];

    for(FTE_INT32 i = 0 ; i < nLen - 2 ; i++)
    {
        nCS += pHead[i];
    }
    
    if ((pHead[nLen - 2] != nCS) ||
        (pHead[nLen - 1] != FTE_HEM12_STOP_CODE))
    {
        return  FTE_RET_INVALID_CHECKSUM;
    }
    
#if 0
    for(FTE_INT32 i = 0 ; i < 6 ; i++)
    {
        if (pConfig->pAddress[i] != pHead[6 - i])
        {
            return  FTE_RET_INVALID_PARAMETER;
        }
    }
#endif
    
    nValue = (pHead[15] - 0x33);
    nPower = ((nValue >> 4) * 10) + (nValue & 0x0F);
    nValue = (pHead[14] - 0x33);
    nPower = nPower * 100 + ((nValue >> 4) * 10) + (nValue & 0x0F);
    nValue = (pHead[13] - 0x33);
    nPower = nPower * 100 + ((nValue >> 4) * 10) + (nValue & 0x0F);
    nValue = (pHead[12] - 0x33);
    nPower = nPower * 100 + ((nValue >> 4) * 10) + (nValue & 0x0F);
   
    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[0], nPower * 10);
#endif
    return  pStatus->xGUS.xRet;
}




FTE_VALUE_TYPE  FTE_TASCON_HEM12_06M_valueTypes[] =
{
    FTE_VALUE_TYPE_PWR_KWH,
    FTE_VALUE_TYPE_PWR_KW,
    FTE_VALUE_TYPE_VOLTAGE,
    FTE_VALUE_TYPE_CURRENT
};

FTE_RET     FTE_TASCON_HEM12_06M_FRAME_responsePower
(
    FTE_UINT8_PTR   pFrame, 
    FTE_UINT32      ulLen, 
    FTE_UINT32_PTR  pulAmountOfPower, 
    FTE_UINT32_PTR  pulPower
)
{
    FTE_UINT32     ulValue;
    FTE_UINT32     ulAmountOfPower;
    FTE_UINT32     ulPower;
    
    if ( !FTE_TASCON_HEM12_06M_isValidFrame(pFrame, ulLen) )
    {
        return  FTE_RET_INVALID_CHECKSUM;
    }
    
    ulValue = (pFrame[25] - 0x66);
    ulAmountOfPower = ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[24] - 0x66);
    ulAmountOfPower = ulAmountOfPower * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[23] - 0x66);
    ulAmountOfPower = ulAmountOfPower * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[22] - 0x66);
    ulAmountOfPower = ulAmountOfPower * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
  
    *pulAmountOfPower = ulAmountOfPower * 10;
    
    ulValue = (pFrame[28] - 0x66);
    ulPower = ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[27] - 0x66);
    ulPower = ulPower * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[26] - 0x66);
    ulPower = ulPower * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
   
    *pulPower = ulPower / 10;
    
    return  FTE_RET_OK;
}

FTE_RET     FTE_TASCON_HEM12_06M_FRAME_responseVoltage
(
    FTE_UINT8_PTR   pFrame, 
    FTE_UINT32      ulLen, 
    FTE_UINT32_PTR  pulVoltage
)
{
    FTE_UINT32     ulValue;
    FTE_UINT32     ulVoltage;

    if ( !FTE_TASCON_HEM12_06M_isValidFrame(pFrame, ulLen) )
    {
        return  FTE_RET_INVALID_CHECKSUM;
    }
    
    ulValue = (pFrame[23] - 0x66);
    ulVoltage = ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[22] - 0x66);
    ulVoltage = ulVoltage * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
  
    *pulVoltage = ulVoltage * 100;
    
    return  FTE_RET_OK;
}

FTE_RET     FTE_TASCON_HEM12_06M_FRAME_responseCurrent
(
    FTE_UINT8_PTR   pFrame, 
    FTE_UINT32      ulLen, 
    FTE_UINT32_PTR  pulCurrent
)
{
    FTE_UINT32     ulValue;
    FTE_UINT32     ulCurrent;
     
    if ( !FTE_TASCON_HEM12_06M_isValidFrame(pFrame, ulLen) )
    {
        return  FTE_RET_INVALID_CHECKSUM;
    }

    ulValue = (pFrame[24] - 0x66);
    ulCurrent = ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[23] - 0x66);
    ulCurrent = ulCurrent * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[22] - 0x66);
    ulCurrent = ulCurrent * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
  
    *pulCurrent = ulCurrent;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_TASCON_HEM12_06M_FRAME_create
(
    FTE_UINT8_PTR   pAddress, 
    FTE_UINT32      ulType, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulBuffLen, 
    FTE_UINT32_PTR  pulFrameSize
)
{
    FTE_UINT8  ucCRC;

    const FTE_UINT8  pBaseFrame[] = {  FTE_HEM12_FRAME_START, 
                                    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
                                    FTE_HEM12_START_CODE,
                                    FTE_HEM12_CMD_REQ_READ, 
                                    0x0e, 0x9c, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                                    0xdd, 0x9c, 0x34, 0x35, 0x00, 0x00, 0x00, 0x00,
                                    0xFF, 
                                    FTE_HEM12_STOP_CODE, 0x00 };

    memcpy(pBuff, pBaseFrame, sizeof(pBaseFrame));
    *pulFrameSize = sizeof(pBaseFrame);
    
    pBuff[1] = pAddress[5];
    pBuff[2] = pAddress[4];
    pBuff[3] = pAddress[3];
    pBuff[4] = pAddress[2];
    pBuff[5] = pAddress[1];
    pBuff[6] = pAddress[0];
    
    switch(ulType)
    {
    case   FTE_HEM12_FIELD_VOLTAGE:
        pBuff[20] = 0x85;
        pBuff[21] = 0x1c;
        pBuff[22] = 0x3f;    
        pBuff[23] = 0xc9;
        break;
    
    case    FTE_HEM12_FIELD_CURRENT:
        pBuff[20] = 0x95;
        pBuff[21] = 0x1c;
        pBuff[22] = 0x4f;    
        pBuff[23] = 0xc9;
        break;
    
    case    FTE_HEM12_FIELD_POWER:
        pBuff[20] = 0x7a;
        pBuff[21] = 0x57;
        pBuff[22] = 0x6f;    
        pBuff[23] = 0xc9;
        break;
    }    
    
    ucCRC = FTE_TASCON_HEM12_CRC(pBuff, 24);
    pBuff[24] = ucCRC;
    
    return  FTE_RET_OK;
} 

static    const FTE_CHAR_PTR    pStringCmd = "cmd";
static    const FTE_CHAR_PTR    pStringAddress = "address";

FTE_RET   FTE_TASCON_HEM12_06M_setConfig
(
    FTE_OBJECT_PTR      pObj, 
    FTE_CHAR_PTR        pString
)
{
    FTE_TASCON_HEM12_CONFIG_PTR  pConfig;

    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
    const nx_json* pxJSON = nx_json_parse_utf8(pString);
    if (pxJSON == NULL)
    {
        return  FTE_RET_ERROR;
    } 
    
    const nx_json* pxCmd = nx_json_get(pxJSON, pStringCmd);
    const nx_json* pxAddress = nx_json_get(pxJSON, pStringAddress);

    if ((pxCmd == NULL) || (pxCmd->type != NX_JSON_STRING))
    {
        goto error;
    }
    
    if (strcmp(pxCmd->text_value, "set_addr") == 0)
    {
        FTE_INT32     i;
        FTE_UINT8  pValues[12];
        
        if ((pxAddress->type != NX_JSON_STRING) || (strlen(pxAddress->text_value) != 12))
        {
            goto error;
        }
        
        for(i = 0 ; i < 12 ; i++)
        {
            if ('0' <= pxAddress->text_value[i] && pxAddress->text_value[i] <= '9')
            {
                pValues[i] = pxAddress->text_value[i] - '0';
            } 
            else if ('A' <= pxAddress->text_value[i] && pxAddress->text_value[i] <= 'F')
            {
                pValues[i] = pxAddress->text_value[i] - 'A' + 10;
            }
            else if ('a' <= pxAddress->text_value[i] && pxAddress->text_value[i] <= 'f')
            {
                pValues[i] = pxAddress->text_value[i] - 'a' + 10;
            }
            else
            {
                goto error;
            }
        }
        
        for(i = 0 ; i < 6 ; i++)
        {
            pConfig->pSensorID[i] = (pValues[i*2] << 4) | pValues[i*2+1];
        }
    }
    else
    {
        goto error;
    }
    
    FTE_CFG_OBJ_save(pObj);
    
    nx_json_free(pxJSON);
   
    return  FTE_RET_OK;
    
error:
    
    nx_json_free(pxJSON);
    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_TASCON_HEM12_06M_getConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    FTE_TASCON_HEM12_CONFIG_PTR    pConfig;
    FTE_JSON_VALUE_PTR          pJOSNObject;
    FTE_JSON_VALUE_PTR          pJOSNValue;
    FTE_CHAR    pIDString[13];
    
    if (pObj == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
    
    pJOSNObject = FTE_JSON_VALUE_createObject(1);
    if (pJOSNObject == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    sprintf(pIDString, "%02x%02x%02x%02x%02x%02x", 
            pConfig->pSensorID[0],
            pConfig->pSensorID[1],
            pConfig->pSensorID[2],
            pConfig->pSensorID[3],
            pConfig->pSensorID[4],
            pConfig->pSensorID[5]);
    
    pJOSNValue = FTE_JSON_VALUE_createString(pIDString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringAddress, pJOSNValue);        
    if (FTE_JSON_VALUE_buffSize(pJOSNObject) >= ulBuffLen)
    {
        FTE_JSON_VALUE_destroy(pJOSNObject);
        return  FTE_RET_ERROR;
    }
    
    FTE_JSON_VALUE_snprint(pBuff, ulBuffLen, pJOSNObject);    
    FTE_JSON_VALUE_destroy(pJOSNObject);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_TASCON_HEM12_06M_request
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_TASCON_HEM12_STATUS_PTR    pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;
    FTE_TASCON_HEM12_CONFIG_PTR    pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
    FTE_UINT32                     nFieldType;
    static FTE_UINT8               pReqBuff[64];
    FTE_UINT32                     ulReqLen;
    FTE_UINT8                      pRcvdBuff[64];
    FTE_UINT32                     ulRcvdLen;
    FTE_UINT8_PTR                  pRespBuff = NULL;
    FTE_UINT32                     ulRespLen;
    FTE_UINT32                     ulVoltage, ulCurrent, ulAmountOfPower, ulPower;
    pStatus->xGUS.xRet = FTE_RET_OK;
    for(nFieldType = FTE_HEM12_FIELD_POWER ; nFieldType <=  FTE_HEM12_FIELD_CURRENT; nFieldType++)
    {
 #if FTE_TASCON_PACKET_DEBUG
    FTE_BOOL                     bPacketDump = FALSE;
#endif
    
       pStatus->nField = nFieldType;
    
        FTE_TASCON_HEM12_06M_FRAME_create(pConfig->pSensorID, pStatus->nField, pReqBuff, sizeof(pReqBuff), &ulReqLen);    

        FTE_UCS_setBaudrate(pStatus->xGUS.pUCS, FTE_TASCON_HEM12_06M_DEFAULT_BAUDRATE);
        FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
        FTE_UCS_clear(pStatus->xGUS.pUCS);    
        
		memset(pRcvdBuff,0,sizeof(pRcvdBuff));
        ulRcvdLen = FTE_UCS_sendAndRecv(pStatus->xGUS.pUCS, pReqBuff, ulReqLen, pRcvdBuff, sizeof(pRcvdBuff), FTE_HEM12_RESP_DELAY_TIME, FTE_HEM12_RESP_WAIT_TIME);            
        
        pRespBuff = pRcvdBuff;
        ulRespLen = ulRcvdLen;
        for(; ulRespLen > 0 ; ulRespLen--)
        {
            if (pRespBuff[0] == FTE_HEM12_FRAME_START)
            {
                break;
            }
            
            pRespBuff++;
        }
        
        switch(pStatus->nField)
        {
        case    FTE_HEM12_FIELD_VOLTAGE:
            {
                if (FTE_TASCON_HEM12_06M_FRAME_responseVoltage(pRespBuff, ulRespLen, &ulVoltage) == FTE_RET_OK)
                {
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[2], ulVoltage);
                }
                else
                {
#if FTE_TASCON_PACKET_DEBUG
                    bPacketDump = TRUE;
#endif
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[2], FALSE);
                    pStatus->xGUS.xRet = FTE_RET_ERROR;
                }
            }
            break;

        case    FTE_HEM12_FIELD_CURRENT:
            {
                if (FTE_TASCON_HEM12_06M_FRAME_responseCurrent(pRespBuff, ulRespLen, &ulCurrent) == FTE_RET_OK)
                {
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[3], ulCurrent);
                }
                else
                {
#if FTE_TASCON_PACKET_DEBUG
                    bPacketDump = TRUE;
#endif
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[3], FALSE);
                    pStatus->xGUS.xRet = FTE_RET_ERROR;
                }
            }
            break;
            
        case    FTE_HEM12_FIELD_POWER:
            {
                if (FTE_TASCON_HEM12_06M_FRAME_responsePower(pRespBuff, ulRespLen, &ulAmountOfPower, &ulPower) == FTE_RET_OK)
                {
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[0], ulAmountOfPower);
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[1], ulPower);
                }
                else
                {
#if FTE_TASCON_PACKET_DEBUG
                    bPacketDump = TRUE;
#endif
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[1], FALSE);
                    pStatus->xGUS.xRet = FTE_RET_ERROR;
                }
            }
            break;
            
        default:
            pStatus->xGUS.xRet = FTE_RET_ERROR;
        }    
    
#if FTE_TASCON_PACKET_DEBUG
        if (bDebugON && bPacketDump)
        {
            FTE_INT32 i;

            printf("SEND[%d] : ", pStatus->nField);
            for(i = 0 ; i < ulReqLen ; i++)
            {
                printf("%02x ", pReqBuff[i]);
            }
            printf("\n");

            printf("RECV[%8d] : ", pStatus->nField);
            if (ulRcvdLen == 0)
            {
                static FTE_INT32 nError = 0;
                printf("Error! [%5d]\n", ++nError);
            }
            else
            {
                for(i = 0 ; i < ulRcvdLen ; i++)
                {
                    printf("%02x ", pRcvdBuff[i]);
                }
                printf("\n");
            }
        }
#endif   
    
    }
    
    return  pStatus->xGUS.xRet;

}

FTE_RET     FTE_TASCON_HEM12_06M_received
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_TASCON_HEM12_STATUS_PTR    pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;
    /*
    FTE_UINT8                      pBuff[64];
    FTE_UINT32                     ulLen;
    FTE_UINT32                     ulVoltage, ulCurrent, ulAmountOfPower, ulPower;

    ulLen = FTE_UCS_recv(pStatus->xGUS.pUCS, pBuff, sizeof(pBuff));

    switch(pStatus->nField)
    {
    case    FTE_HEM12_FIELD_VOLTAGE:
        {
            if (FTE_TASCON_HEM12_06M_FRAME_responseVoltage(pBuff, ulLen, &ulVoltage) == FTE_RET_OK)
            {
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[2], ulVoltage);
            }
            else
            {
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[2], FALSE);
                pStatus->xGUS.xRet = FTE_RET_ERROR;
            }
        }
        break;

    case    FTE_HEM12_FIELD_CURRENT:
        {
            if (FTE_TASCON_HEM12_06M_FRAME_responseCurrent(pBuff, ulLen, &ulCurrent) == FTE_RET_OK)
            {
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[3], ulCurrent);
            }
            else
            {
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[3], FALSE);
                pStatus->xGUS.xRet = FTE_RET_ERROR;
            }
        }
        break;
        
    case    FTE_HEM12_FIELD_POWER:
        {
            if (FTE_TASCON_HEM12_06M_FRAME_responsePower(pBuff, ulLen, &ulAmountOfPower, &ulPower) == FTE_RET_OK)
            {
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[0], ulAmountOfPower);
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[1], ulPower);
            }
            else
            {
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[1], FALSE);
                pStatus->xGUS.xRet = FTE_RET_ERROR;
            }
        }
        break;
        
    default:
        pStatus->xGUS.xRet = FTE_RET_ERROR;
    }
    */
    return  pStatus->xGUS.xRet;
}

FTE_RET FTE_TASCON_create
(
    FTE_UINT32  xType,
    FTE_UINT8   pSensorID[6],
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    FTE_INT32 i;
    FTE_RET                 xRet;
    FTE_OBJECT_CONFIG_PTR   pBaseConfig = NULL;
    FTE_OBJECT_CONFIG_PTR   pConfig = NULL;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_TASCON_HEM12_FIELD_MAX];
    FTE_UINT32              ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    for(i = 0 ; i < FTE_TASCON_MAX ; i++)
    {
        if (pDevices[i].pObj != NULL)
        {
            if (memcmp(((FTE_TASCON_HEM12_CONFIG_PTR)pDevices[i].pObj->pConfig)->pSensorID, pSensorID, 6) == 0)
            {
                return  FTE_RET_DUPLICATE;
            }
        }
    }
    
    switch(xType)
    {
    //case    FTE_OBJ_TYPE_MULTI_HEM12:
	case FTE_TASCON_MODEL_HEM12 :
        {
            pBaseConfig = (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_defaultConfig;
        }
        break;
        
    //case    FTE_OBJ_TYPE_MULTI_HEM12_06M:
	case    FTE_TASCON_MODEL_HEM12_06M:
        {
            pBaseConfig = (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_06M_defaultConfig;
        }
        break;
        
    default:
        return  FTE_RET_INVALID_MODEL;
    }
    
    memcpy(((FTE_TASCON_HEM12_CONFIG_PTR)pBaseConfig)->pSensorID, pSensorID, 6);
    
    xRet = FTE_CFG_OBJ_create(pBaseConfig, &pConfig, pChildConfig, FTE_TASCON_HEM12_FIELD_MAX, &ulChildCount);    
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }    
    
    pObj = FTE_OBJ_create(pConfig);
    if (pObj == NULL)
    {
        return  FTE_RET_INSUFFICIENT_MEMORY;
    }
    
    if (ulChildCount != 0)
    {
        for(i = 0 ; i < ulChildCount ; i++)
        {
            FTE_OBJECT_PTR  pChild;
            
            pChild = FTE_OBJ_create(pChildConfig[i]);
            if (pChild == NULL)
            {
                ERROR("The child object creation failed.\n");
            }
        }
    }
    
    *ppObj = pObj;
        
    return  FTE_RET_OK;    
}

FTE_RET FTE_TASCON_HEM12_create
(
    FTE_UINT8   pSensorID[6],
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    return  FTE_TASCON_create(FTE_OBJ_TYPE_MULTI_HEM12, pSensorID, ppObj);
}

FTE_RET FTE_TASCON_HEM12_06M_create
(
    FTE_UINT8   pSensorID[6],
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    return  FTE_TASCON_create(FTE_OBJ_TYPE_MULTI_HEM12_06M, pSensorID, ppObj);
}

FTE_INT32  FTE_TASCON_HEM12_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL bPrintUsage, bShortHelp = FALSE;
    FTE_INT32  nRetCode = SHELL_EXIT_SUCCESS;
    FTE_RET xRet;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                FTE_UINT32 count = FTE_OBJ_count(FTE_OBJ_TYPE_MASK, FTE_OBJ_TYPE_MULTI_HEM12_06M, FALSE);
                FTE_UINT32 i;

                printf("%-8s %-16s %-24s %-8s %-16s %-s\n", 
                        "ID", "TYPE", "NAME", "STATUS", "VALUE", "TIME");
                for(i = 0 ; i < count ; i++)
                {
                    FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MASK, FTE_OBJ_TYPE_MULTI_HEM12_06M, i, FALSE);
                    FTE_TASCON_HEM12_CONFIG_PTR pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
                    FTE_TASCON_HEM12_STATUS_PTR pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;
                    
                    if (pObj != NULL)
                    {
                        printf("%08x %-16s %-8s(%02x%02x%02x%02x%02x%02x) %8s", 
                              pConfig->xCommon.nID, 
                              FTE_OBJ_typeString(pObj), 
                              pConfig->xCommon.pName, 
                              pConfig->pSensorID[0],
                              pConfig->pSensorID[1],
                              pConfig->pSensorID[2],
                              pConfig->pSensorID[3],
                              pConfig->pSensorID[4],
                              pConfig->pSensorID[5],
                              FTE_OBJ_IS_ENABLED(pObj)?"RUN":"STOP");                       
                        
                        if (pStatus->xGUS.xCommon.pValue != NULL)
                        {
                            TIME_STRUCT xTime;
                            FTE_CHAR    pTimeString[64];
                            FTE_CHAR    pValueString[32];
                            FTE_CHAR    pUnitString[8];
                            
                            FTE_VALUE_toString(pStatus->xGUS.xCommon.pValue, pValueString, sizeof(pValueString));
                            FTE_VALUE_unit(pStatus->xGUS.xCommon.pValue, pUnitString, sizeof(pUnitString));
                            FTE_VALUE_getTimeStamp(pStatus->xGUS.xCommon.pValue, &xTime);
                            FTE_TIME_toStr(&xTime, pTimeString, sizeof(pTimeString));
                            printf(" %8s %-7s %s", pValueString, pUnitString, pTimeString);
                        }
                        
                        FTE_OBJECT_STATISTICS   xStatistics;
                        
                        xRet = FTE_OBJ_getStatistics(pObj, &xStatistics);
                        if (xRet == FTE_RET_OK)
                        {
                            FTE_UINT32                 nRatio;
                            
                            nRatio = (xStatistics.nTotalTrial - xStatistics.nTotalFail) * 10000 / xStatistics.nTotalTrial;
                            
                            printf(" %3d.%02d%%(%d, %d, %d)", nRatio/100, nRatio%100, xStatistics.nTotalTrial, xStatistics.nTotalFail, xStatistics.nPartialFail);
                        }
                        printf("\n");
                    }
                }
            }
            break;
               
        case    3:
            {
#if FTE_TASCON_PACKET_DEBUG
                if (strcmp(pArgv[1], "dump") == 0)
                {
                    if (strcmp(pArgv[2], "on") == 0)
                    {
                        bDebugON = TRUE;
                    }
                    else if (strcmp(pArgv[2], "off") == 0)
                    {
                        bDebugON = FALSE;
                    }
                    else
                    {
                        bPrintUsage = TRUE;
                    }
                }
#endif
                if (strcmp(pArgv[2], "get_addr") == 0)
                {
                    FTE_UINT32  nOID = 0;
                    FTE_strToHex(pArgv[1], &nOID);
                    
                    FTE_OBJECT_PTR  pObj = FTE_OBJ_get(nOID);                    
                    if (pObj == NULL)
                    {
                        printf("Invalid OID[%08x]\n", nOID);
						break;
                    }
                    
                    if (FTE_OBJ_TYPE(pObj) == FTE_OBJ_TYPE_MULTI_HEM12)
                    {                    
                        FTE_UINT8      pReqBuff[32];
                        FTE_UINT32     ulReqLen;
                        FTE_UINT8      pRcvdBuff[32];
                        FTE_UINT32     ulRcvdLen;
                        FTE_CHAR_PTR    pHead;
                        FTE_UINT32     nSkip = 0;
                        FTE_UCS_UART_CONFIG xUARTConfig;
                        
                        FTE_TASCON_HEM12_CONFIG_PTR pConfig = (FTE_TASCON_HEM12_CONFIG_PTR)pObj->pConfig;
                        FTE_TASCON_HEM12_STATUS_PTR pStatus = (FTE_TASCON_HEM12_STATUS_PTR)pObj->pStatus;
                        FTE_TASCON_HEM12_FRAME_create(pConfig->pSensorID, 1, pReqBuff, sizeof(pReqBuff), &ulReqLen);    

                        FTE_UCS_getUART(pStatus->xGUS.pUCS, &xUARTConfig);
                        
                        FTE_UCS_setUART(pStatus->xGUS.pUCS, (FTE_UCS_UART_CONFIG_PTR)&pStatus->xGUS.pModelInfo->xUARTConfig);
                        
                        FTE_UCS_clear(pStatus->xGUS.pUCS);    
                        ulRcvdLen = FTE_UCS_sendAndRecv(pStatus->xGUS.pUCS, pReqBuff, ulReqLen, pRcvdBuff, sizeof(pRcvdBuff), FTE_HEM12_RESP_DELAY_TIME, FTE_HEM12_RESP_WAIT_TIME);            
                        
                        FTE_UCS_setUART(pStatus->xGUS.pUCS, &xUARTConfig);

                        if (ulRcvdLen >= 20)
                        {
                            pHead = (FTE_CHAR_PTR)pRcvdBuff;
                            for(FTE_INT32 i = 0 ; i < ulRcvdLen ; i++)
                            {
                                if (pHead[i] == 0xFE)
                                {
                                    nSkip++;
                                }
                                else
                                {
                                    break;
                                }
                            }
                            
                            pHead += nSkip;
                            ulRcvdLen -= nSkip;
                            
                            if ((ulRcvdLen < 20) ||
                                (pHead[0] != FTE_HEM12_FRAME_START) ||
                                (pHead[7] == FTE_HEM12_START_CODE) ||
                                (ulRcvdLen < (12 + pHead[9])))
                            {
                                printf("Received invalid frame.\n");
                            }
                            else
                            {
                                for(FTE_INT32 i = 0 ; i < 6 ; i++)
                                {
                                    printf("%02x ", pHead[12+i]);
                                }
                                printf("\n");
                            }
                        }
                     }                
                }
            }
            break;
            
         case    4:
            {
                
                
                if (strcmp(pArgv[2], "set_addr") == 0)
                {
 					FTE_UINT32  nOID = 0;
                	FTE_strToHex(pArgv[1], &nOID);
                
                	FTE_OBJECT_PTR  pObj = FTE_OBJ_get(nOID);                    
                	if (pObj == NULL)
                	{
                    	printf("Invalid OID[%08x]\n", nOID);
						break;
                	}
					
                    if (FTE_TASCON_HEM12_06M_setAddress(pObj, pArgv[3], strlen(pArgv[3])) == FTE_RET_OK)
                    {
                        FTE_CFG_OBJ_save(pObj);
                    }
                    else
                    {
                        printf("Invalid Adddress[%s]\n", pArgv[3]);
                    }
                }
                else if (strcasecmp(pArgv[1], "create") == 0)
                {
                    FTE_UINT32      ulLen;
                    FTE_TASCON_MODEL    xModel;
                    FTE_UINT8       pSensorID[6];
                    FTE_OBJECT_PTR  pObj;
                    
                    if (strlen(pArgv[3]) != 12)
                    {
                        printf("Invalid Sensor ID\n");
                        break;
                    }
                    
                    xRet = FTE_strToHexArray(pArgv[3], pSensorID, 6, &ulLen);
                    if ((xRet != FTE_RET_OK) || (ulLen != 6))
                    {
                        printf("Invalid Sensor ID\n");
                        break;
                    }
                    
                    if (strcasecmp(pArgv[2], "hem12") == 0)
                    {
                        xModel = FTE_TASCON_MODEL_HEM12;
                    }
                    else if (strcasecmp(pArgv[2], "hem12-06m") == 0)
                    {
                        xModel = FTE_TASCON_MODEL_HEM12_06M;
                    }
                                            
                    xRet = FTE_TASCON_create(xModel, pSensorID, &pObj);
                    if (xRet == FTE_RET_OK)
                    {
                        printf("The object[%08x] is created successfully.\n", pObj->pConfig->xCommon.nID);
                    }
					else if(xRet == FTE_RET_DUPLICATE)
					{
					    printf("The ADDRESS[%s] is duplicated.\n", pArgv[3]);
					}
                    else
                    {
                        printf("Not supported or invalid model[%s]\n", pArgv[2]);
                    }
                }
            }
            break;
                       
        default:
            bPrintUsage = TRUE;
        }
    }
    
    if (bPrintUsage || (nRetCode !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<command>]\n", pArgv[0]);
        }
        else
        {
            printf ("Usage: %s [<command>]\n", pArgv[0]);
            printf ("  Commands:\n");
            printf ("  ---------------------------------------\n");
			printf ("    <id> info\n");
            printf ("        Show object information.\n");
			printf ("  ---------------------------------------\n");
			printf ("    create <MODEL> <ADDRESS>\n");
			printf ("		<MODEL>   HEM12 or HEM12-06m \n");
			printf ("		<ADDRESS> Device Address. \n");
			printf ("  ---------------------------------------\n");
            printf ("    <id> set_addr <address>\n");
            printf ("        Set HEM12 Device Address.\n");
			printf ("  ---------------------------------------\n");
        }
    }
    return   nRetCode;
}
#endif