#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

FTE_RET   FTE_BOTEM_PN1500_switchCtrl(FTE_OBJECT_PTR pObj, FTE_BOOL bSwitchON);
FTE_RET   FTE_BOTEM_PN1500_reset(FTE_OBJECT_PTR pObj);
FTE_RET   FTE_BOTEM_PN1500_countReset(FTE_OBJECT_PTR pObj);
FTE_RET   FTE_BOTEM_PN1500_accumCountReset(FTE_OBJECT_PTR pObj);
FTE_RET FTE_BOTEM_PN1500_createJSON
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32      ulOption,
    FTE_JSON_OBJECT_PTR _PTR_ ppObject
);

static FTE_UINT32  _version = 1;


static const 
FTE_IFCE_CONFIG FTE_BOTEM_PN1500_HUMAN_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0x0001),
        .pName      = "COUNT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 0,
    .nInterval  = FTE_BOTEM_PN1500_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_BOTEM_PN1500_ACCUM_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0x0001),
        .pName      = "ACCUM",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 1,
    .nInterval  = FTE_BOTEM_PN1500_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_BOTEM_PN1500_ACCRM_RESET_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0001),
        .pName      = "ACCUM RESET",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 1,
    .nInterval  = FTE_BOTEM_PN1500_DEFAULT_UPDATE_INTERVAL
};


static const 
FTE_IFCE_CONFIG FTE_BOTEM_PN1500_SWITCH_STAT_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0001),
        .pName      = "SWITCH",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 2,
    .nInterval  = FTE_BOTEM_PN1500_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_BOTEM_PN1500_SWITCH_CTRL_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0001),
        .pName      = "SWITCH",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 2,
    .nInterval  = FTE_BOTEM_PN1500_DEFAULT_UPDATE_INTERVAL
};


static const 
FTE_OBJECT_CONFIG_PTR FTE_BOTEM_PN1500_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_BOTEM_PN1500_HUMAN_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_BOTEM_PN1500_ACCUM_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_BOTEM_PN1500_ACCRM_RESET_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_BOTEM_PN1500_SWITCH_STAT_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_BOTEM_PN1500_SWITCH_CTRL_defaultConfig,
};

FTE_GUS_CONFIG FTE_BOTEM_PN1500_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
        .pName      = "PN1500",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
        .ulChild    = sizeof(FTE_BOTEM_PN1500_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_BOTEM_PN1500_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_BOTEM_PN1500,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_BOTEM_PN1500_DEFAULT_UPDATE_INTERVAL
};

FTE_VALUE_TYPE  FTE_BOTEM_PN1500_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_DIO
};

const 
FTE_GUS_MODEL_INFO   FTE_BOTEM_PN1500_GUSModelInfo = 
{
    .nModel     = FTE_GUS_MODEL_BOTEM_PN1500,
    .pName      = "BOTEM PN1500",
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_BOTEM_PN1500_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_BOTEM_PN1500_DEFAULT_DATABITS,
        .nParity    =   FTE_BOTEM_PN1500_DEFAULT_PARITY,
        .nStopBits  =   FTE_BOTEM_PN1500_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_BOTEM_PN1500_DEFAULT_FULL_DUPLEX
    },
    .nFieldCount= 3,
    .pValueTypes= FTE_BOTEM_PN1500_valueTypes,
    .fRequest   = FTE_BOTEM_PN1500_request,
    .fReceived  = FTE_BOTEM_PN1500_received,
    .fSet       = FTE_BOTEM_PN1500_set,
    .fSetConfig = FTE_BOTEM_PN1500_setConfig,
    .fGetConfig = FTE_BOTEM_PN1500_getConfig,
    .fCreateJSON= FTE_BOTEM_PN1500_createJSON
};

FTE_RET   FTE_BOTEM_PN1500_request
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    FTE_UINT8  pCMD[] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'R', ']', '\0'};
    
    pCMD[0] = (FTE_UINT8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), TRUE);    

    return  FTE_RET_OK; 
}

