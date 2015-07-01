#if FTE_GPIO_DI_SUPPORTED
const FTE_DI_CONFIG fte_init_gpio_di0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 1),
        .pName      = "DI0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    },
    .nGPIO      = FTE_DEV_GPIO_DI_0,
};
#endif

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

#if  FTE_RTD0_SUPPORTED
static const FTE_RTD_CONFIG fte_init_rtd0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_PT100, 0x0001),
        .pName      = "RTD-1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = FTE_DEV_AD7785_0,
    .nInterval  = FTE_RTD_INTERVAL
};
#endif

#if  FTE_RTD1_SUPPORTED
static const FTE_RTD_CONFIG fte_init_rtd1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_PT100, 0x0002),
        .pName      = "RTD-2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = FTE_DEV_AD7785_1,
    .nInterval  = FTE_RTD_INTERVAL
};
#endif

#if FTE_SHT_SUPPORTED
static const FTE_IFCE_CONFIG fte_init_sht_humi_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0x0000),
        .pName      = "SHT-HUMI",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SHT, 0x0001),
    .nRegID     = 0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_sht_temp_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0x0000),
        .pName      = "SHT-TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SHT, 0x0000),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_OBJECT_CONFIG_PTR fte_init_sht_child_configs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sht_humi_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sht_temp_config
};

static const FTE_SHT_CONFIG fte_init_sht_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_SHT, 0x0001),
        .pName      = "SHT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
        .ulChild    = sizeof(fte_init_sht_child_configs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)fte_init_sht_child_configs
    },
    .nSDA       = FTE_DEV_LWGPIO_SHT_SDA,
    .nSCL       = FTE_DEV_LWGPIO_SHT_SCL,
    .nInterval  = FTE_SHT_INTERVAL,
    .ulDelay    = FTE_SHT_SCL_INTERVAL
};
#endif

#if FTE_TASCON_HEM12_SUPPORTED
static const FTE_GUS_CONFIG fte_init_tascon_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_POWER, 0x0101),
        .pName      = "TASCON HEM12",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_HEM12_INTERVAL,
};
#endif

#if FTE_TASCON_HEM12_06M_SUPPORTED
static const FTE_IFCE_CONFIG    fte_init_tascon_hem12_06m_kwh_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_POWER, 0x0000),
        .pName      = "HEM12-POWER(Wh)",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0x0000),
    .nRegID     = 0,
    .nInterval  = 1
};

static const FTE_IFCE_CONFIG    fte_init_tascon_hem12_06m_pwr_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_POWER, 0x0000),
        .pName      = "HEM12-POWER(W)",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0x0000),
    .nRegID     = 1,
    .nInterval  = 1
};

static const FTE_IFCE_CONFIG    fte_init_tascon_hem12_06m_voltage_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VOLTAGE, 0x0000),
        .pName      = "MEM12-VOLTAGE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0x0000),
    .nRegID     = 2,
    .nInterval  = 1
};

static const FTE_IFCE_CONFIG    fte_init_tascon_hem12_06m_current_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CURRENT, 0x0000),
        .pName      = "HEM12-CURRENT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0x0000),
    .nRegID     = 3,
    .nInterval  = 1
};


static const FTE_OBJECT_CONFIG_PTR fte_init_tascon_hem12_06m_child_configs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_hem12_06m_kwh_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_hem12_06m_pwr_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_hem12_06m_voltage_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_hem12_06m_current_config,
};

static const FTE_HEM12_06M_CONFIG fte_init_tascon_hem12_06m_0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0),
        .pName      = "HEM12-AGU",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
        .ulChild    = sizeof(fte_init_tascon_hem12_06m_child_configs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)fte_init_tascon_hem12_06m_child_configs
    },
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12_06M,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_HEM12_INTERVAL,
    .pSensorID  = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
};

static const FTE_HEM12_06M_CONFIG fte_init_tascon_hem12_06m_1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0),
        .pName      = "HEM12-AGU",
        .xFlags     = 0, 
        .ulChild    = sizeof(fte_init_tascon_hem12_06m_child_configs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)fte_init_tascon_hem12_06m_child_configs
    },
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12_06M,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_HEM12_INTERVAL,
    .pSensorID  = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB},
};

