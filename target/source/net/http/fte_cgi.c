#include <stdlib.h>
#include "fte_target.h"
#include "fte_net.h"
#include "fte_config.h"
#include "fte_object.h"
#include "httpsrv.h"
#include "fte_buff.h"
#include "fte_cgi.h"
#include "fte_json.h"

#define CGI_TRACE(...)    TRACE(DEBUG_NET_CGI, __VA_ARGS__)
#define CGI_ERROR(...)    ERROR(__VA_ARGS__)

static 
FTE_RET _cgi_request(HTTPSRV_CGI_REQ_STRUCT* param);

static 
FTE_RET _cgi_request_get(HTTPSRV_CGI_REQ_STRUCT* param);

static 
FTE_RET _cgi_request_post(HTTPSRV_CGI_REQ_STRUCT* param);

const 
HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = 
{
    { "request",        _cgi_request},
    { 0, 0 }    // DO NOT REMOVE - last item - end of table
};

FTE_UINT32 fte_cgi_query_count
(   
    FTE_CHAR_PTR    pQuery
)
{
    FTE_UINT32 ulCount = 0;
    if (strlen(pQuery) != 0)
    {
        FTE_CHAR_PTR pQueryEnd, pQueryStart = pQuery;
        while(1)
        {
            FTE_CHAR_PTR    pValue;
            
            pQueryEnd = strchr(pQueryStart, '&');
            pValu  = strchr(pQueryStart, '=');
            if (pValue != 0)
            {
                ulCount++;
            }
            
            if (pQueryEnd == NULL)
            {
                break;
            }
            
            pQueryStart = pQueryEnd + 1;
        }
    }
    
    return  ulCount;
}

FTE_CGI_QUERY_PTR   fte_cgi_query_alloc
(
    FTE_UINT32  count
)
{
    FTE_CGI_QUERY_PTR   query;
   
    query = (FTE_CGI_QUERY_PTR)FTE_MEM_allocZero(sizeof(FTE_CGI_QUERY) + sizeof(FTE_CGI_QUERY_TUPLE) * count);
    if (query != NULL)
    {
        query->max_count = count;
    }
   
    if (query == NULL)
    {
        printf("Can't alloc CGI query buffer. Memory not enough\n");
    }
   return   query;
}

void        fte_cgi_query_free
(
    FTE_CGI_QUERY_PTR   query
)
{
    if (query != NULL)
    {
        FTE_MEM_free(query);
    }
}

FTE_RET    fte_cgi_query_parser
(
    FTE_CHAR_PTR    pQuery, 
    FTE_CGI_QUERY_PTR   query
)
{
    query->count = 0;
    if (strlen(pQuery) == 0)
    {
        return  MQX_OK;
    }
    
    FTE_CHAR_PTR pQueryEnd, pQueryStart = pQuery;
    while(1)
    {
        FTE_CHAR_PTR    _name, _value;
        
        pQueryEnd = strchr(pQueryStart, '&');
        if (pQueryEnd != NULL)
        {
            *pQueryEnd = '\0';
        }
        
        _name   = pQueryStart;
        _value  = strchr(pQueryStart, '=');
        if (_value != 0)
        {
            *_value = '\0';
            _value++;
            
            query->tuples[query->count].name    = _name;            
            query->tuples[query->count].value   = _value;
            query->count++;
        }
        
        if (pQueryEnd == NULL)
        {
            break;
        }
        else
        {
            pQueryStart = pQueryEnd + 1;
        }        
    }
    
    return  MQX_OK;
}

FTE_CHAR_PTR fte_cgi_query_search
(
    FTE_CGI_QUERY_PTR   query, 
    FTE_CHAR_PTR        name
)
{
    assert(query != NULL && name != NULL);
    
    FTE_UINT32 i;
    for(i = 0 ; i < query->count ; i++)
    {
        if (strcmp(name, query->tuples[i].name) == 0)
        {
            return  query->tuples[i].value;
        }
    }
    
    return  NULL;
}

FTE_RET fte_cgi_query_search_ip
(
    FTE_CGI_QUERY_PTR   query, 
    FTE_CHAR_PTR        name, 
    FTE_UINT32_PTR      ip
)
{
    assert(query != NULL && name != NULL);
    
    FTE_UINT32 i;
    for(i = 0 ; i < query->count ; i++)
    {
        if (strcmp(name, query->tuples[i].name) == 0)
        {
            return  fte_cgi_str_to_ip(query->tuples[i].value, ip);
        }
    }
    
    return  MQX_ERROR;
}

