#include "fte_target.h"
#include "FTE_I2C.h"

#if FTE_I2C_SUPPORTED

#define MAX_I2C_COUNT   2
//#define DEBUG_I2C       1
static _mqx_uint   _FTE_I2C_lock(FTE_I2C_PTR pI2C);
static _mqx_uint   _FTE_I2C_unlock(FTE_I2C_PTR pI2C);

static void _FTE_I2C_writeCommand(MQX_FILE_PTR fd, uint_8 uiCmd);
static void _FTE_I2C_write(MQX_FILE_PTR fd, uint_32 addr, uint_8_ptr pBuff, uint_32 ulLen);      
static void _FTE_I2C_read(MQX_FILE_PTR fd, uint_32 addr, uint_8_ptr pBuff, uint_32 ulLen);

static const   char_ptr channels[MAX_I2C_COUNT] = 
{
    "ii2c0:",
    "ii2c1:"
};

static FTE_I2C_CHANNEL    _pChannels[MAX_I2C_COUNT] = 
{
    { .xFD = NULL },
    { .xFD = NULL }
};

static FTE_I2C_PTR  _pHead  = NULL;
static uint_32      _nI2C   = 0;


_mqx_uint   FTE_I2C_create(FTE_I2C_CONFIG_PTR pConfig)
{
    FTE_I2C_PTR pI2C;
    
    pI2C = (FTE_I2C_PTR)FTE_MEM_allocZero(sizeof(FTE_I2C));
    if (pI2C == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }
    
    _pChannels[pConfig->xPort].xFlags       = pConfig->xFlags;

    pI2C->pConfig           = pConfig;
    pI2C->pChannel          = &_pChannels[pConfig->xPort];
//    pI2C->callback.CALLBACK = _FTE_I2C_set_cs;
//    pI2C->callback.USERDATA = (pointer)pI2C;    
    pI2C->pNext             = _pHead;
    
    _pHead = pI2C;
    _nI2C++;
        
    return  MQX_OK;
}

