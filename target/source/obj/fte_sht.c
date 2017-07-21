#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#if FTE_SHT_SUPPORTED

#define POLYNOMIAL 0x131 
void        FTE_SHT_SCK_inMode(FTE_OBJECT_PTR  pObj);
void        FTE_SHT_SCK_setHi(FTE_OBJECT_PTR  pObj);
void        FTE_SHT_SCK_setlog(FTE_OBJECT_PTR  pObj);
void        FTE_SHT_SDA_inMode(FTE_OBJECT_PTR  pObj);
void        FTE_SHT_SDA_setHi(FTE_OBJECT_PTR  pObj);
void        FTE_SHT_SDA_setLo(FTE_OBJECT_PTR  pObj);

FTE_RET FTE_SHT_reset(FTE_OBJECT_PTR  pObj);
FTE_RET FTE_SHT_break(FTE_OBJECT_PTR  pObj);
FTE_RET FTE_SHT_setSingleShotDataAcquisitionMode(FTE_OBJECT_PTR  pObj, FTE_INT32_PTR pHumidity, FTE_INT32_PTR pTemperature, FTE_UINT32 nTimeout);
FTE_RET FTE_SHT_setPeriodicDataAcquisitionMode(FTE_OBJECT_PTR  pObj);
FTE_RET FTE_SHT_read(FTE_OBJECT_PTR  pObj, FTE_UINT8 nAddr, FTE_UINT8_PTR   pBuff, FTE_UINT32      nData);
FTE_RET FTE_SHT_writeByte(FTE_OBJECT_PTR  pObj, FTE_UINT8 nData);
FTE_RET FTE_SHT_write(FTE_OBJECT_PTR  pObj, FTE_UINT8 nAddr, FTE_UINT8_PTR   pData,     FTE_UINT32      nData, FTE_BOOL        bFinish);
FTE_RET FTE_SHT_startCondition(FTE_OBJECT_PTR   pObj);
FTE_RET FTE_SHT_stopCondition(FTE_OBJECT_PTR   pObj);
FTE_UINT8   FTE_SHT_calcCRC(FTE_UINT8_PTR  pData, FTE_UINT32 nLen);

FTE_RET FTE_SHT_init
(
    FTE_OBJECT_PTR  pObj
);

static  
FTE_RET FTE_SHT_run
(
    FTE_OBJECT_PTR  pObj
);

static  
FTE_RET FTE_SHT_stop
(
    FTE_OBJECT_PTR  pObj
);

static 
void FTE_SHT_restartConvert
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
);

static 
FTE_RET  FTE_SHT_startConvert
(
    FTE_OBJECT_PTR  pObj
);

static 
FTE_RET FTE_SHT_getUpdateInterval
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32_PTR  pulInterval
);

static 
FTE_RET  FTE_SHT_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
);

static 
FTE_RET  FTE_SHT_statistic
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTICS_PTR pStatistics
);

static 
FTE_RET  FTE_SHT_getChildCount
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pCount
);

static 
FTE_RET FTE_SHT_getChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_OBJECT_ID _PTR_ pxChildID
);

static
FTE_RET FTE_SHT_attachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID   xChildID
);

static
FTE_RET FTE_SHT_detachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID   xChildID
);

static const 
FTE_IFCE_CONFIG FTE_SHT_HUMIDITY_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0x0000),
        .pName      = "SHT-HUMI",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE | FTE_OBJ_CONFIG_FLAG_SYNC,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SHT, 0x0001),
    .nRegID     = FTE_SHT_FIELD_HUMI,
    .nInterval  = FTE_SHT_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_SHT_TEMPERATURE_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0x0000),
        .pName      = "SHT-TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE | FTE_OBJ_CONFIG_FLAG_SYNC,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SHT, 0x0000),
    .nRegID     = FTE_SHT_FIELD_TEMP,
    .nInterval  = FTE_SHT_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_OBJECT_CONFIG_PTR FTE_SHT_defaultChildConfigs [] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_SHT_HUMIDITY_defaultConfig ,
    (FTE_OBJECT_CONFIG_PTR)&FTE_SHT_TEMPERATURE_defaultConfig 
};

