#ifndef __FTE_MODBUS_H__
#define __FTE_MODBUS_H__    1

#include "fte_type.h"

#define FTE_MODBUS_PDU_LEN           253

#define FTE_MODBUS_MAX_COILS                2000
#define FTE_MODBUS_MAX_DISCRETE_INPUTS      2000
#define FTE_MODBUS_MAX_HOINDING_REGISTERS   125
#define FTE_MODBUS_MAX_INPUT_REGISTERS      125

#define FTE_MODBUS_FUNCTION_READ_COILS                  1
#define FTE_MODBUS_FUNCTION_READ_DISCRETE_INPUTS        2
#define FTE_MODBUS_FUNCTION_READ_HOLDING_REGISTERS      3
#define FTE_MODBUS_FUNCTION_READ_INPUT_REGISTERS        4
#define FTE_MODBUS_FUNCTION_WRITE_SINGLE_COIL           5
#define FTE_MODBUS_FUNCTION_WRITE_SINGLE_REGISTER       6
#define FTE_MODBUS_FUNCTION_WRITE_MULTI_COILS           15
#define FTE_MODBUS_FUNCTION_WRITE_MULTI_REGISTERS       16
#define FTE_MODBUS_FUNCTION_WRITE_READ_MULTI_REGISTERS  23
#define FTE_MODBUS_FUNCTION_READ_DEVICE_IDENT           43

#define FTE_MODBUS_EXCEPT_INVALID_FUNCTION          1
#define FTE_MODBUS_EXCEPT_INVALID_ADDRESS           2
#define FTE_MODBUS_EXCEPT_INVALID_DATA              3
#define FTE_MODBUS_EXCEPT_EXECUTE_ERROR             4

typedef enum
{
    FTE_MODBUS_SLAVE_TYPE_GENERAL = 0
}   FTE_MODBUS_SLAVE_TYPE, _PTR_ FTE_MODBUS_SLAVE_TYPE_PTR;

#pragma pack(push,1)
typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
}   FTE_MODBUS_REQ_PDU_COMMON, _PTR_ FTE_MODBUS_REQ_PDU_COMMON_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucByteCount;
}   FTE_MODBUS_RESP_PDU_COMMON, _PTR_ FTE_MODBUS_RESP_PDU_COMMON_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucExceptCode;
}   FTE_MODBUS_RESP_PDU_ERROR, _PTR_ FTE_MODBUS_RESP_PDU_ERROR_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
}   FTE_MODBUS_REQ_PDU_READ_COILS, _PTR_ FTE_MODBUS_REQ_PDU_READ_COILS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucByteCount;
    FTE_UINT8       pCoilStatus[];
}   FTE_MODBUS_RESP_PDU_READ_COILS, _PTR_ FTE_MODBUS_RESP_PDU_READ_COILS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
}   FTE_MODBUS_REQ_PDU_READ_DISCRETE_INPUTS, _PTR_ FTE_MODBUS_REQ_PDU_READ_DISCRETE_INPUTS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucByteCount;
    FTE_UINT8       pInputStatus[];
}   FTE_MODBUS_RESP_PDU_READ_DISCRETE_INPUTS, _PTR_ FTE_MODBUS_RESP_PDU_READ_DISCRETE_INPUTS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
}   FTE_MODBUS_REQ_PDU_READ_HOLDING_REGISTERS, _PTR_ FTE_MODBUS_REQ_PDU_READ_HOLDING_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucByteCount;
    FTE_UINT16      pValues[];
}   FTE_MODBUS_RESP_PDU_READ_HOLDING_REGISTERS, _PTR_ FTE_MODBUS_RESP_PDU_READ_HOLDING_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
}   FTE_MODBUS_REQ_PDU_READ_INPUT_REGISTERS, _PTR_ FTE_MODBUS_REQ_PDU_READ_INPUT_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucByteCount;
    FTE_UINT16      pValues[];
}   FTE_MODBUS_RESP_PDU_READ_INPUT_REGISTERS, _PTR_ FTE_MODBUS_RESP_PDU_READ_INPUT_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usValue;
}   FTE_MODBUS_REQ_PDU_WRITE_SINGLE_COIL, _PTR_ FTE_MODBUS_REQ_PDU_WRITE_SINGLE_COIL_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usValue;
}   FTE_MODBUS_RESP_PDU_WRITE_SINGLE_COIL, _PTR_ FTE_MODBUS_RESP_PDU_WRITE_SINGLE_COIL_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usValue;
}   FTE_MODBUS_REQ_PDU_WRITE_SINGLE_REGISTER, _PTR_ FTE_MODBUS_REQ_PDU_WRITE_SINGLE_REGISTER_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usValue;
}   FTE_MODBUS_RESP_PDU_WRITE_SINGLE_REGISTER, _PTR_ FTE_MODBUS_RESP_PDU_WRITE_SINGLE_REGISTER_PTR;


typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
    FTE_UINT8       ucByteCount;
    FTE_UINT8       pValues[];
}   FTE_MODBUS_REQ_PDU_WRITE_MULTI_COILS, _PTR_ FTE_MODBUS_REQ_PDU_WRITE_MULTI_COILS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
}   FTE_MODBUS_RESP_PDU_WRITE_MULTI_COILS, _PTR_ FTE_MODBUS_RESP_PDU_WRITE_MULTI_COILS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
    FTE_UINT8       ucByteCount;
    FTE_UINT16      pValues[];
}   FTE_MODBUS_REQ_PDU_WRITE_MULTI_REGISTERS, _PTR_ FTE_MODBUS_REQ_PDU_WRITE_MULTI_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usAddress;
    FTE_UINT16      usQuantity;
}   FTE_MODBUS_RESP_PDU_WRITE_MULTI_REGISTERS, _PTR_ FTE_MODBUS_RESP_PDU_WRITE_MULTI_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT16      usReadAddress;
    FTE_UINT16      usReadQuantity;
    FTE_UINT16      usWriteAddress;
    FTE_UINT16      usWriteQuantity;
    FTE_UINT8       ucWriteByteCount;
    FTE_UINT16      pWriteValues[];
}   FTE_MODBUS_REQ_PDU_WRITE_READ_MULTI_REGISTERS, _PTR_ FTE_MODBUS_REQ_PDU_WRITE_READ_MULTI_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucByteCount;
    FTE_UINT16      pReadValues[];
}   FTE_MODBUS_RESP_PDU_WRITE_READ_MULTI_REGISTERS, _PTR_ FTE_MODBUS_RESP_PDU_WRITE_READ_MULTI_REGISTERS_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucMEIType;
    FTE_UINT8       ucDeviceIDCode;
    FTE_UINT8       ucObjectID;
}   FTE_MODBUS_REQ_PDU_READ_DEVICE_IDENT, _PTR_ FTE_MODBUS_REQ_PDU_READ_DEVICE_IDENT_PTR;

typedef struct
{
    FTE_UINT8       ucFunctionCode;
    FTE_UINT8       ucMEIType;
    FTE_UINT8       ucDeviceIDCode;
    FTE_UINT8       ucConformityLevel;
    FTE_UINT8       ucMoreFollows;
    FTE_UINT8       ucNextObjectID;
    FTE_UINT8       ucObjectCount;
    FTE_UINT8       pObjectList[];
}   FTE_MODBUS_RESP_PDU_READ_DEVICE_IDENT, _PTR_ FTE_MODBUS_RESP_PDU_READ_DEVICE_IDENT_PTR;

typedef struct
{
    FTE_UINT8       ucID;
    FTE_UINT8       ucLen;
    FTE_UINT8       pValue[32];
}   FTE_MODBUS_OBJECT_VALUE, _PTR_ FTE_MODBUS_OBJECT_VALUE_PTR;

