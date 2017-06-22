#include "fte_target.h"
#include "fte_object.h"

typedef struct 
{
    FTE_OBJECT_ID   xClass;
    FTE_CHAR_PTR        pName;
} FTE_CLASS_DESC;

static const 
FTE_CLASS_DESC _pClassDescs[] =
{
    {
        .xClass = FTE_OBJ_CLASS_TEMPERATURE,
        .pName  = "Temperature"
    },
    {
        .xClass = FTE_OBJ_CLASS_HUMIDITY,
        .pName  = "Humidity"
    },
    {
        .xClass = FTE_OBJ_CLASS_VOLTAGE,
        .pName  = "Voltage"
    },
    {
        .xClass = FTE_OBJ_CLASS_CURRENT,
        .pName  = "Current"
    },
    {
        .xClass = FTE_OBJ_CLASS_DI,
        .pName  = "Digital Input"
    },
    {
        .xClass = FTE_OBJ_CLASS_DO,
        .pName  = "Digital Output"
    },
    {
        .xClass = FTE_OBJ_CLASS_GAS,
        .pName  = "Gas"
    },
    {
        .xClass = FTE_OBJ_CLASS_POWER,
        .pName  = "Power"
    },
    {
        .xClass = FTE_OBJ_CLASS_SRF,
        .pName  = "Sound Range Finder"
    },
    {
        .xClass = FTE_OBJ_CLASS_AI,
        .pName  = "Analog Input"
    },
    {
        .xClass = FTE_OBJ_CLASS_COUNT,
        .pName  = "Count"
    },
    {
        .xClass = FTE_OBJ_CLASS_PRESSURE,
        .pName  = "Pressure"
    },
    {
        .xClass = FTE_OBJ_CLASS_DISCRETE,
        .pName  = "Discrete"
    },
    {
        .xClass = FTE_OBJ_CLASS_CTRL,
        .pName  = "Control"
    },
    {
        .xClass = FTE_OBJ_CLASS_MULTI,
        .pName  = "Multi Function Device"
    }
};

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
        .f_attach           = FTE_RTD_attach,     
        .f_detach           = FTE_RTD_detach
    },
#endif
#if FTE_DS18B20_SUPPORTED
    {    
        .nType              = FTE_OBJ_TYPE_DS18B20,   
        .pName              = "DS18B20",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_SN,
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_SHT_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_SHT,   
        .pName              = "SHT",
        .pVendor            = "Sensirion AG",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,   
        .nConfigSize        = sizeof(FTE_SHT_CONFIG),
        .nStatusSize        = sizeof(FTE_SHT_STATUS),
        .f_create           = FTE_SHT_create,
        .f_attach           = FTE_SHT_attach, 
        .f_detach           = FTE_SHT_detach

    },
#endif
#if FTE_MULTI_HUMI_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_HUMI,   
        .pName              = "HUMIDITY",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_DO_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_DO,        
        .pName              = "DO",
        .xFlags             = FTE_OBJ_FLAG_CTRL,
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_RL_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_RL,        
        .pName              = "RELAY",
        .nMaxCount          = FTE_RL_MAX_COUNT,
        .xFlags             = FTE_OBJ_FLAG_CTRL,
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
        .xFlags             = FTE_OBJ_FLAG_CTRL,
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
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
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
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_COZIR_AX5000_GUSModelInfo
    },
#endif
#if FTE_SRF_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_SRF,   
        .pName              = "SONIC RANGE METER",
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
        .xFlags             = FTE_OBJ_FLAG_GUS,
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
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_HM1001_GUSModelInfo
    },
#endif
#if FTE_MULTI_COUNT_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_COUNT,   
        .pName              = "COUNT",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_PRESSURE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_FLEXIFORCE,   
        .pName              = "PRESSURE",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_CO2_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_CO2,   
        .pName              = "CO2",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_VOC_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_VOC,   
        .pName              = "VOC",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_VOLTAGE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_VOLTAGE,   
        .pName              = "VOLTAGE",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_CURRENT_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_CURRENT,   
        .pName              = "CURRENT",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_POWER_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_POWER,   
        .pName              = "POWER",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_AI_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_AI,   
        .pName              = "VALUE",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_VALUE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_CTRL,   
        .pName              = "CTRL",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_MULTI_DISCRETE_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_DISCRETE,   
        .pName              = "DISCRETE",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
