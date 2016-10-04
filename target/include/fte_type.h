#ifndef __FTE_TYPE_H__
#define __FTE_TYPE_H__

#include <mqx.h>

typedef _mqx_uint   FTE_RET;

typedef char        FTE_CHAR, _PTR_ FTE_CHAR_PTR;
typedef boolean     FTE_BOOL, _PTR_ FTE_BOOL_PTR;
typedef int_16      FTE_INT16, _PTR_ FTE_INT16_PTR;
typedef uint_16     FTE_UINT16, _PTR_ FTE_UINT16_PTR;
typedef int_32      FTE_INT32, _PTR_ FTE_INT32_PTR;
typedef uint_32     FTE_UINT32, _PTR_ FTE_UINT32_PTR;
typedef double      FTE_FLOAT, _PTR_ FTE_FLOAT_PTR;

#define MQX_NOT_ENOUGH_MEMORY       (MQX_ERROR_BASE|0x80)
#define MQX_NOT_SUPPORTED_FUNCTION  (MQX_ERROR_BASE|0x81)
#define MQX_INVALID_OBJECT          (MQX_ERROR_BASE|0x82)
#define MQX_ALREADY_EXISTS          (MQX_ERROR_BASE|0x83)


#define FTE_RET_OK                  (MQX_OK)
#define FTE_RET_ERROR               (MQX_ERROR)

#define FTE_RET_OBJECT_FULL             (MQX_ERROR_BASE|0x80)
#define FTE_RET_TASK_CREATION_FAILED    (MQX_ERROR_BASE|0x81)
#define FTE_RET_INSUFFICIENT_MEMORY     (MQX_ERROR_BASE|0x82)
#define FTE_RET_OBJECT_NOT_FOUND        (MQX_ERROR_BASE|0x83)
#define FTE_RET_INVALID_ID              (MQX_ERROR_BASE|0x84)

#endif
