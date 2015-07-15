#ifndef __FTE_COMMON_H__
#define __FTE_COMMON_H__

#define FTE_DEV_LWGPIO_RESET            (FTE_DEV_TYPE_LWGPIO | 0x00)
#define FTE_DEV_LWGPIO_AD7785_0_CS      (FTE_DEV_TYPE_LWGPIO | 0x01)
#define FTE_DEV_LWGPIO_AD7785_1_CS      (FTE_DEV_TYPE_LWGPIO | 0x02)
#define FTE_DEV_LWGPIO_SHT_SDA          (FTE_DEV_TYPE_LWGPIO | 0x03)
#define FTE_DEV_LWGPIO_SHT_SCL          (FTE_DEV_TYPE_LWGPIO | 0x04)
#define FTE_DEV_LWGPIO_POWER_HOLD       (FTE_DEV_TYPE_LWGPIO | 0x05)
#if FTE_V2
#define FTE_DEV_LWGPIO_POWER_IN         (FTE_DEV_TYPE_LWGPIO | 0x06)
#else
#define FTE_DEV_LWGPIO_POWER_CTRL       (FTE_DEV_TYPE_LWGPIO | 0x06)
#endif
#define FTE_DEV_LWGPIO_SW_DETECT        (FTE_DEV_TYPE_LWGPIO | 0x07)
#define FTE_DEV_LWGPIO_FACTORY_RESET    (FTE_DEV_TYPE_LWGPIO | 0x08)
#define FTE_DEV_LWGPIO_LED_0            (FTE_DEV_TYPE_LWGPIO | 0x09)
#define FTE_DEV_LWGPIO_MCP23S08_RESET   (FTE_DEV_TYPE_LWGPIO | 0x0A)
#define FTE_DEV_LWGPIO_MCP23S08_INT     (FTE_DEV_TYPE_LWGPIO | 0x0B)
#define FTE_DEV_LWGPIO_MCP23S08_CS      (FTE_DEV_TYPE_LWGPIO | 0x0C)
#define FTE_DEV_LWGPIO_1WIRE_1          (FTE_DEV_TYPE_LWGPIO | 0x0E)
#ifdef  FTE_LCD_SUPPORTED
#define FTE_DEV_LWGPIO_LCD_POWER        (FTE_DEV_TYPE_LWGPIO | 0x0F)
#else
#define FTE_DEV_LWGPIO_LED_1            (FTE_DEV_TYPE_LWGPIO | 0x0F)
#endif
#define FTE_DEV_LWGPIO_DI_0             (FTE_DEV_TYPE_LWGPIO | 0x10)

#define FTE_DEV_GPIO_RESET              (FTE_DEV_TYPE_GPIO | 0x00)
#define FTE_DEV_GPIO_AD7785_0_CS        (FTE_DEV_TYPE_GPIO | 0x01)
#define FTE_DEV_GPIO_AD7785_1_CS        (FTE_DEV_TYPE_GPIO | 0x02)
#define FTE_DEV_GPIO_SHT_SDA            (FTE_DEV_TYPE_GPIO | 0x03)
#define FTE_DEV_GPIO_SHT_SCL            (FTE_DEV_TYPE_GPIO | 0x04)
#define FTE_DEV_GPIO_POWER_HOLD         (FTE_DEV_TYPE_GPIO | 0x05)
#if FTE_V2
#define FTE_DEV_GPIO_POWER_IN           (FTE_DEV_TYPE_GPIO | 0x06)
#else
#define FTE_DEV_GPIO_POWER_CTRL         (FTE_DEV_TYPE_GPIO | 0x06)
#endif
#define FTE_DEV_GPIO_SW_DETECT          (FTE_DEV_TYPE_GPIO | 0x07)
#define FTE_DEV_GPIO_FACTORY_RESET      (FTE_DEV_TYPE_GPIO | 0x08)
#define FTE_DEV_GPIO_LED_0              (FTE_DEV_TYPE_GPIO | 0x09)
#define FTE_DEV_GPIO_MCP23S08_1_GPIO_1  (FTE_DEV_TYPE_GPIO | 0x0A)
#define FTE_DEV_GPIO_MCP23S08_1_GPIO_2  (FTE_DEV_TYPE_GPIO | 0x0B)
#define FTE_DEV_GPIO_MCP23S08_1_GPIO_3  (FTE_DEV_TYPE_GPIO | 0x0C)
#define FTE_DEV_GPIO_MCP23S08_1_GPIO_4  (FTE_DEV_TYPE_GPIO | 0x0D)
#define FTE_DEV_GPIO_1WIRE_0            (FTE_DEV_TYPE_GPIO | 0x0E)
#define FTE_DEV_GPIO_1WIRE_1            (FTE_DEV_TYPE_GPIO | 0x0F)
#ifdef  FTE_LCD_SUPPORTED
#define FTE_DEV_GPIO_LCD_POWER          (FTE_DEV_TYPE_GPIO | 0x10)
#else
#define FTE_DEV_GPIO_LED_1              (FTE_DEV_TYPE_GPIO | 0x10)
#endif
#define FTE_DEV_GPIO_DI_0               (FTE_DEV_TYPE_GPIO | 0x11)