#endif
#if FTE_SOHA_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_SH_MV250,   
        .pName              = "MV250HT",
        .pVendor            = "SOHA",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_SOHA_CONFIG),
        .nStatusSize        = sizeof(FTE_SOHA_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_SOHA_MV250_GUSModelInfo
    },
#endif
#if FTE_TRUEYES_AIRQ_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_AIRQ,   
        .pName              = "AIRQ",
        .pVendor            = "Trueyes",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
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
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_TRUEYES_AIRQ_GUSModelInfo
    },
#endif
#if FTE_ELT_AQM100_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_AQM100,   
        .pName              = "AQM100",
        .pVendor            = "ELT",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_ELT_CONFIG),
        .nStatusSize        = sizeof(FTE_ELT_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_ELT_AQM100_GUSModelInfo
    },
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_AQM100M,   
        .pName              = "AQM100M",
        .pVendor            = "ELT",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_ELT_CONFIG),
        .nStatusSize        = sizeof(FTE_ELT_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_ELT_AQM100M_GUSModelInfo
    },
#endif
#if FTE_MST_MEX510C_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_MEX510C,   
        .pName              = "MEX510C",
        .pVendor            = "MST",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
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
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_MST_MEX510C_GUSModelInfo
    },
#endif
#if FTE_GS_DPC_HL_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_DPC_HL,   
        .pName              = "DPC-HL",
        .pVendor            = "Green System",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
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
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_GS_DPC_GUSModelInfo
    },
#endif
#if FTE_BOTEM_PN1500_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_PN1500,   
        .pName              = "PN1500",
        .pVendor            = "Botem",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
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
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTM_VOID_PTR)&FTE_MOTEM_PN1500_GUSModelInfo
    },
#endif
#if FTE_TASCON_HEM12_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_HEM12,   
        .pName              = "HEM12",
        .pVendor            = "Tascon",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_TASCON_HEM12_CONFIG),
        .nStatusSize        = sizeof(FTE_TASCON_HEM12_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_TASCON_HEM12_GUSModelInfo
    },
#endif
#if FTE_TASCON_HEM12_06M_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_HEM12_06M,   
        .pName              = "HEM12-06M",
        .pVendor            = "Tascon",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_TASCON_HEM12_CONFIG),
        .nStatusSize        = sizeof(FTE_TASCON_HEM12_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_TASCON_HEM12_06M_GUSModelInfo
    },
#endif
#if FTE_FTLM_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_FTLM,   
        .pName              = "FTLM",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
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
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_FTLM_GUSModelInfo
    },
#endif
#if FTE_CIAS_SIOUX_CU_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_CIAS_ZONE,   
        .pName              = "CU ZONE",
        .pVendor            = "CIAS",
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
        .f_attach           = FTE_IFCE_attach, 
        .f_detach           = FTE_IFCE_detach
    },
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_CIAS_SIOUX_CU,
        .pName              = "SIOUX_CU",
        .pVendor            = "CIAS",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_CIAS_SIOUX_CU_CONFIG),
        .nStatusSize        = sizeof(FTE_CIAS_SIOUX_CU_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_CIAS_SIOUX_CU_GUSModelInfo
    },
#endif
#if FTE_IOEX_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_IOEX,
        .pName              = "IOEX",
        .pVendor            = "FutureTek,Inc.",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_IOEX_CONFIG),
        .nStatusSize        = sizeof(FTE_IOEX_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_IOEX_GUSModelInfo
        
    },
#endif
#if FTE_MULTI_DIO_NODE_SUPPORTED
	{   
        .nType              = FTE_OBJ_TYPE_MULTI_DIO_NODE,
        .pName              = "DIO_NODE",
        .pVendor            = "FutureTek,Inc.",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_DIO_NODE_CONFIG),
        .nStatusSize        = sizeof(FTE_DIO_NODE_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_DIO_NODE_GUSModelInfo
        
    },
