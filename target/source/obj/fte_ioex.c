#include "fte_target.h"
#include "fte_crc.h"
#include "fte_ioex.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"
#include "fte_task.h"
#include "fte_debug.h"

#ifndef FTE_IOEX_PACKET_LEN_MAX 
#define FTE_IOEX_PACKET_LEN_MAX 32
#endif

#ifndef FTE_IOEX_DEFAULT_LOOP_PERIOD
#define FTE_IOEX_DEFAULT_LOOP_PERIOD        100 // ms
#endif

#ifndef FTE_IOEX_DEFAULT_UPDATE_PERIOD
#define FTE_IOEX_DEFAULT_UPDATE_PERIOD      3000 // ms
#endif

#ifndef FTE_IOEX_DEFAULT_REQUEST_TIMEOUT
#define FTE_IOEX_DEFAULT_REQUEST_TIMEOUT    500
#endif

#ifndef FTE_IOEX_DEFAULT_RETRY_COUNT
#define FTE_IOEX_DEFUALT_RETRY_COUNT        3
#endif

#define FTE_IOEX_DEBUG  1

#if  FTE_IOEX_DEBUG
#define FTE_IOEX_DUMP_SEND_PKT(x,y) { if (bTraceOn && bTraceTxPkt) DUMP("TX", (x), (y), 0); }
#define FTE_IOEX_DUMP_RECV_PKT(x,y) { if (bTraceOn && bTraceRxPkt) DUMP("RX", (x), (y), 0); }
static  FTE_BOOL     bTraceOn = FALSE;
static  FTE_BOOL     bTraceRxPkt = TRUE;
static  FTE_BOOL     bTraceTxPkt = TRUE;
#else
#define FTE_IOEX_DUMP(x,y)   
#endif

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI0_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 0,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI1_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 1,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI2_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI3",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 2,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI3_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI4",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 3,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI4_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI5",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 4,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI5_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI6",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 5,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI6_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI7",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 6,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI7_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI8",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 7,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI8_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI9",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 8,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI9_defaultConfig  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI10",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 9,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI10_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI11",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 10,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI11_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI12",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 11,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI12_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI13",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 12,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI13_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI14",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 13,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI14_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI15",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 14,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};

static const 
FTE_IFCE_CONFIG FTE_IOEX_DI15_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "DI16",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
    .nRegID     = 15,
    .nInterval  = FTE_IOEX_DI_DEFAULT_UPDATE_INTERVAL
};


static const 
FTE_OBJECT_CONFIG_PTR FTE_IOEX_defaultChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI0_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI1_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI2_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI3_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI4_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI5_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI6_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI7_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI8_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI9_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI10_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI11_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI12_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI13_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI14_defaultConfig,
    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_DI15_defaultConfig,
};

FTE_GUS_CONFIG FTE_IOEX_defaultConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_IOEX, 0x0001),
        .pName      = "IOEX",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
        .ulChild    = sizeof(FTE_IOEX_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_IOEX_defaultChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_IOEX,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_IOEX_DEFAULT_UPDATE_INTERVAL
};

static  
FTE_IOEX_EXT_CONFIG xGlobalConfig = 
{
    .ulLoopPeriod = FTE_IOEX_DEFAULT_LOOP_PERIOD,
    .ulUpdatePeriod = FTE_IOEX_DEFAULT_UPDATE_PERIOD
};

FTE_VALUE_TYPE  FTE_IOEX_valueTypes[] =
{
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
};

const 
FTE_GUS_MODEL_INFO    FTE_IOEX_GUSModelInfo =
{
    .nModel         = FTE_GUS_MODEL_IOEX,
    .pName          = "IOEX",
    .nFieldCount    = FTE_IOEX_DI_MAX,
    .pValueTypes    = FTE_IOEX_valueTypes,
    .fAttach        = FTE_IOEX_attach,
    .fDetach        = FTE_IOEX_detach,
    .fGet           = FTE_IOEX_get,
};

