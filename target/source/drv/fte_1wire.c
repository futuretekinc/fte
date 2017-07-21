#include "fte_target.h"

#if FTE_1WIRE_SUPPORTED

#define FTE_1WIRE_TIME_SLOT (1000000 / (FTE_1WIRE_BAUDRATE))
static 
FTE_RET     FTE_1WIRE_readBits(FTE_1WIRE_PTR p1Wire, FTE_UINT8_PTR data, FTE_UINT32 bit_len);

static 
FTE_RET     FTE_1WIRE_writeBits(FTE_1WIRE_PTR p1Wire, FTE_UINT8_PTR data, FTE_UINT32 bit_len);

static 
FTE_UINT32  FTE_1WIRE_search(FTE_1WIRE_PTR p1Wire, FTE_1WIRE_ROM_CODE_PTR pROMCodes, FTE_UINT32 max_count);

static 
void _FTE_1WIRE_autoRecoveryRun(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);

static 
FTE_1WIRE_PTR   _pHead      = NULL;

static 
FTE_UINT32      _n1Wires    = 0;

static 
FTE_1WIRE_FAMILY_NAME   _pFamilyNames[] = 
{
    {   .xFamilyCode = 0x28, .pName = "18B20" },
    {   .xFamilyCode = 0x0}
};
 
FTE_RET FTE_1WIRE_create
(
    FTE_1WIRE_CONFIG_CONST_PTR  pConfig
)
{
    ASSERT(pConfig != NULL);
    
    FTE_RET xRet;
    FTE_1WIRE_PTR   p1Wire;
    
    
    p1Wire = (FTE_1WIRE_PTR)FTE_MEM_allocZero(sizeof(FTE_1WIRE));
    if (p1Wire == NULL)
    {
        xRet = FTE_RET_NOT_ENOUGH_MEMORY;
        goto error;
    }

    p1Wire->pROMCodes = FTE_MEM_allocZero(sizeof(FTE_1WIRE_ROM_CODE) * pConfig->nMaxDevices);
    if (p1Wire->pROMCodes == NULL)
    {
        xRet = FTE_RET_NOT_ENOUGH_MEMORY;
        goto error;
    }
        
    p1Wire->nROMCodes = 0;    
    p1Wire->pConfig = pConfig;

    p1Wire->pNext = _pHead;
    xRet = FTE_SYS_LOCK_init(&p1Wire->xLock, 1);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }
        
    if (pConfig->xFlags & FTE_DEV_FLAG_SYSTEM_DEVICE)
    {
        xRet = FTE_1WIRE_attach(p1Wire, FTE_DEV_TYPE_ROOT);
        if (xRet != FTE_RET_OK)
        {
            goto error;
        }
    }
    
    _pHead  = p1Wire;
    _n1Wires++;
    
    return  FTE_RET_OK;
    
error:
    if (p1Wire != NULL)
    {
        FTE_SYS_LOCK_final(&p1Wire->xLock);
        if (p1Wire->pROMCodes != NULL)
        {
            FTE_MEM_free(p1Wire->pROMCodes);
        }
        
        FTE_MEM_free(p1Wire);
    }
    
    return  xRet;
}

