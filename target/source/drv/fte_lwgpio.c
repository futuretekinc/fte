#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include <assert.h>
#include <lwgpio_kgpio.h>
#include <lwgpio.h>
#include "fte_type.h"
#include "fte_mem.h"
#include "fte_drv.h"
  
static FTE_LWGPIO_PTR   _pHead  = NULL;
static FTE_UINT32          _nGPIOs = 0;
 
FTE_RET FTE_LWGPIO_create
(
    FTE_LWGPIO_CONFIG_CONST_PTR     pConfig
)
{   
    FTE_LWGPIO_PTR  pLWGPIO;
    
    assert(pConfig != NULL);
    
    pLWGPIO = (FTE_LWGPIO_PTR)FTE_MEM_allocZero(sizeof(FTE_LWGPIO));
    if (pLWGPIO == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }

    pLWGPIO->pNext  = _pHead;
    pLWGPIO->pConfig= pConfig;
        
    _pHead = pLWGPIO;
    _nGPIOs++;
    
    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_attach
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_UINT32      nParent
)
{
    assert(pLWGPIO != NULL);
    
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    if (!lwgpio_init(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nLWGPIO, pLWGPIO->pConfig->nDIR, pLWGPIO->pConfig->nInit))
    {
        return  MQX_INVALID_DEVICE;
    }
        
    lwgpio_set_functionality(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nMUX);
    
    lwgpio_set_value(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nInit);
    
    pLWGPIO->nParent = nParent;
    
    return  MQX_OK;
}
    
FTE_RET   FTE_LWGPIO_detach
(
    FTE_LWGPIO_PTR  pLWGPIO
)
{
    if (pLWGPIO != NULL)
    {
        memset(pLWGPIO, 0, sizeof(FTE_LWGPIO));
        _nGPIOs--;
            
        return  MQX_OK;
    }
    
    return  MQX_ERROR;
}

FTE_UINT32 FTE_LWGPIO_count(void)
{
    return  _nGPIOs;
}

FTE_LWGPIO_PTR FTE_LWGPIO_get
(
    FTE_UINT32  nID
)
{
    FTE_LWGPIO_PTR pLWGPIO = _pHead;
    
    while(pLWGPIO != NULL)
    {
        if (pLWGPIO->pConfig->nID == nID)
        {
            return  pLWGPIO;
        }
        
        pLWGPIO = pLWGPIO->pNext;
    }
    
    return  NULL;
}
FTE_RET   FTE_LWGPIO_setValue
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL        value
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    if (value)
    {
        lwgpio_set_value(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nActive);
    }
    else
    {
        lwgpio_set_value(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nInactive);
    }

    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_getValue
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL_PTR    value
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    if (lwgpio_get_value(&pLWGPIO->xLWGPIO) == pLWGPIO->pConfig->nActive)
    {
        *value = TRUE;
    }
    else
    {
        *value = FALSE;
    }

    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_setDirection
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    LWGPIO_DIR      nValue
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    lwgpio_set_direction(&pLWGPIO->xLWGPIO, nValue);

    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_setPullUp
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL        bEnable
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    if (bEnable)
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);
    }
    else
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_DISABLE);
    }

    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_setPullDown
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL        bEnable
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    if (bEnable)
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_DOWN, LWGPIO_AVAL_ENABLE);
    }
    else
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_DOWN, LWGPIO_AVAL_DISABLE);
    }

    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_setISR
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    void            (*func)(FTE_VOID_PTR ), 
    FTE_VOID_PTR params
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    _int_install_isr(lwgpio_int_get_vector(&pLWGPIO->xLWGPIO), func, params);
        
    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_INT_init
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_UINT32      priority, 
    FTE_UINT32      subpriority, 
    FTE_BOOL        enable
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    _bsp_int_init(lwgpio_int_get_vector(&pLWGPIO->xLWGPIO), priority, subpriority, enable);
    
    return  MQX_OK;
}

FTE_RET   FTE_LWGPIO_INT_setPolarity
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_UINT32      polarity
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
   switch(polarity)
    {
    case    FTE_LWGPIO_INT_RISING:
        {
            if (pLWGPIO->pConfig->nActive == LWGPIO_VALUE_HIGH)
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_RISING);
            }
            else
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_FALLING);
            }
        }
        break;
        
    case    FTE_LWGPIO_INT_FALLING:
        {
            if (pLWGPIO->pConfig->nActive == LWGPIO_VALUE_HIGH)
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_FALLING);
            }
            else
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_RISING);
            }
        }
        break;
        
    case    FTE_LWGPIO_INT_LOW:
        {
            if (pLWGPIO->pConfig->nActive == LWGPIO_VALUE_HIGH)
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_LOW);
            }
            else
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_HIGH);
            }
        }
        break;
        
    case    FTE_LWGPIO_INT_HIGH:
        {
            if (pLWGPIO->pConfig->nActive == LWGPIO_VALUE_HIGH)
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_HIGH);
            }
            else
            {
                lwgpio_int_init(&pLWGPIO->xLWGPIO, LWGPIO_INT_MODE_LOW);
            }
        }
        break;
    default:
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

FTE_RET    FTE_LWGPIO_INT_setEnable
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL        enable
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }

    lwgpio_int_enable(&pLWGPIO->xLWGPIO, enable);

    return  MQX_OK;
}

FTE_RET    FTE_LWGPIO_INT_getFlag
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL_PTR    flag
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    *flag = lwgpio_int_get_flag(&pLWGPIO->xLWGPIO);

    return  MQX_OK;
}

FTE_RET    FTE_LWGPIO_INT_clrFlag
(
    FTE_LWGPIO_PTR  pLWGPIO
)
{
    assert(pLWGPIO != NULL);
    if (pLWGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    lwgpio_int_clear_flag(&pLWGPIO->xLWGPIO);

    return  MQX_OK;
}

FTE_INT32      FTE_LWGPIO_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    { 
        switch(nArgc)
        {
        case    1:
            { 
                /*      01234567890123456789012345678901234567890123456789 */
                printf("%8s %8s %8s %8s\n", "ID", "PORT", "DIR", "POLARITY");
                for(int xDevID = 0 ; xDevID < FTE_LWGPIO_count() ; xDevID++)
                {
                    FTE_LWGPIO_PTR pGPIO = FTE_LWGPIO_get(xDevID);
                    if (pGPIO != NULL)
                    {
                        printf("%8d PORT%c_%02d %8s %8s\n", xDevID, 
                               'A' +((pGPIO->pConfig->nLWGPIO >> GPIO_PIN_PORT_SHIFT) & GPIO_PIN_BIT_MASK),
                               pGPIO->pConfig->nLWGPIO & 0x1F,
                               (pGPIO->pConfig->nDIR == LWGPIO_DIR_INPUT)?"IN":"OUT",
                               (pGPIO->pConfig->nActive == LWGPIO_VALUE_HIGH)?"HI":"LO");
                    }
                }                       
            }
            break;
            
        default:
            {
                bPrintUsage = TRUE;
            }
        }
    }

   if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<xDevID>]n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<xDevID>]\n", pArgv[0]);
            printf("        xDevID - GPIO ID\n");
        }
    }

    return   xRet;
}

/******************************************************************************
 * Internal Functions
 ******************************************************************************/
