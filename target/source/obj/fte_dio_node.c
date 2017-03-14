#include "fte_target.h"
#include "fte_crc.h"
#include "fte_dio_node.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"
#include "fte_task.h"
#include "fte_debug.h"

#ifndef FTE_DIO_NODE_PACKET_LEN_MAX 
#define FTE_DIO_NODE_PACKET_LEN_MAX 256
#endif

#ifndef FTE_DIO_NODE_DEFAULT_LOOP_PERIOD
#define FTE_DIO_NODE_DEFAULT_LOOP_PERIOD    200// ms
#endif

#ifndef FTE_DIO_NODE_DEFAULT_UPDATE_PERIOD
#define FTE_DIO_NODE_DEFAULT_UPDATE_PERIOD      10000 // ms
#endif

#ifndef FTE_DIO_NODE_DEFAULT_REQUEST_TIMEOUT
#define FTE_DIO_NODE_DEFAULT_REQUEST_TIMEOUT    500
#endif

#ifndef FTE_DIO_NODE_DEFAULT_RETRY_COUNT
#define FTE_DIO_NODE_DEFUALT_RETRY_COUNT        3
#endif

#define FTE_DIO_NODE_DIRSET_ADDRESS     30000
#define FTE_DIO_NODE_DATA_ADDRESS       40000

static   uint_8 FTE_DIO_NODE_DI_COUNT;
static   uint_8 FTE_DIO_NODE_DO_COUNT;

/*#define FTE_DIO_NODE_DEBUG  0

#if  FTE_DIO_NODE_DEBUG
#define FTE_DIO_NODE_DUMP_SEND_PKT(x,y) { if (bTraceOn && bTraceTxPkt) DUMP("TX", (x), (y), 0); }
#define FTE_DIO_NODE_DUMP_RECV_PKT(x,y) { if (bTraceOn && bTraceRxPkt) DUMP("RX", (x), (y), 0); }
static  boolean     bTraceOn = FALSE;
static  boolean     bTraceRxPkt = TRUE;
static  boolean     bTraceTxPkt = TRUE;
#else
#define FTE_DIO_NODE_DUMP(x,y)   
#endif*/


typedef struct  FTE_DIO_NODE_DEVICE_STRUCT
{
    _task_id        xTaskID;
    FTE_OBJECT_PTR  pObj;
}   FTE_DIO_NODE_DEVICE, _PTR_ FTE_DIO_NODE_DEVICE_PTR;

static FTE_DIO_NODE_DEVICE pDevice;

const FTE_GUS_MODEL_INFO    FTE_DIO_NODE_GUSModelInfo = 
{
    .nModel         = FTE_GUS_MODEL_DIO_NODE,
    .pName          = "DIO_NODE",
    .xFlags         = FTE_GUS_FLAG_SHARED,
    .xUARTConfig    = 
    {
        .nBaudrate  =   FTE_DIO_NODE_DEFAULT_BAUDRATE,
        .nDataBits  =   FTE_DIO_NODE_DEFAULT_DATABITS,
        .nParity    =   FTE_DIO_NODE_DEFAULT_PARITY,
        .nStopBits  =   FTE_DIO_NODE_DEFAULT_STOPBITS,
        .bFullDuplex=   FTE_DIO_NODE_DEFAULT_FULL_DUPLEX
    },
    .nFieldCount    = FTE_DIO_NODE_FIELD_MAX,
    .pValueTypes    = FTE_DIO_NODE_valueTypes,
    .fCreate        = FTE_DIO_NODE_create,
    .fRun           = FTE_DIO_NODE_run,
    .fStop          = FTE_DIO_NODE_stop,
    .fAttach        = FTE_DIO_NODE_attach,
    .fDetach        = FTE_DIO_NODE_detach,
    .fGet           = FTE_DIO_NODE_get,
	.fSet           = FTE_DIO_NODE_set,
//    .fGetChildConfig= FTE_DIO_NODE_getChildConfig,
//    .fSetChildConfig= FTE_DIO_NODE_setChildConfig,
};

