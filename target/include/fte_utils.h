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

void    fte_udelay(uint_32 usec);
boolean fte_parse_ip_address( char_ptr pIPString, uint_32 _PTR_ pIP);
boolean fte_parse_enet_address( char_ptr pMACString, _enet_address enet_address);
uint_32 fte_parse_hex_string(char_ptr pString, uint_8 *pBuff, uint_32 ulBuffLen);

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
