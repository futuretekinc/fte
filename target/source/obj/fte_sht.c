#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"
#include "fte_time.h"
#if FTE_SHT_SUPPORTED

static  _mqx_uint   _FTE_SHT_init(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _FTE_SHT_run(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _FTE_SHT_stop(FTE_OBJECT_PTR pObj);
static  void        _FTE_SHT_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static  _mqx_uint   _FTE_SHT_connectionReset(FTE_OBJECT_PTR pObj);
static  _mqx_uint   _FTE_SHT_startTransmission(FTE_OBJECT_PTR pObj);
static _mqx_uint    _FTE_SHT_startConvert(FTE_OBJECT_PTR pObj);
static  void        _FTE_SHT_restartConvert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);
static _mqx_uint    _FTE_SHT_getHumidity(FTE_OBJECT_PTR pObj, uint_32 *nHumidity);
static _mqx_uint    _FTE_SHT_getTemperature(FTE_OBJECT_PTR pObj, int_32 *pTemperature);
static uint_32      _FTE_SHT_getUpdateInterval(FTE_OBJECT_PTR pObj);
static _mqx_uint    _FTE_SHT_setUpdateInterval(FTE_OBJECT_PTR pObj, uint_32 nInterval);
static _mqx_uint    _FTE_SHT_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistics);

FTE_VALUE_TYPE  FTE_SHT_valueTypes[] =
{
    FTE_VALUE_TYPE_HUMIDITY,
    FTE_VALUE_TYPE_TEMPERATURE
};
static  FTE_OBJECT_ACTION _Action = 
{
    .f_init         = _FTE_SHT_init,
    .f_run          = _FTE_SHT_run,
    .f_stop         = _FTE_SHT_stop, 
    .f_set          = NULL,
    .f_get_update_interval = _FTE_SHT_getUpdateInterval,
    .f_set_update_interval = _FTE_SHT_setUpdateInterval,
    .f_get_statistic = _FTE_SHT_statistic,
};

static FTE_LIST _xObjList = {0, NULL, NULL};

_mqx_uint   FTE_SHT_attach(FTE_OBJECT_PTR pObj)
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
    
    if (FTE_LWGPIO_attach(pLWGPIO_SDA, pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }
    
    if (FTE_LWGPIO_attach(pLWGPIO_SCL, pConfig->xCommon.nID) != MQX_OK)
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
    FTE_LWGPIO_setDirection(pLWGPIO_SCL, LWGPIO_DIR_OUTPUT);
   
    pStatus->pLWGPIO_SDA = pLWGPIO_SDA;
    pStatus->pLWGPIO_SCL = pLWGPIO_SCL;
    
    pObj->pAction = (FTE_OBJECT_ACTION_PTR)&_Action;
  
    if (_FTE_SHT_init(pObj) != MQX_OK)
    {
        goto error;
    }

    FTE_LIST_pushBack(&_xObjList, pObj);
    
    return  MQX_OK;
    
error:

    if (pStatus->xCommon.pValue != NULL)
    {
        FTE_VALUE_destroy(pStatus->xCommon.pValue);
        pStatus->xCommon.pValue = NULL;
        pStatus->xCommon.nValueCount = 0;            
    }
        
    FTE_LWGPIO_detach(pLWGPIO_SDA);
    FTE_LWGPIO_detach(pLWGPIO_SCL);
    
    return  MQX_ERROR;
    
}

_mqx_uint FTE_SHT_detach(FTE_OBJECT_PTR pObj)
{
    if (!FTE_LIST_isExist(&_xObjList, pObj))
    {
        goto error;
    }

    FTE_LIST_remove(&_xObjList, pObj);
    pObj->pAction = NULL;
    
    return  MQX_OK;
    
error:    
    return  MQX_ERROR;
}

_mqx_uint   _FTE_SHT_init(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
  
    _FTE_SHT_connectionReset(pObj);
    
    return  MQX_OK;
}

