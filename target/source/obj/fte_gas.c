#include "fte_target.h"
#include "fte_config.h"
#include "fte_log.h"


int_32  fte_gas_shell_cmd(int_32 argc, char_ptr argv[] )
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
                int count = FTE_OBJ_count(FTE_OBJ_TYPE_GAS, FTE_OBJ_CLASS_MASK, FALSE);
                for(int i = 0 ; i < count ; i++)
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
                uint_32 nValue;
                
                if (! Shell_parse_number( argv[2], &nValue))  
                {
                   return_code = SHELL_EXIT_ERROR;
                   goto error;
                }
                
                FTE_OBJECT_PTR          pObj = FTE_OBJ_get(nValue);
                if (pObj == NULL)
                {
                   return_code = SHELL_EXIT_ERROR;
                   goto error;
                }
                
                
                if (strcmp(argv[1], "mode") == 0)
                {
                    char    pBuff[32];
                    
                    if (!Shell_parse_number(argv[3], &nValue))
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    sprintf(pBuff, "K %d\r\n", nValue);
                    fte_ucs_write(((FTE_COZIR_STATUS_PTR)pObj->pStatus)->pUCS, (uint_8_ptr)pBuff, strlen(pBuff));
                    
                }
            }
            break; 
        }
    }
    
error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<id>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<id>]\n", argv[0]);
            printf("        id - COZIR Index \n");
        }
    }

    return   return_code;
}
            