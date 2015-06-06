#ifndef _FTE_TRUEYES_H__
#define _FTE_TRUEYES_H__

#ifndef FTE_TRUEYES_AIRQ_INTERVAL           
#define FTE_TRUEYES_AIRQ_INTERVAL           5
#endif

#ifndef FTE_TRUEYES_AIRQ_EVENT_TYPE_ABOVE   
#define FTE_TRUEYES_AIRQ_EVENT_TYPE_ABOVE   FTE_EVENT_CONDITION_ABOVE
#endif

#ifndef FTE_TRUEYES_AIRQ_EVENT_UPPER_LIMIT  
#define FTE_TRUEYES_AIRQ_EVENT_UPPER_LIMIT  5
#endif

#ifndef FTE_TRUEYES_AIRQ_EVENT_LOWER_LIMIT  
#define FTE_TRUEYES_AIRQ_EVENT_LOWER_LIMIT  100
#endif

#ifndef FTE_TRUEYES_AIRQ_EVENT_THRESHOLD    
#define FTE_TRUEYES_AIRQ_EVENT_THRESHOLD    20
#endif

#ifndef FTE_TRUEYES_AIRQ_EVENT_DELAY        
#define FTE_TRUEYES_AIRQ_EVENT_DELAY        0
#endif

uint_32     FTE_AIRQ_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_AIRQ_received(FTE_OBJECT_PTR pObj);

extern  FTE_VALUE_TYPE  FTE_TRUEYES_AIRQ_valueTypes[];

#define FTE_TRUEYES_AIRQ_DESCRIPTOR  {\
        .nModel         = FTE_GUS_MODEL_TRUEYES_AIRQ,   \
        .pName          = "TRUEYES AIRQ",               \
        .nFieldCount    = 4,                            \
         .pValueTypes   = FTE_TRUEYES_AIRQ_valueTypes,  \
        .f_request      = FTE_AIRQ_request,     \
        .f_received     = FTE_AIRQ_received     \
    }

#endif