FTE_RET fte_cgi_query_search_uint32
(
    FTE_CGI_QUERY_PTR   query, 
    FTE_CHAR_PTR        name, 
    FTE_UINT32_PTR      value
)
{
    assert(query != NULL && name != NULL);
    
    FTE_UINT32 i;
    for(i = 0 ; i < query->count ; i++)
    {
        if (strcmp(name, query->tuples[i].name) == 0)
        {
            *value = atoi(query->tuples[i].value);
            return  MQX_OK;
        }
    }
    
    return  MQX_ERROR;
}

FTE_RET fte_cgi_query_search_hexnum
(
    FTE_CGI_QUERY_PTR   query, 
    FTE_CHAR_PTR        name, 
    FTE_UINT32_PTR      value
)
{
    assert(query != NULL && name != NULL);
    
    FTE_UINT32 i;
    for(i = 0 ; i < query->count ; i++)
    {
        if (strcmp(name, query->tuples[i].name) == 0)
        {
            *value = strtoul(query->tuples[i].value, NULL, 16);
            
            return  MQX_OK;
        }
    }
    
    return  MQX_ERROR;
}

FTE_RET fte_cgi_str_to_ip
(
    FTE_CHAR_PTR    str, 
    FTE_UINT32_PTR  ip
)
{
    FTE_UINT32     i;
    FTE_CHAR_PTR    byte[4] = {0, };
    char        buf[16];
    FTE_CHAR_PTR    ptr;
    
    if (strlen(str) >= sizeof(buf))
    {
        return  MQX_ERROR;        
    }
    
    strcpy(buf, str);
    ptr  = buf;
    
    if (strlen(ptr) != 0)
    {
        for(i = 0 ; i < 3 ; i++)
        {
            ptr = strchr(str, '.');
            if (ptr != NULL)
            {
                byte[i] = str;
                *ptr = '\0';
                str = ptr + 1;
            }
            else
            {
                return  MQX_ERROR;
            }
        }
        
        byte[3] = str;

        *ip = ((atoi(byte[0]) & 0xFF) << 24) | ((atoi(byte[1]) & 0xFF) << 16) | ((atoi(byte[2])  & 0xFF) << 8) | (atoi(byte[3]) & 0xFF);
    }
    else
    {
        *ip = 0;
    }
    
    return  MQX_OK;
}

FTE_RET fte_cgi_send_response
(
    FTE_UINT32  handle, 
    FTE_UINT32  status_code, 
    FTE_CHAR_PTR data, 
    FTE_UINT32  length
)
{
    HTTPSRV_CGI_RES_STRUCT  response = {.content_length = 0};
    
    response.ses_handle     = handle;
    response.content_type   = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code    = status_code;
    response.data           = data;
    response.data_length    = length;
    response.content_length = response.data_length;
    
    /* Send response */
    return  HTTPSRV_cgi_write(&response);
}

FTE_RET _cgi_request
(
    HTTPSRV_CGI_REQ_STRUCT* param
)
{
    if (param->request_method == HTTPSRV_REQ_GET)
    {
        return  _cgi_request_get(param);
    }
    else 
    {
        return  _cgi_request_post(param);
    }
}