FTE_SHT_CONFIG FTE_SHT_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_SHT, 0x0001),
        .pName      = "SHT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE,
        .ulChild    = sizeof(FTE_SHT_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_SHT_defaultChildConfigs
    },
    .nSDA       = FTE_DEV_LWGPIO_SHT_SDA,
    .nSCL       = FTE_DEV_LWGPIO_SHT_SCL,
    .nInterval  = FTE_SHT_DEFAULT_UPDATE_INTERVAL,
    .ulDelay    = FTE_SHT_DEFAULT_SCL_INTERVAL
};


FTE_VALUE_TYPE  FTE_SHT_valueTypes[] =
{
    FTE_VALUE_TYPE_HUMIDITY,
    FTE_VALUE_TYPE_TEMPERATURE
};

static  
FTE_OBJECT_ACTION _Action = 
{
    .fInit  = FTE_SHT_init,
    .fRun   = FTE_SHT_run,
    .fStop  = FTE_SHT_stop, 
    .fSet   = NULL,
    .fGetInterval   = FTE_SHT_getUpdateInterval,
    .fSetInterval   = FTE_SHT_setUpdateInterval,
    .fGetStatistics = FTE_SHT_statistic,
    .fGetChildCount = FTE_SHT_getChildCount,
    .fGetChild      = FTE_SHT_getChild,
    .fAttachChild   = FTE_SHT_attachChild,
    .fDetachChild   = FTE_SHT_detachChild
};

static 
FTE_LIST _xObjList = {0, NULL, NULL};
static
FTE_BOOL    bTrace = 0;
FTE_CHAR    pTraceBuff[128];

FTE_RET   FTE_SHT_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT(pObj != NULL);
    
    FTE_SHT_CONFIG_PTR  pConfig = (FTE_SHT_CONFIG_PTR)pObj->pConfig;
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
    FTE_LWGPIO_PTR      pLWGPIO_SDA;
    FTE_LWGPIO_PTR      pLWGPIO_SCL;

    pLWGPIO_SDA = FTE_LWGPIO_get(pConfig->nSDA);
    if (pLWGPIO_SDA == NULL)
    {
        goto error;
    }

    pLWGPIO_SCL = FTE_LWGPIO_get(pConfig->nSCL);
    if (pLWGPIO_SCL == NULL)
    {
        goto error;
    }
    
    if (FTE_LWGPIO_attach(pLWGPIO_SDA, pConfig->xCommon.nID) != FTE_RET_OK)
    {
        goto error;
    }
    
    if (FTE_LWGPIO_attach(pLWGPIO_SCL, pConfig->xCommon.nID) != FTE_RET_OK)
    {
        goto error;
    }
    
    pStatus->xCommon.nValueCount = 2;
    pStatus->xCommon.pValue = FTE_VALUE_createArray(FTE_SHT_valueTypes, 2);
    if (pStatus->xCommon.pValue == NULL)
    {
        goto error;
    }
        
    FTE_LWGPIO_setValue(pLWGPIO_SDA, FALSE);
    FTE_LWGPIO_setValue(pLWGPIO_SCL, FALSE);

    FTE_LWGPIO_setDirection(pLWGPIO_SDA, LWGPIO_DIR_INPUT);
    FTE_LWGPIO_setDirection(pLWGPIO_SCL, LWGPIO_DIR_INPUT);
   
    pStatus->pLWGPIO_SDA = pLWGPIO_SDA;
    pStatus->pLWGPIO_SCL = pLWGPIO_SCL;
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_Action;
  
    if (FTE_SHT_init(pObj) != FTE_RET_OK)
    {
        goto error;
    }

    FTE_LIST_pushBack(&_xObjList, pObj);
    
    return  FTE_RET_OK;
    
