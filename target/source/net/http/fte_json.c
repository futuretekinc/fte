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

const 
FTE_CHAR_PTR  xTableFieldTitles[] = 
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


FTE_UINT32 FTE_JSON_printObjectBegin
(
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      nBuffLen
)
{
    return  snprintf(pBuff, nBuffLen, "{");
}
FTE_UINT32 FTE_JSON_printObjectEnd(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen)
{
    return  snprintf(pBuff, nBuffLen, "}");
}

FTE_UINT32 FTE_JSON_printComma(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen)
{
    return  snprintf(pBuff, nBuffLen, ",");
}

FTE_UINT32 FTE_JSON_printArrayTouple(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, FTE_CHAR_PTR pString, FTE_CHAR_PTR pValue)
{
    return  snprintf(pBuff, nBuffLen, "[\"%s\",\"%s\"]", pString, pValue);
}

FTE_UINT32 FTE_JSON_printObjectSTRING(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, FTE_CHAR_PTR pString, FTE_CHAR_PTR pValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%s\"", pString, pValue);
}

FTE_UINT32 FTE_JSON_printObjectHEX(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, FTE_CHAR_PTR pString, FTE_UINT32 nValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%08x\"", pString, nValue);
}

FTE_UINT32 FTE_JSON_printObjectUINT(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, FTE_CHAR_PTR pString, FTE_UINT32 nValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%d\"", pString, nValue);
}

FTE_UINT32 FTE_JSON_printObjectIP(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, FTE_CHAR_PTR pString, FTE_UINT32 nValue)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%d.%d.%d.%d\"", pString, IPBYTES(nValue));
}

FTE_UINT32 FTE_JSON_printObjectMAC(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, FTE_CHAR_PTR pString, _enet_address pMAC)
{
    return  snprintf(pBuff, nBuffLen, "\"%s\":\"%02x:%02x:%02x:%02x:%02x:%02x\"", 
                       pString,
                       pMAC[0],pMAC[1],pMAC[2],
                       pMAC[3],pMAC[4],pMAC[5]);
}

FTE_UINT32 FTE_JSON_printObjectGroups(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, FTE_UINT32 nFields)
{
    FTE_UINT32     ulDescCount, nLen;
    FTE_BOOL     bFirst = TRUE;
        
    ulDescCount = FTE_OBJ_DESC_count();
    
    nLen = snprintf(pBuff, nBuffLen, "\"groups\":[");        
    for(FTE_UINT32 i = 0 ; i < ulDescCount ; i++)
    {
        FTE_OBJECT_DESC_PTR pDesc;
       
        pDesc = FTE_OBJ_DESC_getAt(i);
    
        if (FTE_OBJ_count(pDesc->nType, FTE_OBJ_TYPE_MASK, FALSE) != 0)
        {
            if (!bFirst)
            {
                nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
            }
            
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "{");
            nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "name", pDesc->pName);
            nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "\"fields\":");
            nLen += FTE_JSON_printArrayObjectFields(&pBuff[nLen], nBuffLen - nLen, pDesc, nFields);
            nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "\"objects\":");
            nLen += FTE_JSON_printArrayObject(&pBuff[nLen], nBuffLen - nLen, pDesc->nType, nFields);
            nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
            bFirst = FALSE;
        }
    }
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");        
    
    return  nLen;
}
        
FTE_UINT32 FTE_JSON_printObjectSetProductInfo(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen)
{
    FTE_UINT32 nLen;
    FTE_PRODUCT_DESC const _PTR_    pProductDesc = FTE_getProductDescription();
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();

    /* Calculate content length while saving it to buffer */
    nLen  = snprintf(pBuff, nBuffLen , "{\"descs\" : [");
    nLen += FTE_JSON_printObjectBegin(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "title", "ID");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "value", FTE_SYS_getOIDString());
    nLen += FTE_JSON_printObjectEnd(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectBegin(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "title", "Model");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "value", pProductDesc->pModel);
    nLen += FTE_JSON_printObjectEnd(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectBegin(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"title", "Manufacturer");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"value", pProductDesc->pManufacturer);
    nLen += FTE_JSON_printObjectEnd(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectBegin(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "title",  "H/W Version");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectIP(&pBuff[nLen], nBuffLen - nLen, "value",  pProductDesc->xVersion.hw);
    nLen += FTE_JSON_printObjectEnd(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectBegin(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "title",  "S/W Version");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectIP(&pBuff[nLen], nBuffLen - nLen, "value",  pProductDesc->xVersion.sw);
    nLen += FTE_JSON_printObjectEnd(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectBegin(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "title",  "IP Address");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectIP(&pBuff[nLen], nBuffLen - nLen, "value",  pCfgNet->xIPData.ip);
    nLen += FTE_JSON_printObjectEnd(&pBuff[nLen], nBuffLen - nLen);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]}");
    
    return  nLen;
}

FTE_UINT32 FTE_JSON_printObjectSetNetwork(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen)
{
    FTE_UINT32         nLen;
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();
    
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += FTE_JSON_printObjectMAC(&pBuff[nLen], nBuffLen - nLen, "mac", pCfgNet->xMACAddr);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "type", (pCfgNet->nType == FTE_NET_TYPE_DHCP)?"dhcp":"static");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectIP(&pBuff[nLen], nBuffLen - nLen, "ip", pCfgNet->xIPData.ip);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectIP(&pBuff[nLen], nBuffLen - nLen, "mask", pCfgNet->xIPData.mask);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectIP(&pBuff[nLen], nBuffLen - nLen, "gateway", pCfgNet->xIPData.gateway);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
    
    return  nLen;
}

