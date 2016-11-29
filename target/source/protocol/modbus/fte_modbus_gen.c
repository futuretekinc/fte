#include "fte_target.h"
#include "fte_modbus.h"
#include "fte_assert.h"
#include "fte_debug.h"
#include "fte_modbus_registers.h"

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readCoils
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_VOID_PTR    pData
)
{
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readDiscreteInputs
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_VOID_PTR    pData
)
{
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readHoldingRegister
(
    FTE_UINT16      usAddress,
    FTE_UINT16_PTR  pValue,
    FTE_VOID_PTR    pData
)
{
    FTE_UINT32  ulGroup = (FTE_UINT32)(usAddress / 1000);
    FTE_UINT32  ulIndex = (FTE_UINT32)(usAddress % 1000);

    if(ulGroup == 0)
    {
        return  FTE_MODBUS_REG_read(ulIndex, pValue, pData);
    }
    else
    {
        FTE_UINT32  ulClass =   ulGroup << FTE_OBJ_CLASS_SHIFT;
        
        if (ulClass == FTE_OBJ_CLASS_DO)
        {
            FTE_INT16   nValue = 0;
            FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(ulClass , FTE_OBJ_CLASS_MASK , ulIndex, FALSE);
            if (pObj == NULL)
            {
                return  FTE_RET_MODBUS_INVALID_ADDRESS;
            }
            
            FTE_VALUE_toINT16(pObj->pStatus->pValue, &nValue);
            htons(*pValue, nValue);        
        }
        else
        {
            return  FTE_RET_MODBUS_INVALID_ADDRESS;
        }
    }
    
    return  FTE_RET_OK;
}


static
FTE_RET FTE_MODBUS_SLAVE_GEN_readHoldingRegisters
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pRegisters,
    FTE_VOID_PTR    pData
)
{
    FTE_INT     i;
            
    for(i = 0 ; i < usQuantity ; i++)
    {
        FTE_MODBUS_SLAVE_GEN_readHoldingRegister(usAddress + i, &pRegisters[i], pData);
    }
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeSingleCoil
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue,
    FTE_VOID_PTR    pData
)
{
    if ((usValue != 0) && (usValue != 0xFF00))
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeSingleRegister
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue,
    FTE_VOID_PTR    pData
)
{
    FTE_RET     xRet;
    FTE_UINT32  ulGroup = (FTE_UINT32)(usAddress / 1000);
    FTE_UINT32  ulIndex = (FTE_UINT32)(usAddress % 1000);

    switch(ulGroup)
    {
    case    0:
        {
            return  FTE_MODBUS_REG_write(ulIndex, usValue, pData);
        }
        break;
        
    default:
        {
            FTE_UINT32  ulClass =   ulGroup << FTE_OBJ_CLASS_SHIFT;
            
            if (ulClass == FTE_OBJ_CLASS_DO)
            {
                FTE_VALUE   xValue;
                
                FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(ulClass , FTE_OBJ_CLASS_MASK , ulIndex, FALSE);
                if (pObj == NULL)
                {
                    return  FTE_RET_MODBUS_INVALID_ADDRESS;
                }

                FTE_VALUE_copy(&xValue, pObj->pStatus->pValue);
                xRet = FTE_VALUE_setULONG(&xValue, usValue);
                if (xRet != FTE_RET_OK)
                { 
                    return FTE_RET_MODBUS_INVALID_DATA;
                }
                
                xRet = FTE_OBJ_setValue(pObj, &xValue);
                if (xRet != FTE_RET_OK)
                {
                    return FTE_RET_MODBUS_INVALID_DATA;
                }                
            }
            else
            {
                return  FTE_RET_MODBUS_INVALID_ADDRESS;
            }
        }
    }
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeMultiCoils
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pValues,
    FTE_VOID_PTR    pData
)
{
    FTE_RET     xRet;
    FTE_INT     i;
    
    for(i = 0 ; i < usQuantity ; i++)
    {
        if ((pValues[i / 8] >> (i % 8)) & 0x01)
        {
            xRet = FTE_MODBUS_SLAVE_GEN_writeSingleCoil(usAddress + i, 0xFF00, pData);
        }
        else
        {
            xRet = FTE_MODBUS_SLAVE_GEN_writeSingleCoil(usAddress + i, 0x0000, pData);
        }
        
        if (xRet != FTE_RET_OK)
        {
            return  xRet;
        }
    }
    
    return  FTE_RET_OK;
}


