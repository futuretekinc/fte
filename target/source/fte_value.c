#include "fte_target.h"
#include "fte_value.h"

typedef struct
{
    FTE_VALUE_TYPE  xType;
}   FTE_VALUE_TYPE_DESCRIPT, _PTR_ FTE_VALUE_TYPE_DESCRIPT_PTR;

FTE_VALUE_PTR   FTE_VALUE_create
(
    FTE_VALUE_TYPE  xType
)
{
    switch(xType)
    {
    case    FTE_VALUE_TYPE_ULONG:       return  FTE_VALUE_createULONG();
    case    FTE_VALUE_TYPE_TEMPERATURE: return  FTE_VALUE_createTemperature();
    case    FTE_VALUE_TYPE_HUMIDITY:    return  FTE_VALUE_createHumidity();
    case    FTE_VALUE_TYPE_PPM:         return  FTE_VALUE_createPPM();
    case    FTE_VALUE_TYPE_PH:         return  FTE_VALUE_createPH();
    case    FTE_VALUE_TYPE_MM:         return  FTE_VALUE_createMM();
    case    FTE_VALUE_TYPE_BOOL: 
    case    FTE_VALUE_TYPE_DIO:         return  FTE_VALUE_createDIO();
    case    FTE_VALUE_TYPE_VOLTAGE:     return  FTE_VALUE_createVoltage();
    case    FTE_VALUE_TYPE_CURRENT:     return  FTE_VALUE_createCurrent();
    case    FTE_VALUE_TYPE_PRESSURE:    return  FTE_VALUE_createPressure();
    case    FTE_VALUE_TYPE_PWR_KW:      return  FTE_VALUE_createPowerKW();
    case    FTE_VALUE_TYPE_PWR_KWH:     return  FTE_VALUE_createPowerKWH();
    case    FTE_VALUE_TYPE_LED:         return  FTE_VALUE_createLED();
    case    FTE_VALUE_TYPE_HEX32:       return  FTE_VALUE_createHex32();
    }
    
    return  NULL;
}

