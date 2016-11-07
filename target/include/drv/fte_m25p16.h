#ifndef __FTE_M25P16_H__
#define __FTE_M25P16_H__

typedef struct _FTE_M25P16_CONFIG_STRUCT
{
    FTE_UINT32     nID;
    FTE_CHAR_PTR    pName;
    FTE_UINT32     xSPI;       // SPI channel id
    FTE_UINT32     xSlaveAddr;
    FTE_UINT32     xGPIOHold;
    FTE_UINT32     xGPIOWP;
}   FTE_M25P16_CONFIG, _PTR_ FTE_M25P16_CONFIG_PTR;

typedef FTE_M25P16_CONFIG const _PTR_ FTE_M25P16_CONFIG_CONST_PTR;

typedef struct  _FTE_M25P16_STRUCT
{
    struct _FTE_M25P16_STRUCT * pNext;
    FTE_M25P16_CONFIG_CONST_PTR pConfig;
    FTE_SPI_PTR                 pSPI;
    FTE_LWGPIO_PTR              pHold;
    FTE_LWGPIO_PTR              pWP;
    FTE_BOOL                     bWriteEnable;
}   FTE_M25P16, _PTR_ FTE_M25P16_PTR;

FTE_RET     FTE_M25P16_create(FTE_M25P16_CONFIG_PTR pConfig);
FTE_RET     FTE_M25P16_attach(FTE_M25P16_PTR pObj, FTE_UINT32 nParentID);
FTE_RET     FTE_M25P16_detach(FTE_M25P16_PTR pObj, FTE_UINT32 nParentID);

FTE_BOOL    FTE_M25P16_isExist(FTE_M25P16_PTR pM25P16);
FTE_BOOL    FTE_M25P16_isWriteEnabled(FTE_M25P16_PTR pM25P16);
FTE_RET     FTE_M25P16_writeEnable(FTE_M25P16_PTR pObj, FTE_BOOL bEnable);

FTE_RET     FTE_M25P16_eraseSector(FTE_M25P16_PTR pObj, FTE_UINT32 ulAddress);
FTE_RET     FTE_M25P16_read(FTE_M25P16_PTR pObj, FTE_UINT32 ulAddress, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);
FTE_RET     FTE_M25P16_write(FTE_M25P16_PTR pObj, FTE_UINT32 ulAddress, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);

FTE_RET     FTE_M25P16_readStatus(FTE_M25P16_PTR pObj, FTE_UINT32_PTR pStatus);

FTE_M25P16_PTR  FTE_M25P16_get(FTE_UINT32 nID);
FTE_M25P16_PTR  FTE_M25P16_get_first(void);
FTE_M25P16_PTR  FTE_M25P16_getNext(FTE_M25P16_PTR pObj);

#endif

