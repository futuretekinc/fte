#include <stdlib.h>
#include "fte_target.h"

#if  FTE_HTTPD_SUPPORTED

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
FTE_RET FTE_CGI_request(HTTPSRV_CGI_REQ_STRUCT _PTR_ pParam);

static 
FTE_RET FTE_CGI_requestGet(HTTPSRV_CGI_REQ_STRUCT _PTR_ pParam);

static 
FTE_RET FTE_CGI_requestPost(HTTPSRV_CGI_REQ_STRUCT _PTR_ pParam);

const 
HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = 
{
    { "request",        (HTTPSRV_CGI_CALLBACK_FN)FTE_CGI_request},
    { 0, 0 }    // DO NOT REMOVE - last item - end of table
};

FTE_RET FTE_CGI_QUERY_count
(   
    FTE_CHAR_PTR    pQuery,
    FTE_UINT32_PTR  pulCount
)
{
    ASSERT((pQuery != NULL) && (pulCount != NULL));
    
    FTE_UINT32 ulCount = 0;
    
    if (strlen(pQuery) != 0)
    {
        FTE_CHAR_PTR pEnd, pStart = pQuery;
        
        while(TRUE)
        {
            FTE_CHAR_PTR    pValue;
            
            pEnd = strchr(pStart, '&');
            pValue = strchr(pStart, '=');
            if (pValue != 0)
            {
                ulCount++;
            }
            
            if (pEnd == NULL)
            {
                break;
            }
            
            pStart = pEnd + 1;
        }
    }
    
    *pulCount = ulCount;
    
    return  FTE_RET_OK;
}

FTE_CGI_QUERY_PTR   FTE_CGI_QUERY_alloc
(
    FTE_UINT32  ulCount
)
{
    FTE_CGI_QUERY_PTR   pQuery;
   
    pQuery = (FTE_CGI_QUERY_PTR)FTE_MEM_allocZero(sizeof(FTE_CGI_QUERY) + sizeof(FTE_CGI_QUERY_TUPLE) * ulCount);
    if (pQuery != NULL)
    {
        pQuery->ulMaxCount = ulCount;
    }
   
    if (pQuery == NULL)
    {
        printf("Can't alloc CGI query buffer. Memory not enough\n");
    }
   return   pQuery;
}

void        FTE_CGI_QUERY_free
(
    FTE_CGI_QUERY_PTR   pQuery
)
{
    if (pQuery != NULL)
    {
        FTE_MEM_free(pQuery);
    }
}

