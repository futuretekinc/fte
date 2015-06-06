#ifndef _FTE_PM1001_H__
#define _FTE_PM1001_H__

uint_32     FTE_PM1001_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_PM1001_received(FTE_OBJECT_PTR pObj);

extern FTE_VALUE_TYPE   FTE_PM1001_valueTypes[];

#define FTE_PM1001_DESCRIPTOR  {\
        .nModel     = FTE_GUS_MODEL_PM1001,     \
        .pName      = "PM1001",                 \
        .nFieldCount= 1,                        \
        .pValueTypes= FTE_PM1001_valueTypes,    \
        .f_request  = FTE_PM1001_request,  \
        .f_received = FTE_PM1001_received \
    }


#endif