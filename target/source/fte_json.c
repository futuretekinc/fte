
#include <stdlib.h>
#include "fte_target.h"
#include "fte_net.h"
#include "fte_config.h"
#include "fte_object.h"
#include "httpsrv.h"
#include "fte_buff.h"
#include "fte_cgi.h"
#include "fte_json.h"

#define FTE_JSON_alloc(x)   FTE_MEM_allocZero(x)
#define FTE_JSON_free(x)    FTE_MEM_free(x)

FTE_RET FTE_JSON_init
(
    FTE_JSON_PTR    pJSON
)
{
    return  FTE_RET_OK;    
}

FTE_RET FTE_JSON_final
(
    FTE_JSON_PTR    pJSON
)
{
    return  FTE_RET_OK;    
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createString
(
    FTE_CHAR_PTR    pString
)
{
    FTE_JSON_STRING_PTR pItem = NULL;
    
    pItem = (FTE_JSON_STRING_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_STRING));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_STRING;
    pItem->pValue = FTE_JSON_alloc(strlen(pString) + 1);
    if (pItem->pValue == NULL)
    {
        FTE_JSON_free(pItem);
        return  NULL;
    }
    
    strcpy(pItem->pValue, pString);
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createNumber
(
    long    nValue
)
{
    FTE_JSON_NUMBER_PTR pItem = NULL;
    
    pItem = (FTE_JSON_NUMBER_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_NUMBER));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_NUMBER;
    pItem->nValue = nValue;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createHex
(
    long    nValue
)
{
    FTE_JSON_NUMBER_PTR pItem = NULL;
    
    pItem = (FTE_JSON_NUMBER_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_HEX));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_HEX;
    pItem->nValue = nValue;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createFloat
(
    long    nValue
)
{
    FTE_JSON_FLOAT_PTR pItem = NULL;
    
    pItem = (FTE_JSON_FLOAT_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_FLOAT));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_FLOAT;
    pItem->nValue = nValue;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createValue
(
    FTE_VALUE_PTR   pValue
)
{
    switch(pValue->xType)
    {
    case    FTE_VALUE_TYPE_BOOL:
    case    FTE_VALUE_TYPE_DIO:
    case    FTE_VALUE_TYPE_LED:
        {
            if (pValue->xData.bValue != FALSE)
            {
                return  FTE_JSON_VALUE_createNumber(1);
            }
            else
            {
                return  FTE_JSON_VALUE_createNumber(0);
            }
        }
        
    case    FTE_VALUE_TYPE_ULONG:
    case    FTE_VALUE_TYPE_PPM:
        {
            return  FTE_JSON_VALUE_createNumber(pValue->xData.ulValue);
        }
        
    case    FTE_VALUE_TYPE_HEX32:
        {
            return  FTE_JSON_VALUE_createHex(pValue->xData.ulValue);
        }
        
    case    FTE_VALUE_TYPE_TEMPERATURE:
    case    FTE_VALUE_TYPE_HUMIDITY:
    case    FTE_VALUE_TYPE_VOLTAGE:
    case    FTE_VALUE_TYPE_CURRENT:
    case    FTE_VALUE_TYPE_PRESSURE:
    case    FTE_VALUE_TYPE_PWR_KWH:
    case   FTE_VALUE_TYPE_PWR_KW:
        {
            return  FTE_JSON_VALUE_createFloat(pValue->xData.ulValue);
        }
    }
    
    return  NULL;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createPair
(
    FTE_CHAR_PTR    pString, 
    FTE_JSON_VALUE_PTR pValue
)
{
    FTE_JSON_PAIR_PTR pItem = NULL;
    
    pItem = (FTE_JSON_PAIR_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_PAIR));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_PAIR;
    pItem->pString = FTE_JSON_alloc(strlen(pString) + 1);
    if (pItem->pString== NULL)
    {
        FTE_JSON_free(pItem);
        return  NULL;
    }
    strcpy(pItem->pString, pString);
    pItem->pValue = pValue;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}


FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createObject(int nMaxCount)
{
    FTE_JSON_OBJECT_PTR pItem = NULL;
    
    pItem = (FTE_JSON_OBJECT_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_OBJECT) + sizeof(FTE_JSON_PAIR_PTR)*nMaxCount);
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_OBJECT;
    pItem->nMaxCount = nMaxCount;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createArray
(
    int     nMaxCount
)
{
    FTE_JSON_ARRAY_PTR pItem = NULL;
    
    pItem = (FTE_JSON_ARRAY_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_ARRAY) + sizeof(FTE_JSON_VALUE_PTR)*nMaxCount);
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_ARRAY;
    pItem->nMaxCount = nMaxCount;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}


