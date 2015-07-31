#ifndef __FTE_DB_H__
#define __FTE_DB_H__

_mqx_uint   FTE_DB_init(void);

_mqx_uint   FTE_DB_OBJ_create(FTE_OBJECT_ID xID);
_mqx_uint   FTE_DB_OBJ_destroy(FTE_OBJECT_ID xID);

uint_32     FTE_DB_OBJ_count(FTE_OBJECT_ID xID);
_mqx_uint   FTE_DB_OBJ_appendValue(FTE_OBJECT_ID xID);

int_32      FTE_DB_SHELL_cmd(int_32 nArgc, char_ptr pArgv[]);

#endif
