#include "fte_target.h"
#include "fte_modbus.h"
#include "fte_assert.h"


FTE_RET FTE_MODBUS_SLAVE_getModel
(
    FTE_MODBUS_SLAVE_TYPE   xType,
    FTE_MODBUS_SLAVE_MODEL_PTR _PTR_ ppModel
);

static
FTE_RET FTE_MODBUS_SLAVE_readCoils
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen
);

static
FTE_RET FTE_MODBUS_SLAVE_readDiscreteInputs
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen
);

static
FTE_RET FTE_MODBUS_SLAVE_readHoldingRegisters
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,    
    FTE_UINT16_PTR  pRegisters
);

static
FTE_RET FTE_MODBUS_SLAVE_readInputRegisters
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pRegisters
);

static
FTE_RET FTE_MODBUS_SLAVE_writeSingleCoil
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue
);

static
FTE_RET FTE_MODBUS_SLAVE_writeSingleRegister
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue
);

static
FTE_RET FTE_MODBUS_SLAVE_writeMultiCoils
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pOutputValues
);

static
FTE_RET FTE_MODBUS_SLAVE_writeMultiRegisters
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,    
    FTE_UINT16_PTR  pValues
);

static
FTE_RET FTE_MODBUS_SLAVE_readDeviceIdent
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT8       ucMEIType,
    FTE_UINT8       ucDeviceIDCode,
    FTE_UINT8       ucObjectID,
    FTE_MODBUS_OBJECT_VALUE_PTR pValues,
    FTE_UINT8       ucMaxValues,
    FTE_UINT8_PTR   pValueCount,
    FTE_UINT8_PTR   pConformityLevel,
    FTE_UINT8_PTR   pMoreFollows,
    FTE_UINT8_PTR   pNextObjectID
);

FTE_RET FTE_MODBUS_SLAVE_init
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_MODBUS_SLAVE_CONFIG_PTR pConfig
)
{
    ASSERT(pSlave != NULL);
    ASSERT(pConfig != NULL);    
    FTE_RET xRet;
    
    memcpy(&pSlave->xConfig, pConfig, sizeof(FTE_MODBUS_SLAVE_CONFIG));
    
    xRet = FTE_MODBUS_SLAVE_getModel(pSlave->xConfig.xType, &pSlave->pModel);
    if (xRet != FTE_RET_OK)
    {
        return  xRet;
    }

    return  FTE_RET_OK;
}