#endif
#if FTE_DOTECH_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_DOTECH_FX3D,
        .pName              = "FX3D",
        .pVendor            = "Dotech Inc.",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC | FTE_OBJ_FLAG_GUS,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_DOTECH_CONFIG),
        .nStatusSize        = sizeof(FTE_DOTECH_FX3D_STATUS),
        .f_attach           = FTE_GUS_attach, 
        .f_detach           = FTE_GUS_detach,
        .pOpts              = (FTE_VOID_PTR)&FTE_DOTECH_FX3D_GUSModelInfo
    },
#endif
#if FTE_TURBOMAX_SUPPORTED
    {   
        .nType              = FTE_OBJ_TYPE_MULTI_TURBOMAX_MAX20,
        .pName              = "MAX20",
        .pVendor            = "TurboMax.",
        .xFlags             = FTE_OBJ_FLAG_DYNAMIC,
        .xSupportedFields   = FTE_OBJ_FIELD_ID | 
                              FTE_OBJ_FIELD_NAME |
                              FTE_OBJ_FIELD_NAME_EDIT |
                              FTE_OBJ_FIELD_VALUE |
                              FTE_OBJ_FIELD_STATE |
                              FTE_OBJ_FIELD_ENABLE |
                              FTE_OBJ_FIELD_ENABLE_EDIT,                
        .nConfigSize        = sizeof(FTE_TURBOMAX_CONFIG),
        .nStatusSize        = sizeof(FTE_TURBOMAX_STATUS),
        .f_create           = FTE_TURBOMAX_create,
        .f_attach           = FTE_TURBOMAX_attach, 
        .f_detach           = FTE_TURBOMAX_detach,
        .pOpts              = NULL
    },
#endif
};

static const FTE_UINT32 _nObjDescs  = sizeof(_pObjDescs) / sizeof(FTE_OBJECT_DESC); 
 
FTE_UINT32 FTE_OBJ_DESC_count(void)
{
    return  _nObjDescs;
}

FTE_OBJECT_DESC_PTR FTE_OBJ_DESC_getAt(FTE_UINT32 ulIndex)
{
    if (ulIndex >= _nObjDescs)
    {
        return  NULL;
    }

    return  (FTE_OBJECT_DESC_PTR)&_pObjDescs[ulIndex];
}

FTE_OBJECT_DESC_PTR FTE_OBJ_DESC_get(FTE_UINT32 nType)
{
    FTE_UINT32 ulCount;
    
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

static 
FTE_UINT32_PTR  _pClass = NULL;

static 
FTE_UINT32      _nClass = 0;

FTE_UINT32 FTE_OBJ_DESC_CLASS_count(void)
{
    FTE_UINT32 i, j;
    
    FTE_UINT32_PTR pClass = NULL;
    FTE_UINT32     nClass = 0;
    
    if (_pClass != NULL)
    {
        return  _nClass;
    }
    
    pClass = FTE_MEM_allocZero(sizeof(FTE_UINT32) * _nObjDescs);    
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
    _pClass = FTE_MEM_allocZero(sizeof(FTE_UINT32) * nClass);
    if (_pClass == NULL)
    {
        goto error;
    }

    memcpy(_pClass, pClass, sizeof(FTE_UINT32) * nClass);
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

FTE_UINT32 FTE_OBJ_DESC_CLASS_getAt(FTE_UINT32 ulIndex)
{
    if (ulIndex >= _nClass)
    {
        return  0;
    }
    
    return  _pClass[ulIndex];
}

FTE_RET FTE_OBJ_CLASS_getName
(   
    FTE_OBJECT_ID   nID, 
    FTE_CHAR_PTR    pName, 
    FTE_UINT32      nBuffLen
)
{
    FTE_UINT32 i;
    
    if (nBuffLen > 1)
    {
        for(i = 0 ; i < sizeof(_pClassDescs) / sizeof(FTE_CLASS_DESC) ; i++)
        {
            if (_pClassDescs[i].xClass == (nID & FTE_OBJ_CLASS_MASK))
            {
                strncpy(pName, _pClassDescs[i].pName, nBuffLen - 1);
                return   FTE_RET_OK;
            }
        }
    }
    
    strcpy(pName, "Unknown");
    
    return   FTE_RET_ERROR;
}
