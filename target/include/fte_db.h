#ifndef __FTE_DB_H__
#define __FTE_DB_H__

typedef struct
{
    FTE_UINT32     xTime;
    FTE_UINT32     ulValue;
}   FTE_DB_VALUE, _PTR_ FTE_DB_VALUE_PTR;

FTE_RET   FTE_DB_init(FTE_BOOL bForce);

FTE_RET     FTE_DB_OBJ_create(FTE_OBJECT_ID xID);
FTE_RET     FTE_DB_OBJ_destroy(FTE_OBJECT_ID xID);
FTE_UINT32  FTE_DB_OBJ_count(FTE_OBJECT_ID xID);
FTE_RET     FTE_DB_OBJ_appendValue(FTE_OBJECT_ID xID, FTE_DB_VALUE_PTR pValue);
FTE_RET     FTE_DB_OBJ_getValues(FTE_OBJECT_ID xID, FTE_UINT32 ulIndex, FTE_DB_VALUE_PTR pValues, FTE_UINT32 ulCount);


int_32      FTE_DB_SHELL_cmd(int_32 nArgc, char_ptr pArgv[]);

#endif
