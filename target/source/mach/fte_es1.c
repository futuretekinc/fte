#if FTE_MCP23S08_SUPPORTED
const FTE_DI_CONFIG fte_init_mcp23s08_di0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 1),
        .pName      = "DI0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    },
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_1,
};

const FTE_DI_CONFIG fte_init_mcp23s08_di1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 2),
        .pName      = "DI1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    },
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_2,
};

const FTE_DO_CONFIG fte_init_mcp23s08_do0_config =
{ 
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 1), 
        .pName      = "DO0", 
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF, 
    },
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_3,  
};

const FTE_DO_CONFIG fte_init_mcp23s08_do1_config =
{ 
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 2), 
        .pName      = "DO1", 
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF, 
    },
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_4,  
};
#endif




const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] =
{
#if FTE_GPIO_DI_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&FTE_GPIO_DI_defaultConfig,
#endif    
#if FTE_MCP23S08_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_di0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_di1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_do0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_do1_config,
#endif
#if  FTE_RTD_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_RTD_defaultConfig,
#endif
#if FTE_SHT_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_SHT_defaultConfig,
#endif
#if FTE_TASCON_HEM12_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_defaultConfig,
#endif
#if FTE_TASCON_HEM12_06M_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_TASCON_HEM12_06M_defaultConfig,
#endif
#if FTE_SOHA_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_SOHA_MV250_defaultConfig,
#endif
#if FTE_ELT_AQM100_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_ELT_AQM100_defaultConfig,
#endif
#if FTE_COZIR_AX5000_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_COZIR_AX5000_defaultConfig,
#endif
#if FTE_TRUEYES_AIRQ_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_TRUEYES_AIRQ_defaultConfig,
#endif
#if FTE_MST_MEX510C_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_MST_MEX510C_defaultConfig,
#endif
#if FTE_GS_DPC_HL_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_GS_DPC_defaultConfig,
#endif
#if FTE_GAI_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_GAI_VOLTAGE_defaultConfig ,
//    (FTE_OBJECT_CONFIG_PTR)&FTE_GAI_CURRENT_defaultConfig,
#endif

#if FTE_BOTEM_PN1500_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_BOTEM_PN1500_defaultConfigs,
#endif
#if FTE_FTLM_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_FTLM_defaultConfig,
#endif
#if FTE_CIAS_SIOUX_CU_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_CIAS_SIOUX_CU_defaultConfig,
#endif
#if FTE_IOEX_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_IOEX_defaultConfig,
#endif
#if FTE_DOTECH_SUPPORTED
//    (FTE_OBJECT_CONFIG_PTR)&FTE_DOTECH_FX3D_defaultConfig,
#endif
};

#if FTE_GPIO_DI_SUPPORTED
static const FTE_EVENT_CONFIG fte_report_di_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

static const FTE_EVENT_CONFIG fte_occurred_di_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 1,
            .ulThreshold = 0
        }
    }
};


static const FTE_EVENT_CONFIG fte_release_di_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 0,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_changed_di_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};


#endif

#if FTE_MCP23S08_SUPPORTED

static const FTE_EVENT_CONFIG fte_report_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

static const FTE_EVENT_CONFIG fte_occurred_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 1,
            .ulThreshold = 0
        }
    }
};


static const FTE_EVENT_CONFIG fte_release_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 0,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_changed_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};


static const FTE_EVENT_CONFIG fte_report_di1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

static const FTE_EVENT_CONFIG fte_occurred_di1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 1,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_release_di1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 0,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_changed_di1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_report_do0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};


static const FTE_EVENT_CONFIG fte_occurred_do0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 1,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_release_do0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 0,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_changed_do0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_report_do1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

static const FTE_EVENT_CONFIG fte_occurred_do1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 1,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_changed_do1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_release_do1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 0,
            .ulThreshold = 0
        }
    }
};
#endif

#if FTE_TEMP_SUPPORTED
static const FTE_EVENT_CONFIG fte_event_temp_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_TEMPERATURE, 0x0000),
    .xType      = FTE_EVENT_TYPE_LOG | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_OUTSIDE,
    .xParams    =
    {
        .xRange =
        {
            .nUpper = 25,
            .nLower = 20,
            .ulThreshold = 2
        }
    },
    .ulDelayTime=   1000,
    .ulHoldTime =   60000,
};

static const FTE_EVENT_CONFIG fte_report_temp_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_TEMPERATURE, 0x0000),
    .xType      = FTE_EVENT_TYPE_MQTT_PUB,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval =   FTE_EVENT_REPORT_INTERVAL
    }
};
#endif

#if FTE_HUMIDITY_SUPPORTED
static const FTE_EVENT_CONFIG fte_event_humi_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_HUMIDITY, 0x0000),
    .xType      = FTE_EVENT_TYPE_LOG | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_OUTSIDE,
    .xParams    =
    {
        .xRange =
        {
            .nUpper = 80,
            .nLower = 20,
            .ulThreshold = 5
        }
    },
    .ulDelayTime=   1000,
    .ulHoldTime =   60000,
};


static const FTE_EVENT_CONFIG fte_report_humi_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_HUMIDITY, 0x0000),
    .xType      = FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = FTE_EVENT_REPORT_INTERVAL
    }
};
#endif

#if FTE_POWER_SUPPORTED
static const FTE_EVENT_CONFIG fte_report_power_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_POWER, 0x0000),
    .xType      = FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = FTE_EVENT_REPORT_INTERVAL
    }
};
#endif

