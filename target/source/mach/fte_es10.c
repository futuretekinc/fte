static const FTE_GUS_CONFIG fte_init_elt_aqm100_sens_config =
{
    .nID        = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .pName      = "CO2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nModel     = FTE_GUS_MODEL_ELT_AQM100,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_ELT_AQM100_INTERVAL,
};

static const FTE_IFCE_CONFIG ifte_init_elt_aqm100_temp_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_ELT_AQM100_TEMP, 0x0001),
    .pName      = "TEMP",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_elt_aqm100_humi_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_ELT_AQM100_HUMI, 0x0001),
    .pName      = "HUMI",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_elt_aqm100_co2_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0001),
    .pName      = "CO2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_elt_aqm100_voc_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_VOC, 0x0001),
    .pName      = "VOC",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 3,
    .nInterval  = 2
};

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_elt_aqm100_sens_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_elt_aqm100_co2_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_elt_aqm100_voc_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_elt_aqm100_temp_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_elt_aqm100_humi_config
};


static const FTE_EVENT_CONFIG fte_report_aqm100_co2_config =
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

static const FTE_EVENT_CONFIG fte_warning_aqm100_co2_config =
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

static const FTE_EVENT_CONFIG fte_release_aqm100_co2_config =
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

const FTE_CFG_EVENT_PTR pInitEventConfigs[] =
{
    (FTE_CFG_EVENT_PTR)&fte_report_aqm100_co2_config,
    (FTE_CFG_EVENT_PTR)&fte_warning_aqm100_co2_config,
    (FTE_CFG_EVENT_PTR)&fte_release_aqm100_co2_config
};