#include <stdlib.h>
#include "fte_target.h"
#include "fte_net.h"
#include "fte_config.h"
#include "fte_object.h"
#include "httpsrv.h"
#include "fte_buff.h"
#include "fte_cgi.h"
#include "fte_json.h"

const char_ptr  xTableFieldTitles[] = 
{
    "OID",
    "OID",
    "NAME",
    "NAME",
    "VALUE",
    "VALUE",
    "STATUS",
    "STATUS",
    "ENABLE",
    "ENABLE",
    "S/N",
    "S/N",
    "CTRL",
    "CTRL",
    "INIT",
    "INIT"
};


uint_32 fte_print_json_object_start(char_ptr pBuff, uint_32 nBuffLen)
{
    return  snprintf(pBuff, nBuffLen, "{");
}
uint_32 fte_print_json_object_end(char_ptr pBuff, uint_32 nBuffLen)
{
    return  snprintf(pBuff, nBuffLen, "}");
}

uint_32 fte_print_json_comma(char_ptr pBuff, uint_32 nBuffLen)
{
    return  snprintf(pBuff, nBuffLen, ",");
}

uint_32 fte_print_json_array_touple(char_ptr pBuff, uint_32 nBuffLen, char_ptr pString, char_ptr pValue)
{
    return  snprintf(pBuff, nBuffLen, "[\"%s\",\"%s\"]", pString, pValue);
}

uint_32 fte_print_json_object_string(char_ptr pBuff, uint_32 nBuffLen, char_ptr pString, char_ptr pValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%s\"", pString, pValue);
}

uint_32 fte_print_json_object_hexnum(char_ptr pBuff, uint_32 nBuffLen, char_ptr pString, uint_32 nValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%08x\"", pString, nValue);
}

uint_32 fte_print_json_object_uint(char_ptr pBuff, uint_32 nBuffLen, char_ptr pString, uint_32 nValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%d\"", pString, nValue);
}

uint_32 fte_print_json_object_ip(char_ptr pBuff, uint_32 nBuffLen, char_ptr pString, uint_32 nValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%d.%d.%d.%d\"", pString, IPBYTES(nValue));
}

uint_32 fte_print_json_object_mac(char_ptr pBuff, uint_32 nBuffLen, char_ptr pString, _enet_address pMAC)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%02x:%02x:%02x:%02x:%02x:%02x\"", 
                       pString,
                       pMAC[0],pMAC[1],pMAC[2],
                       pMAC[3],pMAC[4],pMAC[5]);
}

uint_32 fte_print_json_object_groups(char_ptr pBuff, uint_32 nBuffLen, uint_32 nFields)
{
    uint_32     ulDescCount, nLen;
    boolean     bFirst = TRUE;
        
    ulDescCount = FTE_OBJ_DESC_count();
    
    nLen = snprintf(pBuff, nBuffLen, "\"groups\":[");        
    for(uint_32 i = 0 ; i < ulDescCount ; i++)
    {
        FTE_OBJECT_DESC_PTR pDesc;
       
        pDesc = FTE_OBJ_DESC_getAt(i);
    
        if (FTE_OBJ_count(pDesc->nType, FTE_OBJ_TYPE_MASK, FALSE) != 0)
        {
            if (!bFirst)
            {
                nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
            }
            
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "{");
            nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "name", pDesc->pName);
            nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "\"fields\":");
            nLen += fte_print_json_array_object_fields(&pBuff[nLen], nBuffLen - nLen, pDesc, nFields);
            nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "\"objects\":");
            nLen += fte_print_json_array_object(&pBuff[nLen], nBuffLen - nLen, pDesc->nType, nFields);
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
            bFirst = FALSE;
        }
    }
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");        
    
    return  nLen;
}
        