FTE_IFCE_CONFIG FTE_DIO_NODE_defaultconfig_Array[FTE_DIO_NODE_FIELD_MAX];
FTE_OBJECT_CONFIG_PTR FTE_DIO_NODE_defaultChildConfigs[FTE_DIO_NODE_FIELD_MAX];

FTE_DIO_NODE_CONFIG FTE_DIO_NODE_defaultConfig =
{
    .xGUS       =
    {
        .xCommon    =
        {
            .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DIO_NODE, 0),
            .pName      = "DIO_NODE",
            .xFlags     = FTE_OBJ_CONFIG_FLAG_DISABLE, 
            .ulChild    = 0,   //sizeof(FTE_DIO_NODE_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
            .pChild     = NULL, //(FTE_OBJECT_CONFIG_PTR _PTR_)FTE_DIO_NODE_defaultChildConfigs
        },
        .nModel     = FTE_GUS_MODEL_DIO_NODE,
        .nSensorID  = 0x01,
        .nUCSID     = FTE_DEV_UCS_1,
        .nInterval  = FTE_DIO_NODE_DEFAULT_UPDATE_INTERVAL
    }
};


static  FTE_DIO_NODE_EXT_CONFIG xGlobalConfig = 
{
    .ulLoopPeriod 		= FTE_DIO_NODE_DEFAULT_LOOP_PERIOD,
    .ulUpdatePeriod 	= FTE_DIO_NODE_DEFAULT_UPDATE_PERIOD,
	.ulRequestTimeout   = FTE_DIO_NODE_DEFAULT_REQUEST_TIMEOUT,
    .ulRetryCount       = FTE_DIO_NODE_DEFUALT_RETRY_COUNT
};

FTE_VALUE_TYPE  FTE_DIO_NODE_valueTypes[] =
{
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO,
    FTE_VALUE_TYPE_DIO
};

