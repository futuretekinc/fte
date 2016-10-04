/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mqx.h"
#include "fte_target.h"
#include "fte_time.h"
#include "fte_lwgpio.h"
#include "utilities.h"
#include "lora_timer.h"
#include "delay.h"
#include "lora_gpio.h"
#include "lora_spi.h"
//#include "i2c.h"
#include "radio.h"
#include "sx1276/sx1276.h"
//#include "mpl3115.h"
//#include "rtc-board.h"
//#include "timer-board.h"
#include "sx1276-board.h"
//#include "selector.h"

#define USE_BAND_915    1
#define USE_MODEM_LORA  1
/*!
 * NULL definition
 */
#ifndef NULL
    #define NULL                                    ( ( void * )0 )
#endif

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0  
#endif

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED                                   (0x01030502)

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;
extern Gpio_t Led3;

/*!
 * Hex coder selector GPIO pins objects
 */
extern Gpio_t Sel1;
extern Gpio_t Sel2;
extern Gpio_t Sel3;
extern Gpio_t Sel4;

/*!
 * Debug GPIO pins objects
 */
#if defined( USE_DEBUG_PINS )
extern Gpio_t DbgPin1;
extern Gpio_t DbgPin2;
extern Gpio_t DbgPin3;
extern Gpio_t DbgPin4;
#endif

/*!
 * MCU objects
 */
//extern I2c_t I2c;

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardMeasureBatterieLevel( void );

/*!
 * \brief Gets the board 64 bits unique ID 
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

#define         __enable_irq( )
#define         __disable_irq( )



#endif // __BOARD_H__
