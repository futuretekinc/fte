#ifndef _FTE_SOHA_H__
#define _FTE_SOHA_H__


#ifndef FTE_SH_MV250_INTERVAL               
#define FTE_SH_MV250_INTERVAL               5
#endif

#ifndef FTE_SH_MV250_RESPONSE_TIME            
#define FTE_SH_MV250_RESPONSE_TIME          1000
#endif

#ifndef FTE_SH_MV250_EVENT_TYPE_ABOVE       
#define FTE_SH_MV250_EVENT_TYPE_ABOVE       FTE_EVENT_CONDITION_ABOVE
#endif

#ifndef FTE_SH_MV250_EVENT_UPPER_LIMIT      
#define FTE_SH_MV250_EVENT_UPPER_LIMIT      1000
#endif

#ifndef FTE_SH_MV250_EVENT_LOWER_LIMIT      
#define FTE_SH_MV250_EVENT_LOWER_LIMIT      0
#endif

#ifndef FTE_SH_MV250_EVENT_THRESHOLD        
#define FTE_SH_MV250_EVENT_THRESHOLD        50
#endif

#ifndef FTE_SH_MV250_EVENT_DELAY            
#define FTE_SH_MV250_EVENT_DELAY            0
#endif

#define FTE_SH_MV250_FRAME_LENGTH           24

typedef struct
{
    uint_8  uiSTX;
    uint_8  uiID;
    uint_8  uiReserved1;
    uint_8  puiCO2[4];
    uint_8  uiReserved2;
    uint_8  puiTemp[5];
    uint_8  uiReserved3;
    uint_8  puiHumi[4];
    uint_8  uiReserved4;
    uint_8  uiSHT;
    uint_8  uiMode;
    uint_8  uiCS;
    uint_8  uiCR;
    uint_8  uiLF;    
}   FTE_SH_MV250_FRAME, _PTR_ FTE_SH_MV250_FRAME_PTR;

_mqx_uint   FTE_SOHA_init(FTE_OBJECT_PTR pObj);
uint_32     FTE_SOHA_request(FTE_OBJECT_PTR pObj);
_mqx_uint   FTE_SOHA_received(FTE_OBJECT_PTR pObj);

extern  FTE_VALUE_TYPE  FTE_SOHA_MV250_valueTypes[];

#define FTE_SH_MV250_DESCRIPTOR  {                  \
        .nModel         = FTE_GUS_MODEL_SH_MV250,   \
        .pName          = "SOHA MV250",             \
        .nMaxResponseTime=FTE_SH_MV250_RESPONSE_TIME,\
        .nFieldCount    = 3,                        \
        .pValueTypes    = FTE_SOHA_MV250_valueTypes,\
        .f_init         = FTE_SOHA_init,            \
        .f_request      = FTE_SOHA_request,         \
        .f_received     = FTE_SOHA_received         \
    }


#endif