_mqx_uint   _FTE_SHT_run(FTE_OBJECT_PTR pObj)
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
    if (pConfig->nInterval < 2)
    {
        _time_add_msec_to_ticks(&xDTicks, 1000);
    }
    else
    {
        _time_add_msec_to_ticks(&xDTicks, pConfig->nInterval * 500);
    }
    
    _time_get_elapsed_ticks(&pStatus->xCommon.xStartTicks);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    pStatus->hRepeatTimer = _timer_start_periodic_at_ticks(_FTE_SHT_restartConvert, pObj, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);
    
    pStatus->bHumidity = TRUE;
    _FTE_SHT_startConvert(pObj);
    
    _time_init_ticks(&xDTicks, _time_get_ticks_per_sec());
    pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_FTE_SHT_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);

    return  MQX_OK;
} 

_mqx_uint   _FTE_SHT_stop(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    
    return  MQX_OK;
    
}

static void _FTE_SHT_done(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;

    if (FTE_FLAG_IS_SET(pStatus->xCommon.xFlags, FTE_SHT_FLAG_CONVERT_HUMIDITY))
    {
        uint_32             nHumidity;
        
        if (_FTE_SHT_getHumidity(pObj, &nHumidity) == MQX_OK)
        {
            FTE_VALUE_setHumidity(&pStatus->xCommon.pValue[0], nHumidity);
            FT_OBJ_STAT_incSucceed(&pStatus->xCommon.xStatistics);
        }
        else
        {
            FTE_VALUE_setValid(&pStatus->xCommon.pValue[0], FALSE);
            FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
        }
        
        pStatus->bHumidity = FALSE;
        pStatus->xCommon.xFlags = FTE_FLAG_CLR(pStatus->xCommon.xFlags, FTE_SHT_FLAG_CONVERT_HUMIDITY);
    }
    else
    {
        int_32             nTemperature;
        
        if (_FTE_SHT_getTemperature(pObj, &nTemperature) == MQX_OK)
        {
            FTE_VALUE_setTemperature(&pStatus->xCommon.pValue[1], nTemperature);
            FT_OBJ_STAT_incSucceed(&pStatus->xCommon.xStatistics);
        }
        else
        {
            FTE_VALUE_setValid(&pStatus->xCommon.pValue[1], FALSE);
            FT_OBJ_STAT_incFailed(&pStatus->xCommon.xStatistics);
        }

        pStatus->bHumidity = TRUE;
        pStatus->xCommon.xFlags = FTE_FLAG_CLR(pStatus->xCommon.xFlags, FTE_SHT_FLAG_CONVERT_TEMPERATURE);
    }
}


static void _FTE_SHT_restartConvert(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)data_ptr;
    MQX_TICK_STRUCT     xDTicks;            
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;

    _time_get_elapsed_ticks(&pStatus->xCommon.xStartTicks);
    
    if (FTE_OBJ_IS_ENABLED(pObj))
    {
        _FTE_SHT_startConvert(pObj);
        
        _time_init_ticks(&xDTicks, _time_get_ticks_per_sec());
        pStatus->hConvertTimer = _timer_start_oneshot_after_ticks(_FTE_SHT_done, pObj, TIMER_ELAPSED_TIME_MODE, &xDTicks);
    }
    else
    {
        FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
        _timer_cancel(pStatus->hRepeatTimer);
        pStatus->hRepeatTimer = 0;
    }
}  

void    _sht_delay(uint_32  nDelay)
{
    _time_delay(0);
}

void    _sht_sck_hi(FTE_OBJECT_PTR pObj)
{
    FTE_LWGPIO_setValue(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SCL, TRUE);
    _sht_delay(1);
}

void    _sht_sck_lo(FTE_OBJECT_PTR pObj)
{
    FTE_LWGPIO_setValue(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SCL, FALSE);
    _sht_delay(1);
}

void    _sht_data_in_mode(FTE_OBJECT_PTR pObj)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, LWGPIO_DIR_INPUT);
    _sht_delay(1);
}

void    _sht_data_out_mode(FTE_OBJECT_PTR pObj)
{
    //FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, LWGPIO_DIR_OUTPUT);
    _sht_delay(1);
}

