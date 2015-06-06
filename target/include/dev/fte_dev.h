#ifndef __FTE_DEVICES_H__
#define __FTE_DEVICES_H__

typedef struct _FTE_DEVICE
{
    char_ptr    pName;
    _mqx_uint   (*f_set)(struct _FTE_DEVICE *pDevice, char_ptr pJSON);
    _mqx_uint   (*f_get)(struct _FTE_DEVICE *pDevice, char_ptr pBuff, uint_32 ulBuffLen);    
}   FTE_DEVICE, _PTR_ FTE_DEVICE_PTR;

_mqx_uint       FTE_DEVICE_attach(FTE_DEVICE_PTR pDevice);
uint_32         FTE_DEVICE_count(void);
FTE_DEVICE_PTR  FTE_DEVICE_getAt(uint_32 ulIndex);
_mqx_uint       FTE_DEVICE_set(FTE_DEVICE_PTR pDevice, char_ptr pJSON);
_mqx_uint       FTE_DEVICE_get(FTE_DEVICE_PTR pDevice, char_ptr pBuff, uint_32 ulBuffLen);

#endif