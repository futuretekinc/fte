#ifndef __FTE_DEVICES_H__
#define __FTE_DEVICES_H__

typedef struct _FTE_DEVICE
{
    FTE_CHAR_PTR    pName;
    FTE_RET   (*f_set)(struct _FTE_DEVICE *pDevice, FTE_CHAR_PTR pJSON);
    FTE_RET   (*f_get)(struct _FTE_DEVICE *pDevice, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);    
}   FTE_DEVICE, _PTR_ FTE_DEVICE_PTR;

FTE_RET         FTE_DEVICE_attach(FTE_DEVICE_PTR pDevice);
FTE_UINT32      FTE_DEVICE_count(void);
FTE_DEVICE_PTR  FTE_DEVICE_getAt(FTE_UINT32 ulIndex);
FTE_RET         FTE_DEVICE_set(FTE_DEVICE_PTR pDevice, FTE_CHAR_PTR pJSON);
FTE_RET         FTE_DEVICE_get(FTE_DEVICE_PTR pDevice, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);

#endif