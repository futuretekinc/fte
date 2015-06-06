#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "nxjson.h"

static uint_8  FTE_TASCON_HEM12_CRC(uint_8_ptr pData, uint_32 ulDataLen)
{
    uint_8  uiCS = 0;
    
    for(int i = 0 ; i < ulDataLen ; i++)
    {
        uiCS += pData[i];
    }
    
    return  uiCS;
}

static boolean FTE_TASCON_HEM12_06M_isValidFrame(uint_8_ptr pFrame, uint_32 ulLen)
{
    if ((ulLen < 12) || (ulLen != (pFrame[9] + 12)) || (pFrame[0] != 0x69) || (pFrame[ulLen - 1] != 0x96))
    {
        return  FALSE;
    }    
        
    if (FTE_TASCON_HEM12_CRC(pFrame, ulLen - 2) != pFrame[ulLen - 2])
    {
        return  FALSE;
    }
    
    return  TRUE;
}

static _mqx_uint  FTE_TASCON_HEM12_06M_setAddress(FTE_OBJECT_PTR pObj, char_ptr pAddress, uint_32 ulLen)
{
    int     i;
    uint_8  pSensorID[6];
    FTE_HEM12_06M_CONFIG_PTR  pConfig;

    if ((pObj == NULL) || (ulLen != 12))
    {
        return  MQX_ERROR;
    }
    
    pConfig = (FTE_HEM12_06M_CONFIG_PTR)pObj->pConfig;    
    
    memset(pSensorID, 0, sizeof(pSensorID));
    
    for(i = 0 ; i < 12 ; i++)
    {
        uint_8  nValue;
        
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
            return  MQX_ERROR;
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
    
    return  MQX_OK;
}

FTE_VALUE_TYPE  FTE_TASCON_HEM12_valueTypes[] =
{
    FTE_VALUE_TYPE_PWR_KW
};

_mqx_uint   FTE_TASCON_HEM12_request(FTE_OBJECT_PTR pObj)
{
    FTE_HEM12_06M_STATUS_PTR    pStatus = (FTE_HEM12_06M_STATUS_PTR)pObj->pStatus;
    uint_8      pBuff[32];
    uint_32     nLen = 0;
    uint_8      nCS = 0;
    
    pBuff[nLen++] = 0x69;
    pBuff[nLen++] = 0xAA;
    pBuff[nLen++] = 0xAA;
    pBuff[nLen++] = 0xAA;
    pBuff[nLen++] = 0xAA;
    pBuff[nLen++] = 0xAA;
    pBuff[nLen++] = 0xAA;
    pBuff[nLen++] = 0x69; 
    pBuff[nLen++] = 0x01;
    pBuff[nLen++] = 0x02;
    pBuff[nLen++] = 0x47;
    pBuff[nLen++] = 0x14;
    
    for(int i = 0 ; i < nLen ; i++)
    {
        nCS += pBuff[i];
    }
    
    pBuff[nLen++] = nCS;
    pBuff[nLen++] = 0x96;
    pBuff[nLen++] = 0x96;

    
    FTE_UCS_clear(pStatus->xGUS.pUCS);    
    FTE_UCS_send(pStatus->xGUS.pUCS, pBuff, nLen, FALSE);    

    return  MQX_OK;
}

_mqx_uint     FTE_TASCON_HEM12_received(FTE_OBJECT_PTR pObj)
{
    FTE_HEM12_06M_STATUS_PTR  pStatus = (FTE_HEM12_06M_STATUS_PTR)pObj->pStatus;
    uint_32             nPower, nSkip = 0, nValue;    
    uint_8              pBuff[64];
    char_ptr            pHead;
    uint_32             nLen;
    uint_8              nCS = 0;
    
    nLen = FTE_UCS_recv(pStatus->xGUS.pUCS, pBuff, sizeof(pBuff));
    if (nLen == 0)
    {
        return  MQX_ERROR;
    }
    
    pHead = (char_ptr)pBuff;
    
    
    for(int i = 0 ; i < nLen ; i++)
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
        (pHead[0] != 0x69) || 
        (pHead[7] != 0x69) || 
        (nLen < (12 + pHead[9])))
    {
        return  MQX_INVALID_CHECKSUM;
    }
    
    nLen = 12 + pHead[9];

    for(int i = 0 ; i < nLen - 2 ; i++)
    {
        nCS += pHead[i];
    }
    
    if ((pHead[nLen - 2] != nCS) ||
        (pHead[nLen - 1] != 0x96))
    {
        return  MQX_INVALID_CHECKSUM;
    }
    
#if 0
    for(int i = 0 ; i < 6 ; i++)
    {
        if (pConfig->pAddress[i] != pHead[6 - i])
        {
            return  MQX_INVALID_PARAMETER;
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
   
    FTE_VALUE_setPowerKW(&pStatus->xGUS.xCommon.pValue[0], nPower * 10);
    
    return  MQX_OK;
}




FTE_VALUE_TYPE  FTE_TASCON_HEM12_06M_valueTypes[] =
{
    FTE_VALUE_TYPE_PWR_KWH,
    FTE_VALUE_TYPE_PWR_KW,
    FTE_VALUE_TYPE_VOLTAGE,
    FTE_VALUE_TYPE_CURRENT
};

_mqx_uint     FTE_TASCON_HEM12_06M_FRAME_responsePower(uint_8_ptr pFrame, uint_32 ulLen, uint_32_ptr pulAmountOfPower, uint_32_ptr pulPower)
{
    uint_32     ulValue;
    uint_32     ulAmountOfPower;
    uint_32     ulPower;
    
    if ( !FTE_TASCON_HEM12_06M_isValidFrame(pFrame, ulLen) )
    {
        return  MQX_INVALID_CHECKSUM;
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
    
    return  MQX_OK;
}

_mqx_uint     FTE_TASCON_HEM12_06M_FRAME_responseVoltage(uint_8_ptr pFrame, uint_32 ulLen, uint_32_ptr pulVoltage)
{
    uint_32     ulValue;
    uint_32     ulVoltage;

    if ( !FTE_TASCON_HEM12_06M_isValidFrame(pFrame, ulLen) )
    {
        return  MQX_INVALID_CHECKSUM;
    }
    
    ulValue = (pFrame[23] - 0x66);
    ulVoltage = ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[22] - 0x66);
    ulVoltage = ulVoltage * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
  
    *pulVoltage = ulVoltage * 100;
    
    return  MQX_OK;
}

_mqx_uint     FTE_TASCON_HEM12_06M_FRAME_responseCurrent(uint_8_ptr pFrame, uint_32 ulLen, uint_32_ptr pulCurrent)
{
    uint_32     ulValue;
    uint_32     ulCurrent;
     
    if ( !FTE_TASCON_HEM12_06M_isValidFrame(pFrame, ulLen) )
    {
        return  MQX_INVALID_CHECKSUM;
    }

    ulValue = (pFrame[24] - 0x66);
    ulCurrent = ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[23] - 0x66);
    ulCurrent = ulCurrent * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
    ulValue = (pFrame[22] - 0x66);
    ulCurrent = ulCurrent * 100 + ((ulValue >> 4) * 10) + (ulValue & 0x0F);
  
    *pulCurrent = ulCurrent;
    
    return  MQX_OK;
}

_mqx_uint   FTE_TASCON_HEM12_06M_FRAME_create(uint_8_ptr pAddress, uint_32 ulType, uint_8_ptr pBuff, uint_32 ulBuffLen, uint_32_ptr pulFrameSize)
{
    uint_8  ucCRC;
    const uint_8  pBaseFrame[] = {  0x69, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x69,
                                    0x78, 0x0e, 0x9c, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                                    0xdd, 0x9c, 0x34, 0x35, 0x00, 0x00, 0x00, 0x00,
                                    0xFF, 0x96, 0x00 };

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
    
    return  MQX_OK;
} 

static    const char_ptr    pStringCmd = "cmd";
static    const char_ptr    pStringAddress = "address";

_mqx_uint   FTE_TASCON_HEM12_06M_setConfig(FTE_OBJECT_PTR  pObj, char_ptr pString)
{
    FTE_HEM12_06M_CONFIG_PTR  pConfig;

    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    pConfig = (FTE_HEM12_06M_CONFIG_PTR)pObj->pConfig;
    const nx_json* pxJSON = nx_json_parse_utf8(pString);
    if (pxJSON == NULL)
    {
        return  MQX_ERROR;
    } 
    
    const nx_json* pxCmd = nx_json_get(pxJSON, pStringCmd);
    const nx_json* pxAddress = nx_json_get(pxJSON, pStringAddress);

    if ((pxCmd == NULL) || (pxCmd->type != NX_JSON_STRING))
    {
        goto error;
    }
    
    if (strcmp(pxCmd->text_value, "set_addr") == 0)
    {
        int     i;
        uint_8  pValues[12];
        
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
   
    return  MQX_OK;
    
error:
    
    nx_json_free(pxJSON);
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_TASCON_HEM12_06M_getConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 ulBuffLen)
{
    FTE_HEM12_06M_CONFIG_PTR    pConfig;
    FTE_JSON_VALUE_PTR          pJOSNObject;
    FTE_JSON_VALUE_PTR          pJOSNValue;
    char                        pIDString[13];
    
    if (pObj == NULL)
    {
        return  MQX_ERROR;
    }
    
    pConfig = (FTE_HEM12_06M_CONFIG_PTR)pObj->pConfig;
    
    pJOSNObject = FTE_JSON_VALUE_createObject(1);
    if (pJOSNObject == NULL)
    {
        return  MQX_ERROR;
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
        return  MQX_ERROR;
    }
    
    FTE_JSON_VALUE_snprint(pBuff, ulBuffLen, pJOSNObject);    
    FTE_JSON_VALUE_destroy(pJOSNObject);
    
    return  MQX_OK;
}

_mqx_uint   FTE_TASCON_HEM12_06M_request(FTE_OBJECT_PTR pObj)
{
    FTE_HEM12_06M_STATUS_PTR    pStatus = (FTE_HEM12_06M_STATUS_PTR)pObj->pStatus;
    FTE_HEM12_06M_CONFIG_PTR    pConfig = (FTE_HEM12_06M_CONFIG_PTR)pObj->pConfig;
    uint_32                     nFieldType, ulReqLen;
    static uint_8               pReqBuff[64];
    uint_8                      pBuff[64];
    uint_32                     ulLen;
    uint_32                     ulVoltage, ulCurrent, ulAmountOfPower, ulPower;
    
    pStatus->xGUS.xRet = MQX_OK;
    for(nFieldType = FTE_HEM12_FIELD_POWER ; nFieldType <=  FTE_HEM12_FIELD_CURRENT; nFieldType++)
    {
        pStatus->nField = nFieldType;
    
        FTE_TASCON_HEM12_06M_FRAME_create(pConfig->pSensorID, pStatus->nField, pReqBuff, sizeof(pReqBuff), &ulReqLen);    
        FTE_UCS_clear(pStatus->xGUS.pUCS);    
        ulLen = FTE_UCS_sendAndRecv(pStatus->xGUS.pUCS, pReqBuff, ulReqLen, pBuff, sizeof(pBuff), 100, 1000);            

        switch(pStatus->nField)
        {
        case    FTE_HEM12_FIELD_VOLTAGE:
            {
                if (FTE_TASCON_HEM12_06M_FRAME_responseVoltage(pBuff, ulLen, &ulVoltage) == MQX_OK)
                {
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[2], ulVoltage);
                }
                else
                {
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[2], FALSE);
                    pStatus->xGUS.xRet = MQX_ERROR;
                }
            }
            break;

        case    FTE_HEM12_FIELD_CURRENT:
            {
                if (FTE_TASCON_HEM12_06M_FRAME_responseCurrent(pBuff, ulLen, &ulCurrent) == MQX_OK)
                {
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[3], ulCurrent);
                }
                else
                {
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[3], FALSE);
                    pStatus->xGUS.xRet = MQX_ERROR;
                }
            }
            break;
            
        case    FTE_HEM12_FIELD_POWER:
            {
                if (FTE_TASCON_HEM12_06M_FRAME_responsePower(pBuff, ulLen, &ulAmountOfPower, &ulPower) == MQX_OK)
                {
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[0], ulAmountOfPower);
                    FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[1], ulPower);
                }
                else
                {
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
                    FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[1], FALSE);
                    pStatus->xGUS.xRet = MQX_ERROR;
                }
            }
            break;
            
        default:
            pStatus->xGUS.xRet = MQX_ERROR;
        }    
    }
    
    return  pStatus->xGUS.xRet;

}