FTE_RET _cgi_request_get
(
    HTTPSRV_CGI_REQ_STRUCT* param
)
{
    FTE_CGI_QUERY_PTR       cgi_query = NULL;
    FTE_CHAR_PTR                cmd, subcmd;
    FTE_CHAR_PTR                pBuff = NULL;
    FTE_UINT32                 query_count = 0, nMaxLen = 0;
    FTE_UINT32                 nLen  = 0, ret;
    boolean                 reboot = FALSE;
    
    CGI_TRACE("CALLED");
    nMaxLen = FTE_NET_HTTP_CGI_BUFF_SIZE;
    pBuff = (FTE_CHAR_PTR)FTE_MEM_allocZero(nMaxLen);
    if (pBuff == NULL)
    {
        CGI_ERROR("Not enough memory.[ Size = %d ]\n", nMaxLen);
        goto error;
    }    
    
    query_count = fte_cgi_query_count(param->pQuery);
    cgi_query = fte_cgi_query_alloc(query_count);
    if (cgi_query == NULL)
    {
        CGI_ERROR("fte_cgi_query_alloc(%d) failed\n", query_count);
        goto error;
    }
    
    fte_cgi_query_parser(param->pQuery, cgi_query);
    
    cmd = fte_cgi_query_search(cgi_query, "cmd");
    if (cmd == NULL)
    {
        CGI_ERROR("fte_cgi_query_search(cgi_query, cmd) not found\n");
        goto error;
    }
    
    if (strcmp(cmd, "view") == 0)
    {
        FTE_PRODUCT_DESC const _PTR_    pProductDesc = fte_get_product_desc();
        
        subcmd = fte_cgi_query_search(cgi_query, "page");
        
        nLen = snprintf(pBuff, nMaxLen, "{");
        nLen += fte_print_json_object_string(&pBuff[nLen], nMaxLen - nLen, "model", pProductDesc->pModel);
        nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
        nLen += fte_print_json_object_string(&pBuff[nLen], nMaxLen - nLen, "type", subcmd);
        
        if (strcmp(subcmd, "status") == 0)
        {
            TIME_STRUCT                     xTime;
            FTE_UINT32 nFields = FTE_OBJ_FIELD_ID | FTE_OBJ_FIELD_NAME | FTE_OBJ_FIELD_VALUE | FTE_OBJ_FIELD_STATUS | FTE_OBJ_FIELD_SN | FTE_OBJ_FIELD_CTRL;
            
            _time_get(&xTime);
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_object_string(&pBuff[nLen], nMaxLen - nLen, "title", "Status");
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_object_uint(&pBuff[nLen], nMaxLen - nLen, "LUT", xTime.SECONDS);
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_object_uint(&pBuff[nLen], nMaxLen - nLen, "RI", 5);
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"product_info\":");
            nLen += fte_print_json_object_set_product_info(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_object_groups(&pBuff[nLen], nMaxLen - nLen, nFields);        
        }
        else if (strcmp(subcmd, "config") == 0)
        {
            FTE_UINT32 nFields = FTE_OBJ_FIELD_ID | FTE_OBJ_FIELD_NAME | FTE_OBJ_FIELD_ENABLE | FTE_OBJ_FIELD_SN;
            
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_object_string(&pBuff[nLen], nMaxLen - nLen, "title", "Point Configurations");
            
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_object_groups(&pBuff[nLen], nMaxLen - nLen, nFields);        
        }
        else if (strcmp(subcmd, "system") == 0)
        {
            FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();

            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += fte_print_json_object_string(&pBuff[nLen], nMaxLen - nLen, "title", "System Configurations");
            
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"system\": {");
            
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"network\":");
            nLen += fte_print_json_object_set_network(&pBuff[nLen], nMaxLen - nLen);
            
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"servers\":[");
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"%d.%d.%d.%d\",", IPBYTES(pCfgNet->xSNMP.xTrap.pList[0]));
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"%d.%d.%d.%d\"]", IPBYTES(pCfgNet->xSNMP.xTrap.pList[1]));
#if FTE_CONSOLE_SUPPORTED
            nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"console\":");
            nLen += fte_print_json_object_set_console(&pBuff[nLen], nMaxLen - nLen);
