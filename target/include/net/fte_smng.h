#ifndef __FTE_SMNG_H__
#define __FTE_SMNG_H__

typedef struct
{
    uint_32 pulHideClass[16];
}   FTE_SMNGD_CFG;

uint_32   FTE_SMNGD_init(void _PTR_ Params);
char_ptr  FTE_SMNG_getDiscoveryMessage(void);

int_32  FTE_SMNGD_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
