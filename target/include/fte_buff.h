#ifndef __FTS_BUFF_H__
#define __FTS_BUFF_H__

_mqx_uint fte_buff_init(uint_32 size);
_mqx_uint fte_buff_lock(pointer _PTR_ buffer);
_mqx_uint fte_buff_unlock(pointer buffer);

uint_32 fte_buff_size(void);
#endif
