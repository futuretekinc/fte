#ifndef __FTE_TASCON_H__
#define __FTE_TASCON_H__

#define FTE_HEM12_FIELD_POWER           0 
#define FTE_HEM12_FIELD_VOLTAGE         1 
#define FTE_HEM12_FIELD_CURRENT         2 

typedef struct FTE_HEM12_06M_CONFIG_STRUCT
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nModel;
    uint_32             nUCSID;
    uint_32             nInterval;
    uint_8              pSensorID[6];
}   FTE_HEM12_06M_CONFIG, _PTR_ FTE_HEM12_06M_CONFIG_PTR;

typedef struct FTE_HEM12_06M_STATUS_STRUCT
{
    FTE_GUS_STATUS      xGUS;
    uint_32             nField;
}   FTE_HEM12_06M_STATUS, _PTR_ FTE_HEM12_06M_STATUS_PTR;

uint_32     FTE_TASCON_HEM12_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_TASCON_HEM12_received(FTE_OBJECT_PTR pObj);

extern  FTE_VALUE_TYPE  FTE_TASCON_HEM12_valueTypes[];

#define FTE_TASCON_HEM12_DESCRIPTOR  {\
        .nModel     = FTE_GUS_MODEL_TASCON_HEM12,   \
        .pName      = "TASCON HEM12",               \
        .nFieldCount= 1,                            \
        .pValueTypes= FTE_TASCON_HEM12_valueTypes,  \
        .f_request  = FTE_TASCON_HEM12_request,\
        .f_received = FTE_TASCON_HEM12_received\
    }

uint_32     FTE_TASCON_HEM12_06M_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_TASCON_HEM12_06M_received(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_TASCON_HEM12_06M_setConfig(FTE_OBJECT_PTR pDevice, char_ptr pJSON);
_mqx_uint   FTE_TASCON_HEM12_06M_getConfig(FTE_OBJECT_PTR pDevice, char_ptr pBuff, uint_32 ulBuffLen);

extern  FTE_VALUE_TYPE  FTE_TASCON_HEM12_06M_valueTypes[];

#define FTE_TASCON_HEM12_06M_DESCRIPTOR  {\
        .nModel     = FTE_GUS_MODEL_TASCON_HEM12_06M,   \
        .pName      = "TASCON HEM12-06M",               \
        .nFieldCount= 4,                                \
        .pValueTypes= FTE_TASCON_HEM12_06M_valueTypes,  \
        .f_request  = FTE_TASCON_HEM12_06M_request,     \
        .f_received = FTE_TASCON_HEM12_06M_received,    \
        .f_set_config = FTE_TASCON_HEM12_06M_setConfig, \
        .f_get_config = FTE_TASCON_HEM12_06M_getConfig, \
    }

int_32  FTE_TASCON_HEM12_SHELL_cmd(int_32 argc, char_ptr argv[]);

#endif