_mqx_uint   FTE_DIO_NODE_attach(FTE_OBJECT_PTR pObj)
{
    FTE_DIO_NODE_STATUS_PTR pStatus;
    FTE_UCS_PTR         pUCS = NULL;
    
    ASSERT(pObj != NULL);
	

    pUCS = (FTE_UCS_PTR)FTE_UCS_get(((FTE_DIO_NODE_CONFIG_PTR)pObj->pConfig)->xGUS.nUCSID);
    pStatus = (FTE_DIO_NODE_STATUS_PTR)pObj->pStatus;    
    if (pUCS == NULL)
    {
        goto error;
    }
        
    if (FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }
	
	FTE_UCS_setUART(pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
        
    pStatus->xGUS.pUCS = pUCS;
    
    pDevice.pObj = pObj;
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_DIO_NODE_detach(FTE_OBJECT_PTR pObj)
{
    FTE_DIO_NODE_STATUS_PTR  pStatus;

    ASSERT(pObj != NULL);

    pStatus = (FTE_DIO_NODE_STATUS_PTR)pObj->pStatus;
    FTE_DIO_NODE_stop(pObj);
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
	
    pDevice.pObj = NULL;
    return  MQX_OK;
}

uint_32     FTE_DIO_NODE_get(FTE_OBJECT_PTR pObject, uint_32 ulIndex, FTE_VALUE_PTR pValue)
{
    ASSERT ((pObject != NULL) && (pValue != NULL));
    
    if (ulIndex < FTE_DIO_NODE_DIO_MAX)
    {
        return  FTE_VALUE_setDIO(pValue, ((FTE_DIO_NODE_STATUS_PTR)pObject->pStatus)->pDIO[ulIndex].bValue);
    }
    
    return  MQX_ERROR;
}

uint_32     FTE_DIO_NODE_set(FTE_OBJECT_PTR pObject, uint_32 ulIndex, FTE_VALUE_PTR pValue)
{
    ASSERT ((pObject != NULL) && (pValue != NULL));
    
    if (ulIndex < FTE_DIO_NODE_DIO_MAX)
    {
	  if(MQX_OK == FTE_UCS_MODBUS_setReg(((FTE_DIO_NODE_STATUS_PTR)pObject->pStatus)->xGUS.pUCS, 1, ulIndex, pValue->xData.bValue, 500))
	  {
		((FTE_DIO_NODE_STATUS_PTR)pObject->pStatus)->pDIO[ulIndex].bValue = pValue->xData.bValue;
		return MQX_OK;
	  }  
    }
    
    return  MQX_ERROR;
}


_mqx_uint FTE_DIO_NODE_init(FTE_OBJECT_PTR pObj)
{
  
    FTE_DIO_NODE_loadExtConfig(pObj);
	FTE_DIO_NODE_reset();
	
    FTE_TASK_create(FTE_TASK_DIO_NODE, (FTE_UINT32)pObj, &(pDevice.xTaskID));
    return  MQX_OK;
}

_mqx_uint FTE_DIO_NODE_reset(void)
{
    if (FTE_OBJ_get(FTE_OBJ_TYPE_DIO_NODE_RESET) != 0)
    {
        FTE_DO_setValue(FTE_OBJ_TYPE_DIO_NODE_RESET, TRUE);
        _time_delay(200);
        FTE_DO_setValue(FTE_OBJ_TYPE_DIO_NODE_RESET, FALSE);
    }
    
    return  MQX_OK;
}

_mqx_uint FTE_DIO_NODE_initDefaultExtConfig(FTE_DIO_NODE_EXT_CONFIG_PTR pConfig)
{
    ASSERT(pConfig != NULL);
    
    pConfig->ulLoopPeriod = FTE_DIO_NODE_DEFAULT_LOOP_PERIOD;
    pConfig->ulUpdatePeriod = FTE_DIO_NODE_DEFAULT_UPDATE_PERIOD;
    pConfig->ulRequestTimeout= FTE_DIO_NODE_DEFAULT_REQUEST_TIMEOUT;
    pConfig->ulRetryCount = FTE_DIO_NODE_DEFUALT_RETRY_COUNT;
    return  MQX_OK;    
}
 
_mqx_uint FTE_DIO_NODE_loadExtConfig(FTE_OBJECT_PTR pObj)
{
    FTE_IOEX_EXT_CONFIG    xConfig;
        
    if (FTE_CFG_DIO_NODE_getExtConfig(&xConfig, sizeof(xConfig)) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    xGlobalConfig.ulLoopPeriod = xConfig.ulLoopPeriod;
    xGlobalConfig.ulUpdatePeriod = xConfig.ulUpdatePeriod;
    xGlobalConfig.ulRequestTimeout = xConfig.ulRequestTimeout;
    xGlobalConfig.ulRetryCount = xConfig.ulRetryCount;
    return  MQX_OK;
}


_mqx_uint FTE_DIO_NODE_saveExtConfig(FTE_OBJECT_PTR pObj)
{
    return  FTE_CFG_DIO_NODE_setExtConfig(&xGlobalConfig, sizeof(xGlobalConfig));
}


_mqx_uint   FTE_DIO_NODE_mb_get_reg(FTE_OBJECT_PTR pObj, uint_8 ucDeviceID, uint_16 usAddr, uint_16* gRegs, uint_8 nCount, uint_32 nTimeout)
{
  	FTE_UCS_PTR         pUCS = NULL;
	FTE_DIO_NODE_CONFIG_PTR pConfig;
	
	pConfig = (FTE_DIO_NODE_CONFIG_PTR)pObj->pConfig;
  	pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return FTE_RET_ERROR;
    }
	return FTE_UCS_MODBUS_getRegs(pUCS, ucDeviceID, usAddr, gRegs, nCount, nTimeout);  
}


_mqx_uint FTE_DIO_init_dirset(uint_16* pReg)
{
  
  FTE_UCS_PTR         pUCS = NULL;
  FTE_UINT32 xRet;
  
  FTE_DIO_NODE_reset();
  
  
  pUCS = (FTE_UCS_PTR)FTE_UCS_get(FTE_DIO_NODE_defaultConfig.xGUS.nUCSID);
  if (pUCS == NULL)
  {
        ERROR("UCS[%d] is not exist\n", FTE_DIO_NODE_defaultConfig.xGUS.nUCSID);
        return FTE_RET_ERROR;
  }
  
  xRet = FTE_UCS_attach(pUCS, FTE_DIO_NODE_defaultConfig.xGUS.xCommon.nID);
  if (xRet != FTE_RET_OK)
  {
        return  xRet;
  }
  FTE_UCS_setUART(pUCS, &(FTE_DIO_NODE_GUSModelInfo.xUARTConfig));
  
  while(1){
  	
	xRet = FTE_UCS_MODBUS_getRegs(pUCS, 1, FTE_DIO_NODE_DIRSET_ADDRESS, pReg, FTE_DIO_NODE_FIELD_MAX, 300);
  	if(xRet == FTE_RET_OK)
	  break;
	_time_delay(300);
  }
  
  FTE_UCS_detach(pUCS, FTE_DIO_NODE_defaultConfig.xGUS.xCommon.nID);
  return FTE_RET_OK;
  
}

FTE_RET FTE_DIO_NODE_create
(
    FTE_CHAR_PTR    pSlaveID,
    FTE_OBJECT_PTR _PTR_ ppObj
)
{
  
  
  
  int i;
  FTE_OBJECT_CONFIG_PTR   pConfig;
  FTE_OBJECT_CONFIG_PTR   pChildConfig[FTE_DIO_NODE_FIELD_MAX];
  FTE_UINT32              ulChildCount = 0 ;
  FTE_UINT32              ulSlaveID;
  FTE_OBJECT_PTR  pObj;
  FTE_UINT32 xRet;
  uint_16  pReg[FTE_DIO_NODE_FIELD_MAX];
  
  ASSERT((pSlaveID != NULL) && (ppObj != NULL));
  
  xRet = FTE_strToUINT32(pSlaveID, &ulSlaveID);
  if (xRet != FTE_RET_OK)
  {
        return  xRet;
  }
  if(pDevice.pObj != NULL)
  {
	if(((FTE_DIO_NODE_CONFIG_PTR)pDevice.pObj->pConfig)->xGUS.nSensorID== ulSlaveID)
	{
	  return FTE_RET_OK;
	}
  }
  FTE_DIO_NODE_defaultConfig.xGUS.nSensorID = ulSlaveID;
     
  xRet = FTE_DIO_init_dirset(pReg);
  
  if(xRet != FTE_RET_OK)
	return FTE_RET_ERROR;
  
  
  FTE_DIO_NODE_DI_COUNT = 0;
  FTE_DIO_NODE_DO_COUNT = 0;
  for( i = 0; i < FTE_DIO_NODE_FIELD_MAX; i++)
  {
	if(pReg[i] == 0)
	{
  		FTE_DIO_NODE_defaultconfig_Array[i].xCommon.nID = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0);
  		sprintf(FTE_DIO_NODE_defaultconfig_Array[i].xCommon.pName, "DI%d", FTE_DIO_NODE_DI_COUNT++);
	}
	else
	{
	  	FTE_DIO_NODE_defaultconfig_Array[i].xCommon.nID = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0);
  		sprintf(FTE_DIO_NODE_defaultconfig_Array[i].xCommon.pName, "DO%d", FTE_DIO_NODE_DO_COUNT++);
	}
  	FTE_DIO_NODE_defaultconfig_Array[i].xCommon.xFlags = FTE_OBJ_CONFIG_FLAG_DISABLE;
  	FTE_DIO_NODE_defaultconfig_Array[i].nDevID = MAKE_ID(FTE_OBJ_TYPE_MULTI_DIO_NODE, 0);
  	FTE_DIO_NODE_defaultconfig_Array[i].nRegID = i;
  	FTE_DIO_NODE_defaultconfig_Array[i].nInterval = FTE_DIO_NODE_DEFAULT_UPDATE_INTERVAL;
  }
  
  for( i = 0; i < FTE_DIO_NODE_FIELD_MAX; i++)
  {
	FTE_DIO_NODE_defaultChildConfigs[i] = (FTE_OBJECT_CONFIG_PTR)&FTE_DIO_NODE_defaultconfig_Array[i];
  }
  
  FTE_DIO_NODE_defaultConfig.xGUS.xCommon.ulChild = sizeof(FTE_DIO_NODE_defaultChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR);
  FTE_DIO_NODE_defaultConfig.xGUS.xCommon.pChild = (FTE_OBJECT_CONFIG_PTR _PTR_)FTE_DIO_NODE_defaultChildConfigs;
  
  xRet = FTE_CFG_OBJ_create((FTE_OBJECT_CONFIG_PTR)&FTE_DIO_NODE_defaultConfig, &pConfig, pChildConfig, FTE_DIO_NODE_FIELD_MAX, &ulChildCount);
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


FTE_RET FTE_DIO_NODE_update(FTE_OBJECT_PTR pObj)
{
    FTE_DIO_NODE_CONFIG_PTR pConfig;
	FTE_DIO_NODE_STATUS_PTR pStatus;
	uint_16  pReg_data[FTE_DIO_NODE_FIELD_MAX];
	FTE_UINT32 xRet = 0;
	
 	pStatus = (FTE_DIO_NODE_STATUS_PTR)pObj->pStatus;
	pConfig = (FTE_DIO_NODE_CONFIG_PTR)pObj->pConfig; 
  	
	
	for(int i = 0 ; i < FTE_DIO_NODE_FIELD_MAX ; i++)
	{
	  	if(pStatus->pDIO[i].bType == 0x0000)
		{
			xRet = FTE_DIO_NODE_mb_get_reg(pObj,pConfig->xGUS.nSensorID, FTE_DIO_NODE_DATA_ADDRESS + i, pReg_data, 1, 100);
			if(xRet != FTE_RET_OK)
			{
			  	return xRet;
			}
			else
			{
		  		FTE_VALUE_setDIO(&pStatus->xGUS.xCommon.pValue[i], ((pReg_data[0]== 0x0001) ? 1 : 0));
		  		pStatus->pDIO[i].bValue = pStatus->xGUS.xCommon.pValue[i].xData.bValue;
			}
		}
	}
	return FTE_RET_OK;
}



void FTE_DIO_NODE_task(uint_32 datas)
{

	FTE_OBJECT_PTR      pObj;
	uint_16  pReg_dirset[FTE_DIO_NODE_FIELD_MAX];
    int i;
	FTE_UCS_PTR pUCS;
	FTE_DIO_NODE_CONFIG_PTR pConfig;
	FTE_DIO_NODE_STATUS_PTR pStatus;
	FTE_UINT32 xRet = 0;
	TIME_STRUCT xCurrentTime;
    FTE_INT32   nElapsedUpdateTime = 0;

    
    
	pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MULTI_DIO_NODE, FTE_OBJ_TYPE_MASK, 0, FALSE);
	if (pObj == NULL)
    {
        ERROR("The object[%08x] is not exist\n", datas);
        return;
    }
	ASSERT((pObj->pConfig != NULL) && (pObj->pStatus != NULL));
	
	pStatus = (FTE_DIO_NODE_STATUS_PTR)pObj->pStatus;
	pConfig = (FTE_DIO_NODE_CONFIG_PTR)pObj->pConfig;
	
    pUCS = FTE_UCS_get(pConfig->xGUS.nUCSID);
    if (pUCS == NULL)
    {
        ERROR("UCS[%d] is not exist\n", pConfig->xGUS.nUCSID);
        return;
    }
        
    FTE_TIME_DELAY      xDelay;
    FTE_TIME_DELAY_init(&xDelay, FTE_DIO_NODE_DEFAULT_LOOP_PERIOD);
    FTE_UCS_setUART(pStatus->xGUS.pUCS, &pStatus->xGUS.pModelInfo->xUARTConfig);
	
	while(TRUE)
	{
		xRet = FTE_DIO_NODE_mb_get_reg(pObj, pConfig->xGUS.nSensorID, FTE_DIO_NODE_DIRSET_ADDRESS, pReg_dirset, FTE_DIO_NODE_FIELD_MAX, 100);
		if(xRet == FTE_RET_OK)
		  break;
	}
	for( i = 0; i < FTE_DIO_NODE_FIELD_MAX ; i++)
	{
	  pStatus->pDIO[i].bType = ((pReg_dirset[i] == 0x0001) ? 1 : 0);
	}
	
	pStatus->ulRetryCount = 0;
    _time_get(&pStatus->xLastUpdateTime);  
    while(1)
    {
	  		_time_get(&xCurrentTime);
			FTE_TIME_diffMilliseconds(&xCurrentTime, &pStatus->xLastUpdateTime, &nElapsedUpdateTime);
			if(nElapsedUpdateTime >= xGlobalConfig.ulUpdatePeriod)
			{
			  xRet = FTE_DIO_NODE_update(pObj);
			  if(xRet == FTE_RET_OK)
			  {
				FT_OBJ_STAT_incSucceed(&pStatus->xGUS.xCommon.xStatistics);
				_time_get(&pStatus->xLastUpdateTime);  
                pStatus->ulRetryCount=0;
			  }
			  else if (pStatus->ulRetryCount >= xGlobalConfig.ulRetryCount)
              {
               	FT_OBJ_STAT_incFailed(&pStatus->xGUS.xCommon.xStatistics);
				FTE_DIO_NODE_reset();
                pStatus->ulRetryCount=0;
              }
              else
              {
                    pStatus->ulRetryCount++;
              }
			}
			FTE_TIME_DELAY_waitingAndSetNext(&xDelay);
    }      
}

