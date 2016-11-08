#ifndef __FTS_MEM_H__
#define __FTS_MEM_H__

#if FTE_DEBUG
#define FTE_MEM_alloc(nSize)        _FTE_MEM_DEBUG_alloc(__func__, __LINE__, nSize)      
#define FTE_MEM_allocZero(nSize)    _FTE_MEM_DEBUG_allocZero(__func__, __LINE__, nSize)      
#define FTE_MEM_free(pMem);         _FTE_MEM_DEBUG_free(__func__, __LINE__, pMem)      
#else
#define FTE_MEM_alloc(nSize)        _mem_alloc_system(nSize)      
#define FTE_MEM_allocZero(nSize)    _mem_alloc_system_zero(nSize)      
#define FTE_MEM_free(pMem);         _mem_free(pMem)      
#endif

FTE_VOID_PTR _FTE_MEM_DEBUG_alloc(const FTE_CHAR _PTR_ pFunc, FTE_INT32 nLine, FTE_UINT32 nSize);
FTE_VOID_PTR _FTE_MEM_DEBUG_allocZero(const FTE_CHAR _PTR_ pFunc, FTE_INT32 nLine, FTE_UINT32 nSize);
void        _FTE_MEM_DEBUG_free(const FTE_CHAR _PTR_ pFunc, FTE_INT32 nLine, FTE_VOID_PTR pMem);

FTE_INT32   FTE_MEM_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
#endif
