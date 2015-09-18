#ifndef __FTE_DB_H__
#define __FTE_DB_H__

typedef struct
{
    uint_32     xTime;
    uint_32     ulValue;
}   FTE_DB_VALUE, _PTR_ FTE_DB_VALUE_PTR;

_mqx_uint   FTE_DB_init(boolean bForce);

_mqx_uint   FTE_DB_OBJ_create(FTE_OBJECT_ID xID);
_mqx_uint   FTE_DB_OBJ_destroy(FTE_OBJECT_ID xID);
uint_32     FTE_DB_OBJ_count(FTE_OBJECT_ID xID);
_mqx_uint   FTE_DB_OBJ_appendValue(FTE_OBJECT_ID xID, FTE_DB_VALUE_PTR pValue);
_mqx_uint   FTE_DB_OBJ_getValues(FTE_OBJECT_ID xID, uint_32 ulIndex, FTE_DB_VALUE_PTR pValues, uint_32 ulCount);


int_32      FTE_DB_SHELL_cmd(int_32 nArgc, char_ptr pArgv[]);

#endif