_mqx_uint     FTE_TASCON_HEM12_06M_received(FTE_OBJECT_PTR pObj)
{
    FTE_HEM12_06M_STATUS_PTR    pStatus = (FTE_HEM12_06M_STATUS_PTR)pObj->pStatus;
    /*
    uint_8                      pBuff[64];
    uint_32                     ulLen;
    uint_32                     ulVoltage, ulCurrent, ulAmountOfPower, ulPower;

    ulLen = FTE_UCS_recv(pStatus->xGUS.pUCS, pBuff, sizeof(pBuff));

    switch(pStatus->nField)
    {
    case    FTE_HEM12_FIELD_VOLTAGE:
        {
            if (FTE_TASCON_HEM12_06M_FRAME_responseVoltage(pBuff, ulLen, &ulVoltage) == MQX_OK)
            {
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[2], ulVoltage);
            }
            else
            {
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[2], FALSE);
                pStatus->xGUS.xRet = MQX_ERROR;
            }
        }
        break;

    case    FTE_HEM12_FIELD_CURRENT:
        {
            if (FTE_TASCON_HEM12_06M_FRAME_responseCurrent(pBuff, ulLen, &ulCurrent) == MQX_OK)
            {
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[3], ulCurrent);
            }
            else
            {
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[3], FALSE);
                pStatus->xGUS.xRet = MQX_ERROR;
            }
        }
        break;
        
    case    FTE_HEM12_FIELD_POWER:
        {
            if (FTE_TASCON_HEM12_06M_FRAME_responsePower(pBuff, ulLen, &ulAmountOfPower, &ulPower) == MQX_OK)
            {
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[0], ulAmountOfPower);
                FTE_VALUE_setULONG(&pStatus->xGUS.xCommon.pValue[1], ulPower);
            }
            else
            {
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[0], FALSE);
                FTE_VALUE_setValid(&pStatus->xGUS.xCommon.pValue[1], FALSE);
                pStatus->xGUS.xRet = MQX_ERROR;
            }
        }
        break;
        
    default:
        pStatus->xGUS.xRet = MQX_ERROR;
    }
    */
    return  pStatus->xGUS.xRet;
}

