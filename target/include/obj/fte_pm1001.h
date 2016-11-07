#ifndef _FTE_PM1001_H__
#define _FTE_PM1001_H__

FTE_UINT32  FTE_PM1001_request(FTE_OBJECT_PTR pObj);
FTE_RET     FTE_PM1001_received(FTE_OBJECT_PTR pObj);

extern  
FTE_VALUE_TYPE   FTE_PM1001_valueTypes[];

extern  
const FTE_GUS_MODEL_INFO    FTE_PM1001_GUSModelInfo;

#endif