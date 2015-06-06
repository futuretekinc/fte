#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"

#if  FTE_DS18B20_SUPPORTED

#define FTE_DS18B20_CONVERT_TIME    800

static  _mqx_uint   _ds18b20_init(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _ds18b20_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _ds18b20_stop(FTE_OBJECT_PTR pObj);
static  void        _ds18b20_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static _mqx_uint    _ds18b20_start_convert(FTE_OBJECT_PTR pObj);
static  void        _ds18b20_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static _mqx_uint    _ds18b20_get_temperature(FTE_OBJECT_PTR pObj, int_32_ptr pnTemperature);
static  _mqx_uint   _ds18b20_get_sn(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen);
static uint_32      _ds18b20_get_update_interval(FTE_OBJECT_PTR pObj);
static _mqx_uint    _ds18b20_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval);
static  uint_8      _ds18b20_crc(uint_8_ptr pData, uint_32 nLen, uint_8 nSeed);
static  _mqx_uint   _ds18b20_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistics);

static FTE_LIST _xObjList = {0, NULL, NULL};

static const FTE_DS18B20_CONFIG _default_config =
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
    .nInterval  = FTE_DS18B20_INTERVAL
};

static  FTE_OBJECT_ACTION  _ds18b20_action = 
{
    .f_init         = _ds18b20_init,
    .f_run          = _ds18b20_run,
    .f_stop         = _ds18b20_stop, 
    .f_set          = NULL,
    .f_get_sn       = _ds18b20_get_sn,
    .f_get_update_interval = _ds18b20_get_update_interval,
    .f_set_update_interval = _ds18b20_set_update_interval,
    .f_get_statistic= _ds18b20_statistic
};

FTE_OBJECT_PTR  fte_ds18b20_create(FTE_DS18B20_CREATE_PARAMS_PTR pParams)
{
    FTE_OBJECT_PTR  pObj;
    uint_32         nOID;
    uint_8          nIDs[255];    
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
            uint_32 ulCount = FTE_CFG_OBJ_count(FTE_OBJ_CLASS_TEMPERATURE, FTE_OBJ_CLASS_MASK);

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
    
_mqx_uint   fte_ds18b20_destroy(FTE_OBJECT_PTR pObj)
{
    FTE_CFG_OBJ_free(pObj->pConfig->xCommon.nID);
    FTE_OBJ_destroy(pObj);
    
    return  MQX_OK;    
}
    
_mqx_uint   fte_ds18b20_attach(FTE_OBJECT_PTR pObj)
{
    ASSERT( pObj != NULL);
    
    FTE_DS18B20_CONFIG_PTR   pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    FTE_DS18B20_STATUS_PTR   pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    pStatus->xCommon.nValueCount= 1;
    pStatus->xCommon.pValue     = FTE_VALUE_createTemperature();
    if (pStatus->xCommon.pValue == NULL)
    {
        goto error;
    }
    
    pStatus->p1Wire = FTE_1WIRE_get(pConfig->nBUSID);
    if (pStatus->p1Wire == NULL)
    {
        goto error;
    }
#if 0    
    if (FTE_1WIRE_attach(pConfig->nBUSID, pObj->pConfig->nID) != MQX_OK)
    {
        goto error;
    }
#endif
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_ds18b20_action;
    
    _ds18b20_init(pObj);
    
    FTE_LIST_pushBack(&_xObjList, pObj);

    
    return  MQX_OK;
    
error:
    
    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.pValue = NULL;
        pStatus->xCommon.nValueCount = 0;
    }
    
    return  MQX_ERROR;
    
}

_mqx_uint fte_ds18b20_detach(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);

    if (FTE_LIST_isExist(&_xObjList, pObj) == FALSE)
    {
        goto error;
    }
    
    _ds18b20_stop(pObj);
    pObj->pAction = NULL;

    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

