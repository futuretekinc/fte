#ifndef __FTE_H__
#define __FTE_H__

#define FTE_EC1                         1

#define FTE_MODEL                       "FTE-EC1"

#define FTE_CONSOLE_SUPPORTED           1
#define FTE_PWR_SW_SUPPORTED            1
#define FTE_FACTORY_RESET_SUPPORTED     0

#define FTE_SPI_SUPPORTED               0
#define FTE_AD7785_SUPPORTED            0
#define FTE_1WIRE_SUPPORTED             0
#define FTE_MCP23S08_SUPPORTED          0
#define FTE_UCS_SUPPORTED               1
#define FTE_UCM_SUPPORTED               1

#define FTE_RTD_SUPPORTED               0
#define FTE_RTD0_SUPPORTED              0
#define FTE_RTD1_SUPPORTED              0
#define FTE_GAI_SUPPORTED               0
#define FTE_SHT_SUPPORTED               0
#define FTE_DS18B20_SUPPORTED           0
#define FTE_DI_SUPPORTED                0
#define FTE_DO_SUPPORTED                1
#define FTE_RL_SUPPORTED                0
#define FTE_LED_SUPPORTED               1
#define FTE_SRF_SUPPORTED               0
#define FTE_FIS_SUPPORTED               0
#define FTE_FIS3061_SUPPORTED           0
#define FTE_COZIR_SUPPORTED             0
#define FTE_COZIR_AX5000_SUPPORTED      0
#define FTE_TASCON_SUPPORTED            0
#define FTE_TASCON_HEM12_SUPPORTED      0
#define FTE_TRUEYES_MULTI_SENS_SUPPORTED    1

#define FTE_DI_MAX_COUNT                0
#define FTE_DO_MAX_COUNT                1
#define FTE_RL_MAX_COUNT                0
#define FTE_LED_MAX_COUNT               1
#define FTE_PT100_MAX_COUNT             0
#define FTE_DS18B20_MAX_COUNT           0
#define FTE_SHT_MAX_COUNT               0
#define FTE_1WIRE_MAX_DEVICES           0

#define FTE_DEV_LWGPIO_RESET            (FTE_DEV_TYPE_LWGPIO | 0x00)
#define FTE_DEV_LWGPIO_AD7785_0_CS      (FTE_DEV_TYPE_LWGPIO | 0x01)
#define FTE_DEV_LWGPIO_AD7785_1_CS      (FTE_DEV_TYPE_LWGPIO | 0x02)
#define FTE_DEV_LWGPIO_SHT_SDA          (FTE_DEV_TYPE_LWGPIO | 0x03)
#define FTE_DEV_LWGPIO_SHT_SCL          (FTE_DEV_TYPE_LWGPIO | 0x04)
#define FTE_DEV_LWGPIO_POWER_HOLD       (FTE_DEV_TYPE_LWGPIO | 0x05)
#define FTE_DEV_LWGPIO_POWER_CTRL       (FTE_DEV_TYPE_LWGPIO | 0x06)
#define FTE_DEV_LWGPIO_SW_DETECT        (FTE_DEV_TYPE_LWGPIO | 0x07)
#define FTE_DEV_LWGPIO_FACTORY_RESET    (FTE_DEV_TYPE_LWGPIO | 0x08)
#define FTE_DEV_LWGPIO_LED_0            (FTE_DEV_TYPE_LWGPIO | 0x09)
#define FTE_DEV_LWGPIO_GAS_OUTPUT       (FTE_DEV_TYPE_LWGPIO | 0x0A)
#define FTE_DEV_LWGPIO_UTS_FLOWCTRL     (FTE_DEV_TYPE_LWGPIO | 0x0B)
#define FTE_DEV_LWGPIO_UTS_FLOWCTRL2    (FTE_DEV_TYPE_LWGPIO | 0x0C)

#define FTE_DEV_GPIO_RESET              (FTE_DEV_TYPE_GPIO | 0x00)
#define FTE_DEV_GPIO_AD7785_0_CS        (FTE_DEV_TYPE_GPIO | 0x01)
#define FTE_DEV_GPIO_AD7785_1_CS        (FTE_DEV_TYPE_GPIO | 0x02)
#define FTE_DEV_GPIO_SHT_SDA            (FTE_DEV_TYPE_GPIO | 0x03)
#define FTE_DEV_GPIO_SHT_SCL            (FTE_DEV_TYPE_GPIO | 0x04)
#define FTE_DEV_GPIO_POWER_HOLD         (FTE_DEV_TYPE_GPIO | 0x05)
#define FTE_DEV_GPIO_POWER_CTRL         (FTE_DEV_TYPE_GPIO | 0x06)
#define FTE_DEV_GPIO_SW_DETECT          (FTE_DEV_TYPE_GPIO | 0x07)
#define FTE_DEV_GPIO_FACTORY_RESET      (FTE_DEV_TYPE_GPIO | 0x08)
#define FTE_DEV_GPIO_LED_0              (FTE_DEV_TYPE_GPIO | 0x09)
#define FTE_DEV_GPIO_GAS_OUTPUT         (FTE_DEV_TYPE_GPIO | 0x0A)