FTE_RET FTE_1WIRE_attach
(
    FTE_1WIRE_PTR   p1Wire, 
    FTE_UINT32      nParent
)
{
    ASSERT(p1Wire != NULL);
    
    p1Wire->pLWGPIO = FTE_LWGPIO_get(p1Wire->pConfig->xGPIO);
    if (p1Wire->pLWGPIO == NULL)
    {
        return  FTE_RET_INVALID_OBJECT;
    }
    
    if (FTE_LWGPIO_attach(p1Wire->pLWGPIO, p1Wire->pConfig->nID) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    FTE_LWGPIO_setValue(p1Wire->pLWGPIO, FALSE);

    p1Wire->nParent = nParent;
    
    FTE_1WIRE_discovery(p1Wire);

    return  FTE_RET_OK;
}


FTE_RET FTE_1WIRE_detach
(
    FTE_1WIRE_PTR   p1Wire
)
{
    ASSERT(p1Wire != NULL);
   
    FTE_LWGPIO_detach(p1Wire->pLWGPIO);
    p1Wire->pLWGPIO = NULL;
    p1Wire->nParent = 0;
    
    return  FTE_RET_OK;

}

FTE_RET FTE_1WIRE_get
(   
    FTE_UINT32  nID,
    FTE_1WIRE_PTR _PTR_ pp1WIRE
)
{
    ASSERT(pp1WIRE != NULL);
    
    FTE_1WIRE_PTR   p1Wire;
    
    p1Wire = _pHead;
    while(p1Wire != NULL)
    {
        if (p1Wire->pConfig->nID == nID)
        {
            *pp1WIRE = p1Wire;
            
            return  FTE_RET_OK;
        }
        
        p1Wire = p1Wire->pNext;
    }
    
    return  FTE_RET_OBJECT_NOT_FOUND;
}

FTE_RET FTE_1WIRE_discovery
(
    FTE_1WIRE_PTR   p1Wire
)
{
    ASSERT (p1Wire != NULL);
    
    if (p1Wire->pConfig->nMaxDevices != 0)
    {
        memset(p1Wire->pROMCodes, 0, sizeof(FTE_1WIRE_ROM_CODE) * p1Wire->pConfig->nMaxDevices);
    }
               
    p1Wire->nROMCodes = FTE_1WIRE_search(p1Wire, p1Wire->pROMCodes, p1Wire->pConfig->nMaxDevices);
    
    return  FTE_RET_OK;
}

FTE_UINT32 FTE_1WIRE_count(void)
{
    return  _n1Wires;
}

FTE_VOID_PTR FTE_1WIRE_getFirst(void)
{
    return  _pHead;
}

FTE_VOID_PTR FTE_1WIRE_getNext
(
    FTE_VOID_PTR     p1Wire
)
{
    if (p1Wire != NULL)
    {
       return   ((FTE_1WIRE_PTR)p1Wire)->pNext; 
    }
    
    return  NULL;
}

FTE_RET FTE_1WIRE_DEV_count
(
    FTE_1WIRE_PTR       p1Wire,
    FTE_UINT32_PTR      pulCount
)
{
    ASSERT((p1Wire != NULL) && (pulCount != NULL));
    
    *pulCount = p1Wire->nROMCodes;
    
    return  FTE_RET_OK;
}

FTE_RET FTE_1WIRE_DEV_getROMCode
(
    FTE_1WIRE_PTR       p1Wire, 
    FTE_UINT32          nIdx, 
    FTE_1WIRE_ROM_CODE  pROMCode
)
{
    ASSERT(p1Wire != NULL);    
    
    memcpy(pROMCode, p1Wire->pROMCodes[nIdx], sizeof(FTE_1WIRE_ROM_CODE));
    
    return  FTE_RET_OK;
}

FTE_RET FTE_1WIRE_DEV_appendROMCode
(
    FTE_1WIRE_PTR       p1Wire, 
    FTE_1WIRE_ROM_CODE  pROMCode
)
{
    ASSERT(p1Wire != NULL);    
    
    if (p1Wire->nROMCodes >= p1Wire->pConfig->nMaxDevices)
    {
        return  FTE_RET_ERROR;
    }

    memcpy(p1Wire->pROMCodes[p1Wire->nROMCodes++], pROMCode, sizeof(FTE_1WIRE_ROM_CODE));
    
    return  FTE_RET_OK;
}

FTE_RET FTE_1WIRE_read
(
    FTE_1WIRE_PTR   p1Wire, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      nBuffLen
)
{
    FTE_UINT32 i, j;
    ASSERT(p1Wire != NULL);    
    
    memset(pBuff, 0, nBuffLen);
    for(i = 0 ; i < nBuffLen; i++)
    {
        for(j = 0 ; j < 8 ; j++)
        {
            FTE_BOOL bit = 0;
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            FTE_udelay(2);
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            FTE_udelay(5);
            FTE_LWGPIO_getValue(p1Wire->pLWGPIO, &bit);
            if (bit)
            {
                pBuff[i] |= 1 << j;
            }
            FTE_udelay(FTE_1WIRE_TIME_SLOT - 7);
            _int_enable();
        }
    }

    return FTE_RET_OK;
}

FTE_RET FTE_1WIRE_write
(
    FTE_1WIRE_PTR   p1Wire, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      len
)
{
    FTE_UINT32 i, j;
    
    ASSERT(p1Wire != NULL);    
    
    for(i = 0 ; i < len ; i++)
    {
        for(j = 0 ; j < 8 ; j++)
        {
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            FTE_udelay(5);
            if (((pBuff[i] >> j) & 0x01) == 0)
            {
                FTE_udelay(FTE_1WIRE_TIME_SLOT - 20);
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            }
            else
            {
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
                FTE_udelay(FTE_1WIRE_TIME_SLOT - 20);
            }
            _int_enable();
            FTE_udelay(15);
        }
    }
    
    _time_delay(1);
    
    return FTE_RET_OK;
}
#define GET_BIT_AT(value,n) ((((FTE_UINT8_PTR)value)[n / 8] >> (n % 8)) & 0x01)
#define SET_BIT_AT(value,n) (((FTE_UINT8_PTR)value)[n / 8] |= 1 << (n % 8))
#define CLR_BIT_AT(value,n) (((FTE_UINT8_PTR)value)[n / 8] &= ~(1 << (n % 8)))

FTE_UINT32  FTE_1WIRE_search
(
    FTE_1WIRE_PTR           p1Wire, 
    FTE_1WIRE_ROM_CODE_PTR  pROMCodes, 
    FTE_UINT32              nMaxCount
)
{
    FTE_INT32  i, j;
    FTE_INT32  nCount = 0;
    FTE_BOOL bFound;
    FTE_UINT8  data;
    FTE_UINT8  data2;
    FTE_UINT8  pROMCode[8];
    FTE_UINT8  pComPos[64];
    FTE_UINT8  nComPos = 0;
    
    FTE_1WIRE_lock(p1Wire);    
    
    memset(pROMCode, 0x00, sizeof(pROMCode));
    memset(pComPos, 0x00, sizeof(pComPos));

    while(1)
    {
        bFound = FALSE;
  
        FTE_1WIRE_reset(p1Wire);
        FTE_1WIRE_writeByte(p1Wire, 0xF0);
        
        for(i = 0 ; i < 64 ; i++)
        {
            FTE_1WIRE_readBits(p1Wire, &data, 2);

            switch(data)
            {
            case    0:
                {
                    if (GET_BIT_AT(pROMCode, i) == 0)
                    {
                        if ((nComPos == 0) || (pComPos[nComPos-1] < i))
                        {
                            pComPos[nComPos++] = i;
                        }
                        else if (pComPos[nComPos-1] == i)
                        {
                            SET_BIT_AT(pROMCode, i);
                            nComPos--;

                            for(j = i+1 ; j < ((i / 8) + 1) * 8 ; j++)
                            {
                                CLR_BIT_AT(pROMCode, j);
                            }
                            
                            for(j = (i / 8) + 1; j < 8 ; j++)
                            {
                                pROMCode[j] = 0;
                            }
                        }
                        bFound = TRUE;
                    }
                }
                break;
                
            case    1:
                {
                    if (GET_BIT_AT(pROMCode, i) == 0)
                    {
                        SET_BIT_AT(pROMCode, i);
                        bFound = TRUE;
                    }
                }
                break;
                
            case    2:
                {
                    if (GET_BIT_AT(pROMCode, i) != 0)
                    {
                        CLR_BIT_AT(pROMCode, i);
                        bFound = TRUE;
                    }
                }
                break;
                
            default:
                goto finished;
            }
            
            data = GET_BIT_AT(pROMCode, i);
            FTE_1WIRE_writeBits(p1Wire, &data, 1);      
        }

        data = 0xFF;
        data2 = 0x00;
        for(i = 0 ; i < 8 ; i++)
        {
            data &= pROMCode[i];
            data2 |= pROMCode[i];
        }

        if (bFound && (data != 0xFF) && (data2 != 0x00))
        {
            if (nCount < nMaxCount)
            {
                memcpy(pROMCodes[nCount++], pROMCode, sizeof(FTE_1WIRE_ROM_CODE));
            }
        }
        else
        {
            break;
        }
        
        _time_delay(10);
    }        
    
finished:
    FTE_1WIRE_unlock(p1Wire);

    return  nCount;
}

void    FTE_1WIRE_printInfo(void)
{
    FTE_1WIRE_PTR   p1Wire;
    
    printf("%8s %8s %8s %8s\n", "OID", "GPIO", "MAX", "ATTACHED");
    
    p1Wire = _pHead;
    while(p1Wire != NULL)
    {
        if (p1Wire->pConfig != NULL)
        {
            printf("%08x %08x %8d %8d\n", 
                   p1Wire->pConfig->nID, 
                   p1Wire->pConfig->xGPIO, 
                   p1Wire->pConfig->nMaxDevices, 
                   p1Wire->nROMCodes);
            
            FTE_UINT32  ulDevCount = 0;
            FTE_1WIRE_DEV_count(p1Wire, &ulDevCount);
            for(FTE_INT32 i = 0 ; i < ulDevCount; i++)
            {
                FTE_1WIRE_ROM_CODE pROMCode;
                
                FTE_1WIRE_DEV_getROMCode(p1Wire, i, pROMCode);
                printf("%8d : %02x %02x %02x %02x %02x %02x %02x %02x \n",
                       i, 
                       pROMCode[0], pROMCode[1], pROMCode[2], pROMCode[3],
                       pROMCode[4], pROMCode[5], pROMCode[6], pROMCode[7]);
            }
            
        }
        
        p1Wire = p1Wire->pNext;
        
        
    }                       
}

void    FTE_1WIRE_printDevices
(
    FTE_1WIRE_PTR   p1Wire
)
{
    ASSERT(p1Wire != NULL);
    FTE_CHAR    pFamilyName[32];
    
    memset(pFamilyName, 0, sizeof(pFamilyName));
    
    printf("     ID : %08x\n", p1Wire->pConfig->nID);
    printf("DEVICES : %d\n", p1Wire->nROMCodes);
    printf("%-6s %-16s %-24s\n", "INDEX", "DEVICE", "ROM_CODE");                
    for(int id = 0 ; id < p1Wire->nROMCodes ; id++)
    {
        FTE_1WIRE_getFamilyName(p1Wire->pROMCodes[id][0], pFamilyName, sizeof(pFamilyName) - 1);
        printf("%4d : %-16s %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
               id, 
               pFamilyName,
               p1Wire->pROMCodes[id][0], p1Wire->pROMCodes[id][1], 
               p1Wire->pROMCodes[id][2], p1Wire->pROMCodes[id][3],
               p1Wire->pROMCodes[id][4], p1Wire->pROMCodes[id][5], 
               p1Wire->pROMCodes[id][6], p1Wire->pROMCodes[id][7]);
    }
}

FTE_INT32  FTE_1WIRE_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   xRet = SHELL_EXIT_SUCCESS;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (bPrintUsage)
    { 
        goto    error;
    }
    
    switch(nArgc)
    {
    case    1:
        {
            FTE_1WIRE_printInfo();
        }
        break;
        
    case    2:
        {
            if (strncmp(pArgv[1], "discovery", strlen(pArgv[1])) == 0)
            {
                FTE_1WIRE_PTR   p1Wire = _pHead;
                
                while(p1Wire != NULL)
                {
                    FTE_1WIRE_discovery(p1Wire);
                
                    printf("CHANNEL :  %08x\n", p1Wire->pConfig->nID);
                    for(int j = 0 ; j < p1Wire->nROMCodes ; j++)
                    {
                        printf("%d : %02x %02x %02x %02x %02x %02x %02x %02x \n",
                               j, 
                               p1Wire->pROMCodes[j][0], 
                               p1Wire->pROMCodes[j][1], 
                               p1Wire->pROMCodes[j][2], 
                               p1Wire->pROMCodes[j][3],
                               p1Wire->pROMCodes[j][4], 
                               p1Wire->pROMCodes[j][5], 
                               p1Wire->pROMCodes[j][6], 
                               p1Wire->pROMCodes[j][7]);
                    }
                    
                    p1Wire = p1Wire->pNext;
                }
            }
            else if (strcmp(pArgv[1], "all") == 0)
            {
                FTE_1WIRE_PTR   p1Wire = _pHead;
                
                while(p1Wire != NULL)
                {
                    FTE_1WIRE_printDevices(p1Wire);                    
                }
            }
            else
            {
                FTE_UINT32 nID;
                
                if (FTE_strToHex( pArgv[1], &nID) != FTE_RET_OK)  
                {
                    xRet = SHELL_EXIT_ERROR;
                    goto error;
                }
                
                FTE_1WIRE_PTR  p1Wire;
               xRet = FTE_1WIRE_get(nID, &p1Wire);
                if (xRet != FTE_RET_OK)
                {
                    printf("1-Wire xDevID%d not exists!\n", nID);
                    goto error;
                }

                FTE_1WIRE_printDevices(p1Wire);
            }
        }
        break;
    
    case    3:
        {
            if (strncmp(pArgv[1], "discovery", strlen(pArgv[1])) == 0)
            {
                FTE_1WIRE_PTR   p1Wire;
                FTE_UINT32 nID;
                
                if (FTE_strToHex( pArgv[2], &nID) != FTE_RET_OK)  
                {
                    xRet = SHELL_EXIT_ERROR;
                    goto error;
                }
            
                xRet = FTE_1WIRE_get(nID, &p1Wire);
                if( xRet != FTE_RET_OK)
                {
                    goto error;
                }
                
                FTE_1WIRE_discovery(p1Wire);                
            
                FTE_UINT32  ulDevCount = 0;
                FTE_1WIRE_DEV_count(p1Wire, &ulDevCount);
                
                for(FTE_INT32 i = 0 ; i < ulDevCount ; i++)
                {
                    FTE_1WIRE_ROM_CODE pROMCode;
                    
                    FTE_1WIRE_DEV_getROMCode(p1Wire, i, pROMCode);
                    printf("%d-%d : %02x %02x %02x %02x %02x %02x %02x %02x \n",
                           nID, i, 
                           pROMCode[0], pROMCode[1], pROMCode[2], pROMCode[3],
                           pROMCode[4], pROMCode[5], pROMCode[6], pROMCode[7]);
                }
            }
        }
        break;
        
    case    4:
        {
            if (strcmp(pArgv[1], "add") == 0)
            {
                FTE_1WIRE_PTR   p1Wire;
                FTE_UINT32 nID;
                
                FTE_UINT8  pROMCode[8];
                
                if (FTE_strToHex( pArgv[2], &nID) != FTE_RET_OK)  
                {
                    xRet = SHELL_EXIT_ERROR;
                    goto error;
                }
            
                xRet = FTE_1WIRE_get(nID, &p1Wire);
                if( xRet != FTE_RET_OK)
                {
                    xRet = SHELL_EXIT_ERROR;
                    goto error;
                }
                
                
                if (strlen(pArgv[3]) != 16)
                {
                    printf("Invalid ROM Code\n");
                    goto error;
                }
                
                for(int i = 0 ; i < 8 ; i++)
                {
                    FTE_UINT8  nHI, nLO;
                    
                    if ('0' <= pArgv[3][i*2] && pArgv[3][i*2] <= '9')
                    {
                        nHI = pArgv[3][i*2] - '0';
                    }
                    else if ('A' <= pArgv[3][i*2] && pArgv[3][i*2] <= 'F')
                    {
                        nHI = pArgv[3][i*2] - 'A' + 10;
                    }
                    else if ('a' <= pArgv[3][i*2] && pArgv[3][i*2] <= 'f')
                    {
                        nHI = pArgv[3][i*2] - 'a' + 10;
                    }
                    else
                    {
                        printf("Invalid ROM Code\n");
                        goto error;
                    }
                    
                    if ('0' <= pArgv[3][i*2+1] && pArgv[3][i*2+1] <= '9')
                    {
                        nLO = pArgv[3][i*2+1] - '0';
                    }
                    else if ('A' <= pArgv[3][i*2+1] && pArgv[3][i*2+1] <= 'F')
                    {
                        nLO = pArgv[3][i*2+1] - 'A' + 10;
                    }
                    else if ('a' <= pArgv[3][i*2+1] && pArgv[3][i*2+1] <= 'f')
                    {
                        nLO = pArgv[3][i*2+1] - 'a' + 10;
                    }
                    else
                    {
                        printf("Invalid ROM Code\n");
                        goto error;
                    }
                    
                    pROMCode[i] = (nHI << 4) | nLO;
                }
                FTE_1WIRE_DEV_appendROMCode(p1Wire, pROMCode);               
                
            }
        }
        break;
        
    default:
        bPrintUsage = TRUE;    
    }

error:    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [ discovery | info ] [<xChannel>] \n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [ discovery | info ] [<xChannel>] \n", pArgv[0]);
            printf("        xDevID - 1WIRE Channel \n");
            printf("        discovery - discover connected devices\n");
            printf("        info - channel informations\n");
        }
    }
    

    return   xRet;
}

