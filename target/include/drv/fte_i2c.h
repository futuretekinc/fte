#ifndef _FTE_I2C_H__
#define _FTE_I2C_H__

typedef struct _FTE_I2C_CONFIG_STRUCT
{
    uint_32         nID;
    uint_32         xType;
    char_ptr        pName;
    uint_32         xPort;
    uint_32         nBaudrate;
    uint_32         xFlags;
}   FTE_I2C_CONFIG, _PTR_ FTE_I2C_CONFIG_PTR;

typedef FTE_I2C_CONFIG const _PTR_ FTE_I2C_CONFIG_CONST_PTR;

typedef struct _FTE_I2C_CHANNEL_STRUCT
{
    MQX_FILE_PTR    xFD;
    LWSEM_STRUCT    xLWSEM;
    uint_32         nBaudrate;
    uint_32         xFlags;
    uint_32         nCount;
}   FTE_I2C_CHANNEL, _PTR_ FTE_I2C_CHANNEL_PTR;

typedef struct _FTE_I2C_STRUCT
{
    struct _FTE_I2C_STRUCT *    pNext;
    FTE_I2C_CONFIG_CONST_PTR    pConfig;
    FTE_I2C_CHANNEL_PTR         pChannel;
    uint_32                     nParent;

}   FTE_I2C, _PTR_ FTE_I2C_PTR;

_mqx_uint   FTE_I2C_create(FTE_I2C_CONFIG_PTR pConfig);
_mqx_uint   FTE_I2C_attach(FTE_I2C_PTR pI2C, uint_32 nParent);
_mqx_uint   FTE_I2C_detach(FTE_I2C_PTR pI2C);

uint_32     FTE_I2C_count(void);
FTE_I2C_PTR FTE_I2C_get(uint_32 xID);
uint_32     FTE_I2C_parent_get(FTE_I2C_PTR pI2C);
FTE_I2C_PTR FTE_I2C_get_first(void);
FTE_I2C_PTR FTE_I2C_get_next(FTE_I2C_PTR pI2C);

_mqx_uint   FTE_I2C_set_flags(FTE_I2C_PTR pI2C, uint_32 xFlags);                         
_mqx_uint   FTE_I2C_get_flags(FTE_I2C_PTR pI2C, uint_32 *pFlags);
_mqx_uint   FTE_I2C_read(FTE_I2C_PTR pI2C, uint_8   nID, uint_8_ptr pBuff, uint_32 ulLen);
_mqx_uint   FTE_I2C_write(FTE_I2C_PTR pI2C, uint_8 nID, uint_8_ptr pBuff, uint_32 ulLen);
_mqx_uint   FTE_I2C_write_byte(FTE_I2C_PTR pI2C, uint_8 nID, uint_8 uiData);

int_32      FTE_I2C_SHELL_cmd(int_32 argc, char_ptr argv[] );
#endif