void    _sht_data_hi(FTE_OBJECT_PTR pObj)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, LWGPIO_DIR_INPUT);
    _sht_delay(1);
}

void    _sht_data_lo(FTE_OBJECT_PTR pObj)
{
    FTE_LWGPIO_setDirection(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, LWGPIO_DIR_OUTPUT);
    _sht_delay(1);
}

boolean _sht_data_get(FTE_OBJECT_PTR pObj)
{
    boolean bValue;
    
    FTE_LWGPIO_getValue(((FTE_SHT_STATUS_PTR)pObj->pStatus)->pLWGPIO_SDA, &bValue);

    return  bValue;
}

_mqx_uint   _FTE_SHT_connectionReset(FTE_OBJECT_PTR pObj)
{
    for(int i = 0 ; i < 9 ; i++)
    {
        _sht_sck_hi(pObj);
        _sht_sck_lo(pObj);
    }
    _sht_sck_hi(pObj);
    _sht_data_lo(pObj);
    _sht_sck_lo(pObj);
    _sht_sck_hi(pObj);
    _sht_data_hi(pObj);
    _sht_sck_lo(pObj);
    
    return  MQX_OK;
}


_mqx_uint   _FTE_SHT_startTransmission(FTE_OBJECT_PTR pObj)
{    
    _sht_sck_hi(pObj);
    _sht_data_lo(pObj);
    _sht_sck_lo(pObj);
    _sht_sck_hi(pObj);
    _sht_data_hi(pObj);
    _sht_sck_lo(pObj);
    _sht_data_lo(pObj);
    
    return  MQX_OK;
}
 
_mqx_uint   _sht_write(FTE_OBJECT_PTR pObj, uint_8_ptr pData, uint_32 nData)
{
    ASSERT(pObj != NULL);

    for(int i = 0 ; i < nData ; i++)
    {
        for(int j = 0 ; j < 8 ; j++)
        {
            if ((pData[i] >> (7-j)) & 0x01)
            {
                _sht_data_hi(pObj);
            }
            _sht_sck_hi(pObj);
            _sht_sck_lo(pObj);
            _sht_data_lo(pObj);
        }
    }
    
    _sht_data_in_mode(pObj);
    _sht_sck_hi(pObj);
    boolean bACK = _sht_data_get(pObj);
    _sht_sck_lo(pObj);
       
    if (!bACK)
    {
        return  MQX_OK;
    }
    else
    {
        return  MQX_ERROR;
    }
}

_mqx_uint   _sht_read(FTE_OBJECT_PTR pObj, uint_8_ptr pBuff, uint_32 nData)
{
    ASSERT(pObj != NULL);

    for(int i = 0 ; i < nData ; i++)
    {
        uint_8  nByte = 0;
        
        _sht_data_in_mode(pObj);
        for(int j = 0 ; j < 8 ; j++)
        {
            _sht_sck_hi(pObj);
            if (i < 2)
            {
                if (_sht_data_get(pObj) == TRUE)
                {
                    nByte |= 1 << (7 - j);
                }
            }
            else
            {
                if (_sht_data_get(pObj) == TRUE)
                {
                    nByte |= 1 << j;
                }
            }
            _sht_sck_lo(pObj);
        }
        
        pBuff[i] = nByte;
        
        _sht_data_out_mode(pObj);
        _sht_data_lo(pObj);
        _sht_sck_hi(pObj);
        _sht_sck_lo(pObj);
    }
            
    return  MQX_OK;
}

_mqx_uint   _sht_send_cmd(FTE_OBJECT_PTR pObj, uint_32 nAddr, uint_32 nCmd)
{
    uint_8  pData[1];
    ASSERT(pObj != NULL);

    
     pData[0] = ((nAddr & 0x07) << 5) | (nCmd & 0x1F);
    _FTE_SHT_startTransmission(pObj);
    
    return  _sht_write(pObj, pData, 1);
}

