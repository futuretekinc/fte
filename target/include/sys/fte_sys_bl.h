#ifndef __FTE_SYS_BL_H__
#define __FTE_SYS_BL_H__

FTE_RET     FTE_SYS_BL_startUpgrade(void);
FTE_RET     FTE_SYS_BL_load(void);
FTE_RET     FTE_SYS_BL_save(void);
FTE_INT32   FTE_SYS_BL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[]);
FTE_RET     FTE_SYS_BL_getOID(FTE_UINT8_PTR pOID);
FTE_RET     FTE_SYS_BL_getMAC(FTE_UINT8_PTR pMAC);

#endif