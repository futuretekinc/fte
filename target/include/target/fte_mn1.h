#ifndef __FTE_H__
#define __FTE_H__

#define FTE_MUSHNET                         1

#define FTE_MODEL                           "FTE-MN1"

#define FTE_TEMP_SUPPORTED                  1
#define FTE_HUMIDITY_SUPPORTED              1
#define FTE_CO2_SUPPORTED                   1
#define FTE_POWER_SUPPORTED                 0

#define FTE_CONSOLE_SUPPORTED               1
#define FTE_LOG_SUPPORTED                   1
#define FTE_PWR_SW_SUPPORTED                1
#define FTE_FACTORY_RESET_SUPPORTED         0

#define FTE_I2C_SUPPORTED                   0
#define FTE_SSD1305_SUPPORTED               0
#define FTE_LCD_SUPPORTED                   0

#define FTE_SPI_SUPPORTED                   0
#define FTE_AD7785_SUPPORTED                0
#define FTE_1WIRE_SUPPORTED                 1
#define FTE_MCP23S08_SUPPORTED              0
#define FTE_UCS_SUPPORTED                   1
#define FTE_UCM_SUPPORTED                   0
#define FTE_DEV_LWGPIO_UTS_FLOWCTRL         1
#define FTE_DEV_LWGPIO_UTS_FLOWCTRL2        1

#define FTE_RTD_SUPPORTED                   0
#define FTE_RTD0_SUPPORTED                  0
#define FTE_RTD1_SUPPORTED                  0
#define FTE_GAI_SUPPORTED                   0
#define FTE_PRESSURE_SUPPORTED              0
#define FTE_SHT_SUPPORTED                   1
#define FTE_DS18B20_SUPPORTED               1
#define FTE_DI_SUPPORTED                    1
#define FTE_DO_SUPPORTED                    1
#define FTE_RL_SUPPORTED                    0
#define FTE_LED_SUPPORTED                   1
#define FTE_SRF_SUPPORTED                   0
#define FTE_FIS_SUPPORTED                   0
#define FTE_FIS3061_SUPPORTED               0
#define FTE_MULTI_DI_SUPPORTED              1
#define FTE_MULTI_DO_SUPPORTED              1

// CO2 Sensors
#define FTE_SH_MV250_SUPPORTED              1
#define FTE_TRUEYES_AIRQ_SUPPORTED          0
#define FTE_ELT_AQM100_SUPPORTED            0
#define FTE_COZIR_AX5000_SUPPORTED          0

// Power Meters
#define FTE_TASCON_HEM12_SUPPORTED          0

// Multi-Function
#define FTE_BOTEM_PN1500_SUPPORTED          0
#define FTE_GUS_SUPPORTED                   1
#define FTE_HM1001_SUPPORTED                0
#define FTE_MST_MEX510C_SUPPORTED           0

#define FTE_HTTPD_SUPPORTED                 0
#define FTE_SNMPD_SUPPORTED                 1
#define FTE_MQTT_SUPPORTED                  0
#define FTE_SSL_SUPPORTED                   0
#define FTE_TELNETD_SUPPORTED               1
#define FTE_SMNGD_SUPPORTED                 1

#define FTE_DI_MAX_COUNT                    2
#define FTE_DO_MAX_COUNT                    1
#define FTE_RL_MAX_COUNT                    0
#define FTE_LED_MAX_COUNT                   1
#define FTE_PT100_MAX_COUNT                 0
#define FTE_DS18B20_MAX_COUNT               10
#define FTE_SHT_MAX_COUNT                   1
#define FTE_1WIRE_MAX_DEVICES               10

#define FTE_DEBUG                           1

#include "fte_common.h"

#endif
