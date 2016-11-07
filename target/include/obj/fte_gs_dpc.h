#ifndef _FTE_GS_DPC_HL_H__
#define _FTE_GS_DPC_HL_H__

#define FTE_GS_DPC_DEFAULT_UPDATE_INTERVAL          2000

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

FTE_BOOL    FTE_GS_DPC_HL_isExist(FTE_UINT32 ulUCSID);
FTE_UINT32  FTE_GS_DPC_HL_request(FTE_OBJECT_PTR pObj);
FTE_RET     FTE_GS_DPC_HL_received(FTE_OBJECT_PTR pObj);
FTE_RET     FTE_GS_DPC_HL_set(FTE_OBJECT_PTR pObject, FTE_UINT32 nIndex, FTE_VALUE_PTR pValue);

extern  
FTE_GUS_CONFIG FTE_GS_DPC_defaultConfig ;

extern  
FTE_VALUE_TYPE  FTE_GS_DPC_HL_valueTypes[];

extern  const 
FTE_GUS_MODEL_INFO    FTE_GS_DPC_GUSModelInfo;

#endif