uint_32 fte_print_json_object_set_product_info(char_ptr pBuff, uint_32 nBuffLen)
{
    uint_32 nLen;
    FTE_PRODUCT_DESC const _PTR_    pProductDesc = fte_get_product_desc();
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();

    /* Calculate content length while saving it to buffer */
    nLen  = snprintf(pBuff, nBuffLen , "{\"descs\" : [");
    nLen += fte_print_json_object_start(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "title", "ID");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "value", FTE_SYS_getOIDString());
    nLen += fte_print_json_object_end(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_start(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "title", "Model");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "value", pProductDesc->pModel);
    nLen += fte_print_json_object_end(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_start(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"title", "Manufacturer");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"value", pProductDesc->pManufacturer);
    nLen += fte_print_json_object_end(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_start(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "title",  "H/W Version");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_ip(&pBuff[nLen], nBuffLen - nLen, "value",  pProductDesc->xVersion.hw);
    nLen += fte_print_json_object_end(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_start(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "title",  "S/W Version");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_ip(&pBuff[nLen], nBuffLen - nLen, "value",  pProductDesc->xVersion.sw);
    nLen += fte_print_json_object_end(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_start(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "title",  "IP Address");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_ip(&pBuff[nLen], nBuffLen - nLen, "value",  pCfgNet->xIPData.ip);
    nLen += fte_print_json_object_end(&pBuff[nLen], nBuffLen - nLen);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]}");
    
    return  nLen;
}

uint_32 fte_print_json_object_set_network(char_ptr pBuff, uint_32 nBuffLen)
{
    uint_32         nLen;
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();
    
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += fte_print_json_object_mac(&pBuff[nLen], nBuffLen - nLen, "mac", pCfgNet->xMACAddr);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "type", (pCfgNet->nType == FTE_NET_TYPE_DHCP)?"dhcp":"static");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_ip(&pBuff[nLen], nBuffLen - nLen, "ip", pCfgNet->xIPData.ip);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_ip(&pBuff[nLen], nBuffLen - nLen, "mask", pCfgNet->xIPData.mask);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_ip(&pBuff[nLen], nBuffLen - nLen, "gateway", pCfgNet->xIPData.gateway);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
    
    return  nLen;
}

uint_32 fte_print_json_object_set_console(char_ptr pBuff, uint_32 nBuffLen)
{
    uint_32 nLen;
    
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += fte_print_json_object_uint(&pBuff[nLen], nBuffLen - nLen, "baudrate", 115200);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_uint(&pBuff[nLen], nBuffLen - nLen, "databits", 8);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_uint(&pBuff[nLen], nBuffLen - nLen, "stopbits", 1);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "paritybits", "none");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "flowcontrol", "none");
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");

    return  nLen;
}

uint_32 fte_print_json_object_set_select(char_ptr pBuff, uint_32 nBuffLen, fte_input_select_ptr pSelect)
{
    uint_32 nLen;

    /* Calculate content length while saving it to buffer */
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"type", "select");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "{\"items\":[");
    for(int i = 0 ; i < pSelect->nOptions ; i++)
    {
        if (i != 0)
        {
            nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
        }
        nLen += fte_print_json_object_start(&pBuff[nLen], nBuffLen - nLen);
        nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"name", pSelect->pOptions[i].pName);
        nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
        nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"value", pSelect->pOptions[i].pValue);
        nLen += fte_print_json_object_end(&pBuff[nLen], nBuffLen - nLen);
    }
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);    
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"value", pSelect->pValue);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
    
    return  nLen;
    
}

