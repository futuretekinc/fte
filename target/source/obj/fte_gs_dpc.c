#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
 
#define FTE_GS_DPC_HL_FRAME_FIELD_VERION            0
#define FTE_GS_DPC_HL_FRAME_FIELD_MODEL1            1
#define FTE_GS_DPC_HL_FRAME_FIELD_MODEL2            2
#define FTE_GS_DPC_HL_FRAME_FIELD_RELAY_STATE       3
#define FTE_GS_DPC_HL_FRAME_FIELD_ERROR             4
#define FTE_GS_DPC_HL_FRAME_FIELD_HP                5
#define FTE_GS_DPC_HL_FRAME_FIELD_LP                6
#define FTE_GS_DPC_HL_FRAME_FIELD_HP_TEMP           9
#define FTE_GS_DPC_HL_FRAME_FIELD_LP_TEMP           10

typedef struct
{
    uint_16 usVersion;
    uint_16 usModel;
    uint_16 usID;
    uint_16 usRelay;
    uint_16 usError;
    int_16  nHP;
    int_16  nLP;
    uint_16 usReserved[2];
    int_16  sHPTemp;
    int_16  sLPTemp;
}   FTE_GS_DPC_HL_FRAME, _PTR_ FTE_GS_DPC_HL_FRAME_PTR;

FTE_VALUE_TYPE  FTE_GS_DPC_HL_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       FTE_VALUE_TYPE_ULONG,       
    FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,         FTE_VALUE_TYPE_PRESSURE,       FTE_VALUE_TYPE_PRESSURE,       FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_TEMPERATURE
};

_mqx_uint   FTE_GS_DPC_HL_request(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GUS_CONFIG_PTR  pConfig = (FTE_GUS_CONFIG_PTR)pObj->pConfig;
    
    uint_8  pCMD[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x0f, 0x00};
    
    pCMD[0] = (uint_8)pConfig->nSensorID;
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCMD, sizeof(pCMD), FALSE);    

    return  MQX_OK;
}

_mqx_uint   FTE_GS_DPC_HL_received(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR      pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_GS_DPC_HL_FRAME_PTR pFrame = NULL;
    uint_8      pBuff[128];
    uint_32     nLen;
    uint_16     uiCRC;
    
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen != 31)
    {
        return  MQX_ERROR; 
    }

    if ((pBuff[0] != 0x01))
    {
        return  MQX_ERROR;
    }
    
    uiCRC = fte_crc16(pBuff, 29);
    if (uiCRC != (pBuff[29] | ((uint_16)pBuff[30] << 8)))
    {
        return  MQX_ERROR;
    }
    
    pFrame = (FTE_GS_DPC_HL_FRAME_PTR)&pBuff[3];
    // Temperature
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_VERSION],       ntohs(&pFrame->usVersion));
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_MODEL0],        pFrame->usModel);
    FTE_VALUE_setULONG(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_ID],            ntohs(&pFrame->usID));       
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HPS_STATE],       (pFrame->usRelay & 0x01));
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LPS_STATE],       (pFrame->usRelay >> 1) & 0x01);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HPC_STATE],       (pFrame->usRelay >> 2) & 0x01);
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HPS_ERROR],       (pFrame->usError & 0x01));
    FTE_VALUE_setDIO(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LPS_ERROR],       (pFrame->usError >> 1) & 0x01);
    FTE_VALUE_setPressure(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HP],         (short)ntohs(&pFrame->nHP));
    FTE_VALUE_setPressure(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LP],         (short)ntohs(&pFrame->nLP));    
    FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_HP_TEMP], (short)ntohs(&pFrame->sHPTemp) * 10);
    FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[FTE_GS_DPC_HL_INDEX_LP_TEMP], (short)ntohs(&pFrame->sLPTemp) * 10);    
    
    return  MQX_OK;
}

_mqx_uint   FTE_GS_DPC_HL_set(FTE_OBJECT_PTR pObj, uint_32 nIndex, FTE_VALUE_PTR pValue)
{
//    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    return  MQX_ERROR;
}