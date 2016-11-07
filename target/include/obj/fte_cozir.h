#ifndef _FTE_COZIR_H__
#define _FTE_COZIR_H__

#ifndef FTE_AX5000_INTERVAL                 
#define FTE_AX5000_INTERVAL                 5
#endif

#ifndef FTE_AX5000_RESPONSE_TIME            
#define FTE_AX5000_RESPONSE_TIME            1000
#endif

#ifndef FTE_AX5000_EVENT_TYPE_ABOVE         
#define FTE_AX5000_EVENT_TYPE_ABOVE         FTE_OBJ_EVENT_TYPE_ABOVE
#endif

#ifndef FTE_AX5000_EVENT_UPPER_LIMIT         
#define FTE_AX5000_EVENT_UPPER_LIMIT        1000
#endif

#ifndef FTE_AX5000_EVENT_LOWER_LIMIT         
#define FTE_AX5000_EVENT_LOWER_LIMIT        0
#endif

#ifndef FTE_AX5000_EVENT_THRESHOLD           
#define FTE_AX5000_EVENT_THRESHOLD          50
#endif

#ifndef FTE_AX5000_EVENT_DELAY               
#define FTE_AX5000_EVENT_DELAY              0
#endif

FTE_RET   FTE_COZIR_init(FTE_OBJECT_PTR pObj);
FTE_RET   FTE_COZIR_request(FTE_OBJECT_PTR pObj);
FTE_RET   FTE_COZIR_received(FTE_OBJECT_PTR pObj);

extern  
FTE_GUS_CONFIG FTE_COZIR_AX5000_defaultConfig;

extern  
FTE_VALUE_TYPE  FT_COZIR_valueTypes[];

extern  const 
FTE_GUS_MODEL_INFO    FTE_COZIR_AX5000_GUSModelInfo;


#endif