#endif
            nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "}");            
        }
        nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "}");            
            

                   
    }
    else if (strcmp(cmd, "get") == 0)
    {
        subcmd = fte_cgi_query_search(cgi_query, "subcmd");
        if (strcmp(subcmd, "desc") == 0)
        {
            //nLen = fte_print_json_object_set_product_info(pBuff, nMaxLen);
        }
        else if (strcmp(subcmd, "value") == 0)
        {
            FTE_UINT32 nOID;
            
            if (fte_cgi_query_search_hexnum(cgi_query, "oid", &nOID) == MQX_OK)
            {
            }
            else
            {                
            }
        }
    }
    else if (strcmp(cmd, "ctrl") == 0)
    {
        FTE_UINT32 nOID;
        if (fte_cgi_query_search_hexnum(cgi_query, "oid", &nOID) != MQX_OK)
        {
            CGI_ERROR("fte_cgi_query_search_hexnum\n");
            goto error;
        }
        
        FTE_OBJECT_PTR pObj = FTE_OBJ_get(nOID);
        if (pObj == NULL)
        {
            CGI_ERROR("FTE_OBJ_get\n");
            goto error;
        }
            
        FTE_CHAR_PTR pValue = fte_cgi_query_search(cgi_query, "value");
        switch(FTE_OBJ_TYPE(pObj))
        {
        case    FTE_OBJ_TYPE_DO:
            {
                if (strcmp(pValue, "on") == 0)
                {
                    FTE_VALUE_setDIO(pObj->pStatus->pValue, TRUE);
                }
                else if (strcmp(pValue, "off") == 0)
                {
                    FTE_VALUE_setDIO(pObj->pStatus->pValue, FALSE);
                }
                
                nLen = snprintf(pBuff, nMaxLen, "{\"ret\":\"success\",\"oid\":\"%08x\",\"value\":\"%s\",\"ctrl\":\"%s\"}",
                                  pObj->pConfig->nID, 
                                  (((FTE_DO_STATUS_PTR)pObj->pStatus)->nValue)?"on":"off",
                                  (((FTE_DO_STATUS_PTR)pObj->pStatus)->nValue)?"off":"on");
            }
            break;
            
        case    FTE_OBJ_TYPE_RL:
            {
                if (strcmp(pValue, "close") == 0)
                {
                    ((FTE_DO_ACTION_PTR)pObj->pAction)->f_set(pObj, TRUE);
                }
                else if (strcmp(pValue, "open") == 0)
                {
                    ((FTE_DO_ACTION_PTR)pObj->pAction)->f_set(pObj, FALSE);
                }

                nLen = snprintf(pBuff, nMaxLen, "{\"ret\":\"success\",\"oid\":\"%08x\",\"value\":\"%s\",\"ctrl\":\"%s\"}",
                                  pObj->pConfig->nID, 
                                  (((FTE_DO_STATUS_PTR)pObj->pStatus)->nValue)?"open":"close",
                                  (((FTE_DO_STATUS_PTR)pObj->pStatus)->nValue)?"close":"open");
            }
            break;
            
        default:
            CGI_ERROR("Not supported object[ OBJ_TYPE = %08x]\n", FTE_OBJ_TYPE(pObj));
            goto error;
        }
    }
    else if (strcmp(cmd, "update") == 0)
    {
        TIME_STRUCT xCurrentTime;
        TIME_STRUCT xLastUpdatedTime = { .SECONDS = 0, .MILLISECONDS = 0};
        
        if (fte_cgi_query_search_uint32(cgi_query, "lut", &xLastUpdatedTime.SECONDS) != MQX_OK)
        {
            CGI_ERROR("fte_cgi_query_search_uint32\n");
            goto error;
        }
        
        _time_get(&xCurrentTime);
        
        nLen = snprintf(pBuff, nMaxLen, "{");
        nLen += fte_print_json_object_uint(&pBuff[nLen], nMaxLen - nLen, "LUT", xCurrentTime.SECONDS);
        nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
        nLen += snprintf(&pBuff[nLen], nMaxLen - nLen, "\"objects\":[");        
        
        boolean bFirst = TRUE;
        FTE_UINT32 nCount = FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE);
        if (nCount != 0)
        {
            FTE_OBJECT_PTR _PTR_ pObjList = (FTE_OBJECT_PTR _PTR_)FTE_MEM_allocZero(sizeof(FTE_OBJECT_PTR) * nCount);
            if (pObjList != NULL)
            {
                nCount = FTE_OBJ_getList(FTE_OBJ_TYPE_UNKNOWN, 0, pObjList, nCount);
                for( int i = 0 ; i < nCount ; i++)
                {
                    if (!bFirst)
                    {
                        nLen += fte_print_json_comma(&pBuff[nLen], nMaxLen - nLen);
                    }

                    nLen += fte_print_json_object_set_object(&pBuff[nLen], nMaxLen - nLen, pObjList[i], FTE_OBJ_FIELD_VALUE | FTE_OBJ_FIELD_STATUS);                
                    bFirst = FALSE;
                }
            }
            
            FTE_MEM_free(pObjList);
        }
        
        nLen += snprintf(&pBuff[nLen], nMaxLen - nLen,"]}");
    }    
    else if (strcmp(cmd, "factory_reset") == 0)
    {
        FTE_CFG_clear(&FTE_CFG_desc);
        reboot = TRUE;
        nLen = sprintf(pBuff, "SUCCESS");               
    }
    else if (strcmp(cmd, "reboot") == 0)
    {
        reboot = TRUE;
        nLen = sprintf(pBuff, "SUCCESS");               
    }
    else
    {
        nLen = snprintf(pBuff, nMaxLen, "ERROR");
    }
 
    ret = fte_cgi_send_response(param->ses_handle, 200, pBuff, nLen);

error:

    if (cgi_query != NULL)
    {
        fte_cgi_query_free(cgi_query);
        cgi_query = NULL;
    }

    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
    }
    
    if (reboot)
    {
        fte_system_reset();
    }
    
    return ret;
}