error:

    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.pValue = NULL;
        pStatus->xCommon.nValueCount = 0;            
    }
        
    FTE_LWGPIO_detach(pLWGPIO_SDA);
    FTE_LWGPIO_detach(pLWGPIO_SCL);
    
    return  FTE_RET_ERROR;
    
}

FTE_RET FTE_SHT_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    if (!FTE_LIST_isExist(&_xObjList, pObj))
    {
        goto error;
    }

    FTE_LIST_remove(&_xObjList, pObj);
    pObj->pAction = NULL;
    
    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_SHT_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_SHT_reset(pObj);
    FTE_SHT_break(pObj);

    return  FTE_RET_OK;
}

FTE_RET   FTE_SHT_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;            
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
    FTE_SHT_CONFIG_PTR  pConfig = (FTE_SHT_CONFIG_PTR)pObj->pConfig;
    
    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    _time_init_ticks(&xDTicks, 0);
    if (pConfig->nInterval < FTE_SHT_DEFAULT_UPDATE_INTERVAL)
    {
        _time_add_msec_to_ticks(&xDTicks, FTE_SHT_DEFAULT_UPDATE_INTERVAL);
    }
    else
    {
        _time_add_msec_to_ticks(&xDTicks, pConfig->nInterval);
    }
    
    _time_get_elapsed_ticks(&pStatus->xCommon.xStartTicks);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_msec_to_ticks(&xTicks, 1000);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(FTE_SHT_restartConvert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);
    
    FTE_SHT_startConvert(pObj);
    
    return  FTE_RET_OK;
} 

FTE_RET   FTE_SHT_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    
    return  FTE_RET_OK;
    
}

static 
void FTE_SHT_restartConvert
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;

    _time_get_elapsed_ticks(&pStatus->xCommon.xStartTicks);
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        FTE_SHT_startConvert(pObj);
    }
    else
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}  



FTE_RET FTE_SHT_create
(
    FTE_OBJECT_CONFIG_PTR   pBaseConfig,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
    FTE_INT32 i;
    FTE_RET                 xRet;
    FTE_OBJECT_CONFIG_PTR   pConfig;
    FTE_OBJECT_CONFIG_PTR   pChildConfig[2];
    FTE_UINT32              ulChildCount = 0 ;
    FTE_OBJECT_PTR  pObj;
    
    if (FTE_LIST_count(&_xObjList) != 0)
    {
        return  FTE_RET_OBJECT_FULL;
    }
    
    if (pBaseConfig == NULL)
    {
        pBaseConfig = (FTE_OBJECT_CONFIG_PTR)&FTE_SHT_defaultConfig;
    }
    
    xRet = FTE_CFG_OBJ_create(pBaseConfig, &pConfig, pChildConfig, 2, &ulChildCount);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }    
    
    pObj = FTE_OBJ_create(pConfig);
    if (pObj == NULL)
    {
        return  FTE_RET_INSUFFICIENT_MEMORY;
    }
    
    if (ulChildCount != 0)
    {
        for(i = 0 ; i < ulChildCount ; i++)
        {
            FTE_OBJECT_PTR  pChild;
            
            pChild = FTE_OBJ_create(pChildConfig[i]);
            if (pChild == NULL)
            {
                ERROR("The child object creation failed.\n");
            }
        }
    }
    
    *ppObj = pObj;
        
    return  FTE_RET_OK;    
}

void    FTE_SHT_delay
(
    FTE_UINT32  nDelay
)
{
    for(int i = 0 ; i < 1000 ; i++);
    //_time_delay(1);
}

void    FTE_SHT_SCK_inMode
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SCL, LWGPIO_DIR_INPUT);
    FTE_SHT_delay(1);
}

void    FTE_SHT_SCK_setHi
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SCL, LWGPIO_DIR_INPUT);
    FTE_SHT_delay(1);
}

void    FTE_SHT_SCK_setLo
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SCL, LWGPIO_DIR_OUTPUT);
    FTE_SHT_delay(1);
}

void    FTE_SHT_SDA_inMode
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, LWGPIO_DIR_INPUT);
    FTE_SHT_delay(1);
}

