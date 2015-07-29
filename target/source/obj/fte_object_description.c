#include "fte_target.h"
#include "fte_object.h"

static const FTE_OBJECT_DESC _pObjDescs[] = 
{
#if FTE_RTD_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_PT100,     
        .pName              = "TEMPERATURE",
        .nMaxCount          = FTE_PT100_MAX_COUNT,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_RTD_CONFIG),
        .nStatusSize        = sizeof(FTE_RTD_STATUS),
        .f_attach           = fte_rtd_attach,     
        .f_detach           = fte_rtd_detach
    },
#endif
#if FTE_DS18B20_SUPPORTED
    {    
        .nType              = FTE_OBJ_TYPE_DS18B20,   
        .pName              = "TEMPERATURE",
        .nMaxCount          = FTE_DS18B20_MAX_COUNT,
        .xFlags             = FTE_OBJ_HAVE_SN,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT |
                              FTE_OBJ_FIELD_SN,                
        .nConfigSize        = sizeof(FTE_DS18B20_CONFIG),
        .nStatusSize        = sizeof(FTE_DS18B20_STATUS),
        .f_attach           = FTE_DS18B20_attach, 
        .f_detach           = FTE_DS18B20_detach

    },
#endif
#if FTE_MULTI_TEMP_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_TEMP,   
        .pName              = "TEMPERATURE",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_SHT_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_SHT,   
        .pName              = "SHT",
        .nMaxCount          = FTE_SHT_MAX_COUNT,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,   
        .nConfigSize        = sizeof(FTE_SHT_CONFIG),
        .nStatusSize        = sizeof(FTE_SHT_STATUS),
        .f_attach           = FTE_SHT_attach, 
        .f_detach           = FTE_SHT_detach

    },
#endif
#if FTE_MULTI_HUMI_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_HUMI,   
        .pName              = "HUMIDITY",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif

#if FTE_GAI_VOLTAGE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_GAI_VOLTAGE,   
        .pName              = "VOLTAGE",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GAI_CONFIG),
        .nStatusSize        = sizeof(FTE_GAI_STATUS),
        .f_attach           = FTE_GAI_attach, 
        .f_detach           = FTE_GAI_detach
    },
#endif
#if FTE_GAI_CURRENT_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_GAI_CURRENT,   
        .pName              = "CURRENT",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GAI_CONFIG),
        .nStatusSize        = sizeof(FTE_GAI_STATUS),
        .f_attach           = FTE_GAI_attach, 
        .f_detach           = FTE_GAI_detach
    },
#endif
#if FTE_DI_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_DI,        
        .pName              = "DI",
        .nMaxCount          = FTE_DI_MAX_COUNT,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT, 
        .nConfigSize        = sizeof(FTE_DI_CONFIG),
        .nStatusSize        = sizeof(FTE_DI_STATUS),
        .f_attach           = FTE_DI_attach,      
        .f_detach           = FTE_DI_detach
    },
#endif
#if FTE_MULTI_DI_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_DI,   
        .pName              = "DI",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_DO_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_DO,        
        .pName              = "DO",
        .nMaxCount          = FTE_DO_MAX_COUNT,
        .xFlags             = FTE_OBJ_HAVE_CTRL,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_CTRL |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_DO_CONFIG),
        .nStatusSize        = sizeof(FTE_DO_STATUS),
        .f_attach           = FTE_DO_attach,      
        .f_detach           = FTE_DO_detach
    },
#endif
#if FTE_MULTI_DO_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_DO,   
        .pName              = "DO",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_RL_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_RL,        
        .pName              = "RELAY",
        .nMaxCount          = FTE_RL_MAX_COUNT,
        .xFlags             = FTE_OBJ_HAVE_CTRL,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_CTRL |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT |
                              FTE_OBJ_FIELD_INIT |
                              FTE_OBJ_FIELD_INIT_EDIT,                
        .nConfigSize        = sizeof(FTE_RL_CONFIG),
        .nStatusSize        = sizeof(FTE_RL_STATUS),
        .f_attach           = FTE_RL_attach,      
        .f_detach           = FTE_RL_detach
    },
#endif
#if FTE_LED_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_LED,        
        .pName              = "LED",
        .nMaxCount          = FTE_LED_MAX_COUNT,
        .xFlags             = FTE_OBJ_HAVE_CTRL,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_CTRL |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_LED_CONFIG),
        .nStatusSize        = sizeof(FTE_LED_STATUS),
        .f_attach           = FTE_LED_attach,      
        .f_detach           = FTE_LED_detach
    },
#endif
#if FTE_FIS3061_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_LPG,   
        .pName              = "LPG",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_FIS_CONFIG),
        .nStatusSize        = sizeof(FTE_FIS_STATUS),
        .f_attach           = fte_fis_attach, 
        .f_detach           = fte_fis_detach
    }
#endif
#if FTE_COZIR_AX5000_SUPPORTED 
    {   
        .nType              = FTE_OBJ_TYPE_CO2,   
        .pName              = "CO2",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_TASCON_HEM12_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_POWER,   
        .pName              = "POWER",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    }
