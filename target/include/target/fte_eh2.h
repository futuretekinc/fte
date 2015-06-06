#ifndef __FTE_H__
#define __FTE_H__

#define FTE_EH2                         1

#define FTE_MODEL                       "FTE-EH2"

#define FTE_CONSOLE_SUPPORTED           0
#define FTE_PWR_SW_SUPPORTED            1
#define FTE_FACTORY_RESET_SUPPORTED     0

#define FTE_AD7785_SUPPORTED            0
#define FTE_1WIRE_SUPPORTED             0
#define FTE_MCP23S08_SUPPORTED          0

#define FTE_RTD_SUPPORTED               0
#define FTE_RTD0_SUPPORTED              0
#define FTE_RTD1_SUPPORTED              0
#define FTE_SHT_SUPPORTED               0
#define FTE_DS18B20_SUPPORTED           0
#define FTE_DI_SUPPORTED                1
#define FTE_DO_SUPPORTED                1
#define FTE_RL_SUPPORTED                1

#define FTE_DI_MAX_COUNT                4
#define FTE_DO_MAX_COUNT                4
#define FTE_RL_MAX_COUNT                5
#define FTE_PT100_MAX_COUNT             0
#define FTE_DS18B20_MAX_COUNT           0
#define FTE_SHT_MAX_COUNT               0
#define FTE_1WIRE_MAX_DEVICES           0

#define FTE_DEV_LWGPIO_DI_0             (FTE_DEV_TYPE_LWGPIO | 0x00)
#define FTE_DEV_LWGPIO_DI_1             (FTE_DEV_TYPE_LWGPIO | 0x01)
#define FTE_DEV_LWGPIO_DI_2             (FTE_DEV_TYPE_LWGPIO | 0x02)
#define FTE_DEV_LWGPIO_DI_3             (FTE_DEV_TYPE_LWGPIO | 0x03)
#define FTE_DEV_LWGPIO_DO_0             (FTE_DEV_TYPE_LWGPIO | 0x04)
#define FTE_DEV_LWGPIO_DO_1             (FTE_DEV_TYPE_LWGPIO | 0x05)
#define FTE_DEV_LWGPIO_DO_2             (FTE_DEV_TYPE_LWGPIO | 0x06)
#define FTE_DEV_LWGPIO_DO_3             (FTE_DEV_TYPE_LWGPIO | 0x07)
#define FTE_DEV_LWGPIO_RL_0_H           (FTE_DEV_TYPE_LWGPIO | 0x08)
#define FTE_DEV_LWGPIO_RL_0_L           (FTE_DEV_TYPE_LWGPIO | 0x09)
#define FTE_DEV_LWGPIO_RL_1_H           (FTE_DEV_TYPE_LWGPIO | 0x0a)
#define FTE_DEV_LWGPIO_RL_1_L           (FTE_DEV_TYPE_LWGPIO | 0x0b)
#define FTE_DEV_LWGPIO_RL_2_H           (FTE_DEV_TYPE_LWGPIO | 0x0c)
#define FTE_DEV_LWGPIO_RL_2_L           (FTE_DEV_TYPE_LWGPIO | 0x0d)
#define FTE_DEV_LWGPIO_RL_3_H           (FTE_DEV_TYPE_LWGPIO | 0x0e)
#define FTE_DEV_LWGPIO_RL_3_L           (FTE_DEV_TYPE_LWGPIO | 0x0f)
#define FTE_DEV_LWGPIO_RL_4_H           (FTE_DEV_TYPE_LWGPIO | 0x10)
#define FTE_DEV_LWGPIO_RL_4_L           (FTE_DEV_TYPE_LWGPIO | 0x11)
#define FTE_DEV_LWGPIO_LED_0            (FTE_DEV_TYPE_LWGPIO | 0x12)
#define FTE_DEV_LWGPIO_LED_1            (FTE_DEV_TYPE_LWGPIO | 0x13)
#define FTE_DEV_LWGPIO_LED_2            (FTE_DEV_TYPE_LWGPIO | 0x14)
#define FTE_DEV_LWGPIO_LED_3            (FTE_DEV_TYPE_LWGPIO | 0x15)
#define FTE_DEV_LWGPIO_LED_4            (FTE_DEV_TYPE_LWGPIO | 0x16)
#define FTE_DEV_LWGPIO_LED_5            (FTE_DEV_TYPE_LWGPIO | 0x17)
#define FTE_DEV_LWGPIO_LED_6            (FTE_DEV_TYPE_LWGPIO | 0x18)
#define FTE_DEV_LWGPIO_LED_7            (FTE_DEV_TYPE_LWGPIO | 0x19)
#define FTE_DEV_LWGPIO_LED_8            (FTE_DEV_TYPE_LWGPIO | 0x1a)
#define FTE_DEV_LWGPIO_LED_9            (FTE_DEV_TYPE_LWGPIO | 0x1b)
#define FTE_DEV_LWGPIO_LED_10           (FTE_DEV_TYPE_LWGPIO | 0x1c)
#define FTE_DEV_LWGPIO_LED_11           (FTE_DEV_TYPE_LWGPIO | 0x1d)
#define FTE_DEV_LWGPIO_LED_12           (FTE_DEV_TYPE_LWGPIO | 0x1e)

