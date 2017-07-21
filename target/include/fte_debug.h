#ifndef __FTE_DEBUG_H__
#define __FTE_DEBUG_H__

#define DEBUG_UNKNOWN       0x00000000
#define DEBUG_OBJECT        0x00000001
#define DEBUG_EVENT         0x00000002
#define DEBUG_DI            0x00000004
#define DEBUG_DEVICE        0x00000008
#define DEBUG_NET_MQTT      0x00000100
#define DEBUG_NET_CGI       0x00000200
#define DEBUG_NET_SMNG      0x00000400
#define DEBUG_NET_SNMP      0x00000800
#define DEBUG_NET_SSL       0x00001000
#define DEBUG_NET_LORA      0x00002000
#define DEBUG_NET_MBTCP     0x00004000

#define DEBUG_ALL           0xFFFFFFFF

#define FTE_MODULE_NET_SNMP     (1 << 11)     
#define FTE_MODULE_NET_SMNG     (1 << 10)     
#define FTE_MODULE_NET_MBTCP    (1 << 14)
#define FTE_MODULE_NET_LORA     (1 << 13)
#define FTE_MODULE_NET_SSL      (1 << 12)
#define FTE_MODULE_DI           (1 << 2)
#define FTE_MODULE_EVENT        (1 << 1)
#define FTE_MODULE_OBJECT       (1 << 0)

#define __MODULE__      0

#if FTE_DEBUG
#define TRACE(...)      FTE_DEBUG_trace(__MODULE__, __VA_ARGS__)
#define TRACE_ON()      FTE_DEBUG_traceOn(__MODULE__)
#define TRACE_OFF()     FTE_DEBUG_traceOff(__MODULE__)
#define DEBUG(...)      FTE_DEBUG_error(__MODULE__, __func__, __LINE__, __VA_ARGS__)
#define ERROR(...)      FTE_DEBUG_error(__MODULE__, __func__, __LINE__, __VA_ARGS__)
#define err_sys(...)    FTE_DEBUG_error(__MODULE__, __func__, __LINE__, __VA_ARGS__)
#define DUMP(pTitle, pBuff, ulSize, ulWrapLen)     FTE_DEBUG_dump(__MODULE__, __func__, __LINE__, pTitle, pBuff, ulSize, ulWrapLen)
#else
#define TRACE(...)    
#define TRACE_ON()      
#define TRACE_OFF()     
#define DEBUG(...)              
#define ERROR(...)              
#define err_sys(...)            
#define DUMP(...)
#endif
void        FTE_DEBUG_init(void);
FTE_RET     FTE_DEBUG_trace(FTE_UINT32 ulModule, const FTE_CHAR _PTR_ fmt_ptr, ... );
FTE_RET     FTE_DEBUG_traceOn(FTE_UINT32 ulModule);
FTE_RET     FTE_DEBUG_traceOff(FTE_UINT32 ulModule);
FTE_BOOL    FTE_DEBUG_isTraceOn(FTE_UINT32 ulModule);
FTE_RET     FTE_DEBUG_error(FTE_UINT32 ulModule, const FTE_CHAR _PTR_ pFuncName, FTE_INT32 nLine, const FTE_CHAR _PTR_ fmt_ptr, ... );
FTE_RET     FTE_DEBUG_dump(FTE_UINT32 ulModule, const FTE_CHAR _PTR_ pFuncName, FTE_INT32 nLine, FTE_CHAR_PTR pTitle, FTE_VOID_PTR pBuff, FTE_UINT32 ulSize, FTE_UINT32 ulWrapLen);

FTE_INT32   FTE_TRACE_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);

#endif
