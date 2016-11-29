#include <rtcs.h>
#include "fte_target.h"
#include "fte_modbus.h"
#include "fte_object.h"
#include "fte_modbus_registers.h"

typedef struct
{
    FTE_RET (*fRead)(FTE_MODBUS_SLAVE_PTR pSlave, FTE_UINT16_PTR pValue);
    FTE_RET (*fWrite)(FTE_MODBUS_SLAVE_PTR pSlave, FTE_UINT16    pValue);
}   FTE_MODBUS_REGISTER_CB, _PTR_ FTE_MODBUS_REGISTER_CB_PTR;

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readTemperatureCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_TEMPERATURE, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readHumidityCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_HUMIDITY, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readVoltageCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_VOLTAGE, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readCurrentCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_CURRENT, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readDICount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_DI, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readDOCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_DO, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readGASCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_GAS, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readPowerCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_POWER, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readSRFCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_SRF, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readAICount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_AI, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readCounterCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_COUNT, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readPressureCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_PRESSURE, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readDiscreteCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_DISCRETE, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readTempCtrlCount
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR  pValue
)
{
    *pValue = FTE_OBJ_count(FTE_OBJ_CLASS_TEMP_CTRL, FTE_OBJ_CLASS_MASK, FALSE);
    
    return  FTE_RET_OK;
}

const
FTE_MODBUS_REGISTER_CB  pSensorRegisters[] =
{
    
    [FTE_MODBUS_REG_TEMPERATURE_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readTemperatureCount
    },
    [FTE_MODBUS_REG_HUMIDITY_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readHumidityCount
    },
    [FTE_MODBUS_REG_VOLTAGE_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readVoltageCount
    },
    [FTE_MODBUS_REG_CURRENT_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readCurrentCount
    },
    [FTE_MODBUS_REG_DI_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readDICount
    },
    [FTE_MODBUS_REG_DO_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readDOCount
    },
    [FTE_MODBUS_REG_GAS_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readGASCount
    },
    [FTE_MODBUS_REG_POWER_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readPowerCount
    },
    [FTE_MODBUS_REG_SRF_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readSRFCount
    },
    [FTE_MODBUS_REG_AI_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readAICount
    },
    [FTE_MODBUS_REG_COUNTER_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readCounterCount
    },
    [FTE_MODBUS_REG_PRESSURE_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readPressureCount
    },
    [FTE_MODBUS_REG_DISCRETE_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readDiscreteCount
    },
    [FTE_MODBUS_REG_TEMP_CTRL_COUNT] = 
    {
        .fRead = FTE_MODBUS_SLAVE_GEN_readTempCtrlCount
    },
};

/******************************************************************************/

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readReset
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR          pValue
)
{
    ASSERT(pSlave != NULL);
    
    pSlave->usResetKey = (RTCS_rand() & 0xFFFF);
    
    *pValue = pSlave->usResetKey;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeReset
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16  usValue
)
{
    if ((usValue != 0) && (usValue == pSlave->usResetKey))
    {
        FTE_SYS_reset();
    }
    else
    {
        pSlave->usResetKey = (RTCS_rand() & 0xFFFF);
    }
    
    return  FTE_RET_OK;
}

const
FTE_MODBUS_REGISTER_CB  pSystemRegisters[] =
{
    
    [FTE_MODBUS_REG_RESET] = 
    {
        .fRead  = FTE_MODBUS_SLAVE_GEN_readReset,
        .fWrite = FTE_MODBUS_SLAVE_GEN_writeReset
    }
};

/******************************************************************************/

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readRxCountLo
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR          pValue
)
{
    ASSERT(pSlave != NULL);
    
    *pValue = pSlave->xStatistics.ulRxPkt % 10000;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeRxCountLo
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16  usValue
)
{
    if (usValue != 0)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }

    pSlave->xStatistics.ulRxPkt = 0;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readRxCountHi
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR          pValue
)
{
    ASSERT(pSlave != NULL);
    
    *pValue = pSlave->xStatistics.ulRxPkt / 10000;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeRxCountHi
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16  usValue
)
{
    if (usValue != 0)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }

    pSlave->xStatistics.ulRxPkt = 0;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readTxCountLo
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR          pValue
)
{
    ASSERT(pSlave != NULL);
    
    *pValue = pSlave->xStatistics.ulTxPkt % 10000;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeTxCountLo
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16  usValue
)
{
    if (usValue != 0)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }

    pSlave->xStatistics.ulTxPkt = 0;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readTxCountHi
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR          pValue
)
{
    ASSERT(pSlave != NULL);
    
    *pValue = pSlave->xStatistics.ulTxPkt / 10000;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeTxCountHi
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16  usValue
)
{
    if (usValue != 0)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }

    pSlave->xStatistics.ulTxPkt = 0;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readErrorCountLo
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR          pValue
)
{
    ASSERT(pSlave != NULL);
    
    *pValue = pSlave->xStatistics.ulRxError % 10000;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeErrorCountLo
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16  usValue
)
{
    if (usValue != 0)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }

    pSlave->xStatistics.ulRxError  = 0;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_readErrorCountHi
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16_PTR          pValue
)
{
    ASSERT(pSlave != NULL);
    
    *pValue = pSlave->xStatistics.ulRxError  / 10000;
    
    return  FTE_RET_OK;
}

