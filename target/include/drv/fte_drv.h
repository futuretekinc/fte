#ifndef __FTE_DRIVER_H__
#define __FTE_DRIVER_H__

#define FTE_DEV_TYPE_ROOT           0xFFFFFF00

#define FTE_DEV_TYPE_LWGPIO         0x00010000
#define FTE_DEV_TYPE_SPI            0x00020000
#define FTE_DEV_TYPE_I2C            0x00030000
#define FTE_DEV_TYPE_UART           0x00040000

#define FTE_DEV_TYPE_1WIRE          0x00010100
#define FTE_DEV_TYPE_GPIO           0x00010200

#define FTE_DEV_TYPE_AD7785         0x00020100
#define FTE_DEV_TYPE_MCP23S08       0x00020200

#define FTE_DEV_TYPE_SSD1305        0x00030100

#define FTE_DEV_TYPE_UCS            0x00040100
#define FTE_DEV_TYPE_UCM            0x00040200
#define FTE_DEV_TYPE_SRF            0x00040300

#define FTE_DEV_TYPE_MASK           0xFFFFFF00
#define FTE_DEV_INDEX_MASK          0x000000FF

#define FTE_DEV_TYPE(x)             ((x) & FTE_DEV_TYPE_MASK)
#define FTE_DEV_INDEX(x)            ((x) & FTE_DEV_INDEX_MASK)

#define FTE_DEV_FLAG_SYSTEM_DEVICE  0x0001

typedef uint_32 fte_dev_type, _PTR_ fte_dev_type_ptr;

typedef _mqx_uint   (*DRIVER_CREATE_FUNC)(void *);
typedef _mqx_uint   (*DRIVER_ATTACH_FUNC)(uint_32, uint_32);
typedef _mqx_uint   (*DRIVER_DETACH_FUNC)(uint_32, uint_32);

typedef struct  _FTE_DRIVER_DESCRIPT_STRUCT
{
    fte_dev_type                        nType;
    char_ptr                            pName;
    DRIVER_CREATE_FUNC                  f_create;
    DRIVER_ATTACH_FUNC                  f_attach;
    DRIVER_DETACH_FUNC                  f_detach;
}   FTE_DRIVER_DESCRIPT, _PTR_ FTE_DRIVER_DESCRIPT_PTR;

typedef FTE_DRIVER_DESCRIPT const _PTR_ FTE_DRIVER_DESCRIPT_CONST_PTR;

typedef struct _FTE_DRIVER_CONFIG_STRUCT
{
    uint_32         nID;
}   FTE_DRIVER_CONFIG, _PTR_ FTE_DRIVER_CONFIG_PTR;

typedef struct _FTE_DRIVER_STRUCT 
{
    struct _FTE_DRIVER_STRUCT  *    pNext;
    FTE_DRIVER_CONFIG_PTR           pConfig;    
} FTE_DRIVER, _PTR_ FTE_DRIVER_PTR;

_mqx_uint   FTE_DRV_init(FTE_DRIVER_DESCRIPT_CONST_PTR pDriverDescript);
_mqx_uint   FTE_DRV_create(fte_dev_type xType, void *pParams);

#include "drv/fte_lwgpio.h"
#include "drv/fte_gpio.h"
#include "drv/fte_spi.h"
#include "drv/fte_i2c.h"
#include "drv/fte_ssd1305.h"
#include "drv/fte_ad7785.h"
#include "drv/fte_mcp23s08.h"
#include "drv/fte_mcp23s08_gpio.h"
#include "drv/fte_1wire.h"
#include "drv/fte_ucs.h"
#include "drv/fte_ucm.h"

#endif
