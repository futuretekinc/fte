static const FTE_GAI_CONFIG fte_init_gai_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_FLEXIFORCE, 1),
    .pName      = "FlexiForce",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = FTE_DEV_AD7785_0,
    .nInterval  = FTE_GAI_INTERVAL,
    .nGain      = 1
};

const FTE_DO_CONFIG fte_init_do0_config =
{ 
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 1), 
    .pName      = "DUST_ON", 
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF, 
    .nGPIO      = FTE_DEV_GPIO_DUST_ON,  
};

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_gai_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_do0_config,
};
