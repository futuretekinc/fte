#include "fte_target.h"
#include "fte_dev.h"

static  FTE_LIST            _xDeviceList    = { 0, NULL, NULL };

FTE_RET FTE_DEVICE_attach
(
    FTE_DEVICE_PTR  pDevice
)
{
    FTE_LIST_pushBack(&_xDeviceList, pDevice);
    
    return  MQX_OK;
}

FTE_UINT32      FTE_DEVICE_count(void)
{
    return  FTE_LIST_count(&_xDeviceList);
}

FTE_DEVICE_PTR  FTE_DEVICE_getAt
(
    FTE_UINT32  ulIndex
)
{
    return  (FTE_DEVICE_PTR)FTE_LIST_getAt(&_xDeviceList, ulIndex);
}

FTE_RET FTE_DEVICE_set
(
    FTE_DEVICE_PTR  pDevice, 
    FTE_CHAR_PTR    pJSON
)
{
    ASSERT( pDevice != NULL);
    
    if (pDevice->f_set != NULL)
    {
        return  pDevice->f_set(pDevice, pJSON);
    }
    
    return  MQX_OK;
}

FTE_RET FTE_DEVICE_get
(
    FTE_DEVICE_PTR  pDevice, 
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      ulBuffLen
)
{
    ASSERT( pDevice != NULL);
    
    if (pDevice->f_get != NULL)
    {
        return  pDevice->f_get(pDevice, pBuff, ulBuffLen);
    }
    
    return  MQX_OK;
}

