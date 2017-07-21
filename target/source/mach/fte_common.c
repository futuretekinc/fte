/**HEADER********************************************************************
*
* Copyright (c) 2010 FutureTek, Inc.;
* All Rights Reserved
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: fte_p5.c$
* $Version : 1.0.0.0$
* $Date    : May-18-2014$
*
* Comments:
*
*   Remote Power Controller
*
*END************************************************************************/
#include "fte_target.h"
#include <sh_rtcs.h>
#include "fte_config.h"
#include "fte_time.h"
#include "fte_drv.h"
#include "fte_log.h"
#include "sys/fte_sys.h"
#include "sys/fte_sys_timer.h"
#include "fte_object.h"

#include "fte_es1.c"

void    FTE_SYS_STATE_CB_changed( pointer pParams );

FTE_LWGPIO_CONFIG   pLWGPIOConfigs[] =
{
    {
        .nID        = FTE_DEV_LWGPIO_RESET,
        .nLWGPIO    = FTE_GPIO_RESET,
        .nMUX       = FTE_GPIO_RESET_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
    {
        .nID        = FTE_DEV_LWGPIO_POWER_HOLD,
        .nLWGPIO    = FTE_GPIO_POWER_HOLD,
        .nMUX       = FTE_GPIO_POWER_HOLD_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#if FTE_DEV_LWGPIO_POWER_CTRL
    {
        .nID        = FTE_DEV_LWGPIO_POWER_CTRL,
        .nLWGPIO    = FTE_GPIO_POWER_CTRL,
        .nMUX       = FTE_GPIO_POWER_CTRL_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_POWER_IN
    {
        .nID        = FTE_DEV_LWGPIO_POWER_IN,
        .nLWGPIO    = FTE_GPIO_POWER_IN,
        .nMUX       = FTE_GPIO_POWER_IN_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if  FTE_DEV_LWGPIO_SW_DETECT
    {
        .nID        = FTE_DEV_LWGPIO_SW_DETECT,
        .nLWGPIO    = FTE_GPIO_SW_DETECT,
        .nMUX       = FTE_GPIO_SW_DETECT_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_FACTORY_RESET
    {
        .nID        = FTE_DEV_LWGPIO_FACTORY_RESET,
        .nLWGPIO    = FTE_GPIO_FACTORY_RESET,
        .nMUX       = FTE_GPIO_FACTORY_RESET_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LCD_POWER
    {
        .nID        = FTE_DEV_LWGPIO_LCD_POWER,
        .nLWGPIO    = FTE_GPIO_LCD_POWER,
        .nMUX       = FTE_GPIO_LCD_POWER_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_SYS_LED_0
    {
        .nID        = FTE_DEV_LWGPIO_SYS_LED_0,
        .nLWGPIO    = FTE_GPIO_SYS_LED_0,
        .nMUX       = FTE_GPIO_SYS_LED_0_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_SYS_LED_1
    {
        .nID        = FTE_DEV_LWGPIO_SYS_LED_1,
        .nLWGPIO    = FTE_GPIO_SYS_LED_1,
        .nMUX       = FTE_GPIO_SYS_LED_1_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_0
    {
        .nID        = FTE_DEV_LWGPIO_LED_0,
        .nLWGPIO    = FTE_GPIO_LED_0,
        .nMUX       = FTE_GPIO_LED_0_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_1
    {
        .nID        = FTE_DEV_LWGPIO_LED_1,
        .nLWGPIO    = FTE_GPIO_LED_1,
        .nMUX       = FTE_GPIO_LED_1_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_2
    {
        .nID        = FTE_DEV_LWGPIO_LED_2,
        .nLWGPIO    = FTE_GPIO_LED_2,
        .nMUX       = FTE_GPIO_LED_2_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_3
    {
        .nID        = FTE_DEV_LWGPIO_LED_3,
        .nLWGPIO    = FTE_GPIO_LED_3,
        .nMUX       = FTE_GPIO_LED_3_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_4
    {
        .nID        = FTE_DEV_LWGPIO_LED_4,
        .nLWGPIO    = FTE_GPIO_LED_4,
        .nMUX       = FTE_GPIO_LED_4_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_5
    {
        .nID        = FTE_DEV_LWGPIO_LED_5,
        .nLWGPIO    = FTE_GPIO_LED_5,
        .nMUX       = FTE_GPIO_LED_5_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_6
    {
        .nID        = FTE_DEV_LWGPIO_LED_6,
        .nLWGPIO    = FTE_GPIO_LED_6,
        .nMUX       = FTE_GPIO_LED_6_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_7
    {
        .nID        = FTE_DEV_LWGPIO_LED_7,
        .nLWGPIO    = FTE_GPIO_LED_7,
        .nMUX       = FTE_GPIO_LED_7_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_8
    {
        .nID        = FTE_DEV_LWGPIO_LED_8,
        .nLWGPIO    = FTE_GPIO_LED_8,
        .nMUX       = FTE_GPIO_LED_8_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_9
    {
        .nID        = FTE_DEV_LWGPIO_LED_9,
        .nLWGPIO    = FTE_GPIO_LED_9,
        .nMUX       = FTE_GPIO_LED_9_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_10
    {
        .nID        = FTE_DEV_LWGPIO_LED_10,
        .nLWGPIO    = FTE_GPIO_LED_10,
        .nMUX       = FTE_GPIO_LED_10_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_11
    {
        .nID        = FTE_DEV_LWGPIO_LED_11,
        .nLWGPIO    = FTE_GPIO_LED_11,
        .nMUX       = FTE_GPIO_LED_11_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LED_12
    {
        .nID        = FTE_DEV_LWGPIO_LED_12,
        .nLWGPIO    = FTE_GPIO_LED_12,
        .nMUX       = FTE_GPIO_LED_12_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_DI_0
    {
        .nID        = FTE_DEV_LWGPIO_DI_0,
        .nLWGPIO    = FTE_GPIO_DI_0,
        .nMUX       = FTE_GPIO_DI_0_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_DI_1
    {
        .nID        = FTE_DEV_LWGPIO_DI_1,
        .nLWGPIO    = FTE_GPIO_DI_1,
        .nMUX       = FTE_GPIO_DI_1_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_DI_2
    {
        .nID        = FTE_DEV_LWGPIO_DI_2,
        .nLWGPIO    = FTE_GPIO_DI_2,
        .nMUX       = FTE_GPIO_DI_2_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_DI_3
    {
        .nID        = FTE_DEV_LWGPIO_DI_3,
        .nLWGPIO    = FTE_GPIO_DI_3,
        .nMUX       = FTE_GPIO_DI_3_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_DO_0
    {
        .nID        = FTE_DEV_LWGPIO_DO_0,
        .nLWGPIO    = FTE_GPIO_DO_0,
        .nMUX       = FTE_GPIO_DO_0_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_DO_1
    {
        .nID        = FTE_DEV_LWGPIO_DO_1,
        .nLWGPIO    = FTE_GPIO_DO_1,
        .nMUX       = FTE_GPIO_DO_1_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_DO_2
    {
        .nID        = FTE_DEV_LWGPIO_DO_2,
        .nLWGPIO    = FTE_GPIO_DO_2,
        .nMUX       = FTE_GPIO_DO_2_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_DO_3
    {
        .nID        = FTE_DEV_LWGPIO_DO_3,
        .nLWGPIO    = FTE_GPIO_DO_3,
        .nMUX       = FTE_GPIO_DO_3_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLO_0
    {
        .nID        = FTE_DEV_LWGPIO_RLO_0,
        .nLWGPIO    = FTE_GPIO_RLO_0,
        .nMUX       = FTE_GPIO_RLO_0_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLC_0
    {
        .nID        = FTE_DEV_LWGPIO_RLC_0,
        .nLWGPIO    = FTE_GPIO_RLC_0,
        .nMUX       = FTE_GPIO_RLC_0_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLO_1
    {
        .nID        = FTE_DEV_LWGPIO_RLO_1,
        .nLWGPIO    = FTE_GPIO_RLO_1,
        .nMUX       = FTE_GPIO_RLO_1_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLC_1
    {
        .nID        = FTE_DEV_LWGPIO_RLC_1,
        .nLWGPIO    = FTE_GPIO_RLC_1,
        .nMUX       = FTE_GPIO_RLC_1_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLO_2
    {
        .nID        = FTE_DEV_LWGPIO_RLO_2,
        .nLWGPIO    = FTE_GPIO_RLO_2,
        .nMUX       = FTE_GPIO_RLO_2_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLC_2
    {
        .nID        = FTE_DEV_LWGPIO_RLC_2,
        .nLWGPIO    = FTE_GPIO_RLC_2,
        .nMUX       = FTE_GPIO_RLC_2_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLO_3
    {
        .nID        = FTE_DEV_LWGPIO_RLO_3,
        .nLWGPIO    = FTE_GPIO_RLO_3,
        .nMUX       = FTE_GPIO_RLO_3_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLC_3
    {
        .nID        = FTE_DEV_LWGPIO_RLC_3,
        .nLWGPIO    = FTE_GPIO_RLC_3,
        .nMUX       = FTE_GPIO_RLC_3_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLO_4
    {
        .nID        = FTE_DEV_LWGPIO_RLO_4,
        .nLWGPIO    = FTE_GPIO_RLO_4,
        .nMUX       = FTE_GPIO_RLO_4_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLC_4
    {
        .nID        = FTE_DEV_LWGPIO_RLC_4,
        .nLWGPIO    = FTE_GPIO_RLC_4,
        .nMUX       = FTE_GPIO_RLC_4_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLO_5
    {
        .nID        = FTE_DEV_LWGPIO_RLO_5,
        .nLWGPIO    = FTE_GPIO_RLO_5,
        .nMUX       = FTE_GPIO_RLO_5_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_RLC_5
    {
        .nID        = FTE_DEV_LWGPIO_RLC_5,
        .nLWGPIO    = FTE_GPIO_RLC_5,
        .nMUX       = FTE_GPIO_RLC_5_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_AD7785_0_CS
    {
        .nID        = FTE_DEV_LWGPIO_AD7785_0_CS,
        .nLWGPIO    = FTE_GPIO_AD7785_0_CS,
        .nMUX       = FTE_GPIO_AD7785_0_CS_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_AD7785_1_CS
    {
        .nID        = FTE_DEV_LWGPIO_AD7785_1_CS,
        .nLWGPIO    = FTE_GPIO_AD7785_1_CS,
        .nMUX       = FTE_GPIO_AD7785_1_CS_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_M25P16_CS
    {
        .nID        = FTE_DEV_LWGPIO_M25P16_CS,
        .nLWGPIO    = FTE_GPIO_M25P16_CS,
        .nMUX       = FTE_GPIO_M25P16_CS_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_M25P16_WP
    {
        .nID        = FTE_DEV_LWGPIO_M25P16_WP,
        .nLWGPIO    = FTE_GPIO_M25P16_WP,
        .nMUX       = FTE_GPIO_M25P16_WP_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_M25P16_HOLD
    {
        .nID        = FTE_DEV_LWGPIO_M25P16_HOLD,
        .nLWGPIO    = FTE_GPIO_M25P16_HOLD,
        .nMUX       = FTE_GPIO_M25P16_HOLD_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_SHT_SDA
    {
        .nID        = FTE_DEV_LWGPIO_SHT_SDA,
        .nLWGPIO    = FTE_GPIO_SHT_SDA,
        .nMUX       = FTE_GPIO_SHT_SDA_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_SHT_SCL
    {
        .nID        = FTE_DEV_LWGPIO_SHT_SCL,
        .nLWGPIO    = FTE_GPIO_SHT_SCL,
        .nMUX       = FTE_GPIO_SHT_SCL_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_1WIRE_0
    {
        .nID        = FTE_DEV_LWGPIO_1WIRE_0,
        .nLWGPIO    = FTE_GPIO_1WIRE_0,
        .nMUX       = FTE_GPIO_1WIRE_0_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_1WIRE_1
    {
        .nID        = FTE_DEV_LWGPIO_1WIRE_1,
        .nLWGPIO    = FTE_GPIO_1WIRE_1,
        .nMUX       = FTE_GPIO_1WIRE_1_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_MCP23S08_RESET
    {
        .nID        = FTE_DEV_LWGPIO_MCP23S08_RESET,
        .nLWGPIO    = FTE_GPIO_MCP23S08_RESET,
        .nMUX       = FTE_GPIO_MCP32S08_RESET_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_MCP23S08_INT
    {
        .nID        = FTE_DEV_LWGPIO_MCP23S08_INT,
        .nLWGPIO    = FTE_GPIO_MCP23S08_INT,
        .nMUX       = FTE_GPIO_MCP32S08_INT_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_MCP23S08_CS
    {
        .nID        = FTE_DEV_LWGPIO_MCP23S08_CS,
        .nLWGPIO    = FTE_GPIO_MCP23S08_CS,
        .nMUX       = FTE_GPIO_MCP23S08_CS_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LORA_CS
    {
        .nID        = FTE_DEV_LWGPIO_LORA_CS,
        .nLWGPIO    = FTE_GPIO_LORA_CS,
        .nMUX       = FTE_GPIO_LORA_CS_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_LWGPIO_LORA_DIO0
    {
        .nID        = FTE_DEV_LWGPIO_LORA_DIO0,
        .nLWGPIO    = FTE_GPIO_LORA_DIO0,
        .nMUX       = FTE_GPIO_LORA_DIO0_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_LORA_DIO1
    {
        .nID        = FTE_DEV_LWGPIO_LORA_DIO1,
        .nLWGPIO    = FTE_GPIO_LORA_DIO1,
        .nMUX       = FTE_GPIO_LORA_DIO1_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_LORA_DIO2
    {
        .nID        = FTE_DEV_LWGPIO_LORA_DIO2,
        .nLWGPIO    = FTE_GPIO_LORA_DIO2,
        .nMUX       = FTE_GPIO_LORA_DIO2_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_LORA_DIO3 
    {
        .nID        = FTE_DEV_LWGPIO_LORA_DIO3,
        .nLWGPIO    = FTE_GPIO_LORA_DIO3,
        .nMUX       = FTE_GPIO_LORA_DIO3_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_LORA_DIO4
    {
        .nID        = FTE_DEV_LWGPIO_LORA_DIO4,
        .nLWGPIO    = FTE_GPIO_LORA_DIO4,
        .nMUX       = FTE_GPIO_LORA_DIO4_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_LORA_DIO5
    {
        .nID        = FTE_DEV_LWGPIO_LORA_DIO5,
        .nLWGPIO    = FTE_GPIO_LORA_DIO5,
        .nMUX       = FTE_GPIO_LORA_DIO5_MUX,
        .nDIR       = LWGPIO_DIR_INPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_LORA_CTX
    {
        .nID        = FTE_DEV_LWGPIO_LORA_CTX,
        .nLWGPIO    = FTE_GPIO_LORA_CTX,
        .nMUX       = FTE_GPIO_LORA_CTX_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_LWGPIO_GAS_OUTPUT
    {
        .nID        = FTE_DEV_LWGPIO_GAS_OUTPUT,
        .nLWGPIO    = FTE_GPIO_GAS_OUTPUT,
        .nMUX       = FTE_GPIO_GAS_OUTPUT_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif

#if FTE_DEV_LWGPIO_DUST_ON
    {
        .nID        = FTE_DEV_LWGPIO_DUST_ON,
        .nLWGPIO    = FTE_GPIO_DUST_ON,
        .nMUX       = FTE_GPIO_DUST_ON_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif

#if FTE_DEV_LWGPIO_UCS1_RCTRL_SUPPORTED
    {
        .nID        = FTE_DEV_LWGPIO_UCS1_RCTRL,
        .nLWGPIO    = FTE_GPIO_UCS1_RCTRL,
        .nMUX       = FTE_GPIO_UCS1_RCTRL_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif

#if FTE_DEV_LWGPIO_UCS1_WCTRL_SUPPORTED
    {
        .nID        = FTE_DEV_LWGPIO_UCS1_WCTRL,
        .nLWGPIO    = FTE_GPIO_UCS1_WCTRL,
        .nMUX       = FTE_GPIO_UCS1_WCTRL_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif

#if FTE_DEV_LWGPIO_UCS2_RCTRL_SUPPORTED
    {
        .nID        = FTE_DEV_LWGPIO_UCS2_RCTRL,
        .nLWGPIO    = FTE_GPIO_UCS2_RCTRL,
        .nMUX       = FTE_GPIO_UCS2_RCTRL_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif

#if FTE_DEV_LWGPIO_UCS2_WCTRL_SUPPORTED
    {
        .nID        = FTE_DEV_LWGPIO_UCS2_WCTRL,
        .nLWGPIO    = FTE_GPIO_UCS2_WCTRL,
        .nMUX       = FTE_GPIO_UCS2_WCTRL_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_LOW,
        .nActive    = LWGPIO_VALUE_HIGH,
        .nInactive  = LWGPIO_VALUE_LOW
    },
#endif

#if FTE_DEV_LWGPIO_IOEX_RESET
    {
        .nID        = FTE_DEV_LWGPIO_IOEX_RESET,
        .nLWGPIO    = FTE_GPIO_IOEX_RESET,
        .nMUX       = FTE_GPIO_IOEX_RESET_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif

#if FTE_DEV_LWGPIO_DIO_NODE_RESET
    {
        .nID        = FTE_DEV_LWGPIO_DIO_NODE_RESET,
        .nLWGPIO    = FTE_GPIO_DIO_NODE_RESET,
        .nMUX       = FTE_GPIO_DIO_NODE_RESET_MUX,
        .nDIR       = LWGPIO_DIR_OUTPUT,
        .nInit      = LWGPIO_VALUE_HIGH,
        .nActive    = LWGPIO_VALUE_LOW,
        .nInactive  = LWGPIO_VALUE_HIGH
    },
#endif
};

FTE_GPIO_CONFIG     pGPIOConfigs[] =
{
    {
        .nID        = FTE_DEV_GPIO_RESET,
        .nDevID     = FTE_DEV_LWGPIO_RESET,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
    {
        .nID        = FTE_DEV_GPIO_POWER_HOLD,
        .nDevID     = FTE_DEV_LWGPIO_POWER_HOLD,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#if FTE_DEV_LWGPIO_POWER_IN
    {
        .nID        = FTE_DEV_GPIO_POWER_IN,
        .nDevID     = FTE_DEV_LWGPIO_POWER_IN,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_POWER_CTRL
    {
        .nID        = FTE_DEV_GPIO_POWER_CTRL,
        .nDevID     = FTE_DEV_LWGPIO_POWER_CTRL,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if  FTE_DEV_GPIO_SW_DETECT
    {
        .nID        = FTE_DEV_GPIO_SW_DETECT,
        .nDevID     = FTE_DEV_LWGPIO_SW_DETECT,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if  FTE_DEV_GPIO_FACTORY_RESET
    {
        .nID        = FTE_DEV_GPIO_FACTORY_RESET,
        .nDevID     = FTE_DEV_LWGPIO_FACTORY_RESET,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if  FTE_DEV_GPIO_LCD_POWER
    {
        .nID        = FTE_DEV_GPIO_LCD_POWER,
        .nDevID     = FTE_DEV_LWGPIO_LCD_POWER,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_SYS_LED_0
    {
        .nID        = FTE_DEV_GPIO_SYS_LED_0,
        .nDevID     = FTE_DEV_LWGPIO_SYS_LED_0,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_SYS_LED_1
    {
        .nID        = FTE_DEV_GPIO_SYS_LED_1,
        .nDevID     = FTE_DEV_LWGPIO_SYS_LED_1,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_0
    {
        .nID        = FTE_DEV_GPIO_LED_0,
        .nDevID     = FTE_DEV_LWGPIO_LED_0,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_1
    {
        .nID        = FTE_DEV_GPIO_LED_1,
        .nDevID     = FTE_DEV_LWGPIO_LED_1,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_2
    {
        .nID        = FTE_DEV_GPIO_LED_2,
        .nDevID     = FTE_DEV_LWGPIO_LED_2,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_3
    {
        .nID        = FTE_DEV_GPIO_LED_3,
        .nDevID     = FTE_DEV_LWGPIO_LED_3,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_4
    {
        .nID        = FTE_DEV_GPIO_LED_4,
        .nDevID     = FTE_DEV_LWGPIO_LED_4,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_5
    {
        .nID        = FTE_DEV_GPIO_LED_5,
        .nDevID     = FTE_DEV_LWGPIO_LED_5,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_6
    {
        .nID        = FTE_DEV_GPIO_LED_6,
        .nDevID     = FTE_DEV_LWGPIO_LED_6,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_7
    {
        .nID        = FTE_DEV_GPIO_LED_7,
        .nDevID     = FTE_DEV_LWGPIO_LED_7,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_8
    {
        .nID        = FTE_DEV_GPIO_LED_8,
        .nDevID     = FTE_DEV_LWGPIO_LED_8,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_9
    {
        .nID        = FTE_DEV_GPIO_LED_9,
        .nDevID     = FTE_DEV_LWGPIO_LED_9,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_10
    {
        .nID        = FTE_DEV_GPIO_LED_10,
        .nDevID     = FTE_DEV_LWGPIO_LED_10,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_11
    {
        .nID        = FTE_DEV_GPIO_LED_11,
        .nDevID     = FTE_DEV_LWGPIO_LED_11,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_LED_12
    {
        .nID        = FTE_DEV_GPIO_LED_12,
        .nDevID     = FTE_DEV_LWGPIO_LED_12,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW
    },
#endif
#if FTE_DEV_GPIO_DI_0
    {
        .nID        = FTE_DEV_GPIO_DI_0,
        .nDevID     = FTE_DEV_LWGPIO_DI_0,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_DI_1
    {
        .nID        = FTE_DEV_GPIO_DI_1,
        .nDevID     = FTE_DEV_LWGPIO_DI_1,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_DI_2
    {
        .nID        = FTE_DEV_GPIO_DI_2,
        .nDevID     = FTE_DEV_LWGPIO_DI_2,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_DI_3
    {
        .nID        = FTE_DEV_GPIO_DI_3,
        .nDevID     = FTE_DEV_LWGPIO_DI_3,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_DO_0
    {
        .nID        = FTE_DEV_GPIO_DO_0,
        .nDevID     = FTE_DEV_LWGPIO_DO_0,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_DO_1
    {
        .nID        = FTE_DEV_GPIO_DO_1,
        .nDevID     = FTE_DEV_LWGPIO_DO_1,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_DO_2
    {
        .nID        = FTE_DEV_GPIO_DO_2,
        .nDevID     = FTE_DEV_LWGPIO_DO_2,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_DO_3
    {
        .nID        = FTE_DEV_GPIO_DO_3,
        .nDevID     = FTE_DEV_LWGPIO_DO_3,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLO_0
    {
        .nID        = FTE_DEV_GPIO_RLO_0,
        .nDevID     = FTE_DEV_LWGPIO_RLO_0,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLC_0
    {
        .nID        = FTE_DEV_GPIO_RLC_0,
        .nDevID     = FTE_DEV_LWGPIO_RLC_0,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLO_1
    {
        .nID        = FTE_DEV_GPIO_RLO_1,
        .nDevID     = FTE_DEV_LWGPIO_RLO_1,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLC_1
    {
        .nID        = FTE_DEV_GPIO_RLC_1,
        .nDevID     = FTE_DEV_LWGPIO_RLC_1,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLO_2
    {
        .nID        = FTE_DEV_GPIO_RLO_2,
        .nDevID     = FTE_DEV_LWGPIO_RLO_2,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLC_2
    {
        .nID        = FTE_DEV_GPIO_RLC_2,
        .nDevID     = FTE_DEV_LWGPIO_RLC_2,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLO_3
    {
        .nID        = FTE_DEV_GPIO_RLO_3,
        .nDevID     = FTE_DEV_LWGPIO_RLO_3,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLC_3
    {
        .nID        = FTE_DEV_GPIO_RLC_3,
        .nDevID     = FTE_DEV_LWGPIO_RLC_3,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLO_4
    {
        .nID        = FTE_DEV_GPIO_RLO_4,
        .nDevID     = FTE_DEV_LWGPIO_RLO_4,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLC_4
    {
        .nID        = FTE_DEV_GPIO_RLC_4,
        .nDevID     = FTE_DEV_LWGPIO_RLC_4,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLO_5
    {
        .nID        = FTE_DEV_GPIO_RLO_5,
        .nDevID     = FTE_DEV_LWGPIO_RLO_5,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_RLC_5
    {
        .nID        = FTE_DEV_GPIO_RLC_5,
        .nDevID     = FTE_DEV_LWGPIO_RLC_5,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH
    },
#endif
#if FTE_DEV_GPIO_MCP23S08_1_GPIO_1
    {
        .nID        = FTE_DEV_GPIO_MCP23S08_1_GPIO_1,
        .nDevID     = FTE_DEV_MCP23S08_1_GPIO_1,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH,
    },
#endif
#if FTE_DEV_GPIO_MCP23S08_1_GPIO_2
{
        .nID        = FTE_DEV_GPIO_MCP23S08_1_GPIO_2,
        .nDevID     = FTE_DEV_MCP23S08_1_GPIO_2,
        .nDIR       = FTE_GPIO_DIR_INPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH,
    },
#endif
#if FTE_DEV_GPIO_MCP23S08_1_GPIO_3
    {
        .nID        = FTE_DEV_GPIO_MCP23S08_1_GPIO_3,
        .nDevID     = FTE_DEV_MCP23S08_1_GPIO_3,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH,
    },
#endif
#if FTE_DEV_GPIO_MCP23S08_1_GPIO_4
    {
        .nID        = FTE_DEV_GPIO_MCP23S08_1_GPIO_4,
        .nDevID     = FTE_DEV_MCP23S08_1_GPIO_4,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_LOW,
        .nActive    = FTE_GPIO_VALUE_HIGH,
    },
#endif
#if FTE_DEV_GPIO_GAS_OUTPUT
    {
        .nID        = FTE_DEV_GPIO_GAS_OUTPUT,
        .nDevID     = FTE_DEV_LWGPIO_GAS_OUTPUT,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW,
    },
#endif
#if FTE_DEV_GPIO_DUST_ON
    {
        .nID        = FTE_DEV_GPIO_DUST_ON,
        .nDevID     = FTE_DEV_LWGPIO_DUST_ON,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW,
    },
#endif
#if FTE_DEV_GPIO_IOEX_RESET
    {
        .nID        = FTE_DEV_GPIO_IOEX_RESET,
        .nDevID     = FTE_DEV_LWGPIO_IOEX_RESET,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW,
    },
#endif

#if FTE_DEV_GPIO_DIO_NODE_RESET
    {
        .nID        = FTE_DEV_GPIO_DIO_NODE_RESET,
        .nDevID     = FTE_DEV_LWGPIO_DIO_NODE_RESET,
        .nDIR       = FTE_GPIO_DIR_OUTPUT,
        .nInit      = FTE_GPIO_VALUE_HIGH,
        .nActive    = FTE_GPIO_VALUE_LOW,
    },
#endif
};

#if FTE_I2C_SUPPORTED
FTE_I2C_CONFIG      pI2CConfigs[] =
{
#if  FTE_DEV_I2C_0
    {   .nID        = FTE_DEV_I2C_0,
        .xType      = 0,
        .pName      = NULL,
        .xPort      = 0,
        .nBaudrate  = 100000,
        .xFlags     = 0
    },
#endif
};
#endif


#if FTE_SPI_SUPPORTED
FTE_SPI_CONFIG      pSPIConfigs[] =
{
#if  FTE_DEV_SPI_0_0
#if FTE_AD7785_SUPPORTED
    {   .nID        = FTE_DEV_SPI_0_0,
        .xPort      = 0,
        .xCSGPIO    = FTE_DEV_LWGPIO_AD7785_0_CS,
        .nBaudrate  = 500000,
        .xFlags     = 0
    },
#endif
#if FTE_MCP23S08_SUPPORTED
    {   .nID        = FTE_DEV_SPI_0_0,
        .xPort      = 0,
        .xCSGPIO    = FTE_DEV_LWGPIO_MCP23S08_CS,
        .nBaudrate  = 500000,
        .xFlags     = 0
    },
#endif
#if FTE_LORA_SUPPORTED
    {   .nID        = FTE_DEV_SPI_0_0,
        .xPort      = 0,
        .xCSGPIO    = FTE_DEV_LWGPIO_LORA_CS,
        .nBaudrate  = 500000,
        .xFlags     = 0
    },
#endif
#endif
#if  FTE_DEV_SPI_0_1
#if FTE_AD7785_SUPPORTED
    {   .nID        = FTE_DEV_SPI_0_1,
        .xPort      = 0,
        .xCSGPIO    = FTE_DEV_LWGPIO_AD7785_1_CS,
        .nBaudrate  = 500000,
        .xFlags     = 0
    },
#endif
#endif
#if  FTE_DEV_SPI_2_0
    {   .nID        = FTE_DEV_SPI_2_0,
        .xPort      = 2,
        .xCSGPIO    = FTE_DEV_LWGPIO_M25P16_CS,
        .nBaudrate  = 500000,
        .xFlags     = 0
    },
#endif
};
#endif

#if FTE_SSD1305_SUPPORTED
FTE_SSD1305_CONFIG   pSSD1305Configs[] =
{
    {
        .nID        = FTE_DEV_SSD1305_0,
        .xType      = 0,
        .pName      = NULL,
        .xGPIOPower = FTE_DEV_GPIO_LCD_POWER,
        .xI2C       = FTE_DEV_I2C_0
    }
};
#endif

#if FTE_AD7785_SUPPORTED
FTE_AD7785_CONFIG   pAD7785Configs[] =
{
#if FTE_DEV_AD7785_0
     {
        .nID        = FTE_DEV_AD7785_0,
        .xSPI       = FTE_DEV_SPI_0_0
    },
#endif
#if FTE_DEV_AD7785_1
    {
        .nID        = FTE_DEV_AD7785_1,
        .xSPI       = FTE_DEV_SPI_0_1
    }
#endif
};
#endif

#if FTE_1WIRE_SUPPORTED
FTE_1WIRE_CONFIG p1WireConfigs[] =
{
#if FTE_DEV_LWGPIO_1WIRE_0
    {
        .nID        = FTE_DEV_TYPE_1WIRE | 0,
        .xGPIO      = FTE_DEV_LWGPIO_1WIRE_0,
        .nMaxDevices= FTE_1WIRE_MAX_DEVICES,
        .xFlags     = FTE_DEV_FLAG_SYSTEM_DEVICE
    },
#endif
#if FTE_DEV_LWGPIO_1WIRE_1
    {
        .nID        = FTE_DEV_TYPE_1WIRE | 1,
        .xGPIO      = FTE_DEV_LWGPIO_1WIRE_1,
        .nMaxDevices= FTE_1WIRE_MAX_DEVICES,
        .xFlags     = FTE_DEV_FLAG_SYSTEM_DEVICE
    }
#endif
};
#endif

#if FTE_MCP23S08_SUPPORTED
FTE_MCP23S08_CONFIG pMCP23S08Configs[] =
{
    {
        .nID        = FTE_DEV_MCP23S08_1,
        .pName      = "MCP23S08",
        .xSPI       = FTE_DEV_SPI_0_0,
        .xSlaveAddr = 0x03,
        .xGPIOReset = FTE_DEV_LWGPIO_MCP23S08_RESET,
        .xGPIOInt   = FTE_DEV_LWGPIO_MCP23S08_INT
    }
};

FTE_MCP23S08_GPIO_CONFIG pMCP23S08GPIOConfigs[] =
{
    {
        .nID        = FTE_DEV_MCP23S08_1_GPIO_1,
        .pName      = "MCP23S08_1_1",
        .nDrvID     = FTE_DEV_MCP23S08_1,
        .nBit       = 0,
        .nDir       = FTE_GPIO_DIR_INPUT
    },
    {
        .nID        = FTE_DEV_MCP23S08_1_GPIO_2,
        .pName      = "MCP23S08_1_2",
        .nDrvID     = FTE_DEV_MCP23S08_1,
        .nBit       = 1,
        .nDir       = FTE_GPIO_DIR_INPUT
    },
    {
        .nID        = FTE_DEV_MCP23S08_1_GPIO_3,
        .pName      = "MCP23S08_1_3",
        .nDrvID     = FTE_DEV_MCP23S08_1,
        .nBit       = 2,
        .nDir       = FTE_GPIO_DIR_OUTPUT
    },
    {
        .nID        = FTE_DEV_MCP23S08_1_GPIO_4,
        .pName      = "MCP23S08_1_4",
        .nDrvID     = FTE_DEV_MCP23S08_1,
        .nBit       = 3,
        .nDir       = FTE_GPIO_DIR_OUTPUT
    }
};
#endif

#if FTE_UCS_SUPPORTED
const FTE_UCS_CONFIG pUCSConfigs[] =
{
#if FTE_DEV_UCS_1 
    {
        .nID        = FTE_DEV_UCS_1,
        .pName      = "UCS_1",
        .pUART      = "ittyc:",
#if FTE_DEV_LWGPIO_UCS1_WCTRL_SUPPORTED
        .nFlowCtrlID= FTE_DEV_LWGPIO_UCS1_WCTRL,
#else
        .nFlowCtrlID= 0,
#endif
#if FTE_DEV_LWGPIO_UCS1_RCTRL_SUPPORTED
        .nFlowCtrl2ID= FTE_DEV_LWGPIO_UCS1_RCTRL,
#else
        .nFlowCtrl2ID= 0,
#endif
        .bFullDuplex= FTE_UCS_1_FULL_DUPLEX,
        .nBaudrate  = FTE_UCS_1_BAUDRATE,
        .nDataBits  = FTE_UCS_1_DATABITS,
        .nParity    = FTE_UCS_1_PARITY,
        .nStopBits  = FTE_UCS_1_STOPBITS,
        .nRecvBufLen= 256,
        .nSendBufLen= 256
    },
#endif
#if FTE_DEV_UCS_2
    {
        .nID        = FTE_DEV_UCS_2,
        .pName      = "UCS_2",
        .pUART      = "ittyf:",
#if FTE_DEV_LWGPIO_UCS2_RCTRL_SUPPORTED
        .nFlowCtrlID= FTE_DEV_LWGPIO_UCS2_RCTRL,
#else
        .nFlowCtrlID= 0,
#endif
#if FTE_DEV_LWGPIO_UCS2_WCTRL_SUPPORTED
        .nFlowCtrl2ID= FTE_DEV_LWGPIO_UCS2_WCTRL,
#else
        .nFlowCtrl2ID= 0,
#endif
        .bFullDuplex= FTE_UCS_2_FULL_DUPLEX,
        .nBaudrate  = FTE_UCS_2_BAUDRATE,
        .nDataBits  = FTE_UCS_2_DATABITS,
        .nParity    = FTE_UCS_2_PARITY,
        .nStopBits  = FTE_UCS_2_STOPBITS,
        .nRecvBufLen= 256,
        .nSendBufLen= 256
    },
#endif
};
#endif

#if FTE_UCM_SUPPORTED
const FTE_UCM_CONFIG pUCMConfigs[] =
{
    {
        .nID        = FTE_DEV_UCM_1,
        .nUCSID     = FTE_DEV_UCS_1
    }
};
#endif

#if FTE_M25P16_SUPPORTED
FTE_M25P16_CONFIG pM25P16Configs[] =
{
    {
        .nID        = FTE_DEV_M25P16_0,
        .pName      = "M25P16",
        .xSPI       = FTE_DEV_SPI_2_0,
        .xSlaveAddr = 0x00,
        .xGPIOHold  = FTE_DEV_LWGPIO_M25P16_HOLD,
        .xGPIOWP    = FTE_DEV_LWGPIO_M25P16_WP
    }
};
#endif

#if FTE_SRF_SUPPORTED
const FTE_SRF_CONFIG pSRFConfigs[] =
{
    {
        .nID        = FTE_DEV_SRF_1,
        .pName      = "SRF-1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE
    }
};
#endif

const FTE_LED_CONFIG xLedSysStatus0Config =
{
    .xCommon    =
    {
        .nID        = FTE_OBJ_LED_SYS0_STATUS,
        .pName      = "STATUS0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_SYS_LED_0,
    .nInit      = FALSE
};

const FTE_LED_CONFIG xLedSysStatus1Config =
{
    .xCommon    =
    {
        .nID        = FTE_OBJ_LED_SYS1_STATUS,
        .pName      = "STATUS1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_SYS_LED_1,
    .nInit      = FALSE
};

#if FTE_LCD_SUPPORTED
const FTE_LED_CONFIG xLCDPowerConfig =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_DO, 2),
        .pName      = "LCD Power",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LCD_POWER,
    .nInit      = FALSE
};
#endif

#if FTE_DEV_GPIO_LED_0
const FTE_LED_CONFIG xLed0Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 1),
        .pName      = "LED0",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_0,
    .nInit      = FALSE
};
#endif


#if FTE_DEV_GPIO_LED_1
const FTE_LED_CONFIG xLed1Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 2),
        .pName      = "LED1",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_1,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_2
const FTE_LED_CONFIG xLed2Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 3),
        .pName      = "LED2",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_2,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_3
const FTE_LED_CONFIG xLed3Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 4),
        .pName      = "LED3",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_3,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_4
const FTE_LED_CONFIG xLed4Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 5),
        .pName      = "LED4",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_4,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_5
const FTE_LED_CONFIG xLed5Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 6),
        .pName      = "LED5",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_5,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_6
const FTE_LED_CONFIG xLed6Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 7),
        .pName      = "LED6",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_6,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_7
const FTE_LED_CONFIG xLed7Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 8),
        .pName      = "LED7",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_7,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_8
const FTE_LED_CONFIG xLed8Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 9),
        .pName      = "LED8",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_8,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_9
const FTE_LED_CONFIG xLed9Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 10),
        .pName      = "LED9",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_9,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_10
const FTE_LED_CONFIG xLed10Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 11),
        .pName      = "LED10",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_10,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_11
const FTE_LED_CONFIG xLed11Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 12),
        .pName      = "LED11",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_11,
    .nInit      = FALSE
};
#endif
#if FTE_DEV_GPIO_LED_12
const FTE_LED_CONFIG xLed12Config =
{
    .xCommon    =
    {
        .nID        = MAKE_SYSTEM_ID(FTE_OBJ_TYPE_LED, 13),
        .pName      = "LED12",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE,
    },
    .nGPIO      = FTE_DEV_GPIO_LED_12,
    .nInit      = FALSE
};
#endif

#if FTE_IOEX_SUPPORTED
const FTE_DO_CONFIG fte_ioex_reset_config =
{
    .xCommon    =
    {
        .nID        = FTE_OBJ_TYPE_IOEX_RESET,
        .pName      = "IOEX Reset",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    },
    .nGPIO      = FTE_DEV_GPIO_IOEX_RESET,
    .nLED       = 0
};
#endif 

#if FTE_MULTI_DIO_NODE_SUPPORTED
const FTE_DO_CONFIG fte_dio_node_reset_config =
{
    .xCommon    =
    {
        .nID        = FTE_OBJ_TYPE_DIO_NODE_RESET,
        .pName      = "DIO_NODE Reset",
        .xFlags     = FTE_OBJ_CONFIG_FLAG_ENABLE | FTE_OBJ_CONFIG_FLAG_TRAP_DIFF,
    },
    .nGPIO      = FTE_DEV_GPIO_DIO_NODE_RESET,
    .nLED       = 0
};
#endif

const 
FTE_PRODUCT_DESC FTE_productDescription =
{
    .pModel         = FTE_MODEL,
    .pManufacturer  = FTE_MANUFACTURER,
    .xVersion       =
    {
        .hw = FTE_HW_VERSION,
        .sw = FTE_SW_VERSION
    }
};

static const FTE_DRIVER_DESCRIPT    _pDriverDescripts[] =
{
    {
        .nType      = FTE_DEV_TYPE_LWGPIO,
        .pName      = "LWGPIO",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_LWGPIO_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_LWGPIO_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_LWGPIO_detach
    },
    {
        .nType      = FTE_DEV_TYPE_GPIO,
        .pName      = "GPIO",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_GPIO_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_GPIO_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_GPIO_detach
    },
    {
        .nType      = FTE_DEV_TYPE_SPI,
        .pName      = "SPI",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_SPI_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_SPI_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_SPI_detach
    },
#if FTE_SSD1305_SUPPORTED
    {
        .nType      = FTE_DEV_TYPE_SSD1305,
        .pName      = "SSD1305",
        .f_create   = (DRIVER_CREATE_FUNC)fte_ssd1305_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)fte_ssd1305_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)fte_ssd1305_detach
    },
#endif
#if FTE_AD7785_SUPPORTED
    {
        .nType      = FTE_DEV_TYPE_AD7785,
        .pName      = "AD7785",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_AD7785_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_AD7785_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_AD7785_detach
    },
#endif
#if FTE_MCP23S08_SUPPORTED
    {
        .nType      = FTE_DEV_TYPE_MCP23S08,
        .pName      = "MCP23S08",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_MCP23S08_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_MCP23S08_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_MCP23S08_detach
    },
#endif
#if FTE_1WIRE_SUPPORTED
    {
        .nType      = FTE_DEV_TYPE_1WIRE,
        .pName      = "1WIRE",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_1WIRE_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_1WIRE_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_1WIRE_detach
    },
#endif
#if FTE_UCS_SUPPORTED
    {
        .nType      = FTE_DEV_TYPE_UCS,
        .pName      = "UCS",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_UCS_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_UCS_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_UCS_detach
    },
#endif
#if FTE_UCM_SUPPORTED
    {
        .nType      = FTE_DEV_TYPE_UCM,
        .pName      = "UCM",
        .f_create   = (DRIVER_CREATE_FUNC)fte_ucm_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)fte_ucm_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)fte_ucm_detach
    },
#endif
#if FTE_M25P16_SUPPORTED
    {
        .nType      = FTE_DEV_TYPE_M25P16,
        .pName      = "M25P16",
        .f_create   = (DRIVER_CREATE_FUNC)FTE_M25P16_create,
        .f_attach   = (DRIVER_ATTACH_FUNC)FTE_M25P16_attach,
        .f_detach   = (DRIVER_DETACH_FUNC)FTE_M25P16_detach
    },
#endif
};


const FTE_OBJECT_CONFIG_PTR pSystemObjectConfigs[] =
{
    (FTE_OBJECT_CONFIG_PTR)&xLedSysStatus0Config,
    (FTE_OBJECT_CONFIG_PTR)&xLedSysStatus1Config,
#if 0
    (FTE_OBJECT_CONFIG_PTR)&xLed0Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed1Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed2Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed3Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed4Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed5Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed6Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed7Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed8Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed9Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed10Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed11Config,
    (FTE_OBJECT_CONFIG_PTR)&xLed12Config,
#endif
#if FTE_IOEX_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_ioex_reset_config
#endif
	  
#if FTE_MULTI_DIO_NODE_SUPPORTED
    (FTE_OBJECT_CONFIG_PTR)&fte_dio_node_reset_config
#endif
};

HTTPSRV_AUTH_USER_STRUCT users[] =
{
    { NULL, NULL} /* Array terminator */
};

HTTPSRV_AUTH_REALM_STRUCT auth_realms[] =
{
    { NULL,                NULL,            HTTPSRV_AUTH_INVALID, NULL} /* Array terminator */
};

const 
FTE_SYS_CONFIG FTE_defaultSystemConfig =
{
    .xFlags = 
    {
        .bSystemMonitor = FALSE
    },
    .ulAllowFailureCount    = FTE_OBJ_ALLOW_FAILURE_COUNT,
    .ulKeepAliveTime        = FTE_SYS_KEEP_ALIVE_TIME,                  /* seconds */
    .ulAutoSaveInterval     = FTE_SYS_AUTO_SAVE_INTERVAL                /* milliseconds */
};

const 
FTE_NET_CFG FTE_defaultNetConfig =
{
//    .nType      = FTE_NET_TYPE_STATIC,
    .nType      = FTE_NET_TYPE_DHCP,
    .xIPData    =
    {
        .ip     = IPADDR(192,168,1,222),
        .mask   = IPADDR(255,255,255,0),
        .gateway= IPADDR(192,168,1,1)
    },

#if FTE_SNMPD_SUPPORTED
    .xSNMP      =
    {
        .bEnable=   TRUE,
        .xTrap  =
        {
            .ulCount = 1,
            .pList  = { IPADDR(192,168,1,1), 0},
            .pCommunity = "public"
        }
    },
#endif

#if FTE_HTTPD_SUPPORTED
    .xHTTP      =
    {
        .bEnable=   FALSE,
        .ulAtthRealmCount   = 0,
        .pAuthRealms        =
        {
            {   NULL,                NULL,            HTTPSRV_AUTH_INVALID, NULL} /* Array terminator */
        }
    },
#endif

#if FTE_MQTT_SUPPORTED
    .xMQTT      =
    {
        .pClientID  = {0,},
        .bEnable    = FALSE,
        .xBroker    =
        {
            .xIPAddress     = FTE_NET_MQTT_BROKER,
            .usPort         = FTE_NET_MQTT_PORT,
            .ulKeepalive    = FTE_NET_MQTT_KEEPALIVE,
            .xAuth          =
            {
                .bEnabled   = FALSE,
            }
        },
#if FTE_SSL_SUPPORTED
        .xSSL           = 
        {
            .bEnabled   = FTE_NET_MQTT_WITH_SSL,
            .nMethod    = FTE_NET_MQTT_SSL_METHOD,
            .pCipher    = "AES256-SHA"
        },
#endif
        .ulPubTimeout = FTE_NET_MQTT_PUB_TIMEOUT   

    },
#endif
#if FTE_TELNETD_SUPPORTED
    .xTelnetd   =
    {
        .pID    = "admin",
        .pPasswd= "admin"
    },
#endif
#if FTE_MBTCP_SUPPORTED
    .xMBTCP     =
    {
        .usPort =   FTE_NET_MBTCP_PORT,
        .ulMaxSessions= FTE_NET_MBTCP_MAX_SESSION,
        .xMBSlave=  
        {
            .xType  = FTE_MODBUS_SLAVE_TYPE_GENERAL
        }
    },
#endif
#if FTE_MBTCP_MASTER_SUPPORTED
    .nMBTCPMasterCount  = 2,
    .xMBTCPMasters      =
    {
        {
            .ulUnitID   = 1,
            .usOffset   = 0,
            .bEnable    = TRUE,
            .xIPAddress = FTE_NET_MBTCP_MASTER_SLAVE_IP,
            .usPort     = FTE_NET_MBTCP_MASTER_PORT,
            .ulKeepalive= FTE_NET_MBTCP_MASTER_KEEPALIVE
        },
        {
            .ulUnitID   = 2,
            .usOffset   = 0,
            .bEnable    = TRUE,
            .xIPAddress = FTE_NET_MBTCP_MASTER_SLAVE_IP,
            .usPort     = FTE_NET_MBTCP_MASTER_PORT,
            .ulKeepalive= FTE_NET_MBTCP_MASTER_KEEPALIVE
        }
    }
#endif
};

static const 
FTE_SHELL_CONFIG    FTE_defaultShellConfig =
{
    .pUserID    = "admin",
    .pPasswd    = "admin",
    .ulTimeout  = FTE_SHELL_TIMEOUT
};

const 
FTE_CFG_DESC  FTE_CFG_desc =
{
    .pMTDs  =
    {
        "flashx:data0",
        "flashx:data1",
        "flashx:data2",
        "flashx:data3",
        "flashx:data4",
        "flashx:data5",
        "flashx:data6",
        "flashx:data7"
    },
    .pSystem    = &FTE_defaultSystemConfig,
    .pShell     = &FTE_defaultShellConfig,
    .pNetwork   = &FTE_defaultNetConfig,
#if FTE_DYNAMIC_OBJECT_SUPPORTED
    .nObjects   =  0,
    .pObjects   =  NULL,
#else
    .nObjects   =  sizeof(pInitObjConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR),
    .pObjects   =  pInitObjConfigs,
#endif
	.nEvents    = sizeof(pInitEventConfigs) / sizeof(FTE_CFG_EVENT_PTR),
    .pEvents    = pInitEventConfigs
};

FTE_PRODUCT_DESC const *FTE_getProductDescription(void)
{
    return  &FTE_productDescription;
}

FTE_RET FTE_PLATFORM_init(void)
{
    
    /* Init device drivers */
    for(FTE_INT32 i = 0 ; i < sizeof(_pDriverDescripts) / sizeof(FTE_DRIVER_DESCRIPT) ; i++)
    {
        FTE_DRV_init(&_pDriverDescripts[i]);
    }

    for(FTE_INT32 i = 0 ; i < sizeof(pLWGPIOConfigs) / sizeof(FTE_LWGPIO_CONFIG) ; i++)
    {
        FTE_LWGPIO_create(&pLWGPIOConfigs[i]);
    }

    for(FTE_INT32 i = 0 ; i < sizeof(pGPIOConfigs) / sizeof(FTE_GPIO_CONFIG) ; i++)
    {
        FTE_GPIO_create(&pGPIOConfigs[i]);
    }

#if FTE_I2C_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pI2CConfigs) / sizeof(FTE_I2C_CONFIG) ; i++)
    {
        FTE_I2C_create(&pI2CConfigs[i]);
    }
#endif

#if FTE_SSD1305_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pSSD1305Configs) / sizeof(FTE_SSD1305_CONFIG) ; i++)
    {
        FTE_SSD1305_create(&pSSD1305Configs[i]);
    }
#endif

#if FTE_SPI_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pSPIConfigs) / sizeof(FTE_SPI_CONFIG) ; i++)
    {
        FTE_SPI_create(&pSPIConfigs[i]);
    }
#endif

#if FTE_AD7785_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pAD7785Configs) / sizeof(FTE_AD7785_CONFIG) ; i++)
    {
        FTE_AD7785_create(&pAD7785Configs[i]);
    }
#endif

#if FTE_MCP23S08_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pMCP23S08Configs) / sizeof(FTE_MCP23S08_CONFIG) ; i++)
    {
        FTE_MCP23S08_create(&pMCP23S08Configs[i]);
    }

    for(int i = 0 ; i < sizeof(pMCP23S08GPIOConfigs) / sizeof(FTE_MCP23S08_GPIO_CONFIG) ; i++)
    {
        FTE_MCP23S08_GPIO_create(&pMCP23S08GPIOConfigs[i]);
    }
#endif

#if FTE_1WIRE_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(p1WireConfigs) / sizeof(FTE_1WIRE_CONFIG) ; i++)
    {
        FTE_1WIRE_create(&p1WireConfigs[i]);
    }
#endif

#if FTE_UCS_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pUCSConfigs) / sizeof(FTE_UCS_CONFIG) ; i++)
    {
        FTE_UCS_create(&pUCSConfigs[i]);
    }
#endif

#if FTE_UCM_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pUCMConfigs) / sizeof(FTE_UCM_CONFIG) ; i++)
    {
        FTE_UCM_create(&pUCMConfigs[i]);
    }
#endif

#if FTE_M25P16_SUPPORTED
    for(FTE_INT32 i = 0 ; i < sizeof(pM25P16Configs) / sizeof(FTE_M25P16_CONFIG) ; i++)
    {
        FTE_M25P16_create(&pM25P16Configs[i]);
    }
#endif

    FTE_SYS_powerHoldInit();
    FTE_SYS_DEVICE_resetInit();
#if  FTE_PWR_SW_SUPPORTED
    FTE_SYS_SWDetectInit();
#endif
#if FTE_FACTORY_RESET_SUPPORTED
    FTE_SYS_factoryResetInit();
#endif
    
    FTE_SYS_powerStateInit();

#if FTE_LCD_SUPPORTED
    FTE_LCD_init();
#endif
    
    for(FTE_INT32 i = 0 ; i < sizeof(pSystemObjectConfigs) / sizeof(FTE_OBJECT_CONFIG_PTR) ; i++)
    {
        FTE_OBJECT_PTR pObj = FTE_OBJ_create(pSystemObjectConfigs[i]);
        if (pObj != NULL)
        {
            FTE_OBJ_init(pObj);
            FTE_OBJ_start(pObj);
        }
    }

    /* Configuration Init & Loading from Flash Memory */
    FTE_CFG_init(&FTE_CFG_desc);

    FTE_OBJECT_CONFIG_PTR pConfig = NULL;

    FTE_SYS_STATE_setChangeCB(FTE_SYS_STATE_CB_changed);
    FTE_SYS_powerUp();

    FTE_EVENT_init();

    /* Create Object */
    for(FTE_INT32 i = 0 ; i < FTE_CFG_OBJ_count(0, 0) ; i++)
    {
        pConfig = FTE_CFG_OBJ_getAt(0, 0, i); 
        if (pConfig != NULL)
        {
            if (FTE_OBJ_create(pConfig) == NULL)
            {
                DEBUG("Can't create object[TYPE = %08lx]\n", pConfig->xCommon.nID);
            }
        }
    }
   
    for(FTE_INT32 i = 0 ; i < FTE_CFG_EVENT_count() ; i++)
    {
        FTE_CFG_EVENT_PTR pConfig;
        FTE_EVENT_PTR     pEvent;

        if (FTE_CFG_EVENT_getAt(i, &pConfig) == FTE_RET_OK)
        {
            if (FTE_EVENT_create(pConfig, &pEvent) == FTE_RET_OK)
            {
                if ((pConfig->ulEPID & 0x0000FFFF) != 0)
                {
                    FTE_OBJECT_PTR  pObj = FTE_OBJ_get(pConfig->ulEPID);

                    if (pObj != NULL)
                    {
                        FTE_OBJ_EVENT_attach(pObj, pEvent);
                    }
                }
                else
                {
                    FTE_UINT32 ulCount;
                    
                    ulCount = FTE_OBJ_count(pConfig->ulEPID, FTE_OBJ_CLASS_MASK, FALSE);
                    for(FTE_UINT32 i = 0 ; i < ulCount ; i++)
                    {
                        FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(pConfig->ulEPID, FTE_OBJ_CLASS_MASK, i, FALSE);
                        if (pObj != NULL)
                        {
                            FTE_OBJ_EVENT_attach(pObj, pEvent);
                        }                
                    }
                }
            }
        }
    }

 
    FTE_UINT32 ulCount = FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE);
    for(FTE_INT32 i = 0 ; i < ulCount ; i++)
    {
        FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_UNKNOWN, 0, i, FALSE);
        if (pObj != NULL)
        {
            FTE_OBJ_init(pObj);
        }                
    }
    
    FTE_SYS_STATE_initialized();

    return  FTE_RET_OK;
}

FTE_RET FTE_PLATFORM_run(void)
{
    FTE_UINT32 ulCount = FTE_OBJ_count(FTE_OBJ_TYPE_UNKNOWN, 0, FALSE);
    for(FTE_UINT32 i = 0 ; i < ulCount ; i++)
    {
        FTE_OBJECT_PTR  pObj = FTE_OBJ_getAt(FTE_OBJ_TYPE_UNKNOWN, 0, i, FALSE);
        if (pObj != NULL)
        {
            if (FTE_OBJ_IS_SET(pObj, FTE_OBJ_CONFIG_FLAG_ENABLE) && FTE_OBJ_IS_RESET(pObj, FTE_OBJ_CONFIG_FLAG_SYNC))
            {
                FTE_OBJ_start(pObj);
            }
        }                
    }
    
    return  FTE_RET_OK;
}

void    FTE_SYS_STATE_changed( void )
{
    FTE_SYS_STATE_CB_changed( 0 );
}

void    FTE_SYS_STATE_CB_changed
(
   FTE_VOID_PTR     pParams 
)
{
    static FTE_UINT32 ulState = 0;
    FTE_UINT32  ulNewState = 0;
    
    FTE_SYS_STATE_get(&ulNewState);    
    if (ulState != ulNewState)
    {
        ulState = ulNewState;
        if (ulState & FTE_STATE_ALERT)
        {
            FTE_LED_setValue(FTE_OBJ_LED_SYS0_STATUS, FTE_LED_STATE_ON);
            FTE_LED_setValue(FTE_OBJ_LED_SYS1_STATUS, FTE_LED_STATE_OFF);
        }
        else if (ulState & FTE_STATE_WARNING)
        {
            FTE_LED_setValue(FTE_OBJ_LED_SYS0_STATUS, FTE_LED_STATE_BLINK);
            FTE_LED_setValue(FTE_OBJ_LED_SYS1_STATUS, FTE_LED_STATE_OFF);
        }
        else
        {
            switch(ulState)
            {
            case    FTE_STATE_POWER_UP:
                {
                    FTE_LED_setValue(FTE_OBJ_LED_SYS0_STATUS, FTE_LED_STATE_ON);
                    FTE_LED_setValue(FTE_OBJ_LED_SYS1_STATUS, FTE_LED_STATE_OFF);
                }
                break;

            case    FTE_STATE_POWER_UP | FTE_STATE_INITIALIZED:
                {
                    FTE_LED_setValue(FTE_OBJ_LED_SYS0_STATUS, FTE_LED_STATE_OFF);
                    FTE_LED_setValue(FTE_OBJ_LED_SYS1_STATUS, FTE_LED_STATE_BLINK);
                }
                break;

            case    FTE_STATE_POWER_UP | FTE_STATE_INITIALIZED | FTE_STATE_CONNECTED:
                {
                    FTE_LED_setValue(FTE_OBJ_LED_SYS0_STATUS, FTE_LED_STATE_OFF);
                    FTE_LED_setValue(FTE_OBJ_LED_SYS1_STATUS, FTE_LED_STATE_ON);
                }
                break;
            }
        }
    }
}