void    FTE_SHT_SDA_setHi
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, LWGPIO_DIR_INPUT);
    FTE_SHT_delay(1);
}

void    FTE_SHT_SDA_setLo
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, LWGPIO_DIR_OUTPUT);
    FTE_SHT_delay(1);
}

FTE_BOOL FTE_SHT_SDA_get
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_BOOL bValue;
    
    FTE_LWGPIO_getValue(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, &bValue);

    return  bValue;
}

FTE_RET FTE_SHT_startCondition
(
    FTE_OBJECT_PTR   pObj
)
{
    FTE_SHT_SDA_inMode(pObj);
    FTE_SHT_SCK_inMode(pObj);
    FTE_SHT_SDA_setLo(pObj);
    FTE_SHT_SCK_setLo(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_SHT_stopCondition
(
    FTE_OBJECT_PTR   pObj
)
{
    FTE_SHT_SCK_setLo(pObj);
    FTE_SHT_SDA_setLo(pObj);
    FTE_SHT_SCK_inMode(pObj);
    FTE_SHT_SDA_inMode(pObj);
   
    return  FTE_RET_OK;
}

FTE_RET FTE_SHT_writeByte
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT8       nData
)
{
    for(FTE_INT32 j = 0 ; j < 8 ; j++)
    {
        if ((nData >> (7-j)) & 0x01)
        {
            FTE_SHT_SDA_setHi(pObj);
        }
        else
        {
            FTE_SHT_SDA_setLo(pObj);
        }
        FTE_SHT_SCK_setHi(pObj);
        FTE_SHT_SCK_setLo(pObj);
    }

    FTE_SHT_SDA_inMode(pObj);
    FTE_SHT_SCK_setHi(pObj);
    FTE_BOOL    bNACK = FTE_SHT_SDA_get(pObj);
    FTE_SHT_SCK_setLo(pObj);
    
    if (bNACK)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_SHT_readByte
(
    FTE_OBJECT_PTR  pObj,
    FTE_UINT8_PTR   pData,
    FTE_BOOL        bAck
)
{
    FTE_UINT8   nByte = 0;
    
    FTE_SHT_SDA_inMode(pObj);
    for(FTE_INT32 j = 0 ; j < 8 ; j++)
    {
        FTE_SHT_SCK_setHi(pObj);
        FTE_SHT_delay(1);
        if (FTE_SHT_SDA_get(pObj) == TRUE)
        {
            nByte |= 1 << (7 - j);
        }
        FTE_SHT_SCK_setLo(pObj);
    }
        
    *pData = nByte;
    
    if (bAck)
    {
        FTE_SHT_SDA_setLo(pObj);
    }
    else
    {
        FTE_SHT_SDA_setHi(pObj);
    }
    
    FTE_SHT_SCK_setHi(pObj);
    FTE_SHT_SCK_setLo(pObj);
    FTE_SHT_SDA_inMode(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SHT_write
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT8       nAddr,
    FTE_UINT8_PTR   pData, 
    FTE_UINT32      nData,
    FTE_BOOL        bFinish
)
{
    ASSERT(pObj != NULL);
    
    FTE_SHT_startCondition(pObj);

    nAddr = nAddr << 1;
    
    if (FTE_SHT_writeByte(pObj, nAddr) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    for(FTE_INT32 i = 0 ; i < nData ; i++)
    {
        if (FTE_SHT_writeByte(pObj, pData[i]) != FTE_RET_OK)
        {        
           return  FTE_RET_ERROR;
        }
    }
   
    if (bFinish)
    {
        FTE_SHT_startCondition(pObj);    
    }
      
    return  FTE_RET_OK;
}


FTE_RET   FTE_SHT_read
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT8       nAddr,
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      nData
)
{
    ASSERT(pObj != NULL);

    FTE_SHT_startCondition(pObj);
    
    nAddr = (nAddr << 1) | 0x01;
    
    if (FTE_SHT_writeByte(pObj, nAddr) != FTE_RET_OK)
    {
        FTE_SHT_startCondition(pObj);    
        return  FTE_RET_ERROR;
    }
    
    for(FTE_INT32 i = 0 ; i < nData ; i++)
    {
        FTE_SHT_readByte(pObj, &pBuff[i], (i != nData -1));
    }

    FTE_SHT_stopCondition(pObj);    
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SHT_startConvert
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
    FTE_RET             xRet;
    FTE_OBJECT_PTR      pChild = NULL;
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
    FTE_INT32           nHumidity = 0, nTemperature = 0;
    
    xRet = FTE_SHT_setSingleShotDataAcquisitionMode(pObj, &nHumidity, &nTemperature, 100);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
     
    FTE_VALUE_setHumidity(&pStatus->xCommon.pValue[FTE_SHT_FIELD_HUMI], nHumidity);
    FT_OBJ_STAT_incSucceed(&pStatus->xCommon.xStatistics);

    xRet = FTE_OBJ_getChild(pObj, FTE_SHT_FIELD_HUMI, &pChild);
    if (xRet == FTE_RET_OK)
    {
        FTE_VALUE   xValue;
        
        FTE_VALUE_copy(&xValue, pChild->pStatus->pValue);

        FTE_VALUE_copy(pChild->pStatus->pValue, &pStatus->xCommon.pValue[FTE_SHT_FIELD_HUMI]);
        
        if (!FTE_VALUE_equal(&xValue, pChild->pStatus->pValue))
        {
            FTE_OBJ_wasChanged(pChild);
        }
        else
        {
            FTE_OBJ_wasUpdated(pChild);
        }
    }

    
    FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[FTE_SHT_FIELD_TEMP], nTemperature);
    FT_OBJ_STAT_incSucceed(&pStatus->xCommon.xStatistics);

    xRet = FTE_OBJ_getChild(pObj, FTE_SHT_FIELD_TEMP, &pChild);
    if (xRet == FTE_RET_OK)
    {
        FTE_VALUE   xValue;
        
        FTE_VALUE_copy(&xValue, pChild->pStatus->pValue);

        FTE_VALUE_copy(pChild->pStatus->pValue, &pStatus->xCommon.pValue[FTE_SHT_FIELD_TEMP]);
        
        if (!FTE_VALUE_equal(&xValue, pChild->pStatus->pValue))
        {
            FTE_OBJ_wasChanged(pChild);
        }
        else
        {
            FTE_OBJ_wasUpdated(pChild);
        }
    }

    return  FTE_RET_OK;
}

FTE_UINT8 FTE_SHT_crc8
(
    FTE_UINT8       ubCRC, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      nLen
)
{
#if 0
    for(FTE_UINT8 i = 0 ; i < nLen ; i++)
    {
        FTE_UINT8  ubData = pBuff[i];
        
        for (FTE_UINT8 j = 0; j < 8; ++j) 
        {
            if ((ubCRC ^ ubData) & 0x80) 
            {
                ubCRC <<= 1;
                ubCRC ^= 0x31;
            } 
            else
            {
                ubCRC <<= 1;
            }
            ubData <<= 1;
        }
    }
#else
     for(FTE_UINT8 i = 0; i < nLen ; i++)
     {
         ubCRC ^= (pBuff[i]);
        for(FTE_UINT8 j = 8; j > 0; --j)
        {
            if(ubCRC & 0x80)
            {
               ubCRC = (ubCRC << 1) ^ 0x131;
            }
            else
            {
                ubCRC = (ubCRC << 1);
            }
        }
    }
    
#endif
    return  ubCRC;
}

FTE_UINT32      FTE_SHT_getUpdateInterval
(   
    FTE_OBJECT_PTR  pObj,
    FTE_UINT32_PTR  pulInterval
)
{
    FTE_SHT_CONFIG_PTR  pConfig = (FTE_SHT_CONFIG_PTR)pObj->pConfig;
    
    *pulInterval = pConfig->nInterval;
    
    return  FTE_RET_OK;
}

FTE_RET    FTE_SHT_setUpdateInterval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_SHT_CONFIG_PTR  pConfig = (FTE_SHT_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_SHT_statistic
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTICS_PTR pStatistics
)
{
    ASSERT((pObj != NULL) && (pStatistics != NULL));
    
    FTE_SHT_STATUS_PTR    pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistics, &pStatus->xCommon.xStatistics, sizeof(FTE_OBJECT_STATISTICS));
    
    return  FTE_RET_OK;
}

FTE_RET  FTE_SHT_getChildCount
(   
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pCount
)
{
    ASSERT((pObj != NULL) && (pCount != NULL));
    
    *pCount = pObj->pStatus->nValueCount;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_SHT_getChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      ulIndex, 
    FTE_OBJECT_ID _PTR_ pxChildID
)
{
    ASSERT((pObj != NULL) && (pxChildID != NULL));
    FTE_UINT32  ulGroupID;
    
    if (ulIndex >= 2)
    {
        return  FTE_RET_OBJECT_NOT_FOUND;
    }

    ulGroupID = (pObj->pConfig->xCommon.nID & 0x00FF0000) >> 8;
    
    for(FTE_INT32 i = 0 ; i < FTE_OBJ_count(ulGroupID, 0x0000FF00, FALSE); i++)
    {
        FTE_OBJECT_PTR  pChild;
        
        pChild = FTE_OBJ_getAt(ulGroupID, 0x0000FF00, i, FALSE);
        if ((pChild != NULL) && (((FTE_IFCE_CONFIG_PTR)pChild->pConfig)->nRegID == ulIndex))
        {
            *pxChildID = pChild->pConfig->xCommon.nID;
            
            return  FTE_RET_OK;
        }
    }

    return  FTE_RET_OBJECT_NOT_FOUND;
}

FTE_RET FTE_SHT_attachChild
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_ID   xChildID
)
{
    ASSERT(pObj != NULL);
    
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;

    FTE_UINT32  ulGroupID = ((pObj->pConfig->xCommon.nID & 0x00FF0000) >> 8);
    
    if (ulGroupID != (xChildID & 0x0000FF00))
    {
        return  FTE_RET_INVALID_OBJECT;
    }
    
    switch(xChildID & FTE_OBJ_CLASS_MASK)
    {
    case FTE_OBJ_CLASS_TEMPERATURE:
        {
            if (pStatus->xTObjID != 0)
            {
                return  RTE_RET_ALREADY_EXISTS;
            }
            
            pStatus->xTObjID = xChildID;
        }
        break;
        
    case FTE_OBJ_CLASS_HUMIDITY:
        {
            if (pStatus->xHObjID != 0)
            {
                return  RTE_RET_ALREADY_EXISTS;
            }
            
            pStatus->xHObjID = xChildID;
        }
        break;
     
    default:
        return  FTE_RET_INVALID_OBJECT;
    }
    
    
    return  FTE_RET_OK;
}

