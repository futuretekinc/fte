#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"

#if  FTE_DS18B20_SUPPORTED

#define FTE_DS18B20_CONVERT_TIME    800

static FTE_RET  _FTE_DS18B20_init(FTE_OBJECT_PTR pObj);
static FTE_RET  _FTE_DS18B20_run(FTE_OBJECT_PTR pObj);
static FTE_RET  _FTE_DS18B20_stop(FTE_OBJECT_PTR pObj);
static void     _FTE_DS18B20_done(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);
static FTE_RET  _FTE_DS18B20_start_convert(FTE_OBJECT_PTR pObj);
static void     _FTE_DS18B20_restart_convert(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);
static FTE_RET  _FTE_DS18B20_get_temperature(FTE_OBJECT_PTR pObj, FTE_INT32_PTR pnTemperature);
static FTE_RET  _FTE_DS18B20_get_sn(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);
static FTE_RET  _FTE_DS18B20_get_update_interval(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pulInterval);
static FTE_RET  _FTE_DS18B20_set_update_interval(FTE_OBJECT_PTR pObj, FTE_UINT32 nInterval);
static FTE_UINT8   _FTE_DS18B20_crc(FTE_UINT8_PTR pData, FTE_UINT32 nLen, FTE_UINT8 nSeed);
static FTE_RET  _FTE_DS18B20_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistics);

static FTE_LIST _xObjList = {0, NULL, NULL};

static const 
FTE_DS18B20_CONFIG _default_config =
{
    .xCommon    = 
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_DS18B20, 0),
        .pName      = "DS18B20",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_DYNAMIC, 
        .ulChild    = 0,
        .pChild     = NULL
    },
    .nBUSID     = 0,
    .pROMCode   = {0x00,},
    .nInterval  = FTE_DS18B20_DEFAULT_UPDATE_INTERVAL
};

static  FTE_OBJECT_ACTION  _FTE_DS18B20_action = 
{
    .fInit  = _FTE_DS18B20_init,
    .fRun   = _FTE_DS18B20_run,
    .fStop  = _FTE_DS18B20_stop, 
    .fSet   = NULL,
    .fGetSN = _FTE_DS18B20_get_sn,
    .fGetInterval   = _FTE_DS18B20_get_update_interval,
    .fSetInterval   = _FTE_DS18B20_set_update_interval,
    .fGetStatistics = _FTE_DS18B20_statistic
};

FTE_OBJECT_PTR  FTE_DS18B20_create
(
    FTE_DS18B20_CREATE_PARAMS_PTR   pParams
)
{
    FTE_OBJECT_PTR  pObj;
    FTE_UINT32         nOID;
    FTE_UINT8          nIDs[255];    
    FTE_LIST_ITERATOR   xIter;
    
    ASSERT(pParams != NULL);
    
    memset(nIDs, 0, sizeof(nIDs));
    
    FTE_LIST_ITER_init(&_xObjList, &xIter);
    while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        nIDs[(pObj->pConfig->xCommon.nID & 0xFF) - 1] = 1;
    }

    for(int i = 0 ; i < 255 ; i++)
    {
        if (nIDs[i] == 0)
        {
            FTE_UINT32 ulCount = FTE_CFG_OBJ_count(FTE_OBJ_CLASS_TEMPERATURE, FTE_OBJ_CLASS_MASK);

            nOID = MAKE_ID(FTE_OBJ_TYPE_DS18B20, (ulCount+1));

            break;
        }
    }
    
    FTE_DS18B20_CONFIG_PTR   pConfig = (FTE_DS18B20_CONFIG_PTR)FTE_CFG_OBJ_alloc(nOID);
    if (pConfig == NULL)
    {
        printf("Not enough object memory!\n");
        goto error;
    }
                        
    pConfig->nBUSID         = pParams->nBUSID;
    pConfig->xCommon.nID    = nOID;
    pConfig->xCommon.xFlags = _default_config.xCommon.xFlags;
    pConfig->nInterval      = _default_config.nInterval;
    snprintf(pConfig->xCommon.pName, sizeof(pConfig->xCommon.pName), "%s-%d", _default_config.xCommon.pName, nOID);
    memcpy(pConfig->pROMCode, pParams->pROMCode, 8);
    
    pObj = FTE_OBJ_create((FTE_OBJECT_CONFIG_PTR)pConfig);    
    if (pObj == NULL)
    {
        FTE_CFG_OBJ_free(nOID);
        
        goto error;
    }
    
    return  pObj;

