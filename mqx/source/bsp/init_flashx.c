/**HEADER********************************************************************
* 
* Copyright (c) 2008 Freescale Semiconductor;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: init_flashx.c$
* $Version : 3.8.6.0$
* $Date    : Jun-6-2012$
*
* Comments:
*
*   This file contains the global generic settings for FLASHX driver.
*
*END************************************************************************/

#include "mqx.h" 
#include "bsp.h"

#if 1
#define FTE_LOADER_ADDRESS          0x00000000
#define FTE_LOADER_SIZE             0x0000E000
#define FTE_LOADER_PARAMS_ADDRESS   0x0000E000
#define FTE_LOADER_PARAMS_SIZE      0x00000800
#define FTE_IMAGE_START_ADDRESS     0x00010000
#define FTE_IMAGE_SIZE              0x00060000
#define FTE_DATA_SIZE               (BSP_INTERNAL_FLASH_SECTOR_SIZE * 2)

const FLASHX_FILE_BLOCK _bsp_flashx_file_blocks[] = {
    { "bl",         FTE_LOADER_ADDRESS                                , 
                    FTE_LOADER_ADDRESS + FTE_LOADER_SIZE - 1 },
    { "bl_static",  FTE_LOADER_PARAMS_ADDRESS, 
                    FTE_LOADER_PARAMS_ADDRESS + FTE_LOADER_PARAMS_SIZE - 1 },
    { "bl_dynamic", FTE_LOADER_PARAMS_ADDRESS + FTE_LOADER_PARAMS_SIZE, 
                    FTE_LOADER_PARAMS_ADDRESS + FTE_LOADER_PARAMS_SIZE * 2 - 1 },
    { "image",      FTE_IMAGE_START_ADDRESS, 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE - 1 },                    
    { "data0",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE, 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE - 1 },                    
    { "data1",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE, 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 2 - 1 },                    
    { "data2",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 2, 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 3 - 1 },                    
    { "data3",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 3 , 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 4 - 1 },                    
    { "data4",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 4 , 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 5 - 1 },                    
    { "data5",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 5 , 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 6 - 1 },                    
    { "data6",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 6 , 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 7 - 1 },                    
    { "data7",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 7 , 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 8 - 1 },                    
    { "log0",       FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 8 , 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 10 - 1 },                    
    { "log1",       FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 10 , 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 12- 1 },                    
    { "cert0",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 12, 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 14- 1 },                    
    { "cert1",      FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 14, 
                    FTE_IMAGE_START_ADDRESS + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 16- 1 },                    
    { NULL   ,                             0,                           0 }
};
#else
#define FTE_DATA_SIZE           BSP_INTERNAL_FLASH_SECTOR_SIZE
#define FTE_IMAGE_SIZE          (BSP_INTERNAL_FLASH_SIZE - FTE_DATA_SIZE * 6)

const FLASHX_FILE_BLOCK _bsp_flashx_file_blocks[] = {
    { "image",      BSP_INTERNAL_FLASH_BASE , 
                    BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE - 1 },                    
    { "data0",      BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE, 
                    BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE - 1 },                    
    { "data1",      BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE, 
                    BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 2 - 1 },                    
    { "data2",      BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 2, 
                    BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 3 - 1 },                    
    { "data3",      BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 3 , 
                    BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 4 - 1 },                    
    { "data4",      BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 4 , 
                    BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 5 - 1 },                    
    { "data5",      BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 5 , 
                    BSP_INTERNAL_FLASH_BASE + FTE_IMAGE_SIZE + FTE_DATA_SIZE * 6 - 1 },                    
    { NULL   ,                             0,                           0 }
};
#endif
const FLASHX_INIT_STRUCT _bsp_flashx_init = {
    0x00000000, /* BASE_ADDR should be 0 for internal flashes */
    _flashx_kinetisN_block_map, /* HW block map for KinetisN devices */
    _bsp_flashx_file_blocks, /* Files on the device defined by the BSP */
    &_flashx_ftfl_if, /* Interface for low level driver */
    32, /* For external devices, data lines for the flash. Not used for internal flash devices. */
    1,  /* Number of parallel external flash devices. Not used for internal flash devices. */
    0,  /* 0 if the write verify is requested, non-zero otherwise */
    NULL /* low level driver specific data */
};

/* EOF */