_mqx_uint       fte_ds18b20_set_rom_code(FTE_OBJECT_PTR pObj, uint_8 pROMCode[FTE_1WIRE_ROM_CODE_SIZE])
{
    ASSERT(pObj != NULL);
    
    memcpy(((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode, pROMCode, FTE_1WIRE_ROM_CODE_SIZE);
    
    FTE_OBJ_save(pObj);
    
    return  MQX_OK;
}

boolean         fte_ds18b20_is_valid(FTE_OBJECT_PTR pObj)
{
    uint_8  nTemp = 0;

    ASSERT(pObj != NULL);
    
    for(int i = 0 ; i < FTE_1WIRE_ROM_CODE_SIZE ; i++)
    {
        nTemp |= ((FTE_DS18B20_CONFIG_PTR)pObj->pConfig)->pROMCode[i];
    }
    
    return  (nTemp != 0);
}

boolean         fte_ds18b20_is_exist_rom_code(uint_8 pROMCode[8])
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


_mqx_uint   _ds18b20_init(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
//    FTE_DS18B20_STATUS_PTR pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;
    
    return  MQX_OK;
}

_mqx_uint   _ds18b20_run(FTE_OBJECT_PTR pObj)
{ 
    ASSERT(pObj != NULL);

    MQX_TICK_STRUCT     xTicks, xDTicks;            
    
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    if (!fte_ds18b20_is_valid(pObj))
    {
        return  MQX_ERROR;
    }
    
    _time_get_ticks(&pStatus->xCommon.xStartTicks);
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_sec_to_ticks(&xDTicks, pConfig->nInterval);
    _time_get_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_ds18b20_restart_convert, pObj, TIMER_KERNEL_TIME_MODE, &xTicks, &xDTicks);

    _ds18b20_start_convert(pObj);
    
    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, FTE_DS18B20_CONVERT_TIME);    
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_ds18b20_done, pObj, TIMER_KERNEL_TIME_MODE, &xDTicks);

    return  MQX_OK;
}

_mqx_uint   _ds18b20_stop(FTE_OBJECT_PTR pObj)
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
    
    return  MQX_OK;
}

static void _ds18b20_restart_convert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    _time_get_ticks(&pStatus->xCommon.xStartTicks);

    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        _ds18b20_start_convert(pObj);
        
        _time_init_ticks(&xDTicks, 0);
        _time_add_msec_to_ticks(&xDTicks, FTE_DS18B20_CONVERT_TIME);    
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_ds18b20_done, pObj, TIMER_KERNEL_TIME_MODE, &xDTicks);
    }
    else 
    {
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}

static void _ds18b20_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    int_32                  nTemperature;
    FTE_OBJECT_PTR          pObj = (FTE_OBJECT_PTR)data_ptr;
    FTE_DS18B20_STATUS_PTR  pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;

    pStatus->hConvertTimer = 0;
    if (_ds18b20_get_temperature(pObj, &nTemperature) != MQX_OK)
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

_mqx_uint   _ds18b20_get_sn(FTE_OBJECT_PTR pObj, char_ptr pBuff, uint_32 nLen)
{
    ASSERT(pObj != NULL);
    
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    
    snprintf(pBuff, nLen, "%02x%02x%02x%02x%02x%02x%02x%02x",
             pConfig->pROMCode[0], pConfig->pROMCode[1],
             pConfig->pROMCode[2], pConfig->pROMCode[3],
             pConfig->pROMCode[4], pConfig->pROMCode[5],
             pConfig->pROMCode[6], pConfig->pROMCode[7]);
    
    return  MQX_OK;
}

