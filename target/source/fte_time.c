#include <mqx.h>
#include <bsp.h>
#include <shell.h>
#include <stdlib.h>
#include "fte_sys.h"

_mqx_uint   FTE_TIME_init(void)
{
    _rtc_init(RTC_INIT_FLAG_ENABLE);    
    
    return  MQX_OK;
}

uint_32     FTE_TIME_diff(TIME_STRUCT_PTR pTime1, TIME_STRUCT_PTR pTime2)
{
    MQX_TICK_STRUCT xTick1, xTick2;
    boolean         bOverflow;
    
    _time_to_ticks(pTime1, &xTick1);
    _time_to_ticks(pTime2, &xTick2);
    
    return  _time_diff_seconds(&xTick1, &xTick2, &bOverflow);
}

uint_32 FTE_TIME_toString(TIME_STRUCT *pTime, char_ptr pBuff, uint_32 nBuffLen)
{
    DATE_STRUCT     xDate;
   
    _time_to_date (pTime, &xDate);
    return  snprintf(pBuff, nBuffLen, "%04d-%02d-%02d %02d:%02d:%02d", xDate.YEAR, xDate.MONTH, xDate.DAY, xDate.HOUR, xDate.MINUTE, xDate.SECOND);
}

uint_32 FTE_TIME_toTime(char_ptr pBuff, TIME_STRUCT *pTime)
{
    DATE_STRUCT xDate;
    char_ptr    pEnd, pStart = pBuff;
    
    pEnd = strchr(pStart, '-');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.YEAR = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, '-');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.MONTH = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ' ');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.DAY = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ':');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.HOUR = atoi(pStart);
    pStart = pEnd + 1;
    
    pEnd = strchr(pStart, ':');
    if (pEnd == NULL)
    {
        return  MQX_ERROR;
    }
    *pEnd = '\0';
    xDate.MINUTE = atoi(pStart);
    pStart = pEnd + 1;
    
    xDate.SECOND = atoi(pStart);

    _time_from_date (&xDate, pTime);
    
    return  MQX_OK;
}

uint_64     FTE_TIME_getMilliSeconds(void)
{
    TIME_STRUCT         xTime;

    _time_get(&xTime);
    
    return  xTime.SECONDS * 1000 + xTime.MILLISECONDS;
}

int_32  FTE_TIME_SHELL_cmd(int_32 argc, char_ptr argv[] )
{ /* Body */
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        uint_32 tmp;
        
        switch(argc)
        {
        case    1:
            { 
                TIME_STRUCT             xTime;
                char                    pBuff[30];

#if BSPCFG_ENABLE_RTCDEV                
                RTC_TIME_STRUCT         xRTC;
                
                _rtc_get_time (&xRTC);
                _rtc_time_to_mqx_time (&xRTC, &xTime);
#else                
                _time_get (&xTime);
#endif
                FTE_TIME_toString(&xTime, pBuff, sizeof(pBuff));
                printf("%s\n", pBuff);
            }
            break;
            
        case    2:
            {
                char            buff[15];
                DATE_STRUCT     xDate;
                TIME_STRUCT     xTime;
                
                if (!Shell_parse_number( argv[1], &tmp) || (strlen(argv[1]) != 14))
                {
                    print_usage = TRUE;
                }
                else
                {
                    strcpy(buff, argv[1]);
                    
                    xDate.MILLISEC= 0;
                    Shell_parse_number( &buff[12], &tmp);
                    if (tmp > 60)
                    {
                        print_usage = TRUE;
                    }
                    else
                    {
                        xDate.SECOND = tmp;
                        buff[12] = 0;
                        Shell_parse_number( &buff[10], &tmp);
                        if (tmp > 60)
                        {
                            print_usage = TRUE;
                        }
                        else
                        {
                            xDate.MINUTE = tmp;
                            buff[10] = 0;
                            Shell_parse_number( &buff[8], &tmp);
                            if (tmp > 23)
                            {
                                print_usage = TRUE;
                            }
                            else
                            {
                                xDate.HOUR = tmp;
                                buff[8] = 0;
                                Shell_parse_number( &buff[6], &tmp);
                                if (tmp > 31)
                                {
                                    print_usage = TRUE;
                                }
                                else
                                {
                                    xDate.DAY = tmp;
                                    buff[6] = 0;
                                    Shell_parse_number( &buff[4], &tmp);
                                    if (tmp > 12)
                                    {
                                        print_usage = TRUE;
                                    }
                                    else
                                    {
                                        xDate.MONTH = tmp;
                                         buff[4] = 0;
                                        Shell_parse_number( &buff[0], &tmp);
                                        if (tmp < 2010)
                                        {
                                            print_usage = TRUE;
                                        }
                                        else
                                        {
                                            xDate.YEAR = tmp;
                                         
                                            _time_from_date (&xDate, &xTime);
                                            _time_set(&xTime);
                                            //_rtc_init(RTC_INIT_FLAG_CLEAR | RTC_INIT_FLAG_ENABLE);
                                            _rtc_init(RTC_INIT_FLAG_ENABLE);
                                            if( _rtc_sync_with_mqx(FALSE) != MQX_OK )
                                            {
                                                printf("\nError synchronize time!\n");
                                            }
                                            
                                            FTE_SYS_reset();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }                
            }
            break;
            
        default:
            print_usage = TRUE;
        }
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [yyyymmddHHMMSS]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [yyyymmddHHMMSS]\n", argv[0]);
            printf("        yyyy - Year\n");
            printf("        mm   - Numeric month, a number from 1 to 12.\n");
            printf("        dd   - Day, a number from 1 to 31.\n");
            printf("        HH   - Hour, a number from 0 to 23.\n");
            printf("        MM   - Minutes, a number from 0 to 59.\n");
            printf("        SS   - Seconds, a number from 0 to 59.\n");
        }
    }
    return   return_code;
}

