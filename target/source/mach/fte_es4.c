static const FTE_GUS_CONFIG fte_init_tascon_config =
{
    .nID        = MAKE_ID(FTE_OBJ_TYPE_HEM12, 0x0101),
    .pName      = "TASCON HEM12",
    .nModel     = FTE_GUS_MODEL_TASCON_HEM12,
    .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE, 
    .nUCSID     = FTE_DEV_UCS_1,
    .nInterval  = FTE_HEM12_INTERVAL,
    .nEventType = FTE_HEM12_EVENT_TYPE,
    .nUpperLimit= FTE_HEM12_EVENT_UPPER_LIMIT,
    .nLowerLimit= FTE_HEM12_EVENT_LOWER_LIMIT,
    .nThreshold = FTE_HEM12_EVENT_THRESHOLD,
    .nEventDelay= FTE_HEM12_EVENT_DELAY
};

const FTE_OBJECT_CONFIG_PTR pInitObjConfigs[] = 
{
    (FTE_OBJECT_CONFIG_PTR)&fte_init_tascon_config,
};