typedef union
{
    FTE_MODBUS_REQ_PDU_COMMON                       xCommon;
    FTE_MODBUS_REQ_PDU_READ_COILS                   xReadCoils;
    FTE_MODBUS_REQ_PDU_READ_DISCRETE_INPUTS         xReadDiscreteInputs;
    FTE_MODBUS_REQ_PDU_READ_HOLDING_REGISTERS       xReadHoldingRegisters;
    FTE_MODBUS_REQ_PDU_READ_INPUT_REGISTERS         xReadInputRegisters;
    FTE_MODBUS_REQ_PDU_WRITE_SINGLE_COIL            xWriteSingleCoil;
    FTE_MODBUS_REQ_PDU_WRITE_SINGLE_REGISTER        xWriteSingleRegister;
    FTE_MODBUS_REQ_PDU_WRITE_MULTI_COILS            xWriteMultiCoils;
    FTE_MODBUS_REQ_PDU_WRITE_MULTI_REGISTERS        xWriteMultiRegisters;
    FTE_MODBUS_REQ_PDU_WRITE_READ_MULTI_REGISTERS   xWriteReadMultiRegisters;
    FTE_MODBUS_REQ_PDU_READ_DEVICE_IDENT            xReadDeviceIdent;
}   FTE_MODBUS_REQ_PDU, _PTR_ FTE_MODBUS_REQ_PDU_PTR;
    
typedef union
{
    FTE_MODBUS_RESP_PDU_COMMON                      xCommon;
    FTE_MODBUS_RESP_PDU_ERROR                       xError;
    FTE_MODBUS_RESP_PDU_READ_COILS                  xReadCoils;
    FTE_MODBUS_RESP_PDU_READ_DISCRETE_INPUTS        xReadDiscreteInputs;
    FTE_MODBUS_RESP_PDU_READ_HOLDING_REGISTERS      xReadHoldingRegisters;
    FTE_MODBUS_RESP_PDU_READ_INPUT_REGISTERS        xReadInputRegisters;
    FTE_MODBUS_RESP_PDU_WRITE_SINGLE_COIL           xWriteSingleCoil;
    FTE_MODBUS_RESP_PDU_WRITE_SINGLE_REGISTER       xWriteSingleRegister;
    FTE_MODBUS_RESP_PDU_WRITE_MULTI_COILS           xWriteMultiCoils;
    FTE_MODBUS_RESP_PDU_WRITE_MULTI_REGISTERS       xWriteMultiRegisters;
    FTE_MODBUS_RESP_PDU_WRITE_READ_MULTI_REGISTERS  xWriteReadMultiRegisters;
    FTE_MODBUS_RESP_PDU_READ_DEVICE_IDENT           xReadDeviceIdent;
}   FTE_MODBUS_RESP_PDU, _PTR_ FTE_MODBUS_RESP_PDU_PTR;
    
typedef FTE_RET (*FTE_MODBUS_SLAVE_READ_COILS_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_READ_DISCRETE_INPUTS_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pBuff,
    FTE_UINT32      ulBuffLen,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_READ_HOLDING_REGISTERS_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pRegisters,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_READ_INPUT_REGISTERS_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pRegisters,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_WRITE_SINGLE_COIL_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_WRITE_SINGLE_REGISTER_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usValue,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_WRITE_MULTI_COILS_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT8_PTR   pOutputValues,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_WRITE_MULTI_REGISTERS_CB)
(
    FTE_UINT16      usAddress,
    FTE_UINT16      usQuantity,
    FTE_UINT16_PTR  pValues,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_WRITE_READ_MULTI_REGISTERS_CB)
(
    FTE_UINT16      usWriteAddress,
    FTE_UINT16      usWriteQuantity,
    FTE_UINT16_PTR  pWriteValues,
    FTE_UINT16      usReadAddress,
    FTE_UINT16      usReadQuantity,
    FTE_UINT16_PTR  pReadValues,
    FTE_VOID_PTR    pData
);