FTE_UINT32 FTE_JSON_printObjectSetConsole(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen)
{
    FTE_UINT32 nLen;
    
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += FTE_JSON_printObjectUINT(&pBuff[nLen], nBuffLen - nLen, "baudrate", 115200);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectUINT(&pBuff[nLen], nBuffLen - nLen, "databits", 8);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectUINT(&pBuff[nLen], nBuffLen - nLen, "stopbits", 1);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "paritybits", "none");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "flowcontrol", "none");
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");

    return  nLen;
}

FTE_UINT32 FTE_JSON_printObjectSetSelect(FTE_CHAR_PTR pBuff, FTE_UINT32 nBuffLen, fte_input_select_ptr pSelect)
{
    FTE_UINT32 nLen;

    /* Calculate content length while saving it to buffer */
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"type", "select");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "{\"items\":[");
    for(int i = 0 ; i < pSelect->nOptions ; i++)
    {
        if (i != 0)
        {
            nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
        }
        nLen += FTE_JSON_printObjectBegin(&pBuff[nLen], nBuffLen - nLen);
        nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"name", pSelect->pOptions[i].pName);
        nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
        nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"value", pSelect->pOptions[i].pValue);
        nLen += FTE_JSON_printObjectEnd(&pBuff[nLen], nBuffLen - nLen);
    }
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);    
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"value", pSelect->pValue);
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
    
    return  nLen;
    
}

FTE_UINT32 FTE_JSON_printObjectSetObject
(
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      nBuffLen, 
    FTE_OBJECT_PTR  pObject, 
    FTE_UINT32      nField
)
{
    FTE_UINT32 nLen;
    FTE_UINT32 nTempBuff;
    FTE_CHAR    pTempBuff[32];
    /* Calculate content length while saving it to buffer */
    nLen  = snprintf(pBuff, nBuffLen, "{");
    nLen += FTE_JSON_printObjectHEX(&pBuff[nLen], nBuffLen - nLen,"id", pObject->pConfig->nID);
    nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
    nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"name", pObject->pConfig->pName);
    if (nField & FTE_OBJ_FIELD_VALUE)
    {
        FTE_UINT32 nValue;
        
        pObject->pAction->f_get(pObject, &nValue, NULL);
        pObject->pAction->f_print_value(pObject, &nValue, pTempBuff, sizeof(pTempBuff));    
        nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
        nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"value", pTempBuff);
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
        nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
        nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"status", pTempBuff);
    }    
 
    if (nField & FTE_OBJ_FIELD_ENABLE)
    {
        nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
        if (FTE_FLAG_IS_SET(pObject->pConfig->xFlags, FTE_OBJ_CONFIG_FLAG_ENABLE))
        {
            nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "enable", "true");
        }
        else
        {
            nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen, "enable", "false");
        }
    }    
     
    if (nField & FTE_OBJ_FIELD_CTRL)
    {
        switch (FTE_OBJ_TYPE(pObject))
        {
        case    FTE_OBJ_TYPE_DO:
            nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
            if (((FTE_DO_STATUS_PTR)pObject->pStatus)->nValue == 0)
            {
                nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"cvalue", "ON");
            }
            else
            {
                nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"cvalue", "OFF");
            }
            break;
            
        case    FTE_OBJ_TYPE_RL:
            nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
            if (((FTE_RL_STATUS_PTR)pObject->pStatus)->nValue == 0)
            {
                nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"cvalue", "CLOSE");
            }
            else
            {
                nLen += FTE_JSON_printObjectSTRING(&pBuff[nLen], nBuffLen - nLen,"cvalue", "OPEN");
            }
            break;
        }
    }
    nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "}");
    
    return  nLen;
}

FTE_UINT32 FTE_JSON_printArrayObject
(
    FTE_CHAR_PTR    pBuff, 
    FTE_UINT32      nBuffLen, 
    FTE_UINT32      nType, 
    FTE_UINT32      nField
)
{
    FTE_UINT32 nLen = 0;
    FTE_UINT32 nObjCount = FTE_OBJ_count(nType, FTE_OBJ_TYPE_MASK, FALSE);

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
                nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
            }
            nLen += FTE_JSON_printObjectSetObject(&pBuff[nLen], nBuffLen - nLen, pObjectList[i], nField);
        }
    
        nLen += snprintf(&pBuff[nLen], nBuffLen - nLen, "]");
        
        FTE_MEM_free(pObjectList);
    }
    
    return  nLen;
}


FTE_UINT32 FTE_JSON_printArrayObjectFields
(
    FTE_CHAR_PTR        pBuff, 
    FTE_UINT32          nBuffLen, 
    FTE_OBJECT_DESC_PTR pDesc, 
    FTE_UINT32          nFields
)
{
    FTE_UINT32  nLen = 0;
    FTE_BOOL    bFirst = TRUE;
    
    nLen = snprintf(pBuff, nBuffLen, "[");
    for(FTE_UINT32 i = 0 ; (1 << i) != FTE_OBJ_FIELD_END ; i++)
    {
        if ((1 << i) & nFields & pDesc->xSupportedFields)
        {
            if (!bFirst)
            {
                nLen += FTE_JSON_printComma(&pBuff[nLen], nBuffLen - nLen);
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

#endfi