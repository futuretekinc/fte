#ifndef __FTE_UTILS_H__
#define __FTE_UTILS_H__

#ifndef isascii
#define in_range(c, lo, up)  ((uint_8)(c) >= lo && (uint_8)(c) <= up)
#define isascii(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#endif		

#define FTE_NTOHS(p)   (FTE_INT16)(((FTE_UINT16)((p) & 0xFF) << 8) | ((FTE_UINT16)((p) & 0xFF00) >> 8))
#define FTE_HTONS(p)   (FTE_INT16)(((FTE_UINT16)((p) & 0xFF) << 8) | ((FTE_UINT16)((p) & 0xFF00) >> 8))
#define FTE_NTOHUS(p)  (((FTE_UINT16)((p) & 0xFF) << 8) | ((FTE_UINT16)((p) & 0xFF00) >> 8))
#define FTE_HTONUS(p)  (((FTE_UINT16)((p) & 0xFF) << 8) | ((FTE_UINT16)((p) & 0xFF00) >> 8))
#define FTE_NTOHL(p)   (FTE_INT32)(((FTE_UINT32)((p) & 0xFF) << 24) | ((FTE_UINT32)((p) & 0xFF00) << 8) | \
                        ((FTE_UINT32)((p) & 0xFF0000) >> 8) | ((FTE_UINT32)((p) & 0xFF000000) >> 24))
#define FTE_HTONL(p)   (FTE_INT32)(((FTE_UINT32)((p) & 0xFF) << 24) | ((FTE_UINT32)((p) & 0xFF00) << 8) | \
                        ((FTE_UINT32)((p) & 0xFF0000) >> 8) | ((FTE_UINT32)((p) & 0xFF000000) >> 24))
#define FTE_NTOHUL(p)  (((FTE_UINT32)((p) & 0xFF) << 24) | ((FTE_UINT32)((p) & 0xFF00) << 8) | \
                        ((FTE_UINT32)((p) & 0xFF0000) >> 8) | ((FTE_UINT32)((p) & 0xFF000000) >> 24))
#define FTE_HTONUL(p)  (((FTE_UINT32)((p) & 0xFF) << 24) | ((FTE_UINT32)((p) & 0xFF00) << 8) | \
                        ((FTE_UINT32)((p) & 0xFF0000) >> 8) | ((FTE_UINT32)((p) & 0xFF000000) >> 24))

void    FTE_udelay(FTE_UINT32 usec);
FTE_RET FTE_strToIP( FTE_CHAR_PTR pIPString, FTE_UINT32_PTR pIP);
FTE_RET FTE_strToMAC( FTE_CHAR_PTR pMACString, _enet_address enet_address);
FTE_RET FTE_strToHexArray(FTE_CHAR_PTR pString, FTE_UINT8_PTR pBuff, FTE_UINT32 ulBuffLen, FTE_UINT32_PTR pLen);

FTE_RET FTE_strToFLOAT
(
    FTE_CHAR_PTR    pString, 
    FTE_FLOAT_PTR   pValue
);

FTE_RET FTE_strToUINT16
(
   FTE_CHAR_PTR     pString, 
   FTE_UINT16_PTR   pValue
);

FTE_RET FTE_strToUINT32
(
   FTE_CHAR_PTR     pString,
   FTE_UINT32_PTR   pValue
);

FTE_RET FTE_strToHex
(
    FTE_CHAR_PTR     pString,
    FTE_UINT32_PTR   pValue
);

#include "fte_crc.h"

#endif
