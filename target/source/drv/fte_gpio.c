#include "fte_target.h"

static FTE_GPIO_PTR  _pHead     = NULL;
static uint_32       _nGPIOs    = 0;
 
_mqx_uint FTE_GPIO_create(FTE_GPIO_CONFIG_PTR pConfig)
{
    FTE_GPIO_PTR    pGPIO;
    
    pGPIO = (FTE_GPIO_PTR)FTE_MEM_allocZero(sizeof(FTE_GPIO));
    if (pGPIO == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }
    
    pGPIO->pNext    = _pHead;
    pGPIO->pConfig  = pConfig;   

    _pHead = pGPIO;
    _nGPIOs++;
    
    return  MQX_OK;
}

_mqx_uint   FTE_GPIO_attach(FTE_GPIO_PTR pGPIO, uint_32 nParent)
{
    assert(pGPIO != NULL);
    if (pGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            FTE_LWGPIO_PTR  pLWGPIO = FTE_LWGPIO_get(pGPIO->pConfig->nDevID);
            if (pLWGPIO == NULL)
            {
                goto error;
            }
            
            if (FTE_LWGPIO_attach(pLWGPIO, pGPIO->pConfig->nID) != MQX_OK)
            {
                goto error;
            }
            
            switch (pGPIO->pConfig->nDIR)
            {
            case FTE_GPIO_DIR_OUTPUT: FTE_LWGPIO_setDirection(pLWGPIO, LWGPIO_DIR_OUTPUT); break;
            case FTE_GPIO_DIR_INPUT: FTE_LWGPIO_setDirection(pLWGPIO, LWGPIO_DIR_INPUT); break;
            default: FTE_LWGPIO_setDirection(pLWGPIO, LWGPIO_DIR_NOCHANGE); break;
            }

            if (pGPIO->pConfig->nDIR != FTE_GPIO_DIR_INPUT)
            {
            }
            else
            {
                switch(pGPIO->pConfig->nInit)
                {
                case    FTE_GPIO_VALUE_LOW: FTE_LWGPIO_setValue(pLWGPIO, FALSE); break;
                case    FTE_GPIO_VALUE_HIGH: FTE_LWGPIO_setValue(pLWGPIO, TRUE); break;            
                }
            }
            pGPIO->pPort = (FTE_DRIVER_PTR)pLWGPIO;
        }
        break;
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            FTE_MCP23S08_GPIO_PTR pMCP23S08_GPIO;
            
            pMCP23S08_GPIO = FTE_MCP23S08_GPIO_get(pGPIO->pConfig->nDevID);
            if (pMCP23S08_GPIO == NULL)
            {
                goto error;
            }
            
            if (FTE_MCP23S08_GPIO_attach(pMCP23S08_GPIO, pGPIO->pConfig->nID) != MQX_OK)
            {
                goto error;
            }
            
            FTE_MCP23S08_GPIO_setDIR(pMCP23S08_GPIO, pGPIO->pConfig->nDIR);
            
            pGPIO->pPort = (FTE_DRIVER_PTR)pMCP23S08_GPIO;
        }
        break;
#endif
    default:
        goto error;
    }
    
    pGPIO->nParent = nParent;

    return  MQX_OK;
error:
    
    return  MQX_ERROR;
}
    
_mqx_uint   FTE_GPIO_detach(FTE_GPIO_PTR pGPIO)
{
    assert(pGPIO != NULL);
    if (pGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            FTE_LWGPIO_PTR pLWGPIO = FTE_LWGPIO_get(pGPIO->pConfig->nDevID);
            if (pLWGPIO == NULL)
            {
                goto error;
            }
            FTE_LWGPIO_attach(pLWGPIO, pGPIO->pConfig->nID);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            FTE_MCP23S08_GPIO_PTR    pMCP23S08_GPIO;
            
            pMCP23S08_GPIO = FTE_MCP23S08_GPIO_get(pGPIO->pConfig->nDevID);
            if (pMCP23S08_GPIO == NULL)
            {
                goto    error;
            }
            
            FTE_MCP23S08_GPIO_attach(pMCP23S08_GPIO, pGPIO->pConfig->nID);
        }
        break;
#endif
        
    default:
        goto error;
    }

    pGPIO->nParent = 0;
    _nGPIOs--;
            

    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

uint_32 FTE_GPIO_count(void)
{
    return  _nGPIOs;
}

FTE_GPIO_PTR FTE_GPIO_get(uint_32 nID)
{
    FTE_GPIO_PTR    pGPIO;
    
    pGPIO = _pHead;
    while(pGPIO != NULL)
    {
        if (pGPIO->pConfig->nID == nID)
        {
            return  pGPIO;
        }
        
        pGPIO = pGPIO->pNext;
    }    
    
    return  NULL;
}

_mqx_uint   FTE_GPIO_setValue(FTE_GPIO_PTR pGPIO, boolean nValue)
{
    assert(pGPIO != NULL);
    if (pGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            return  FTE_LWGPIO_setValue((FTE_LWGPIO_PTR)pGPIO->pPort, nValue);
        }
        break;
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            if (pGPIO->pConfig->nActive == FTE_GPIO_VALUE_HIGH)
            {
                return  FTE_MCP23S08_GPIO_setValue((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, nValue);
            }
            else
            {
                return  FTE_MCP23S08_GPIO_setValue((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, !nValue);
            }
        }
        break;
#endif
    }

    return  MQX_ERROR;
    
}