#define FTE_DEV_SPI_0_0                 (FTE_DEV_TYPE_SPI | 0)
#define FTE_DEV_SPI_0_1                 (FTE_DEV_TYPE_SPI | 1)

#define FTE_DEV_AD7785_0                (FTE_DEV_TYPE_AD7785 | 0)
#define FTE_DEV_AD7785_1                (FTE_DEV_TYPE_AD7785 | 1)

#define FTE_DEV_UCS_1                   (FTE_DEV_TYPE_UCS | 1)
#define FTE_DEV_UCM_1                   (FTE_DEV_TYPE_UCM | 1)

#define FTE_DEV_SRF_1                   (FTE_DEV_TYPE_SRF | 1)

#define FTE_GPIO_AD7785_0_CS            (GPIO_PORT_C | GPIO_PIN4)
#define FTE_GPIO_AD7785_1_CS            (GPIO_PORT_C | GPIO_PIN3)

#define FTE_GPIO_SHT_SDA                (GPIO_PORT_C | GPIO_PIN2)
#define FTE_GPIO_SHT_SCL                (GPIO_PORT_C | GPIO_PIN1)

#define FTE_GPIO_RESET                  (GPIO_PORT_B | GPIO_PIN3)
#define FTE_GPIO_POWER_HOLD             (GPIO_PORT_B | GPIO_PIN8)
#define FTE_GPIO_POWER_CTRL             (GPIO_PORT_B | GPIO_PIN9)
#define FTE_GPIO_SW_DETECT              (GPIO_PORT_B | GPIO_PIN11)
#define FTE_GPIO_FACTORY_RESET          (GPIO_PORT_B | GPIO_PIN10)
#define FTE_GPIO_LED_0                  (GPIO_PORT_C | GPIO_PIN12)

#define FTE_GPIO_GAS_OUTPUT             (GPIO_PORT_C | GPIO_PIN1)
#define FTE_GPIO_UART1_RTS              (GPIO_PORT_C | GPIO_PIN1)
#define FTE_GPIO_UCS_FLOWCTRL           (GPIO_PORT_D | GPIO_PIN0)
#define FTE_GPIO_UCS_FLOWCTRL2          (GPIO_PORT_D | GPIO_PIN1)

#define FTE_GPIO_AD7785_0_CS_MUX        (LWGPIO_MUX_C4_GPIO)
#define FTE_GPIO_AD7785_1_CS_MUX        (LWGPIO_MUX_C3_GPIO)

#define FTE_GPIO_SHT_SDA_MUX            (LWGPIO_MUX_C2_GPIO)
#define FTE_GPIO_SHT_SCL_MUX            (LWGPIO_MUX_C1_GPIO)

#define FTE_GPIO_RESET_MUX              (LWGPIO_MUX_B3_GPIO)
#define FTE_GPIO_POWER_HOLD_MUX         (LWGPIO_MUX_B8_GPIO)
#define FTE_GPIO_POWER_CTRL_MUX         (LWGPIO_MUX_B9_GPIO)
#define FTE_GPIO_SW_DETECT_MUX          (LWGPIO_MUX_B11_GPIO)
#define FTE_GPIO_FACTORY_RESET_MUX      (LWGPIO_MUX_B10_GPIO)
#define FTE_GPIO_LED_0_MUX              (LWGPIO_MUX_C12_GPIO)
#define FTE_GPIO_GAS_OUTPUT_MUX         (LWGPIO_MUX_C1_GPIO)
#define FTE_GPIO_UART1_RTS_MUX          (LWGPIO_MUX_C1_GPIO)
#define FTE_GPIO_UCS_FLOWCTRL_MUX       (LWGPIO_MUX_D0_GPIO)
#define FTE_GPIO_UCS_FLOWCTRL2_MUX       (LWGPIO_MUX_D1_GPIO)

#define FTE_GAI_INTERVAL                1000
#define FTE_RTD_INTERVAL                10000
#define FTE_SRF_INTERVAL                10000
#define FTE_FIS_INTERVAL                1000
#define FTE_COZIR_INTERVAL              2000
#define FTE_TASCON_INTERVAL             2000

#endif