static
FTE_RET FTE_MODBUS_SLAVE_GEN_writeErrorCountHi
(
    FTE_MODBUS_SLAVE_PTR    pSlave,
    FTE_UINT16  usValue
)
{
    if (usValue != 0)
    {
        return  FTE_RET_MODBUS_INVALID_DATA;
    }

    pSlave->xStatistics.ulRxError  = 0;
    
    return  FTE_RET_OK;
}

const
FTE_MODBUS_REGISTER_CB  pStatisticsRegisters[] =
{    
    [FTE_MODBUS_REG_RX_COUNT_LO] = 
    {
        .fRead  = FTE_MODBUS_SLAVE_GEN_readRxCountLo,
        .fWrite = FTE_MODBUS_SLAVE_GEN_writeRxCountLo
    },
    [FTE_MODBUS_REG_RX_COUNT_HI] = 
    {
        .fRead  = FTE_MODBUS_SLAVE_GEN_readRxCountHi,
        .fWrite = FTE_MODBUS_SLAVE_GEN_writeRxCountHi
    },
    [FTE_MODBUS_REG_TX_COUNT_LO] = 
    {
        .fRead  = FTE_MODBUS_SLAVE_GEN_readTxCountLo,
        .fWrite = FTE_MODBUS_SLAVE_GEN_writeTxCountLo
    },
    [FTE_MODBUS_REG_TX_COUNT_HI] = 
    {
        .fRead  = FTE_MODBUS_SLAVE_GEN_readTxCountHi,
        .fWrite = FTE_MODBUS_SLAVE_GEN_writeTxCountHi
    },
    [FTE_MODBUS_REG_ERROR_COUNT_LO] = 
    {
        .fRead  = FTE_MODBUS_SLAVE_GEN_readErrorCountLo,
        .fWrite = FTE_MODBUS_SLAVE_GEN_writeErrorCountLo
    },
    [FTE_MODBUS_REG_ERROR_COUNT_HI] = 
    {
        .fRead  = FTE_MODBUS_SLAVE_GEN_readErrorCountHi,
        .fWrite = FTE_MODBUS_SLAVE_GEN_writeErrorCountHi
    },
};

/******************************************************************************/
FTE_RET FTE_MODBUS_REG_read(FTE_UINT16   usReg, FTE_UINT16_PTR pValue, FTE_VOID_PTR pData)
{
    FTE_INT nGroup = usReg / 100;
    FTE_INT nIndex = usReg % 100;
    
    switch(nGroup)
    {
    case    FTE_MODBUS_REG_SYSTEM_BASE:
        {
            if ((nIndex < sizeof(pSystemRegisters) / sizeof(FTE_MODBUS_REGISTER_CB)) && (pSystemRegisters[nIndex].fRead != NULL))
            {
                return  pSystemRegisters[nIndex].fRead((FTE_MODBUS_SLAVE_PTR)pData, pValue);
            }
        }
        break;
        
    case    FTE_MODBUS_REG_SENSOR_BASE:
        {
            if ((nIndex < sizeof(pSensorRegisters) / sizeof(FTE_MODBUS_REGISTER_CB)) && (pSensorRegisters[nIndex].fRead != NULL))
            {
                return  pSensorRegisters[nIndex].fRead((FTE_MODBUS_SLAVE_PTR)pData, pValue);
            }
        }
        break;

    case    FTE_MODBUS_REG_STATISTICS_BASE:
        {
            if ((nIndex < sizeof(pStatisticsRegisters) / sizeof(FTE_MODBUS_REGISTER_CB)) && (pStatisticsRegisters[nIndex].fRead != NULL))
            {
                return  pStatisticsRegisters[nIndex].fRead((FTE_MODBUS_SLAVE_PTR)pData, pValue);
            }
        }
        break;
    }
    
    *pValue = 0;
    
    return  FTE_RET_MODBUS_INVALID_ADDRESS;
}

FTE_RET FTE_MODBUS_REG_write(FTE_UINT16   usReg, FTE_UINT16   usValue, FTE_VOID_PTR pData)
{
    FTE_INT nGroup = usReg / 100;
    FTE_INT nIndex = usReg % 100;
    
    switch(nGroup)
    {
    case    FTE_MODBUS_REG_SENSOR_BASE:
        {
            if ((nIndex < sizeof(pSensorRegisters) / sizeof(FTE_MODBUS_REGISTER_CB)) && (pSensorRegisters[nIndex].fWrite != NULL))
            {
                return  pSensorRegisters[nIndex].fWrite((FTE_MODBUS_SLAVE_PTR)pData, usValue);
            }
        }
        break;
            
    case    FTE_MODBUS_REG_SYSTEM_BASE:
        {
            if ((nIndex < sizeof(pSystemRegisters) / sizeof(FTE_MODBUS_REGISTER_CB)) && (pSystemRegisters[nIndex].fWrite != NULL))
            {
                return  pSystemRegisters[nIndex].fWrite((FTE_MODBUS_SLAVE_PTR)pData, usValue);
            }
        }
        break;
            
    case    FTE_MODBUS_REG_STATISTICS_BASE:
        {
            if ((nIndex < sizeof(pStatisticsRegisters) / sizeof(FTE_MODBUS_REGISTER_CB)) && (pStatisticsRegisters[nIndex].fWrite != NULL))
            {
                return  pStatisticsRegisters[nIndex].fWrite((FTE_MODBUS_SLAVE_PTR)pData, usValue);
            }
        }
        break;
    }
    
    return  FTE_RET_MODBUS_INVALID_ADDRESS;
}