int_32  fte_ds18b20_shell_cmd(int_32 argc, char_ptr argv[] )
{ 
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (print_usage)
    { 
        goto    error;
    }

    if (argc == 1)
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
        if (strcmp(argv[1], "attach") == 0)
        {
            FTE_DS18B20_CREATE_PARAMS   xParams;
            uint_32 nIndex;
            
            switch(argc)
            {
            case    3:
                {
                    if (strcmp(argv[2], "all") == 0)
                    {
                        FTE_1WIRE_PTR   p1Wire = FTE_1WIRE_getFirst();
                        while(p1Wire != NULL)
                        {
                            xParams.nBUSID = p1Wire->pConfig->nID;
                            for(nIndex = 0 ; nIndex < FTE_1WIRE_DEV_count(p1Wire) ; nIndex++)
                            {
                                if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != MQX_OK)
                                {
                                   return_code = SHELL_EXIT_ERROR;
                                   goto error;
                                }
                                    
                                if (!fte_ds18b20_is_exist_rom_code(xParams.pROMCode) )
                                {
                                    FTE_OBJECT_PTR          pObj = fte_ds18b20_create(&xParams);
                                    if (pObj == NULL)
                                    {
                                       return_code = SHELL_EXIT_ERROR;
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

                                    pObj->pAction->f_run(pObj);
                                }
                            }
                            
                            p1Wire = FTE_1WIRE_getNext(p1Wire);
                        }
                    }
                    else
                    {
                        if (! Shell_parse_number( argv[2], &xParams.nBUSID))  
                        {
                           return_code = SHELL_EXIT_ERROR;
                           goto error;
                        }

                        FTE_1WIRE_PTR   p1Wire = FTE_1WIRE_get(xParams.nBUSID);
                        if (p1Wire == NULL)
                        {
                            return_code = SHELL_EXIT_ERROR;
                            goto error; 
                        }
                        
                        for(nIndex = 0 ; nIndex < FTE_1WIRE_DEV_count(p1Wire) ; nIndex++)
                        {
                            if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != MQX_OK)
                            {
                               return_code = SHELL_EXIT_ERROR;
                               goto error;
                            }
                                
                            FTE_OBJECT_PTR          pObj = fte_ds18b20_create(&xParams);
                            if (pObj == NULL)
                            {
                               return_code = SHELL_EXIT_ERROR;
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

                            pObj->pAction->f_run(pObj);
                        }
                    }
                }
                break;
                
            case    4:
                {
                    if (! Shell_parse_number( argv[2], &xParams.nBUSID))  
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    if (! Shell_parse_number( argv[3], &nIndex))  
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    FTE_1WIRE_PTR   p1Wire = FTE_1WIRE_get(xParams.nBUSID);
                    if (p1Wire == NULL)
                    {
                        return_code = SHELL_EXIT_ERROR;
                        goto error; 
                    }
                    
                    
                    if (FTE_1WIRE_DEV_getROMCode(p1Wire, nIndex, xParams.pROMCode) != MQX_OK)
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                        
                    FTE_OBJECT_PTR          pObj = fte_ds18b20_create(&xParams);
                    if (pObj == NULL)
                    {
                       return_code = SHELL_EXIT_ERROR;
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

                    pObj->pAction->f_run(pObj);
                }
                break;
                
            default:
                {
                    return_code = SHELL_EXIT_ERROR;
                    goto error;
                }
            }
        }
        else if (strcmp(argv[1], "detach") == 0)
        {
            switch(argc)
            {
            case    3:
                {
                    if (strcmp(argv[2], "all") == 0)
                    {
                        
                    }
                }
            }
        
        }
            
    }
    
error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<id>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]\n", argv[0]);
            printf("        id - DS18B20 Index \n");
        }
    }

    return   return_code;
}
            
_mqx_uint   _ds18b20_start_convert(FTE_OBJECT_PTR pObj)
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
    
    return  MQX_OK;
}

_mqx_uint   _ds18b20_get_temperature(FTE_OBJECT_PTR pObj, int_32_ptr pnTemperature)
{
    uint_8  pValues[9];
    int_32  nValue;
    
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

    if ((pValues[7] == 0xFF) || (_ds18b20_crc(pValues, 9, 0) != 0))
    {
        return  MQX_ERROR;
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
    
    return  MQX_OK;
}


uint_32      _ds18b20_get_update_interval(FTE_OBJECT_PTR pObj)
{
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

_mqx_uint    _ds18b20_set_update_interval(FTE_OBJECT_PTR pObj, uint_32 nInterval)
{
    FTE_DS18B20_CONFIG_PTR  pConfig = (FTE_DS18B20_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  MQX_OK;
}

uint_8 _ds18b20_crc(uint_8_ptr pData, uint_32 nLen, uint_8 nSeed)
{
    uint_8  nData;
    uint_32 nBitsLeft;
    uint_32 temp;
 
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

_mqx_uint   _ds18b20_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistics)
{
    ASSERT((pObj != NULL) && (pStatistics != NULL));
    
    FTE_DS18B20_STATUS_PTR    pStatus = (FTE_DS18B20_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistics, &pStatus->xCommon.xStatistics, sizeof(FTE_OBJECT_STATISTICS));
    
    return  MQX_OK;
}

#endif
