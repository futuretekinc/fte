#ifndef __FTE_CGI_H__
#define __FTE_CGI_H__

typedef struct
{
    char_ptr    pValue;
    uint_32     nOptions;
    struct  
    {
        char_ptr    pName;
        char_ptr    pValue;
    }   pOptions[10];
}   fte_input_select,   _PTR_ fte_input_select_ptr;

typedef struct  _fte_cgi_query_tuple_struct
{
    char_ptr    name;
    char_ptr    value;
}   FTE_CGI_QUERY_TUPLE, _PTR_ FTE_CGI_QUERY_TUPLE_PTR;

typedef struct  _fte_cgi_query_struct
{
    uint_32             max_count;
    uint_32             count;
    FTE_CGI_QUERY_TUPLE tuples[1];
}   FTE_CGI_QUERY,  _PTR_ FTE_CGI_QUERY_PTR;

uint_32             fte_cgi_query_count(char_ptr query_string);
FTE_CGI_QUERY_PTR   fte_cgi_query_alloc(uint_32 count);
void                fte_cgi_query_free(FTE_CGI_QUERY_PTR query);
_mqx_int            fte_cgi_query_parser(char_ptr query_string, FTE_CGI_QUERY_PTR query);
char_ptr            fte_cgi_query_search(FTE_CGI_QUERY_PTR query, char_ptr name);
_mqx_uint           fte_cgi_query_search_ip(FTE_CGI_QUERY_PTR query, char_ptr name, uint_32 *ip);
_mqx_uint           fte_cgi_query_search_uint32(FTE_CGI_QUERY_PTR query, char_ptr name, uint_32 *value);
_mqx_uint           fte_cgi_query_search_hexnum(FTE_CGI_QUERY_PTR query, char_ptr name, uint_32 *value);
_mqx_uint           fte_cgi_str_to_ip(char_ptr str, uint_32 *ip);


_mqx_int            fte_cgi_send_response(uint_32 handle, uint_32 status_code, char_ptr data, uint_32 length);


#endif