FTE_RET FTE_SHT_detachChild
(
    FTE_OBJECT_PTR pObj, 
    FTE_OBJECT_ID   xChildID
)
{
    ASSERT(pObj != NULL);
    
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;

    FTE_UINT32  ulGroupID = ((pObj->pConfig->xCommon.nID & 0x00FF0000) >> 8);
    
    if (ulGroupID != (xChildID & 0x0000FF00))
    {
        return  FTE_RET_INVALID_OBJECT;
    }
    
    switch(xChildID & FTE_OBJ_CLASS_MASK)
    {
    case FTE_OBJ_CLASS_TEMPERATURE:
        {
            if (pStatus->xTObjID == 0)
            {
                return  FTE_RET_OBJECT_NOT_FOUND;
            }
            else if (pStatus->xTObjID != xChildID)
            {
                return  FTE_RET_INVALID_OBJECT;
            }
            
            pStatus->xTObjID = 0;
        }
        break;
        
    case FTE_OBJ_CLASS_HUMIDITY:
        {
            if (pStatus->xHObjID == 0)
            {
                return  FTE_RET_OBJECT_NOT_FOUND;
            }
            else if (pStatus->xHObjID != xChildID)
            {
                return  FTE_RET_INVALID_OBJECT;
            }
            
            pStatus->xHObjID = 0;
        }
        break;
     
    default:
        return  FTE_RET_INVALID_OBJECT;
    }
    
    
    return  FTE_RET_OK;
}