_mqx_uint   FTE_GPIO_getValue(FTE_GPIO_PTR pGPIO, boolean *pValue)
{
    assert(pGPIO != NULL);
    if (pGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            return  FTE_LWGPIO_getValue((FTE_LWGPIO_PTR)pGPIO->pPort, pValue);
        }
        break;
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            uint_32 nValue;
            if (FTE_MCP23S08_GPIO_getValue((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, &nValue) != MQX_OK)
            {
                return  MQX_ERROR;
            }
            
            if (pGPIO->pConfig->nActive == FTE_GPIO_VALUE_HIGH)
            {
                *pValue = (nValue != 0);
            }
            else
            {
                *pValue = (nValue == 0);
            }
            
            return  MQX_OK;
            
        }
#endif
        break;
    }

    return  MQX_ERROR;
}

_mqx_uint   FTE_GPIO_setDir(FTE_GPIO_PTR pGPIO, FTE_GPIO_DIR nValue)
{
    assert(pGPIO != NULL);
    if (pGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            switch(nValue)
            {
            case    FTE_GPIO_DIR_INPUT:
                return  FTE_LWGPIO_setDirection((FTE_LWGPIO_PTR)pGPIO->pPort, LWGPIO_DIR_INPUT);
            case    FTE_GPIO_DIR_OUTPUT:
                return  FTE_LWGPIO_setDirection((FTE_LWGPIO_PTR)pGPIO->pPort, LWGPIO_DIR_OUTPUT);
            }        
            
            return  FTE_LWGPIO_setDirection((FTE_LWGPIO_PTR)pGPIO->pPort, LWGPIO_DIR_NOCHANGE);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            return  FTE_MCP23S08_GPIO_setDIR((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, nValue);
        }
        break;
#endif
    }

    return  MQX_ERROR;
}

_mqx_uint   FTE_GPIO_setISR(FTE_GPIO_PTR pGPIO, void (*f_isr)(void *), void *pParams)
{
    assert(pGPIO != NULL);
    if (pGPIO == NULL)
    {
        return  MQX_INVALID_COMPONENT_HANDLE;
    }
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {    
            return  FTE_LWGPIO_setISR((FTE_LWGPIO_PTR)pGPIO->pPort, f_isr, pParams);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            return  FTE_MCP23S08_GPIO_ISR_set((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, f_isr, NULL);
        }
        break;
#endif
    }

    return  MQX_ERROR;
}

_mqx_uint   FTE_GPIO_INT_init(FTE_GPIO_PTR pGPIO, uint_32 nPriority, uint_32 nSubpriority, boolean nEnable)
{
     ASSERT(pGPIO != NULL);
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            return  FTE_LWGPIO_INT_init((FTE_LWGPIO_PTR)pGPIO->pPort, nPriority, nSubpriority, nEnable);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            return  MQX_OK;
        }
        break;
#endif
    }

    return  MQX_ERROR;
}

_mqx_uint   FTE_GPIO_INT_setPolarity(FTE_GPIO_PTR pGPIO, uint_32 nPolarity)
{    
     ASSERT(pGPIO != NULL);
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            return  FTE_LWGPIO_INT_setPolarity((FTE_LWGPIO_PTR)pGPIO->pPort, nPolarity);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            return  FTE_MCP23S08_GPIO_INT_setPolarity((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, nPolarity);
        }
        break;
#endif
    }

    return  MQX_ERROR;
}

_mqx_uint    FTE_GPIO_INT_setEnable(FTE_GPIO_PTR pGPIO, boolean nEnable)
{
     ASSERT(pGPIO != NULL);
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            return  FTE_LWGPIO_INT_setEnable((FTE_LWGPIO_PTR)pGPIO->pPort, nEnable);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            return  FTE_MCP23S08_GPIO_INT_enable((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, nEnable);
        }
        break;
#endif
    }

    return  MQX_ERROR;
}

_mqx_uint    FTE_GPIO_INT_getFlag(FTE_GPIO_PTR pGPIO, boolean *pFlag)
{
     ASSERT(pGPIO != NULL);
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {
            return  FTE_LWGPIO_INT_getFlag((FTE_LWGPIO_PTR)pGPIO->pPort, pFlag);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            return  FTE_MCP23S08_GPIO_INT_getFlag((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort, pFlag);
        }
        break;
#endif
    }

    return  MQX_ERROR;
}

_mqx_uint    FTE_GPIO_INT_clrFlag(FTE_GPIO_PTR pGPIO)
{
     ASSERT(pGPIO != NULL);
    
    switch(FTE_DEV_TYPE(pGPIO->pConfig->nDevID))
    {
    case    FTE_DEV_TYPE_LWGPIO:
        {    
            return  FTE_LWGPIO_INT_clrFlag((FTE_LWGPIO_PTR)pGPIO->pPort);
        }
        break;
        
#if FTE_MCP23S08_SUPPORTED
    case    FTE_DEV_TYPE_MCP23S08:
        {
            return  FTE_MCP23S08_GPIO_INT_clrFlag((FTE_MCP23S08_GPIO_PTR)pGPIO->pPort);
        }
        break;
#endif
    }

    return  MQX_ERROR;
}

int_32      FTE_GPIO_SHELL_cmd(int_32 argc, char_ptr argv[] )
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    { 
        switch(argc)
        {
        case    1:
            { 
            }
            break;
            
        default:
            {
                print_usage = TRUE;
            }
        }
    }

   if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<xDevID>]n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<xDevID>]\n", argv[0]);
            printf("        xDevID - GPIO ID\n");
        }
    }

    return   return_code;
}

/******************************************************************************
 * Internal Functions
 ******************************************************************************/