FTE_RET FTE_JSON_OBJECT_setPair
(
    FTE_JSON_OBJECT_PTR pObject, 
    FTE_CHAR_PTR        pString, 
    FTE_JSON_VALUE_PTR  pValue
)
{
    ASSERT((pObject != NULL) && (pValue != NULL));
    
    if (pObject->nCount < pObject->nMaxCount)
    {
        int i;
        
        for(i = 0; i < pObject->nMaxCount ; i++)
        {
            if (pObject->pPairs[i] == 0)
            {
                FTE_JSON_PAIR_PTR   pPair = (FTE_JSON_PAIR_PTR)FTE_JSON_VALUE_createPair(pString, pValue);
                if (pPair == NULL)
                {
                    return  FTE_RET_ERROR;
                }
                
                pObject->pPairs[i] = pPair;
                pObject->nCount++;

                return  FTE_RET_OK;
            }
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_JSON_OBJECT_addStringPair
(
    FTE_JSON_OBJECT_PTR     pObject, 
    FTE_CHAR_PTR            pName, 
    FTE_CHAR_PTR            pString
)
{
    ASSERT((pObject != NULL) && (pName!= NULL) && (pString != NULL));
    
    if (pObject->nCount < pObject->nMaxCount)
    {
        FTE_JSON_VALUE_PTR  pValue;
        int i;
        
        pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createString(pString);
        if (pValue == NULL)
        {
            return  FTE_RET_ERROR;
        }
        
        for(i = 0; i < pObject->nMaxCount ; i++)
        {
            if (pObject->pPairs[i] == 0)
            {
                FTE_JSON_PAIR_PTR   pPair = (FTE_JSON_PAIR_PTR)FTE_JSON_VALUE_createPair(pName, pValue);
                if (pPair == NULL)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    return  FTE_RET_ERROR;
                }
                
                pObject->pPairs[i] = pPair;
                pObject->nCount++;

                return  FTE_RET_OK;
            }
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_JSON_OBJECT_addNumberPair
(
    FTE_JSON_OBJECT_PTR     pObject, 
    FTE_CHAR_PTR            pName, 
    FTE_INT32               nValue
)
{
    ASSERT((pObject != NULL) && (pName!= NULL));
    
    if (pObject->nCount < pObject->nMaxCount)
    {
        FTE_JSON_VALUE_PTR  pValue;
        int i;
        
        pValue = (FTE_JSON_VALUE_PTR)FTE_JSON_VALUE_createNumber(nValue);
        if (pValue == NULL)
        {
            return  FTE_RET_ERROR;
        }
        
        for(i = 0; i < pObject->nMaxCount ; i++)
        {
            if (pObject->pPairs[i] == 0)
            {
                FTE_JSON_PAIR_PTR   pPair = (FTE_JSON_PAIR_PTR)FTE_JSON_VALUE_createPair(pName, pValue);
                if (pPair == NULL)
                {
                    FTE_JSON_VALUE_destroy(pValue);
                    return  FTE_RET_ERROR;
                }
                
                pObject->pPairs[i] = pPair;
                pObject->nCount++;

                return  FTE_RET_OK;
            }
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_JSON_ARRAY_setElement
(
    FTE_JSON_ARRAY_PTR  pArray, 
    FTE_JSON_VALUE_PTR  pElement
)
{
    ASSERT((pArray != NULL) && (pElement != NULL));
    
    if (pArray->nCount < pArray->nMaxCount)
    {
        int i;
        
        for(i = 0; i < pArray->nMaxCount ; i++)
        {
            if (pArray->pElements[i] == 0)
            {
                pArray->pElements[i] = pElement;
                pArray->nCount++;
        
                return  FTE_RET_OK;
            }
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createTrue(void)
{
    FTE_JSON_VALUE_PTR pItem = NULL;
    
    pItem = (FTE_JSON_VALUE_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_VALUE));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_TRUE;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createFalse(void)
{
    FTE_JSON_VALUE_PTR pItem = NULL;
    
    pItem = (FTE_JSON_VALUE_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_VALUE));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_FALSE;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createNull(void)
{
    FTE_JSON_VALUE_PTR pItem = NULL;
    
    pItem = (FTE_JSON_VALUE_PTR)FTE_JSON_alloc(sizeof(FTE_JSON_VALUE));
    if (pItem == NULL)
    {
        return  NULL;
    }
    
    pItem->xType = FTE_JSON_TYPE_NULL;
    
    return  (FTE_JSON_VALUE_PTR)pItem;
}

FTE_RET FTE_JSON_VALUE_destroy
(
    FTE_JSON_VALUE_PTR  pValue
)
{
    ASSERT(pValue != NULL);
    
    switch(pValue->xType)
    {
    case    FTE_JSON_TYPE_STRING:
        {           
            if (((FTE_JSON_STRING_PTR)pValue)->pValue != NULL)
            {
                FTE_JSON_free(((FTE_JSON_STRING_PTR)pValue)->pValue);
                ((FTE_JSON_STRING_PTR)pValue)->pValue = NULL;
            }
        }
        break;
        
    case    FTE_JSON_TYPE_PAIR:   
        {
            if (((FTE_JSON_PAIR_PTR)pValue)->pString != NULL)
            {
                FTE_JSON_free(((FTE_JSON_PAIR_PTR)pValue)->pString);
                ((FTE_JSON_PAIR_PTR)pValue)->pString = NULL;
            }
            
            if (((FTE_JSON_PAIR_PTR)pValue)->pValue != NULL)
            {
                FTE_JSON_VALUE_destroy(((FTE_JSON_PAIR_PTR)pValue)->pValue);
                ((FTE_JSON_PAIR_PTR)pValue)->pValue = NULL;
            }
        }
        break;

    case    FTE_JSON_TYPE_OBJECT:
        {
            int i;
            
            for(i = 0 ; i < ((FTE_JSON_OBJECT_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTE_JSON_OBJECT_PTR)pValue)->pPairs[i] != NULL)
                {
                    FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)((FTE_JSON_OBJECT_PTR)pValue)->pPairs[i]);
                    ((FTE_JSON_OBJECT_PTR)pValue)->pPairs[i] = NULL;
                }
            }
            ((FTE_JSON_OBJECT_PTR)pValue)->nCount = 0;
        }
        break;
        
    case    FTE_JSON_TYPE_ARRAY:
        {
            int i;
            
            for(i = 0 ; i < ((FTE_JSON_ARRAY_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTE_JSON_ARRAY_PTR)pValue)->pElements[i] != NULL)
                {
                    FTE_JSON_VALUE_destroy(((FTE_JSON_ARRAY_PTR)pValue)->pElements[i]);
                    ((FTE_JSON_ARRAY_PTR)pValue)->pElements[i] = NULL;
                }
            }
            ((FTE_JSON_ARRAY_PTR)pValue)->nCount = 0;
        }
        break;
    case    FTE_JSON_TYPE_NUMBER:
    case    FTE_JSON_TYPE_HEX:
    case    FTE_JSON_TYPE_FLOAT:
    case    FTE_JSON_TYPE_TRUE:
    case    FTE_JSON_TYPE_FALSE:
    case    FTE_JSON_TYPE_NULL:
        break;
        
    default:
        return  FTE_RET_ERROR;
    }
    
    FTE_JSON_free(pValue);
    
    return  FTE_RET_OK;
}

FTE_INT32   FTE_JSON_VALUE_snprint
(
    FTE_CHAR_PTR    pBuff, 
    FTE_INT32       nBuffLen, 
    FTE_JSON_VALUE_PTR pValue
)
{
    int nLen = 0;
    
    ASSERT((pBuff != NULL) && (pValue != NULL));
    
    switch(pValue->xType)
    {
    case    FTE_JSON_TYPE_STRING:
        {
            nLen = snprintf(pBuff, nBuffLen, "\"%s\"", ((FTE_JSON_STRING_PTR)pValue)->pValue);
        }
        break;
        
    case    FTE_JSON_TYPE_PAIR:   
        {
            nLen = snprintf(pBuff, nBuffLen, "\"%s\":", ((FTE_JSON_PAIR_PTR)pValue)->pString);
            nLen += FTE_JSON_VALUE_snprint(&pBuff[nLen], nBuffLen - nLen, ((FTE_JSON_PAIR_PTR)pValue)->pValue);
        }
        break;
        
    case    FTE_JSON_TYPE_OBJECT:
        {            
            int i;
            int bFirst = TRUE;
            
            nLen = snprintf(pBuff, nBuffLen, "{");
            for(i = 0 ; i < ((FTE_JSON_OBJECT_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTE_JSON_OBJECT_PTR)pValue)->pPairs[i] != NULL)
                {
                    if (!bFirst)
                    {
                        nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, ",");
                    }
                    nLen += FTE_JSON_VALUE_snprint(&pBuff[nLen], nBuffLen - nLen, (FTE_JSON_VALUE_PTR)((FTE_JSON_OBJECT_PTR)pValue)->pPairs[i]);
                    bFirst = FALSE;
                }
            }
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
        }
        break;

    case    FTE_JSON_TYPE_ARRAY:
        {            
            int i;
            int bFirst = TRUE;
             
            nLen = snprintf(pBuff, nBuffLen, "[");
            for(i = 0 ; i < ((FTE_JSON_ARRAY_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTE_JSON_ARRAY_PTR)pValue)->pElements[i] != NULL)
                {
                    if (!bFirst)
                    {
                        nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, ",");
                    }
                    nLen += FTE_JSON_VALUE_snprint(&pBuff[nLen], nBuffLen - nLen, ((FTE_JSON_ARRAY_PTR)pValue)->pElements[i]);
                    bFirst = FALSE;
                }
            }
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");
        }
        break;

    case    FTE_JSON_TYPE_NUMBER:
        {
            nLen = snprintf(pBuff, nBuffLen, "%d", ((FTE_JSON_NUMBER_PTR)pValue)->nValue);
       }
        break;
        
    case    FTE_JSON_TYPE_HEX:
        {
            nLen = snprintf(pBuff, nBuffLen, "0x%08x", ((FTE_JSON_HEX_PTR)pValue)->nValue);
       }
        break;
        
    case    FTE_JSON_TYPE_FLOAT:
        {
            if ((((FTE_JSON_FLOAT_PTR)pValue)->nValue % 100) == 0)
            {
                nLen = snprintf(pBuff, nBuffLen, "%d", ((FTE_JSON_FLOAT_PTR)pValue)->nValue / 100);
            }
            else
            {
                nLen = snprintf(pBuff, nBuffLen, "%d.%02d", ((FTE_JSON_FLOAT_PTR)pValue)->nValue / 100, ((FTE_JSON_FLOAT_PTR)pValue)->nValue % 100);
            }
       }
        break;
    case    FTE_JSON_TYPE_TRUE:
        {
             nLen = snprintf(pBuff, nBuffLen, "true");
        }
        break;
    case    FTE_JSON_TYPE_FALSE:
        {
             nLen = snprintf(pBuff, nBuffLen, "false");
        }
        break;
    case    FTE_JSON_TYPE_NULL:
        {
             nLen = snprintf(pBuff, nBuffLen, "null");
        }
        break;
        
    default:
        return  0;
    }
    
    return  nLen;
}

FTE_INT32   FTE_JSON_VALUE_buffSize
(
    FTE_JSON_VALUE_PTR  pValue
)
{
    FTE_INT32   nLen = 0;
    static FTE_CHAR pBuff[128];    
    
    ASSERT((pBuff != NULL) && (pValue != NULL));
    
    switch(pValue->xType)
    {
    case    FTE_JSON_TYPE_STRING:
        {
            nLen = snprintf(pBuff, sizeof(pBuff), "\"%s\"", ((FTE_JSON_STRING_PTR)pValue)->pValue);
        }
        break;
        
    case    FTE_JSON_TYPE_PAIR:   
        {
            nLen = snprintf(pBuff, sizeof(pBuff), "\"%s\":", ((FTE_JSON_PAIR_PTR)pValue)->pString);
            nLen += FTE_JSON_VALUE_buffSize(((FTE_JSON_PAIR_PTR)pValue)->pValue);
        }
        break;
        
    case    FTE_JSON_TYPE_OBJECT:
        {            
            int i;
            int bFirst = TRUE;
            
            nLen = snprintf(pBuff, sizeof(pBuff), "{");
            for(i = 0 ; i < ((FTE_JSON_OBJECT_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTE_JSON_OBJECT_PTR)pValue)->pPairs[i] != NULL)
                {
                    if (!bFirst)
                    {
                        nLen += snprintf(pBuff, sizeof(pBuff), ",");
                    }
                    nLen += FTE_JSON_VALUE_buffSize((FTE_JSON_VALUE_PTR)((FTE_JSON_OBJECT_PTR)pValue)->pPairs[i]);
                    bFirst = FALSE;
                }
            }
            nLen += snprintf(pBuff, sizeof(pBuff), "}");
        }
        break;

    case    FTE_JSON_TYPE_ARRAY:
        {            
            int i;
            int bFirst = TRUE;
            
            nLen = snprintf(pBuff, sizeof(pBuff), "[");
            for(i = 0 ; i < ((FTE_JSON_ARRAY_PTR)pValue)->nMaxCount ; i++)
            {
                if (((FTE_JSON_ARRAY_PTR)pValue)->pElements[i] != NULL)
                {
                    if (!bFirst)
                    {
                        nLen += snprintf(pBuff, sizeof(pBuff), ",");
                    }
                    nLen += FTE_JSON_VALUE_buffSize(((FTE_JSON_ARRAY_PTR)pValue)->pElements[i]);
                    bFirst = FALSE;
                }
            }
            nLen += snprintf(pBuff, sizeof(pBuff), "]");
        }
        break;

    case    FTE_JSON_TYPE_NUMBER:
        {
            nLen = snprintf(pBuff, sizeof(pBuff), "%d", ((FTE_JSON_NUMBER_PTR)pValue)->nValue);
       }
        break;
        
    case    FTE_JSON_TYPE_HEX:
        {
            nLen = snprintf(pBuff, sizeof(pBuff), "0x%08x", ((FTE_JSON_HEX_PTR)pValue)->nValue);
       }
        break;
        
    case    FTE_JSON_TYPE_FLOAT:
        {
            if ((((FTE_JSON_FLOAT_PTR)pValue)->nValue % 100) == 0)
            {
                nLen = snprintf(pBuff, sizeof(pBuff), "%d", ((FTE_JSON_FLOAT_PTR)pValue)->nValue / 100);
            }
            else
            {
                nLen = snprintf(pBuff, sizeof(pBuff), "%d.%02d", ((FTE_JSON_FLOAT_PTR)pValue)->nValue / 100, ((FTE_JSON_FLOAT_PTR)pValue)->nValue % 100);
            }

       }
        break;
    case    FTE_JSON_TYPE_TRUE:
        {
             nLen = snprintf(pBuff, sizeof(pBuff), "true");
        }
        break;
    case    FTE_JSON_TYPE_FALSE:
        {
             nLen = snprintf(pBuff, sizeof(pBuff), "false");
        }
        break;
    case    FTE_JSON_TYPE_NULL:
        {
             nLen = snprintf(pBuff, sizeof(pBuff), "null");
        }
        break;
        
    default:
        return  0;
    }
    
    return  nLen + 1;
}
