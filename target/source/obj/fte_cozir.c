#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"

#if FTE_COZIR_AX5000_SUPPORTED

FTE_GUS_CONFIG FTE_COZIR_AX5000_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
        .pName      = "CO2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nModel     = FTE_GUS_MODEL_COZIR_AX5000,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_AX5000_INTERVAL,
};

FTE_VALUE_TYPE  FT_COZIR_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM
};


const FTE_GUS_MODEL_INFO    FTE_COZIR_AX5000_GUSModelInfo =
{
    .nModel         = FTE_GUS_MODEL_COZIR_AX5000,
    .pName          = "COZIR AX5000",
    .nMaxResponseTime=FTE_AX5000_RESPONSE_TIME,
    .nFieldCount    = 1,
    .pValueTypes    = FT_COZIR_valueTypes,
    .f_request      = FTE_COZIR_request,
    .f_received     = FTE_COZIR_received
};

FTE_RET   FTE_COZIR_request
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, "Z\r\n", 3, FALSE);    

    return  FTE_RET_OK;
}

FTE_RET   FTE_COZIR_received
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    FTE_CHAR_PTR    pToken = NULL;
    FTE_UINT32     nIndex = 0;
    FTE_UINT32     nCount = 0;
    FTE_UINT32     nPPM1 = 0;
#if 0
    FTE_UINT32     nPPM2;
#endif
    FTE_UINT8      pBuff[64];
    FTE_CHAR_PTR    pHead;
    FTE_UINT32     nLen;
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen == 0)
    {
        return  FTE_RET_ERROR;
    }
    
    pHead = (FTE_CHAR_PTR)pBuff;
    while(((pToken = strtok(pHead, " \t\r\n")) != NULL) && (nCount < 2))
    {
        switch(nIndex)
        {
        case    0:  
            if (strcmp(pToken, "Z") != 0)
            {
                return  0;
            }
            break;
            
        case    1:  
            {
                nPPM1 = atoi(pToken); 
                nCount++;
            }
            break;
#if 0
        case    2:  
            if (strcmp(pToken, "z") != 0)
            {
                return  0;
            }
            break;
            
        case    3:  
            {
                nPPM2 = atoi(pToken); 
                nCount++;
            }
            break;
#endif
        }
        
        pHead = pToken + strlen(pToken) + 1;
        nIndex++;
    }
    
    FTE_VALUE_setPPM(pStatus->xCommon.pValue, nPPM1);
    
    return  FTE_RET_OK;
}

#endif