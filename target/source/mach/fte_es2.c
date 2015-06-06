static const FTE_SHT_CONFIG fte_init_sht_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_SHT, 1),
    .pName      = "SHT-1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    .nSDA       = FTE_DEV_LWGPIO_SHT_SDA,
    .nSCL       = FTE_DEV_LWGPIO_SHT_SCL,
    .nInterval  = FTE_SHT_INTERVAL
};

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_sht_config
};


static const FTE_EVENT_CONFIG fte_report_sht_config = 
{
    .ulEPID     = MAKE_ID(FTE_OBJ_TYPE_SHT, 0x0001),
    .xType      = FTE_EVENT_TYPE_MQTT_PUB,
    .xLevel     = FTE_EVENT_LEVEL_INFO,
    .xCondition = FTE_EVENT_CONDITION_INTERVAL,
    .xParams    =
    {
        .ulInterval = 60
    }
};

const FTE_CFG_EVENT_PTR pInitEventConfigs[] =
{
    (FTE_CFG_EVENT_PTR)&fte_report_sht_config
};