FTE_RET FTE_MODBUS_SLAVE_process
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT8_PTR   pReqFrame,
    FTE_UINT32      ulReqFrameLen,
    FTE_UINT8_PTR   pRespBuff,
    FTE_UINT32      ulRespBuffLen,
    FTE_UINT32_PTR  pRespPDULen
)
{
    ASSERT(pReqFrame != NULL);
    ASSERT(pRespBuff != NULL);
    ASSERT(pRespPDULen != NULL);
    FTE_RET xRet;
    
    FTE_MODBUS_REQ_PDU_PTR  pReq = (FTE_MODBUS_REQ_PDU_PTR)pReqFrame;
    FTE_MODBUS_RESP_PDU_PTR pResp= (FTE_MODBUS_RESP_PDU_PTR)pRespBuff;
    
    switch(pReq->xCommon.ucFunctionCode)
    {
    case    FTE_MODBUS_FUNCTION_READ_COILS:
        {
            FTE_UINT16  usAddress;
            FTE_UINT16  usQuantity;
            FTE_UINT8   ucByteCount;
            FTE_UINT8_PTR   pCoilStatus;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_READ_COILS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            
            usAddress = FTE_NTOHS(pReq->xReadCoils.usAddress);
            usQuantity = FTE_NTOHS(pReq->xReadCoils.usQuantity);
            
            if ((usQuantity < 1) || (usQuantity > FTE_MODBUS_MAX_COILS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            
            ucByteCount = (usQuantity + 7) / 8;
            if (ulRespBuffLen < 2 + ucByteCount)
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
            
            pCoilStatus = (FTE_UINT8_PTR)FTE_MEM_allocZero(ucByteCount);
            if (pCoilStatus == NULL)
            {
                xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                break;
            }
            
            xRet = FTE_MODBUS_SLAVE_readCoils(pSlave, usAddress, usQuantity, pCoilStatus, ucByteCount);
            if (xRet == FTE_RET_OK)
            {
                pResp->xReadCoils.ucFunctionCode   = pReq->xReadCoils.ucFunctionCode;
                pResp->xReadCoils.ucByteCount      = ucByteCount;
                memcpy(pResp->xReadCoils.pCoilStatus, pCoilStatus, ucByteCount);
                
                *pRespPDULen = 2 + ucByteCount;
            }

            FTE_MEM_free(pCoilStatus);
        }
        break;

    case    FTE_MODBUS_FUNCTION_READ_DISCRETE_INPUTS:
        {
            FTE_UINT16  usAddress;
            FTE_UINT16  usQuantity;
            FTE_UINT8   ucByteCount;
            FTE_UINT8_PTR   pInputStatus;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_READ_DISCRETE_INPUTS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            
            usAddress = FTE_NTOHS(pReq->xReadDiscreteInputs.usAddress);
            usQuantity = FTE_NTOHS(pReq->xReadDiscreteInputs.usQuantity);
            
            if ((usQuantity < 1) || (usQuantity > FTE_MODBUS_MAX_DISCRETE_INPUTS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }            
            
            ucByteCount = (usQuantity + 7) / 8;
            if (ulRespBuffLen < 2 + ucByteCount)
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
            
            pInputStatus = (FTE_UINT8_PTR)FTE_MEM_allocZero(ucByteCount);
            if (pInputStatus == NULL)
            {
                xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                break;
            }
            
            xRet = FTE_MODBUS_SLAVE_readDiscreteInputs(pSlave, usAddress, usQuantity, pInputStatus, ucByteCount);
            if (xRet == FTE_RET_OK)
            {
                pResp->xReadDiscreteInputs.ucFunctionCode   = pReq->xReadDiscreteInputs.ucFunctionCode;
                pResp->xReadDiscreteInputs.ucByteCount      = ucByteCount;
                memcpy(pResp->xReadDiscreteInputs.pInputStatus, pInputStatus, ucByteCount);
                
                *pRespPDULen = 2 + ucByteCount;
            }
            
            FTE_MEM_free(pInputStatus);
        }
        break;

    case    FTE_MODBUS_FUNCTION_READ_HOLDING_REGISTERS:
        {
            FTE_UINT16      usAddress;
            FTE_UINT16      usQuantity;
            FTE_UINT16_PTR  pRegisters;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_READ_HOLDING_REGISTERS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            usAddress = FTE_NTOHS(pReq->xReadHoldingRegisters.usAddress);
            usQuantity = FTE_NTOHS(pReq->xReadHoldingRegisters.usQuantity);
            
            if ((usQuantity < 1) || (usQuantity > FTE_MODBUS_MAX_HOINDING_REGISTERS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }                        
            
            if (ulRespBuffLen < 2 + usQuantity * 2)
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }

            pRegisters = (FTE_UINT16_PTR)FTE_MEM_allocZero(sizeof(FTE_UINT16) * usQuantity);
            if (pRegisters == NULL)
            {
                xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                break;
            }
            
            xRet = FTE_MODBUS_SLAVE_readHoldingRegisters(pSlave, usAddress, usQuantity, pRegisters);
            if (xRet == FTE_RET_OK)
            {
                FTE_INT i;
                
                pResp->xReadHoldingRegisters.ucFunctionCode = pReq->xReadHoldingRegisters.ucFunctionCode;
                pResp->xReadHoldingRegisters.ucByteCount  =  usQuantity * 2;
                for(i = 0 ; i < usQuantity ; i++)
                {
                    pResp->xReadHoldingRegisters.pValues[i] = FTE_HTONS(pRegisters[i]);
                }
                
                *pRespPDULen = 2 + usQuantity * 2;
            }
            
            FTE_MEM_free(pRegisters);
        }
        break;

    case    FTE_MODBUS_FUNCTION_READ_INPUT_REGISTERS:
        {
            FTE_UINT16  usAddress;
            FTE_UINT16  usQuantity;
            FTE_UINT16_PTR  pRegisters;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_READ_INPUT_REGISTERS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            usAddress = FTE_NTOHS(pReq->xReadInputRegisters.usAddress);
            usQuantity = FTE_NTOHS(pReq->xReadInputRegisters.usQuantity);
            
            if ((usQuantity < 1) || (usQuantity > FTE_MODBUS_MAX_INPUT_REGISTERS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }                        
                         
            if (ulRespBuffLen < 2 + usQuantity * 2)
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
            
            pRegisters = (FTE_UINT16_PTR)FTE_MEM_allocZero(sizeof(FTE_UINT16) * usQuantity);
            if (pRegisters == NULL)
            {
                xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                break;
            }
            
            xRet = FTE_MODBUS_SLAVE_readInputRegisters(pSlave, usAddress, usQuantity, pRegisters);
            if (xRet == FTE_RET_OK)
            {
                FTE_INT i;
                
                pResp->xReadInputRegisters.ucFunctionCode = pReq->xReadInputRegisters.ucFunctionCode;
                pResp->xReadInputRegisters.ucByteCount  =  usQuantity * 2;
                for(i = 0 ; i < usQuantity ; i++)
                {
                    pResp->xReadInputRegisters.pValues[i] = FTE_HTONS(pRegisters[i]);
                }
                
                *pRespPDULen = 2 + usQuantity * 2;
            }
            
            FTE_MEM_free(pRegisters);
        }
        break;
        

    case    FTE_MODBUS_FUNCTION_WRITE_SINGLE_COIL:
        {
            FTE_UINT16  usAddress;
            FTE_UINT16  usValue;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_WRITE_SINGLE_COIL))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            usAddress   = FTE_NTOHS(pReq->xWriteSingleCoil.usAddress);
            usValue     = FTE_NTOHS(pReq->xWriteSingleCoil.usValue);
            
            if (ulRespBuffLen < sizeof(FTE_MODBUS_RESP_PDU_WRITE_SINGLE_COIL))
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
            
            xRet = FTE_MODBUS_SLAVE_writeSingleCoil(pSlave, usAddress, usValue);
            if (xRet == FTE_RET_OK)
            {
                pResp->xWriteSingleCoil.ucFunctionCode  = pReq->xWriteSingleCoil.ucFunctionCode;
                pResp->xWriteSingleCoil.usAddress = FTE_HTONUS(usAddress);
                pResp->xWriteSingleCoil.usValue   = FTE_HTONUS(usValue);
                
                *pRespPDULen = sizeof(FTE_MODBUS_REQ_PDU_WRITE_SINGLE_COIL);
            }
        }
        break;
        
    case    FTE_MODBUS_FUNCTION_WRITE_SINGLE_REGISTER:
        {
            FTE_UINT16  usAddress;
            FTE_UINT16  usValue;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_WRITE_SINGLE_REGISTER))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            usAddress   = FTE_NTOHS(pReq->xWriteSingleRegister.usAddress);
            usValue     = FTE_NTOHS(pReq->xWriteSingleRegister.usValue);
            
            if (ulRespBuffLen < sizeof(FTE_MODBUS_RESP_PDU_WRITE_SINGLE_REGISTER))
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
            
            xRet = FTE_MODBUS_SLAVE_writeSingleRegister(pSlave, usAddress, usValue);
            if (xRet == FTE_RET_OK)
            {
                pResp->xWriteSingleRegister.ucFunctionCode  = pReq->xWriteSingleRegister.ucFunctionCode;
                pResp->xWriteSingleRegister.usAddress       = FTE_HTONUS(usAddress);
                pResp->xWriteSingleRegister.usValue         = FTE_HTONUS(usValue);
                
                *pRespPDULen = sizeof(FTE_MODBUS_REQ_PDU_WRITE_SINGLE_COIL);
            }
        }
        break;
        
        
    case    FTE_MODBUS_FUNCTION_WRITE_MULTI_COILS:
        {
            FTE_UINT16  usAddress;
            FTE_UINT16  usQuantity;
            FTE_UINT8   ucByteCount;
            if (ulReqFrameLen <= sizeof(FTE_MODBUS_REQ_PDU_WRITE_MULTI_COILS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            
            usAddress   = FTE_NTOHS(pReq->xWriteMultiCoils.usAddress);
            usQuantity  = FTE_NTOHS(pReq->xWriteMultiCoils.usQuantity);
            ucByteCount = pReq->xWriteMultiCoils.ucByteCount;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_WRITE_MULTI_COILS) + ucByteCount)
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            
            if (ulRespBuffLen < sizeof(FTE_MODBUS_RESP_PDU_WRITE_MULTI_COILS))
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
            
            xRet = FTE_MODBUS_SLAVE_writeMultiCoils(pSlave, usAddress, usQuantity, pReq->xWriteMultiCoils.pValues);
            if (xRet == FTE_RET_OK)
            {
                pResp->xWriteMultiCoils.ucFunctionCode  = pReq->xWriteMultiCoils.ucFunctionCode;
                pResp->xWriteMultiCoils.usAddress       = FTE_HTONUS(usAddress);
                pResp->xWriteMultiCoils.usQuantity      = FTE_HTONUS(usQuantity);
                
                *pRespPDULen = sizeof(FTE_MODBUS_RESP_PDU_WRITE_MULTI_COILS);
            }
        }
        break;
        
    case    FTE_MODBUS_FUNCTION_WRITE_MULTI_REGISTERS:
        {
            FTE_UINT16  usAddress;
            FTE_UINT16  usQuantity;
            FTE_UINT8   ucByteCount;
            FTE_UINT16_PTR  pValues;
            FTE_INT     i;
            
            if (ulReqFrameLen <= sizeof(FTE_MODBUS_REQ_PDU_WRITE_MULTI_REGISTERS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            usAddress   = FTE_NTOHS(pReq->xWriteMultiRegisters.usAddress);
            usQuantity  = FTE_NTOHS(pReq->xWriteMultiRegisters.usQuantity);
            ucByteCount = pReq->xWriteMultiRegisters.ucByteCount;
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_WRITE_MULTI_REGISTERS) + ucByteCount)
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            
            if (ulRespBuffLen < sizeof(FTE_MODBUS_RESP_PDU_WRITE_MULTI_REGISTERS))
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
          
            pValues = (FTE_UINT16_PTR)FTE_MEM_allocZero(sizeof(FTE_UINT16) * usQuantity);
            if (pValues == NULL)
            {
                xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                break;
            }
            
            for(i = 0 ; i < usQuantity ; i++)
            {
                pValues[i] = FTE_NTOHS(pReq->xWriteMultiRegisters.pValues[i]);
            }
            
            xRet = FTE_MODBUS_SLAVE_writeMultiRegisters(pSlave, usAddress, usQuantity, pValues);
            if (xRet == FTE_RET_OK)
            {
                pResp->xWriteMultiRegisters.ucFunctionCode  = pReq->xWriteMultiRegisters.ucFunctionCode;
                pResp->xWriteMultiRegisters.usAddress   = FTE_HTONUS(usAddress);
                pResp->xWriteMultiRegisters.usQuantity  = FTE_HTONUS(usQuantity);
                
                *pRespPDULen = sizeof(FTE_MODBUS_RESP_PDU_WRITE_MULTI_REGISTERS);
            }
            
            FTE_MEM_free(pValues);
        }
        break;        
        
    case    FTE_MODBUS_FUNCTION_WRITE_READ_MULTI_REGISTERS:
        {
            FTE_UINT16  usWriteAddress;
            FTE_UINT16  usWriteQuantity;
            FTE_UINT16  usReadAddress;
            FTE_UINT16  usReadQuantity;
            FTE_UINT8   ucByteCount;
            FTE_UINT16_PTR  pWriteValues;
            FTE_UINT16_PTR  pReadValues;
            FTE_INT     i;
            
            if (ulReqFrameLen <= sizeof(FTE_MODBUS_REQ_PDU_WRITE_READ_MULTI_REGISTERS))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            usWriteAddress  = FTE_NTOHS(pReq->xWriteReadMultiRegisters.usWriteAddress);
            usWriteQuantity = FTE_NTOHS(pReq->xWriteReadMultiRegisters.usWriteQuantity);
            ucByteCount     = pReq->xWriteReadMultiRegisters.ucWriteByteCount;
            usReadAddress   = FTE_NTOHS(pReq->xWriteReadMultiRegisters.usReadAddress);
            usReadQuantity  = FTE_NTOHS(pReq->xWriteReadMultiRegisters.usReadQuantity);
            
            if (ulReqFrameLen != sizeof(FTE_MODBUS_REQ_PDU_WRITE_READ_MULTI_REGISTERS) + ucByteCount)
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            
            if (ulRespBuffLen < sizeof(FTE_MODBUS_RESP_PDU_WRITE_READ_MULTI_REGISTERS))
            {
                xRet = FTE_RET_BUFFER_TOO_SMALL;
                break;
            }
          
            pWriteValues = (FTE_UINT16_PTR)FTE_MEM_allocZero(sizeof(FTE_UINT16) * usWriteQuantity);
            if (pWriteValues == NULL)
            {
                xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                break;
            }
            
            for(i = 0 ; i < usWriteQuantity ; i++)
            {
                pWriteValues[i] = FTE_NTOHS(pReq->xWriteMultiRegisters.pValues[i]);
            }
            
            xRet = FTE_MODBUS_SLAVE_writeMultiRegisters(pSlave, usWriteAddress, usWriteQuantity, pWriteValues);
            
            FTE_MEM_free(pWriteValues);

            if (xRet == FTE_RET_OK)
            {
                pReadValues = (FTE_UINT16_PTR)FTE_MEM_allocZero(sizeof(FTE_UINT16) * usReadQuantity);
                if (pReadValues == NULL)
                {
                    xRet = FTE_RET_NOT_ENOUGH_MEMORY;
                    break;
                }
                
                xRet = FTE_MODBUS_SLAVE_readInputRegisters(pSlave, usReadAddress, usReadQuantity, pReadValues);
                if (xRet == FTE_RET_OK)
                {
                    FTE_INT i;
                    
                    pResp->xWriteReadMultiRegisters.ucFunctionCode = pReq->xWriteReadMultiRegisters.ucFunctionCode;
                    pResp->xWriteReadMultiRegisters.ucByteCount  =  usReadQuantity * 2;
                    for(i = 0 ; i < usReadQuantity ; i++)
                    {
                        pResp->xWriteReadMultiRegisters.pReadValues[i] = FTE_HTONS(pReadValues[i]);
                    }
                    
                    *pRespPDULen = 2 + usReadQuantity * 2;
                }
                
                FTE_MEM_free(pReadValues);
            }
        }
        break;        
        
    case    FTE_MODBUS_FUNCTION_READ_DEVICE_IDENT:
        {
            FTE_INT     i;
            FTE_UINT8   ucMEIType;
            FTE_UINT8   ucDeviceIDCode;
            FTE_UINT8   ucObjectID;
            FTE_MODBUS_OBJECT_VALUE pValues[3];
            FTE_UINT8   ucValueCount;
            FTE_UINT8   ucConformityLevel;
            FTE_UINT8   ucMoreFollows;
            FTE_UINT8   ucNextObjectID;

            if (ulReqFrameLen <= sizeof(FTE_MODBUS_REQ_PDU_READ_DEVICE_IDENT))
            {
                xRet = FTE_RET_INVALID_MSG;
                break;
            }
            ucMEIType   = pReq->xReadDeviceIdent.ucMEIType;
            ucDeviceIDCode = pReq->xReadDeviceIdent.ucDeviceIDCode;
            ucObjectID  = pReq->xReadDeviceIdent.ucObjectID;
            
            xRet = FTE_MODBUS_SLAVE_readDeviceIdent(pSlave, ucMEIType, ucDeviceIDCode, ucObjectID, pValues, 3, &ucValueCount, &ucConformityLevel, &ucMoreFollows, &ucNextObjectID);
            if (xRet == FTE_RET_OK)
            {
                FTE_INT nPos = 0;
                for(i = 0 ; i < ucValueCount ; i++)
                {
                    nPos += 2 + pValues[i].ucLen;
                }
                if (ulRespBuffLen < sizeof(FTE_MODBUS_RESP_PDU_READ_DEVICE_IDENT) + nPos)
                {
                    xRet = FTE_RET_BUFFER_TOO_SMALL;
                    break;
                }
                
                pResp->xReadDeviceIdent.ucFunctionCode  = pReq->xWriteMultiRegisters.ucFunctionCode;
                pResp->xReadDeviceIdent.ucMEIType       = ucMEIType;
                pResp->xReadDeviceIdent.ucDeviceIDCode  = ucDeviceIDCode;
                pResp->xReadDeviceIdent.ucConformityLevel= ucConformityLevel;
                pResp->xReadDeviceIdent.ucMoreFollows   = ucMoreFollows;
                pResp->xReadDeviceIdent.ucNextObjectID  = ucNextObjectID;
                pResp->xReadDeviceIdent.ucObjectCount   = ucValueCount;
                
                nPos = 0;
                for(i = 0 ; i < ucValueCount ; i++)
                {
                    memcpy(&pResp->xReadDeviceIdent.pObjectList[nPos], &pValues[i], 2 + pValues[i].ucLen);
                    nPos += 2 + pValues[i].ucLen;
                }
                *pRespPDULen = sizeof(FTE_MODBUS_RESP_PDU_READ_DEVICE_IDENT) + nPos;
            }
            
            FTE_MEM_free(pValues);
        }
        break;        
        
        
    default:
        {
            xRet = FTE_MODBUS_EXCEPT_INVALID_FUNCTION;
        }
        
    }
    
    if ((xRet == FTE_RET_MODBUS_INVALID_FUNCTION) ||
        (xRet == FTE_RET_MODBUS_INVALID_ADDRESS) ||
        (xRet == FTE_RET_MODBUS_INVALID_DATA) ||
        (xRet == FTE_RET_MODBUS_EXECUTE_ERROR))
    {
        
        pResp->xError.ucFunctionCode = pReq->xCommon.ucFunctionCode | 0x80;
        pResp->xError.ucExceptCode   = xRet & (0x0F);
            
        *pRespPDULen = 2;
        
        xRet = FTE_RET_OK;
    }
    
    return  xRet;
}

