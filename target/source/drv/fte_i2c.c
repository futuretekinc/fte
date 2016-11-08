#include "fte_target.h"
#include "FTE_I2C.h"

#if FTE_I2C_SUPPORTED

#define MAX_I2C_COUNT   2
//#define DEBUG_I2C       1
static 
FTE_RET   _FTE_I2C_lock(FTE_I2C_PTR pI2C);
static 
FTE_RET   _FTE_I2C_unlock(FTE_I2C_PTR pI2C);

static 
void _FTE_I2C_writeCommand(MQX_FILE_PTR fd, FTE_UINT8 uiCmd);

static 
void _FTE_I2C_write(MQX_FILE_PTR fd, FTE_UINT32 addr, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);      

static 
void _FTE_I2C_read(MQX_FILE_PTR fd, FTE_UINT32 addr, FTE_UINT8_PTR pBuff, FTE_UINT32 ulLen);

static const   FTE_CHAR_PTR channels[MAX_I2C_COUNT] = 
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
static FTE_UINT32      _nI2C   = 0;


FTE_RET   FTE_I2C_create
(
    FTE_I2C_CONFIG_PTR  pConfig
)
{
    FTE_I2C_PTR pI2C;
    
    pI2C = (FTE_I2C_PTR)FTE_MEM_allocZero(sizeof(FTE_I2C));
    if (pI2C == NULL)
    {
        return  FTE_RET_NOT_ENOUGH_MEMORY;
    }
    
    _pChannels[pConfig->xPort].xFlags       = pConfig->xFlags;

    pI2C->pConfig           = pConfig;
    pI2C->pChannel          = &_pChannels[pConfig->xPort];
//    pI2C->callback.CALLBACK = _FTE_I2C_set_cs;
//    pI2C->callback.USERDATA = (pointer)pI2C;    
    pI2C->pNext             = _pHead;
    
    _pHead = pI2C;
    _nI2C++;
        
    return  FTE_RET_OK;
}

