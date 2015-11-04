#ifndef __CRC_H__
#define __CRC_H__

uint_16 fte_crc_ccitt(uint_16 ulInit, const uint_8 *pData, uint_16 uiLen);
uint_32 crc32(uint_32 crc, const void *buf, uint_32 size);

#endif
