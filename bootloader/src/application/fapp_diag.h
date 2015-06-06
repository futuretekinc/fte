#ifndef __FAPP_DIAG_H__
#define __FAPP_DIAG_H__

#include "fapp_config.h"
#include "fapp.h"

struct fapp_diag_image_header
{
    char            pModel[32];
    char            pVendor[32];
    unsigned long   ulVersion;
    unsigned long   ulLen;
    unsigned long   ulCheckSum;
};

int fapp_diag_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
int fapp_diag_firmware(void);
unsigned long fapp_crc32(unsigned long crc, const void *buf, unsigned long size);

#endif
