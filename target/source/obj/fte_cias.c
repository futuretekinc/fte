#include "fte_target.h"
#include "fte_crc.h"
#include "fte_cias.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"
#include "fte_task.h"

#if FTE_CIAS_SIOUX_CU_SUPPORTED

static  FTE_CIAS_SIOUX_CU   SIOUX_CU[FTE_CIAS_SIOUX_CU_MAX];
FTE_VALUE_TYPE  FTE_CIAS_SIOUX_CU_valueTypes[] =
{    
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_HEX32,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG,
    FTE_VALUE_TYPE_ULONG
};
 
_mqx_uint   FTE_CIAS_SIOUX_CU_attach(FTE_OBJECT_PTR pObj)
{
    FTE_CIAS_SIOUX_CU_STATUS_PTR    pStatus;
    FTE_UCS_PTR                     pUCS = NULL;
    
    ASSERT(pObj != NULL);

    pUCS = (FTE_UCS_PTR)FTE_UCS_get(((FTE_CIAS_SIOUX_CU_CONFIG_PTR)pObj->pConfig)->xGUS.nUCSID);
    pStatus = (FTE_CIAS_SIOUX_CU_STATUS_PTR)pObj->pStatus;    
    if (pUCS == NULL)
    {
        goto error;
    }
        
    if (FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }
        
    pStatus->xGUS.pUCS = pUCS;
    
    FTE_CIAS_SIOUX_CU_init(pObj->pConfig->xCommon.nID);
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_CIAS_SIOUX_CU_detach(FTE_OBJECT_PTR pObj)
{
    FTE_CIAS_SIOUX_CU_STATUS_PTR  pStatus;

    ASSERT(pObj != NULL);

    for(int i = 0 ; i < FTE_CIAS_SIOUX_CU_MAX ; i++)
    {
        if (SIOUX_CU[i].ulObjectID == pObj->pConfig->xCommon.nID)
        {
            _task_destroy(SIOUX_CU[i].xTaskID);            
            FTE_TASK_remove(SIOUX_CU[i].xTaskID);
            
            SIOUX_CU[i].ulObjectID = 0;    
            SIOUX_CU[i].xTaskID = 0;
            break;
        }
    }

    pStatus = (FTE_CIAS_SIOUX_CU_STATUS_PTR)pObj->pStatus;    
    if (pStatus->xGUS.pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->xGUS.pUCS, pObj->pConfig->xCommon.nID);
        pStatus->xGUS.pUCS = NULL;
    }
    
    return  MQX_OK;
}

_mqx_uint   FTE_CIAS_SIOUX_CU_setConfig(FTE_OBJECT_PTR pObj, char_ptr pBuff)
{
    return  MQX_OK;
}

_mqx_uint   FTE_CIAS_SIOUX_CU_getConfig(FTE_OBJECT_PTR pObject, char_ptr pBuff, uint_32 ulBuffLen)
{
    return  MQX_OK;
}

_mqx_uint   FTE_CIAS_SIOUX_CU_request(FTE_OBJECT_PTR pObj)
{
    return  MQX_OK;
}


_mqx_uint   FTE_CIAS_SIOUX_CU_received(FTE_OBJECT_PTR pObj)
{
    return  MQX_OK;
}

