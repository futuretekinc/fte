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

void *  _FTE_MEM_DEBUG_alloc(const char *pFunc, int nLine, uint_32 nSize);
void *  _FTE_MEM_DEBUG_allocZero(const char *pFunc, int nLine, uint_32 nSize);
void    _FTE_MEM_DEBUG_free(const char *pFunc, int nLine, void *pMem);

int_32  FTE_MEM_SHELL_cmd(int_32 argc, char_ptr argv[]);
#endif