#define FTE_DEV_I2C_0                   (FTE_DEV_TYPE_I2C | 0)
#define FTE_DEV_I2C_1                   (FTE_DEV_TYPE_I2C | 1)

#define FTE_DEV_SSD1305_0               (FTE_DEV_TYPE_SSD1305 | 1)

#define FTE_DEV_SPI_0_0                 (FTE_DEV_TYPE_SPI | 0)
#define FTE_DEV_SPI_0_1                 (FTE_DEV_TYPE_SPI | 1)

#define FTE_DEV_AD7785_0                (FTE_DEV_TYPE_AD7785 | 0)
#define FTE_DEV_AD7785_1                (FTE_DEV_TYPE_AD7785 | 1)

#define FTE_DEV_MCP23S08_1              (FTE_DEV_TYPE_MCP23S08 | 0x10)
#define FTE_DEV_MCP23S08_2              (FTE_DEV_TYPE_MCP23S08 | 0x20)

#define FTE_DEV_MCP23S08_1_GPIO_1       (FTE_DEV_MCP23S08_1 | 1)
#define FTE_DEV_MCP23S08_1_GPIO_2       (FTE_DEV_MCP23S08_1 | 2)
#define FTE_DEV_MCP23S08_1_GPIO_3       (FTE_DEV_MCP23S08_1 | 3)
#define FTE_DEV_MCP23S08_1_GPIO_4       (FTE_DEV_MCP23S08_1 | 4)
#define FTE_DEV_MCP23S08_1_GPIO_5       (FTE_DEV_MCP23S08_1 | 5)
#define FTE_DEV_MCP23S08_1_GPIO_6       (FTE_DEV_MCP23S08_1 | 6)
#define FTE_DEV_MCP23S08_1_GPIO_7       (FTE_DEV_MCP23S08_1 | 7)
#define FTE_DEV_MCP23S08_1_GPIO_8       (FTE_DEV_MCP23S08_1 | 8)

#define FTE_DEV_UCS_1                   (FTE_DEV_TYPE_UCS | 1)
#define FTE_DEV_UCM_1                   (FTE_DEV_TYPE_UCM | 1)

#define FTE_DEV_SRF_1                   (FTE_DEV_TYPE_SRF | 1)

#define FTE_GPIO_AD7785_0_CS            (GPIO_PORT_C | GPIO_PIN4)
#define FTE_GPIO_AD7785_1_CS            (GPIO_PORT_C | GPIO_PIN3)

#define FTE_GPIO_SHT_SDA                (GPIO_PORT_C | GPIO_PIN2)
#define FTE_GPIO_SHT_SCL                (GPIO_PORT_C | GPIO_PIN1)