_mqx_uint FTE_CIAS_SIOUX_CU_loadConfig(uint_32 ulIndex)
{
    FTE_CIAS_SIOUX_CU_EXT_CONFIG    xConfig;
    
    if (ulIndex >= FTE_CIAS_SIOUX_CU_MAX)
    {
        return  MQX_ERROR;
    }
    
    if (FTE_CFG_CIAS_getExtConfig(&xConfig, sizeof(xConfig)) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    SIOUX_CU[ulIndex].ulDistance    = xConfig.ulDistance;
    SIOUX_CU[ulIndex].ulSensorCount = xConfig.ulSensorCount;
    for(int i = 0 ; i < FTE_CIAS_SIOUX_CU_ZONE_MAX ; i++)
    {
        SIOUX_CU[ulIndex].pZones[i].nDeviceNumber = xConfig.pZones[i].nDeviceNumber;
        SIOUX_CU[ulIndex].pZones[i].bInOperation = xConfig.pZones[i].bActivation;
    }
    
    return  MQX_OK;
}

_mqx_uint FTE_CIAS_SIOUX_CU_saveConfig(uint_32 ulIndex)
{
    FTE_CIAS_SIOUX_CU_EXT_CONFIG    xConfig;
    
    if (ulIndex >= FTE_CIAS_SIOUX_CU_MAX)
    {
        return  MQX_ERROR;
    }
    
    xConfig.ulSensorCount   = SIOUX_CU[ulIndex].ulSensorCount;
    xConfig.ulDistance      = SIOUX_CU[ulIndex].ulDistance;
    for(int i = 0 ; i < FTE_CIAS_SIOUX_CU_ZONE_MAX ; i++)
    {
        xConfig.pZones[i].nDeviceNumber = SIOUX_CU[ulIndex].pZones[i].nDeviceNumber;
        xConfig.pZones[i].bActivation = SIOUX_CU[ulIndex].pZones[i].bInOperation;
    }
    
    return  FTE_CFG_CIAS_setExtConfig(&xConfig, sizeof(xConfig));
}

uint_32     FTE_CIAS_SIOUX_CU_get(FTE_OBJECT_PTR pObject, uint_32 ulIndex, FTE_VALUE_PTR pValue)
{
    ASSERT ((pObject != NULL) && (pValue != NULL));
    
    if (ulIndex < FTE_CIAS_SIOUX_CU_ALARM_MAX)
    {
        FTE_VALUE_setULONG(pValue, SIOUX_CU[0].pAlarms[ulIndex].ulValue);
    }
    else if (ulIndex < FTE_CIAS_SIOUX_CU_ALARM_MAX + FTE_CIAS_SIOUX_CU_ZONE_MAX / 2)
    {
        uint_32 ulSensor1 = 0;
        uint_32 ulSensor2 = 0;
        uint_32 ulValue = 0;
        uint_32 ulZone = (ulIndex - FTE_CIAS_SIOUX_CU_ALARM_MAX)*2;
        uint_32 ulDistance = SIOUX_CU[0].ulDistance;
        
        if (ulDistance < FTE_CIAS_SIOUX_CU_DEFAULT_DISTANCE)
        {
            ulDistance = FTE_CIAS_SIOUX_CU_DEFAULT_DISTANCE;
        }
        
        if (SIOUX_CU[0].pZones[ulZone].ulValue != 0)
        {
            ulSensor1 = (SIOUX_CU[0].pZones[ulZone].ulValue + (ulDistance / 2 - 1)) / ulDistance;
            ulSensor1+= 1;
            
            if (ulSensor1 > SIOUX_CU[0].ulSensorCount)
            {
                ulSensor1 = SIOUX_CU[0].ulSensorCount;
            }
        }
        else
        {
            ulSensor1 = 0;
        }
        
        if (SIOUX_CU[0].pZones[ulZone+1].ulValue != 0)
        {
            ulSensor2 = (SIOUX_CU[0].pZones[ulZone+1].ulValue + (ulDistance / 2 - 1)) / ulDistance;
            ulSensor2+= 1;
            
            if (ulSensor2 > SIOUX_CU[0].ulSensorCount)
            {
                ulSensor2 = SIOUX_CU[0].ulSensorCount;
            }
        }
        else
        {
            ulSensor2 = 0;
        }
        
        ulValue =   (ulSensor1 & 0xFF) | 
                   (( (SIOUX_CU[0].pZones[ulZone].ulAlarm) & 0xFF) << 8) |
                   ((ulSensor2 & 0xFF) << 16) | 
                   (( (SIOUX_CU[0].pZones[ulZone+1].ulAlarm) & 0xFF) << 24);
        FTE_VALUE_setULONG(pValue, ulValue);
    }
    
    return  MQX_OK;
}

void FTE_CIAS_SIOUX_CU_init(uint_32 ulObjectID)
{
    FTE_CIAS_SIOUX_CU_loadConfig(0);
    
    SIOUX_CU[0].ulObjectID = ulObjectID;    
    SIOUX_CU[0].xTaskID = _task_create(0, FTE_TASK_CIAS_SIOUX_CU, 0);
}

_mqx_uint FTE_CIAS_SIOUX_CU_initDefaultExtConfig(FTE_CIAS_SIOUX_CU_EXT_CONFIG_PTR pConfig)
{
    pConfig->ulDistance     = FTE_CIAS_SIOUX_CU_DEFAULT_DISTANCE;
    pConfig->ulSensorCount  = FTE_CIAS_SIOUX_CU_DEFAULT_SNESOR_COUNT;
    for(int i = 0 ; i < FTE_CIAS_SIOUX_CU_ZONE_MAX ; i++)
    {
        pConfig->pZones[i].nDeviceNumber = i+1;
        pConfig->pZones[i].bActivation = FALSE;
    }

    return  MQX_OK;
}
                                            
void FTE_CIAS_SIOUX_CU_task(uint_32 ulID)
{
    FTE_OBJECT_PTR  pObj = NULL;    

    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    while(!pObj)
    {
        _time_delay(1000);
        pObj = FTE_OBJ_get(SIOUX_CU[ulID].ulObjectID);
    }
    
    FTE_UCS_PTR pUCS = FTE_UCS_get(((FTE_CIAS_SIOUX_CU_CONFIG_PTR)pObj->pConfig)->xGUS.nUCSID);
    
   
    while(1)
    {
        uint_32 i, nZone;
        uint_32 ulAlarms[FTE_CIAS_SIOUX_CU_ALARM_MAX] = {0,};
        
        for(i = 0 ; i < FTE_CIAS_SIOUX_CU_ALARM_MAX ; i++)
        {
            ulAlarms[i] = SIOUX_CU[ulID].pAlarms[i].ulValue;
        }

        for(nZone = 1 ; nZone <= FTE_CIAS_SIOUX_CU_ZONE_MAX ; nZone++)
        {
            if (SIOUX_CU[ulID].pZones[nZone-1].bInOperation)
            {
                uint_8  pReqFrame[16];
                uint_32 ulReqLen;
                uint_8  pRecvBuff[20];
                uint_32 ulRecvLen;
                uint_16 ulCRC = 0xFFFF;

                ulReqLen = 0;
                pReqFrame[ulReqLen++] = 0x01;
                ulCRC = fte_crc_ccitt(ulCRC, &pReqFrame[ulReqLen-1], 1);
                
                if ((SIOUX_CU[ulID].pZones[nZone-1].nDeviceNumber == 0x01) || 
                    (SIOUX_CU[ulID].pZones[nZone-1].nDeviceNumber == 0x03) ||
                    (SIOUX_CU[ulID].pZones[nZone-1].nDeviceNumber == 0x10))
                {
                    pReqFrame[ulReqLen++] = 0x10;
                    pReqFrame[ulReqLen++] = SIOUX_CU[ulID].pZones[nZone-1].nDeviceNumber + 0x80;                    
                }
                else
                {
                    pReqFrame[ulReqLen++] = SIOUX_CU[ulID].pZones[nZone-1].nDeviceNumber;
                }
                ulCRC = fte_crc_ccitt(ulCRC, &SIOUX_CU[ulID].pZones[nZone-1].nDeviceNumber, 1);
                
                pReqFrame[ulReqLen++] = 0x50;
                ulCRC = fte_crc_ccitt(ulCRC, &pReqFrame[ulReqLen-1], 1);
                pReqFrame[ulReqLen++] = 0xA3;
                ulCRC = fte_crc_ccitt(ulCRC, &pReqFrame[ulReqLen-1], 1);
                pReqFrame[ulReqLen++] = 0x5B;
                ulCRC = fte_crc_ccitt(ulCRC, &pReqFrame[ulReqLen-1], 1);
                
                pReqFrame[ulReqLen++] = (ulCRC >> 8) & 0xFF;
                pReqFrame[ulReqLen++] = (ulCRC     ) & 0xFF;
                pReqFrame[ulReqLen++] = 0x03;
                
                FTE_UCS_clear(pUCS);
                ulRecvLen = FTE_UCS_sendAndRecv(pUCS, pReqFrame, ulReqLen, pRecvBuff, sizeof(pRecvBuff), 0, 50);
                if (ulRecvLen >= 12)
                {
                    uint_8  pRespFrame[13];
                    uint_32 ulRespLen = 0;
                    
                    for(int i = 0 ; i < ulRecvLen ; i++)
                    {
                        switch(ulRespLen)
                        {
                        case    0:
                            {
                                if (pRecvBuff[i] == 0x01)
                                {
                                    pRespFrame[ulRespLen++] = pRecvBuff[i];
                                }
                            }
                            break;
                            
                        case    12:    
                            {
                                if (pRecvBuff[i] == 0x03)
                                {
                                    pRespFrame[ulRespLen++] = pRecvBuff[i];
                                }
                            }
                            break;

                        default:
                            {
                                if (pRecvBuff[i] == 0x10) 
                                {
                                    if (((i + 1) < ulRecvLen) && 
                                        ((pRecvBuff[i+1] == 0x81) ||
                                         (pRecvBuff[i+1] == 0x83) ||
                                         (pRecvBuff[i+1] == 0x90)))
                                    {
                                        pRespFrame[ulRespLen++] = pRecvBuff[i+1] - 0x80;
                                        i++;
                                    }
                                }
                                else
                                {
                                    pRespFrame[ulRespLen++] = pRecvBuff[i];
                                }
                            }
                        }
                    }
                    if (ulRespLen == 12)
                    {
                        int i;
                        
                        ulCRC = fte_crc_ccitt(0xFFFF, pRespFrame, 9);
                        if ((((ulCRC >> 8) & 0xFF) == pRespFrame[9]) && ((ulCRC & 0xFF) == pRespFrame[10]))
                        {
                            uint_32     ulValue;
                            TIME_STRUCT xTime;
                            
                            ulValue = ((uint_16)pRespFrame[5] << 8) | ((uint_16)pRespFrame[6]);
                            _time_get(&xTime);
                            
                            if (SIOUX_CU[ulID].pZones[nZone-1].ulValue != ulValue)
                            {
                                SIOUX_CU[ulID].pZones[nZone-1].ulValue = ulValue;
                                SIOUX_CU[ulID].pZones[nZone-1].xVOT = xTime;
                            }                            
                            SIOUX_CU[ulID].pZones[nZone-1].ulAlarm = pRespFrame[3] & 0xBF;
                            
                            for(i = 0 ; i < FTE_CIAS_SIOUX_CU_ALARM_MAX ; i++)
                            {
                                if ((pRespFrame[3] >> i) & 0x01)
                                {
                                    ulAlarms[i] |= 1 << (nZone - 1);
                                }
                                else
                                {
                                    ulAlarms[i] &= ~(1 << (nZone - 1));
                                }
                            }                            
                        }                    
                    }
                }
            }
            else
            {
                SIOUX_CU[ulID].pZones[nZone-1].ulValue = 0;
                
                for(i = 0 ; i < FTE_CIAS_SIOUX_CU_ALARM_MAX ; i++)
                {
                    ulAlarms[i] &= ~(1 << (nZone - 1));
                }                            
            }
        }
        
        for(i = 0 ; i < FTE_CIAS_SIOUX_CU_ALARM_MAX ; i++)
        {
            if (SIOUX_CU[ulID].pAlarms[i].ulValue != ulAlarms[i])
            {
                SIOUX_CU[ulID].pAlarms[i].ulValue = ulAlarms[i];
                _time_get(&SIOUX_CU[ulID].pAlarms[i].xVOT);
            }
        }

        
      _time_delay(1000);
    }      
}


int_32 FTE_CIAS_SIOUX_CU_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] )
{ 
    boolean                 bPrintUsage, bShortHelp = FALSE;
    int_32                  nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            int_32  i;
            printf("       Number of Sensors : %d\n", SIOUX_CU[0].ulSensorCount);
            printf("Distance between sensors : %d\n", SIOUX_CU[0].ulDistance);
                   
            printf("%4s %4s %8s %8s\n", "Zone", "D/N", "STATUS", "PPM");
                   
            for(i = 0 ; i < FTE_CIAS_SIOUX_CU_ZONE_MAX ; i++)
            {
                printf("%4d %4d %8s %6d.%d\n", 
                       i+1, 
                       SIOUX_CU[0].pZones[i].nDeviceNumber,
                       (SIOUX_CU[0].pZones[i].bInOperation)?"RUN":"STOP",
                       SIOUX_CU[0].pZones[i].ulValue / 10, SIOUX_CU[0].pZones[i].ulValue % 10);
            }
        }
        break;
  
    case    2:
        {
            if (strcmp(pArgv[1], "save") == 0)
            {
                FTE_CIAS_SIOUX_CU_saveConfig(0);
            }
        }
        break;
        
    case    3:
        {
            uint_32  ulZone;

            if (strcmp(pArgv[2], "run") == 0)
            {
                if ((!Shell_parse_number(pArgv[1], &ulZone)) || (ulZone < 1 || FTE_CIAS_SIOUX_CU_ZONE_MAX < ulZone))
                {
                    printf ("Error in %s command, invalid zone!\n", pArgv[0]);
                    return SHELL_EXIT_ERROR;
                }

                if (SIOUX_CU[0].pZones[ulZone - 1].bInOperation == FALSE)
                {
                    SIOUX_CU[0].pZones[ulZone - 1].bInOperation = TRUE;
                    FTE_CIAS_SIOUX_CU_saveConfig(0);
                }
            }
            else if (strcmp(pArgv[2], "stop") == 0)
            {
                if ((!Shell_parse_number(pArgv[1], &ulZone)) || (ulZone < 1 || FTE_CIAS_SIOUX_CU_ZONE_MAX < ulZone))
                {
                    printf ("Error in %s command, invalid zone!\n", pArgv[0]);
                    return SHELL_EXIT_ERROR;
                }

                if (SIOUX_CU[0].pZones[ulZone - 1].bInOperation == TRUE)
                {
                    SIOUX_CU[0].pZones[ulZone - 1].bInOperation = FALSE;
                    FTE_CIAS_SIOUX_CU_saveConfig(0);
                }
            }
            else if (strcmp(pArgv[1], "sn") == 0)
            {
                uint_32  ulSN;
               
                if (!Shell_parse_number(pArgv[2], &ulSN) || (127 < ulSN))
                {
                    printf ("Error in %s command, invalid sensor number!\n", pArgv[0]);
                    return SHELL_EXIT_ERROR;
                }
               
                SIOUX_CU[0].ulSensorCount = ulSN;               
                FTE_CIAS_SIOUX_CU_saveConfig(0);
            }
            else if (strcmp(pArgv[1], "distance") == 0)
            {
                uint_32  ulDistance;
               
                if (!Shell_parse_number(pArgv[2], &ulDistance) || (ulDistance == 0) || (1000 < ulDistance))
                {
                    printf ("Error in %s command, invalid distance between sensors!\n", pArgv[0]);
                    return SHELL_EXIT_ERROR;
                }
               
                SIOUX_CU[0].ulDistance = ulDistance;
                FTE_CIAS_SIOUX_CU_saveConfig(0);
            }
        }
        break;
        
    case    4:
        {
            if (strcmp(pArgv[2], "dn") == 0)
            {
                uint_32  ulZone, ulDN;
               
                if ((!Shell_parse_number(pArgv[1], &ulZone)) || (ulZone < 1 || FTE_CIAS_SIOUX_CU_ZONE_MAX < ulZone))
                {
                    printf ("Error in %s command, invalid zone!\n", pArgv[0]);
                    return SHELL_EXIT_ERROR;
                }
               
                if (!Shell_parse_number(pArgv[3], &ulDN) || (127 < ulDN))
                {
                    printf ("Error in %s command, invalid device number!\n", pArgv[0]);
                    return SHELL_EXIT_ERROR;
                }
               
                SIOUX_CU[0].pZones[ulZone-1].nDeviceNumber = ulDN;               
                FTE_CIAS_SIOUX_CU_saveConfig(0);
            }
        }
        break;
        
    default:
        {
            bPrintUsage = TRUE;
        }
    }

    if (bPrintUsage)  
    {
        if (bShortHelp)  
        {
            printf("%s <commands>\n", pArgv[0]);
        } 
        else  
        {
            printf("Usage: %s [<zone>] <commands>\n",pArgv[0]);
            printf("  Command:\n");
            printf("    <zone> run\n");
            printf("        Start monitoring for zone.\n"); 
            printf("    <zone> stop\n");
            printf("        Stop monitoring for zone.\n"); 
            printf("    <zone> dn <n>\n");
            printf("        Set device number for zone.\n"); 
            printf("    sn <n>\n");
            printf("        Set sensor number for all zones.\n"); 
            printf("    distance <n>\n");
            printf("        Distance between sensors.\n"); 
            printf("    save\n");
            printf("        Save configuration.\n"); 
            printf("  Parameter:\n");
            printf("    zone = target zone\n");
            printf("    n    = Number of sensor for zone\n");
            printf("         default : 7\n");            
        }
    }
    
    return  nRet;
} 
#endif