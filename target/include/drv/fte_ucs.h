#ifndef _FTE_UCS_H__
#define _FTE_UCS_H__

#include "fte_type.h"
#include "fte_list.h"

#define FTE_UART_CONFIG_DUPLEX_FULL     (0x0100)
#define FTE_UART_TX_BUFF_SIZE           (1024)
#define FTE_UART_RX_BUFF_SIZE           (1024)

#define FTE_UART_STOP_BITS_1            (1)
#define FTE_UART_STOP_BITS_1_5          (2)
#define FTE_UART_STOP_BITS_2            (3)

#define FTE_UART_PARITY_NONE            (1)
#define FTE_UART_PARITY_ODD             (2)
#define FTE_UART_PARITY_EVEN            (3)
#define FTE_UART_PARITY_FORCE           (4)
#define FTE_UART_PARITY_MARK            (5)
#define FTE_UART_PARITY_SPACE           (6)

typedef struct FTE_UCS_UART_CONFIG_STRUCT
{
    FTE_BOOL        bFullDuplex;
    FTE_UINT32      nBaudrate;
    FTE_UINT32      nDataBits;
    FTE_UINT32      nParity;
    FTE_UINT32      nStopBits;
}   FTE_UCS_UART_CONFIG, _PTR_ FTE_UCS_UART_CONFIG_PTR;

typedef struct _FTE_UCS_CONFIG_STRUCT
{
    FTE_UINT32      nID;
    FTE_CHAR_PTR    pName;
    FTE_CHAR_PTR    pUART;
    FTE_UINT32      nFlowCtrlID;
    FTE_UINT32      nFlowCtrl2ID;
    FTE_BOOL        bFullDuplex;
    FTE_UINT32      nBaudrate;
    FTE_UINT32      nDataBits;
    FTE_UINT32      nParity;
    FTE_UINT32      nStopBits;
    FTE_UINT32      nRecvBufLen;
    FTE_UINT32      nSendBufLen;
}   FTE_UCS_CONFIG, _PTR_ FTE_UCS_CONFIG_PTR;

typedef FTE_UCS_CONFIG const _PTR_ FTE_UCS_CONFIG_CONST_PTR;

typedef struct _FTE_UART_CMD_SERVER_STRUCT
{
    FTE_UCS_CONFIG_CONST_PTR    pConfig;
    FTE_LIST        xParents;
    FTE_BOOL        bRootAttached;
    pointer         pIter;

    FTE_BOOL        bFullDuplex;
    FTE_UINT32      nBaudrate;
    FTE_UINT32      nDataBits;
    FTE_UINT32      nParity;
    FTE_UINT32      nStopBits;

    _task_id        hTaskRX;
    _task_id        hTaskTX;

    MQX_FILE_PTR    pFD;
    FTE_LWGPIO_PTR  pFlowCtrl;
    FTE_LWGPIO_PTR  pFlowCtrl2;

    FTE_SYS_LOCK    xSEMLock;
    FTE_SYS_LOCK    xSEMSend;
    FTE_SYS_LOCK    xSEMRecv;
    FTE_SYS_LOCK    xSEMSendNotEmtry;

    FTE_UINT32      nRecvHead;
    FTE_UINT32      nRecvTail;
    FTE_UINT32      nRecvCount;
    FTE_UINT8_PTR   pRecvBuf;

    FTE_UINT32      nSendHead;
    FTE_UINT32      nSendTail;
    FTE_UINT32      nSendCount;
    FTE_UINT8_PTR   pSendBuf;
}   FTE_UCS, _PTR_ FTE_UCS_PTR;

FTE_RET     FTE_UCS_create(FTE_UCS_CONFIG_CONST_PTR pConfig);
FTE_RET     FTE_UCS_destroy(FTE_UCS_PTR pUCS);
FTE_RET     FTE_UCS_attach(FTE_UCS_PTR pUCS, FTE_UINT32 nParent);
FTE_RET     FTE_UCS_detach(FTE_UCS_PTR pUCS, FTE_UINT32 nParent);

FTE_UCS_PTR FTE_UCS_get(FTE_UINT32 nID);