static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeMultiRegisters
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pValues,
    FTE_VOID_PTR    pData
)
{
    FTE_RET     xRet;
    FTE_INT     i;
    
    for(i = 0 ; i < usQuantity ; i++)
    {
        xRet = FTE_MODBUS_SLAVE_GEN_writeSingleRegister(usAddress + i, pValues[i], pData);
        if (xRet != FTE_RET_OK)
        {
            return  xRet;
        }
    }
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeReadMultiRegisters
(
    FTE_UINT16      usWriteAddress,
    FTE_UINT16      usWriteQuantity,
    FTE_UINT16_PTR  pWriteValues,
    FTE_UINT16      usReadAddress,
    FTE_UINT16      usReadQuantity,
    FTE_UINT16_PTR  pReadValues,
    FTE_VOID_PTR    pData
)
{    
    FTE_RET     xRet;
    
    xRet = FTE_MODBUS_SLAVE_GEN_writeMultiRegisters(usWriteAddress, usWriteQuantity, pWriteValues, pData);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }
    
    return  FTE_MODBUS_SLAVE_GEN_readHoldingRegisters(usReadAddress, usReadQuantity, pReadValues, pData);
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readDeviceIdent
(
    FTE_UINT8       ucMEIType,
    FTE_UINT8       ucDeviceIDCode,
    FTE_UINT8       ucObjectID,
    FTE_MODBUS_OBJECT_VALUE_PTR pValues,
    FTE_UINT8       ucMaxValues,
    FTE_UINT8_PTR   pValueCount,
    FTE_UINT8_PTR   pConformityLevel,
    FTE_UINT8_PTR   pMoreFollows,
    FTE_UINT8_PTR   pNextObjectID,
    FTE_VOID_PTR    pData
)
{
    FTE_INT i;
    
    if (ucMEIType != 0x0E)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }
 
    if (ucObjectID >= 3)
    {
        return  FTE_RET_MODBUS_INVALID_ADDRESS;
    }
    
    if (ucDeviceIDCode != 0)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }
    
    *pValueCount = 0;
    for(i = 0; i < ucMaxValues && ucObjectID < 3 ; i++, ucObjectID++)
    {
        pValues[i].ucID = ucObjectID;
        switch(ucObjectID)
        {
        case    0:
            {
                memcpy(pValues[i].pValue, "FutureTek,Inc.", 14);
                pValues[i].ucLen = 14;
            }
            break;

        case    1:
            {
                memcpy(pValues[i].pValue, "fte series", 9);
                pValues[i].ucLen = 9;
            }
            break;
            
        case    2:
            {
                memcpy(pValues[i].pValue, "v1.0", 4);
                pValues[i].ucLen = 4;
            }
            break;            
        }
        
        (*pValueCount)++;
    }
    
    *pConformityLevel   = 0x81;
    *pMoreFollows       = 0;
    *pNextObjectID      = 0;

    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readInputRegister
(
    FTE_UINT16      usAddress,
    FTE_UINT16_PTR  pValue,
    FTE_VOID_PTR    pData
)
{
    FTE_INT16   nValue = 0;
    FTE_UINT32  ulClass = (FTE_UINT32)(usAddress / 1000)  << FTE_OBJ_CLASS_SHIFT;
    FTE_UINT32  ulIndex = (FTE_UINT32)(usAddress % 1000);
    
    FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(ulClass , FTE_OBJ_CLASS_MASK , ulIndex, FALSE);
    if (pObj == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_ADDRESS;
    }
    
    FTE_VALUE_toINT16(pObj->pStatus->pValue, &nValue);
    
    *pValue = nValue;        
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readInputRegisters
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pValues,
    FTE_VOID_PTR    pData
)
{
    ASSERT(pValues != NULL);
    
    FTE_RET     xRet;
    FTE_INT     i;
    
    for(i = 0 ; i < usQuantity ; i++)
    {
        xRet = FTE_MODBUS_SLAVE_GEN_readInputRegister(usAddress + i, &pValues[i], pData);
        if (xRet != FTE_RET_OK)
        {
            return  xRet;
        }
    }
    
    return  FTE_RET_OK;
}

static
FTE_MODBUS_SLAVE_MODEL  pModels[] =
{
    [0] =
    {
        .xType = FTE_MODBUS_SLAVE_TYPE_GENERAL,
        .fReadCoils             =   FTE_MODBUS_SLAVE_GEN_readCoils,
        .fReadDiscreteInputs    =   FTE_MODBUS_SLAVE_GEN_readDiscreteInputs,
        .fReadHoldingRegisters  =   FTE_MODBUS_SLAVE_GEN_readHoldingRegisters,
        .fReadInputRegisters    =   FTE_MODBUS_SLAVE_GEN_readInputRegisters,
        .fWriteSingleRegister   =   FTE_MODBUS_SLAVE_GEN_writeSingleRegister,
        .fWriteMultiCoils       =   FTE_MODBUS_SLAVE_GEN_writeMultiCoils,
        .fWriteMultiRegisters   =   FTE_MODBUS_SLAVE_GEN_writeMultiRegisters,
        .fWriteReadMultiRegisters=  FTE_MODBUS_SLAVE_GEN_writeReadMultiRegisters,
        .fReadDeviceIdent       =   FTE_MODBUS_SLAVE_GEN_readDeviceIdent
    }
};

FTE_RET FTE_MODBUS_SLAVE_getModel
(
    FTE_MODBUS_SLAVE_TYPE   xType,
    FTE_MODBUS_SLAVE_MODEL_PTR _PTR_ ppModel
)
{
    for(FTE_INT i = 0 ; i < sizeof(pModels) / sizeof(FTE_MODBUS_SLAVE_MODEL) ; i++)
    {
        if (pModels[i].xType == xType)
        {
            *ppModel = &pModels[i];
            
            return  FTE_RET_OK;
        }
    }
    
    return  FTE_RET_OBJECT_NOT_FOUND;
}