#define FTE_GPIO_RESET                  (GPIO_PORT_B | GPIO_PIN3)

#define FTE_GPIO_LED_0                  (GPIO_PORT_B | GPIO_PIN22)
#define FTE_GPIO_LED_1                  (GPIO_PORT_B | GPIO_PIN23)
#define FTE_GPIO_LED_2                  (GPIO_PORT_B | GPIO_PIN20)
#define FTE_GPIO_LED_3                  (GPIO_PORT_B | GPIO_PIN21)

#define FTE_GPIO_LED_4                  (GPIO_PORT_B | GPIO_PIN18)
#define FTE_GPIO_LED_5                  (GPIO_PORT_B | GPIO_PIN19)
#define FTE_GPIO_LED_6                  (GPIO_PORT_B | GPIO_PIN16)
#define FTE_GPIO_LED_7                  (GPIO_PORT_B | GPIO_PIN17)

#define FTE_GPIO_LED_8                  (GPIO_PORT_C | GPIO_PIN10)
#define FTE_GPIO_LED_9                  (GPIO_PORT_C | GPIO_PIN11)
#define FTE_GPIO_LED_10                 (GPIO_PORT_C | GPIO_PIN8)
#define FTE_GPIO_LED_11                 (GPIO_PORT_C | GPIO_PIN9)

#define FTE_GPIO_LED_12                 (GPIO_PORT_C | GPIO_PIN7)

#define FTE_GPIO_RL_0_L                 (GPIO_PORT_D | GPIO_PIN10)
#define FTE_GPIO_RL_1_L                 (GPIO_PORT_D | GPIO_PIN11)
#define FTE_GPIO_RL_2_L                 (GPIO_PORT_D | GPIO_PIN12)
#define FTE_GPIO_RL_3_L                 (GPIO_PORT_D | GPIO_PIN13)
#define FTE_GPIO_RL_4_L                 (GPIO_PORT_D | GPIO_PIN14)

#define FTE_GPIO_RL_0_H                 (GPIO_PORT_E | GPIO_PIN4)
#define FTE_GPIO_RL_1_H                 (GPIO_PORT_E | GPIO_PIN1)
#define FTE_GPIO_RL_2_H                 (GPIO_PORT_E | GPIO_PIN2)
#define FTE_GPIO_RL_3_H                 (GPIO_PORT_E | GPIO_PIN3)
#define FTE_GPIO_RL_4_H                 (GPIO_PORT_E | GPIO_PIN0)