FTE_RET FTE_1WIRE_reset(FTE_1WIRE_PTR p1Wire)
{
    ASSERT(p1Wire != NULL);
    
    FTE_LWGPIO_setValue(p1Wire->pLWGPIO, FALSE);
    FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
    FTE_udelay(1000);
    FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
    _time_delay(1);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_1WIRE_writeByte(FTE_1WIRE_PTR p1Wire, FTE_UINT8 data)
{
    ASSERT(p1Wire != NULL);
    
    return  FTE_1WIRE_writeBits(p1Wire, &data, 8);
}

FTE_RET FTE_1WIRE_writeBits(FTE_1WIRE_PTR p1Wire, FTE_UINT8_PTR data, FTE_UINT32 bit_len)
{
    int             i;

    ASSERT(p1Wire != NULL);
    
    for(i = 0 ; i < bit_len / 8; i++)
    {
        for(int j = 0 ; j < 8 ; j++)
        {
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            FTE_udelay(2);
            if (((data[i] >> j) & 0x01) == 0)
            {
                FTE_udelay(100);
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            }
            else
            {
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
                FTE_udelay(100);
            }
            _int_enable();
        FTE_udelay(FTE_1WIRE_TIME_SLOT - 100);
        }
    }
    
    for(int j = 0 ; j < bit_len % 8 ; j++)
    {
       _int_disable();
       FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
        FTE_udelay(2);
        if (((data[i] >> j) & 0x01) == 0)
        {
                FTE_udelay(100);
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
        }
        else
        {
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            FTE_udelay(100);
        }
        _int_disable();
        FTE_udelay(FTE_1WIRE_TIME_SLOT - 100);
    }
    return FTE_RET_OK;
}

FTE_RET FTE_1WIRE_readBits
(
    FTE_1WIRE_PTR   p1Wire, 
    FTE_UINT8_PTR   data, 
    FTE_UINT32      bit_len
)
{
    int     i;
    FTE_UINT8  nBuff;
 
    ASSERT(p1Wire != NULL);
    
    for(i = 0 ; i < bit_len / 8; i++)
    {
        nBuff = 0;
        for(int j = 0 ; j < 8 ; j++)
        {
            FTE_BOOL bit = 0;
            
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            FTE_udelay(2);
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            FTE_udelay(5);
            FTE_LWGPIO_getValue(p1Wire->pLWGPIO, &bit);
            if (bit)
            {
                nBuff |= 1 << j;
            }
            _int_enable();
            FTE_udelay(FTE_1WIRE_TIME_SLOT - 7);
        }
        
        data[i] = nBuff;
    }
    
    nBuff = 0;
    for(int j = 0 ; j < bit_len % 8 ; j++)
    {
        FTE_BOOL bit = 0;
        
        _int_disable();
        FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
        FTE_udelay(2);
        FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
        FTE_udelay(5);
        FTE_LWGPIO_getValue(p1Wire->pLWGPIO, &bit);
        if (bit)
        {
            nBuff |= 1 << j;
        }
        _int_enable();
        FTE_udelay(FTE_1WIRE_TIME_SLOT - 7);
    }
    data[i] = nBuff;
 
    
    
    return FTE_RET_ERROR;
}

FTE_RET FTE_1WIRE_getFamilyName
(
    FTE_UINT32      ulCode,
    FTE_CHAR_PTR    pBuff,
    FTE_UINT32      ulLen
)
{
    ASSERT(pBuff != NULL);
    
    FTE_1WIRE_FAMILY_NAME_PTR pFamilyName = _pFamilyNames;
    
    while(pFamilyName->xFamilyCode)
    {
        if (ulCode == pFamilyName->xFamilyCode)
        {
            strncpy(pBuff, pFamilyName->pName, ulLen);
            return  FTE_RET_OK;
        }
        
        pFamilyName++;
    }
    
    snprintf(pBuff, ulLen, "unknown(%02x)", ulCode);
    
    return  FTE_RET_OBJECT_NOT_FOUND;
}

static 
FTE_UINT32          _n1WireRecoveryTimerID = 0;

FTE_RET FTE_1WIRE_autoRecoveryStart(void)
{
    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;

    _time_init_ticks(&xDTicks, 0);
    _time_add_sec_to_ticks(&xDTicks, 300);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    _n1WireRecoveryTimerID = _timer_start_periodic_at_ticks(_FTE_1WIRE_autoRecoveryRun, NULL, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);
    
    return  FTE_RET_OK;
}

void _FTE_1WIRE_autoRecoveryRun
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR     pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    if (_n1WireRecoveryTimerID != xTimerID)
    {
        return;
    }
    
    FTE_1WIRE_discovery(0);    
}

FTE_RET FTE_1WIRE_lock
(
    FTE_1WIRE_PTR   p1Wire
)
{
    ASSERT(p1Wire != NULL);
    
    if (FTE_SYS_LOCK_enable(&p1Wire->xLock) != FTE_RET_OK)
    {  
        DEBUG("\n_xLock_wait failed");
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

FTE_RET FTE_1WIRE_unlock
(
    FTE_1WIRE_PTR   p1Wire
)
{   
    ASSERT(p1Wire != NULL);

    if (FTE_SYS_LOCK_disable(&p1Wire->xLock) != FTE_RET_OK)
    {
        DEBUG("\n_xLock_post failed");
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}

#endif