error:    
    return  NULL;    
}
    
FTE_RET   FTE_DS18B20_destroy
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_CFG_OBJ_free(pObj->pConfig->xCommon.nID);
    FTE_OBJ_destroy(pObj);
    
    return  FTE_RET_OK;    
}
    
FTE_RET   FTE_DS18B20_attach
(
    FTE_OBJECT_PTR  pObj, 
    FTE_VOID_PTR    pOpts
)
{
    ASSERT( pObj != NULL);
    
    FTE_RET xRet;
    FTE_DS18B20_CONFIG_PTR   pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    FTE_DS18B20_STATUS_PTR   pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    pStatus->xCommon.nValueCount= 1;
    pStatus->xCommon.pValue     = FTE_VALUE_createTemperature();
    if (pStatus->xCommon.pValue == NULL)
    {
        goto error;
    }
    
    xRet = FTE_1WIRE_get(pConfig->nBUSID, &pStatus->p1Wire);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }
#if 0    
    if (FTE_1WIRE_attach(pConfig->nBUSID, pObj->pConfig->nID) != FTE_RET_OK)
    {
        goto error;
    }
#endif
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_FTE_DS18B20_action;
    
    _FTE_DS18B20_init(pObj);
    
    FTE_LIST_pushBack(&_xObjList, pObj);

    
    return  FTE_RET_OK;
    
error:
    
    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.pValue = NULL;
        pStatus->xCommon.nValueCount = 0;
    }
    
    return  FTE_RET_ERROR;
    
}

FTE_RET FTE_DS18B20_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    if (FTE_LIST_isExist(&_xObjList, pObj) == FALSE)
    {
        goto error;
    }
    
    _FTE_DS18B20_stop(pObj);
    pObj->pAction = NULL;

    return  FTE_RET_OK;
    
error:    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_DS18B20_setROMCode
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT8       pROMCode[FTE_1WIRE_ROM_CODE_SIZE]
)
{
    ASSERT(pObj != NULL);
    
    memcpy(((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode, pROMCode, FTE_1WIRE_ROM_CODE_SIZE);
    
    FTE_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_BOOL         FTE_DS18B20_isValid(FTE_OBJECT_PTR pObj)
{
    FTE_UINT8  nTemp = 0;

    ASSERT(pObj != NULL);
    
    for(int i = 0 ; i < FTE_1WIRE_ROM_CODE_SIZE ; i++)
    {
        nTemp |= ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[i];
    }
    
    return  (nTemp != 0);
}

FTE_BOOL         FTE_DS18B20_isExistROMCode(FTE_UINT8 pROMCode[8])
{
    FTE_OBJECT_PTR pObj;
    FTE_LIST_ITERATOR xIter;
    
    FTE_LIST_ITER_init(&_xObjList, &xIter);
    while((pObj = (FTE_OBJECT_PTR)FTE_LIST_ITER_getNext(&xIter)) != NULL)
    {
        if (memcmp(((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode, pROMCode, 8) == 0)
        {
            return  TRUE;
        }
    }
    
    return  FALSE;
}


FTE_RET   _FTE_DS18B20_init
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);
//    FTE_DS18B20_STATUS_PTR pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;
    
    return  FTE_RET_OK;
}

FTE_RET   _FTE_DS18B20_run
(
    FTE_OBJECT_PTR  pObj
)
{ 
    ASSERT(pObj != NULL);

    MQX_TICK_STRUCT     xTicks, xDTicks;            
    
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    if (!FTE_DS18B20_isValid(pObj))
    {
        return  FTE_RET_ERROR;
    }
    
    _time_get_elapsed_ticks(&pStatus->xCommon.xStartTicks);
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_msec_to_ticks(&xTicks, 1000);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_FTE_DS18B20_restart_convert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);

    _FTE_DS18B20_start_convert(pObj);
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, FTE_DS18B20_CONVERT_TIME);    
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_FTE_DS18B20_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);

    return  FTE_RET_OK;
}

