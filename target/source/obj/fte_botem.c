#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

_mqx_uint   fte_botem_pn1500_switchCtrl(FTE_OBJECT_PTR pObj, boolean bSwitchON);
_mqx_uint   fte_botem_pn1500_countReset(FTE_OBJECT_PTR pObj);

FTE_VALUE_TYPE  FTE_BOTEM_PN1500_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_ULONG, 
    FTE_VALUE_TYPE_DIO
};

_mqx_uint   fte_botem_pn1500_request_data(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    uint_8  pCMD[] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'R', ']', '\0'};
    
    pCMD[0] = (uint_8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), TRUE);    

    return  MQX_OK; 
}

_mqx_uint   fte_botem_pn1500_receive_data(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_32     nPeopleCount = 0;
    uint_32     nAccum = 0;
    boolean     bSwitch = 0;
    uint_8      pBuff[64];
    uint_32     nLen;
    uint_32     nPrevPeopleCount = 0;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 23)
    {
        return  MQX_ERROR;
    }

    if ((pBuff[0] != '[') || (pBuff[nLen-1] != ']'))
    {
        return  MQX_ERROR;
    }
    
    pBuff[14] = 0;
    pBuff[16] = 0;
    pBuff[22] = 0;

    nPeopleCount = atoi((char *)&pBuff[9]);
    bSwitch = (pBuff[15] == '1');
    nAccum = atoi((char *)&pBuff[17]);

    FTE_VALUE_getULONG(&pStatus->xCommon.pValue[0], &nPrevPeopleCount);
    if (nPrevPeopleCount != nPeopleCount)
    {
        FTE_OBJ_wasChanged(pObj);
    }
    else
    {
        FTE_OBJ_wasUpdated(pObj);
    }
    
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[0], nPeopleCount);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[1], nAccum);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[2], bSwitch);
    
    return  MQX_OK;
}

_mqx_uint   fte_botem_pn1500_set(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue)
{
    uint_8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T'};
    uint_32 nCmdLen = 8;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    switch(nIndex)
    {
    case    1:
        {
            pCMD[nCmdLen++] = 'Z';        
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
                return  MQX_ERROR;
            }
        }
    }
    
    pCMD[nCmdLen++] = ']';
    pCMD[nCmdLen++] = '\0';
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, TRUE);    
    
    return  MQX_OK;
}


_mqx_uint   fte_botem_pn1500_setConfig(FTE_OBJECT_PTR  pObj, char_ptr pString)
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
        fte_botem_pn1500_switchCtrl(pObj, TRUE);
    }
    else if (strcmp(pxCmd->text_value, "switch_off") == 0)
    {
        fte_botem_pn1500_switchCtrl(pObj, FALSE);
    }
    else if (strcmp(pxCmd->text_value, "count_reset") == 0)
    {
        fte_botem_pn1500_countReset(pObj);
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

_mqx_uint   fte_botem_pn1500_getConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen)
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

_mqx_uint   fte_botem_pn1500_switchCtrl(FTE_OBJECT_PTR pObj, boolean bSwitchON)
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

_mqx_uint   fte_botem_pn1500_countReset(FTE_OBJECT_PTR pObj)
{
    uint_8  pCMD[16] = { '\0', '[', '0', '0', '0', '0', 'B', 'T', 'Z', ']', '\0'};
    uint_32 nCmdLen = 11;
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, nCmdLen, FALSE);    
    
    return  MQX_OK;
}
