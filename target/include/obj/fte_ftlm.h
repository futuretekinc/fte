#ifndef _FTE_FTLM_H__
#define _FTE_FTLM_H__

#define FTE_FTLM_DEFAULT_UPDATE_INTERVAL    2000

#define FTE_FTLM_DEFAULT_FULL_DUPLEX        FALSE
#define FTE_FTLM_DEFAULT_BAUDRATE           38400
#define FTE_FTLM_DEFAULT_DATABITS           8
#define FTE_FTLM_DEFAULT_PARITY             FTE_UART_PARITY_NONE
#define FTE_FTLM_DEFAULT_STOPBITS           FTE_UART_STOP_BITS_1

FTE_UINT32     FTE_FTLM_requestData(FTE_OBJECT_PTR pObj);
FTE_RET   FTE_FTLM_receiveData(FTE_OBJECT_PTR pObj);
FTE_RET   FTE_FTLM_set(FTE_OBJECT_PTR pObject, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);
FTE_RET   FTE_FTLM_setAll(FTE_OBJECT_PTR pObj, FTE_UINT32_PTR pulValues);
FTE_RET   FTE_FTLM_setConfig(FTE_OBJECT_PTR pDevice, FTE_CHAR_PTR pJSON);
FTE_RET   FTE_FTLM_getConfig(FTE_OBJECT_PTR pDevice, FTE_CHAR_PTR pBuff, FTE_UINT32 ulBuffLen);

extern  
FTE_GUS_CONFIG FTE_FTLM_defaultConfig;

extern  
FTE_VALUE_TYPE  FTE_FTLM_valueTypes[];

extern  const 
FTE_GUS_MODEL_INFO    FTE_FTLM_GUSModelInfo;

#endif