FTE_RET   FTE_IOEX_attach
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_IOEX_STATUS_PTR pStatus;
    FTE_UCS_PTR         pUCS = NULL;
    
    ASSERT(pObj != NULL);

    pUCS = (FTE_UCS_PTR)FTE_UCS_get(((FTE_IOEX_CONFIG_PTR)pObj->pConfig)->xGUS.nUCSID);
    pStatus = (FTE_IOEX_STATUS_PTR)pObj->pStatus;    
    if (pUCS == NULL)
    {
        goto error;
    }
        
    if (FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID) != FTE_RET_OK)
    {
        goto error;
    }
        
    pStatus->xGUS.pUCS = pUCS;
    
    FTE_IOEX_init(pObj);
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_IOEX_detach
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_IOEX_STATUS_PTR  pStatus;

    ASSERT(pObj != NULL);

    pStatus = (FTE_IOEX_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
    
    return  FTE_RET_OK;
}

FTE_UINT32     FTE_IOEX_get
(
    FTE_OBJECT_PTR  pObject, 
    FTE_UINT32      ulIndex, 
    FTE_VALUE_PTR   pValue
)
{
    ASSERT ((pObject != NULL) && (pValue != NULL));
    
    if (ulIndex < FTE_IOEX_DI_MAX)
    {
        return  FTE_VALUE_setDIO(pValue, ((FTE_IOEX_STATUS_PTR)pObject->pStatus)->pDI[ulIndex].bValue);
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_IOEX_init
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_IOEX_loadExtConfig(pObj);
    
    FTE_TASK_create(FTE_TASK_IOEX, (FTE_UINT32)pObj, NULL);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_IOEX_reset(void)
{
    if (FTE_OBJ_get(FTE_OBJ_TYPE_IOEX_RESET) != 0)
    {
        FTE_DO_setValue(FTE_OBJ_TYPE_IOEX_RESET, TRUE);
        _time_delay(100);
        FTE_DO_setValue(FTE_OBJ_TYPE_IOEX_RESET, FALSE);
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_IOEX_initDefaultExtConfig
(
    FTE_IOEX_EXT_CONFIG_PTR pConfig
)
{
    ASSERT(pConfig != NULL);
    
    pConfig->ulLoopPeriod = FTE_IOEX_DEFAULT_LOOP_PERIOD;
    pConfig->ulUpdatePeriod = FTE_IOEX_DEFAULT_UPDATE_PERIOD;
    pConfig->ulRequestTimeout= FTE_IOEX_DEFAULT_REQUEST_TIMEOUT;
    pConfig->ulRetryCount = FTE_IOEX_DEFUALT_RETRY_COUNT;
    return  FTE_RET_OK;    
}
 
FTE_RET FTE_IOEX_loadExtConfig
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_IOEX_EXT_CONFIG    xConfig;
        
    if (FTE_CFG_IOEX_getExtConfig(&xConfig, sizeof(xConfig)) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    xGlobalConfig.ulLoopPeriod = xConfig.ulLoopPeriod;
    xGlobalConfig.ulUpdatePeriod = xConfig.ulUpdatePeriod;
    xGlobalConfig.ulRequestTimeout = xConfig.ulRequestTimeout;
    xGlobalConfig.ulRetryCount = xConfig.ulRetryCount;
    return  FTE_RET_OK;
}


FTE_RET FTE_IOEX_saveExtConfig
(
    FTE_OBJECT_PTR  pObj
)
{
    return  FTE_CFG_IOEX_setExtConfig(&xGlobalConfig, sizeof(xGlobalConfig));
}

FTE_RET   FTE_IOEX_sendRequest
(
    FTE_OBJECT_PTR  pObj
)
{
    FTE_UINT8  pSendBuff[FTE_IOEX_PACKET_LEN_MAX];
    FTE_UINT32 ulSendLen;
    FTE_UINT16 uiCRC;
    
    ulSendLen = 0;
    pSendBuff[ulSendLen++] = 0x00;
    pSendBuff[ulSendLen++] = 0x02;
    pSendBuff[ulSendLen++] = 0x20;
    pSendBuff[ulSendLen++] = 0x00;
    pSendBuff[ulSendLen++] = 0x00;
    pSendBuff[ulSendLen++] = FTE_IOEX_DI_MAX;
    uiCRC = FTE_CRC16(pSendBuff, ulSendLen);
    pSendBuff[ulSendLen++] = (uiCRC >> 8) & 0xFF;
    pSendBuff[ulSendLen++] = (uiCRC     ) & 0xFF;
                
    FTE_UCS_send(((FTE_IOEX_STATUS_PTR)pObj->pStatus)->xGUS.pUCS, pSendBuff, ulSendLen, 0);
    FTE_IOEX_DUMP_SEND_PKT(pSendBuff, ulSendLen);
    _time_get(&((FTE_IOEX_STATUS_PTR)pObj->pStatus)->xLastRequestTime);
    
    return  FTE_RET_OK;
}

void FTE_IOEX_task
(
    FTE_UINT32  ulParam
)
{
    FTE_OBJECT_PTR      pObj = (FTE_OBJECT_PTR)ulParam;
    FTE_UINT32             ulValidPktLen = FTE_IOEX_DI_MAX + 5;
    
    ASSERT(pObj != NULL);
    
    FTE_IOEX_STATUS_PTR pStatus;
    pStatus = (FTE_IOEX_STATUS_PTR)pObj->pStatus;
    
    pStatus->xTaskID = _task_get_id();
    pStatus->ulRetryCount = 0;
    _time_get(&pStatus->xLastUpdateTime);
    
    FTE_TIME_DELAY      xDelay;
    FTE_TIME_DELAY_init(&xDelay, FTE_IOEX_DEFAULT_LOOP_PERIOD);
    
    while(1)
    {
        TIME_STRUCT xCurrentTime;
        FTE_UINT8  pRecvBuff[FTE_IOEX_PACKET_LEN_MAX];
        FTE_UINT32 ulRecvLen;
        FTE_INT32   nElapsedUpdateTime = 0;
        int     i;

        _time_get(&xCurrentTime);
        
        FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastUpdateTime, &nElapsedUpdateTime);
        
        if (nElapsedUpdateTime >= xGlobalConfig.ulUpdatePeriod)
        {   
            if (pStatus->ulRetryCount == 0)
            {
                FTE_IOEX_sendRequest(pObj);
                pStatus->ulRetryCount++;
            }
            else if (pStatus->ulRetryCount >= xGlobalConfig.ulRetryCount)
            {
                FTE_IOEX_reset();
                pStatus->ulRetryCount = 0;
            }
            else  
            {
                FTE_INT32   nElapsedRequestTime = 0;
                
                FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastRequestTime, &nElapsedRequestTime);
                if( nElapsedRequestTime >= xGlobalConfig.ulRequestTimeout)
                {
                    FTE_IOEX_sendRequest(pObj);
                    pStatus->ulRetryCount++;
                }
            }

        }
        
        if (FTE_UCS_recvdLen(pStatus->xGUS.pUCS) >= ulValidPktLen)
        {
            ulRecvLen = FTE_UCS_recv(pStatus->xGUS.pUCS, pRecvBuff, sizeof(pRecvBuff));
            if (ulRecvLen >= ulValidPktLen)
            {
                FTE_INT32  nIndex = -1;
                FTE_IOEX_DUMP_RECV_PKT(pRecvBuff, ulRecvLen);
                
                for(i = 0 ; i < ulRecvLen - (ulValidPktLen - 1);i++)
                {
                    if ((pRecvBuff[i] == 0x00) && (pRecvBuff[i+1] == 0x02) && (pRecvBuff[i+2] == FTE_IOEX_DI_MAX))
                    {
                        nIndex = i;
                        break;
                    }
                }
                
                if (nIndex >= 0)
                {
                    FTE_UINT16 uiCRC = FTE_CRC16(&pRecvBuff[nIndex], ulValidPktLen - 2);
                    if ((pRecvBuff[nIndex + ulValidPktLen - 2] == ((uiCRC >> 8) & 0xFF)) && (pRecvBuff[nIndex + ulValidPktLen - 1] == ((uiCRC) & 0xFF)))
                    {
                        for(i = 0 ; i < FTE_IOEX_DI_MAX ; i++)
                        {
                            pStatus->pDI[i].bValue = (pRecvBuff[nIndex + i + 3] != 0);
                        }
                      
                        _time_get(&pStatus->xLastUpdateTime);
                        pStatus->ulRetryCount = 0; 
                    }
                }
            }
            
            ulRecvLen = FTE_UCS_clear(pStatus->xGUS.pUCS);
        }

        FTE_TIME_DELAY_waitingAndSetNext(&xDelay);
    }      
}


FTE_INT32 FTE_IOEX_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            FTE_OBJECT_PTR  pObj;
            
            printf("I/O Informations\n");
            printf("    %5s %8s\n", "INDEX", "STATUS");
                   
            pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MULTI_IOEX, FTE_OBJ_TYPE_MASK, 0, FALSE);
            if (pObj != NULL)
            {            
                FTE_INT32  i;
                for(i = 0 ; i < FTE_IOEX_DI_MAX ; i++)
                {
                    printf("    %5d %8s\n", i+1,((FTE_IOEX_STATUS_PTR)pObj->pStatus)->pDI[i].bValue?"ON":"OFF");
                }
                printf("\n");
                printf("Global Configurations\n");
                printf("      Loop Period : %d ms\n", xGlobalConfig.ulLoopPeriod);
                printf("    Update Period : %d ms\n", xGlobalConfig.ulUpdatePeriod);
                printf("      Retry Count : %d ms\n", xGlobalConfig.ulRetryCount);
                
            }
                
                
#if FTE_IOEX_DEBUG
            printf("\n");
            printf("Debug Informations\n");
            printf("    Trace : %s\n", (bTraceOn)?"ON":"OFF");
            printf("   Rx Pkt : %s\n", (bTraceOn && bTraceRxPkt)?"ON":"OFF");
            printf("   Tx Pkt : %s\n", (bTraceOn && bTraceTxPkt)?"ON":"OFF");
#endif            
        }
        break;
        
    case    2:
        {
            if (strcmp(pArgv[1], "save") == 0)
            {
                FTE_OBJECT_PTR  pObj;
                pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MULTI_IOEX, FTE_OBJ_TYPE_MASK, 0, FALSE);
                if (pObj != NULL)
                {
                    FTE_IOEX_saveExtConfig(pObj);
                }
            }
            else if (strcmp(pArgv[1], "reset") == 0)
            {
                FTE_IOEX_reset();
            }
                
        }
        break;
        
