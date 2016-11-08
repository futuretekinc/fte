
/**HEADER********************************************************************
* 
* Copyright (c) 2014 FutureTek,Inc.;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: fte_p5_cgi.c$
* $Version : 1.0.0.0$
* $Date    : Jun-01-2014$
*
* Comments:
* 
*
*END************************************************************************/
#if FTE_EC1

#include <stdlib.h>
#include "fte_target.h"
#include "fte_net.h"
#include "fte_config.h"
#include "fte_object.h"
#include "httpsrv.h"
#include "fte_buff.h"
#include "fte_cgi.h"


_mqx_int cgi_request(HTTPSRV_CGI_REQ_STRUCT* param)
{
    FTE_CGI_QUERY_PTR       cgi_query = NULL;
    char_ptr                cmd, subcmd;
    char_ptr                pBuff = NULL;
    FTE_UINT32                 query_count = 0, nMaxLen = 0;
    FTE_UINT32                 length = 0, ret;
    
    nMaxLen = FTE_NET_HTTP_CGI_BUFF_SIZE;
    pBuff = (char_ptr)FTE_MEM_alloc(nMaxLen);
    if (pBuff == NULL)
    {
        goto error;
    }    
    
    if (param->request_method != HTTPSRV_REQ_GET)
    {
        goto    error;
    }

    query_count = fte_cgi_query_count(param->query_string);
    cgi_query = fte_cgi_query_alloc(query_count);
    if (cgi_query == NULL)
    {
        goto error;
    }
    
    fte_cgi_query_parser(param->query_string, cgi_query);
    
    cmd = fte_cgi_query_search(cgi_query, "cmd");
    if (cmd == NULL)
    {
        goto error;
    }
    
    if (strcmp(cmd, "get") == 0)
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
            goto error;
        }
        
        FTE_OBJECT_PTR pObj = FTE_OBJ_get(nOID);
        if (pObj == NULL)
        {
            goto error;
        }
            
        char_ptr pValue = fte_cgi_query_search(cgi_query, "value");
        switch(FTE_OBJ_TYPE(pObj))
        {
        case    FTE_OBJ_TYPE_DO:
            {
                if (strcmp(pValue, "on") == 0)
                {
                    ((FTE_DO_ACTION_PTR)pObj->pAction)->f_set(pObj, TRUE);
                }
                else if (strcmp(pValue, "off") == 0)
                {
                    ((FTE_DO_ACTION_PTR)pObj->pAction)->f_set(pObj, FALSE);
                }
                
                length = snprintf(pBuff, nMaxLen, "{\"ret\":\"success\",\"oid\":\"%08x\",\"value\":\"%s\",\"ctrl\":\"%s\"}",
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

                length = snprintf(pBuff, nMaxLen, "{\"ret\":\"success\",\"oid\":\"%08x\",\"value\":\"%s\",\"ctrl\":\"%s\"}",
                                  pObj->pConfig->nID, 
                                  (((FTE_DO_STATUS_PTR)pObj->pStatus)->nValue)?"open":"close",
                                  (((FTE_DO_STATUS_PTR)pObj->pStatus)->nValue)?"close":"open");
            }
            break;
            
        default:
            goto error;
        }
    }
    else
    {
        length = snprintf(pBuff, nMaxLen, "ERROR");
    }

    ret = fte_cgi_send_response(param->ses_handle, 200, pBuff, length);

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
    
    return ret;
}

#endif
