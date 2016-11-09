#ifndef __FTE_CGI_H__
#define __FTE_CGI_H__

typedef struct
{
    FTE_CHAR_PTR    pValue;
    FTE_UINT32      nOptions;
    struct  
    {
        FTE_CHAR_PTR    pName;
        FTE_CHAR_PTR    pValue;
    }   pOptions[10];
}   fte_input_select,   _PTR_ fte_input_select_ptr;

typedef struct  FTE_CGI_QUERY_TUPLE_STRUCT
{
    FTE_CHAR_PTR    pName;
    FTE_CHAR_PTR    pValue;
}   FTE_CGI_QUERY_TUPLE, _PTR_ FTE_CGI_QUERY_TUPLE_PTR;

typedef struct  FTE_CGI_QUERY_STRUCT
{
    FTE_UINT32              ulMaxCount;
    FTE_UINT32              ulCount;
    FTE_CGI_QUERY_TUPLE     pTuples[1];
}   FTE_CGI_QUERY,  _PTR_ FTE_CGI_QUERY_PTR;

FTE_CGI_QUERY_PTR   FTE_CGI_QUERY_alloc(FTE_UINT32  ulCount);
void        FTE_CGI_QUERY_free(FTE_CGI_QUERY_PTR query);

FTE_RET FTE_CGI_QUERY_count(FTE_CHAR_PTR    pQuery, FTE_UINT32_PTR  pulCount);
FTE_RET FTE_CGI_QUERY_parser(FTE_CHAR_PTR pString, FTE_CGI_QUERY_PTR query);
FTE_RET FTE_CGI_QUERY_get(FTE_CGI_QUERY_PTR query, FTE_CHAR_PTR pName, FTE_CHAR_PTR _PTR_ ppValue);
FTE_RET FTE_CGI_QUERY_getIP(FTE_CGI_QUERY_PTR query, FTE_CHAR_PTR pName, FTE_UINT32 *ip);
FTE_RET FTE_CGI_QUERY_getUINT32(FTE_CGI_QUERY_PTR query, FTE_CHAR_PTR name, FTE_UINT32 *value);
FTE_RET FTE_CGI_QUERY_getHEXNUM(FTE_CGI_QUERY_PTR query, FTE_CHAR_PTR name, FTE_UINT32 *value);

FTE_RET     FTE_CGI_sendResponse(FTE_UINT32 handle, FTE_UINT32 status_code, FTE_CHAR_PTR data, FTE_UINT32 length);


#endif

