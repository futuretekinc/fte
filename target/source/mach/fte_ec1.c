static const FTE_GAI_CONFIG fte_init_gai_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_FLEXIFORCE, 1),
    .pName      = "FlexiForce",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = FTE_DEV_AD7785_0,
    .nInterval  = FTE_GAI_INTERVAL,
    .nGain      = 1
};

static const FTE_SRF_CONFIG fte_init_srf_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_SRF, 1),
    .pName      = "MB7092-1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = 0,
    .nInterval  = FTE_SRF_INTERVAL
};

#if FTE_FIS3061_SUPPORTED
static const FTE_FIS_CONFIG fte_init_fis_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_LPG, 0x0201),
    .pName      = "FIS3061",
    .nModel     = FTE_FIS_MODEL_3061,
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nUCSID     = FTE_DEV_UCS_1,
    .nREQID     = FTE_DEV_LWGPIO_GAS_OUTPUT,
    .nInterval  = FTE_FIS_INTERVAL
};
#endif

const FTE_DO_CONFIG fte_init_do0_config =
{ 
    .nID        = MAKE_ID(FTE_OBJ_TYPE_DO, 1), 
    .pName      = "GAS_OUTPUT", 
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF, 
    .nGPIO      = FTE_DEV_GPIO_GAS_OUTPUT,  
};

#if  FTE_COZIR_AX5000_SUPPORTED
static const FTE_COZIR_CONFIG fte_init_cozir_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_CO2, 0x0101),
    .pName      = "COZIR-AX5000",
    .nModel     = FTE_COZIR_MODEL_AX5000,
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_COZIR_INTERVAL,
    .nEventType = 1,
    .nUpperLimit= 2500,
    .nLowerLimit= 0,
    .nThreshold = 100,
    .nEventDelay= 5
};
#endif


#if FTE_TASCON_HEM12_SUPPORTED
static const FTE_TASCON_CONFIG fte_init_tascon_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_HEM12, 0x0101),
    .pName      = "TASCON HEM12",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nModel     = FTE_TASCON_MODEL_HEM12,
    .pAddress   = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_TASCON_INTERVAL
};
#endif

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_cozir_config,
};