static const FTE_HEM12_06M_CONFIG fte_init_tascon_hem12_06m_2_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HEM12_06M, 0),
        .pName      = "HEM12-AGU",
        .xFlags     = 0, 
        .ulChild    = sizeof(fte_init_tascon_hem12_06m_child_configs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)fte_init_tascon_hem12_06m_child_configs
    },
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12_06M,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_HEM12_INTERVAL,
    .pSensorID  = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAC},
};

#endif

#if FTE_SH_MV250_SUPPORTED
static const FTE_IFCE_CONFIG    fte_init_sh_mv250_co2_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CO2, 0x0000),
        .pName      = "MV250-CO2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0x0000),
    .nRegID     = 0,
    .nInterval  = FTE_SH_MV250_INTERVAL
};

static const FTE_IFCE_CONFIG    fte_init_sh_mv250_temp_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0x0000),
        .pName      = "MV250-TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0x0000),
    .nRegID     = 1,
    .nInterval  = FTE_SH_MV250_INTERVAL
};

static const FTE_IFCE_CONFIG    fte_init_sh_mv250_humi_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0x0000),
        .pName      = "MV250-HUMI",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0x0000),
    .nRegID     = 2,
    .nInterval  = FTE_SH_MV250_INTERVAL
};
static const FTE_OBJECT_CONFIG_PTR fte_init_sh_mv250_child_configs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sh_mv250_co2_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sh_mv250_temp_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sh_mv250_humi_config
};

static const FTE_GUS_CONFIG fte_init_sh_mv250_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_SH_MV250, 0x0000),
        .pName      = "MV250",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
        .ulChild    = sizeof(fte_init_sh_mv250_child_configs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)fte_init_sh_mv250_child_configs
    },
    .nModel     = FTE_GUS_MODEL_SH_MV250,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_SH_MV250_INTERVAL,
};

#endif

#if FTE_ELT_AQM100_SUPPORTED

static const FTE_IFCE_CONFIG fte_init_elt_aqm100_co2_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CO2, 0x0001),
        .pName      = "AQM100-CO2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0x0001),
    .nRegID     = 0,
    .nInterval  = FTE_ELT_AQM100_INTERVAL
};

static const FTE_IFCE_CONFIG fte_init_elt_aqm100_temp_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0x0001),
        .pName      = "AQM100-TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0x0001),
    .nRegID     = 1,
    .nInterval  = FTE_ELT_AQM100_INTERVAL
};

static const FTE_IFCE_CONFIG fte_init_elt_aqm100_humi_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0x0001),
        .pName      = "AQM100-HUMI",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0x0001),
    .nRegID     = 2,
    .nInterval  = FTE_ELT_AQM100_INTERVAL
};

static const FTE_IFCE_CONFIG fte_init_elt_aqm100_voc_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VOC, 0x0001),
        .pName      = "AQM100-VOC",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0x0001),
    .nRegID     = 3,
    .nInterval  = FTE_ELT_AQM100_INTERVAL
};

static const FTE_OBJECT_CONFIG_PTR fte_init_elt_aqm100_child_configs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_elt_aqm100_co2_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_elt_aqm100_temp_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_elt_aqm100_humi_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_elt_aqm100_voc_config
};

static const FTE_GUS_CONFIG fte_init_elt_aqm100_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AQM100, 0x0001),
        .pName      = "AQM100",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
        .ulChild    = sizeof(fte_init_elt_aqm100_child_configs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)fte_init_elt_aqm100_child_configs
    },
    .nModel     = FTE_GUS_MODEL_ELT_AQM100,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_ELT_AQM100_INTERVAL,
};

#endif

#if FTE_COZIR_AX5000_SUPPORTED
static const FTE_GUS_CONFIG fte_init_cozir_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
        .pName      = "CO2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nModel     = FTE_GUS_MODEL_COZIR_AX5000,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_AX5000_INTERVAL,
};
#endif

#if FTE_TRUEYES_AIRQ_SUPPORTED