_mqx_uint   FTE_I2C_attach(FTE_I2C_PTR pI2C, uint_32 nParent)
{
    assert(pI2C != NULL);
    if (pI2C == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
        
    if (pI2C->pChannel->xFD == NULL)
    {
        /* Open the I2C driver */
        pI2C->pChannel->xFD = fopen (channels[pI2C->pConfig->xPort], NULL);
        if (pI2C->pChannel->xFD == NULL)
        {
            goto error;
        }
        
        if (_lwsem_create(&pI2C->pChannel->xLWSEM, 1) != MQX_OK)
        {
            goto error;
        }

        printf ("Set master mode ... ");
        if (I2C_OK != ioctl (pI2C->pChannel->xFD, IO_IOCTL_I2C_SET_MASTER_MODE, NULL))
        {
            goto error;
        }               

        pI2C->pChannel->nCount = 1;
    }  
    else
    {
        pI2C->pChannel->nCount++;
    }

    pI2C->nParent = nParent;
    
    return  MQX_OK;
    
error:
    
    if (pI2C->pChannel->xFD != NULL)
    {
        fclose(pI2C->pChannel->xFD);
        pI2C->pChannel->xFD = NULL;
    }
    
   
    return  MQX_ERROR;
}

_mqx_uint   FTE_I2C_detach(FTE_I2C_PTR pI2C)
{
    assert(pI2C != NULL);
    if (pI2C == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    if (pI2C->pChannel->nCount == 1)
    {
        /* Open the I2C driver */
        fclose(pI2C->pChannel->xFD);        
        pI2C->pChannel->xFD = NULL;
        _lwsem_destroy(&pI2C->pChannel->xLWSEM);        
        pI2C->pChannel->nCount = 0;
    }  
    else
    {
        pI2C->pChannel->nCount--;
    }

    pI2C->nParent = 0;
    
    return  MQX_OK;
}

uint_32     FTE_I2C_count(void)
{
    return  _nI2C;
}

FTE_I2C_PTR FTE_I2C_get_first(void)
{
    return  _pHead;
}

FTE_I2C_PTR FTE_I2C_get_next(FTE_I2C_PTR pI2C)
{
    if (pI2C == NULL)
    {
        return  NULL;
    }
    
    return  pI2C->pNext;
}

FTE_I2C_PTR FTE_I2C_get(uint_32 nID)
{
    FTE_I2C_PTR pI2C;
    
    pI2C = _pHead;
    while(pI2C != NULL)
    {
        if (pI2C->pConfig->nID == nID)
        {
            return  pI2C;
        }
        
        pI2C = pI2C->pNext;
    }

    return  NULL;
}

uint_32     FTE_I2C_parent_get(FTE_I2C_PTR pI2C)
{
    assert(pI2C != NULL);
    
    return  pI2C->nParent;
}


_mqx_uint   FTE_I2C_read(FTE_I2C_PTR pI2C, uint_8   nID, uint_8_ptr pBuff, uint_32 ulLen)
{
    assert(pI2C != NULL);
    
    if (pI2C == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    _FTE_I2C_lock(pI2C);

    if (I2C_OK != ioctl (pI2C->pChannel->xFD, IO_IOCTL_I2C_SET_DESTINATION_ADDRESS, &nID))
    {
        goto error;
    } 
    
    if (ulLen != fread(pBuff, 1, ulLen, pI2C->pChannel->xFD))
    {
        goto error;
    }
  
    _FTE_I2C_unlock(pI2C);
    
    return MQX_OK;
    
error:  
    _FTE_I2C_unlock(pI2C);
    
    return MQX_ERROR;
}

_mqx_uint   FTE_I2C_write(FTE_I2C_PTR pI2C, uint_8 nID, uint_8_ptr pBuff, uint_32 ulLen)
{
    assert(pI2C != NULL);
    
    if (pI2C == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    _FTE_I2C_lock(pI2C);

    if (I2C_OK != ioctl (pI2C->pChannel->xFD, IO_IOCTL_I2C_SET_DESTINATION_ADDRESS, &nID))
    {
        goto error;
    } 
    
    if (ulLen != fwrite(pBuff, 1, ulLen, pI2C->pChannel->xFD))
    {
        goto error;
    }

    _FTE_I2C_unlock(pI2C);
    
    return MQX_OK;
    
error:  
    _FTE_I2C_unlock(pI2C);
    
    return MQX_ERROR;
}

_mqx_uint   FTE_I2C_write_byte(FTE_I2C_PTR pI2C, uint_8 nID, uint_8 uiData)
{
    assert(pI2C != NULL);
    
    if (pI2C == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }

    _FTE_I2C_lock(pI2C);

    if (I2C_OK != ioctl (pI2C->pChannel->xFD, IO_IOCTL_I2C_SET_DESTINATION_ADDRESS, &nID))
    {
        goto error;
    } 
    
    if (1 != fwrite(&uiData, 1, 1, pI2C->pChannel->xFD))
    {
        goto error;
    }

    _FTE_I2C_unlock(pI2C);
    
    return MQX_OK;
    
error:  
    _FTE_I2C_unlock(pI2C);
    
    return MQX_ERROR;
}

_mqx_uint   FTE_I2C_set_baudrate(FTE_I2C_PTR pI2C, uint_32 ulBaudrate)
{
    return  MQX_OK;
}

_mqx_uint   FTE_I2C_get_baudrate(FTE_I2C_PTR pI2C, uint_32 *pulBaudrate)
{
    assert(pI2C != NULL);
    
    if (pI2C == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    *pulBaudrate = pI2C->pConfig->nBaudrate;

    return  MQX_OK;
}

_mqx_uint   FTE_I2C_set_flags(FTE_I2C_PTR pI2C, uint_32 xFlags)
{
    return  MQX_OK;
}

_mqx_uint   FTE_I2C_get_flags(FTE_I2C_PTR pI2C, uint_32 *pxFlags)
{
    assert(pI2C != NULL);
    if (pI2C == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
            
    *pxFlags = pI2C->pConfig->xFlags;

    return  MQX_OK;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
_mqx_uint   _FTE_I2C_lock(FTE_I2C_PTR pI2C)
{
    assert(pI2C != NULL);
    
    if (_lwsem_wait(&pI2C->pChannel->xLWSEM) != MQX_OK)
    {  
        DEBUG("\n_lwsem_wait failed");
        goto error;
    }
    
    return  MQX_OK;
error:
    
    return  MQX_ERROR;
}

_mqx_uint   _FTE_I2C_unlock(FTE_I2C_PTR pI2C)
{   
    assert(pI2C != NULL);
    
//    fflush(pI2C->pChannel->xFD);
    
    if (_lwsem_post(&pI2C->pChannel->xLWSEM) != MQX_OK)
    {
        DEBUG("\n_lwsem_post failed");
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
} 

#define I2C_DEVICE_INTERRUPT "ii2c0:"
#define I2C_EEPROM_BUS_ADDRESS  0x3c

int_32  FTE_I2C_SHELL_cmd(int_32 argc, char_ptr argv[] )
{ 
    boolean             print_usage, shorthelp = FALSE;
    int_32              return_code = SHELL_EXIT_SUCCESS;

    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
            }
            break;
            
        default:
            {
#if 0
                if (strcmp(argv[2], "read") == 0)
                {
                    FTE_I2C_PTR pI2C;
                    uint_32 id, nCmdLen, nRecvLen = 1, i;
                    uint_8  pSendBuff[64];
                    uint_8  pRecvBuff[64];
                    
                    if (argc < 4)
                    {
                       return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (! Shell_parse_number( argv[1], &id))  
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    pI2C = FTE_I2C_get(id);
                    if (pI2C == NULL)
                    {
                        goto error;
                    }
                    
                    if (! Shell_parse_number( argv[3], &nCmdLen))
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (argc < 4 + nCmdLen + 1)
                    {
                       return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    for(i = 0 ; i < nCmdLen ; i++)
                    {
                        uint_32 nValue;
                        Shell_parse_hexnum( argv[4 + i], &nValue);
                        pSendBuff[i] = nValue;
                    }
                    
                    if (! Shell_parse_number( argv[4 + nCmdLen], &nRecvLen))
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (MQX_OK != FTE_I2C_read(pI2C, (uint_8 *)&pSendBuff, nCmdLen, pRecvBuff, nRecvLen))
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    for (i = 0 ; i < nRecvLen ; i++)
                    {
                        printf("%02x ", pRecvBuff[i]);
                    }
                    printf("\n");
                }
                else if (strcmp(argv[2], "write") == 0)
                {
                    FTE_I2C_PTR pI2C;
                    uint_32 nID, nSendLen, i;
                    uint_8 pSendBuff[64];
                     
                    if (argc < 6)
                    {
                       return_code = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (! Shell_parse_number( argv[1], &nID))  
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    pI2C = FTE_I2C_get(nID);
                    
                    if (! Shell_parse_number( argv[3], &nSendLen))
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    for(i = 0 ; i < nSendLen ; i++)
                    {
                        uint_32 nValue;
                        if (! Shell_parse_hexnum( argv[4+i], &nValue))
                        {
                           return_code = SHELL_EXIT_ERROR;
                           goto error;
                        }
                        pSendBuff[i] = nValue;
                    }
                    
                    if (MQX_OK != FTE_I2C_write(pI2C, pSendBuff, nSendLen, NULL, 0))
                    {
                       return_code = SHELL_EXIT_ERROR;
                       goto error;
                    }
                }
                else
                {
                    print_usage = TRUE;
                }
#endif
            }
        }
    }

    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<id>] [ baudrate | flags | read | write ] [<len>] [<data>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<id>] [ baudrate | flags | read | write ] [<len>] [<data>]\n", argv[0]);
            printf("        id       - I2C Channel \n");
            printf("        baudrate - I2C speed \n");
        }
    }

    return   return_code;
}


void FTE_I2C_write_command(MQX_FILE_PTR fd, uint_8 uiCmd)
{
          _FTE_I2C_write(fd, 1, &uiCmd, 1);
}

/*FUNCTION****************************************************************
* 
* Function Name    : _FTE_I2C_write
* Returned Value   : void
* Comments         : 
*   Writes the provided data buffer 
*
*END*********************************************************************/
void _FTE_I2C_write
   (
      /* [IN] The file pointer for the I2C channel */
      MQX_FILE_PTR  fd,

      /* [IN] I2C bus address */
      uint_32       addr,

      /* [IN] The array of characters are to be written in EEPROM */
      uint_8_ptr    pBuff,

      /* [IN] Number of bytes in that buffer */
      uint_32       n      
   )
{ /* Body */

   uint_32       param;
   _mqx_int    length;
   _mqx_int    result;
   uint_8        mem[128];

   /* Protect I2C transaction in multitask environment */
//   _lwsem_wait (&lock);

   printf ("Writing %d bytes to address 0x%08x ...\n", n, addr);
   do
   {
      /* I2C bus address also contains memory block index */
      param = I2C_EEPROM_BUS_ADDRESS;
      printf ("  Set I2C bus address to 0x%02x ... ", param);
      if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_SET_DESTINATION_ADDRESS, &param))
      {
         printf ("OK\n");
      } else {
         printf ("ERROR\n");
      }

//      length = (_mqx_int)(I2C_EEPROM_PAGE_SIZE - (addr & (I2C_EEPROM_PAGE_SIZE - 1)));
///      if (length > n) length = n;
        length = n;
      /* Initiate start and send I2C bus address */
      printf ("  I2C start, send address and get ack ... ");
      fwrite (mem, 1, 0, fd);

      /* Check ack (device exists) */
      if (I2C_OK == ioctl (fd, IO_IOCTL_FLUSH_OUTPUT, &param))
      {
         printf ("OK ... ack == %d\n", param);
         if (param) 
         {
            /* Stop I2C transfer */
            printf ("  Stop transfer ... ");
            if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_STOP, NULL))
            {
               printf ("OK\n");
            } else {
               printf ("ERROR\n");
            }
            break;
         }
      } else {
         printf ("ERROR\n");
      }

      /* Write address within memory block */
#if I2C_EEPROM_MEMORY_WIDTH == 2
      mem[0] = (uint_8)(addr >> 8);
      mem[1] = (uint_8)addr;
      printf ("  Write to address 0x%02x%02x ... ", mem[0], mem[1]);
      do 
      {
         result = fwrite (mem, 1, 2, fd);
      } while (result < 2);
      printf ("OK\n");
#else
      mem[0] = (uint_8)addr;
      printf ("  Write to address 0x%02x ... ", mem[0]);
      do 
      {
         result = fwrite (mem, 1, 1, fd);
      } while (result < 1);
      printf ("OK\n");
#endif

      /* Page write of data */
      printf ("  Page write %d bytes ... ", length);
      result = 0;
      do
      {
         result += fwrite (pBuff + result, 1, length - result, fd);
      } while (result < length);
      printf ("OK\n");
      
      /* Wait for completion */
      printf ("  Flush ... ");
      result = fflush (fd);
      if (MQX_OK == result)
      {
         printf ("OK\n");
      } else {
         printf ("ERROR\n");
      }

      /* Stop I2C transfer - initiate EEPROM write cycle */
      printf ("  Stop transfer ... ");
      if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_STOP, NULL))
      {
         printf ("OK\n");
      } else {
         printf ("ERROR\n");
      }

      /* Wait for EEPROM write cycle finish - acknowledge */
      result = 0;
      do 
      {  /* Sends just I2C bus address, returns acknowledge bit and stops */
         fwrite (&result, 1, 0, fd);
         
         if (I2C_OK != ioctl (fd, IO_IOCTL_FLUSH_OUTPUT, &param))
         {
            printf ("  ERROR during wait (flush)\n");
         }
         
         if (I2C_OK != ioctl (fd, IO_IOCTL_I2C_STOP, NULL))
         {
            printf ("  ERROR during wait (stop)\n");
         }
         result++;
      } while ((param & 1) || (result <= 1));
      
      printf ("  EEPROM polled %d times.\n", result);

      /* Next round */
      pBuff += length;
      addr += length;
      n -= length;
        
   } while (n);

   /* Release the transaction lock */