FTE_UINT32  FTE_UCS_count(void);
FTE_UCS_PTR FTE_UCS_getAt(FTE_UINT32 ulIndex);
/*
FTE_UCS_PTR FTE_UCS_get_first(pointer _PTR_ ppIter);
FTE_UCS_PTR FTE_UCS_get_next(pointer _PTR_ ppIter);
*/
FTE_RET     FTE_UCS_init(FTE_UCS_PTR pUCS);

FTE_RET     FTE_UCS_setBaudrate(FTE_UCS_PTR pUCS, FTE_UINT32 nBaudrate);
FTE_RET     FTE_UCS_getBaudrate(FTE_UCS_PTR pUCS, FTE_UINT32_PTR pBaudrate);
FTE_RET     FTE_UCS_setDatabits(FTE_UCS_PTR pUCS, FTE_UINT32 nDataBits);
FTE_RET     FTE_UCS_getDatabits(FTE_UCS_PTR pUCS, FTE_UINT32_PTR pDataBits);
FTE_RET     FTE_UCS_setParity(FTE_UCS_PTR pUCS, FTE_UINT32 nParity);
FTE_RET     FTE_UCS_getParity(FTE_UCS_PTR pUCS, FTE_UINT32_PTR pParity);
FTE_RET     FTE_UCS_setStopbits(FTE_UCS_PTR pUCS, FTE_UINT32 nStopBits);
FTE_RET     FTE_UCS_getStopbits(FTE_UCS_PTR pUCS, FTE_UINT32_PTR pStopBits);
FTE_RET     FTE_UCS_setDuplexMode(FTE_UCS_PTR pUCS, FTE_BOOL bFullDuplex);
FTE_RET     FTE_UCS_getDuplexMode(FTE_UCS_PTR pUCS, FTE_BOOL_PTR FullDuplex);
FTE_RET     FTE_UCS_setUART
(
    FTE_UCS_PTR pUCS,
    FTE_UCS_UART_CONFIG const _PTR_ pConfig
);
    
FTE_RET     FTE_UCS_getUART
(
    FTE_UCS_PTR     pUCS,
    FTE_UCS_UART_CONFIG_PTR pConfig
);
    
FTE_UINT32  FTE_UCS_clear(FTE_UCS_PTR pUCS);
FTE_UINT32  FTE_UCS_recvdLen(FTE_UCS_PTR pUCS);
FTE_UINT32  FTE_UCS_recv(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen);
FTE_UINT32  FTE_UCS_recvLast(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen);
FTE_UINT32  FTE_UCS_send(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pData, FTE_UINT32 nDataLen, FTE_BOOL bHold);
FTE_BOOL    FTE_UCS_waitForSendCompletion(FTE_UCS_PTR pUCS, FTE_UINT32 nTimeout);
FTE_UINT32  FTE_UCS_sendAndRecv(FTE_UCS_PTR pUCS, FTE_UINT8_PTR pData, FTE_UINT32 nDataLen, FTE_UINT8_PTR pBuff, FTE_UINT32 nBuffLen, FTE_UINT32 nDelay, FTE_UINT32 nTimeout);

FTE_UINT32  FTE_UCS_MODBUS_getRegs(FTE_UCS_PTR pUCS, FTE_UINT8 ucDeviceID, FTE_UINT16 usAddr, FTE_UINT16_PTR pRegs, FTE_UINT8 nCount, FTE_UINT32 nTimeout);
FTE_UINT32  FTE_UCS_MODBUS_setReg(FTE_UCS_PTR pUCS, FTE_UINT8 ucDeviceID, FTE_UINT16 usAddr, FTE_UINT16 usValue, FTE_UINT32 nTimeout);

int_32      FTE_UCS_SHELL_cmd(int_32 argc, FTE_CHAR_PTR argv[] );

void        FTE_UCS_TASK_send(FTE_UINT32 nParams);
void        FTE_UCS_TASK_recv(FTE_UINT32 nParams);

FTE_RET     FTE_UCS_setUART
(
    FTE_UCS_PTR pUCS,
    FTE_UCS_UART_CONFIG const _PTR_ pConfig
);
    
FTE_RET     FTE_UCS_getUART
(
    FTE_UCS_PTR     pUCS,
    FTE_UCS_UART_CONFIG_PTR pConfig
);


#endif