static const FTE_IFCE_CONFIG xAirQTemperatureConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0x0001),
        .pName      = "AIRQ-TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AIRQ, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xAirQHumidityConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_HUMI, 0x0001),
        .pName      = "AIRQ-HUMI",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AIRQ, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xAirQCO2Config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_CO2, 0x0001),
        .pName      = "AIRQ-CO2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_AIRQ, 0x0001),
    .nRegID     = 0,
    .nInterval  = 2
};

static const FTE_OBJECT_CONFIG_PTR pAirQChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&xAirQTemperatureConfig,
    (FTE_OBJECT_CONFIG_PTR)&xAirQHumidityConfig,
    (FTE_OBJECT_CONFIG_PTR)&xAirQCO2Config
};

static const FTE_GUS_CONFIG fte_init_airq_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_AIRQ, 0x0001),
        .pName      = "AIRQ",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
        .ulChild    = sizeof(pAirQChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)pAirQChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_TRUEYES_AIRQ, 
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_TRUEYES_AIRQ_INTERVAL
};

#endif


#if FTE_MST_MEX510C_SUPPORTED
static const FTE_IFCE_CONFIG xMEX510CTemperatureConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_TEMPERATURE,
    .nInterval  = 2
};


static const FTE_IFCE_CONFIG xMEX510CSetTemperatureConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "SET TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_SET_TEMPERATURE,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CSetHighTemperatureConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "SET HIGH TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_SET_HIGH_TEMP,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CSetLowTemperatureConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "SET LOW TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_SET_LOW_TEMP,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CCoolTemperatureDeviationConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "COOL DEVIATION",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_COOL_TEMP_DEVIATION,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CHeatTemperatureDeviationConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "HEAT DEVIATION",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_HEAT_TEMP_DEVIATION,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510COperationStateConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "STATE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_OPERATION,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CAlarmConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "ALARM",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_ALARM,
    .nInterval  = 2
};


static const FTE_IFCE_CONFIG xMEX510CStateHeatConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "HEAT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_HEAT,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CStateDHeatConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "D.HEAT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_DHEAT,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CStateFANConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "FAN",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_FAN,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CStateSolvalveConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "SOLVALVE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_SOLVALVE,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CStateCompConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "COMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_STATE_COMP,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlEnableConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL ENABLE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_ENABLE,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlAConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL A",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_A,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlBConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL B",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_B,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlCConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL C",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_C,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlDConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL D",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_D,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlEConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL E",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_E,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlFConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL F",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_F,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlGConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL G",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_G,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG xMEX510CDirectCtrlHConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "CTRL H",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
    .nRegID     = FTE_MST_MEX510C_INDEX_DIRECT_CTRL_H,
    .nInterval  = 2
};

static const FTE_OBJECT_CONFIG_PTR xMEX510CChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CTemperatureConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CSetTemperatureConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CSetHighTemperatureConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CSetLowTemperatureConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CCoolTemperatureDeviationConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CHeatTemperatureDeviationConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510COperationStateConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CAlarmConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CStateHeatConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CStateDHeatConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CStateFANConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CStateSolvalveConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CStateCompConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlEnableConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlAConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlBConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlCConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlDConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlEConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlFConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlGConfig,
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CDirectCtrlHConfig
};

static const FTE_GUS_CONFIG xMEX510CConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_MEX510C, 0x0001),
        .pName      = "MEX-510C",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
        .ulChild    = sizeof(xMEX510CChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)xMEX510CChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_MST_MEX510C,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_MST_MEX510C_INTERVAL
};

#endif

#if FTE_GS_DPC_HL_SUPPORTED


