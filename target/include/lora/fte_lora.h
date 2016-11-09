#ifndef __LORA_H__
#define __LORA_H__

FTE_RET FTE_LORA_init(void);
FTE_INT32   FTE_LORA_SHELL_cmd(FTE_INT32    nArgc, FTE_CHAR_PTR pArgv[]);

void    FTE_LORA_process(FTE_UINT32 ulParams);
void    FTE_LORA_comm(FTE_UINT32 ulParams);

FTE_RET FTE_LORA_send(FTE_UINT8_PTR pData, FTE_UINT32 ulDataSize);

#endif
