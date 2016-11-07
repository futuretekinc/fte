#ifndef __FTE_LED_H__
#define __FTE_LED_H__

/*****************************************************************************
 * Digital Ouptut Object Structure Description
 *****************************************************************************/

#define FTE_LED_STATE_OFF       0
#define FTE_LED_STATE_ON        1
#define FTE_LED_STATE_BLINK     2
     
#define FTE_LED_NORMAL_OPEN     LWGPIO_VALUE_LOW
#define FTE_LED_NORMAL_CLOSE    LWGPIO_VALUE_HIGH

typedef struct _FTE_LED_CONFIG_PTR
{
    FTE_COMMON_CONFIG   xCommon;
    FTE_UINT32             nGPIO;
    FTE_UINT32             nInit;
}   FTE_LED_CONFIG, _PTR_ FTE_LED_CONFIG_PTR;

typedef struct  _FTE_LED_STATUS_STRUCT
{
    FTE_OBJECT_STATUS   xCommon;
    FTE_GPIO_PTR        pGPIO;
}   FTE_LED_STATUS, _PTR_ FTE_LED_STATUS_PTR;

FTE_RET     FTE_LED_attach(FTE_OBJECT_PTR pObj, FTE_VOID_PTR pOpts);
FTE_RET     FTE_LED_detach(FTE_OBJECT_PTR pObj);
FTE_UINT32  FTE_LED_printValue(FTE_OBJECT_PTR pObj, FTE_CHAR_PTR pBuff, FTE_UINT32 nLen);

FTE_UINT32  FTE_LED_count(void);
pointer     FTE_LED_get(FTE_OBJECT_ID nID);

FTE_RET     FTE_LED_setValue(FTE_OBJECT_ID  nID, FTE_UINT32 nValue);
FTE_BOOL    FTE_LED_isActive(FTE_OBJECT_ID  nID);

#endif
