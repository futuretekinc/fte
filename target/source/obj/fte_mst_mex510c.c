#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

#define FTE_MST_MEX510C_FRAME_FIELD_INPUT               1
#define FTE_MST_MEX510C_FRAME_FIELD_ALERT1              2
#define FTE_MST_MEX510C_FRAME_FIELD_ALERT2              3
#define FTE_MST_MEX510C_FRAME_FIELD_OP                  4
#define FTE_MST_MEX510C_FRAME_FIELD_OUTPUT_STATE        5
#define FTE_MST_MEX510C_FRAME_FIELD_CURR_TEMP           6
#define FTE_MST_MEX510C_FRAME_FIELD_SETUP_TEMP          7
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_DEVIATION      8
#define FTE_MST_MEX510C_FRAME_FIELD_OP_DELAY            9
#define FTE_MST_MEX510C_FRAME_FIELD_STOP_DELAY          10
#define FTE_MST_MEX510C_FRAME_FIELD_TEMP_CAL            11
#define FTE_MST_MEX510C_FRAME_FIELD_HIGH_TEMP           12
#define FTE_MST_MEX510C_FRAME_FIELD_LOW_TEMP            13
#define FTE_MST_MEX510C_FRAME_FIELD_HEAT_DEVIATION      14
#define FTE_MST_MEX510C_FRAME_FIELD_FAN_CTRL            15
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_OP_DELAY       16
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_STOP_DELAY     17
#define FTE_MST_MEX510C_FRAME_FIELD_COOL_REOP_DELAY     18
#define FTE_MST_MEX510C_FRAME_FIELD_LP_ALERT_DELAY      19
#define FTE_MST_MEX510C_FRAME_FIELD_DEVICE_ID           24
#define FTE_MST_MEX510C_FRAME_FIELD_DIRECT_CTRL         32

FTE_VALUE_TYPE  FTE_MST_MEX510C_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_TEMPERATURE, FTE_VALUE_TYPE_TEMPERATURE,   FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE, FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_TEMPERATURE,   FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG
};

static const   char_ptr    pStringTargetTemperature= "targetTemperature";
static const   char_ptr    pStringCoolingDeviation = "coolingDeviation";
static const   char_ptr    pStringHeatingDeviation = "heatingDeviation";

