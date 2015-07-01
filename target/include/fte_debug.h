#ifndef __FTE_DEBUG_H__
#define __FTE_DEBUG_H__

#define DEBUG_UNKNOWN       0x00000000
#define DEBUG_OBJECT        0x00000001
#define DEBUG_EVENT         0x00000002
#define DEBUG_NET_MQTT      0x00000100
#define DEBUG_NET_CGI       0x00000200
#define DEBUG_NET_SMNG      0x00000400
#define DEBUG_NET_SNMP      0x00000800
#define DEBUG_NET_SSL       0x00001000

#define DEBUG_ALL           0xFFFFFFFF


#if FTE_DEBUG
#define TRACE(ulModule, ...)    FTE_DEBUG_trace(ulModule, __VA_ARGS__)
#define TRACE_ON(ulModule)      FTE_DEBUG_traceOn(ulModule)
#define TRACE_OFF(ulModule)     FTE_DEBUG_traceOff(ulModule)
#define DEBUG(...)              FTE_DEBUG_error(__func__, __LINE__, __VA_ARGS__)
#define ERROR(...)              FTE_DEBUG_error(__func__, __LINE__, __VA_ARGS__)
#define err_sys(...)            FTE_DEBUG_error(__func__, __LINE__, __VA_ARGS__)
#else
#define TRACE(ulModule, ...)    
#define TRACE_ON(ulModule)      
#define TRACE_OFF(ulModule)     
#define DEBUG(...)              
#define ERROR(...)              
#define err_sys(...)            
#endif
_mqx_int    FTE_DEBUG_trace(uint_32 ulModule, const char _PTR_ fmt_ptr, ... );
_mqx_int    FTE_DEBUG_traceOn(uint_32 ulModule);
_mqx_int    FTE_DEBUG_traceOff(uint_32 ulModule);
_mqx_int    FTE_DEBUG_error(const char _PTR_ pFuncName, int nLine, const char _PTR_ fmt_ptr, ... );

int_32 FTE_DEBUG_shellCmd(int_32 nArgc, char_ptr pArgv[]);

#endif