uint_32 fte_print_json_object_set_object(char_ptr pBuff, uint_32 nBuffLen, FTE_OBJECT_PTR pObject, uint_32 nField)
{
    uint_32 nLen;
    uint_32 nTempBuff;
    char    pTempBuff[32];
    /* Calculate content length while saving it to buffer */
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += fte_print_json_object_hexnum(&pBuff[nLen], nBuffLen - nLen,"id", pObject->pConfig->nID);
    nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
    nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"name", pObject->pConfig->pName);
    if (nField & FTE_OBJ_FIELD_VALUE)
    {
        uint_32 nValue;
        
        pObject->pAction->f_get(pObject, &nValue, NULL);
        pObject->pAction->f_print_value(pObject, &nValue, pTempBuff, sizeof(pTempBuff));    
        nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
        nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"value", pTempBuff);
    }
        
    if (nField & FTE_OBJ_FIELD_STATUS)
    {
        nTempBuff = 0;
        memset(pTempBuff, 0, sizeof(pTempBuff));
        if (FTE_FLAG_IS_SET(pObject->pConfig->xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
        {
            nTempBuff += snprintf(pTempBuff, sizeof(pTempBuff), "E");
        }
        
        if (nTempBuff != 0)
        {
            nTempBuff += snprintf(&pTempBuff[nTempBuff], sizeof(pTempBuff) - nTempBuff, "-V");
        }
        else
        {
            nTempBuff += snprintf(&pTempBuff[nTempBuff], sizeof(pTempBuff) - nTempBuff, "V");
        }
        nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
        nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"status", pTempBuff);
    }    
 
    if (nField & FTE_OBJ_FIELD_ENABLE)
    {
        nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
        if (FTE_FLAG_IS_SET(pObject->pConfig->xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
        {
            nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "enable", "true");
        }
        else
        {
            nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen, "enable", "false");
        }
    }    
     
    if (nField & FTE_OBJ_FIELD_CTRL)
    {
        switch (FTE_OBJ_TYPE(pObject))
        {
        case    FTE_OBJ_TYPE_DO:
            nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
            if (((FTE_DO_STATUS_PTR)pObject->pStatus)->nValue == 0)
            {
                nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"cvalue", "ON");
            }
            else
            {
                nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"cvalue", "OFF");
            }
            break;
            
        case    FTE_OBJ_TYPE_RL:
            nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
            if (((FTE_RL_STATUS_PTR)pObject->pStatus)->nValue == 0)
            {
                nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"cvalue", "CLOSE");
            }
            else
            {
                nLen += fte_print_json_object_string(&pBuff[nLen], nBuffLen - nLen,"cvalue", "OPEN");
            }
            break;
        }
    }
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
    
    return  nLen;
}

uint_32 fte_print_json_array_object(char_ptr pBuff, uint_32 nBuffLen, uint_32 nType, uint_32 nField)
{
    uint_32 nLen = 0;
    uint_32 nObjCount = FTE_OBJ_count(nType, FTE_OBJ_TYPE_MASK, FALSE);

    if (nObjCount != 0)
    {
        FTE_OBJECT_PTR _PTR_ pObjectList = (FTE_OBJECT_PTR _PTR_)FTE_MEM_allocZero(sizeof(FTE_OBJECT_PTR) * nObjCount);
        if (pObjectList == NULL)
        {
            return  0;
        }
        
        nObjCount = FTE_OBJ_getList(nType, FTE_OBJ_TYPE_MASK, pObjectList, nObjCount);
        
        nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "[");
        for( int i = 0 ; i < nObjCount ; i++)
        {
            if (i != 0)
            {
                nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
            }
            nLen += fte_print_json_object_set_object(&pBuff[nLen], nBuffLen - nLen, pObjectList[i], nField);
        }
    
        nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");
        
        FTE_MEM_free(pObjectList);
    }
    
    return  nLen;
}


uint_32 fte_print_json_array_object_fields(char_ptr pBuff, uint_32 nBuffLen, FTE_OBJECT_DESC_PTR pDesc, uint_32 nFields)
{
    uint_32 nLen = 0;
    boolean bFirst = TRUE;
    
    nLen = snprintf(pBuff, nBuffLen, "[");
    for(uint_32 i = 0 ; (1 << i) != FTE_OBJ_FIELD_END ; i++)
    {
        if ((1 << i) & nFields & pDesc->xSupportedFields)
        {
            if (!bFirst)
            {
                nLen += fte_print_json_comma(&pBuff[nLen], nBuffLen - nLen);
            }
            else
            {                
                bFirst = FALSE;
            }
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "\"%s\"", xTableFieldTitles[i]);
        }
    }
    
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");
    
    return  nLen;
}

