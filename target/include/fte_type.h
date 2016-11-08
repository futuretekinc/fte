#ifndef __FTE_TYPE_H__
#define __FTE_TYPE_H__

#include <mqx.h>

typedef _mqx_uint   FTE_RET;

#define FTE_VOID    void

typedef void _PTR_ FTE_VOID_PTR;
typedef char        FTE_CHAR,    _PTR_ FTE_CHAR_PTR;
typedef boolean     FTE_BOOL,   _PTR_ FTE_BOOL_PTR;
typedef int_8       FTE_INT8,   _PTR_ FTE_INT8_PTR;
typedef uint_8      FTE_UINT8,  _PTR_ FTE_UINT8_PTR;
typedef int_16      FTE_INT16,  _PTR_ FTE_INT16_PTR;
typedef uint_16     FTE_UINT16, _PTR_ FTE_UINT16_PTR;
typedef int_32      FTE_INT32,  _PTR_ FTE_INT32_PTR;
typedef uint_32     FTE_UINT32, _PTR_ FTE_UINT32_PTR;
typedef int_64      FTE_INT64,  _PTR_ FTE_INT64_PTR;
typedef uint_64     FTE_UINT64, _PTR_ FTE_UINT64_PTR;
typedef double      FTE_FLOAT,  _PTR_ FTE_FLOAT_PTR;

#define MQX_NOT_ENOUGH_MEMORY       (MQX_ERROR_BASE|0x80)
#define MQX_NOT_SUPPORTED_FUNCTION  (MQX_ERROR_BASE|0x81)
#define MQX_INVALID_OBJECT          (MQX_ERROR_BASE|0x82)
#define MQX_ALREADY_EXISTS          (MQX_ERROR_BASE|0x83)

#define MQX_LORAWAN_BUSY            (MQX_ERROR_BASE|0xA3)

#define FTE_RET_OK                  (MQX_OK)
#define FTE_RET_ERROR               (MQX_ERROR)

#define FTE_RET_INVALID_CHECKSUM        MQX_INVALID_CHECKSUM
#define FTE_RET_INVALID_PARAMETER       MQX_INVALID_PARAMETER
#define FTE_RET_INVALID_DEVICE          MQX_INVALID_DEVICE
#define FTE_RET_INVALID_SIZE            MQX_INVALID_SIZE

#define FTE_RET_OBJECT_FULL             (MQX_ERROR_BASE|0x80)
#define FTE_RET_NOT_SUPPORTED_FUNCTION  (MQX_ERROR_BASE|0x81)
#define FTE_RET_INVALID_OBJECT          (MQX_ERROR_BASE|0x82)
#define RTE_RET_ALREADY_EXISTS          (MQX_ERROR_BASE|0x83)
#define FTE_RET_TASK_CREATION_FAILED    (MQX_ERROR_BASE|0x84)
#define FTE_RET_INSUFFICIENT_MEMORY     (MQX_ERROR_BASE|0x85)
#define FTE_RET_NOT_ENOUGH_MEMORY       (MQX_ERROR_BASE|0x85)
#define FTE_RET_OBJECT_NOT_FOUND        (MQX_ERROR_BASE|0x86)
#define FTE_RET_INVALID_ID              (MQX_ERROR_BASE|0x87)
#define FTE_RET_INVALID_MODEL           (MQX_ERROR_BASE|0x88)
#define FTE_RET_ASSERT                  (MQX_ERROR_BASE|0x89)
#define FTE_RET_INVALID_ARGUMENT        (MQX_ERROR_BASE|0x8A)
#define FTE_RET_STATIC_OBJECT           (MQX_ERROR_BASE|0x8B)
#define FTE_RET_EXCEEDS_THE_RANGE       (MQX_ERROR_BASE|0x8C)
#define FTE_RET_RUNNING                 (MQX_ERROR_BASE|0x8D)
#define FTE_RET_PARENT_IS_STOPPED       (MQX_ERROR_BASE|0x8E)
#define FTE_RET_INITIALIZE_FAILED       (MQX_ERROR_BASE|0x8F)
#define FTE_RET_INVALID_MSG_FRAME       (MQX_ERROR_BASE|0x90)

#endif
