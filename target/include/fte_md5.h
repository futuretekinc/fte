#ifndef __MD5_H__
#define __MD5_H__

/* Data structure for MD5 (Message Digest) computation */
typedef struct {
  uint_32   i[2];                           /* number of _bits_ handled mod 2^64 */
  uint_32   buf[4];                         /* scratch buffer */
  uchar     in[64];                         /* input buffer */
  uchar     digest[16];                     /* actual digest after MD5Final call */
} FTS_MD5_CTX, _PTR_ FTS_MD5_CTX_PTR;

void fte_md5_init(FTS_MD5_CTX_PTR ctx);
void fte_md5_update(FTS_MD5_CTX_PTR ctx, uchar_ptr inBuf, uint_32 inLen);
void fte_md5_final(FTS_MD5_CTX_PTR ctx);

void fte_md5(uchar_ptr inBuf, uint_32 inLen, uchar digest[16]);

#endif