FTE_RET   FTE_BOTEM_PN1500_received
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_UINT32  nPeopleCount = 0;
    FTE_UINT32  nAccum = 0;
    FTE_BOOL    bSwitch = 0;
    FTE_UINT8   pBuff[64];
    FTE_UINT32  nLen;
    FTE_UINT32  nPrevPeopleCount = 0;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));

    if (_version == 0)
    {
        if (nLen != 23)
        {
            return  FTE_RET_INVALID_MSG_FRAME;
        }

        if ((pBuff[0] != '[') || (pBuff[nLen-1] != ']'))
        {
            return  FTE_RET_INVALID_MSG_FRAME;
        }
        
        pBuff[14] = 0;
        pBuff[16] = 0;
        pBuff[22] = 0;

        nPeopleCount = atoi((FTE_CHAR_PTR)&pBuff[9]);
        bSwitch = (pBuff[15] == '1');
        nAccum = atoi((FTE_CHAR_PTR)&pBuff[17]);

        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[0], &nPrevPeopleCount);
        if (nPrevPeopleCount != nPeopleCount)
        {
            FTE_OBJ_wasChanged(pObj);
        }
        else
        {
            FTE_OBJ_wasUpdated(pObj);
        }
    }
    else
    {
        if (nLen != 54)
        {
            return  FTE_RET_INVALID_MSG_FRAME;
        }

        if ((pBuff[0] != '[') || (pBuff[nLen-1] != ']'))
        {
            return  FTE_RET_INVALID_MSG_FRAME;
        }
        
        pBuff[20] = 0;
        pBuff[26] = 0;
        pBuff[32] = 0;

        nPeopleCount = atoi((FTE_CHAR_PTR)&pBuff[21]);
        bSwitch = (pBuff[12] == '1');
        nAccum = atoi((FTE_CHAR_PTR)&pBuff[27]);

        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[0], &nPrevPeopleCount);
        if (nPrevPeopleCount != nPeopleCount)
        {
            FTE_OBJ_wasChanged(pObj);
        }
        else
        {
            FTE_OBJ_wasUpdated(pObj);
        }
    }
    
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[0], nPeopleCount);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[1], nAccum);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[2], bSwitch);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_BOTEM_PN1500_set
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nIndex, 
    FTE_VALUE_PTR   pValue
)
{
    FTE_UINT8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T'};
    FTE_UINT32 nCmdLen = 8;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    switch(nIndex)
    {
    case    1:
        {
            pCMD[nCmdLen++] = 'Z';        
            if (_version == 1)
            {
                pCMD[nCmdLen++] = '1';        
            }
        }
        break;
        
    case    2:
        {
            if (pValue->xData.ulValue == 0)
            {
                pCMD[nCmdLen++] = 'F';
            }
            else 
            {
                pCMD[nCmdLen++] = 'N';
            }

            if (_version == 1)
            {
                pCMD[nCmdLen++] = '1';        
            }
        }
        break;
        
    default:
        {
            if (10 < pValue->xData.ulValue && pValue->xData.ulValue < 40)
            {
                pCMD[nCmdLen++] = 'T';
                pCMD[nCmdLen++] = (pValue->xData.ulValue / 10) + '0'; 
                pCMD[nCmdLen++] = (pValue->xData.ulValue % 10) + '0'; 
            }
            else
            {
                return  FTE_RET_INVALID_MSG_FRAME;
            }
        }
    }
    
    pCMD[nCmdLen++] = ']';
    pCMD[nCmdLen++] = '\0';
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, TRUE);    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_BOTEM_PN1500_setConfig
(
    FTE_OBJECT_PTR      pObj, 
    FTE_CHAR_PTR        pString
)
{
    ASSERT(pObj != NULL);
    
    const nx_json* pxJSON = nx_json_parse_utf8(pString);
    if (pxJSON == NULL)
    {
        return  FTE_RET_ERROR;
    } 
    
    const nx_json* pxCmd = nx_json_get(pxJSON, "cmd");

    if (pxCmd == NULL)
    {
        goto error;
    }
    
    if (strcmp(pxCmd->text_value, "switch_on") == 0)
    {
        FTE_BOTEM_PN1500_switchCtrl(pObj, TRUE);
    }
    else if (strcmp(pxCmd->text_value, "switch_off") == 0)
    {
        FTE_BOTEM_PN1500_switchCtrl(pObj, FALSE);
    }
    else if (strcmp(pxCmd->text_value, "reset") == 0)
    {
        FTE_BOTEM_PN1500_reset(pObj);
    }
    else if (strcmp(pxCmd->text_value, "count_reset") == 0)
    {
        FTE_BOTEM_PN1500_countReset(pObj);
    }
    else if (strcmp(pxCmd->text_value, "accum_reset") == 0)
    {
        FTE_BOTEM_PN1500_accumCountReset(pObj);
    }
    else
    {
        goto error;
    }
    
    nx_json_free(pxJSON);
   
    return  FTE_RET_OK;
    
error:
    
    nx_json_free(pxJSON);
    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_BOTEM_PN1500_getConfig
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    FTE_GUS_STATUS_PTR      pStatus;
    FTE_JSON_VALUE_PTR      pJSONObject;
    FTE_JSON_VALUE_PTR      pJSONValue;
    FTE_UINT32                 ulValue;
    FTE_BOOL                 bValue;
    
    ASSERT(pObj != NULL);
    
    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    pJSONObject = FTE_JSON_VALUE_createObject(3);
    if (pJSONObject == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    FTE_VALUE_getULONG(&pStatus->xCommon.pValue[0], &ulValue);
    pJSONValue = FTE_JSON_VALUE_createNumber(ulValue);    
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJSONObject, "count", pJSONValue);        
    
    FTE_VALUE_getULONG(&pStatus->xCommon.pValue[1], &ulValue);
    pJSONValue = FTE_JSON_VALUE_createNumber(ulValue);    
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJSONObject, "accum", pJSONValue);        
    

    FTE_VALUE_getDIO(&pStatus->xCommon.pValue[2], &bValue);
    pJSONValue = FTE_JSON_VALUE_createNumber(bValue);    
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJSONObject, "switch", pJSONValue);        
    
    if (FTE_JSON_VALUE_buffSize(pJSONObject) >= ulBuffLen)
    {
        FTE_JSON_VALUE_destroy(pJSONObject);
        return  FTE_RET_ERROR;
    }
    
    FTE_JSON_VALUE_snprint(pBuff, ulBuffLen, pJSONObject);    
    FTE_JSON_VALUE_destroy(pJSONObject);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_BOTEM_PN1500_switchCtrl
