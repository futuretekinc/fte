#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"

FTE_VALUE_TYPE  FTE_SOHA_MV250_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM,
    FTE_VALUE_TYPE_TEMPERATURE,
    FTE_VALUE_TYPE_HUMIDITY
};

_mqx_uint   FTE_SOHA_init(FTE_OBJECT_PTR pObj)
{
//    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
//    const   uint_8  pInitCmd[] = {0x02, 0x00, 0x53, 0x00, 0x31, 0x00, 0x00, 0x86, 0x0d, 0x0a, 0x00};

//    FTE_UCS_send(pStatus->pUCS, pInitCmd, sizeof(pInitCmd), FALSE);
    
    return  MQX_OK;
}
 
_mqx_uint   FTE_SOHA_request(FTE_OBJECT_PTR pObj)
{
#if 0
#if 0
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    uint_32 i;
    
    for(i = 0 ; i < 10 ; i++)
    {
        uint_32 nLen;
        
        nLen  = FTE_UCS_clear(pStatus->pUCS);
        if (nLen == 0)
        {
            break;
        }
        printf("Clear = %d\n", nLen);    
        _time_delay(100);
    }
#else
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    const   uint_8  pReqCmd[] = {0x02, 0x00, 0x52, 0x00, 0x00, 0x00, 0x00, 0x54, 0x0d, 0x0a, 0x00};

    FTE_UCS_clear(pStatus->pUCS);
    FTE_UCS_send(pStatus->pUCS, pReqCmd, sizeof(pReqCmd), FALSE);
#endif
#endif
    return  MQX_OK;
}

static  uint_8  pStaticBuff[FTE_SH_MV250_FRAME_LENGTH * 2];

_mqx_uint   FTE_SOHA_received(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    int_32      nIndex = 0;
    uint_32     nPPM = 0;
    int_32      nTemp = 0;
    uint_32     nHumi = 0;
    uint_8      pBuff[FTE_SH_MV250_FRAME_LENGTH];
    uint_32     nLen;
    uint_8      nCRC = 0;
    boolean     bFound = FALSE;
    
#if 0
    memset(pBuff, 0, sizeof(pBuff));
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    printf("[%d] Received Len = %d\n", _task_get_id(), nLen);
    if (nLen != FTE_SH_MV250_FRAME_LENGTH)
    {
        goto error;
    }

    for(nIndex = 0 ; nIndex < 21 ; nIndex++)
    {
        nCRC += pBuff[nIndex];
    }
    
    if ((pBuff[0] != 0x02))// || (nCRC != pBuff[21]))
    {
        printf("Invalid CRC[%02x != %02x]\n", nCRC, pBuff[21]);
        goto error;
    }
#else
    nLen = FTE_UCS_recvLast(pStatus->pUCS, pStaticBuff, sizeof(pStaticBuff));
    if (nLen < FTE_SH_MV250_FRAME_LENGTH)
    {
        goto error;
    }

#if 0
    for(nIndex = 0 ; nIndex < nLen - 23 ; nIndex++)
    {
        if ((pStaticBuff[nIndex] == 0x02) && 
            (pStaticBuff[nIndex + 22] == 0x0d) &&
            (pStaticBuff[nIndex + 23] == 0x0a))
        {
            int_32  i;
            
            for(i = 0 ; i < 21 ; i++)
            {
                nCRC += pStaticBuff[nIndex+i];
            }
            
            if (nCRC == pStaticBuff[nIndex+21])
            {
                memcpy(pBuff, &pStaticBuff[nIndex], FTE_SH_MV250_FRAME_LENGTH);
                bFound = TRUE;
                break;
            }
        }
    }

#else
    for (nIndex = nLen - FTE_SH_MV250_FRAME_LENGTH ; nIndex >= 0 ; nIndex--)
    {
        if ((pStaticBuff[nIndex] == 0x02) && 
            (pStaticBuff[nIndex + 22] == 0x0d) &&
            (pStaticBuff[nIndex + 23] == 0x0a))
        {
            int_32  i;
            
            for(i = 0 ; i < 21 ; i++)
            {
                nCRC += pStaticBuff[nIndex+i];
            }
            
            if (nCRC == pStaticBuff[nIndex+21])
            {
                memcpy(pBuff, &pStaticBuff[nIndex], FTE_SH_MV250_FRAME_LENGTH);
                bFound = TRUE;
                break;
            }
        }    
    }
    
#endif
    if (bFound == FALSE)
    {
        goto error;
    }
#endif
    
    pBuff[7] = '\0';
    nPPM = atoi((char_ptr)&pBuff[3]);
    pBuff[11] = '\0';
    nTemp = atoi((char_ptr)&pBuff[9]) * 10 + atoi((char_ptr)&pBuff[12]);
    if (pBuff[8] == '-')
    {
        nTemp = 0 - nTemp;
    }
    pBuff[16] = '\0';
    nHumi = atoi((char_ptr)&pBuff[14]) * 10 + atoi((char_ptr)&pBuff[17]);

    FTE_VALUE_setPPM(&pStatus->xCommon.pValue[0], nPPM);
    
    if (pBuff[19] == '0')
    {
        FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[1], nTemp * 10);
        FTE_VALUE_setHumidity(&pStatus->xCommon.pValue[2], nHumi * 10);
    }
    
    return  MQX_OK;
    
error:
    FTE_VALUE_setValid(&pStatus->xCommon.pValue[0], FALSE);
    FTE_VALUE_setValid(&pStatus->xCommon.pValue[1], FALSE);
    FTE_VALUE_setValid(&pStatus->xCommon.pValue[2], FALSE);
    
    return  MQX_ERROR;
}
