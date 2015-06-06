#ifndef __FTE_BL_MODEL_H__
#define __FTE_BL_MODEL_H__

#define FTE_BL_MODEL_ARCH_V1            1

#define FTE_BL_MODEL_MAIN_BOARD_V1      1

#define FTE_BL_MODEL_NETWORK_BOARD_V1   1   // realtek rtl8201
#define FTE_BL_MODEL_NETWORK_BOARD_V2   2   // marvell mv88e6060

#define FTE_BL_MODEL_SENSOR_BOARD_V1    1   // AD7785
#define FTE_BL_MODEL_SENSOR_BOARD_V2    2   // RS485 + GPIO
#define FTE_BL_MODEL_SENSOR_BOARD_V3    3   // DI + DO

#define MK_MODEL(arch, mb, nb, sb)  ((((arch) & 0xFF) << 24) | \
                                     (((mb)   & 0xFF) << 16) | \
                                     (((nb)   & 0xFF) <<  8) | \
                                     (((sb)   & 0xFF) <<  0)) 

#define FTE_BL_MODEL_NB_VERSION(model)  (((model) >> 8) & 0xFF)
#define FTE_BL_MODEL_SB_VERSION(model)  (((model) >> 0) & 0xFF)

#define FTE_BL_MODEL_1              MK_MODEL(FTE_BL_MODEL_ARCH_V1,\
                                             FTE_BL_MODEL_MAIN_BOARD_V1,\
                                             FTE_BL_MODEL_NETWORK_BOARD_V1,\
                                             FTE_BL_MODEL_SENSOR_BOARD_V1)

#define FTE_BL_MODEL_2              MK_MODEL(FTE_BL_MODEL_ARCH_V1,\
                                             FTE_BL_MODEL_MAIN_BOARD_V1,\
                                             FTE_BL_MODEL_NETWORK_BOARD_V2,\
                                             FTE_BL_MODEL_SENSOR_BOARD_V1)

#define FTE_BL_MODEL_3              MK_MODEL(FTE_BL_MODEL_ARCH_V1,\
                                             FTE_BL_MODEL_MAIN_BOARD_V1,\
                                             FTE_BL_MODEL_NETWORK_BOARD_V1,\
                                             FTE_BL_MODEL_SENSOR_BOARD_V2)

#define FTE_BL_MODEL_4              MK_MODEL(FTE_BL_MODEL_ARCH_V1,\
                                             FTE_BL_MODEL_MAIN_BOARD_V1,\
                                             FTE_BL_MODEL_NETWORK_BOARD_V2,\
                                             FTE_BL_MODEL_SENSOR_BOARD_V2)

#define FTE_BL_MODEL_5             MK_MODEL(FTE_BL_MODEL_ARCH_V1,\
                                             FTE_BL_MODEL_MAIN_BOARD_V1,\
                                             FTE_BL_MODEL_NETWORK_BOARD_V1,\
                                             FTE_BL_MODEL_SENSOR_BOARD_V3)

#define FTE_BL_MODEL_6             MK_MODEL(FTE_BL_MODEL_ARCH_V1,\
                                             FTE_BL_MODEL_MAIN_BOARD_V1,\
                                             FTE_BL_MODEL_NETWORK_BOARD_V2,\
                                             FTE_BL_MODEL_SENSOR_BOARD_V3)

#endif