(
    FTE_OBJECT_PTR  pObj, 
    FTE_BOOL        bSwitchON
)
{
    FTE_UINT8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T'};
    FTE_UINT32 nCmdLen = 8;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (bSwitchON)
    {
        pCMD[nCmdLen++] = 'N';        
    }
    else
    {
        pCMD[nCmdLen++] = 'F';        
    }
    
    if (_version == 1)
    {
        pCMD[nCmdLen++] = '1';        
    }
    pCMD[nCmdLen++] = ']';
    pCMD[nCmdLen++] = '\0';
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_BOTEM_PN1500_countReset
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_UINT8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'Z'};
    FTE_UINT32 nCmdLen = 9;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (_version == 1)
    {
        pCMD[nCmdLen++] = '1';        
    }
    pCMD[nCmdLen++] = ']';
    pCMD[nCmdLen++] = '\0';
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_BOTEM_PN1500_accumCountReset
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_UINT8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'Z', '2', ']', '\0'};
    FTE_UINT32 nCmdLen = 12;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_BOTEM_PN1500_reset
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_UINT8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'R', ']', '\0'};
    FTE_UINT32 nCmdLen = 11;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  FTE_RET_OK;
}


FTE_RET FTE_BOTEM_PN1500_createJSON
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32      ulOption,
    FTE_JSON_OBJECT_PTR _PTR_ ppJSON
)
{
    
    ASSERT((pObj != NULL) && (ppJSON != NULL));
    
    FTE_RET xRet;
    FTE_JSON_VALUE_PTR  pValue = NULL;
    FTE_JSON_OBJECT_PTR  pValues ;
    FTE_UINT32 ulValue;
    FTE_BOOL bValue;
    
    ASSERT(pObj != NULL);

    pValues = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(3);
    if (pValues == NULL)
    {
        goto error;
    }
    
    FTE_VALUE_getULONG(&pObj->pStatus->pValue[0], &ulValue);
    pValue = FTE_JSON_VALUE_createNumber(ulValue);
    if (pValue == NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
        goto error;
    }
    if (FTE_JSON_OBJECT_setPair(pValues, "count", pValue) != FTE_RET_OK)
    {
        FTE_JSON_VALUE_destroy(pValue);
        goto error;
    }
    
    FTE_VALUE_getULONG(&pObj->pStatus->pValue[1], &ulValue);
    pValue = FTE_JSON_VALUE_createNumber(ulValue);
    if (pValue == NULL)
    {
        goto error;
    }
    if (FTE_JSON_OBJECT_setPair(pValues, "accum", pValue) != FTE_RET_OK)
    {
        FTE_JSON_VALUE_destroy(pValue);
        goto error;
    }

    FTE_VALUE_getDIO(&pObj->pStatus->pValue[2], &bValue);
    pValue = FTE_JSON_VALUE_createNumber(bValue);
    if (pValue == NULL)
    {
        xRet = FTE_RET_NOT_ENOUGH_MEMORY;
        goto error;
    }
    
    xRet = FTE_JSON_OBJECT_setPair(pValues, "switch", pValue);
    if (xRet != FTE_RET_OK)
    {
        FTE_JSON_VALUE_destroy(pValue);
        goto error;
    }                    
    
    *ppJSON = (FTE_JSON_OBJECT_PTR)pValue;
    
    return  FTE_RET_OK;
error:
    
    if (pValues != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pValues);
        pValues = NULL;
    }
    
    return  xRet;
}
