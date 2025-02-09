#ifndef _FTE_I2C_H__
#define _FTE_I2C_H__

typedef struct _FTE_I2C_CONFIG_STRUCT
{
    FTE_UINT32         nID;
    FTE_UINT32         xType;
    FTE_CHAR_PTR        pName;
    FTE_UINT32         xPort;
    FTE_UINT32         nBaudrate;
    FTE_UINT32         xFlags;
}   FTE_I2C_CONFIG, _PTR_ FTE_I2C_CONFIG_PTR;

typedef FTE_I2C_CONFIG const _PTR_ FTE_I2C_CONFIG_CONST_PTR;

typedef struct _FTE_I2C_CHANNEL_STRUCT
{
    MQX_FILE_PTR    xFD;
    FTE_SYS_LOCK    xLock;
    FTE_UINT32      nBaudrate;
    FTE_UINT32      xFlags;
    FTE_UINT32      nCount;
}   FTE_I2C_CHANNEL, _PTR_ FTE_I2C_CHANNEL_PTR;

typedef struct _FTE_I2C_STRUCT
{
    struct _FTE_I2C_STRUCT *    pNext;
    FTE_I2C_CONFIG_CONST_PTR    pConfig;
    FTE_I2C_CHANNEL_PTR         pChannel;
    FTE_UINT32                     nParent;

}   FTE_I2C, _PTR_ FTE_I2C_PTR;

FTE_RET   FTE_I2C_create(FTE_I2C_CONFIG_PTR pConfig);
FTE_RET   FTE_I2C_attach(FTE_I2C_PTR pI2C, FTE_UINT32 nParent);
FTE_RET   FTE_I2C_detach(FTE_I2C_PTR pI2C);

FTE_UINT32  FTE_I2C_count(void);
FTE_I2C_PTR FTE_I2C_get(FTE_UINT32 xID);
FTE_UINT32  FTE_I2C_parent_get(FTE_I2C_PTR pI2C);
FTE_I2C_PTR FTE_I2C_get_first(void);
FTE_I2C_PTR FTE_I2C_get_next(FTE_I2C_PTR pI2C);

FTE_RET   FTE_I2C_set_flags(FTE_I2C_PTR pI2C, FTE_UINT32 xFlags);                         
FTE_RET   FTE_I2C_get_flags(FTE_I2C_PTR pI2C, FTE_UINT32_PTR pFlags);
FTE_RET   FTE_I2C_read(FTE_I2C_PTR pI2C, FTE_UINT8   nID, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);
FTE_RET   FTE_I2C_write(FTE_I2C_PTR pI2C, FTE_UINT8 nID, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);
FTE_RET   FTE_I2C_write_byte(FTE_I2C_PTR pI2C, FTE_UINT8 nID, FTE_UINT8 uiData);

FTE_INT32 FTE_I2C_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[] );
#endif
