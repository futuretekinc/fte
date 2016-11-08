#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include "fte_type.h"
#include "fte_mem.h"
#include "fte_sys.h"

#define FTE_MEM_DEFAULT_CAPTURE_COUNT       300
#define FTE_MEM_DEFAULT_CAPTURE_SKIP_COUNT   300
typedef struct
{
    FTE_UINT32     ulSize;
    FTE_VOID_PTR    pMem;
}   FTE_MEM_BLOCK, _PTR_ FTE_MEM_BLOCK_PTR;

static  FTE_UINT32 nTotalSize = 0;
static  FTE_UINT32 ulMemTuples = 0;
static  FTE_UINT32 ulMemTuplesMax = 0;
static  FTE_MEM_BLOCK_PTR pMemTuples= NULL;
static  FTE_BOOL    bCapture    = FALSE;
static  FTE_BOOL    bVerbose    = FALSE;
static  FTE_UINT32  ulSkipCount = 0;

FTE_RET   FTE_MEM_CAPTURE_create
(
    FTE_UINT32 ulTuples
)
{
    if ((pMemTuples != NULL) || (ulTuples == 0))
    {
        return  FTE_RET_ERROR;
    }
    
    pMemTuples = (FTE_MEM_BLOCK_PTR)_mem_alloc_system_zero(sizeof(FTE_MEM_BLOCK) * ulTuples);
    if (pMemTuples == NULL)
    {
        return  FTE_RET_ERROR;
    }
    ulMemTuples     = 0;
    ulMemTuplesMax  = ulTuples;
    
    return  FTE_RET_OK;
}

void FTE_MEM_CAPTURE_destroy(void)
{
    if (pMemTuples == NULL)
    {
        return ;
    }
    
    ulMemTuplesMax  = 0;
    ulMemTuples     = 0;    
    _mem_free(pMemTuples);
    pMemTuples      = NULL;
}

FTE_VOID_PTR _FTE_MEM_DEBUG_alloc
(
    const FTE_CHAR _PTR_ pFunc, 
    FTE_INT32 nLine, 
    FTE_UINT32 ulSize
)
{
    if (bCapture)
    {
        void _PTR_ pBlock = NULL;
        
        if (pMemTuples == NULL)
        {
            FTE_MEM_CAPTURE_create(FTE_MEM_DEFAULT_CAPTURE_COUNT);
        }
        
        pBlock = _mem_alloc_system(ulSize);        
        if (pBlock != NULL)
        {
            if (ulSkipCount < FTE_MEM_DEFAULT_CAPTURE_SKIP_COUNT)
            {
                ulSkipCount++;
            }
            else  if (ulMemTuples < ulMemTuplesMax)
            {
                for(int i = 0 ; i < ulMemTuplesMax ; i++)
                {
                    if (pMemTuples[i].pMem == 0)
                    {
                        ulMemTuples++;
                        pMemTuples[i].pMem = pBlock;
                        pMemTuples[i].ulSize = ulSize;
                        nTotalSize += ulSize;
                        break;
                    }
                }
            }
        }
        
        if (bVerbose)
        {
            if (pBlock != NULL)
            {
                printf("MEM ALLOC : %16s[%04d] : %08lx, %4d, %5d\n", pFunc, nLine, pBlock, ulSize, _mem_get_free());
            }
            else
            {
                printf("MEM ALLOC : %16s[%04d] : not allocated, %5d\n", pFunc, nLine, _mem_get_free());
            }
        }
        
        return  pBlock;
    }
    else
    {
        return  _mem_alloc_system(ulSize);
    }
}

FTE_VOID_PTR _FTE_MEM_DEBUG_allocZero
(
    const FTE_CHAR _PTR_ pFunc, 
    FTE_INT32 nLine, 
    FTE_UINT32 ulSize
)
{
    if (bCapture)
    {
        void _PTR_ pBlock = NULL;
        
        if (pMemTuples == NULL)
        {
            FTE_MEM_CAPTURE_create(FTE_MEM_DEFAULT_CAPTURE_COUNT);
        }
        
        pBlock = _mem_alloc_system_zero(ulSize);        
        if (pBlock != NULL)
        {
            if (ulSkipCount < FTE_MEM_DEFAULT_CAPTURE_SKIP_COUNT)
            {
                ulSkipCount++;
            }
            else if (ulMemTuples < ulMemTuplesMax)
            {
                for(int i = 0 ; i < ulMemTuplesMax ; i++)
                {
                    if (pMemTuples[i].pMem == 0)
                    {
                        ulMemTuples++;
                        pMemTuples[i].pMem = pBlock;
                        pMemTuples[i].ulSize = ulSize;
                        nTotalSize += ulSize;
                        break;
                    }
                }
            }
        }
        
        if (bVerbose)
        {
            if (pBlock != NULL)
            {
                printf("MEM ALLOC : %16s[%04d] : %08lx, %4d, %5d\n", pFunc, nLine, pBlock, ulSize, _mem_get_free());
            }
            else
            {
                printf("MEM ALLOC : %16s[%04d] : not allocated, %5d\n", pFunc, nLine, _mem_get_free());
            }
        }
        
        return  pBlock;
    }
    else
    {
        return  _mem_alloc_system_zero(ulSize);
    }
}