static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_hps_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "HPS",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HPS_STATE,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_lps_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "LPS",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LPS_STATE,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_hpc_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0),
        .pName      = "HPC",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HPC_STATE,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_hps_error_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "HPS ERROR",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HPS_ERROR,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_lps_error_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0),
        .pName      = "LPS ERROR",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LPS_ERROR,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_hp_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_PRESSURE, 0),
        .pName      = "HP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HP,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_lp_config  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_PRESSURE, 0),
        .pName      = "LP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LP,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_hp_temp_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "HP TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_HP_TEMP,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG fte_init_gs_dpc_hl_lp_temp_config  =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_TEMP, 0),
        .pName      = "lP TEMP",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
    .nRegID     = FTE_GS_DPC_HL_INDEX_LP_TEMP,
    .nInterval  = 2
};

static const FTE_OBJECT_CONFIG_PTR fte_init_gs_dpc_hl_child_configs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_hps_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_lps_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_hpc_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_hps_error_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_lps_error_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_hp_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_lp_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_hp_temp_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_lp_temp_config
};

static const FTE_GUS_CONFIG fte_init_gs_dpc_hl_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DPC_HL, 0x0001),
        .pName      = "DPC-HL",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
        .ulChild    = sizeof(fte_init_gs_dpc_hl_child_configs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)fte_init_gs_dpc_hl_child_configs
    },
    .nModel     = FTE_GUS_MODEL_GS_DPC_HL,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = 5
};
#endif

#if FTE_GAI_VOLTAGE0_SUPPORTED
static const FTE_GAI_CONFIG fte_init_gai_voltage0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_GAI_VOLTAGE, 0),
        .pName      = "VOLTAGE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID         = FTE_DEV_AD7785_0,
    .nInterval      = 2,
    .nGain          = 1,
    .ulDivide       = 1,
    .xValueType     = FTE_GAI_VALUE_TYPE_0_2_TO_1_518V
};
#endif

#if FTE_GAI_VOLTAGE1_SUPPORTED
static const FTE_GAI_CONFIG fte_init_gai_voltage1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_GAI_VOLTAGE, 0),
        .pName      = "VOLTAGE",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID         = FTE_DEV_AD7785_1,
    .nInterval      = 2,
    .nGain          = 1,
    .ulDivide       = 1,
    .xValueType     = FTE_GAI_VALUE_TYPE_0_2_TO_1_518V
};
#endif

#if FTE_GAI_CURRENT0_SUPPORTED
static const FTE_GAI_CONFIG fte_init_gai_current0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_GAI_CURRENT, 0),
        .pName      = "CURRENT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID         = FTE_DEV_AD7785_0,
    .nInterval      = 2,
    .nGain          = 1,
    .ulDivide       = 1,
    .xValueType     = FTE_GAI_VALUE_TYPE_0_2_TO_1_10A
};
#endif

#if FTE_GAI_CURRENT1_SUPPORTED
static const FTE_GAI_CONFIG fte_init_gai_current1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_GAI_CURRENT, 0),
        .pName      = "CURRENT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID         = FTE_DEV_AD7785_1,
    .nInterval      = 2,
    .nGain          = 1,
    .ulDivide       = 1,
    .xValueType     = FTE_GAI_VALUE_TYPE_0_2_TO_1_10A
};
#endif

#if FTE_BOTEM_PN1500_SUPPORTED
static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_human_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0x0001),
        .pName      = "COUNT",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_human_accum_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0x0001),
        .pName      = "ACCUM",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_accum_reset_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0001),
        .pName      = "ACCUM RESET",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};


static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_switch_stat_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0001),
        .pName      = "SWITCH",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_switch_ctrl_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0001),
        .pName      = "SWITCH",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};


static const FTE_OBJECT_CONFIG_PTR xPN1500ChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_human_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_human_accum_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_accum_reset_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_switch_stat_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_switch_ctrl_config,
};

static const FTE_GUS_CONFIG fte_init_botem_pn1500_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_PN1500, 0x0001),
        .pName      = "PN1500",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
        .ulChild    = sizeof(xPN1500ChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)xPN1500ChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_BOTEM_PN1500,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = 2
};
#endif

#if FTE_FTLM_SUPPORTED
static const FTE_IFCE_CONFIG ifte_init_ftlm_led0_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led1_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led2_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led3_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED3",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 3,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led4_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED4",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 4,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led5_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED5",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 5,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led6_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED6",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 6,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led7_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED7",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 7,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_ftlm_led8_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_VALUE, 0x0001),
        .pName      = "LED8",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    },
    .nDevID     = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
    .nRegID     = 8,
    .nInterval  = 2
};

