#ifndef _FTE_GS_DPC_HL_H__
#define _FTE_GS_DPC_HL_H__

#define FTE_GS_DPC_HL_INDEX_VERSION                 0
#define FTE_GS_DPC_HL_INDEX_MODEL0                  1
#define FTE_GS_DPC_HL_INDEX_ID                      2
#define FTE_GS_DPC_HL_INDEX_HPS_STATE               3
#define FTE_GS_DPC_HL_INDEX_LPS_STATE               4
#define FTE_GS_DPC_HL_INDEX_HPC_STATE               5
#define FTE_GS_DPC_HL_INDEX_HPS_ERROR               6
#define FTE_GS_DPC_HL_INDEX_LPS_ERROR               7
#define FTE_GS_DPC_HL_INDEX_HP                      8
#define FTE_GS_DPC_HL_INDEX_LP                      9
#define FTE_GS_DPC_HL_INDEX_HP_TEMP                 10
#define FTE_GS_DPC_HL_INDEX_LP_TEMP                 11

#define FTE_GS_DPC_HL_INDEX_MAX                     11

boolean     FTE_GS_DPC_HL_isExist(uint_32 ulUCSID);
uint_32     FTE_GS_DPC_HL_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_GS_DPC_HL_received(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_GS_DPC_HL_set(FTE_OBJECT_PTR pObject, uint_32 nIndex, FTE_VALUE_PTR pValue);

extern  FTE_VALUE_TYPE  FTE_GS_DPC_HL_valueTypes[];

#define FTE_GS_DPC_HL_DESCRIPTOR  {\
        .nModel     = FTE_GUS_MODEL_GS_DPC_HL,  \
        .pName      = "DPC-HL",                 \
        .nFieldCount= FTE_GS_DPC_HL_INDEX_MAX+1,\
        .pValueTypes= FTE_GS_DPC_HL_valueTypes, \
        .f_request  = FTE_GS_DPC_HL_request,    \
        .f_received = FTE_GS_DPC_HL_received,   \
        .f_set      = FTE_GS_DPC_HL_set         \
    }
#endif