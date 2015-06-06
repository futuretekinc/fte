#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"

FTE_VALUE_TYPE  FT_COZIR_valueTypes[] =
{
    FTE_VALUE_TYPE_PPM
};

_mqx_uint   FTE_COZIR_request(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    FTE_UCS_clear(pStatus->pUCS);    
    FTE_UCS_send(pStatus->pUCS, "Z\r\n", 3, FALSE);    

    return  MQX_OK;
}

_mqx_uint   FTE_COZIR_received(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    char_ptr    pToken = NULL;
    uint_32     nIndex = 0;
    uint_32     nCount = 0;
    uint_32     nPPM1 = 0;
#if 0
    uint_32     nPPM2;
#endif
    uint_8      pBuff[64];
    char_ptr    pHead;
    uint_32     nLen;
    
    nLen = FTE_UCS_recv(pStatus->pUCS, pBuff, sizeof(pBuff));
    if (nLen == 0)
    {
        return  MQX_ERROR;
    }
    
    pHead = (char_ptr)pBuff;
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
    
    return  MQX_OK;
}