FTE_RET FTE_MODBUS_SLAVE_readCoils
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen
)
{
    ASSERT((pSlave != NULL) && (pSlave->pModel != NULL));
    ASSERT(pBuff != NULL);
    
    if (pSlave->pModel->fReadCoils == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fReadCoils(usAddress, usQuantity, pBuff, ulBuffLen, pSlave);
}

FTE_RET FTE_MODBUS_SLAVE_readDiscreteInputs
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen
)
{
    ASSERT(pSlave != NULL);
    ASSERT(pBuff != NULL);
    
    if (pSlave->pModel->fReadDiscreteInputs == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fReadDiscreteInputs(usAddress, usQuantity, pBuff, ulBuffLen, pSlave);
}

FTE_RET FTE_MODBUS_SLAVE_readHoldingRegisters
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,    
    FTE_UINT16_PTR  pRegisters
)
{
    ASSERT(pSlave != NULL);
    ASSERT(pRegisters != NULL);
    
    if (pSlave->pModel->fReadHoldingRegisters == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fReadHoldingRegisters(usAddress, usQuantity, pRegisters, pSlave);
}

FTE_RET FTE_MODBUS_SLAVE_readInputRegisters
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pRegisters    
)
{
    ASSERT(pSlave != NULL);
    ASSERT(pRegisters != NULL);
    
    if (pSlave->pModel->fReadInputRegisters == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fReadInputRegisters(usAddress, usQuantity, pRegisters, pSlave);
}

FTE_RET FTE_MODBUS_SLAVE_writeSingleCoil
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue
)
{
    ASSERT(pSlave != NULL);
    
    if (pSlave->pModel->fWriteSingleCoil == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fWriteSingleCoil(usAddress, usValue, pSlave);
}

FTE_RET FTE_MODBUS_SLAVE_writeSingleRegister
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue
)
{
    ASSERT(pSlave != NULL);
    
    if (pSlave->pModel->fWriteSingleRegister == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fWriteSingleRegister(usAddress, usValue, pSlave);
}


FTE_RET FTE_MODBUS_SLAVE_writeMultiCoils
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pOutputValues
)
{
    ASSERT(pSlave != NULL);
    
    if (pSlave->pModel->fWriteMultiCoils == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fWriteMultiCoils(usAddress, usQuantity, pOutputValues, pSlave);
}

FTE_RET FTE_MODBUS_SLAVE_writeMultiRegisters
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pValues
)
{
    ASSERT(pSlave != NULL);
    
    if (pSlave->pModel->fWriteMultiRegisters == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fWriteMultiRegisters(usAddress, usQuantity, pValues, pSlave);
}

FTE_RET FTE_MODBUS_SLAVE_readDeviceIdent
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT8       ucMEIType,
    FTE_UINT8       ucDeviceIDCode,
    FTE_UINT8       ucObjectID,
    FTE_MODBUS_OBJECT_VALUE_PTR pValues,
    FTE_UINT8       ucMaxValues,
    FTE_UINT8_PTR   pValueCount,
    FTE_UINT8_PTR   pConformityLevel,
    FTE_UINT8_PTR   pMoreFollows,
    FTE_UINT8_PTR   pNextObjectID
)
{
    ASSERT(pSlave != NULL);
    
    if (pSlave->pModel->fReadDeviceIdent == NULL)
    {
        return  FTE_RET_MODBUS_INVALID_FUNCTION;
    }
    
    return  pSlave->pModel->fReadDeviceIdent(ucMEIType, ucDeviceIDCode, ucObjectID, pValues, ucMaxValues, pValueCount, pConformityLevel, pMoreFollows, pNextObjectID, pSlave);
}