typedef FTE_RET (*FTE_MODBUS_SLAVE_READ_DEVICE_IDENT_CB)
(
    FTE_UINT8       ucMEIType,
    FTE_UINT8       ucDeviceIDCode,
    FTE_UINT8       ucObjectID,
    FTE_MODBUS_OBJECT_VALUE_PTR ppValues,
    FTE_UINT8       ucMaxValues,
    FTE_UINT8_PTR   pValueCount,
    FTE_UINT8_PTR   pConformityLevel,
    FTE_UINT8_PTR   pMoreFollows,
    FTE_UINT8_PTR   pNextObjectID,
    FTE_VOID_PTR    pData
);

typedef struct FTE_MODBUS_SLAVE_MODEL_STRUCT
{
    FTE_MODBUS_SLAVE_TYPE   xType;
    FTE_MODBUS_SLAVE_READ_COILS_CB                  fReadCoils;
    FTE_MODBUS_SLAVE_READ_DISCRETE_INPUTS_CB        fReadDiscreteInputs;
    FTE_MODBUS_SLAVE_READ_HOLDING_REGISTERS_CB      fReadHoldingRegisters;
    FTE_MODBUS_SLAVE_READ_INPUT_REGISTERS_CB        fReadInputRegisters;
    FTE_MODBUS_SLAVE_WRITE_SINGLE_COIL_CB           fWriteSingleCoil;
    FTE_MODBUS_SLAVE_WRITE_SINGLE_REGISTER_CB       fWriteSingleRegister;
    FTE_MODBUS_SLAVE_WRITE_MULTI_COILS_CB           fWriteMultiCoils;
    FTE_MODBUS_SLAVE_WRITE_MULTI_REGISTERS_CB       fWriteMultiRegisters;
    FTE_MODBUS_SLAVE_WRITE_READ_MULTI_REGISTERS_CB  fWriteReadMultiRegisters;
    FTE_MODBUS_SLAVE_READ_DEVICE_IDENT_CB           fReadDeviceIdent;
}   FTE_MODBUS_SLAVE_MODEL, _PTR_ FTE_MODBUS_SLAVE_MODEL_PTR;

#pragma pack(pop)

typedef struct FTE_MODBUS_SLAVE_CONFIG_STRUCT
{
    FTE_MODBUS_SLAVE_TYPE   xType;    
}   FTE_MODBUS_SLAVE_CONFIG, _PTR_ FTE_MODBUS_SLAVE_CONFIG_PTR;
    
typedef struct
{
    FTE_UINT32              ulRxPkt;
    FTE_UINT32              ulRxError;
    FTE_UINT32              ulTxPkt;
}   FTE_MODBUS_SLAVE_STATISTICS, _PTR_ FTE_MODBUS_SLAVE_STATISTICS_PTR;


typedef struct FTE_MODBUS_SLAVE_STRUCT
{
    FTE_MODBUS_SLAVE_CONFIG     xConfig;
    FTE_MODBUS_SLAVE_MODEL_PTR  pModel;
    FTE_UINT16                  usResetKey;
    
    FTE_MODBUS_SLAVE_STATISTICS xStatistics;
} FTE_MODBUS_SLAVE, _PTR_ FTE_MODBUS_SLAVE_PTR;

FTE_RET FTE_MODBUS_SLAVE_init
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_MODBUS_SLAVE_CONFIG_PTR pConfig
);

FTE_RET FTE_MODBUS_SLAVE_process
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT8_PTR   pReqPDU,
    FTE_UINT32      ulReqPDULen,
    FTE_UINT8_PTR   pRespBuff,
    FTE_UINT32      ulRespBuffLen,
    FTE_UINT32_PTR  pRespPDULen
);

#endif
