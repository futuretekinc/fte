#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include <lwgpio_kgpio.h>
#include <lwgpio.h>
#include "fte_type.h"
#include "fte_sys.h"
#include "fte_assert.h"
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
    
    ASSERT(pConfig != NULL);
    
    pLWGPIO = (FTE_LWGPIO_PTR)FTE_MEM_allocZero(sizeof(FTE_LWGPIO));
    if (pLWGPIO == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }

    pLWGPIO->pNext  = _pHead;
    pLWGPIO->pConfig= pConfig;
        
    _pHead = pLWGPIO;
    _nGPIOs++;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_attach
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_UINT32      nParent
)
{
    ASSERT(pLWGPIO != NULL);
        
    if (!lwgpio_init(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nLWGPIO, pLWGPIO->pConfig->nDIR, pLWGPIO->pConfig->nInit))
    {
        return  FTE_RET_INITIALIZE_FAILED;
    }
        
    lwgpio_set_functionality(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nMUX);
    
    lwgpio_set_value(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nInit);
    
    pLWGPIO->nParent = nParent;
    
    return  FTE_RET_OK;
}
    
FTE_RET   FTE_LWGPIO_detach
(
    FTE_LWGPIO_PTR  pLWGPIO
)
{
    /*if (pLWGPIO != NULL)
    {
        memset(pLWGPIO, 0, sizeof(FTE_LWGPIO));
        _nGPIOs--;
            
        return  FTE_RET_OK;
    }*/
    if (pLWGPIO->nParent != 0)
    {
        //memset(pLWGPIO, 0, sizeof(FTE_LWGPIO));
        //_nGPIOs--;
        pLWGPIO->nParent = 0;    
        return  FTE_RET_OK;
    }
    
    return  FTE_RET_ERROR;
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
    ASSERT(pLWGPIO != NULL);
    
    if (value)
    {
        lwgpio_set_value(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nActive);
    }
    else
    {
        lwgpio_set_value(&pLWGPIO->xLWGPIO, pLWGPIO->pConfig->nInactive);
    }

    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_getValue
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL_PTR    value
)
{
    ASSERT(pLWGPIO != NULL);
    
    if (lwgpio_get_value(&pLWGPIO->xLWGPIO) == pLWGPIO->pConfig->nActive)
    {
        *value = TRUE;
    }
    else
    {
        *value = FALSE;
    }

    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_setDirection
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    LWGPIO_DIR      nValue
)
{
    ASSERT(pLWGPIO != NULL);
    
    lwgpio_set_direction(&pLWGPIO->xLWGPIO, nValue);

    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_setPullUp
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL        bEnable
)
{
    ASSERT(pLWGPIO != NULL);
    
    if (bEnable)
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);
    }
    else
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_DISABLE);
    }

    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_setPullDown
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL        bEnable
)
{
    ASSERT(pLWGPIO != NULL);
    
    if (bEnable)
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_DOWN, LWGPIO_AVAL_ENABLE);
    }
    else
    {
        lwgpio_set_attribute(&pLWGPIO->xLWGPIO, LWGPIO_ATTR_PULL_DOWN, LWGPIO_AVAL_DISABLE);
    }

    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_setISR
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    void            (*func)(FTE_VOID_PTR ), 
    FTE_VOID_PTR params
)
{
    ASSERT(pLWGPIO != NULL);
    
    _int_install_isr(lwgpio_int_get_vector(&pLWGPIO->xLWGPIO), func, params);
        
    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_INT_init
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_UINT32      priority, 
    FTE_UINT32      subpriority, 
    FTE_BOOL        enable
)
{
    ASSERT(pLWGPIO != NULL);
    
    _bsp_int_init(lwgpio_int_get_vector(&pLWGPIO->xLWGPIO), priority, subpriority, enable);
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_LWGPIO_INT_setPolarity
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_UINT32      polarity
)
{
    ASSERT(pLWGPIO != NULL);
    
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
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET    FTE_LWGPIO_INT_setEnable
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL        enable
)
{
    ASSERT(pLWGPIO != NULL);

    lwgpio_int_enable(&pLWGPIO->xLWGPIO, enable);

    return  FTE_RET_OK;
}

FTE_RET    FTE_LWGPIO_INT_getFlag
(
    FTE_LWGPIO_PTR  pLWGPIO, 
    FTE_BOOL_PTR    flag
)
{
    ASSERT(pLWGPIO != NULL);
    
    *flag = lwgpio_int_get_flag(&pLWGPIO->xLWGPIO);

    return  FTE_RET_OK;
}

FTE_RET    FTE_LWGPIO_INT_clrFlag
(
    FTE_LWGPIO_PTR  pLWGPIO
)
{
    ASSERT(pLWGPIO != NULL);
    
    lwgpio_int_clear_flag(&pLWGPIO->xLWGPIO);

    return  FTE_RET_OK;
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
            printf ("%s [<xDevID>]\n", pArgv[0]);
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
