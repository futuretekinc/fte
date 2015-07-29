#ifndef _FTE_SPI_H__
#define _FTE_SPI_H__

typedef struct _FTE_SPI_CONFIG_STRUCT
{
    uint_32         nID;
    uint_32         xType;
    char_ptr        pName;
    uint_32         xPort;
    uint_32         xCSGPIO;
    uint_32         nBaudrate;
    uint_32         xFlags;
}   FTE_SPI_CONFIG, _PTR_ FTE_SPI_CONFIG_PTR;

typedef FTE_SPI_CONFIG const _PTR_ FTE_SPI_CONFIG_CONST_PTR;

typedef struct _FTE_SPI_CHANNEL_STRUCT
{
    MQX_FILE_PTR    xFD;
    LWSEM_STRUCT    xLWSEM;
    uint_32         nBaudrate;
    uint_32         xFlags;
    uint_32         nCount;
}   FTE_SPI_CHANNEL, _PTR_ FTE_SPI_CHANNEL_PTR;

typedef struct _FTE_SPI_STRUCT
{
    struct _FTE_SPI_STRUCT *    pNext;
    FTE_SPI_CONFIG_CONST_PTR    pConfig;
    FTE_SPI_CHANNEL_PTR         pChannel;
    uint_32                     nParent;
    FTE_LWGPIO_PTR              pCSPort;
    SPI_CS_CALLBACK_STRUCT  callback;

}   FTE_SPI, _PTR_ FTE_SPI_PTR;

_mqx_uint   FTE_SPI_create(FTE_SPI_CONFIG_PTR pConfig);
_mqx_uint   FTE_SPI_attach(FTE_SPI_PTR pSPI, uint_32 nParent);
_mqx_uint   FTE_SPI_detach(FTE_SPI_PTR pSPI);

uint_32     FTE_SPI_count(void);
FTE_SPI_PTR FTE_SPI_get(uint_32 xID);
uint_32     FTE_SPI_getParent(FTE_SPI_PTR pSPI);
FTE_SPI_PTR FTE_SPI_getFirst(void);
FTE_SPI_PTR FTE_SPI_getNext(FTE_SPI_PTR pSPI);

_mqx_uint   FTE_SPI_setBaudrate(FTE_SPI_PTR pSPI, uint_32 nBaudrate);                         
_mqx_uint   FTE_SPI_getBaudrate(FTE_SPI_PTR pSPI, uint_32 *pBaudrate);                         
_mqx_uint   FTE_SPI_setFlags(FTE_SPI_PTR pSPI, uint_32 xFlags);                         
_mqx_uint   FTE_SPI_getFlags(FTE_SPI_PTR pSPI, uint_32 *pFlags);
_mqx_uint   FTE_SPI_read(FTE_SPI_PTR pSPI, uint_8_ptr pCmd, uint_32 nCmdLen, uint_8_ptr pBuff, uint_32 nBuffLen);
_mqx_uint   FTE_SPI_write(FTE_SPI_PTR pSPI, uint_8_ptr pCmd, uint_32 nCmdLen, uint_8_ptr pData, uint_32 nDataLen);


int_32      FTE_SPI_SHELL_cmd(int_32 argc, char_ptr argv[] );
#endif
