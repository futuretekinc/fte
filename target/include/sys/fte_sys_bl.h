#ifndef __FTE_SYS_BL_H__
#define __FTE_SYS_BL_H__

_mqx_uint       FTE_SYS_BL_startUpgrade(void);
_mqx_uint       FTE_SYS_BL_load(void);
_mqx_uint       FTE_SYS_BL_save(void);
int_32          FTE_SYS_BL_cmd(int_32 nArgc, char_ptr pArgv[]);
_mqx_uint       FTE_SYS_BL_getOID(uint_8_ptr pOID);
_mqx_uint       FTE_SYS_BL_getMAC(unsigned char *pMAC);

#endif