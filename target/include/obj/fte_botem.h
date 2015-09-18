#ifndef _FTE_BOTEM_H__
#define _FTE_BOTEM_H__

uint_32     fte_botem_pn1500_request_data(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_botem_pn1500_receive_data(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_botem_pn1500_set(FTE_OBJECT_PTR pObject, uint_32 nIndex, FTE_VALUE_PTR pValue);
_mqx_uint   fte_botem_pn1500_set_multi(FTE_OBJECT_PTR pSelf, uint_32 nIndex, FTE_VALUE_PTR pValue);
_mqx_uint   fte_botem_pn1500_setConfig(FTE_OBJECT_PTR pDevice, char_ptr pJSON);
_mqx_uint   fte_botem_pn1500_getConfig(FTE_OBJECT_PTR pDevice, char_ptr pBuff, uint_32 ulBuffLen);

extern FTE_VALUE_TYPE  FTE_BOTEM_PN1500_valueTypes[];

#define FTE_BOTEM_PN1500_SENS  {\
        .nModel     = FTE_GUS_MODEL_BOTEM_PN1500,   \
        .pName      = "BOTEM PN1500",               \
        .nFieldCount= 3,                            \
        .pValueTypes= FTE_BOTEM_PN1500_valueTypes,  \
        .f_request  = fte_botem_pn1500_request_data,\
        .f_received = fte_botem_pn1500_receive_data,\
        .f_set      = fte_botem_pn1500_set,         \
        .f_set_config = fte_botem_pn1500_setConfig, \
        .f_get_config = fte_botem_pn1500_getConfig  \
    }

#endif