#ifndef __MD5_H__
#define __MD5_H__

/* Data structure for MD5 (Message Digest) computation */
typedef struct 
{
  FTE_UINT32    i[2];                           /* number of _bits_ handled mod 2^64 */
  FTE_UINT32    pScratchBuf[4];                         /* scratch buffer */
  FTE_UINT8     pInBuf[64];                         /* input buffer */
  FTE_UINT8     pDigest[16];                     /* actual digest after MD5Final call */
} FTS_MD5_CTX, _PTR_ FTS_MD5_CTX_PTR;

void FTE_MD5_init(FTS_MD5_CTX_PTR ctx);
void FTE_MD5_update(FTS_MD5_CTX_PTR ctx, FTE_UINT8_PTR inBuf, FTE_UINT32 inLen);
void FTE_MD5_final(FTS_MD5_CTX_PTR ctx);

void FTE_MD5_calc(FTE_UINT8_PTR inBuf, FTE_UINT32   inLen, FTE_UINT8 pDigest[16]);

#endif
