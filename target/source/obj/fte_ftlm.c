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
};

_mqx_uint   FTE_FTLM_requestData(FTE_OBJECT_PTR pObj)
{
    /*
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
*/
    return  MQX_OK; 
}
 
_mqx_uint   FTE_FTLM_receiveData(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_8      pBuff[128];
    uint_32     nLen;
    uint_16     uiCRC;
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if ((nLen < 4) || (pBuff[0] != 0x3a))// || (pBuff[nLen - 2] != 0x0d) || (pBuff[nLen - 1] != 0x0a) || (nLen != 4 + pBuff[3] + 4))
    {
        return  MQX_ERROR;
    }
    
    if ((pBuff[1] < 1) || (pBuff[1] > 9))
    {
        return  MQX_ERROR;
    }
    
    uiCRC = fte_crc16(&pBuff[1], 3 + pBuff[3]);
    if (uiCRC != (pBuff[4 + pBuff[3]] | ((uint_16)pBuff[4 + pBuff[3] + 1] << 8)))
    {
        return  MQX_ERROR;
    } 
  
    uint_32 ulValue = 0;
    
    ulValue = ((uint_32)pBuff[4 + 1]) | ((uint_32)pBuff[4 + 3] << 8) | ((uint_32)pBuff[4 + 5] << 16) ;
    
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[pBuff[1]], ulValue);

    return  MQX_OK;
}

_mqx_uint   FTE_FTLM_set(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
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
    pCMD[1] = (uint_8)nIndex+1;
    pCMD[4] = (uint_8)101;
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
            nLen = FTE_UCS_sendAndRecv(pStatus->pUCS, pCMD, sizeof(pCMD), pBuff, sizeof(pBuff), 100, 500);
        } 
        while ((ulTry < 3) && ((nLen < 4) || (pBuff[0] != 0x3a) || (pBuff[1] != (nIndex+1))));
        
        uiCRC = fte_crc16(&pBuff[1], 6);
    } 
    while((ulTry < 3) && (uiCRC != (pBuff[7] | ((uint_16)pBuff[8] << 8))));
    
    if (ulTry >= 3)
    {
        DEBUG("Request Timeout[Retry = %d]\n", ulTry);
        return  MQX_ERROR;
    }

    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[nIndex], ulValue);
    pStatus->xCommon.pValue[nIndex].bChanged = TRUE;
    return  MQX_OK;
}



