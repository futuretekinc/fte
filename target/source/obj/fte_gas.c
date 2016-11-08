#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"


FTE_INT32  fte_gas_shell_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    { 
        switch(nArgc)
        {
        case    1:
            { 
                FTE_INT32 count = FTE_OBJ_count(FTE_OBJ_TYPE_GAS, FTE_OBJ_CLASS_MASK, FALSE);
                for(FTE_INT32 i = 0 ; i < count ; i++)
                {
                    FTE_OBJECT_PTR pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_GAS, FTE_OBJ_CLASS_MASK, i, FALSE);
                    if (pObj != NULL)
                    {
                    }
                }
            }
            break;
        case    3:
            {
            }
            break;
            
        case    4:
            {
                FTE_UINT32 nValue;
                
                if (! Shell_parse_number( pArgv[2], &nValue))  
                {
                   xRet = SHELL_EXIT_ERROR;
                   goto error;
                }
                
                FTE_OBJECT_PTR          pObj = FTE_OBJ_get(nValue);
                if (pObj == NULL)
                {
                   xRet = SHELL_EXIT_ERROR;
                   goto error;
                }
                
                
                if (strcmp(pArgv[1], "mode") == 0)
                {
                    char    pBuff[32];
                    
                    if (!Shell_parse_number(pArgv[3], &nValue))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    sprintf(pBuff, "K %d\r\n", nValue);
                    fte_ucs_write(((FTE_COZIR_STATUS_PTR)pObj->pStatus)->pUCS, (FTE_UINT8_PTR)pBuff, strlen(pBuff));
                    
                }
            }
            break; 
        }
    }
    
error:    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]\n", pArgv[0]);
            printf("        id - COZIR Index \n");
        }
    }

    return   xRet;
}
            