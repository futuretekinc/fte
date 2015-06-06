static const FTE_GUS_CONFIG fte_init_botem_pn1500_multi_sens_config =
{
    .nID        = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .pName      = "BT1500",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nModel     = FTE_MULTI_MODEL_BOTEM_PN1500_SENS,
    .nSensorID  = 0x01,
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_BOTEM_PN1500_MULTI_SENS_INTERVAL,
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_human_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0x0001),
    .pName      = "COUNT",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 0,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_human_accum_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_COUNT, 0x0002),
    .pName      = "ACCUM",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_light_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0001),
    .pName      = "LIGHT",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_1_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0002),
    .pName      = "AIRCON1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 3,
    .nInterval  = 2
};
static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_2_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0003),
    .pName      = "AIRCON2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 4,
    .nInterval  = 2
};
static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_3_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0004),
    .pName      = "AIRCON3",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 5,
    .nInterval  = 2
};
static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_4_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0005),
    .pName      = "AIRCON4",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 6,
    .nInterval  = 2
};


static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_accum_reset_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DO, 0x0006),
    .pName      = "ACCUM RESET",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 1,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_light_status_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0001),
    .pName      = "LIGHT",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 2,
    .nInterval  = 2
};

static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_1_status_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0002),
    .pName      = "AIRCON1",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 3,
    .nInterval  = 2
};
static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_2_status_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0003),
    .pName      = "AIRCON2",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 4,
    .nInterval  = 2
};
static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_3_status_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0004),
    .pName      = "AIRCON3",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 5,
    .nInterval  = 2
};
static const FTE_IFCE_CONFIG ifte_init_botem_pn1500_aircon_4_status_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_MULTI_DI, 0x0005),
    .pName      = "AIRCON4",
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nDevID     = MAKE_ID(FTE_OBJ_CLASS_MULTI, 0x0001),
    .nRegID     = 6,
    .nInterval  = 2
};


const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_botem_pn1500_multi_sens_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_human_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_human_accum_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_light_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_1_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_2_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_3_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_4_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_accum_reset_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_light_status_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_1_status_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_2_status_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_3_status_config,
    (FTE_OBJECT_CONFIG_PTR)&ifte_init_botem_pn1500_aircon_4_status_config
};
