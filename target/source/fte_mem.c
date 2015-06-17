#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include "fte_mem.h"
#include "fte_sys.h"

typedef struct
{
    uint_32     ulSize;
    void _PTR_  pMem;
}   FTE_MEM_BLOCK, _PTR_ FTE_MEM_BLOCK_PTR;

static  uint_32 nTotalSize = 0;
static  uint_32 ulMemTuples = 0;
static  uint_32 ulMemTuplesMax = 0;
static  FTE_MEM_BLOCK_PTR pMemTuples= NULL;
static  boolean bCapture    = FALSE;
static  boolean bVerbose    = FALSE;

_mqx_uint   FTE_MEM_CAPTURE_create(uint_32 ulTuples)
{
    if ((pMemTuples != NULL) || (ulTuples == 0))
    {
        return  MQX_ERROR;
    }
    
    pMemTuples = (FTE_MEM_BLOCK_PTR)_mem_alloc_system_zero(sizeof(FTE_MEM_BLOCK) * ulTuples);
    if (pMemTuples == NULL)
    {
        return  MQX_ERROR;
    }
    ulMemTuples     = 0;
    ulMemTuplesMax  = ulTuples;
    
    return  MQX_OK;
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

void *  _FTE_MEM_DEBUG_alloc(const char *pFunc, int nLine, uint_32 ulSize)
{
    if (bCapture && pMemTuples)
    {
        void _PTR_ pBlock = NULL;
        
        if (ulMemTuples < ulMemTuplesMax)
        {
            pBlock = _mem_alloc_system(ulSize);        
            if (pBlock != NULL)
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
                printf("%16s[%04d] : %08lx, %4d, %5d\n", pFunc, nLine, pBlock, ulSize, _mem_get_free());
            }
            else
            {
                printf("%16s[%04d] : not allocated, %5d\n", pFunc, nLine, _mem_get_free());
            }
        }
        
        return  pBlock;
    }
    else
    {
        return  _mem_alloc_system(ulSize);
    }
}

void *  _FTE_MEM_DEBUG_allocZero(const char *pFunc, int nLine, uint_32 ulSize)
{
    if (bCapture && pMemTuples)
    {
        void _PTR_ pBlock = NULL;
        
        if (ulMemTuples < ulMemTuplesMax)
        {
            pBlock = _mem_alloc_system_zero(ulSize);        
            if (pBlock != NULL)
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
                printf("%16s[%04d] : %08lx, %4d, %5d\n", pFunc, nLine, pBlock, ulSize, _mem_get_free());
            }
            else
            {
                printf("%16s[%04d] : not allocated, %5d\n", pFunc, nLine, _mem_get_free());
            }
        }
        
        return  pBlock;
    }
    else
    {
        return  _mem_alloc_system_zero(ulSize);
    }
}

void    _FTE_MEM_DEBUG_free(const char *pFunc, int nLine, void *pMem)
{
    if (bCapture && pMemTuples)
    {
        if (bVerbose)
        {
            printf("%16s[%04d] : %08lx", pFunc, nLine, pMem);
        }
        
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
        
        if (bVerbose)
        {
            printf(", %5d\n", _mem_get_free());
        }
    }
    else
    {
        _mem_free(pMem);
    }
}


/******************************************************************************
 * Shell command
 ******************************************************************************/
int_32  FTE_MEM_SHELL_cmd(int_32 nArgc, char_ptr ppArgv[])
{
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nRet = SHELL_EXIT_SUCCESS;
    
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
                            FTE_MEM_CAPTURE_create(100);
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
                        uint_32 ulCount;
                        if (Shell_parse_number(ppArgv[2], &ulCount) != TRUE)
                        {
                            bPrintUsage = TRUE;
                            break;
                        }
                        
                        if (FTE_MEM_CAPTURE_create(ulCount) == MQX_OK)
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