#endif
#if FTE_SRF_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_SRF,   
        .pName              = "SONIC RANGE METER",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .f_attach           = fte_srf_attach, 
        .f_detach           = fte_srf_detach
    },
#endif
#if FTE_HM1001_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_DUST,   
        .pName              = "DUST",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_MULTI_COUNT_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_COUNT,   
        .pName              = "COUNT",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_PRESSURE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_FLEXIFORCE,   
        .pName              = "PRESSURE",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GAI_CONFIG),
        .nStatusSize        = sizeof(FTE_GAI_STATUS),
        .f_attach           = fte_gai_attach, 
        .f_detach           = fte_gai_detach
    },
#endif
#if FTE_MULTI_PRESSURE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_PRESSURE,   
        .pName              = "PRESSURE",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_MULTI_CO2_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_CO2,   
        .pName              = "CO2",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_MULTI_VOC_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_VOC,   
        .pName              = "VOC",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_MULTI_VOLTAGE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_VOLTAGE,   
        .pName              = "VOLTAGE",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_MULTI_CURRENT_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_CURRENT,   
        .pName              = "CURRENT",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_MULTI_POWER_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_POWER,   
        .pName              = "POWER",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_MULTI_VALUE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_VALUE,   
        .pName              = "VALUE",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IFCE_CONFIG),
        .nStatusSize        = sizeof(FTE_IFCE_STATUS),
        .f_attach           = fte_ifce_attach, 
        .f_detach           = fte_ifce_detach
    },
#endif
#if FTE_SH_MV250_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_SH_MV250,   
        .pName              = "MV250",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_TRUEYES_AIRQ_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_AIRQ,   
        .pName              = "AIRQ",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_ELT_AQM100_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_AQM100,   
        .pName              = "AQM100",
        .nMaxCount          = 4,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_MST_MEX510C_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_MEX510C,   
        .pName              = "MEX510C",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_GS_DPC_HL_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_DPC_HL,   
        .pName              = "DPC-HL",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_BOTEM_PN1500_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_PN1500,   
        .pName              = "PN1500",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    },
#endif
#if FTE_TASCON_HEM12_06M_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_HEM12_06M,   
        .pName              = "HEM12-06M",
        .nMaxCount          = 4,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_HEM12_06M_CONFIG),
        .nStatusSize        = sizeof(FTE_HEM12_06M_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    }
#endif
#if FTE_FTLM_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_FTLM,   
        .pName              = "FTLM",
        .nMaxCount          = 1,
        .xFlags             = 0,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_GUS_CONFIG),
        .nStatusSize        = sizeof(FTE_GUS_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach
    }
#endif
};

static const uint_32 _nObjDescs  = sizeof(_pObjDescs) / sizeof(FTE_OBJECT_DESC); 
 
uint_32 FTE_OBJ_DESC_count(void)
{
    return  _nObjDescs;
}

FTE_OBJECT_DESC_PTR FTE_OBJ_DESC_getAt(uint_32 ulIndex)
{
    if (ulIndex >= _nObjDescs)
    {
        return  NULL;
    }

    return  (FTE_OBJECT_DESC_PTR)&_pObjDescs[ulIndex];
}

FTE_OBJECT_DESC_PTR FTE_OBJ_DESC_get(uint_32 nType)
{
    uint_32 ulCount;
    
    ulCount = FTE_OBJ_DESC_count();
    
    for(int i = 0 ; i < ulCount ; i++)
    {
        if (_pObjDescs[i].nType == nType)
        {
            return  (FTE_OBJECT_DESC_PTR)&_pObjDescs[i];
        }
    }

    return  NULL;
}

static uint_32_ptr  _pClass = NULL;
static uint_32      _nClass = 0;

uint_32 FTE_OBJ_DESC_CLASS_count(void)
{
    uint_32 i, j;
    
    uint_32_ptr pClass = NULL;
    uint_32     nClass = 0;
    
    if (_pClass != NULL)
    {
        return  _nClass;
    }
    
    pClass = FTE_MEM_allocZero(sizeof(uint_32) * _nObjDescs);    
    if (pClass == NULL)
    {
        return  0;
    }
        
    for(i = 0 ; i < _nObjDescs ; i++)
    {
        for(j = 0 ; j < nClass ; j++)
        {
            if ((_pObjDescs[i].nType & FTE_OBJ_CLASS_MASK) == pClass[j])
            {
                break;
            }
        }
        
        if (j == nClass)
        {
            pClass[nClass++] = _pObjDescs[i].nType & FTE_OBJ_CLASS_MASK;
        }
    }
    _pClass = FTE_MEM_allocZero(sizeof(uint_32) * nClass);
    if (_pClass == NULL)
    {
        goto error;
    }

    memcpy(_pClass, pClass, sizeof(uint_32) * nClass);
    _nClass = nClass;
    
    FTE_MEM_free(pClass);

    return  _nClass;
    
error:

    if (pClass != NULL)
    {
        FTE_MEM_free(pClass);
    }
    
    return  0;
}

uint_32 FTE_OBJ_DESC_CLASS_getAt(uint_32 ulIndex)
{
    if (ulIndex >= _nClass)
    {
        return  0;
    }
    
    return  _pClass[ulIndex];
}