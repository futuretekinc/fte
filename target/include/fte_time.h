#ifndef __FTE_TIME_H__
#define __FTE_TIME_H__

_mqx_uint   FTE_TIME_init(void);

extern  uint_32     FTE_TIME_diff(TIME_STRUCT *time1, TIME_STRUCT *time);
extern  uint_32     FTE_TIME_toString(TIME_STRUCT *time, char_ptr pBuff, uint_32 nBuffLen);
extern  uint_32     FTE_TIME_toTime(char_ptr pBuff, TIME_STRUCT *pTime);

extern  uint_64     FTE_TIME_getMilliSeconds(void);

extern  int_32      FTE_TIME_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