FTE_RET   _FTE_DS18B20_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_DS18B20_STATUS_PTR   pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    if (pStatus->hRepeatTimer != 0)
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
    
    if (pStatus->hConvertTimer != 0)
    {
        _timer_cancel(pStatus->hConvertTimer);
        pStatus->hConvertTimer = 0;
    }
    
    return  FTE_RET_OK;
}

static void _FTE_DS18B20_restart_convert
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR     pTick
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)pData;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    _time_get_elapsed_ticks(&pStatus->xCommon.xStartTicks);

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        _FTE_DS18B20_start_convert(pObj);
        
        _time_init_ticks(&xDTicks, 0);
        _time_add_msec_to_ticks(&xDTicks, FTE_DS18B20_CONVERT_TIME);    
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_FTE_DS18B20_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);
    }
    else 
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

static 
void _FTE_DS18B20_done
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR     pTick
)
{
    FTE_INT32                  nTemperature;
    FTE_OBJECT_PTR          pObj = (FTE_OBJECT_PTR)pData;
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    pStatus->hConvertTimer = 0;
    if (_FTE_DS18B20_get_temperature(pObj, &nTemperature) != FTE_RET_OK)
    {
        goto error;
    }
    
    if (nTemperature == FTE_DS18B20_INITIAL_VALUE)
    {
        goto error;
    }
    
    FTE_VALUE_setTemperature(pStatus->xCommon.pValue, nTemperature);
    FT_OBJ_STAT_incSucceed(&pStatus->xCommon.xStatistics);

    return;

error:
    
    FTE_VALUE_setValid(pStatus->xCommon.pValue, FALSE);
    FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
}

FTE_RET   _FTE_DS18B20_get_sn
(
    FTE_OBJECT_PTR  pObj, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      nLen
)
{
    ASSERT(pObj != NULL);
    
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    
    snprintf(pBuff, nLen, "%02x%02x%02x%02x%02x%02x%02x%02x",
             pConfig->pROMCode[0], pConfig->pROMCode[1],
             pConfig->pROMCode[2], pConfig->pROMCode[3],
             pConfig->pROMCode[4], pConfig->pROMCode[5],
             pConfig->pROMCode[6], pConfig->pROMCode[7]);
    
    return  FTE_RET_OK;
}

