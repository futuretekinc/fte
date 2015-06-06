const FTE_DI_CONFIG fte_init_di0_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 1),
    .pName      = "DI0",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_1,
};

const FTE_DI_CONFIG fte_init_di1_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 2),
    .pName      = "DI1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_2,
};

const FTE_DO_CONFIG fte_init_do0_config =
{ 
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 1), 
    .pName      = "DO0", 
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF, 
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_3,  
};

const FTE_DO_CONFIG fte_init_do1_config =
{ 
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 2), 
    .pName      = "DO1", 
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF, 
    .nGPIO      = FTE_DEV_GPIO_MCP23S08_1_GPIO_4,  
};

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_di0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_di1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_do0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_do1_config
};


static const FTE_EVENT_CONFIG fte_report_di0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DI, 0x0001),
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
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
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

static const FTE_EVENT_CONFIG fte_report_do0_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0001),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
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

static const FTE_EVENT_CONFIG fte_report_do1_config = 
{
    .ulEPID      = MAKE_ID(FTE_OBJ_TYPE_DO, 0x0002),
    .xType      = FTE_EVENT_TYPE_ENABLE | FTE_EVENT_TYPE_MQTT_PUB,
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

const FTE_CFG_EVENT_PTR pInitEventConfigs[] =
{
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
};