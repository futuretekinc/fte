#ifndef _FTE_ELT_H__
#define _FTE_ELT_H__

#ifndef FTE_ELT_AQM100_INTERVAL
#define FTE_ELT_AQM100_INTERVAL             10
#endif

#ifndef FTE_ELT_AQM100_REPONSE_TIME         
#define FTE_ELT_AQM100_REPONSE_TIME         500
#endif
#ifndef FTE_ELT_AQM100_EVENT_TYPE_ABOVE     
#define FTE_ELT_AQM100_EVENT_TYPE_ABOVE     FTE_EVENT_CONDITION_ABOVE
#endif

#ifndef FTE_ELT_AQM100_EVENT_UPPER_LIMIT    
#define FTE_ELT_AQM100_EVENT_UPPER_LIMIT    1000
#endif

#ifndef FTE_ELT_AQM100_EVENT_LOWER_LIMIT    
#define FTE_ELT_AQM100_EVENT_LOWER_LIMIT    0
#endif

#ifndef FTE_ELT_AQM100_EVENT_THRESHOLD      
#define FTE_ELT_AQM100_EVENT_THRESHOLD      50
#endif

#ifndef FTE_ELT_AQM100_EVENT_DELAY          
#define FTE_ELT_AQM100_EVENT_DELAY          0
#endif

uint_32     fte_elt_aqm100_request_data(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_elt_aqm100_receive_data(FTE_OBJECT_PTR pObj);

extern  FTE_VALUE_TYPE  FTE_ELT_AQM100_valueTypes[];

#define FTE_ELT_AQM100_DESCRIPTOR  {\
        .nModel         = FTE_GUS_MODEL_ELT_AQM100,     \
        .pName          = "ELT AQM100",                 \
        .nMaxResponseTime=FTE_ELT_AQM100_REPONSE_TIME,  \
        .nFieldCount    = 4,                            \
         .pValueTypes   = FTE_ELT_AQM100_valueTypes,    \
        .f_request      = fte_elt_aqm100_request_data,  \
        .f_received     = fte_elt_aqm100_receive_data   \
    }


#endif