static const FTE_OBJECT_CONFIG_PTR xFTLMChildConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led0_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led1_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led2_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led3_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led4_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led5_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led6_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led7_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_ftlm_led8_config
};

static const FTE_GUS_CONFIG fte_init_ftlm_config =
{
    .xCommon    =
    {
        .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_FTLM, 0x0001),
        .pName      = "FTLM",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
        .ulChild    = sizeof(xFTLMChildConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
        .pChild     = (FTE_OBJECT_CONFIG_PTR _PTR_)xFTLMChildConfigs
    },
    .nModel     = FTE_GUS_MODEL_FTLM,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = 2
};
#endif

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] =
{
#if FTE_GPIO_DI_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gpio_di0_config,
#endif    
#if FTE_MCP23S08_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_di0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_di1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_do0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mcp23s08_do1_config,
#endif
#if  FTE_RTD0_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rtd0_config,
#endif
#if  FTE_RTD1_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rtd1_config,
#endif
#if FTE_SHT_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sht_config,
#endif
#if FTE_TASCON_HEM12_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_config,
#endif
#if FTE_TASCON_HEM12_06M_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_hem12_06m_0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_hem12_06m_1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_hem12_06m_2_config,
#endif
#if FTE_SH_MV250_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sh_mv250_config,
#endif
#if FTE_ELT_AQM100_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_elt_aqm100_config,
#endif
#if FTE_COZIR_AX5000_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_cozir_config,
#endif
#if FTE_TRUEYES_AIRQ_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_airq_config,
#endif
#if FTE_MST_MEX510C_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&xMEX510CConfig,
#endif
#if FTE_GS_DPC_HL_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gs_dpc_hl_config,
#endif
#if FTE_GAI_VOLTAGE0_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gai_voltage0_config,
#endif
#if FTE_GAI_VOLTAGE1_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gai_voltage1_config,
#endif
#if FTE_GAI_CURRENT0_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gai_current0_config,
#endif
#if FTE_GAI_CURRENT1_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gai_current1_config,
#endif

#if FTE_BOTEM_PN1500_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_botem_pn1500_config,
#endif
#if FTE_FTLM_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_ftlm_config,
#endif
};

#if FTE_GPIO_DI_SUPPORTED
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
        .ulInterval = 5
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

static const FTE_EVENT_CONFIG fte_report_di1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 5
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

static const FTE_EVENT_CONFIG fte_report_do0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 5
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

static const FTE_EVENT_CONFIG fte_report_do1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_SNMP_TRAP | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 5
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
    .ulDelayTime=   1,
    .ulHoldTime =   60,
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
    .ulDelayTime=   1,
    .ulHoldTime =   60,
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

const FTE_CFG_EVENT_PTR pInitEventConfigs[] =
{
#if FTE_GPIO_DI_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_occurred_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_release_di0_config,    
#endif    
#if FTE_MCP23S08_SUPPORTED
    (FTE_CFG_EVENT_PTR)&fte_report_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_occurred_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_release_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_report_di1_config,
    (FTE_CFG_EVENT_PTR)&fte_occurred_di1_config,    
    (FTE_CFG_EVENT_PTR)&fte_release_di1_config,    
    (FTE_CFG_EVENT_PTR)&fte_report_do0_config,
    (FTE_CFG_EVENT_PTR)&fte_occurred_do0_config,    
    (FTE_CFG_EVENT_PTR)&fte_release_do0_config,    
    (FTE_CFG_EVENT_PTR)&fte_report_do1_config,
    (FTE_CFG_EVENT_PTR)&fte_occurred_do1_config,    
    (FTE_CFG_EVENT_PTR)&fte_release_do1_config,    
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
    (FTE_CFG_EVENT_PTR)&fte_occurred_di0_config,    
    (FTE_CFG_EVENT_PTR)&fte_release_di0_config,
    (FTE_CFG_EVENT_PTR)&fte_changed_count_config
#endif
};
