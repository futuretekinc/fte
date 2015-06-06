#ifndef _FTE_1WIRE_H__
#define _FTE_1WIRE_H__

#define FTE_1WIRE_BAUDRATE  10000
#define FTE_1WIRE_ROM_CODE_SIZE 8

typedef uint_8  FTE_1WIRE_ROM_CODE[FTE_1WIRE_ROM_CODE_SIZE];
typedef FTE_1WIRE_ROM_CODE _PTR_ FTE_1WIRE_ROM_CODE_PTR;

typedef struct _FTE_1WIRE_CONFIG_STRUCT
{
    uint_32     nID;
    uint_32     xType;
    char_ptr    pName;
    uint_32     xGPIO;
    uint_32     nMaxDevices;
    uint_32     xFlags;
}   FTE_1WIRE_CONFIG, _PTR_ FTE_1WIRE_CONFIG_PTR;

typedef FTE_1WIRE_CONFIG const _PTR_ FTE_1WIRE_CONFIG_CONST_PTR;

typedef struct _FTE_1WIRE_FAMILY_NAME_STRUCT
{
    uint_8      xFamilyCode;
    char_ptr    pName;
}   FTE_1WIRE_FAMILY_NAME, _PTR_ FTE_1WIRE_FAMILY_NAME_PTR;

typedef struct _FTE_1WIRE_STRUCT
{
    struct _FTE_1WIRE_STRUCT *  pNext;
    uint_32                     nParent;
    FTE_1WIRE_CONFIG_CONST_PTR  pConfig;
    FTE_1WIRE_ROM_CODE_PTR      pROMCodes;
    uint_32                     nROMCodes;
    LWSEM_STRUCT                xLWSEM;
    FTE_LWGPIO_PTR              pLWGPIO;
}   FTE_1WIRE, _PTR_ FTE_1WIRE_PTR;

_mqx_uint   FTE_1WIRE_create(FTE_1WIRE_CONFIG_CONST_PTR pConfig);
_mqx_uint   FTE_1WIRE_attach(FTE_1WIRE_PTR p1WIRE, uint_32 nParent);
_mqx_uint   FTE_1WIRE_detach(FTE_1WIRE_PTR p1WIRE);

FTE_1WIRE_PTR   FTE_1WIRE_get(uint_32 nID);

uint_32     FTE_1WIRE_count(void);
pointer     FTE_1WIRE_getFirst(void);
pointer     FTE_1WIRE_getNext(pointer);

_mqx_uint   FTE_1WIRE_lock(FTE_1WIRE_PTR p1WIRE);
_mqx_uint   FTE_1WIRE_unlock(FTE_1WIRE_PTR p1WIRE);
_mqx_uint   FTE_1WIRE_discovery(FTE_1WIRE_PTR p1WIRE);

_mqx_uint   FTE_1WIRE_reset(FTE_1WIRE_PTR p1WIRE);
_mqx_uint   FTE_1WIRE_read(FTE_1WIRE_PTR p1WIRE, uint_8_ptr pBuff, uint_32 nBuff);
_mqx_uint   FTE_1WIRE_write(FTE_1WIRE_PTR p1WIRE, uint_8_ptr pBuff, uint_32 nBuff);
_mqx_uint   FTE_1WIRE_writeByte(FTE_1WIRE_PTR p1WIRE, uint_8 nData);

uint_32     FTE_1WIRE_DEV_count(FTE_1WIRE_PTR p1WIRE);
_mqx_uint   FTE_1WIRE_DEV_getROMCode(FTE_1WIRE_PTR p1WIRE, uint_32 nIdx, FTE_1WIRE_ROM_CODE pROMCode);

char_ptr    FTE_1WIRE_getFailmyName(uint_32 xCode);

int_32      FTE_1WIRE_SHELL_cmd(int_32 argc, char_ptr argv[] );
#endif