_mqx_uint   FTE_MST_MEX510C_setConfig(FTE_OBJECT_PTR  pObj, char_ptr pString)
{
    int_32  nValue = 0;
    FTE_GUS_STATUS_PTR  pStatus;

    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
   
    const nx_json* pxJSON = nx_json_parse_utf8(pString);
    if (pxJSON == NULL)
    {
        return  MQX_ERROR;
    } 
    
    const nx_json* pxTemperature    = nx_json_get(pxJSON, pStringTargetTemperature);
    const nx_json* pxCoolDeviation  = nx_json_get(pxJSON, pStringCoolingDeviation);
    const nx_json* pxHeatDeviation  = nx_json_get(pxJSON, pStringHeatingDeviation);

    if ((pxTemperature != NULL) && (pxTemperature->type != NX_JSON_NULL))
    {
        uint_32 nLen;
        uint_16 uiCRC = 0;  
        uint_8  pBuff[8];
        uint_8  pCMD[9] = { 0x01, 0x06, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};

        if (pxTemperature->type == NX_JSON_DOUBLE)
        {
            nValue = (uint_32)(pxTemperature->dbl_value * 10);
        }
        else if (pxTemperature->type == NX_JSON_INTEGER)
        {
            nValue = (uint_32)(pxTemperature->int_value * 10);
        }
        else if (pxTemperature->type == NX_JSON_STRING)
        {
            nValue = (uint_32)(strtod(pxTemperature->text_value, NULL) * 10);
        }
        else
        {
            return  MQX_ERROR;
        }        
        
        pCMD[4] = (nValue >> 8) & 0xFF;
        pCMD[5] = (nValue     ) & 0xFF;

        uiCRC = fte_crc16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 200, 1000);
        if (nLen == sizeof(pBuff))
        {
            if (memcmp(pCMD, pBuff, sizeof(pBuff)) == 0)
            {
                FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_TEMPERATURE], nValue*10);
            }
        }
    }
    
    if ((pxCoolDeviation != NULL) && (pxCoolDeviation->type != NX_JSON_NULL))
    {
        uint_32 nLen;
        uint_16 uiCRC = 0;  
        uint_8  pBuff[8];
        uint_8  pCMD[9] = { 0x01, 0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        if (pxCoolDeviation->type == NX_JSON_DOUBLE)
        {
            nValue = (uint_32)(pxCoolDeviation->dbl_value * 10);
        }
        else if (pxCoolDeviation->type == NX_JSON_INTEGER)
        {
            nValue = (uint_32)(pxCoolDeviation->int_value * 10);
        }
        else if (pxCoolDeviation->type == NX_JSON_STRING)
        {
            nValue = (uint_32)(strtod(pxCoolDeviation->text_value, NULL) * 10);
        }
        else
        {
            return  MQX_ERROR;
        }        

        pCMD[4] = (nValue >> 8) & 0xFF;
        pCMD[5] = (nValue     ) & 0xFF;

        uiCRC = fte_crc16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 200, 1000);
        if (nLen == sizeof(pBuff))
        {
            if (memcmp(pCMD, pBuff, sizeof(pBuff)) == 0)
            {
                FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION], nValue*10);
            }
        }
    }
    
    if ((pxHeatDeviation != NULL) && (pxHeatDeviation->type != NX_JSON_NULL))
    {
        uint_32 nLen;
        uint_16 uiCRC = 0;  
        uint_8  pBuff[8];
        uint_8  pCMD[9] = { 0x01, 0x06, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00};

        if (pxHeatDeviation->type == NX_JSON_DOUBLE)
        {
            nValue = (uint_32)(pxHeatDeviation->dbl_value * 10);
        }
        else if (pxHeatDeviation->type == NX_JSON_INTEGER)
        {
            nValue = (uint_32)(pxHeatDeviation->int_value * 10);
        }
        else if (pxHeatDeviation->type == NX_JSON_STRING)
        {
            nValue = (uint_32)(strtod(pxHeatDeviation->text_value, NULL) * 10);
        }
        else
        {
            return  MQX_ERROR;
        }        

        pCMD[4] = (nValue >> 8) & 0xFF;
        pCMD[5] = (nValue     ) & 0xFF;

        uiCRC = fte_crc16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 200, 1000);
        if (nLen == sizeof(pBuff))
        {
            if (memcmp(pCMD, pBuff, sizeof(pBuff)) == 0)
            {
                FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION], nValue*10);
            }
        }
    }
    
    nx_json_free(pxJSON);
   
    return  MQX_OK;
}

_mqx_uint   FTE_MST_MEX510C_getConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen)
{
    FTE_GUS_STATUS_PTR  pStatus;
    FTE_JSON_VALUE_PTR  pJOSNObject;
    FTE_JSON_VALUE_PTR  pJOSNValue;
    char                pValueString[32];
    
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    pJOSNObject = FTE_JSON_VALUE_createObject(3);
    if (pJOSNObject == NULL)
    {
        return  MQX_ERROR;
    }
    
    FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_TEMPERATURE], pValueString, sizeof(pValueString));
    pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringTargetTemperature, pJOSNValue);
        
    FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION], pValueString, sizeof(pValueString));
    pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringCoolingDeviation, pJOSNValue);
        
    FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION], pValueString, sizeof(pValueString));
    pJOSNValue = FTE_JSON_VALUE_createString(pValueString);
    FTE_JSON_OBJECT_setPair((FTE_JSON_OBJECT_PTR)pJOSNObject, pStringHeatingDeviation, pJOSNValue);
        
    if (FTE_JSON_VALUE_buffSize(pJOSNObject) >= ulBuffLen)
    {
        FTE_JSON_VALUE_destroy(pJOSNObject);
        return  MQX_ERROR;
    }
    
    FTE_JSON_VALUE_snprint(pBuff, ulBuffLen, pJOSNObject);
    
    FTE_JSON_VALUE_destroy(pJOSNObject);
    
    return  MQX_OK;
}

