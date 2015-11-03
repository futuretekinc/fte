#ifndef _FTE_IOEX_H__
#define _FTE_IOEX_H__

#define FTE_IOEX_DI_MAX  16

typedef struct FTE_IOEX_DI_STRUCT
{
    boolean     bValue;
    TIME_STRUCT xVOT;           // valid operation time
}   FTE_IOEX_DI, _PTR_ FTE_IOEX_DI_PTR;

typedef struct  FTE_IOEX_STRUCT
{
    boolean         bActivation;
    boolean         bChanged;
    _task_id        xTaskID;
    uint_32         ulObjectID;
    FTE_IOEX_DI     pDI[FTE_IOEX_DI_MAX];
}   FTE_IOEX, _PTR_ FTE_IOEX_PTR;

typedef struct  FTE_IOEX_DIO_CONFIG_STRUCT
{
    boolean     bActivation;
    boolean     bActiveLo;
}   FTE_IOEX_DIO_CONFIG, _PTR_ FTE_IOEX_DIO_CONFIG_PTR;

typedef struct  FTE_IOEX_CONFIG_STRUCT
{
    FTE_IOEX_DIO_CONFIG pDI[FTE_IOEX_DI_MAX];
}   FTE_IOEX_CONFIG, _PTR_ FTE_IOEX_CONFIG_PTR;

_mqx_uint FTE_IOEX_init(uint_32 nID);
void FTE_IOEX_task(uint_32 datas);

int_32 FTE_IOEX_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] );


uint_32     FTE_IOEX_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_IOEX_received(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_IOEX_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_IOEX_detach(FTE_OBJECT_PTR pObj);
uint_32     FTE_IOEX_get(FTE_OBJECT_PTR pObject, uint_32 ulIndex, FTE_VALUE_PTR pValue);

extern FTE_VALUE_TYPE  FTE_IOEX_valueTypes[];

#define FTE_IOEX_DESCRIPTOR  {\
        .nModel         = FTE_GUS_MODEL_IOEX,   \
        .pName          = "IOEX",               \
        .nFieldCount    = FTE_IOEX_DI_MAX,      \
        .pValueTypes    = FTE_IOEX_valueTypes,  \
        .f_attach       = FTE_IOEX_attach,      \
        .f_detach       = FTE_IOEX_detach,      \
        .f_get          = FTE_IOEX_get,         \
        .f_request      = FTE_IOEX_request,     \
        .f_received     = FTE_IOEX_received,    \
    }
#endif