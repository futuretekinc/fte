#ifndef __FTE_M25P16_H__
#define __FTE_M25P16_H__

typedef struct _FTE_M25P16_CONFIG_STRUCT
{
    uint_32     nID;
    char_ptr    pName;
    uint_32     xSPI;       // SPI channel id
    uint_32     xSlaveAddr;
    uint_32     xGPIOHold;
    uint_32     xGPIOWP;
}   FTE_M25P16_CONFIG, _PTR_ FTE_M25P16_CONFIG_PTR;

typedef FTE_M25P16_CONFIG const _PTR_ FTE_M25P16_CONFIG_CONST_PTR;

typedef struct  _FTE_M25P16_STRUCT
{
    struct _FTE_M25P16_STRUCT * pNext;
    FTE_M25P16_CONFIG_CONST_PTR pConfig;
    FTE_SPI_PTR                 pSPI;
    FTE_LWGPIO_PTR              pHold;
    FTE_LWGPIO_PTR              pWP;
    boolean                     bWriteEnable;
}   FTE_M25P16, _PTR_ FTE_M25P16_PTR;

_mqx_uint       FTE_M25P16_create(FTE_M25P16_CONFIG_PTR pConfig);
_mqx_uint       FTE_M25P16_attach(FTE_M25P16_PTR pObj, uint_32 nParentID);
_mqx_uint       FTE_M25P16_detach(FTE_M25P16_PTR pObj, uint_32 nParentID);

boolean         FTE_M25P16_isExist(FTE_M25P16_PTR pM25P16);
boolean         FTE_M25P16_isWriteEnabled(FTE_M25P16_PTR pM25P16);
_mqx_uint       FTE_M25P16_writeEnable(FTE_M25P16_PTR pObj, boolean bEnable);

_mqx_uint       FTE_M25P16_eraseSector(FTE_M25P16_PTR pObj, uint_32 ulAddress);
_mqx_uint       FTE_M25P16_read(FTE_M25P16_PTR pObj, uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen);
_mqx_uint       FTE_M25P16_write(FTE_M25P16_PTR pObj, uint_32 ulAddress, uint_8_ptr pBuff, uint_32 ulLen);

_mqx_uint       FTE_M25P16_readStatus(FTE_M25P16_PTR pObj, uint_32_ptr pStatus);

FTE_M25P16_PTR  FTE_M25P16_get(uint_32 nID);
FTE_M25P16_PTR  FTE_M25P16_get_first(void);
FTE_M25P16_PTR  FTE_M25P16_getNext(FTE_M25P16_PTR pObj);

#endif