#if FTE_CO2_SUPPORTED
static const FTE_EVENT_CONFIG fte_report_co2_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = FTE_EVENT_REPORT_INTERVAL
    }
};

static const FTE_EVENT_CONFIG fte_warning_co2_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit =
        {
            .nValue = 1000,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_release_co2_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit =
        {
            .nValue = 1000,
            .ulThreshold = 0
        }
    }
};
#endif

#if FTE_VOLTAGE_SUPPORTED
static const FTE_EVENT_CONFIG fte_report_voltage_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_VOLTAGE, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = FTE_EVENT_REPORT_INTERVAL
    }
};

static const FTE_EVENT_CONFIG fte_warning_voltage_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_VOLTAGE, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit =
        {
            .nValue = 20000,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_release_voltage_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_VOLTAGE, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit =
        {
            .nValue = 20000,
            .ulThreshold = 0
        }
    }
};
#endif

#if FTE_BOTEM_PN1500_SUPPORTED
static const FTE_EVENT_CONFIG fte_report_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

static const FTE_EVENT_CONFIG fte_occurred_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_ABOVE,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 1,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_changed_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_release_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_BELOW,
    .xParams    =
    {
        .xLimit = 
        {
            .nValue = 0,
            .ulThreshold = 0
        }
    }
};

static const FTE_EVENT_CONFIG fte_changed_count_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0x0000),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED
};
#endif

#if FTE_FTLM_SUPPORTED
static const FTE_EVENT_CONFIG fte_changed_led0_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0901),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led1_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0902),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led2_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0903),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led3_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0904),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led4_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0905),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led5_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0906),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led6_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0907),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led7_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0908),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_led8_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AI, 0x0909),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
#endif

#if FTE_CIAS_SIOUX_CU_SUPPORTED
static const FTE_EVENT_CONFIG fte_changed_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b01),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_di1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b02),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di2_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b03),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di3_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b04),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di4_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b05),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di5_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b06),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di6_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b07),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di7_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b08),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di8_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b09),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di9_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b0A),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di10_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b0B),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di11_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b0C),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di12_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b0d),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di13_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b0e),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di14_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b0f),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_di15_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b10),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_line_cutting_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ALARM, 0x0a01),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_line_short_circuit_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ALARM, 0x0a02),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_tamper_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ALARM, 0x0a03),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_fault_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ALARM, 0x0a04),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_alarm_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ALARM, 0x0a05),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone1_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a01),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone3_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a02),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone5_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a03),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone7_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a04),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone9_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a05),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone11_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a06),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone13_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a07),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone15_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a08),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone17_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a09),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_zone19_device_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_CIAS_ZONE, 0x0a0a),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_ALERT,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
#endif


static const FTE_EVENT_CONFIG fte_changed_dio0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b01),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

static const FTE_EVENT_CONFIG fte_changed_dio1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b02),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_dio2_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0b03),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_dio3_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0b04),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_dio4_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0b05),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_dio5_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0b06),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};
static const FTE_EVENT_CONFIG fte_changed_dio6_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0b07),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_WARNING,
    .xCondition = FTE_EVENT_CONDITION_CHANGED,
};

const FTE_CFG_EVENT_PTR pInitEventConfigs[] =
{
#if FTE_GPIO_DI_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_di_config,    
    (FTE_CFG_EVENT_PTR)&fte_changed_di_config,    
#endif    
#if FTE_MCP23S08_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_changed_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_report_di1_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di1_config,    
    (FTE_CFG_EVENT_PTR)&fte_report_do0_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_do0_config,    
    (FTE_CFG_EVENT_PTR)&fte_report_do1_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_do1_config,    
#endif
#if FTE_TEMP_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_event_temp_config,
    (FTE_CFG_EVENT_PTR)&fte_report_temp_config,
#endif
    
#if FTE_HUMIDITY_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_event_humi_config,
    (FTE_CFG_EVENT_PTR)&fte_report_humi_config,
#endif

#if FTE_POWER_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_power_config,
#endif        

#if FTE_CO2_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_co2_config,
    (FTE_CFG_EVENT_PTR)&fte_warning_co2_config,
    (FTE_CFG_EVENT_PTR)&fte_release_co2_config,
#endif
#if FTE_VOLTAGE_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_voltage_config,
    (FTE_CFG_EVENT_PTR)&fte_warning_voltage_config,
    (FTE_CFG_EVENT_PTR)&fte_release_voltage_config,
#endif
#if FTE_BOTEM_PN1500_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_changed_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_changed_count_config,
#endif
#if FTE_FTLM_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_changed_led0_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led1_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led2_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led3_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led4_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led5_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led6_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led7_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_led8_config,
#endif
#if FTE_CIAS_SIOUX_CU_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_changed_di0_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di1_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di2_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di3_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di4_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di5_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di6_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di7_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di8_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di9_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di10_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di11_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di12_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di13_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di14_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_di15_config,
#if 0
    (FTE_CFG_EVENT_PTR)&fte_changed_line_cutting_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_line_short_circuit_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_tamper_device_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_fault_device_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_alarm_device_config,
#endif
    (FTE_CFG_EVENT_PTR)&fte_changed_zone1_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone3_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone5_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone7_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone9_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone11_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone13_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone15_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone17_device_config, 
    (FTE_CFG_EVENT_PTR)&fte_changed_zone19_device_config, 
#endif
	(FTE_CFG_EVENT_PTR)&fte_changed_dio0_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_dio1_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_dio2_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_dio3_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_dio4_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_dio5_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_dio6_config,
};
