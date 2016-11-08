#ifndef _FTE_SPI_H__
#define _FTE_SPI_H__

typedef struct FTE_SPI_CONFIG_STRUCT
{
    FTE_UINT32      nID;
    FTE_UINT32      xType;
    FTE_CHAR_PTR    pName;
    FTE_UINT32      xPort;
    FTE_UINT32      xCSGPIO;
    FTE_UINT32      nBaudrate;
    FTE_UINT32      xFlags;
}   FTE_SPI_CONFIG, _PTR_ FTE_SPI_CONFIG_PTR;

typedef FTE_SPI_CONFIG const _PTR_ FTE_SPI_CONFIG_CONST_PTR;

typedef struct FTE_SPI_CHANNEL_STRUCT
{
    MQX_FILE_PTR    xFD;
    FTE_SYS_LOCK    xLock;
    FTE_UINT32      nBaudrate;
    FTE_UINT32      xFlags;
    FTE_UINT32      nCount;
}   FTE_SPI_CHANNEL, _PTR_ FTE_SPI_CHANNEL_PTR;

typedef struct FTE_SPI_STRUCT
{
    struct FTE_SPI_STRUCT *     pNext;
    FTE_SPI_CONFIG_CONST_PTR    pConfig;
    FTE_SPI_CHANNEL_PTR         pChannel;
    FTE_UINT32                  nParent;
    FTE_LWGPIO_PTR              pCSPort;
    SPI_CS_CALLBACK_STRUCT  callback;

}   FTE_SPI, _PTR_ FTE_SPI_PTR;

FTE_RET   FTE_SPI_create(FTE_SPI_CONFIG_PTR pConfig);
FTE_RET   FTE_SPI_attach(FTE_SPI_PTR pSPI, FTE_UINT32 nParent);
FTE_RET   FTE_SPI_detach(FTE_SPI_PTR pSPI);

FTE_UINT32  FTE_SPI_count(void);
FTE_SPI_PTR FTE_SPI_get(FTE_UINT32 xID);
FTE_UINT32  FTE_SPI_getParent(FTE_SPI_PTR pSPI);
FTE_SPI_PTR FTE_SPI_getFirst(void);
FTE_SPI_PTR FTE_SPI_getNext(FTE_SPI_PTR pSPI);

FTE_RET   FTE_SPI_setBaudrate(FTE_SPI_PTR pSPI, FTE_UINT32 nBaudrate);                         
FTE_RET   FTE_SPI_getBaudrate(FTE_SPI_PTR pSPI, FTE_UINT32_PTR pBaudrate);                         
FTE_RET   FTE_SPI_setFlags(FTE_SPI_PTR pSPI, FTE_UINT32 xFlags);                         
FTE_RET   FTE_SPI_getFlags(FTE_SPI_PTR pSPI, FTE_UINT32_PTR pFlags);
FTE_RET   FTE_SPI_read(FTE_SPI_PTR pSPI, FTE_UINT8_PTR pCmd, FTE_UINT32 nCmdLen, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen);
FTE_RET   FTE_SPI_write(FTE_SPI_PTR pSPI, FTE_UINT8_PTR pCmd, FTE_UINT32 nCmdLen, FTE_UINT8_PTR pData, FTE_UINT32 nDataLen);


FTE_INT32      FTE_SPI_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );
#endif
