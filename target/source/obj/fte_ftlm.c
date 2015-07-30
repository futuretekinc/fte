#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

_mqx_uint   FTE_FTLM_switchCtrl(FTE_OBJECT_PTR pObj, boolean bSwitchON);
_mqx_uint   FTE_FTLM_countReset(FTE_OBJECT_PTR pObj);

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
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG,
};

_mqx_uint   FTE_FTLM_requestData(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    uint_8  pCMD[] = { 0x3a, 0x01, 0x03, 0x00, 0x65, 0x00, 0x1b, 0x00, 0x00, 0x0d, 0x0a, 0x00};
    uint_16 uiCRC;
    
    pCMD[1] = (uint_8)pConfig->nSensorID;
    uiCRC = fte_crc16(&pCMD[1], 6);
    pCMD[7] = (uiCRC     ) & 0xFF;
    pCMD[8] = (uiCRC >> 8) & 0xFF;

    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), TRUE);    

    return  MQX_OK; 
}
 
_mqx_uint   FTE_FTLM_receiveData(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    uint_8      pBuff[128];
    uint_32     nLen, i;
    uint_16     uiCRC;
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if ((nLen < 4) || (pBuff[0] != 0x3a) || (pBuff[1] != pConfig->nSensorID))// || (pBuff[nLen - 2] != 0x0d) || (pBuff[nLen - 1] != 0x0a) || (nLen != 4 + pBuff[3] + 4))
    {
        return  MQX_ERROR;
    }
    
    uiCRC = fte_crc16(&pBuff[1], 3 + pBuff[3]);
    if (uiCRC != (pBuff[4 + pBuff[3]] | ((uint_16)pBuff[4 + pBuff[3] + 1] << 8)))
    {
        return  MQX_ERROR;
    } 
  
    for(i = 0 ; i < pBuff[3] / 2 / 3 ; i++)
    {
        uint_32 ulValue = 0;
        
        ulValue = ((uint_32)pBuff[4 + i*6 + 1]) | ((uint_32)pBuff[4 + i*6 + 3] << 8) | ((uint_32)pBuff[4 + i*6 + 5] << 16) ;
        
        FTE_VALUE_setULONG(&pStatus->xCommon.pValue[i], ulValue);
    }

    return  MQX_OK;
}

_mqx_uint   FTE_FTLM_set(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    uint_8      pCMD[] = { 0x3a, 0x01, 0x10, 0x00, 0x65, 0x00, 0x3, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a, 0x00};
    uint_8      pBuff[128];
    uint_32     nLen;
    uint_16     uiCRC;
    uint_32     ulValue;
    uint_32     ulTry = 0;
    if (pStatus->xCommon.nValueCount <= nIndex)
    {
        return  MQX_ERROR;
    }
    
    memset(pBuff, 0, sizeof(pBuff));
   
    FTE_VALUE_getULONG(pValue, &ulValue);
    
    pCMD[1] = (uint_8)pConfig->nSensorID;
    pCMD[4] = (uint_8)(101 + nIndex*3);
    pCMD[9] = (uint_8)(ulValue & 0xFF);
    pCMD[11]= (uint_8)((ulValue >> 8) & 0xFF);
    pCMD[13]= (uint_8)((ulValue >> 16) & 0xFF);
    uiCRC = fte_crc16(&pCMD[1], 13);
    pCMD[14] = (uiCRC     ) & 0xFF;
    pCMD[15] = (uiCRC >> 8) & 0xFF;

    do
    {
        do 
        {
            ulTry++;
            FTE_UCS_clear(pStatus->pUCS);
            nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 100, 200);
        } 
        while ((ulTry < 3) && ((nLen < 4) || (pBuff[0] != 0x3a) || (pBuff[1] != pConfig->nSensorID)));
        
        uiCRC = fte_crc16(&pBuff[1], 6);
    } 
    while((ulTry < 3) && (uiCRC != (pBuff[7] | ((uint_16)pBuff[8] << 8))));
    
    if (ulTry >= 3)
    {
        return  MQX_ERROR;
    }
    
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[nIndex], ulValue);
    pStatus->xCommon.pValue[nIndex].bChanged = TRUE;
    return  MQX_OK;
}


_mqx_uint   FTE_FTLM_setConfig(FTE_OBJECT_PTR  pObj, char_ptr pString)
{
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    const nx_json* pxJSON = nx_json_parse_utf8(pString);
    if (pxJSON == NULL)
    {
        return  MQX_ERROR;
    } 
    
    const nx_json* pxCmd = nx_json_get(pxJSON, "cmd");

    if (pxCmd == NULL)
    {
        goto error;
    }
    
    if (strcmp(pxCmd->text_value, "switch_on") == 0)
    {
        FTE_FTLM_switchCtrl(pObj, TRUE);
    }
    else if (strcmp(pxCmd->text_value, "switch_off") == 0)
    {
        FTE_FTLM_switchCtrl(pObj, FALSE);
    }
    else if (strcmp(pxCmd->text_value, "count_reset") == 0)
    {
        FTE_FTLM_countReset(pObj);
    }
    else
    {
        goto error;
    }
    
    nx_json_free(pxJSON);
   
    return  MQX_OK;
    
error:
    
    nx_json_free(pxJSON);
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_FTLM_getConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen)
{
    FTE_GUS_STATUS_PTR      pStatus;
    FTE_JSON_VALUE_PTR      pJSONObject;
    FTE_JSON_VALUE_PTR      pJSONValue;
    uint_32                 ulValue;
    boolean                 bValue;
    
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    pJSONObject = FTE_JSON_VALUE_createObject(3);
    if (pJSONObject == NULL)
    {
        return  MQX_ERROR;
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
        return  MQX_ERROR;
    }
    
    FTE_JSON_VALUE_snprint(pBuff, ulBuffLen, pJSONObject);    
    FTE_JSON_VALUE_destroy(pJSONObject);
    
    return  MQX_OK;
}

_mqx_uint   FTE_FTLM_switchCtrl(FTE_OBJECT_PTR pObj, boolean bSwitchON)
{
    uint_8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T'};
    uint_32 nCmdLen = 8;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (bSwitchON)
    {
        pCMD[nCmdLen++] = 'N';        
    }
    else
    {
        pCMD[nCmdLen++] = 'F';        
    }
    
    pCMD[nCmdLen++] = ']';
    pCMD[nCmdLen++] = '\0';
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  MQX_OK;
}

_mqx_uint   FTE_FTLM_countReset(FTE_OBJECT_PTR pObj)
{
    uint_8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'Z', ']', '\0'};
    uint_32 nCmdLen = 11;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  MQX_OK;
}