FTE_RET _cgi_request_post(HTTPSRV_CGI_REQ_STRUCT* param)
{
    FTE_CGI_QUERY_PTR       cgi_query = NULL;
    FTE_CHAR_PTR                cmd;
    FTE_CHAR_PTR                pBuff = NULL;
    FTE_UINT32                 query_count = 0, nMaxLen = 0;
    FTE_UINT32                 nLen  = 0, ret;
    boolean                 reboot = FALSE;
    char                    pField[32];
    FTE_CHAR_PTR                pValue;
    
    
    nMaxLen = FTE_NET_HTTP_CGI_BUFF_SIZE;
    pBuff = (FTE_CHAR_PTR)FTE_MEM_allocZero(nMaxLen);
    if (pBuff == NULL)
    {
        goto error;
    }    
    
    nLen = HTTPSRV_cgi_read(param->ses_handle, pBuff, nMaxLen);
    
    query_count = fte_cgi_query_count(pBuff);
    cgi_query = fte_cgi_query_alloc(query_count);
    if (cgi_query == NULL)
    {
        goto error;
    }
    fte_cgi_query_parser(pBuff, cgi_query);

    cmd = fte_cgi_query_search(cgi_query, "cmd");
    if (cmd == NULL)
    {
        goto error;
    }

    if (strcmp(cmd, "config") == 0)
    {
        for(int nIdx = 0 ; nIdx < FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE); nIdx++)
        {
            FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_UNKNOWN, 0, nIdx, FALSE);
            
            sprintf(pField, "%08x_name", pObj->pConfig->nID);
            pValue = fte_cgi_query_search(cgi_query, pField);
            if (pValue != NULL)
            {
                FTE_OBJ_setName(pObj, pValue, strlen(pValue));
            }
            
            sprintf(pField, "%08x_enable", pObj->pConfig->nID);                                    
            pValue = fte_cgi_query_search(cgi_query, pField);
            if (pValue != NULL)
            {
                if (strcmp(pValue, "true") == 0) 
                {
                    FTE_OBJ_activate(pObj, TRUE);
                }
                else if (strcmp(pValue, "false") == 0) 
                {
                    FTE_OBJ_activate(pObj, FALSE);
                }
            }
        }        

        nLen = snprintf(pBuff, nMaxLen, "SUCCESS");
    }
    else if (strcmp(cmd, "system") == 0)
    {
        FTE_NET_CFG net;
        FTE_CHAR_PTR    pNetType;
        FTE_UINT32 ip, nNetMask, gateway, server1 = 0, server2 = 0;
        
        if ((fte_cgi_query_search_ip(cgi_query, "ip", &ip) != MQX_OK) ||
            (fte_cgi_query_search_ip(cgi_query, "netmask", &nNetMask) != MQX_OK) ||
            (fte_cgi_query_search_ip(cgi_query, "gateway", &gateway) != MQX_OK))
        {
            goto error;
        }
        
        FTE_CHAR_PTR item = fte_cgi_query_search(cgi_query, "server1");
        if (item != NULL)
        {
            if (fte_cgi_str_to_ip(item, &server1) != MQX_OK)
            {
                goto error;
            }
        }

        item = fte_cgi_query_search(cgi_query, "server2");
        if (item != NULL)
        {
            if (fte_cgi_str_to_ip(item, &server2) != MQX_OK)
            {
                goto error;
            }
        }
        
        FTE_CFG_NET_copy(&net);
        
        pNetType = fte_cgi_query_search(cgi_query, "type");
        if (pNetType != NULL && (strcmp(pNetType, "dhcp") == 0))
        {
            net.nType           = FTE_NET_TYPE_DHCP;
            net.xIPData.ip      = 0;
            net.xIPData.mask    = 0;
            net.xIPData.gateway = 0;
        }
        else
        {
            net.nType           = FTE_NET_TYPE_STATIC;
            net.xIPData.ip      = ip;
            net.xIPData.mask    = nNetMask;
            net.xIPData.gateway = gateway;
        }
        net.xSNMP.xTrap.ulCount = 0;
        if (server1 != 0)
        {
            net.xSNMP.xTrap.pList[net.xSNMP.xTrap.ulCount++] = server1;
        }
            
        if (server2 != 0)
        {
            net.xSNMP.xTrap.pList[net.xSNMP.xTrap.ulCount++] = server2;
        }
            
        FTE_CFG_NET_set(&net);
        FTE_CFG_save(FALSE);
        nLen = sprintf(pBuff, "SUCCESS");    
    }
    ret = fte_cgi_send_response(param->ses_handle, 200, pBuff, nLen);

error:

    if (cgi_query != NULL)
    {
        fte_cgi_query_free(cgi_query);
        cgi_query = NULL;
    }

    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
    }
    
    if (reboot)
    {
        extern void __boot(void);
        __boot();
    }
    
    return ret;
}
