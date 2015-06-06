#ifndef __FTE_LOG_H__
#define __FTE_LOG_H__

typedef struct _fte_log_struct
{
    uint_32         nID;
    FTE_VALUE       xValue;
    uint_32         nLevel;
}   FTE_LOG, _PTR_ FTE_LOG_PTR;

_mqx_uint   FTE_LOG_init(uint_32 nCount);
_mqx_uint   FTE_LOG_add(FTE_OBJECT_ID nID, FTE_VALUE_PTR pValue);
FTE_LOG_PTR FTE_LOG_getAt(uint_32 nID);

int         fte_log_count(void);

int_32      FTE_LOG_SHELL_cmd(int_32 argc, char_ptr argv[]);

#endif
