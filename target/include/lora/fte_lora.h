#ifndef __LORA_H__
#define __LORA_H__

_mqx_uint   FTE_LORA_init(void);
int_32      FTE_LORA_SHELL_cmd(int_32 argc, char_ptr argv[]);

void        FTE_LORA_process(uint_32 params);
void        FTE_LORA_comm(uint_32 params);

#endif
