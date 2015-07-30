static const FTE_GUS_CONFIG fte_init_tureyes_multi_sens_config =
{
    .nID        = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .pName      = "CO2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nModel     = FTE_GUS_MODEL_TRUEYES_MULTI_SENS,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_TRUEYES_MULTI_SENS_INTERVAL
};

static const FTE_IFCE_CONFIG ifte_init_trueeyes_temp_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_TRUEYES_TEMP, 0x0001),
    .pName      = "TEMP",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_trueeyes_humi_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_TRUEYES_HUMI, 0x0001),
    .pName      = "HUMI",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_trueeyes_co2_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0001),
    .pName      = "CO2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 0,
    .nInterval  = 2
};

static const FTE_EVENT_CONFIG ifte_event_temp_config = 
{
    .ulOID      = MAKE_ID(FTE_OBJ_TYPE_TRUEYES_TEMP, 0x0001),
    .xType      = FTE_EVENT_TYPE_OUTSIDE,
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
    .ulAction   = FTE_EVENT_ACTION_LOG | FTE_EVENT_ACTION_SNMP_TRAP | FTE_EVENT_ACTION_MQTT_PUB
};


static const FTE_EVENT_CONFIG ifte_report_temp_config = 
{
    .ulOID      = MAKE_ID(FTE_OBJ_TYPE_TRUEYES_TEMP, 0x0001),
    .xType      = FTE_EVENT_TYPE_INTERVAL,
    .xParams    =
    {
        .ulInterval =   60
    },
    .ulAction   = FTE_EVENT_ACTION_MQTT_PUB
};

static const FTE_EVENT_CONFIG ifte_report_humidity_config = 
{
    .ulOID      = MAKE_ID(FTE_OBJ_TYPE_TRUEYES_HUMI, 0x0001),
    .xType      = FTE_EVENT_TYPE_INTERVAL,
    .xParams    =
    {
        .ulInterval =   60
    },
    .ulAction   = FTE_EVENT_ACTION_MQTT_PUB
};

static const FTE_EVENT_CONFIG ifte_event_co2_config = 
{
    .ulOID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0001),
    .xType      = FTE_EVENT_TYPE_ABOVE,
    .xParams    =
    {
        .xLimit =
        {
            .nValue = 1000,
            .ulThreshold = 20
        }
    },
    .ulDelayTime=   1000,
    .ulHoldTime =   60000,
    .ulAction   = FTE_EVENT_ACTION_LOG | FTE_EVENT_ACTION_SNMP_TRAP | FTE_EVENT_ACTION_MQTT_PUB
};

static const FTE_EVENT_CONFIG ifte_report_co2_config = 
{
    .ulOID      = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0001),
    .xType      = FTE_EVENT_TYPE_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    },
    .ulAction   = FTE_EVENT_ACTION_MQTT_PUB
};

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tureyes_multi_sens_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_trueeyes_co2_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_trueeyes_temp_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_trueeyes_humi_config
};

const FTE_CFG_EVENT_PTR pInitEventConfigs[] =
{
    (FTE_CFG_EVENT_PTR)&ifte_report_temp_config,
    (FTE_CFG_EVENT_PTR)&ifte_event_temp_config,
    (FTE_CFG_EVENT_PTR)&ifte_report_humidity_config,
    (FTE_CFG_EVENT_PTR)&ifte_report_co2_config,
    (FTE_CFG_EVENT_PTR)&ifte_event_co2_config
};