#ifndef _FTE_UCS_H__
#define _FTE_UCS_H__

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

typedef struct _FTE_UCS_CONFIG_STRUCT
{
    uint_32         nID;
    char_ptr        pName;
    char_ptr        pUART;
    uint_32         nFlowCtrlID;
    uint_32         nFlowCtrl2ID;
    boolean         bFullDuplex;
    uint_32         nBaudrate;
    uint_32         nDataBits;
    uint_32         nParity;
    uint_32         nStopBits;
    uint_32         nRecvBufLen;
    uint_32         nSendBufLen;
}   FTE_UCS_CONFIG, _PTR_ FTE_UCS_CONFIG_PTR;

typedef FTE_UCS_CONFIG const _PTR_ FTE_UCS_CONFIG_CONST_PTR;

typedef struct _FTE_UART_CMD_SERVER_STRUCT
{
    FTE_UCS_CONFIG_CONST_PTR    pConfig;
    FTE_LIST        xParents;
    boolean         bRootAttached;
    pointer         pIter;

    boolean         bFullDuplex;
    uint_32         nBaudrate;
    uint_32         nDataBits;
    uint_32         nParity;
    uint_32         nStopBits;

    _task_id        hTaskRX;
    _task_id        hTaskTX;

    MQX_FILE_PTR    pFD;
    FTE_LWGPIO_PTR  pFlowCtrl;
    FTE_LWGPIO_PTR  pFlowCtrl2;

    LWSEM_STRUCT    xSEMLock;
    LWSEM_STRUCT    xSEMSend;
    LWSEM_STRUCT    xSEMRecv;
    LWSEM_STRUCT    xSEMSendNotEmtry;
    uint_32         nRecvHead;
    uint_32         nRecvTail;
    uint_32         nRecvCount;
    uint_8_ptr      pRecvBuf;

    uint_32         nSendHead;
    uint_32         nSendTail;
    uint_32         nSendCount;
    uint_8_ptr      pSendBuf;
}   FTE_UCS, _PTR_ FTE_UCS_PTR;

_mqx_uint       FTE_UCS_create(FTE_UCS_CONFIG_CONST_PTR pConfig);
_mqx_uint       FTE_UCS_destroy(FTE_UCS_PTR pUCS);
_mqx_uint       FTE_UCS_attach(FTE_UCS_PTR pUCS, uint_32 nParent);
_mqx_uint       FTE_UCS_detach(FTE_UCS_PTR pUCS, uint_32 nParent);

FTE_UCS_PTR     FTE_UCS_get(uint_32 nID);

uint_32         FTE_UCS_count(void);
FTE_UCS_PTR     FTE_UCS_getAt(uint_32 ulIndex);
/*
FTE_UCS_PTR     FTE_UCS_get_first(pointer _PTR_ ppIter);
FTE_UCS_PTR     FTE_UCS_get_next(pointer _PTR_ ppIter);
*/
_mqx_uint       FTE_UCS_init(FTE_UCS_PTR pUCS);
_mqx_uint       FTE_UCS_setBaudrate(FTE_UCS_PTR pUCS, uint_32 nBaudrate);
_mqx_uint       FTE_UCS_getBaudrate(FTE_UCS_PTR pUCS, uint_32 *pBaudrate);
_mqx_uint       FTE_UCS_setDatabits(FTE_UCS_PTR pUCS, uint_32 nDataBits);
_mqx_uint       FTE_UCS_getDatabits(FTE_UCS_PTR pUCS, uint_32 *pDataBits);
_mqx_uint       FTE_UCS_setParity(FTE_UCS_PTR pUCS, uint_32 nParity);
_mqx_uint       FTE_UCS_getParity(FTE_UCS_PTR pUCS, uint_32 *pParity);
_mqx_uint       FTE_UCS_setStopbits(FTE_UCS_PTR pUCS, uint_32 nStopBits);
_mqx_uint       FTE_UCS_getStopbits(FTE_UCS_PTR pUCS, uint_32 *pStopBits);
_mqx_uint       FTE_UCS_setDuplexMode(FTE_UCS_PTR pUCS, boolean bFullDuplex);
_mqx_uint       FTE_UCS_getDuplexMode(FTE_UCS_PTR pUCS, boolean *pFullDuplex);

uint_32         FTE_UCS_clear(FTE_UCS_PTR pUCS);
uint_32         FTE_UCS_recvdLen(FTE_UCS_PTR pUCS);
uint_32         FTE_UCS_recv(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen);
uint_32         FTE_UCS_recvLast(FTE_UCS_PTR pUCS, uint_8_ptr pBuff, uint_32 nBuffLen);
uint_32         FTE_UCS_send(FTE_UCS_PTR pUCS, uint_8_ptr pData, uint_32 nDataLen, boolean bHold);
boolean         FTE_UCS_waitForSendCompletion(FTE_UCS_PTR pUCS, uint_32 nTimeout);
uint_32         FTE_UCS_sendAndRecv(FTE_UCS_PTR pUCS, uint_8_ptr pData, uint_32 nDataLen, uint_8_ptr pBuff, uint_32 nBuffLen, uint_32 nDelay, uint_32 nTimeout);

uint_32         FTE_UCS_MODBUS_getRegs(FTE_UCS_PTR pUCS, uint_8 ucDeviceID, uint_16 usAddr, uint_16_ptr pRegs, uint_8 nCount, uint_32 nTimeout);
uint_32         FTE_UCS_MODBUS_setReg(FTE_UCS_PTR pUCS, uint_8 ucDeviceID, uint_16 usAddr, uint_16 usValue, uint_32 nTimeout);

int_32          FTE_UCS_SHELL_cmd(int_32 argc, char_ptr argv[] );

void            FTE_UCS_TASK_send(uint_32 nParams);
void            FTE_UCS_TASK_recv(uint_32 nParams);


#endif
