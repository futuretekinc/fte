#if FTE_SH_MV250_SUPPORTED
static const FTE_GUS_CONFIG fte_init_mv250_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
    .pName      = "CO2",
    .nModel     = FTE_GUS_MODEL_SH_MV250,
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_SH_MV250_INTERVAL,
    .nEventType = FTE_SH_MV250_EVENT_TYPE_ABOVE,
    .nUpperLimit= FTE_SH_MV250_EVENT_UPPER_LIMIT,
    .nLowerLimit= FTE_SH_MV250_EVENT_LOWER_LIMIT,
    .nThreshold = FTE_SH_MV250_EVENT_THRESHOLD,
    .nEventDelay= FTE_SH_MV250_EVENT_DELAY
};
#endif

#if FTE_ELT_AQM100_SUPPORTED
static const FTE_GUS_CONFIG fte_init_aqm100_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
    .pName      = "CO2",
    .nModel     = FTE_GUS_MODEL_ELT_AQM100,
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_ELT_AQM100_INTERVAL,
    .nEventType = FTE_ELT_AQM100_EVENT_TYPE_ABOVE,
    .nUpperLimit= FTE_ELT_AQM100_EVENT_UPPER_LIMIT,
    .nLowerLimit= FTE_ELT_AQM100_EVENT_LOWER_LIMIT,
    .nThreshold = FTE_ELT_AQM100_EVENT_THRESHOLD,
    .nEventDelay= FTE_ELT_AQM100_EVENT_DELAY
};
#endif

#if FTE_COZIR_AX5000_SUPPORTED
static const FTE_GUS_CONFIG fte_init_cozir_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
    .pName      = "CO2",
    .nModel     = FTE_GUS_MODEL_COZIR_AX5000,
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_AX5000_INTERVAL,
    .nEventType = FTE_AX5000_EVENT_TYPE_ABOVE,
    .nUpperLimit= FTE_AX5000_EVENT_UPPER_LIMIT,
    .nLowerLimit= FTE_AX5000_EVENT_LOWER_LIMIT,
    .nThreshold = FTE_AX5000_EVENT_THRESHOLD,
    .nEventDelay= FTE_AX5000_EVENT_DELAY
};
#endif

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] =
{
#if FTE_SH_MV250_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_mv250_config,
#endif
#if FTE_ELT_AQM100_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_aqm100_config,
#endif
#if FTE_COZIR_AX5000_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_init_cozir_config,
#endif
};


static const FTE_EVENT_CONFIG fte_report_co2_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

static const FTE_EVENT_CONFIG fte_warning_co2_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
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
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
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

const FTE_CFG_EVENT_PTR pInitEventConfigs[] =
{
    (FTE_CFG_EVENT_PTR)&fte_report_co2_config,
    (FTE_CFG_EVENT_PTR)&fte_warning_co2_config,
    (FTE_CFG_EVENT_PTR)&fte_release_co2_config
};