FTE_RET    FTE_CGI_QUERY_parser
(
    FTE_CHAR_PTR        pString, 
    FTE_CGI_QUERY_PTR   pQuery
)
{
    pQuery->ulCount = 0;
    if (strlen(pString) == 0)
    {
        return  FTE_RET_OK;
    }
    
    FTE_CHAR_PTR pEnd, pStart = pString;
    while(TRUE)
    {
        FTE_CHAR_PTR    pName, pValue;
        
        pEnd = strchr(pStart, '&');
        if (pEnd != NULL)
        {
            *pEnd = '\0';
        }
        
        pName   = pStart;
        pValue  = strchr(pStart, '=');
        if (pValue != 0)
        {
            *pValue = '\0';
            pValue++;
            
            pQuery->pTuples[pQuery->ulCount].pName  = pName;            
            pQuery->pTuples[pQuery->ulCount].pValue = pValue;
            pQuery->ulCount++;
        }
        
        if (pEnd == NULL)
        {
            break;
        }
        else
        {
            pStart = pEnd + 1;
        }        
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_CGI_QUERY_get
(
    FTE_CGI_QUERY_PTR   pQuery, 
    FTE_CHAR_PTR        pName,
    FTE_CHAR_PTR _PTR_  ppValue
)
{
    ASSERT((pQuery != NULL) && (pName != NULL) && (ppValue != NULL));
    
    for(FTE_UINT32 i = 0 ; i < pQuery->ulCount ; i++)
    {
        if (strcmp(pName, pQuery->pTuples[i].pName) == 0)
        {
            *ppValue = pQuery->pTuples[i].pValue;
            return  FTE_RET_OK;
        }
    }
    
    *ppValue = NULL;
    
    return  FTE_RET_OBJECT_NOT_FOUND;
}

FTE_RET FTE_CGI_QUERY_getIP
(
    FTE_CGI_QUERY_PTR   pQuery, 
    FTE_CHAR_PTR        pName, 
    FTE_UINT32_PTR      pIP
)
{
    ASSERT(pQuery != NULL && pName != NULL && pIP != NULL);
    
    for(FTE_UINT32 i = 0 ; i < pQuery->ulCount ; i++)
    {
        if (strcmp(pName, pQuery->pTuples[i].pName) == 0)
        {
            return  FTE_strToIP(pQuery->pTuples[i].pValue, pIP);
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_CGI_QUERY_getUINT32
(
    FTE_CGI_QUERY_PTR   pQuery, 
    FTE_CHAR_PTR        pName, 
    FTE_UINT32_PTR      pValue
)
{
    ASSERT(pQuery != NULL && pName != NULL && pValue != NULL);
    
    for(FTE_UINT32 i = 0 ; i < pQuery->ulCount ; i++)
    {
        if (strcmp(pName, pQuery->pTuples[i].pName) == 0)
        {
            *pValue = atoi(pQuery->pTuples[i].pValue);
            return  FTE_RET_OK;
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_CGI_QUERY_getHEXNUM
(
    FTE_CGI_QUERY_PTR   pQuery, 
    FTE_CHAR_PTR        pName, 
    FTE_UINT32_PTR      pValue
)
{
    ASSERT(pQuery != NULL && pName != NULL && pValue != NULL);
    
    for(FTE_UINT32 i = 0 ; i < pQuery->ulCount ; i++)
    {
        if (strcmp(pName, pQuery->pTuples[i].pName) == 0)
        {
            *pValue = strtoul(pQuery->pTuples[i].pValue, NULL, 16);
            
            return  FTE_RET_OK;
        }
    }
    
    return  FTE_RET_ERROR;
}


FTE_RET FTE_CGI_sendResponse
(
    FTE_UINT32      ulHandle, 
    FTE_UINT32      ulStatusCode, 
    FTE_CHAR_PTR    pData, 
    FTE_UINT32      ulLen
)
{
    HTTPSRV_CGI_RES_STRUCT  xResponse = {.content_length = 0};
    
    xResponse.ses_handle     = ulHandle;
    xResponse.content_type   = HTTPSRV_CONTENT_TYPE_JSON;
    xResponse.status_code    = ulStatusCode;
    xResponse.data           = pData;
    xResponse.data_length    = ulLen;
    xResponse.content_length = xResponse.data_length;
    
    /* Send response */
    HTTPSRV_cgi_write(&xResponse);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_CGI_request
(
    HTTPSRV_CGI_REQ_STRUCT _PTR_ pParam
)
{
    if (pParam->request_method == HTTPSRV_REQ_GET)
    {
        return  FTE_CGI_requestGet(pParam);
    }
    else 
    {
        return  FTE_CGI_requestPost(pParam);
    }
}

FTE_RET FTE_CGI_requestGet
(
    HTTPSRV_CGI_REQ_STRUCT _PTR_ pParam
)
{
    FTE_RET             xRet;
    FTE_CGI_QUERY_PTR   pQuery = NULL;
    FTE_CHAR_PTR        pCmd, pSubCmd;
    FTE_CHAR_PTR        pBuff = NULL;
    FTE_UINT32          ulQueryCount = 0, nMaxLen = 0;
    FTE_UINT32          nLen  = 0;
    FTE_BOOL            bReboot = FALSE;
    FTE_JSON_OBJECT_PTR pJSONObject;
        
    
    CGI_TRACE("CALLED");
    
    xRet = FTE_CGI_QUERY_count(pParam->query_string, &ulQueryCount);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }
    
    pQuery = FTE_CGI_QUERY_alloc(ulQueryCount);
    if (pQuery == NULL)
    {
        CGI_ERROR("FTE_CGI_QUERY_alloc(%d) failed\n", ulQueryCount);
        xRet = FTE_RET_NOT_ENOUGH_MEMORY;
        goto error;
    }
    
    xRet = FTE_CGI_QUERY_parser(pParam->query_string, pQuery);
    if (xRet != FTE_RET_OK)
    {
        xRet = FTE_RET_INVALID_MSG;
        goto error;
    }

    pJSONObject = (FTE_JSON_OBJECT_PTR)FTE_JSON_VALUE_createObject(20);
    if (pJSONObject == NULL)
    {
        xRet = FTE_RET_NOT_ENOUGH_MEMORY;
        goto error;
    }
    
    xRet = FTE_CGI_QUERY_get(pQuery, "cmd", &pCmd);
    if (xRet != FTE_RET_OK)
    {
        CGI_ERROR("FTE_CGI_QUERY_get(cgi_query, cmd) not found\n");
        goto error;
    }
    
    if (strcmp(pCmd, "view") == 0)
    {
        FTE_PRODUCT_DESC const _PTR_    pProductDesc = FTE_getProductDescription();
        
        xRet = FTE_CGI_QUERY_get(pQuery, "page", &pSubCmd);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
        
        xRet = FTE_JSON_OBJECT_addStringPair(pJSONObject, "model", pProductDesc->pModel);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }

        xRet = FTE_JSON_OBJECT_addStringPair(pJSONObject, "type", pSubCmd);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }

        if (strcmp(pSubCmd, "status") == 0)
        {
            TIME_STRUCT                     xTime;
            
            _time_get(&xTime);

            xRet = FTE_JSON_OBJECT_addStringPair(pJSONObject, "title", "Status");
            if (xRet != FTE_RET_OK)
            {
                goto error;
            }

            xRet = FTE_JSON_OBJECT_addNumberPair(pJSONObject, "LUT", xTime.SECONDS);
            if (xRet != FTE_RET_OK)
            {
                goto error;
            }

            xRet = FTE_JSON_OBJECT_addNumberPair(pJSONObject, "RI", 5);
            if (xRet != FTE_RET_OK)
            {
                goto error;
            }

            xRet = FTE_JSON_OBJECT_addNumberPair(pJSONObject, "LUT", xTime.SECONDS);
            if (xRet != FTE_RET_OK)
            {
                goto error;
            }
        }
        else if (strcmp(pSubCmd, "config") == 0)
        {
            xRet = FTE_JSON_OBJECT_addStringPair(pJSONObject, "title", "Point Configurations");
            if (xRet != FTE_RET_OK)
            {
                goto error;
            }
        }
        else if (strcmp(pSubCmd, "system") == 0)
        {
            xRet = FTE_JSON_OBJECT_addStringPair(pJSONObject, "title", "System Configurations");
            if (xRet != FTE_RET_OK)
            {
                goto error;
            }
        }
                   
    }
    else if (strcmp(pCmd, "get") == 0)
    {
        xRet = FTE_CGI_QUERY_get(pQuery, "subcmd", &pSubCmd);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
                                
        if (strcmp(pSubCmd, "desc") == 0)
        {
        }
        else if (strcmp(pSubCmd, "value") == 0)
        {
            FTE_UINT32 nOID;
            
            if (FTE_CGI_QUERY_getHEXNUM(pQuery, "oid", &nOID) == FTE_RET_OK)
            {
            }
            else
            {                
            }
        }
    }
    else if (strcmp(pCmd, "ctrl") == 0)
    {
        FTE_UINT32 nOID;
        
        xRet = FTE_CGI_QUERY_getHEXNUM(pQuery, "oid", &nOID);
        if (xRet != FTE_RET_OK)
        {
            CGI_ERROR("FTE_CGI_QUERY_getHEXNUM\n");
            goto error;
        }
        
        FTE_OBJECT_PTR pObj = FTE_OBJ_get(nOID);
        if (pObj == NULL)
        {
            CGI_ERROR("FTE_OBJ_get\n");
            xRet = FTE_RET_OBJECT_NOT_FOUND;
            goto error;
        }
            
    }

    nMaxLen = FTE_JSON_VALUE_buffSize((FTE_JSON_VALUE_PTR)pJSONObject);
    if (nMaxLen != 0)
    {
        pBuff = (FTE_CHAR_PTR)FTE_MEM_allocZero(nMaxLen);
        if (pBuff == NULL)
        {
            CGI_ERROR("Not enough memory.[ Size = %d ]\n", nMaxLen);
            xRet = FTE_RET_NOT_ENOUGH_MEMORY;
            goto error;
        }    
        
        nLen = FTE_JSON_VALUE_snprint(pBuff, nMaxLen, (FTE_JSON_VALUE_PTR)pJSONObject);
        xRet = FTE_CGI_sendResponse(pParam->ses_handle, 200, pBuff, nLen);
    }
    

error:

    if (pQuery != NULL)
    {
        FTE_CGI_QUERY_free(pQuery);
        pQuery = NULL;
    }

    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
    }
    
    if (bReboot)
    {
        FTE_SYS_reset();
    }
    
    if (pJSONObject != NULL)
    {
        FTE_JSON_VALUE_destroy((FTE_JSON_VALUE_PTR)pJSONObject);
        pJSONObject = NULL;
    }
    
    return xRet;
}

FTE_RET FTE_CGI_requestPost
(
    HTTPSRV_CGI_REQ_STRUCT _PTR_ pParam
)
{
    FTE_RET             xRet;
    FTE_CGI_QUERY_PTR   pQuery = NULL;
    FTE_CHAR_PTR        pCmd;
    FTE_CHAR_PTR        pBuff = NULL;
    FTE_UINT32          ulQueryCount = 0, nMaxLen = 0;
    FTE_UINT32          nLen  = 0, ret;
    FTE_BOOL            bReboot = FALSE;
    FTE_CHAR            pField[32];
    FTE_CHAR_PTR        pValue;
    
    
    nMaxLen = FTE_NET_HTTP_CGI_BUFF_SIZE;
    pBuff = (FTE_CHAR_PTR)FTE_MEM_allocZero(nMaxLen);
    if (pBuff == NULL)
    {
        goto error;
    }    
    
    nLen = HTTPSRV_cgi_read(pParam->ses_handle, pBuff, nMaxLen);
    
    xRet = FTE_CGI_QUERY_count(pBuff, &ulQueryCount);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }
    
    pQuery = FTE_CGI_QUERY_alloc(ulQueryCount);
    if (pQuery == NULL)
    {
        goto error;
    }
    
    xRet = FTE_CGI_QUERY_parser(pBuff, pQuery);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }

    xRet  = FTE_CGI_QUERY_get(pQuery, "cmd", &pCmd);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }

    if (strcmp(pCmd, "config") == 0)
    {
        for(int nIdx = 0 ; nIdx < FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE); nIdx++)
        {
            FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_UNKNOWN, 0, nIdx, FALSE);
            
            sprintf(pField, "%08x_name", pObj->pConfig->xCommon.nID);
            xRet = FTE_CGI_QUERY_get(pQuery, pField, &pValue);
            if (xRet == FTE_RET_OK)
            {
                FTE_OBJ_setName(pObj, pValue, strlen(pValue));
            }
            
            sprintf(pField, "%08x_enable", pObj->pConfig->xCommon.nID);                                    
            xRet = FTE_CGI_QUERY_get(pQuery, pField, &pValue);
            if (xRet == FTE_RET_OK)
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
    else if (strcmp(pCmd, "system") == 0)
    {
        FTE_NET_CFG     net;
        FTE_CHAR_PTR    pNetType;
        FTE_CHAR_PTR    pItem;
        FTE_UINT32      ulIP, nNetMask, gateway, server1 = 0, server2 = 0;
        
        if ((FTE_CGI_QUERY_getIP(pQuery, "ip", &ulIP) != FTE_RET_OK) ||
            (FTE_CGI_QUERY_getIP(pQuery, "netmask", &nNetMask) != FTE_RET_OK) ||
            (FTE_CGI_QUERY_getIP(pQuery, "gateway", &gateway) != FTE_RET_OK))
        {
            goto error;
        }
        
        xRet = FTE_CGI_QUERY_get(pQuery, "server1", &pItem);
        if (xRet == FTE_RET_OK)
        {
            if (FTE_strToIP(pItem, &server1) != FTE_RET_OK)
            {
                goto error;
            }
        }

        xRet = FTE_CGI_QUERY_get(pQuery, "server2", &pItem);
        if (xRet == FTE_RET_OK)
        {
            if (FTE_strToIP(pItem, &server2) != FTE_RET_OK)
            {
                goto error;
            }
        }
        
        FTE_CFG_NET_copy(&net);
        
        xRet = FTE_CGI_QUERY_get(pQuery, "type", &pNetType);
        if ((xRet == FTE_RET_OK) && (strcmp(pNetType, "dhcp") == 0))
        {
            net.nType           = FTE_NET_TYPE_DHCP;
            net.xIPData.ip      = 0;
            net.xIPData.mask    = 0;
            net.xIPData.gateway = 0;
        }
        else
        {
            net.nType           = FTE_NET_TYPE_STATIC;
            net.xIPData.ip      = ulIP;
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
    
    xRet = FTE_CGI_sendResponse(pParam->ses_handle, 200, pBuff, nLen);

error:

    if (pQuery != NULL)
    {
        FTE_CGI_QUERY_free(pQuery);
        pQuery = NULL;
    }

    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
    }
    
    if (bReboot)
    {
        extern void __boot(void);
        __boot();
    }
    
    return ret;
}
            
#endif