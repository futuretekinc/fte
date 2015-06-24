#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

_mqx_uint   fte_ftlm_switchCtrl(FTE_OBJECT_PTR pObj, boolean bSwitchON);
_mqx_uint   fte_ftlm_countReset(FTE_OBJECT_PTR pObj);

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
    FTE_VALUE_TYPE_ULONG 
};

_mqx_uint   fte_ftlm_request_data(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    uint_8  pCMD[] = { 0x01, 0x03, 0x00, 0x65, 0x00, 0x1a, 0x00, 0x00, 0x00};
    uint_16 uiCRC;
    
    uiCRC = fte_crc16(pCMD, 6);
    pCMD[6] = (uiCRC     ) & 0xFF;
    pCMD[7] = (uiCRC >> 8) & 0xFF;

    pCMD[0] = (uint_8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), TRUE);    

    return  MQX_OK; 
}

_mqx_uint   fte_ftlm_receive_data(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_8      pBuff[128];
    uint_32     nLen, i;
    uint_16     uiCRC;
#if 0    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 59)
    {
        return  MQX_ERROR;
    }

    if ((pBuff[0] != 0x01))
    {
        return  MQX_ERROR;
    }
    
    uiCRC = fte_crc16(pBuff, 57);
    if (uiCRC != (pBuff[57] | ((uint_16)pBuff[58] << 8)))
    {
        return  MQX_ERROR;
    }

    for(i = 0 ; i < 9 ; i++)
    {
        uint_32 ulValue = 0;
        
        ulValue = ((uint_32)pBuff[i*6 + 4] << 16) | ((uint_32)pBuff[i*6 + 6] << 8) | ((uint_32)pBuff[i*6 + 8]);
        
        FTE_VALUE_setULONG(&pStatus->xCommon.pValue[i], ulValue);
    }
#else
    for(i = 0 ; i < 9 ; i++)
    {
        FTE_VALUE_setULONG(&pStatus->xCommon.pValue[i], _time_get_hwticks());
    }
#endif
    return  MQX_OK;
}

_mqx_uint   fte_ftlm_set(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue)
{
    return  MQX_OK;
}


_mqx_uint   fte_ftlm_setConfig(FTE_OBJECT_PTR  pObj, char_ptr pString)
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
        fte_ftlm_switchCtrl(pObj, TRUE);
    }
    else if (strcmp(pxCmd->text_value, "switch_off") == 0)
    {
        fte_ftlm_switchCtrl(pObj, FALSE);
    }
    else if (strcmp(pxCmd->text_value, "count_reset") == 0)
    {
        fte_ftlm_countReset(pObj);
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

_mqx_uint   fte_ftlm_getConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen)
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

_mqx_uint   fte_ftlm_switchCtrl(FTE_OBJECT_PTR pObj, boolean bSwitchON)
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

_mqx_uint   fte_ftlm_countReset(FTE_OBJECT_PTR pObj)
{
    uint_8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'Z', ']', '\0'};
    uint_32 nCmdLen = 11;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  MQX_OK;
}
