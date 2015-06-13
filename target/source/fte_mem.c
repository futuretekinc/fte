#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include "fte_mem.h"
#include "fte_sys.h"

#if DEBUG_MEM
#define FTS_MEM_TUPLE_MAX   200
#else
#define FTS_MEM_TUPLE_MAX   1
#endif

typedef struct
{
    uint_32     ulSize;
    uint_32     pMem[];
}   FTE_MEM_BLOCK, _PTR_ FTE_MEM_BLOCK_PTR;

static  uint_32 nTotalSize = 0;
static  uint_32 nMemTuple = 0;
static  uint_32 pMemTuple[FTS_MEM_TUPLE_MAX] = {0,};

void    fte_mem_init(void)
{
}

void *  _FTE_MEM_alloc(uint_32 nSize)
{
    return  _mem_alloc_system(nSize);
}

void *  _FTE_MEM_allocZero(uint_32 nSize)
{
    return  _mem_alloc_system_zero(nSize);
}

void    _FTE_MEM_free(void * pMem)
{
    _mem_free(pMem);
}

uint_32 FTE_MEM_getAllocSize(void)
{
    return  nTotalSize;
}

uint_32 FTE_MEM_allocCount(void)
{
    return  nMemTuple;
}

void *  _FTE_MEM_DEBUG_alloc(const char *pFunc, int nLine, uint_32 nSize)
{
    FTE_MEM_BLOCK_PTR pBlock = _mem_alloc_system(sizeof(FTE_MEM_BLOCK) + nSize);
    
    if (pBlock != NULL)
    {
       pBlock->ulSize = nSize;
        for(int i = 0 ; i < FTS_MEM_TUPLE_MAX ; i++)
        {
            if (pMemTuple[i] == 0)
            {
                nMemTuple++;
                pMemTuple[i] = (uint_32)pBlock;
                break;
            }
        }
        nTotalSize += sizeof(FTE_MEM_BLOCK) + nSize;
    }
    else
    {
        FTE_SYS_reset();
    }
    
    printf("%s[%d] : Size = %d, pMem = %08lx, pFree = %d\n", pFunc, nLine, nSize, pBlock->pMem, _mem_get_free());
    
    return  pBlock->pMem;
}

void *  _FTE_MEM_DEBUG_allocZero(const char *pFunc, int nLine, uint_32 nSize)
{
    FTE_MEM_BLOCK_PTR pBlock = _mem_alloc_system_zero(sizeof(FTE_MEM_BLOCK) + nSize);
    if (pBlock != NULL)
    {
        pBlock->ulSize = nSize;
        for(int i = 0 ; i < FTS_MEM_TUPLE_MAX ; i++)
        {
            if (pMemTuple[i] == 0)
            {
                nMemTuple++;
                pMemTuple[i] = (uint_32)pBlock;
                break;
            }
        }
        nTotalSize += sizeof(FTE_MEM_BLOCK) + nSize;
    }
    else
    {
        FTE_SYS_reset();
    }
    
    printf("%s[%d] : Size = %d, pMem = %08lx, pFree = %d\n", pFunc, nLine, nSize, pBlock->pMem, _mem_get_free());

    return  pBlock->pMem;
}

void    _FTE_MEM_DEBUG_free(const char *pFunc, int nLine, void *pMem)
{
    FTE_MEM_BLOCK_PTR pBlock = (FTE_MEM_BLOCK_PTR)((uint_32)pMem - sizeof(FTE_MEM_BLOCK));

    printf("%s[%d] : pMem = %08lx", pFunc, nLine, pMem);
    
     for(int i = 0 ; i < FTS_MEM_TUPLE_MAX ; i++)
    {
        if (pMemTuple[i] == (uint_32)pBlock)
        {
            nMemTuple--;
            pMemTuple[i] = 0;
            break;
        }
    }
   
    nTotalSize -= sizeof(FTE_MEM_BLOCK) + pBlock->ulSize;   
    _mem_free(pBlock);
    
    printf(", pFree = %d\n", _mem_get_free());
}


/******************************************************************************
 * Shell command
 ******************************************************************************/
int_32  FTE_MEM_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            { 
                printf("Total Allocated Memory : %d\n", nTotalSize);
                printf(" Allocated Block Count : %d\n", nMemTuple);
                printf(" Real free size : %d\n", _mem_get_free());
            }
            break;
            
        case    2:
            {
                if (strcmp(argv[1], "list") == 0)
                {
                    int nCount = 0;
                    
                    printf("Total Allocated Memory : %d\n", nTotalSize);
                    printf(" Allocated Block Count : %d\n", nMemTuple);
                    printf(" Allocated Block\n");
                    
                    for(int i = 0 ; i < FTS_MEM_TUPLE_MAX ; i++)
                    {
                        FTE_MEM_BLOCK_PTR pBlock = (FTE_MEM_BLOCK_PTR)pMemTuple[i];
                        if (pBlock != NULL)
                        {
                            printf("%3d : %08lx : %3d\n", ++nCount, pBlock->pMem, pBlock->ulSize);
                        }
                    }
                }
                else if (strcmp(argv[1], "test") == 0)
                {
                    for(int i = 0 ; i < 1000 ; i++)
                    {
                        void *pData1 = _FTE_MEM_alloc(134);
                        void *pData2 = _FTE_MEM_allocZero(12);
                        printf("%d : pData1 = %08lx, pData2 = %08lx\n", i, pData1, pData2);
                        _FTE_MEM_free(pData1);
                        _FTE_MEM_free(pData2);
                    }
                }
            }            
            break;
        }
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s\n", argv[0]);
        }
        else
        {
            printf("Usage : %s\n", argv[0]);
        }
    }
    return   return_code;
}