#define FTE_GPIO_DI_0                   (GPIO_PORT_E | GPIO_PIN7)
#define FTE_GPIO_DI_1                   (GPIO_PORT_E | GPIO_PIN8)
#define FTE_GPIO_DI_2                   (GPIO_PORT_E | GPIO_PIN9)
#define FTE_GPIO_DI_3                   (GPIO_PORT_E | GPIO_PIN10)

#define FTE_GPIO_DO_0                   (GPIO_PORT_B | GPIO_PIN10)
#define FTE_GPIO_DO_1                   (GPIO_PORT_B | GPIO_PIN6)
#define FTE_GPIO_DO_2                   (GPIO_PORT_B | GPIO_PIN7)
#define FTE_GPIO_DO_3                   (GPIO_PORT_B | GPIO_PIN3)

#define FTE_GPIO_LED_0_MUX              (LWGPIO_MUX_B16_GPIO)
#define FTE_GPIO_LED_1_MUX              (LWGPIO_MUX_B17_GPIO)
#define FTE_GPIO_LED_2_MUX              (LWGPIO_MUX_B18_GPIO)
#define FTE_GPIO_LED_3_MUX              (LWGPIO_MUX_B19_GPIO)

#define FTE_GPIO_LED_4_MUX              (LWGPIO_MUX_B20_GPIO)
#define FTE_GPIO_LED_5_MUX              (LWGPIO_MUX_B21_GPIO)
#define FTE_GPIO_LED_6_MUX              (LWGPIO_MUX_B22_GPIO)
#define FTE_GPIO_LED_7_MUX              (LWGPIO_MUX_B23_GPIO)

#define FTE_GPIO_LED_8_MUX              (LWGPIO_MUX_C7_GPIO)
#define FTE_GPIO_LED_9_MUX              (LWGPIO_MUX_C8_GPIO)
#define FTE_GPIO_LED_10_MUX             (LWGPIO_MUX_C9_GPIO)
#define FTE_GPIO_LED_11_MUX             (LWGPIO_MUX_C10_GPIO)
#define FTE_GPIO_LED_12_MUX             (LWGPIO_MUX_C11_GPIO)

#define FTE_GPIO_RL_0_L_MUX             (LWGPIO_MUX_D10_GPIO)
#define FTE_GPIO_RL_1_L_MUX             (LWGPIO_MUX_D11_GPIO)
#define FTE_GPIO_RL_2_L_MUX             (LWGPIO_MUX_D12_GPIO)
#define FTE_GPIO_RL_3_L_MUX             (LWGPIO_MUX_D13_GPIO)
#define FTE_GPIO_RL_4_L_MUX             (LWGPIO_MUX_D14_GPIO)

#define FTE_GPIO_RL_0_H_MUX             (LWGPIO_MUX_E0_GPIO)
#define FTE_GPIO_RL_1_H_MUX             (LWGPIO_MUX_E1_GPIO)
#define FTE_GPIO_RL_2_H_MUX             (LWGPIO_MUX_E2_GPIO)
#define FTE_GPIO_RL_3_H_MUX             (LWGPIO_MUX_E3_GPIO)
#define FTE_GPIO_RL_4_H_MUX             (LWGPIO_MUX_E4_GPIO)

#define FTE_GPIO_DI_0_MUX               (LWGPIO_MUX_E7_GPIO)
#define FTE_GPIO_DI_1_MUX               (LWGPIO_MUX_E8_GPIO)
#define FTE_GPIO_DI_2_MUX               (LWGPIO_MUX_E9_GPIO)
#define FTE_GPIO_DI_3_MUX               (LWGPIO_MUX_E10_GPIO)

#define FTE_GPIO_DO_0_MUX               (LWGPIO_MUX_B10_GPIO)
#define FTE_GPIO_DO_1_MUX               (LWGPIO_MUX_B6_GPIO)
#define FTE_GPIO_DO_2_MUX               (LWGPIO_MUX_B7_GPIO)
#define FTE_GPIO_DO_3_MUX               (LWGPIO_MUX_B3_GPIO)

#define FTE_GPIO_RESET_MUX              (LWGPIO_MUX_B3_GPIO)

#endif