FTE_INT32   FTE_SHT_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{ /* Body */
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   return_code = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        FTE_UINT32     nID;
        switch(nArgc)
        {
        case    1:
            {
                FTE_CHAR    pBuff[32];
                FTE_UINT32  ulCount = FTE_LIST_count(&_xObjList);
                FTE_UINT32  i;

                printf("%8s %8s %8s %8s %8s %8s\n", "ID", "STATE", "TEMP.", "HUMI.", "INTERVAL", "DELAY");
                for(i = 0 ; i < ulCount ; i++)
                {
                    FTE_OBJECT_PTR  pObj = (FTE_OBJECT_PTR)FTE_LIST_getAt(&_xObjList, i);
                    if (pObj != NULL)
                    {
                        FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
                        FTE_SHT_CONFIG_PTR  pConfig = (FTE_SHT_CONFIG_PTR)pObj->pConfig;
                        
                        printf("%08x %8s", 
                              pObj->pConfig->xCommon.nID, 
                              FTE_OBJ_IS_ENABLED(pObj)?"RUN":"STOP");

                        FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_SHT_FIELD_TEMP], pBuff, sizeof(pBuff) - 1);
                        printf(" %8s", pBuff);
                        
                        FTE_VALUE_toString(&pStatus->xCommon.pValue[FTE_SHT_FIELD_HUMI], pBuff, sizeof(pBuff) - 1);
                        printf(" %8s", pBuff);
                        
                        printf(" %5d ms %5d ms\n",   pConfig->nInterval, pConfig->ulDelay);

                    }
                }
            }
            break;

        case    3:
            {
                if (strcasecmp(pArgv[1], "trace") == 0)
                {
                    if (strcasecmp(pArgv[2], "on") == 0)
                    {
                        bTrace = 1;
                    }
                    else if (strcasecmp(pArgv[2], "off") == 0)
                    {
                        bTrace = 0;
                    }
                    else if (strcasecmp(pArgv[2], "print") == 0)
                    {
                        printf("%s", pTraceBuff);
                    }
                }
            }
            break;
            
        case    4:
            {
                if (FTE_strToHex(pArgv[1], &nID) != FTE_RET_OK)
                {
                    bPrintUsage = TRUE;
                    goto error;
                }

                FTE_OBJECT_PTR  pObj = FTE_OBJ_get(nID);
                if (pObj == NULL)
                {
                    printf("Object[%08x] not found!\n", nID);
                    goto error;
                }
            
                if (FTE_OBJ_TYPE(pObj) != FTE_OBJ_TYPE_MULTI_SHT)
                {
                    printf("Object type[%08x] mismatch!\n", FTE_OBJ_TYPE(pObj));
                    goto error;
                }

                if (strcmp(pArgv[2], "scl") == 0)
                {
                    FTE_UINT32     ulDelay;
                    
                    if (FTE_strToUINT32(pArgv[3], &ulDelay) != FTE_RET_OK)
                    {
                        bPrintUsage = TRUE;
                        goto error;
                    }

                    if (ulDelay > FTE_SHT_DELAY_MAX)
                    {
                        bPrintUsage = TRUE;
                        goto error;
                    }
                    
                    ((FTE_SHT_CONFIG_PTR)pObj->pConfig)->ulDelay = ulDelay;
                    
                    FTE_OBJ_save(pObj);
                }
            }                        
            break;
        default:
            bPrintUsage = TRUE;
        }
    }
                