void    _FTE_MEM_DEBUG_free
(
    const FTE_CHAR _PTR_ pFunc, 
    FTE_INT32 nLine, 
    FTE_VOID_PTR pMem
)
{
    if (bCapture)
    {
        
        if (pMemTuples == NULL)
        {
            FTE_MEM_CAPTURE_create(FTE_MEM_DEFAULT_CAPTURE_COUNT);
        }
        
        if (bVerbose)
        {
            printf("MEM FREE  : %16s[%04d] : %08lx", pFunc, nLine, pMem);
        }
        
        if (pMem != NULL)
        {
             for(int i = 0 ; i < ulMemTuplesMax ; i++)
            {
                if (pMemTuples[i].pMem == pMem)
                {
                    if (bVerbose)
                    {
                        printf(", %4d", pMemTuples[i].ulSize);
                    }
                    nTotalSize -= pMemTuples[i].ulSize;   
                    pMemTuples[i].pMem = NULL;
                    pMemTuples[i].ulSize = 0;
                    ulMemTuples--;
                    break;
                }
            }
           
            _mem_free(pMem);
        }
        
        if (bVerbose)
        {
            printf(", %5d\n", _mem_get_free());
        }
    }
    else
    {
        if (pMem != NULL)
        {
            _mem_free(pMem);
        }
    }
}


/******************************************************************************
 * Shell command
 ******************************************************************************/
FTE_INT32  FTE_MEM_SHELL_cmd
(
    FTE_INT32   nArgc, 
    FTE_CHAR_PTR ppArgv[]
)
{
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, ppArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            { 
                printf("%16s : %d\n", "Allocated Memory",   nTotalSize);
                printf("%16s : %d\n", "Block Count",        ulMemTuples);
                printf("%16s : %d\n", "Real Free Size",     _mem_get_free());
            }
            break;
            
        case    2:
            {
                if (strcmp(ppArgv[1], "list") == 0)
                {
                    int nCount = 0;
                    printf("%16s : %s\n", "State",  bCapture?"Run":"Stop");
                    printf("%16s : %d\n", "Allocated Memory",   nTotalSize);
                    printf("%16s : %d\n", "Block Count",        ulMemTuples);
                    printf("%16s : %d\n", "Real Free Size",     _mem_get_free());
                    printf("%16s\n", "Allocated Block");
                    
                    for(int i = 0 ; i < ulMemTuplesMax ; i++)
                    {
                        if (pMemTuples[i].pMem != NULL)
                        {
                            printf("%3d : %08lx, %4d\n", ++nCount, pMemTuples[i].pMem, pMemTuples[i].ulSize);
                        }
                    }
                }
                else if (strcmp(ppArgv[1], "capture") == 0)
                {
                    if (bCapture)
                    {
                        printf("Already captured.\n");
                    }
                    else
                    {
                        if (pMemTuples == NULL)
                        {
                            FTE_MEM_CAPTURE_create(FTE_MEM_DEFAULT_CAPTURE_COUNT);
                        }
                        bCapture = TRUE;
                    }
                        
                }
                else if (strcmp(ppArgv[1], "verbose") == 0)
                {
                    bVerbose = TRUE;
                }
                else if (strcmp(ppArgv[1], "silent") == 0)
                {
                    bVerbose = FALSE;
                }
                else if (strcmp(ppArgv[1], "stop") == 0)
                {
                    bCapture = FALSE;
                }
                else if (strcmp(ppArgv[1], "clear") == 0)
                {
                    if (bCapture)
                    {
                        printf("During the operation it could not be initialized.\n");
                    }
                    else
                    {
                        FTE_MEM_CAPTURE_destroy();
                    }
                }
            }            
            break;
         case    3:
            {
                if (strcmp(ppArgv[1], "capture") == 0)
                {
                    if (bCapture && (pMemTuples != NULL))
                    {
                        printf("Already captured.\n");
                    }
                    else
                    {
                        FTE_UINT32 ulCount;
                        if (Shell_parse_number(ppArgv[2], &ulCount) != TRUE)
                        {
                            bPrintUsage = TRUE;
                            break;
                        }
                        
                        if (FTE_MEM_CAPTURE_create(ulCount) == FTE_RET_OK)
                        {
                            bCapture = TRUE;
                        }
                        else
                        {
                            bPrintUsage = TRUE;
                        }
                    }                        
                }
            }
       }
    }
    
    if (bPrintUsage || (nRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s\n", ppArgv[0]);
        }
        else
        {
            printf("Usage : %s [<commands>]\n", ppArgv[0]);
            printf("  capture [count]\n");
            printf("    Start the memory allocation information capture.\n");
            printf("  stop\n");
            printf("    Stop the memory allocation information capture.\n");
            printf("  list\n");
            printf("    Show the memory allocation information.\n");
            printf("  clear\n");
            printf("    Clear the memory allocation information.\n");
            printf("  verbose\n");
            printf("    start to show the memory allocation information.\n");
            printf("  silent\n");
            printf("    stop to show the memory allocation information.\n");
        }
    }
    return   nRet;
}