int_32  FTE_TASCON_HEM12_SHELL_cmd(int_32 argc, char_ptr argv[])
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
                uint_32 count = FTE_OBJ_count(FTE_OBJ_TYPE_MASK, FTE_OBJ_TYPE_MULTI_HEM12_06M, FALSE);
                uint_32 i;

                printf("%-8s %-16s %-24s %-8s %-16s %-s\n", 
                        "ID", "TYPE", "NAME", "STATUS", "VALUE", "TIME");
                for(i = 0 ; i < count ; i++)
                {
                    FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MASK, FTE_OBJ_TYPE_MULTI_HEM12_06M, i, FALSE);
                    FTE_HEM12_06M_CONFIG_PTR pConfig = (FTE_HEM12_06M_CONFIG_PTR)pObj->pConfig;
                    FTE_HEM12_06M_STATUS_PTR pStatus = (FTE_HEM12_06M_STATUS_PTR)pObj->pStatus;
                    
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
                            char        pTimeString[64];
                            char        pValueString[32];
                            char        pUnitString[8];
                            
                            FTE_VALUE_toString(pStatus->xGUS.xCommon.pValue, pValueString, sizeof(pValueString));
                            FTE_VALUE_unit(pStatus->xGUS.xCommon.pValue, pUnitString, sizeof(pUnitString));
                            FTE_VALUE_getTimeStamp(pStatus->xGUS.xCommon.pValue, &xTime);
                            FTE_TIME_toString(&xTime, pTimeString, sizeof(pTimeString));
                            printf(" %8s %-7s %s", pValueString, pUnitString, pTimeString);
                        }
                        
                        if (pObj->pAction->f_get_statistic != NULL)
                        {
                            FTE_OBJECT_STATISTICS   xStatistics;
                            uint_32                 nRatio;
                            
                            pObj->pAction->f_get_statistic(pObj, &xStatistics);
                            nRatio = (xStatistics.nTotalTrial - xStatistics.nTotalFail) * 10000 / xStatistics.nTotalTrial;
                            
                            printf(" %3d.%02d%%(%d, %d, %d)", nRatio/100, nRatio%100, xStatistics.nTotalTrial, xStatistics.nTotalFail, xStatistics.nPartialFail);
                        }
                        printf("\n");
                    }
                }
            }
            break;
         case    4:
            {
                uint_32  nOID = 0;
                Shell_parse_hexnum(argv[1], &nOID);
                
                FTE_OBJECT_PTR  pObj = FTE_OBJ_get(nOID);                    
                if (pObj == NULL)
                {
                    printf("Invalid OID[%08x]\n", nOID);
                }
                
                if (strcmp(argv[2], "set_addr") == 0)
                {
 
                    if (FTE_TASCON_HEM12_06M_setAddress(pObj, argv[3], strlen(argv[3])) == MQX_OK)
                    {
                        FTE_CFG_OBJ_save(pObj);
                    }
                    else
                    {
                        printf("Invalid Adddress[%s]\n", argv[3]);
                    }
                }
            }
            break;
                       
        default:
            print_usage = TRUE;
        }
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<command>]\n", argv[0]);
        }
        else
        {
            printf ("Usage: %s [<command>]\n", argv[0]);
            printf ("  Commands:\n");
            printf ("    <id> info\n");
            printf ("        Show object information.\n");
        }
    }
    return   return_code;
}