FTE_RET   FTE_I2C_attach
(
    FTE_I2C_PTR     pI2C, 
    FTE_UINT32      nParent
)
{
    ASSERT(pI2C != NULL);
        
    if (pI2C->pChannel->xFD == NULL)
    {
        /* Open the I2C driver */
        pI2C->pChannel->xFD = fopen (channels[pI2C->pConfig->xPort], NULL);
        if (pI2C->pChannel->xFD == NULL)
        {
            goto error;
        }
        
        if (FTE_SYS_LOCK_init(&pI2C->pChannel->xLWSEM, 1) != FTE_RET_OK)
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
    
    return  FTE_RET_OK;
    
error:
    
    if (pI2C->pChannel->xFD != NULL)
    {
        fclose(pI2C->pChannel->xFD);
        pI2C->pChannel->xFD = NULL;
    }
    
   
    return  FTE_RET_ERROR;
}

FTE_RET   FTE_I2C_detach
(
    FTE_I2C_PTR     pI2C
)
{
    ASSERT(pI2C != NULL);
            
    if (pI2C->pChannel->nCount == 1)
    {
        /* Open the I2C driver */
        fclose(pI2C->pChannel->xFD);        
        pI2C->pChannel->xFD = NULL;
        FTE_SYS_LOCK_final(&pI2C->pChannel->xLWSEM);        
        pI2C->pChannel->nCount = 0;
    }  
    else
    {
        pI2C->pChannel->nCount--;
    }

    pI2C->nParent = 0;
    
    return  FTE_RET_OK;
}

FTE_UINT32     FTE_I2C_count(void)
{
    return  _nI2C;
}

FTE_I2C_PTR FTE_I2C_get_first(void)
{
    return  _pHead;
}

FTE_I2C_PTR FTE_I2C_get_next
(
    FTE_I2C_PTR     pI2C
)
{
    if (pI2C == NULL)
    {
        return  NULL;
    }
    
    return  pI2C->pNext;
}

FTE_I2C_PTR FTE_I2C_get
(
    FTE_UINT32  nID
)
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

FTE_UINT32     FTE_I2C_parent_get
(
    FTE_I2C_PTR     pI2C
)
{
    ASSERT(pI2C != NULL);
    
    return  pI2C->nParent;
}


FTE_RET   FTE_I2C_read
(
    FTE_I2C_PTR     pI2C, 
    FTE_UINT8       nID, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulLen
)
{
    ASSERT(pI2C != NULL);
    
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
    
    return FTE_RET_OK;
    
error:  
    _FTE_I2C_unlock(pI2C);
    
    return FTE_RET_ERROR;
}

FTE_RET   FTE_I2C_write
(
    FTE_I2C_PTR     pI2C, 
    FTE_UINT8       nID, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulLen
)
{
    ASSERT(pI2C != NULL);
    
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
    
    return FTE_RET_OK;
    
error:  
    _FTE_I2C_unlock(pI2C);
    
    return FTE_RET_ERROR;
}

FTE_RET   FTE_I2C_write_byte
(
    FTE_I2C_PTR     pI2C, 
    FTE_UINT8       nID, 
    FTE_UINT8       uiData
)
{
    ASSERT(pI2C != NULL);
    
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
    
    return FTE_RET_OK;
    
error:  
    _FTE_I2C_unlock(pI2C);
    
    return FTE_RET_ERROR;
}

FTE_RET   FTE_I2C_set_baudrate
(
    FTE_I2C_PTR pI2C, 
    FTE_UINT32  ulBaudrate
)
{
    return  FTE_RET_OK;
}

FTE_RET   FTE_I2C_get_baudrate
(
    FTE_I2C_PTR     pI2C, 
    FTE_UINT32_PTR  pulBaudrate
)
{
    ASSERT(pI2C != NULL);
    
    *pulBaudrate = pI2C->pConfig->nBaudrate;

    return  FTE_RET_OK;
}

FTE_RET   FTE_I2C_set_flags
(
    FTE_I2C_PTR pI2C, 
    FTE_UINT32  xFlags
)
{
    return  FTE_RET_OK;
}

FTE_RET   FTE_I2C_get_flags
(
    FTE_I2C_PTR     pI2C, 
    FTE_UINT32_PTR  pxFlags
)
{
    ASSERT(pI2C != NULL);
            
    *pxFlags = pI2C->pConfig->xFlags;

    return  FTE_RET_OK;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
FTE_RET   _FTE_I2C_lock
(
    FTE_I2C_PTR     pI2C
)
{
    ASSERT(pI2C != NULL);
    
    if (FTE_SYS_LOCK_enable(&pI2C->pChannel->xLWSEM) != FTE_RET_OK)
    {  
        DEBUG("\nFTE_SYS_LOCK_enable failed");
        goto error;
    }
    
    return  FTE_RET_OK;
error:
    
    return  FTE_RET_ERROR;
}

FTE_RET   _FTE_I2C_unlock
(
    FTE_I2C_PTR     pI2C
)
{   
    ASSERT(pI2C != NULL);
    
//    fflush(pI2C->pChannel->xFD);
    
    if (FTE_SYS_LOCK_disable(&pI2C->pChannel->xLWSEM) != FTE_RET_OK)
    {
        DEBUG("\nFTE_SYS_LOCK_disable failed");
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
} 

#define I2C_DEVICE_INTERRUPT "ii2c0:"
#define I2C_EEPROM_BUS_ADDRESS  0x3c

FTE_INT32  FTE_I2C_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;

    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
            }
            break;
            
        default:
            {
#if 0
                if (strcmp(pArgv[2], "read") == 0)
                {
                    FTE_I2C_PTR pI2C;
                    FTE_UINT32 id, nCmdLen, nRecvLen = 1, i;
                    FTE_UINT8  pSendBuff[64];
                    FTE_UINT8  pRecvBuff[64];
                    
                    if (nArgc < 4)
                    {
                       xRet = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (! Shell_parse_number( pArgv[1], &id))  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    pI2C = FTE_I2C_get(id);
                    if (pI2C == NULL)
                    {
                        goto error;
                    }
                    
                    if (! Shell_parse_number( pArgv[3], &nCmdLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (nArgc < 4 + nCmdLen + 1)
                    {
                       xRet = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    for(i = 0 ; i < nCmdLen ; i++)
                    {
                        FTE_UINT32 nValue;
                        Shell_parse_hexnum( pArgv[4 + i], &nValue);
                        pSendBuff[i] = nValue;
                    }
                    
                    if (! Shell_parse_number( pArgv[4 + nCmdLen], &nRecvLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    if (FTE_RET_OK != FTE_I2C_read(pI2C, (FTE_UINT8 *)&pSendBuff, nCmdLen, pRecvBuff, nRecvLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    for (i = 0 ; i < nRecvLen ; i++)
                    {
                        printf("%02x ", pRecvBuff[i]);
                    }
                    printf("\n");
                }
                else if (strcmp(pArgv[2], "write") == 0)
                {
                    FTE_I2C_PTR pI2C;
                    FTE_UINT32 nID, nSendLen, i;
                    FTE_UINT8 pSendBuff[64];
                     
                    if (nArgc < 6)
                    {
                       xRet = SHELL_EXIT_ERROR;
                        goto error;
                    }
                    
                    if (! Shell_parse_number( pArgv[1], &nID))  
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }

                    pI2C = FTE_I2C_get(nID);
                    
                    if (! Shell_parse_number( pArgv[3], &nSendLen))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                    
                    for(i = 0 ; i < nSendLen ; i++)
                    {
                        FTE_UINT32 nValue;
                        if (! Shell_parse_hexnum( pArgv[4+i], &nValue))
                        {
                           xRet = SHELL_EXIT_ERROR;
                           goto error;
                        }
                        pSendBuff[i] = nValue;
                    }
                    
                    if (FTE_RET_OK != FTE_I2C_write(pI2C, pSendBuff, nSendLen, NULL, 0))
                    {
                       xRet = SHELL_EXIT_ERROR;
                       goto error;
                    }
                }
                else
                {
                    bPrintUsage = TRUE;
                }
#endif
            }
        }
    }

    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<id>] [ baudrate | flags | read | write ] [<len>] [<data>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<id>] [ baudrate | flags | read | write ] [<len>] [<data>]\n", pArgv[0]);
            printf("        id       - I2C Channel \n");
            printf("        baudrate - I2C speed \n");
        }
    }

    return   xRet;
}


void FTE_I2C_write_command
(
    MQX_FILE_PTR    fd, 
    FTE_UINT8       uiCmd
)
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
      FTE_UINT32       addr,

      /* [IN] The array of characters are to be written in EEPROM */
      FTE_UINT8_PTR    pBuff,

      /* [IN] Number of bytes in that buffer */
      FTE_UINT32       n      
   )
{ /* Body */

   FTE_UINT32       param;
   _mqx_int    length;
   _mqx_int    result;
   FTE_UINT8        mem[128];

   /* Protect I2C transaction in multitask environment */
//   FTE_SYS_LOCK_enable (&lock);

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
      mem[0] = (FTE_UINT8)(addr >> 8);
      mem[1] = (FTE_UINT8)addr;
      printf ("  Write to address 0x%02x%02x ... ", mem[0], mem[1]);
      do 
      {
         result = fwrite (mem, 1, 2, fd);
      } while (result < 2);
      printf ("OK\n");
#else
      mem[0] = (FTE_UINT8)addr;
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
      if (FTE_RET_OK == result)
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
//   FTE_SYS_LOCK_disable (&lock);
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
      FTE_UINT32    addr,

      /* [IN] The array of characters to be written into */
      uFTE_CHAR_PTR  buffer,

      /* [IN] Number of bytes to read */
      _mqx_int   n      
   )
{ /* Body */
   _mqx_int    param;
   _mqx_int    result;
   FTE_UINT8        mem[128];

   printf ("Reading %d bytes from address 0x%08x ...\n", n, addr);
   if (0 == n) 
   {
      printf ("  Nothing to do.\n");
      return;
   }

   /* Protect I2C transaction in multitask environment */
//   FTE_SYS_LOCK_enable (&lock);
   
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
//         FTE_SYS_LOCK_disable (&lock);
         
         return;
      }
   } else {
      printf ("ERROR\n");
   }

   /* Write address within memory block */
#if I2C_EEPROM_MEMORY_WIDTH == 2
      mem[0] = (FTE_UINT8)(addr >> 8);
      mem[1] = (FTE_UINT8)addr;
      printf ("  Write to address 0x%02x%02x ... ", mem[0], mem[1]);
      do 
      {
         result = fwrite (mem, 1, 2, fd);
      } while (result < 2);
      printf ("OK\n");
#else
      mem[0] = (FTE_UINT8)addr;
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
   if (FTE_RET_OK == result)
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
//   FTE_SYS_LOCK_disable (&lock);
} /* Endbody */
#endif
