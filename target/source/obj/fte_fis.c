#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"

FTE_VALUE_TYPE  FTE_FIS_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM
};

typedef struct  
{
    FTE_UINT32     nADC;
    FTE_UINT32     nPPM;
}   FTE_FIS_VALUE_PAIR;


const FTE_GUS_MODEL_INFO    FTE_FIS3061_GUSModelInfo = 
{
    .nModel     = FTE_GUS_MODEL_FIS3061,
    .pName      = "FIS3061",
    .nFieldCount= 1,
    .pValueTypes= FTE_FIS_valueTypes,
    .fAttach   = FTE_FIS_attach,
    .fDetach   = FTE_FIS_detach,
    .fRequest  = FTE_FIS_request,
    .fReceived = FTE_FIS_received
};

const FTE_FIS_VALUE_PAIR    _pFIS3061ValuePairs[] = 
{
    {950, 100}, 
    {931, 200},
    {915, 300},
    {902, 400},
    {893, 500},
    {878, 600},
    {864, 700},
    {847, 800},
    {827, 900},
    {801, 1000},
    {763, 2000},
    {723, 3000},
    {704, 4000},
    {683, 5000},
    {668, 6000},
    {658, 7000},
    {648, 8000},
    {636, 9000},
    {630, 10000}
};

FTE_VALUE_TYPE  pValueTypes[] =
{
    FTE_VALUE_TYPE_PPM
};

FTE_RET   FTE_FIS_attach
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_FIS_STATUS_PTR  pStatus;
    FTE_UCS_PTR         pUCS = NULL;
    FTE_LWGPIO_PTR      pLWGPIO = NULL;
    
    ASSERT(pObj != NULL);

    pUCS = (FTE_UCS_PTR)FTE_UCS_get(((FTE_FIS_CONFIG_PTR)pObj->pConfig)->nUCSID);
    if (pUCS == NULL)
    {
        goto error;
    }
    
    pLWGPIO = (FTE_LWGPIO_PTR)FTE_LWGPIO_get(((FTE_FIS_CONFIG_PTR)pObj->pConfig)->nREQID);
    if (pLWGPIO == NULL)
    {
        goto error;
    }
    
    if (FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }
    
    if (FTE_LWGPIO_attach(pLWGPIO, pObj->pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }
    
    pStatus = (FTE_FIS_STATUS_PTR)FTE_MEM_allocZero(sizeof(FTE_FIS_STATUS));
    if (pStatus == NULL)
    {
        FTE_UCS_detach(pUCS, pObj->pConfig->xCommon.nID);
        goto error;
    }

    pStatus->pUCS = pUCS;
    pStatus->pREQ = pLWGPIO;
    pObj->pAction = NULL;
    pObj->pStatus = (FTE_OBJECT_STATUS_PTR)pStatus;
   
    return  MQX_OK;
    
error:
    if (pUCS != NULL)
    {
        FTE_UCS_detach(pUCS, 0);
    }
    
    if (pLWGPIO != NULL)
    {
        FTE_LWGPIO_detach(pLWGPIO);
    }
    
    return  MQX_ERROR;
    
}

FTE_RET FTE_FIS_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_FIS_STATUS_PTR  pStatus;
    if (pObj == NULL)
    {
        goto error;
    }

    pStatus = (FTE_FIS_STATUS_PTR)pObj->pStatus;
    if (pStatus->pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->pUCS, pObj->pConfig->xCommon.nID);
        pStatus->pUCS = NULL;
    }
    
    if (pStatus->pREQ != NULL)
    {
        FTE_LWGPIO_detach(pStatus->pREQ);
        pStatus->pREQ = NULL;
    }
    
    FTE_MEM_free(pStatus);
    pObj->pAction = NULL;
    pObj->pStatus = NULL;
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

FTE_RET   FTE_FIS_request
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_FIS_STATUS_PTR  pStatus = (FTE_FIS_STATUS_PTR)pObj->pStatus;    
    
    FTE_LWGPIO_setValue(pStatus->pREQ, TRUE);

    return  MQX_OK;
}

FTE_RET   FTE_FIS_received
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_CHAR_PTR    pToken = NULL;
    FTE_UINT32      nCount = 0;  
    FTE_UINT8       pBuff[64];
    FTE_CHAR_PTR    pHead;
    FTE_UINT32      nLen;
    FTE_INT32       nPPM = 0, nTemp = 0, nGAS = 0;
//    FTE_UINT32     nDate = 0, nAlarm = 0, nStat = 0;
    FTE_FIS_STATUS_PTR  pStatus = (FTE_FIS_STATUS_PTR)pObj->pStatus;    
   
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen == 0)
    {
        return  MQX_ERROR;
    }
    
    pHead = (FTE_CHAR_PTR)pBuff;
    
    while((pToken = strtok(pHead, " \t\r\n")) != NULL)
    {
        switch(nCount)
        {
        case    0:  nGAS = atoi(pToken); break;
        case    1:  
            {
                nTemp = atoi(&pToken[1]);
                if (pToken[0] == '-')
                {
                    nTemp = 0 - nTemp;
                }
            }
            break;
//        case    2:  nDate  = atoi(pToken); break;
//        case    3:  nAlarm = atoi(pToken); break;
//        case    4:  nStat  = atoi(pToken); break;
        }
        
        pHead = pToken + strlen(pToken) + 1;
        nCount++;
    }

    if (nCount != 5)
    {    
        return  MQX_ERROR;
    }
    
    for(FTE_INT32 i = 0 ; i < sizeof(_pFIS3061ValuePairs) / sizeof(FTE_FIS_VALUE_PAIR) ; i++)
    {
        if (nGAS < _pFIS3061ValuePairs[i].nADC)
        {
            nPPM = _pFIS3061ValuePairs[i].nPPM;
        }
    }    
    
    FTE_VALUE_setPPM(pStatus->xCommon.pValue, nPPM);
    
    return  MQX_OK;
}
