#include "fte_target.h"

FTE_VALUE_TYPE   FTE_PM1001_valueTypes[] =
{
    FTE_VALUE_TYPE_ULONG
};

const FTE_GUS_MODEL_INFO    FTE_PM1001_GUSModelInfo =
{
    .nModel     = FTE_GUS_MODEL_PM1001,
    .pName      = "PM1001",
    .nFieldCount= 1,
    .pValueTypes= FTE_PM1001_valueTypes,
    .fRequest   = FTE_PM1001_request,
    .fReceived  = FTE_PM1001_received
};



FTE_RET   FTE_PM1001_request
(
    FTE_OBJECT_PTR  pObj
)
{
    static FTE_UINT8       pCmd[] = {0x11, 0x01, 0x01, 0xED};
    FTE_GUS_STATUS_PTR  pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus; 
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, pCmd, sizeof(pCmd), FALSE);    
        
    return  FTE_RET_OK;
}

FTE_RET   FTE_PM1001_received
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_UINT32     nValue;
    FTE_UINT8      pBuff[64];
    FTE_UINT32     nLen;
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen == 0)
    {
        return  FTE_RET_ERROR;
    }

    nValue = pBuff[3] * 0x01000000 + pBuff[4] * 0x00010000 + pBuff[5] * 0x00000100 + pBuff[6];    
    nValue = (FTE_UINT32)((nValue * 3.528) * 10);

    FTE_VALUE_setULONG(pStatus->xCommon.pValue, nValue);
    
    return  FTE_RET_OK;
}