//   _lwsem_post (&lock);
} /* Endbody */

   
/*FUNCTION****************************************************************
* 
* Function Name    : i2c_read_interrupt
* Returned Value   : void
* Comments         : 
*   Reads into the provided data buffer from address in I2C EEPROM
*
*END*********************************************************************/

void i2c_read_interrupt
   (
      /* [IN] The file pointer for the I2C channel */
      MQX_FILE_PTR fd,

      /* [IN] The address in EEPROM to read from */
      uint_32    addr,

      /* [IN] The array of characters to be written into */
      uchar_ptr  buffer,

      /* [IN] Number of bytes to read */
      _mqx_int   n      
   )
{ /* Body */
   _mqx_int    param;
   _mqx_int    result;
   uint_8        mem[128];

   printf ("Reading %d bytes from address 0x%08x ...\n", n, addr);
   if (0 == n) 
   {
      printf ("  Nothing to do.\n");
      return;
   }

   /* Protect I2C transaction in multitask environment */
//   _lwsem_wait (&lock);
   
   /* I2C bus address also contains memory block index */
   param = I2C_EEPROM_BUS_ADDRESS;
   printf ("  Set I2C bus address to 0x%02x ... ", param);
   if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_SET_DESTINATION_ADDRESS, &param))
   {
      printf ("OK\n");
   } else {
      printf ("ERROR\n");
   }

   /* Initiate start and send I2C bus address */
   printf ("  I2C start, send address and get ack ... ");
   fwrite (mem, 1, 0, fd);

   /* Check ack (device exists) */
   if (I2C_OK == ioctl (fd, IO_IOCTL_FLUSH_OUTPUT, &param))
   {
      printf ("OK ... ack == %d\n", param);
      if (param) 
      {
         /* Stop I2C transfer */
         printf ("  Stop transfer ... ");
         if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_STOP, NULL))
         {
            printf ("OK\n");
         } else {
            printf ("ERROR\n");
         }
         
         /* Release the transaction lock */