FTE_RET FTE_DIO_NODE_run
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
        
    return  FTE_DIO_NODE_init(pObj);
}

FTE_RET FTE_DIO_NODE_stop
(
    FTE_OBJECT_PTR  pObj
)
{
    ASSERT((pObj != NULL) && (pObj->pConfig != NULL));
    
	if (pDevice.xTaskID == 0)
    {
        return  FTE_RET_OK;
    }
	
    _task_destroy(pDevice.xTaskID);
    FTE_TASK_remove(pDevice.xTaskID);
	pDevice.xTaskID = 0;
    return  FTE_RET_OK;
}



int_32 FTE_DIO_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] )
{ 
    boolean                 bPrintUsage, bShortHelp = FALSE;
    int_32                  nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
	uint_32     nID;

	if (nArgc > 2)
        {
            if (!Shell_parse_hexnum(pArgv[2], &nID))
            {

            }

        }

    switch(nArgc)
    {
    case    1:
        {
            FTE_OBJECT_PTR  pObj;
            
            printf("I/O Informations\n");
            printf("    %5s %8s\n", "INDEX", "STATUS");
                   
            pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MULTI_DIO_NODE, FTE_OBJ_TYPE_MASK, 0, FALSE);
            if (pObj != NULL)
            {            
                int_32  i;
                for(i = 0 ; i < FTE_DIO_NODE_DIO_MAX ; i++)
                {
                    printf("    %5d %8s\n", i+1,((FTE_DIO_NODE_STATUS_PTR)pObj->pStatus)->pDIO[i].bValue?"ON":"OFF");
                }
                printf("\n");
                printf("Global Configurations\n");
                printf("      Loop Period : %d ms\n", xGlobalConfig.ulLoopPeriod);
                printf("    Update Period : %d ms\n", xGlobalConfig.ulUpdatePeriod);
                printf("      Retry Count : %d ms\n", xGlobalConfig.ulRetryCount);
                
            }
        break;
		}
        
    case    2:
	  {
            if (strcmp(pArgv[1], "reset") == 0)
            {
                FTE_DIO_NODE_reset();
            }
            break;    
      }
    case    3:

        break;
        
    case    4:
        {

			  FTE_OBJECT_PTR  pObj;
              FTE_VALUE       xValue;
			  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_MULTI_DIO_NODE, FTE_OBJ_TYPE_MASK, 0, FALSE);

              if(FTE_DIO_NODE_DI_COUNT > 0)
			  	nID = nID + 3;
	  
			  if (strcasecmp(pArgv[3], "on") == 0)
              {
					xValue.xData.bValue = 1;
              }
              else if (strcasecmp(pArgv[3], "off") == 0)
              {
				 	xValue.xData.bValue = 0;
              }
			  FTE_DIO_NODE_set(pObj, nID, &xValue);

        }
        break;

        
    default:
        {
            bPrintUsage = TRUE;
        }
    }

    if (bPrintUsage)  
    {

    }
    
    return  nRet;
}
