#define FTE_GPIO_RESET                  (GPIO_PORT_B | GPIO_PIN3)
#if FTE_V2
#define FTE_GPIO_POWER_HOLD             (GPIO_PORT_D | GPIO_PIN12)
#define FTE_GPIO_POWER_IN               (GPIO_PORT_C | GPIO_PIN0)
#else
#define FTE_GPIO_POWER_HOLD             (GPIO_PORT_B | GPIO_PIN8)
#define FTE_GPIO_POWER_CTRL             (GPIO_PORT_B | GPIO_PIN9)
#endif
#define FTE_GPIO_SW_DETECT              (GPIO_PORT_B | GPIO_PIN11)
#define FTE_GPIO_FACTORY_RESET          (GPIO_PORT_B | GPIO_PIN10)
#define FTE_GPIO_LED_0                  (GPIO_PORT_C | GPIO_PIN12)
#ifdef  FTE_LCD_SUPPORTED
#define FTE_GPIO_LCD_POWER              (GPIO_PORT_B | GPIO_PIN5)
#else
#define FTE_GPIO_LED_1                  (GPIO_PORT_C | GPIO_PIN5)
#endif

#define FTE_GPIO_MCP23S08_RESET         (GPIO_PORT_C | GPIO_PIN2)
#define FTE_GPIO_MCP23S08_INT           (GPIO_PORT_C | GPIO_PIN3)
#define FTE_GPIO_MCP23S08_CS            (GPIO_PORT_C | GPIO_PIN4)

#define FTE_GPIO_GAS_OUTPUT             (GPIO_PORT_C | GPIO_PIN1)
#define FTE_GPIO_UART1_RTS              (GPIO_PORT_C | GPIO_PIN1)
#define FTE_GPIO_UCS_FLOWCTRL           (GPIO_PORT_D | GPIO_PIN0)
#define FTE_GPIO_UCS_FLOWCTRL2          (GPIO_PORT_D | GPIO_PIN1)

#define FTE_GPIO_1WIRE_0                (GPIO_PORT_C | GPIO_PIN3)
#define FTE_GPIO_1WIRE_1                (GPIO_PORT_C | GPIO_PIN4)

#define FTE_GPIO_DI_0                   (GPIO_PORT_C | GPIO_PIN11)

#define FTE_GPIO_AD7785_0_CS_MUX        (LWGPIO_MUX_C4_GPIO)
#define FTE_GPIO_AD7785_1_CS_MUX        (LWGPIO_MUX_C3_GPIO)

#define FTE_GPIO_SHT_SDA_MUX            (LWGPIO_MUX_C2_GPIO)
#define FTE_GPIO_SHT_SCL_MUX            (LWGPIO_MUX_C1_GPIO)

#define FTE_GPIO_RESET_MUX              (LWGPIO_MUX_B3_GPIO)
#if FTE_V2
#define FTE_GPIO_POWER_HOLD_MUX         (LWGPIO_MUX_D12_GPIO)
#define FTE_GPIO_POWER_IN_MUX           (LWGPIO_MUX_C0_GPIO)
#else
#define FTE_GPIO_POWER_HOLD_MUX         (LWGPIO_MUX_B8_GPIO)
#define FTE_GPIO_POWER_CTRL_MUX         (LWGPIO_MUX_B9_GPIO)
#endif
#define FTE_GPIO_SW_DETECT_MUX          (LWGPIO_MUX_B11_GPIO)
#define FTE_GPIO_FACTORY_RESET_MUX      (LWGPIO_MUX_B10_GPIO)
#define FTE_GPIO_LED_0_MUX              (LWGPIO_MUX_C12_GPIO)
#ifdef  FTE_LCD_SUPPORTED
#define FTE_GPIO_LCD_POWER_MUX          (LWGPIO_MUX_B5_GPIO)
#else
#define FTE_GPIO_LED_1_MUX              (LWGPIO_MUX_B5_GPIO)
#endif

#define FTE_GPIO_MCP32S08_RESET_MUX     (LWGPIO_MUX_C2_GPIO)
#define FTE_GPIO_MCP32S08_INT_MUX       (LWGPIO_MUX_C3_GPIO)
#define FTE_GPIO_MCP23S08_CS_MUX        (LWGPIO_MUX_C4_GPIO)

