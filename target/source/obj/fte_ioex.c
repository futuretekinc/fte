#include "fte_target.h"
#include "fte_crc.h"
#include "fte_cias.h"
#include "fte_config.h"
#include "fte_log.h" 
#include "fte_time.h"
#include "fte_task.h"

#define FTE_IOEX_MAX    1

static  FTE_IOEX   IOEX[FTE_IOEX_MAX];
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
 
_mqx_uint   FTE_IOEX_request(FTE_OBJECT_PTR pObj)
{
    return  MQX_OK;
}


_mqx_uint   FTE_IOEX_received(FTE_OBJECT_PTR pObj)
{
    return  MQX_OK;
}

_mqx_uint   FTE_IOEX_attach(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR              pStatus;
    FTE_UCS_PTR                     pUCS = NULL;
    
    ASSERT(pObj != NULL);

    pUCS = (FTE_UCS_PTR)FTE_UCS_get(((FTE_GUS_CONFIG_PTR)pObj->pConfig)->nUCSID);
    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;    
    if (pUCS == NULL)
    {
        goto error;
    }
        
    if (FTE_UCS_attach(pUCS, pObj->pConfig->xCommon.nID) != MQX_OK)
    {
        goto error;
    }
        
    pStatus->pUCS = pUCS;
    
    FTE_IOEX_init(pObj->pConfig->xCommon.nID);
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

_mqx_uint   FTE_IOEX_detach(FTE_OBJECT_PTR pObj)
{
    FTE_GUS_STATUS_PTR  pStatus;

    ASSERT(pObj != NULL);

    pStatus = (FTE_GUS_STATUS_PTR)pObj->pStatus;
    
    if (pStatus->pUCS != NULL)
    {
        FTE_UCS_detach(pStatus->pUCS, pObj->pConfig->xCommon.nID);
        pStatus->pUCS = NULL;
    }
    
    return  MQX_OK;
}

uint_32     FTE_IOEX_get(FTE_OBJECT_PTR pObject, uint_32 ulIndex, FTE_VALUE_PTR pValue)
{
    ASSERT ((pObject != NULL) && (pValue != NULL));
    
    if (ulIndex < FTE_IOEX_DI_MAX)
    {
        return  FTE_VALUE_setDIO(pValue, IOEX[0].pDI[ulIndex].bValue);
    }
    
    return  MQX_ERROR;
}

_mqx_uint FTE_IOEX_init(uint_32 ulObjectID)
{
    IOEX[0].bChanged    = FALSE; 
    IOEX[0].ulObjectID  = ulObjectID;
    IOEX[0].xTaskID     = _task_create(0, FTE_TASK_IOEX, 0);
    
    return  MQX_OK;
}

void FTE_IOEX_task(uint_32 ulID)
{
    FTE_OBJECT_PTR  pObj = NULL;    

    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
    while(!pObj)
    {
        _time_delay(1000);
        pObj = FTE_OBJ_get(IOEX[ulID].ulObjectID);
    }
    
    FTE_UCS_PTR pUCS = FTE_UCS_get(((FTE_GUS_CONFIG_PTR)pObj->pConfig)->nUCSID);
    
    while(1)
    {
      _time_delay(1000);
    }      
}


int_32 FTE_IOEX_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] )
{ 
    boolean                 bPrintUsage, bShortHelp = FALSE;
    int_32                  nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            int_32  i, j;
            printf("    %4s %8s\n", "INDEX", "STATUS");
                   
            for(i = 0 ; i < FTE_IOEX_MAX ; i++)
            {
                for(j = 0 ; j < FTE_IOEX_DI_MAX ; j++)
                {
                    printf("    %d-%02d %8s\n", i+1, j+1, IOEX[i].pDI[j].bValue?"ON":"OFF");
                }
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
            printf("%s <cmd>\n", pArgv[0]);
        } 
        else  
        {
            printf("Usage: %s <cmd>\n",pArgv[0]);
        }
    }
    
    return  nRet;
} 