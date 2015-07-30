#ifndef _FTE_FTLM_H__
#define _FTE_FTLM_H__

uint_32     FTE_FTLM_requestData(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_FTLM_receiveData(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_FTLM_set(FTE_OBJECT_PTR pObject, uint_32 nIndex, FTE_VALUE_PTR pValue);
_mqx_uint   FTE_FTLM_setConfig(FTE_OBJECT_PTR pDevice, char_ptr pJSON);
_mqx_uint   FTE_FTLM_getConfig(FTE_OBJECT_PTR pDevice, char_ptr pBuff, uint_32 ulBuffLen);

extern FTE_VALUE_TYPE  FTE_FTLM_valueTypes[];

#define FTE_FTLM_SENS  {\
        .nModel         = FTE_GUS_MODEL_FTLM,   \
        .pName          = "FTLM",               \
        .nFieldCount    = 9,                            \
        .pValueTypes    = FTE_FTLM_valueTypes,  \
        .f_request      = FTE_FTLM_requestData,\
        .f_received     = FTE_FTLM_receiveData,\
        .f_set          = FTE_FTLM_set,         \
        .f_set_config   = FTE_FTLM_setConfig, \
        .f_get_config   = FTE_FTLM_getConfig  \
    }

#endif