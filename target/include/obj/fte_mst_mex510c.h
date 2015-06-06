#ifndef _FTE_MST_MEX_510C_H__
#define _FTE_MST_MEX_510C_H__

#define FTE_MST_MEX510C_INDEX_DEVICE_ID                 0
#define FTE_MST_MEX510C_INDEX_TEMPERATURE               1
#define FTE_MST_MEX510C_INDEX_SET_TEMPERATURE           2
#define FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION       3
#define FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION       4
#define FTE_MST_MEX510C_INDEX_TEMP_CALIBRATION          5
#define FTE_MST_MEX510C_INDEX_SET_HIGH_TEMP             6
#define FTE_MST_MEX510C_INDEX_SET_LOW_TEMP              7
#define FTE_MST_MEX510C_INDEX_OPERATION_DELAY           8
#define FTE_MST_MEX510C_INDEX_STOP_DELAY                9
#define FTE_MST_MEX510C_INDEX_COOL_OP_DELAY             10
#define FTE_MST_MEX510C_INDEX_HEAT_OP_DELAY             11
#define FTE_MST_MEX510C_INDEX_RESTART_DELAY             12
#define FTE_MST_MEX510C_INDEX_LP_ALERT_DELAY            13
#define FTE_MST_MEX510C_INDEX_DEFROST_CYCLE             14
#define FTE_MST_MEX510C_INDEX_DELAY_BEFORE_DEFROST_FAN  15
#define FTE_MST_MEX510C_INDEX_DELAY_AFTER_DEFROST_FAN   16
#define FTE_MST_MEX510C_INDEX_DEFROST_TIME              17
#define FTE_MST_MEX510C_INDEX_ALARM                     18
#define FTE_MST_MEX510C_INDEX_STATE_HEAT                19    
#define FTE_MST_MEX510C_INDEX_STATE_DHEAT               20
#define FTE_MST_MEX510C_INDEX_STATE_FAN                 21
#define FTE_MST_MEX510C_INDEX_STATE_SOLVALVE            22
#define FTE_MST_MEX510C_INDEX_STATE_COMP                23
#define FTE_MST_MEX510C_INDEX_STATE_OPERATION           24

#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE        25
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_A             26
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_B             27
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_C             28
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_D             29
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_E             30
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_F             31
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_G             32
#define FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H             33
#define FTE_MST_MEX510C_INDEX_ALARM_RESET               34
#define FTE_MST_MEX510C_INDEX_MAX                       34

_mqx_uint   FTE_MST_MEX510C_setConfig(FTE_OBJECT_PTR pDevice, char_ptr pJSON);
_mqx_uint   FTE_MST_MEX510C_getConfig(FTE_OBJECT_PTR pDevice, char_ptr pBuff, uint_32 ulBuffLen);

uint_32     fte_mst_mex510c_request_data(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_mst_mex510c_receive_data(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_mst_mex510c_set(FTE_OBJECT_PTR pObject, uint_32 nIndex, FTE_VALUE_PTR pValue);

extern  FTE_VALUE_TYPE  FTE_MST_MEX510C_valueTypes[];

#define FTE_MST_MEX510C_DESCRIPTOR  {\
        .nModel         = FTE_GUS_MODEL_MST_MEX510C,    \
        .pName          = "MEX-510C",                   \
        .nFieldCount    = FTE_MST_MEX510C_INDEX_MAX+1,  \
        .pValueTypes    = FTE_MST_MEX510C_valueTypes,   \
        .f_request      = fte_mst_mex510c_request_data, \
        .f_received     = fte_mst_mex510c_receive_data, \
        .f_set          = fte_mst_mex510c_set,          \
        .f_set_config = FTE_MST_MEX510C_setConfig,      \
        .f_get_config = FTE_MST_MEX510C_getConfig       \
    }
#endif