error:    
    if (bPrintUsage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>] [<commands>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]  [<commands>]\n", pArgv[0]);
            printf("  Commands :\n");
            printf("    scl <ms>\n");
            printf("        SCL signal interval (0 ms ~ %d ms)\n", FTE_SHT_DELAY_MAX);
        }
    }
    return   return_code;
}

FTE_RET   FTE_SHT_reset
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_RET     xRet;
    FTE_UINT8   pData[2] = {0x30, 0xA2};
    
    xRet = FTE_SHT_write(pObj, 0x44, pData, sizeof(pData), 1);
    if (xRet != FTE_RET_OK)
    {
        printf("Failed to break operation!\n");
    }
    else
    {
        _time_delay(20);
    }
    
    return  xRet;
}


FTE_RET   FTE_SHT_break
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_RET     xRet;
    FTE_UINT8   pData[2] = {0x30, 0x93};
    
    xRet = FTE_SHT_write(pObj, 0x44, pData, sizeof(pData), 1);
    if (xRet != FTE_RET_OK)
    {
        printf("Failed to break operation!\n");
    }
    else
    {
        _time_delay(20);
    }
    
    return  xRet;
}


FTE_RET   FTE_SHT_setPeriodicDataAcquisitionMode
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_RET     xRet;
    FTE_UINT8   pData[2] = {0x21, 0x30};
    
    xRet = FTE_SHT_write(pObj, 0x44, pData, sizeof(pData), 1);
    if (xRet != FTE_RET_OK)
    {
        printf("Faile to set periodic data acquisition mode!\n");
    }
    else
    {
        printf("Periodic data acquisition mode!\n");    
    }
    return  xRet;
}

