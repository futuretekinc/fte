#ifndef __CRC_H__
#define __CRC_H__

FTE_UINT16  FTE_CRC16
(
    const FTE_VOID_PTR  pData, 
    FTE_UINT32  ulLen
);

FTE_UINT16  FTE_CRC_CCITT
(
    FTE_UINT16  uiInit, 
    const FTE_UINT8_PTR pData, 
    FTE_UINT32  ulLen
);

FTE_UINT32  FTE_CRC32
(
    FTE_UINT32  ulCRC, 
    const FTE_VOID_PTR  pBuf, 
    FTE_UINT32  ulSize
);

#endif