#define FTE_GPIO_GAS_OUTPUT_MUX       	(LWGPIO_MUX_C1_GPIO)
#define FTE_GPIO_UART1_RTS_MUX        	(LWGPIO_MUX_C1_GPIO)
#define FTE_GPIO_UCS_FLOWCTRL_MUX     	(LWGPIO_MUX_D0_GPIO)
#define FTE_GPIO_UCS_FLOWCTRL2_MUX    	(LWGPIO_MUX_D1_GPIO)

#define FTE_GPIO_1WIRE_0_MUX            (LWGPIO_MUX_C3_GPIO)
#define FTE_GPIO_1WIRE_1_MUX            (LWGPIO_MUX_C4_GPIO)

#define FTE_GPIO_DI_0_MUX               (LWGPIO_MUX_C11_GPIO)

#if FTE_TASCON_HEM12_SUPPORTED
#define FTE_UCS_1_FULL_DUPLEX               FALSE
#define FTE_UCS_1_BAUDRATE                  1200
#define FTE_UCS_1_DATABITS                  8
#define FTE_UCS_1_PARITY                    FTE_UART_PARITY_EVEN
#define FTE_UCS_1_STOPBITS                  FTE_UART_STOP_BITS_1
#elif FTE_TASCON_HEM12_06M_SUPPORTED
#define FTE_UCS_1_FULL_DUPLEX               FALSE
#define FTE_UCS_1_BAUDRATE                  19600
#define FTE_UCS_1_DATABITS                  8
#define FTE_UCS_1_PARITY                    FTE_UART_PARITY_EVEN
#define FTE_UCS_1_STOPBITS                  FTE_UART_STOP_BITS_1
#elif FTE_ELT_AQM100_SUPPORTED
#define FTE_UCS_1_FULL_DUPLEX               FALSE
#define FTE_UCS_1_BAUDRATE                  9600
#define FTE_UCS_1_DATABITS                  8
#define FTE_UCS_1_PARITY                    FTE_UART_PARITY_NONE
#define FTE_UCS_1_STOPBITS                  FTE_UART_STOP_BITS_1
#else
#define FTE_UCS_1_FULL_DUPLEX               TRUE
#define FTE_UCS_1_BAUDRATE                  9600
#define FTE_UCS_1_DATABITS                  8
#define FTE_UCS_1_PARITY                    FTE_UART_PARITY_NONE
#define FTE_UCS_1_STOPBITS                  FTE_UART_STOP_BITS_1
#endif

#if FTE_DS18B20_SUPPORTED
#define FTE_DS18B20_INTERVAL                10
#endif

#if FTE_MST_MEX510C_SUPPORTED
#define FTE_MST_MEX510C_INTERVAL            10
#endif

#if FTE_RTD_SUPPORTED
#define FTE_RTD_INTERVAL                    10
#endif

#if FTE_SHT_SUPPORTED
#define FTE_SHT_INTERVAL                    10
#define FTE_SHT_SCL_INTERVAL                0
#endif

#if FTE_TASCON_HEM12_SUPPORTED || FTE_TASCON_HEM12_06M_SUPPORTED
#define FTE_HEM12_INTERVAL                  10
#define FTE_HEM12_EVENT_TYPE                FTE_OBJ_EVENT_TYPE_NONE
#define FTE_HEM12_EVENT_DELAY               0   // 0 seconds
#define FTE_HEM12_EVENT_THRESHOLD           0   // 0 ppm
#define FTE_HEM12_EVENT_UPPER_LIMIT         0   // 0 ppm
#define FTE_HEM12_EVENT_LOWER_LIMIT         0
#endif

#if FTE_MOTEM_PN1500_SUPPORTED
#define FTE_BOTEM_PN1500_SENS_INTERVAL      2
#endif

#if FTE_GAI_VOLTAGE0_SUPPORTED || FTE_GAI_VOLTAGE1_SUPPORTED
#define FTE_GAI_VOLTAGE_SUPPORTED           1
#endif

#if FTE_GAI_CURRENT0_SUPPORTED || FTE_GAI_CURRENT1_SUPPORTED
#define FTE_GAI_CURRENT_SUPPORTED           1
#endif

#endif