_mqx_uint   _FTE_SHT_startConvert(FTE_OBJECT_PTR pObj)
{
    ASSERT(pObj != NULL);
    FTE_SHT_STATUS_PTR  pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
    
    _FTE_SHT_connectionReset(pObj);
    
    if (pStatus->bHumidity)
    {
        pStatus->xCommon.xFlags = FTE_FLAG_SET(pStatus->xCommon.xFlags, FTE_SHT_FLAG_CONVERT_HUMIDITY);
        return  _sht_send_cmd(pObj, 0, 5);
    }
    else
    {
        pStatus->xCommon.xFlags = FTE_FLAG_SET(pStatus->xCommon.xFlags, FTE_SHT_FLAG_CONVERT_TEMPERATURE);
        return  _sht_send_cmd(pObj, 0, 3);
    }
}

uint_8 _FTE_SHT_crc8(uint_8 ubCRC, uint_8_ptr pBuff, uint_32 nLen)
{
    for(uint_8 i = 0 ; i < nLen ; i++)
    {
        uint_8  ubData = pBuff[i];
        
        for (uint_8 j = 0; j < 8; ++j) 
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
    
    return  ubCRC;
}

_mqx_uint   _FTE_SHT_getTemperature(FTE_OBJECT_PTR pObj, int_32 *pTemperature)
{
    uint_8  pBuff[4];
    int_32 nTemperature;
    ASSERT(pObj != NULL);
    
    pBuff[0] = 0x03;
    
    if ((_sht_read(pObj, &pBuff[1], 3) == MQX_OK) && (_FTE_SHT_crc8(0x00, pBuff, 4) == 0))
    {
        nTemperature = (int_32)((-39.6 + 0.01*((((int_32)pBuff[1] << 8) | pBuff[2]) & 0x3FFFF)) * 100);
        
        if (-4000 <= nTemperature && nTemperature <= 12000)
        {
            *pTemperature = nTemperature;
            return  MQX_OK;
        }
    }
    
    return  MQX_ERROR;
}

_mqx_uint   _FTE_SHT_getHumidity(FTE_OBJECT_PTR pObj, uint_32 *pHumidity)
{
    uint_8  pBuff[4];
    int_32  nHumidity;
    uint_8  ubCRC = 0;
    
    ASSERT(pObj != NULL);
    
    pBuff[0] = 0x05;
    
    if ((_sht_read(pObj, &pBuff[1], 3) == MQX_OK) && (_FTE_SHT_crc8(ubCRC, pBuff, 4) == 0))
    {
        int_32  nValue = ((int_32)pBuff[1] << 8) | pBuff[2];        
        nHumidity = (int_32)((-2.0468 + 0.0367*nValue + -0.0000015955*nValue*nValue) * 100);
        
        if (nHumidity <= 0)
        {
            *pHumidity = 0;
        }
        else if(nHumidity >= 10000)
        {
            *pHumidity = 10000;
        }
        else
        {
            *pHumidity = (uint_32)nHumidity;
        }
        
        return  MQX_OK;
    }
    
    return  MQX_ERROR;
}

uint_32      _FTE_SHT_getUpdateInterval(FTE_OBJECT_PTR pObj)
{
    FTE_SHT_CONFIG_PTR  pConfig = (FTE_SHT_CONFIG_PTR)pObj->pConfig;
    
    return  pConfig->nInterval;
}

_mqx_uint    _FTE_SHT_setUpdateInterval(FTE_OBJECT_PTR pObj, uint_32 nInterval)
{
    FTE_SHT_CONFIG_PTR  pConfig = (FTE_SHT_CONFIG_PTR)pObj->pConfig;
    
    pConfig->nInterval = nInterval;
    
    FTE_CFG_OBJ_save(pObj);
    
    return  MQX_OK;
}

_mqx_uint   _FTE_SHT_statistic(FTE_OBJECT_PTR pObj, FTE_OBJECT_STATISTICS_PTR pStatistics)
{
    ASSERT((pObj != NULL) && (pStatistics != NULL));
    
    FTE_SHT_STATUS_PTR    pStatus = (FTE_SHT_STATUS_PTR)pObj->pStatus;
        
    memcpy(pStatistics, &pStatus->xCommon.xStatistics, sizeof(FTE_OBJECT_STATISTICS));
    
    return  MQX_OK;
}

int_32      FTE_SHT_SHELL_cmd(int_32 argc, char_ptr argv[])
{ /* Body */
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        uint_32     nID;
        switch(argc)
        {
        case    1:
            {
                uint_32 count = FTE_OBJ_count(FTE_OBJ_TYPE_MULTI_SHT, 0, FALSE);
                uint_32 i;

                printf("%-8s %-16s %-16s %-8s %-16s %-s\n", 
                        "ID", "TYPE", "NAME", "STATUS", "VALUE", "TIME");
                for(i = 0 ; i < count ; i++)
                {
                    FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_UNKNOWN, 0, i, FALSE);
                    
                    if (pObj != NULL)
                    {
                        printf("%08x %-16s %-16s %8s", 
                              pObj->pConfig->xCommon.nID, 
                              FTE_OBJ_typeString(pObj), 
                              pObj->pConfig->xCommon.pName,
                              FTE_OBJ_IS_ENABLED(pObj)?"RUN":"STOP");                       
                        
                        if (pObj->pStatus->pValue != NULL)
                        {
                            TIME_STRUCT xTime;
                            char        pTimeString[64];
                            char        pValueString[32];
                            char        pUnitString[8];
                            
                            FTE_VALUE_toString(pObj->pStatus->pValue, pValueString, sizeof(pValueString));
                            FTE_VALUE_unit(pObj->pStatus->pValue, pUnitString, sizeof(pUnitString));
                            FTE_VALUE_getTimeStamp(pObj->pStatus->pValue, &xTime);
                            FTE_TIME_toString(&xTime, pTimeString, sizeof(pTimeString));
                            printf(" %8s %-7s %s", pValueString, pUnitString, pTimeString);
                        }
                        
                        if (pObj->pAction->f_get_statistic != NULL)
                        {
                            FTE_OBJECT_STATISTICS    xStatistics;
                            uint_32                 nRatio;
                            
                            pObj->pAction->f_get_statistic(pObj, &xStatistics);
                            nRatio = (xStatistics.nTotalTrial - xStatistics.nTotalFail) * 10000 / xStatistics.nTotalTrial;
                            
                            printf(" %3d.%02d%%(%d, %d, %d)", nRatio/100, nRatio%100, xStatistics.nTotalTrial, xStatistics.nTotalFail, xStatistics.nPartialFail);
                        }
                        printf("\n");
                    }
                }
            }
            break;

        case    2:
            {
                if (!Shell_parse_hexnum(argv[1], &nID))
                {
                    print_usage = TRUE;
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

                printf("%8s : %08x\n",      "ID",       pObj->pConfig->xCommon.nID);
                printf("%8s : %d secs\n",   "Interval", ((FTE_SHT_CONFIG_PTR)pObj->pConfig)->nInterval);
                printf("%8s : %d ms\n",     "Delay",    ((FTE_SHT_CONFIG_PTR)pObj->pConfig)->ulDelay);
            }                        
            break;
            
        case    4:
            {
                if (!Shell_parse_hexnum(argv[1], &nID))
                {
                    print_usage = TRUE;
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

                if (strcmp(argv[2], "scl") == 0)
                {
                    uint_32     ulDelay;
                    
                    if (!Shell_parse_uint_32(argv[3], &ulDelay))
                    {
                        print_usage = TRUE;
                        goto error;
                    }

                    if (ulDelay > FTE_SHT_DELAY_MAX)
                    {
                        print_usage = TRUE;
                        goto error;
                    }
                    
                    ((FTE_SHT_CONFIG_PTR)pObj->pConfig)->ulDelay = ulDelay;
                    
                    FTE_OBJ_save(pObj);
                }
            }                        
            break;
        default:
            print_usage = TRUE;
        }
    }
                
error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<id>] [<commands>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]  [<commands>]\n", argv[0]);
            printf("  Commands :\n");
            printf("    scl <ms>\n");
            printf("        SCL signal interval (0 ms ~ %d ms)\n", FTE_SHT_DELAY_MAX);
        }
    }
    return   return_code;
}

#endif