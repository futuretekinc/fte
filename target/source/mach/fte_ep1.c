const FTE_DI_CONFIG fte_init_di0_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 1),
    .pName      = "DI0",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DI_0,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 1)
};

const FTE_DI_CONFIG fte_init_di1_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 2),
    .pName      = "DI1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DI_1,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 2)
};

const FTE_DI_CONFIG fte_init_di2_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 3),
    .pName      = "DI2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DI_2,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 3)   
};

const FTE_DI_CONFIG fte_init_di3_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DI, 4),
    .pName      = "DI3",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DI_3,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 4)
};

const FTE_DO_CONFIG fte_init_do0_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 1),
    .pName      = "DO0",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DO_0,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 5)
};

const FTE_DO_CONFIG fte_init_do1_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 2),
    .pName      = "DO1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DO_1,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 6)
};

const FTE_DO_CONFIG fte_init_do2_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 3),
    .pName      = "DO2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DO_2,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 7)
};

const  FTE_DO_CONFIG fte_init_do3_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 4),
    .pName      = "DO3",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIO      = FTE_DEV_GPIO_DO_3,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 8)
};

const  FTE_RL_CONFIG fte_init_rl0_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_RL, 1),
    .pName      = "RL0",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIOOpen  = FTE_DEV_GPIO_RLO_0,
    .nGPIOClose = FTE_DEV_GPIO_RLC_0,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 9)
};

const  FTE_RL_CONFIG fte_init_rl1_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_RL, 2),
    .pName      = "RL1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIOOpen  = FTE_DEV_GPIO_RLO_1,
    .nGPIOClose = FTE_DEV_GPIO_RLC_1,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 10)
};

const  FTE_RL_CONFIG fte_init_rl2_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_RL, 3),
    .pName      = "RL2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIOOpen  = FTE_DEV_GPIO_RLO_2,
    .nGPIOClose = FTE_DEV_GPIO_RLC_2,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 11)
};

const  FTE_RL_CONFIG fte_init_rl3_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_RL, 4),
    .pName      = "RL3",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIOOpen  = FTE_DEV_GPIO_RLO_3,
    .nGPIOClose = FTE_DEV_GPIO_RLC_3,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 12)
};

const  FTE_RL_CONFIG fte_init_rl4_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_RL, 5),
    .pName      = "RL4",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIOOpen  = FTE_DEV_GPIO_RLO_4,
    .nGPIOClose = FTE_DEV_GPIO_RLC_4,
    .nLED       = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 13)
};

const  FTE_RL_CONFIG fte_init_rl5_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_RL, 6),
    .pName      = "RL5",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    .nGPIOOpen  = FTE_DEV_GPIO_RLO_5,
    .nGPIOClose = FTE_DEV_GPIO_RLC_5,
};



const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_di0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_di1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_di2_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_di3_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_do0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_do1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_do2_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_do3_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rl0_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rl1_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rl2_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rl3_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rl4_config,
    (FTE_OBJECT_CONFIG_PTR)&fte_init_rl5_config
};

