#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#include "fte_value.h"

FTE_VALUE_TYPE  FTE_ELT_AQM100_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_HUMIDITY,
    FTE_VALUE_TYPE_DIO
};

_mqx_uint   fte_elt_aqm100_request_data(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_16 uiCRC;    
    uint_8  pCMD[9] = { 0x04, 0x03, 0x00, 0x50, 0x00, 0x04, 0x00, 0x00, 0x00};

    uiCRC = fte_crc16(pCMD, 6);
    pCMD[6] = (uiCRC     ) & 0xFF;
    pCMD[7] = (uiCRC >> 8) & 0xFF;
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD,sizeof(pCMD), FALSE);    

    return  MQX_OK;
}

_mqx_uint   fte_elt_aqm100_receive_data(FTE_OBJECT_PTR pObj)
{
#if 1
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_32     nCO2 = 0;
    int_32      nTemp = 0;
    uint_32     nHumi = 0;
    uint_32     nVOC = 0;
    uint_8      pBuff[64];
    uint_32     nLen;
    uint_16     uiCRC = 0;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 13)
    {
        return  MQX_ERROR;
    }

    uiCRC = fte_crc16(pBuff, 11);
    if (uiCRC != (pBuff[11] | ((uint_16)pBuff[12] << 8)))
    {
        return  MQX_ERROR;
    }
    
    nCO2  = ((uint_32)pBuff[3] << 8) | pBuff[4];
    nTemp = (((int_32)pBuff[5] << 8) | pBuff[6]) * 10;
    nHumi = (uint_32)pBuff[8] * 100;
    switch (pBuff[10])
    {
    case 0x00:     nVOC = 0; break;
    case 0x10:     nVOC = 1; break;
    case 0x40:     nVOC = 2; break;
    default:       nVOC = 3; break;
    }
#else
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_32     nIndex = 0;
    uint_32     nCO2 = 0;
    int_32      nTemp = 0;
    uint_32     nHumi = 0;
    uint_32     nVOC = 0;
    uint_8      pBuff[64];
    uint_32     nLen;
    uint_8      nCRC = 0;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 12)
    {
        return  MQX_ERROR;
    }

    for(nIndex = 0 ; nIndex < 10 ; nIndex++)
    {
        nCRC += pBuff[nIndex];
    }
    
    if ((pBuff[0] != 0xFA) || (pBuff[11] != 0x05) || (nCRC != pBuff[10]))
    {
        return  MQX_ERROR;
    }
    
    nCO2  = ((uint_32)pBuff[4] << 8) | pBuff[5];
    nTemp = (((int_32)pBuff[6] << 8) | pBuff[7]) * 10;
    nHumi = (uint_32)pBuff[8] * 100;
    switch (pBuff[9])
    {
    case 0x00:     nVOC = 0; break;
    case 0x10:     nVOC = 1; break;
    case 0x40:     nVOC = 2; break;
    default:       nVOC = 3; break;
    }
#endif
    FTE_VALUE_setPPM(&pStatus->xCommon.pValue[0], nCO2);
    FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[1], nTemp);
    FTE_VALUE_setHumidity(&pStatus->xCommon.pValue[2], nHumi);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[3], nVOC);
    
    return  MQX_OK;
}