#if FTE_IOEX_DEBUG
    case    3:
        {
            if (strcmp(pArgv[1], "trace") == 0)
            {
                if (strcmp(pArgv[2], "on") == 0)
                {
                    bTraceOn = TRUE;
                }
                else if (strcmp(pArgv[2], "off") == 0)
                {
                    bTraceOn = FALSE;
                }
            }
        }
        break;
        
    case    4:
        {
            if (strcmp(pArgv[1], "trace") == 0)
            {
                if (strcmp(pArgv[2], "rx") == 0)
                {
                    if (strcmp(pArgv[3], "on") == 0)
                    {
                        bTraceRxPkt = TRUE;
                    }
                    else if (strcmp(pArgv[3], "off") == 0)
                    {
                        bTraceRxPkt = FALSE;
                    }
                }
                else if (strcmp(pArgv[2], "tx") == 0)
                {
                    if (strcmp(pArgv[3], "on") == 0)
                    {
                        bTraceTxPkt = TRUE;
                    }
                    else if (strcmp(pArgv[3], "off") == 0)
                    {
                        bTraceTxPkt = FALSE;
                    }
                }
            }
        }
        break;
#endif
        
    default:
        {
            bPrintUsage = TRUE;
        }
    }

    if (bPrintUsage)  
    {
        if (bShortHelp)  
        {
            printf("%s [<command>]\n", pArgv[0]);
        } 
        else  
        {
            printf("Usage: %s [<command>]\n",pArgv[0]);
            printf("    Show IOExpander status\n");
            printf("  Commands:\n");
#if FTE_IOEX_DEBUG
            printf("    trace [ on | off ]\n");
            printf("        Trace activation/inactivation\n");
            printf("    trace rx [ on | off ]\n");
            printf("        Receive packet trace activation/inactivation\n");
            printf("    trace tx [ on | off ]\n");
            printf("        Send packet trace activation/inactivation\n");
#endif
        }
    }
    
    return  nRet;
} 