FTE_INT32  FTE_DS18B20_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (bPrintUsage)
    { 
        goto    error;
    }

    if (nArgc == 1)
    {
        int count = FTE_OBJ_count(FTE_OBJ_TYPE_DS18B20, FTE_OBJ_TYPE_MASK, FALSE);
        for(int i = 0 ; i < count ; i++)
        {
            FTE_OBJECT_PTR pObj =  FTE_OBJ_getAt(FTE_OBJ_TYPE_DS18B20, FTE_OBJ_TYPE_MASK, i, FALSE);
            if (pObj != NULL)
            {
                printf("%d: %16s %08x %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
                       i, pObj->pConfig->xCommon.pName, 
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->nBUSID,
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[0],
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[1],
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[2],
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[3],
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[4],
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[5],
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[6],
                       ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[7]);
                       
            }
        }
    } 
    else 
    {
        if (strcmp(pArgv[1], "attach") == 0)
        {
            FTE_DS18B20_CREATE_PARAMS   xParams;
            FTE_UINT32 nIndex;
            
            switch(nArgc)
            {
            case    3:
                {
                    if (strcmp(pArgv[2], "all") == 0)
                    {
                        FTE_1WIRE_PTR   p1Wire = FTE_1WIRE_getFirst();
                        while(p1Wire != NULL)
                        {
                            xParams.nBUSID = p1Wire->pConfig->nID;
                            
                            FTE_UINT32  ulDevCount = 0;
                            FTE_1WIRE_DEV_count(p1Wire, &ulDevCount);
                            for(nIndex = 0 ; nIndex < ulDevCount ; nIndex++)
                            {
                                if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != FTE_RET_OK)
                                {
                                   xRet = SHELL_EXIT_ERROR;
                                   goto error;
                                }
                                    
                                if (!FTE_DS18B20_isExistROMCode(xParams.pROMCode) )
                                {
                                    FTE_OBJECT_PTR          pObj = FTE_DS18B20_create(&xParams);
                                    if (pObj == NULL)
                                    {
                                       xRet = SHELL_EXIT_ERROR;
                                       goto error;
                                    }
                                    
                                    printf("%16s -> %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
                                            pObj->pConfig->xCommon.pName, 
                                            xParams.pROMCode[0],
                                            xParams.pROMCode[1],
                                            xParams.pROMCode[2],
                                            xParams.pROMCode[3],
                                            xParams.pROMCode[4],
                                            xParams.pROMCode[5],
                                            xParams.pROMCode[6],
                                            xParams.pROMCode[7]);

                                    FTE_OBJ_start(pObj);
                                }
                            }
                            
                            p1Wire = FTE_1WIRE_getNext(p1Wire);
                        }
                    }
                    else
                    {
                        if (FTE_strToUINT32( pArgv[2], &xParams.nBUSID) != FTE_RET_OK)  
                        {
                           xRet = SHELL_EXIT_ERROR;
                           goto error;
                        }

                        FTE_1WIRE_PTR   p1Wire;
                       
                        xRet = FTE_1WIRE_get(xParams.nBUSID, &p1Wire);
                        if (xRet != FTE_RET_OK)
                        {
                            xRet = SHELL_EXIT_ERROR;
                            goto error; 
                        }
                        
                        FTE_UINT32  ulDevCount = 0;
                        FTE_1WIRE_DEV_count(p1Wire, &ulDevCount);
                        for(nIndex = 0 ; nIndex < ulDevCount ; nIndex++)
                        {
                            if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != FTE_RET_OK)
                            {
                               xRet = SHELL_EXIT_ERROR;
                               goto error;
                            }
                                
                            FTE_OBJECT_PTR          pObj = FTE_DS18B20_create(&xParams);
                            if (pObj == NULL)
                            {
                               xRet = SHELL_EXIT_ERROR;
                               goto error;
                            }
                            
                            printf("%16s -> %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
                                    pObj->pConfig->xCommon.pName, 
                                    xParams.pROMCode[0],
                                    xParams.pROMCode[1],
                                    xParams.pROMCode[2],
                                    xParams.pROMCode[3],
                                    xParams.pROMCode[4],
                                    xParams.pROMCode[5],
                                    xParams.pROMCode[6],
                                    xParams.pROMCode[7]);

                            FTE_OBJ_start(pObj);
                        }
                    }
                }
                break;
                
            case    4:
                {
                    if (FTE_strToUINT32( pArgv[2], &xParams.nBUSID) != FTE_RET_OK)  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    if (FTE_strToUINT32( pArgv[3], &nIndex) != FTE_RET_OK)  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    FTE_1WIRE_PTR   p1Wire;
                   
                    xRet = FTE_1WIRE_get(xParams.nBUSID, &p1Wire);
                    if (xRet != FTE_RET_OK)
                    {
                        xRet = SHELL_EXIT_ERROR;
                        goto error; 
                    }
                    
                    
                    if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != FTE_RET_OK)
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                        
                    FTE_OBJECT_PTR          pObj = FTE_DS18B20_create(&xParams);
                    if (pObj == NULL)
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    printf("%16s -> %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
                            pObj->pConfig->xCommon.pName, 
                            xParams.pROMCode[0],
                            xParams.pROMCode[1],
                            xParams.pROMCode[2],
                            xParams.pROMCode[3],
                            xParams.pROMCode[4],
                            xParams.pROMCode[5],
                            xParams.pROMCode[6],
                            xParams.pROMCode[7]);

                    FTE_OBJ_start(pObj);
                }
                break;
                
            default:
                {
                    xRet = SHELL_EXIT_ERROR;
                    goto error;
                }
            }
        }
        else if (strcmp(pArgv[1], "detach") == 0)
        {
            switch(nArgc)
            {
            case    3:
                {
                    if (strcmp(pArgv[2], "all") == 0)
                    {
                        
                    }
                }
            }
        
        }
            
    }
    
