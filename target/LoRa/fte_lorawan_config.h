#ifndef __FTE_LORAWAN_CONFIG_H__
#define __FTE_LORAWAN_CONFIG_H__

#define FTE_LORAWAN_RF_FREQUENCY           920000000 // Hz
#define FTE_LORAWAN_TX_OUTPUT_POWER        14        // dBm

#define FTE_LORAWAN_BANDWITDH_125KHZ       0
#define FTE_LORAWAN_BANDWITDH_250KHZ       1
#define FTE_LORAWAN_BANDWITDH_500KHZ       2

#define FTE_LORAWAN_BANDWITDH              FTE_LORAWAN_BANDWITDH_250KHZ

#define FTE_LORAWAN_SF7                    7
#define FTE_LORAWAN_SF8                    8
#define FTE_LORAWAN_SF9                    9
#define FTE_LORAWAN_SF10                   10
#define FTE_LORAWAN_SF11                   11
#define FTE_LORAWAN_SF12                   12

#define FTE_LORAWAN_SF                     FTE_LORAWAN_SF7

#define FTE_LORAWAN_CODING_RATE_4_5        1        // 4/5
#define FTE_LORAWAN_CODING_RATE_4_6        2        // 4/6
#define FTE_LORAWAN_CODING_RATE_4_7        3        // 4/7
#define FTE_LORAWAN_CODING_RATE_4_8        4        // 4/8

#define FTE_LORAWAN_CODING_RATE            FTE_LORAWAN_CODING_RATE_4_6

#define FTE_LORAWAN_PREAMBLE_LENGTH        8        // Same for Tx and Rx
#define FTE_LORAWAN_SYMBOL_TIMEOUT         5        // Symbols

#define FTE_LORAWAN_FIX_LENGTH_PAYLOAD_ON  0

#define FTE_LORAWAN_IQ_INVERSION_ON        0

#define FTE_LORAWAN_RX_TIMEOUT_VALUE       1000000
#define FTE_LORAWAN_BUFFER_SIZE            256

#define FTE_LORAWAN_TX_DUTYCYCLE           100000 // 5 [s] value in us
#define FTE_LORAWAN_TX_DUTYCYCLE_RND        10000 // 1 [s] value in us


/*!
 * Returns individual channel mask
 *
 * \param[IN] channelIndex Channel index 1 based
 * \retval channelMask
 */

#define FTE_LORAWAN_DEFAULT_CHANNEL_MASK    (LC(1))
#endif