_mqx_uint   fte_mst_mex510c_request_data(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    uint_8  pCMD[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x21, 0x85, 0xd2, 0x00};
    
    pCMD[0] = (uint_8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), FALSE);    

    return  MQX_OK;
}

_mqx_uint   fte_mst_mex510c_receive_data(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_8      pBuff[128];
    uint_32     nLen;
    uint_16     uiCRC;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 71)
    {
        return  MQX_ERROR; 
    }

    if ((pBuff[0] != 0x01))
    {
        return  MQX_ERROR;
    }
    
    uiCRC = fte_crc16(pBuff, 69);
    if (uiCRC != (pBuff[69] | ((uint_16)pBuff[70] << 8)))
    {
        return  MQX_ERROR;
    }
    
    // Temperature
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DEVICE_ID], pBuff[FTE_MST_MEX510C_FRAME_FIELD_DEVICE_ID * 2 + 3]);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_ALARM], (pBuff[FTE_MST_MEX510C_FRAME_FIELD_ALERT2 * 2 + 3] >> 7) & 01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_OPERATION], pBuff[FTE_MST_MEX510C_FRAME_FIELD_OP * 2 + 4] & 01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_TEMPERATURE],(((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_CURR_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_CURR_TEMP * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION],(((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_COOL_DEVIATION * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_COOL_DEVIATION * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION],(((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_HEAT_DEVIATION * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_HEAT_DEVIATION * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_TEMPERATURE], (((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_SETUP_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_SETUP_TEMP * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_HIGH_TEMP], (((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_HIGH_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_HIGH_TEMP * 2 + 4])*10);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_SET_LOW_TEMP], (((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_LOW_TEMP * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_LOW_TEMP * 2 + 4])*10);
    
    uint_16 nOutputState = ((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_OUTPUT_STATE * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_OUTPUT_STATE * 2 + 4];
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_HEAT],(nOutputState >> 4) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_DHEAT], (nOutputState >> 3) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_FAN], (nOutputState >> 2) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_SOLVALVE], (nOutputState >> 1) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_STATE_COMP], (nOutputState >> 0) & 0x01);
    
    uint_16 nDirectCtrl = ((uint_16)pBuff[FTE_MST_MEX510C_FRAME_FIELD_DIRECT_CTRL * 2 + 3] << 8) | pBuff[FTE_MST_MEX510C_FRAME_FIELD_DIRECT_CTRL * 2 + 4];

    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE], (nDirectCtrl >> 15) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_A], (nDirectCtrl >> 0) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_B], (nDirectCtrl >> 1) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_C], (nDirectCtrl >> 2) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_D], (nDirectCtrl >> 3) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_E], (nDirectCtrl >> 4) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_F], (nDirectCtrl >> 5) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_G], (nDirectCtrl >> 6) & 0x01);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H], (nDirectCtrl >> 7) & 0x01);
    
    return  MQX_OK;
}

_mqx_uint   fte_mst_mex510c_set(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;

    if (FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE <= nIndex && nIndex <= FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H)
    {
        uint_32 ulValue;
        uint_16 uiValue, uiCRC;
        uint_8  pCMD[9] = { 0x01, 0x06, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        FTE_VALUE_copy(&pStatus->xCommon.pValue[nIndex], pValue);
        
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE], &ulValue);        
        uiValue = ((ulValue & 0x01) << 15);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_A], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 0);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_B], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 1);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_C], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 2);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_D], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 3);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_E], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 4);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_F], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 5);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_G], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 6);
        FTE_VALUE_getULONG(&pStatus->xCommon.pValue[FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H], &ulValue);        
        uiValue |= ((ulValue & 0x01) << 7);

        pCMD[4] = (uiValue >> 8) & 0xFF;
        pCMD[5] = (uiValue     ) & 0xFF;

        uiCRC = fte_crc16(pCMD, 6);
        pCMD[6] = (uiCRC     ) & 0xFF;
        pCMD[7] = (uiCRC >> 8) & 0xFF;
        
        FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), FALSE);               

        return  MQX_OK;
    }
    
    return  MQX_ERROR;
}