FTE_VALUE_PTR   FTE_VALUE_createTemperature(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_TEMPERATURE;
        pValue->bValid= FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createHumidity(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_HUMIDITY;
        pValue->bValid= FALSE;
        pValue->xData.nValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createDIO(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_DIO;
        pValue->bValid= FALSE;
        pValue->xData.bValue = FALSE;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createPPM(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_PPM;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createPH(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_PH;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createMM(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_MM;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createULONG(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_ULONG;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createHex32(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_HEX32;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createVoltage(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_VOLTAGE;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createCurrent(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_CURRENT;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createPressure(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_PRESSURE;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createPowerKW(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_PWR_KW;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createPowerKWH(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_PWR_KWH;
        pValue->bValid = FALSE;
        pValue->xData.ulValue = 0;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createLED(void)
{
    FTE_VALUE_PTR   pValue;
    
    pValue = FTE_MEM_allocZero(sizeof(FTE_VALUE));
    if (pValue != NULL)
    {
        pValue->xType = FTE_VALUE_TYPE_LED;
        pValue->xData.ulValue = 0;
        pValue->xData.bValue = FALSE;
    }
    
    return  pValue;
}

FTE_VALUE_PTR   FTE_VALUE_createArray(FTE_VALUE_TYPE_PTR pTypes, FTE_UINT32 ulCount)
{
    FTE_VALUE_PTR   pValues;
    
    pValues = FTE_MEM_allocZero(sizeof(FTE_VALUE) * ulCount);
    if (pValues != NULL)
    {
        for(FTE_INT32 i = 0 ; i < ulCount ; i++)
        {
            pValues[i].xType        = pTypes[i];
            pValues[i].bValid       = FALSE;
        }
    }
    
    return  pValues;

}

FTE_RET FTE_VALUE_destroy
(
    FTE_VALUE_PTR   pValue
)
{
    ASSERT(pValue != NULL);
    
    FTE_MEM_free(pValue);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_init
(
    FTE_VALUE_PTR   pValue, 
    FTE_VALUE_TYPE  xType
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = xType;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initTemperature
(
    FTE_VALUE_PTR   pValue, 
    FTE_INT32       nValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_TEMPERATURE;
    pValue->xData.nValue = nValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initHumidity
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_HUMIDITY;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initDIO
(
    FTE_VALUE_PTR   pValue, 
    FTE_BOOL        bValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_DIO;
    pValue->xData.bValue = bValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initPPM
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_PPM;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}
FTE_RET FTE_VALUE_initPH
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_PH;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initULONG
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_ULONG;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initHex32
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_HEX32;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initVoltage
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_VOLTAGE;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initCurrent
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_CURRENT;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initPressure
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_PRESSURE;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initPowerKW
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_PWR_KW;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_initPowerKWH
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_PWR_KWH;
    pValue->xData.ulValue = ulValue;
    
    return  FTE_RET_OK;
}


FTE_RET FTE_VALUE_initLED
(
    FTE_VALUE_PTR   pValue, 
    FTE_UINT32      ulState
)
{
    ASSERT(pValue != NULL);
    
    memset(pValue, 0, sizeof(FTE_VALUE));
    pValue->xType = FTE_VALUE_TYPE_LED;
    pValue->xData.xState = ulState;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setValid
(
    FTE_VALUE_PTR   pValue, 
    FTE_BOOL        bValid
)
{
    if (bValid)
    {
        pValue->ulFailCount = 0;
        pValue->bValid = TRUE;
    }
    else
    {
        if (++pValue->ulFailCount >= 3)
        {
            pValue->bValid = FALSE;
        }
    }
    
    return  FTE_RET_OK;
}

FTE_BOOL    FTE_VALUE_isValid
(
    FTE_VALUE_PTR   pValue
)
{
    ASSERT(pValue != NULL);
 
    return  pValue->bValid;
}

FTE_RET FTE_VALUE_set
(
    FTE_VALUE_PTR   pObj, 
    FTE_CHAR_PTR    pString
)
{
    ASSERT((pObj != NULL) && (pString != NULL));
    
    switch(pObj->xType)
    {
    case    FTE_VALUE_TYPE_ULONG:
        {
            pObj->xData.ulValue = strtoul(pString, NULL, 10);
        }
        break;

    case    FTE_VALUE_TYPE_HEX32:
        {
            pObj->xData.ulValue = strtoul(pString, NULL, 16);
        }
        break;

    case    FTE_VALUE_TYPE_VOLTAGE:
    case    FTE_VALUE_TYPE_CURRENT:
    case    FTE_VALUE_TYPE_PWR_KW:
    case    FTE_VALUE_TYPE_PWR_KWH:
        {
            pObj->xData.ulValue = (FTE_UINT32)(strtod(pString, NULL)*1000);
        }
        break;

    case    FTE_VALUE_TYPE_PRESSURE:
        {
            pObj->xData.ulValue = (FTE_UINT32)(strtod(pString, NULL)*10);
        }
        break;
        
    case    FTE_VALUE_TYPE_BOOL:
       {
            if ((strcasecmp(pString, "true") == 0) || (strcasecmp(pString, "1") == 0))
            {
                pObj->xData.bValue = TRUE;
            }
            else if ((strcasecmp(pString, "false") == 0) || (strcasecmp(pString, "0") == 0))
            {
                pObj->xData.bValue = FALSE;
            }
            else
            {
                goto error;
            }
        }
        break;

    case    FTE_VALUE_TYPE_TEMPERATURE:
       {
            FTE_FLOAT   fValue = strtod(pString, NULL);
           
            pObj->xData.ulValue = (FTE_UINT32)(fValue * 100) ;
        }
        break;

    case    FTE_VALUE_TYPE_HUMIDITY:
       {
            FTE_FLOAT   fValue = strtod(pString, NULL);
           
            if (fValue < 0 || fValue > 100)
            {
                goto error; 
            }
            
            pObj->xData.ulValue = (FTE_UINT32)(fValue * 100) ;
        }
        break;

    case    FTE_VALUE_TYPE_PPM:
        {
            FTE_INT32  nValue = strtol(pString, NULL, 10);
            
            if (nValue < 0)
            {
                goto error;                
            }
            
            pObj->xData.ulValue = (FTE_UINT32)nValue;
        }
        break;

    case    FTE_VALUE_TYPE_PH:
    case    FTE_VALUE_TYPE_MM:
        {
            FTE_INT32  nValue = strtol(pString, NULL, 10);
            
            if (nValue < 0)
            {
                goto error;                
            }
            
            pObj->xData.ulValue = (FTE_UINT32)nValue;
        }
        break;

    case    FTE_VALUE_TYPE_DIO:
        {
            if ((strcasecmp(pString, "on") == 0) || (strcasecmp(pString, "1") == 0))
            {
                pObj->xData.bValue = TRUE;
            }
            else if ((strcasecmp(pString, "off") == 0) || (strcasecmp(pString, "0") == 0))
            {
                pObj->xData.bValue = FALSE;
            }
            else
            {
                goto error;
            }
        }
        break;
        
    case    FTE_VALUE_TYPE_LED:
        {
            if ((strcasecmp(pString, "on") == 0) || (strcasecmp(pString, "1") == 0))
            {
                pObj->xData.xState = FTE_LED_STATE_ON;
            }
            else if ((strcasecmp(pString, "off") == 0) || (strcasecmp(pString, "0") == 0))
            {
                pObj->xData.xState = FTE_LED_STATE_OFF;
            }
            else if ((strcasecmp(pString, "blink") == 0) || (strcasecmp(pString, "2") == 0))
            {
                pObj->xData.xState = FTE_LED_STATE_BLINK;
            }
            else
            {
                goto error;
            }
        }
        break;
        
    default:
        goto error;
    }
    
    FTE_VALUE_setValid(pObj, TRUE);
    
    return  FTE_RET_OK;
    
error:
    FTE_VALUE_setValid(pObj, FALSE);

    return  FTE_RET_ERROR;
}


FTE_RET FTE_VALUE_setTemperature
(
    FTE_VALUE_PTR   pObj, 
    FTE_INT32       nValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.nValue != nValue);
    pObj->xData.nValue  = nValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);

    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getTemperature
(
    FTE_VALUE_PTR   pObj, 
    FTE_INT32_PTR   pnValue
)
{
    ASSERT(pObj != NULL);
    
    *pnValue = pObj->xData.nValue;

    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setHumidity
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getHumidity
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setDIO
(
    FTE_VALUE_PTR   pObj, 
    FTE_BOOL        bValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.bValue != bValue);
    pObj->xData.bValue  = bValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getDIO
(
    FTE_VALUE_PTR   pObj, 
    FTE_BOOL_PTR    pbValue
)
{
    ASSERT(pObj != NULL);
    
    *pbValue = pObj->xData.bValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setPPM
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getPPM
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setPH
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getPH
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setMM
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getMM
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setULONG
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getULONG
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setVoltage
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getVoltage
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setCurrent
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getCurrent
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setPressure
(
    FTE_VALUE_PTR   pObj, 
    FTE_INT32       nValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.nValue != nValue);
    pObj->xData.nValue  = nValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getPowerKW
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setPowerKW
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getPowerKWH
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pulValue
)
{
    ASSERT(pObj != NULL);
    
    *pulValue = pObj->xData.ulValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setPowerKWH
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      ulValue
)
{
    ASSERT(pObj != NULL);
    
    pObj->bChanged = (pObj->xData.ulValue != ulValue);
    pObj->xData.ulValue  = ulValue;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}
FTE_RET FTE_VALUE_getPressure
(
    FTE_VALUE_PTR   pObj, 
    FTE_INT32_PTR   pnValue
)
{
    ASSERT(pObj != NULL);
    
    *pnValue = pObj->xData.nValue;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_setLED
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32      xState
)
{
    ASSERT(pObj != NULL);

    pObj->bChanged = (pObj->xData.xState != xState);
    pObj->xData.xState  = xState;
    FTE_VALUE_setValid(pObj, TRUE);
    _time_get (&pObj->xTimeStamp);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getLED
(
    FTE_VALUE_PTR   pObj, 
    FTE_UINT32_PTR  pxState
)
{
    ASSERT(pObj != NULL);
    
    *pxState = pObj->xData.xState;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_getTimeStamp
(
    FTE_VALUE_PTR   pObj, 
    TIME_STRUCT_PTR pTimeStamp
)
{
    ASSERT(pObj != NULL);
    
    if (pTimeStamp != NULL)
    {
        *pTimeStamp = pObj->xTimeStamp;
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_VALUE_copy
(
    FTE_VALUE_PTR   pDest, 
    FTE_VALUE_PTR   pSrc
)
{
    ASSERT((pDest != NULL) && (pSrc != NULL));
    
    memcpy(pDest, pSrc, sizeof(FTE_VALUE));
    
    return  FTE_RET_OK;
}

FTE_CHAR_PTR    FTE_VALUE_toString
(
    FTE_VALUE_PTR   pObj, 
    FTE_CHAR_PTR    pValueString, 
    FTE_UINT32      ulLen
)
{
    if (pObj->bValid == FALSE)
    {
        snprintf(pValueString, ulLen, "N/A");
    }
    else
    {
        switch(pObj->xType)
        {
        case    FTE_VALUE_TYPE_TEMPERATURE:
            {
                snprintf(pValueString, ulLen, "%d.%d", pObj->xData.nValue / 100, abs(pObj->xData.nValue) % 100 / 10);
            }
            break;
            
        case    FTE_VALUE_TYPE_HUMIDITY:
            {
                snprintf(pValueString, ulLen, "%d.%d", pObj->xData.ulValue / 100, pObj->xData.ulValue % 100 / 10);
            }
            break;
            
        case    FTE_VALUE_TYPE_DIO:
            {
                if (pObj->xData.bValue)
                {
                    snprintf(pValueString, ulLen, "1");
                }
                else
                {
                    snprintf(pValueString, ulLen, "0");
                }
            }
            break;
            
        case    FTE_VALUE_TYPE_LED:
            {
                switch(pObj->xData.xState)
                {
                case    0: snprintf(pValueString, ulLen, "off"); break;
                case    1: snprintf(pValueString, ulLen, "on"); break;
                case    2: snprintf(pValueString, ulLen, "blink"); break;
                default: snprintf(pValueString, ulLen, "%d", pObj->xData.xState); break;
                }
            }
            break;
            
         case    FTE_VALUE_TYPE_VOLTAGE:
         case    FTE_VALUE_TYPE_CURRENT:
            {
                if (pObj->xData.ulValue < 1000)
                {
                    snprintf(pValueString, ulLen, "%d.%02d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000 / 10);
                }
#if 0
                else if (pObj->xData.ulValue < 10000)
                {
                    snprintf(pValueString, ulLen, "%d.%d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000 / 100);
                }
                else 
                {
                    snprintf(pValueString, ulLen, "%d", pObj->xData.ulValue / 1000);
                }
#else
                else 
                {
                    snprintf(pValueString, ulLen, "%d.%d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000 / 100);
                }
#endif
            }
            break;
            
         case    FTE_VALUE_TYPE_PWR_KW:
         case    FTE_VALUE_TYPE_PWR_KWH:
            {
                if (pObj->xData.ulValue < 1000)
                {
                    snprintf(pValueString, ulLen, "%d.%03d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000);
                }
#if 0
                else if (pObj->xData.ulValue < 100000)
                {
                    snprintf(pValueString, ulLen, "%d.%02d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000 / 10);
                }
                else if (pObj->xData.ulValue < 1000000)
                {
                    snprintf(pValueString, ulLen, "%d.%01d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000 / 100);
                }
                else 
                {
                    snprintf(pValueString, ulLen, "%d", pObj->xData.ulValue / 1000);
                }
#else
                else 
                {
                    snprintf(pValueString, ulLen, "%d.%02d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000 / 10);
                }
#endif
            }
            break;
            
         case    FTE_VALUE_TYPE_PPM:
         case    FTE_VALUE_TYPE_ULONG:
            {
                snprintf(pValueString, ulLen, "%d", pObj->xData.ulValue);
            }
            break;
            
         case    FTE_VALUE_TYPE_PH:
            {
                snprintf(pValueString, ulLen, "%d.%02d", pObj->xData.ulValue / 100, pObj->xData.ulValue % 100);
            }
            break;
            
         case    FTE_VALUE_TYPE_MM:
            {
                snprintf(pValueString, ulLen, "%d.%02d", pObj->xData.ulValue / 100, pObj->xData.ulValue % 100);
            }
            break;
            
         case    FTE_VALUE_TYPE_HEX32:
            {
                snprintf(pValueString, ulLen, "0x%08x", pObj->xData.ulValue);
            }
            break;
            
          case    FTE_VALUE_TYPE_PRESSURE:
            {
                if (pObj->xData.ulValue < 1000)
                {
                    snprintf(pValueString, ulLen, "%d.%03d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000);
                }
                else 
                {
                    snprintf(pValueString, ulLen, "%d.%02d", pObj->xData.ulValue / 1000, pObj->xData.ulValue % 1000 / 10);
                }
            }
            break;

        }
    }
                     
    return  pValueString;
}

FTE_CHAR_PTR    FTE_VALUE_unit
(
    FTE_VALUE_PTR   pObj, 
    FTE_CHAR_PTR    pUnitString, 
    FTE_UINT32      ulLen
)
{
    switch(pObj->xType)
    {
    case    FTE_VALUE_TYPE_TEMPERATURE:
        {
            snprintf(pUnitString, ulLen, "C");
        }
        break;
        
    case    FTE_VALUE_TYPE_HUMIDITY:
        {
            snprintf(pUnitString, ulLen, "%% RH");
        }
        break;
        
     case    FTE_VALUE_TYPE_VOLTAGE:
        {
            snprintf(pUnitString, ulLen, "V");
        }
        break;
        
     case    FTE_VALUE_TYPE_CURRENT:
        {
            snprintf(pUnitString, ulLen, "A");
        }
        break;
        
     case    FTE_VALUE_TYPE_PWR_KWH:
        {
            snprintf(pUnitString, ulLen, "kWh");
        }
        break;
        
     case    FTE_VALUE_TYPE_PWR_KW:
        {
            snprintf(pUnitString, ulLen, "kW");
        }
        break;
        
     case    FTE_VALUE_TYPE_PPM:
        {
            snprintf(pUnitString, ulLen, "PPM");
        }
        break;
        
     case    FTE_VALUE_TYPE_PH:
        {
            snprintf(pUnitString, ulLen, "PH");
        }
        break;
        
     case    FTE_VALUE_TYPE_MM:
        {
            snprintf(pUnitString, ulLen, "MM");
        }
        break;
        
     case    FTE_VALUE_TYPE_PRESSURE:
        {
            snprintf(pUnitString, ulLen, "kgf");
        }
        break;
        
    default:
        {
            snprintf(pUnitString, ulLen, "");
        }
    }
                     
    return  pUnitString;
}

FTE_CHAR_PTR    FTE_VALUE_printTimeStamp
(
    FTE_VALUE_PTR   pObj
)
{
    static  FTE_CHAR pString[16];
    
    ASSERT(pObj != NULL);
    
    snprintf(pString, sizeof(pString), "%d", pObj->xTimeStamp.SECONDS);
    
    return  pString;
}

FTE_BOOL    FTE_VALUE_equal
(
    FTE_VALUE_PTR   pValue1, 
    FTE_VALUE_PTR   pValue2
)
{
    ASSERT((pValue1 != NULL) && (pValue2 != NULL));
    
    if (pValue1->xType != pValue1->xType)
    {
        return  FALSE;
    }    
    
    switch(pValue1->xType)
    {
    case    FTE_VALUE_TYPE_TEMPERATURE:
    case    FTE_VALUE_TYPE_PRESSURE:
        {
            return  (pValue1->xData.nValue == pValue2->xData.nValue);
        }
        break;
        
    case    FTE_VALUE_TYPE_HUMIDITY:
    case    FTE_VALUE_TYPE_VOLTAGE:
    case    FTE_VALUE_TYPE_CURRENT:
    case    FTE_VALUE_TYPE_PWR_KWH:
    case    FTE_VALUE_TYPE_PWR_KW:
    case    FTE_VALUE_TYPE_PPM:
    case    FTE_VALUE_TYPE_PH:
    case    FTE_VALUE_TYPE_MM:
    case    FTE_VALUE_TYPE_ULONG:
    case    FTE_VALUE_TYPE_HEX32:
        {
            return  (pValue1->xData.nValue == pValue2->xData.nValue);
        }
        break;
        
    case    FTE_VALUE_TYPE_DIO:
    case    FTE_VALUE_TYPE_LED:
        {
            return  (pValue1->xData.bValue == pValue2->xData.bValue);
        }
        break;
        
    }

    return  FALSE;
}

FTE_RET FTE_VALUE_toINT16
(
    FTE_VALUE_PTR   pObj, 
    FTE_INT16_PTR  pValue
)
{
    ASSERT(pObj != NULL);
    ASSERT(pValue != NULL);
    
    if (pObj->bValid == FALSE)
    {
        *pValue = 0;
    }
    else
    {
        switch(pObj->xType)
        {
        case    FTE_VALUE_TYPE_TEMPERATURE:
        case    FTE_VALUE_TYPE_HUMIDITY:
        case    FTE_VALUE_TYPE_PRESSURE:
            {
                *pValue = pObj->xData.nValue;
            }
            break;
            
        case    FTE_VALUE_TYPE_DIO:
            {
                *pValue = pObj->xData.bValue;
            }
            break;
            
        case    FTE_VALUE_TYPE_LED:
            {
                
                *pValue = pObj->xData.xState;
            }
            break;
            
         case    FTE_VALUE_TYPE_VOLTAGE:
         case    FTE_VALUE_TYPE_CURRENT:
            {
                *pValue = pObj->xData.ulValue / 10;
            }
            break;
            
         case    FTE_VALUE_TYPE_PWR_KW:
         case    FTE_VALUE_TYPE_PWR_KWH:
            {
                *pValue = pObj->xData.ulValue / 10;
            }
            break;
            
         case    FTE_VALUE_TYPE_PPM:
         case    FTE_VALUE_TYPE_PH:
         case    FTE_VALUE_TYPE_MM:
         case    FTE_VALUE_TYPE_ULONG:
         case    FTE_VALUE_TYPE_HEX32:
            {
                *pValue = pObj->xData.ulValue;
            }
            break;
        }
    }
                     
    return  FTE_RET_OK;
}