error:    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]\n", pArgv[0]);
            printf("        id - DS18B20 Index \n");
        }
    }

    return   xRet;
}
            
FTE_RET   _FTE_DS18B20_start_convert
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT(pObj != NULL);

    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    FTE_1WIRE_lock(pStatus->p1Wire);
    FTE_1WIRE_reset(pStatus->p1Wire);
    FTE_1WIRE_writeByte(pStatus->p1Wire, 0x55);
    FTE_1WIRE_write(pStatus->p1Wire, pConfig->pROMCode, sizeof(pConfig->pROMCode));
    FTE_1WIRE_writeByte(pStatus->p1Wire, 0x44);
    FTE_1WIRE_unlock(pStatus->p1Wire);
    
    return  FTE_RET_OK;
}

FTE_RET   _FTE_DS18B20_get_temperature
(
    FTE_OBJECT_PTR  pObj, 
    FTE_INT32_PTR   pnTemperature
)
{
    FTE_UINT8  pValues[9];
    FTE_INT32  nValue;
    
    ASSERT(pObj != NULL);

    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    FTE_1WIRE_lock(pStatus->p1Wire);
    FTE_1WIRE_reset(pStatus->p1Wire);
    FTE_1WIRE_writeByte(pStatus->p1Wire, 0x55);
    FTE_1WIRE_write(pStatus->p1Wire, pConfig->pROMCode, sizeof(pConfig->pROMCode));
    FTE_1WIRE_writeByte(pStatus->p1Wire, 0xBE);
    FTE_1WIRE_read(pStatus->p1Wire, pValues, 9);
    FTE_1WIRE_unlock(pStatus->p1Wire);

    if ((pValues[7] == 0xFF) || (_FTE_DS18B20_crc(pValues, 9, 0) != 0))
    {
        return  FTE_RET_ERROR;
    }
    
    nValue = (*(int_16_ptr)pValues) * 625 / 100;
    if (nValue >= FTE_DS18B20_MAX_VALUE)
    {
        nValue = FTE_DS18B20_MAX_VALUE;
    }
    else if (nValue <= FTE_DS18B20_MIN_VALUE)
    {
        nValue = FTE_DS18B20_MIN_VALUE;
    }
    
    *pnTemperature = nValue;
    
    return  FTE_RET_OK;
}


FTE_RET _FTE_DS18B20_get_update_interval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32_PTR  pulInterval
)
{
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    
    *pulInterval = pConfig->nInterval;
    
    return  FTE_RET_OK;
}

FTE_RET    _FTE_DS18B20_set_update_interval
(
    FTE_OBJECT_PTR  pObj, 
    FTE_UINT32      nInterval
)
{
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  FTE_RET_OK;
}

FTE_UINT8 _FTE_DS18B20_crc(FTE_UINT8_PTR pData, FTE_UINT32 nLen, FTE_UINT8 nSeed)
{
    FTE_UINT8  nData;
    FTE_UINT32 nBitsLeft;
    FTE_UINT32 temp;
 
    for(int i = 0 ; i < nLen ; i++)
    {
        nData = pData[i];
        
        for (nBitsLeft = 8; nBitsLeft > 0; nBitsLeft--)
        {
            temp = ((nSeed ^ nData) & 0x01);
            if (temp == 0)
            {
                nSeed >>= 1;
            }
            else
            {
                nSeed ^= 0x18;
                nSeed >>= 1;
                nSeed |= 0x80;
            }
            nData >>= 1;
        }
    }
    
     return nSeed;    
 }

FTE_RET   _FTE_DS18B20_statistic
(
    FTE_OBJECT_PTR  pObj, 
    FTE_OBJECT_STATISTICS_PTR pStatistics
)
{
    ASSERT((pObj != NULL) && (pStatistics != NULL));
    
    FTE_DS18B20_STATUS_PTR    pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistics, &pStatus->xCommon.xStatistics, sizeof(FTE_OBJECT_STATISTICS));
    
    return  FTE_RET_OK;
}

#endif