FTE_RET   FTE_SHT_setSingleShotDataAcquisitionMode
(
    FTE_OBJECT_PTR  pObj,
    FTE_INT32_PTR   pHumidity,
    FTE_INT32_PTR   pTemperature,
    FTE_UINT32      nTimeout
)
{
    FTE_RET     xRet;
    FTE_UINT8   pData[2] = {0x24, 0x00};
    
    xRet = FTE_SHT_write(pObj, 0x44, pData, sizeof(pData), 1);
    if (xRet != FTE_RET_OK)
    {
        printf("Faile to set periodic data acquisition mode!\n");
    }
    else
    {
        FTE_RET xRet;
        FTE_UINT8   pBuff[6];
        FTE_TIME    xCurrentTime, xStartTime;
        FTE_INT32   nDiffTime = 0;
        
        FTE_TIME_getCurrent(&xStartTime);

        while(1)
        {
            xRet = FTE_SHT_read(pObj, 0x44, pBuff, sizeof(pBuff));
            if (xRet == FTE_RET_OK)
            {
                if ((FTE_SHT_crc8(0xFF, &pBuff[0], 2) == pBuff[2]) && (FTE_SHT_crc8(0xFF, &pBuff[3], 2) == pBuff[5]))
                {
                    *pTemperature = ((FTE_INT32)pBuff[0] * 256 + pBuff[1]) * 175 * 100 / 65535 - 4500;
                    *pHumidity = ((FTE_INT32)pBuff[3] * 256 + pBuff[4]) * 10000 / 65535;
                    
                    if (bTrace)
                    {
                        snprintf(pTraceBuff, sizeof(pTraceBuff) - 1, "Data : %02x %02x %02x %02x %02x %02x\n", pBuff[0], pBuff[1], pBuff[2], pBuff[3], pBuff[4], pBuff[5]);
                    }
                }
                else
                {
                    printf("Checksum invalid!");
                    xRet = FTE_RET_INVALID_CHECKSUM;
                }
                break;
            }
            
            FTE_TIME_getCurrent(&xCurrentTime);
            FTE_TIME_diffMilliseconds(&xCurrentTime, &xStartTime, &nDiffTime);            

            if (nDiffTime > nTimeout)
            {
                xRet = FTE_RET_ERROR;
                break;
            }
            _time_delay(1);
        }
    }
    return  xRet;
}

#endif