//         _lwsem_post (&lock);
         
         return;
      }
   } else {
      printf ("ERROR\n");
   }

   /* Write address within memory block */
#if I2C_EEPROM_MEMORY_WIDTH == 2
      mem[0] = (uint_8)(addr >> 8);
      mem[1] = (uint_8)addr;
      printf ("  Write to address 0x%02x%02x ... ", mem[0], mem[1]);
      do 
      {
         result = fwrite (mem, 1, 2, fd);
      } while (result < 2);
      printf ("OK\n");
#else
      mem[0] = (uint_8)addr;
      printf ("  Write to address 0x%02x ... ", mem[0]);
      do 
      {
         result = fwrite (mem, 1, 1, fd);
      } while (result < 1);
      printf ("OK\n");
#endif

   /* Wait for completion */
   printf ("  Flush ... ");
   result = fflush (fd);
   if (MQX_OK == result)
   {
      printf ("OK\n");
   } else {
      printf ("ERROR\n");
   }

   /* Restart I2C transfer for reading */
   printf ("  Initiate repeated start ... ");
   if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_REPEATED_START, NULL))
   {
      printf ("OK\n");
   } else {
      printf ("ERROR\n");
   }

   /* Set read request */
   param = n;
   printf ("  Set number of bytes requested to %d ... ", param);
   if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_SET_RX_REQUEST, &param))
   {
      printf ("OK\n");
   } else {
      printf ("ERROR\n");
   }

   /* Read all data */
   printf ("  Read %d bytes ... ", n);
   result = 0;
   do
   {
      result += fread (buffer + result, 1, n - result, fd);
   } while (result < n);
   printf ("OK\n");
      
   /* Stop I2C transfer - initiate EEPROM write cycle */
   printf ("  Stop transfer ... ");
   if (I2C_OK == ioctl (fd, IO_IOCTL_I2C_STOP, NULL))
   {
      printf ("OK\n");
   } else {
      printf ("ERROR\n");
   }
   
   /* Release the transaction lock */
//   _lwsem_post (&lock);
} /* Endbody */
#endif
