#ifndef _FTE_FTLM_H__
#define _FTE_BOTEM_H__

uint_32     fte_ftlm_request_data(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_ftlm_receive_data(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_ftlm_set(FTE_OBJECT_PTR pObject, uint_32 nIndex, FTE_VALUE_PTR pValue);
_mqx_uint   fte_ftlm_setConfig(FTE_OBJECT_PTR pDevice, char_ptr pJSON);
_mqx_uint   fte_ftlm_getConfig(FTE_OBJECT_PTR pDevice, char_ptr pBuff, uint_32 ulBuffLen);

extern FTE_VALUE_TYPE  FTE_FTLM_valueTypes[];

#define FTE_FTLM_SENS  {\
        .nModel         = FTE_GUS_MODEL_FTLM,   \
        .pName          = "FTLM",               \
        .nFieldCount    = 27,                            \
        .pValueTypes    = FTE_FTLM_valueTypes,  \
        .f_request      = fte_ftlm_request_data,\
        .f_received     = fte_ftlm_receive_data,\
        .f_set          = fte_ftlm_set,         \
        .f_set_config   = fte_ftlm_setConfig, \
        .f_get_config   = fte_ftlm_getConfig  \
    }

#endif