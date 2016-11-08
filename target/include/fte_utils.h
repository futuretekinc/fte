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

void    fte_udelay(FTE_UINT32 usec);
FTE_BOOL fte_parse_ip_address( FTE_CHAR_PTR pIPString, FTE_UINT32_PTR pIP);
FTE_BOOL fte_parse_enet_address( FTE_CHAR_PTR pMACString, _enet_address enet_address);
FTE_UINT32 fte_parse_hex_string(FTE_CHAR_PTR pString, FTE_UINT8_PTR pBuff, FTE_UINT32 ulBuffLen);

void    fte_system_reset(void);

FTE_RET FTE_strToFLOAT
(
    FTE_CHAR_PTR    pString, 
    FTE_FLOAT_PTR   pValue
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
