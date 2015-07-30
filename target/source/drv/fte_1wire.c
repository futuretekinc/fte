#include "fte_target.h"

#if FTE_1WIRE_SUPPORTED

#define FTE_1WIRE_TIME_SLOT (1000000 / (FTE_1WIRE_BAUDRATE))
_mqx_uint               FTE_1WIRE_readBits(FTE_1WIRE_PTR p1Wire, uint_8_ptr data, uint_32 bit_len);
_mqx_uint               FTE_1WIRE_writeBits(FTE_1WIRE_PTR p1Wire, uint_8_ptr data, uint_32 bit_len);
_mqx_uint               FTE_1WIRE_search(FTE_1WIRE_PTR p1Wire, FTE_1WIRE_ROM_CODE_PTR pROMCodes, uint_32 max_count);
static void             _FTE_1WIRE_autoRecoveryRun(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);

static FTE_1WIRE_PTR    _pHead      = NULL;
static uint_32          _n1Wires    = 0;

static FTE_1WIRE_FAMILY_NAME   _pFamilyNames[] = 
{
    {   .xFamilyCode = 0x28, .pName = "18B20" },
    {   .xFamilyCode = 0x0}
};
 
_mqx_uint   FTE_1WIRE_create(FTE_1WIRE_CONFIG_CONST_PTR pConfig)
{
    FTE_1WIRE_PTR   p1Wire;
    
    p1Wire = (FTE_1WIRE_PTR)FTE_MEM_allocZero(sizeof(FTE_1WIRE));
    if (p1Wire == NULL)
    {
        return  MQX_OUT_OF_MEMORY;
    }

    p1Wire->pNext = _pHead;
    if (_lwsem_create(&p1Wire->xLWSEM, 1) != MQX_OK)
    {
        goto error;
    }
        
    p1Wire->pROMCodes = FTE_MEM_allocZero(sizeof(FTE_1WIRE_ROM_CODE) * pConfig->nMaxDevices);
    if (p1Wire->pROMCodes == NULL)
    {
        goto error;
    }
        
    p1Wire->nROMCodes = 0;    
    p1Wire->pConfig = pConfig;
    
    if (pConfig->xFlags & FTE_DEV_FLAG_SYSTEM_DEVICE)
    {
        if (FTE_1WIRE_attach(p1Wire, FTE_DEV_TYPE_ROOT) != MQX_OK)
        {
            goto error;
        }
    }
    
    _pHead  = p1Wire;
    _n1Wires++;
    
    return  MQX_OK;
    
error:
    _lwsem_destroy(&p1Wire->xLWSEM);
    if (p1Wire->pROMCodes != NULL)
    {
        FTE_MEM_free(p1Wire->pROMCodes);
    }
        
    FTE_MEM_free(p1Wire);
    
    return  MQX_ERROR;
}

_mqx_uint   FTE_1WIRE_attach(FTE_1WIRE_PTR p1Wire, uint_32 nParent)
{
    assert(p1Wire != NULL);
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    p1Wire->pLWGPIO = FTE_LWGPIO_get(p1Wire->pConfig->xGPIO);
    if (p1Wire->pLWGPIO == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    if (FTE_LWGPIO_attach(p1Wire->pLWGPIO, p1Wire->pConfig->nID) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    FTE_LWGPIO_setValue(p1Wire->pLWGPIO, FALSE);

    p1Wire->nParent = nParent;
    
    FTE_1WIRE_discovery(p1Wire);

    return  MQX_OK;
}


_mqx_uint   FTE_1WIRE_detach(FTE_1WIRE_PTR p1Wire)
{
    assert(p1Wire != NULL);
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
   
    FTE_LWGPIO_detach(p1Wire->pLWGPIO);
    p1Wire->pLWGPIO = NULL;
    p1Wire->nParent = 0;
    
    return  MQX_OK;

}

FTE_1WIRE_PTR    FTE_1WIRE_get(uint_32 nID)
{
    FTE_1WIRE_PTR   p1Wire;
    
    p1Wire = _pHead;
    while(p1Wire != NULL)
    {
        if (p1Wire->pConfig->nID == nID)
        {
            return  p1Wire;
        }
        
        p1Wire = p1Wire->pNext;
    }
    
    return  NULL;
}

_mqx_uint   FTE_1WIRE_discovery(FTE_1WIRE_PTR p1Wire)
{
    assert (p1Wire != NULL);
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    if (p1Wire->pConfig->nMaxDevices != 0)
    {
        memset(p1Wire->pROMCodes, 0, sizeof(FTE_1WIRE_ROM_CODE) * p1Wire->pConfig->nMaxDevices);
    }
               
    p1Wire->nROMCodes = FTE_1WIRE_search(p1Wire, p1Wire->pROMCodes, p1Wire->pConfig->nMaxDevices);
    
    return  MQX_OK;
}

uint_32 FTE_1WIRE_count(void)
{
    return  _n1Wires;
}

pointer FTE_1WIRE_getFirst(void)
{
    return  _pHead;
}

pointer FTE_1WIRE_getNext(pointer p1Wire)
{
    if (p1Wire != NULL)
    {
       return   ((FTE_1WIRE_PTR)p1Wire)->pNext; 
    }
    
    return  NULL;
}

uint_32     FTE_1WIRE_DEV_count(FTE_1WIRE_PTR p1Wire)
{
    if (p1Wire == NULL)
    {
        return  0;
    }
    
    return  p1Wire->nROMCodes;
}

_mqx_uint   FTE_1WIRE_DEV_getROMCode(FTE_1WIRE_PTR p1Wire, uint_32 nIdx, FTE_1WIRE_ROM_CODE pROMCode)
{
    assert(p1Wire != NULL);    
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    memcpy(pROMCode, p1Wire->pROMCodes[nIdx], sizeof(FTE_1WIRE_ROM_CODE));
    
    return  MQX_OK;
}

_mqx_uint   FTE_1WIRE_DEV_appendROMCode(FTE_1WIRE_PTR p1Wire, FTE_1WIRE_ROM_CODE pROMCode)
{
    assert(p1Wire != NULL);    
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    if (p1Wire->nROMCodes >= p1Wire->pConfig->nMaxDevices)
    {
        return  MQX_ERROR;
    }

    memcpy(p1Wire->pROMCodes[p1Wire->nROMCodes++], pROMCode, sizeof(FTE_1WIRE_ROM_CODE));
    
    return  MQX_OK;
}

_mqx_uint   FTE_1WIRE_read(FTE_1WIRE_PTR p1Wire, uint_8_ptr buff, uint_32 buff_len)
{
    uint_32 i, j;
    assert(p1Wire != NULL);    
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    memset(buff, 0, buff_len);
    for(i = 0 ; i < buff_len; i++)
    {
        for(j = 0 ; j < 8 ; j++)
        {
            boolean bit = 0;
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            fte_udelay(2);
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            fte_udelay(5);
            FTE_LWGPIO_getValue(p1Wire->pLWGPIO, &bit);
            if (bit)
            {
                buff[i] |= 1 << j;
            }
            fte_udelay(FTE_1WIRE_TIME_SLOT - 7);
            _int_enable();
        }
    }

    return MQX_OK;
}

_mqx_uint   FTE_1WIRE_write(FTE_1WIRE_PTR p1Wire, uint_8_ptr buff, uint_32 len)
{
    uint_32 i, j;
    assert(p1Wire != NULL);    
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    for(i = 0 ; i < len ; i++)
    {
        for(j = 0 ; j < 8 ; j++)
        {
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            fte_udelay(5);
            if (((buff[i] >> j) & 0x01) == 0)
            {
                fte_udelay(FTE_1WIRE_TIME_SLOT - 20);
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            }
            else
            {
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
                fte_udelay(FTE_1WIRE_TIME_SLOT - 20);
            }
            _int_enable();
            fte_udelay(15);
        }
    }
    
    _time_delay(1);
    
    return MQX_OK;
}
#define GET_BIT_AT(value,n) ((((uint_8_ptr)value)[n / 8] >> (n % 8)) & 0x01)
#define SET_BIT_AT(value,n) (((uint_8_ptr)value)[n / 8] |= 1 << (n % 8))
#define CLR_BIT_AT(value,n) (((uint_8_ptr)value)[n / 8] &= ~(1 << (n % 8)))

_mqx_uint   FTE_1WIRE_search(FTE_1WIRE_PTR p1Wire, FTE_1WIRE_ROM_CODE_PTR pROMCodes, uint_32 nMaxCount)
{
    int_32  i, j;
    int_32  nCount = 0;
    boolean bFound;
    uint_8  data;
    uint_8  pROMCode[8];
    uint_8  pComPos[64];
    uint_8  nComPos = 0;
    
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
        for(i = 0 ; i < 8 ; i++)
        {
            data &= pROMCode[i];
        }
        
        if (bFound && (data != 0xFF))
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

void    _fte_1wire_print_info(void)
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
            
            for(int i = 0 ; i < FTE_1WIRE_DEV_count(p1Wire) ; i++)
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

void    _fte_1wire_print_devices(FTE_1WIRE_PTR p1Wire)
{
    assert(p1Wire != NULL);
    
    printf("     ID : %08x\n", p1Wire->pConfig->nID);
    printf("DEVICES : %d\n", p1Wire->nROMCodes);
    printf("%-6s %-16s %-24s\n", "INDEX", "DEVICE", "ROM_CODE");                
    for(int id = 0 ; id < p1Wire->nROMCodes ; id++)
    {
        printf("%4d : %-16s %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
               id, 
               FTE_1WIRE_getFailmyName(p1Wire->pROMCodes[id][0]), 
               p1Wire->pROMCodes[id][0], p1Wire->pROMCodes[id][1], 
               p1Wire->pROMCodes[id][2], p1Wire->pROMCodes[id][3],
               p1Wire->pROMCodes[id][4], p1Wire->pROMCodes[id][5], 
               p1Wire->pROMCodes[id][6], p1Wire->pROMCodes[id][7]);
    }
}

int_32  FTE_1WIRE_SHELL_cmd(int_32 argc, char_ptr argv[] )
{ 
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (print_usage)
    { 
        goto    error;
    }
    
    switch(argc)
    {
    case    1:
        {
            _fte_1wire_print_info();
        }
        break;
        
    case    2:
        {
            if (strncmp(argv[1], "discovery", strlen(argv[1])) == 0)
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
            else if (strcmp(argv[1], "all") == 0)
            {
                FTE_1WIRE_PTR   p1Wire = _pHead;
                
                while(p1Wire != NULL)
                {
                    _fte_1wire_print_devices(p1Wire);                    
                }
            }
            else
            {
                uint_32 nID;
                
                if (! Shell_parse_hexnum( argv[1], &nID))  
                {
                    return_code = SHELL_EXIT_ERROR;
                    goto error;
                }
                
                FTE_1WIRE_PTR  p1Wire = FTE_1WIRE_get(nID);
                if (p1Wire == NULL)
                {
                    printf("1-Wire xDevID%d not exists!\n", nID);
                   return_code = SHELL_EXIT_ERROR;
                   goto error;
                }

                _fte_1wire_print_devices(p1Wire);
            }
        }
        break;
    
    case    3:
        {
            if (strncmp(argv[1], "discovery", strlen(argv[1])) == 0)
            {
                FTE_1WIRE_PTR   p1Wire;
                uint_32 nID;
                
                if (! Shell_parse_hexnum( argv[2], &nID))  
                {
                    return_code = SHELL_EXIT_ERROR;
                    goto error;
                }
            
                p1Wire = FTE_1WIRE_get(nID);
                if( p1Wire == NULL)
                {
                    return_code = SHELL_EXIT_ERROR;
                    goto error;
                }
                
                FTE_1WIRE_discovery(p1Wire);                
            
                for(int i = 0 ; i < FTE_1WIRE_DEV_count(p1Wire) ; i++)
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
            if (strcmp(argv[1], "add") == 0)
            {
                FTE_1WIRE_PTR   p1Wire;
                uint_32 nID;
                
                uint_8  pROMCode[8];
                
                if (! Shell_parse_hexnum( argv[2], &nID))  
                {
                    return_code = SHELL_EXIT_ERROR;
                    goto error;
                }
            
                p1Wire = FTE_1WIRE_get(nID);
                if( p1Wire == NULL)
                {
                    return_code = SHELL_EXIT_ERROR;
                    goto error;
                }
                
                
                if (strlen(argv[3]) != 16)
                {
                    printf("Invalid ROM Code\n");
                    goto error;
                }
                
                for(int i = 0 ; i < 8 ; i++)
                {
                    uint_8  nHI, nLO;
                    
                    if ('0' <= argv[3][i*2] && argv[3][i*2] <= '9')
                    {
                        nHI = argv[3][i*2] - '0';
                    }
                    else if ('A' <= argv[3][i*2] && argv[3][i*2] <= 'F')
                    {
                        nHI = argv[3][i*2] - 'A' + 10;
                    }
                    else if ('a' <= argv[3][i*2] && argv[3][i*2] <= 'f')
                    {
                        nHI = argv[3][i*2] - 'a' + 10;
                    }
                    else
                    {
                        printf("Invalid ROM Code\n");
                        goto error;
                    }
                    
                    if ('0' <= argv[3][i*2+1] && argv[3][i*2+1] <= '9')
                    {
                        nLO = argv[3][i*2+1] - '0';
                    }
                    else if ('A' <= argv[3][i*2+1] && argv[3][i*2+1] <= 'F')
                    {
                        nLO = argv[3][i*2+1] - 'A' + 10;
                    }
                    else if ('a' <= argv[3][i*2+1] && argv[3][i*2+1] <= 'f')
                    {
                        nLO = argv[3][i*2+1] - 'a' + 10;
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
        print_usage = TRUE;    
    }

error:    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [ discovery | info ] [<xChannel>] \n", argv[0]);
        }
        else
        {
            printf("Usage : %s [ discovery | info ] [<xChannel>] \n", argv[0]);
            printf("        xDevID - 1WIRE Channel \n");
            printf("        discovery - discover connected devices\n");
            printf("        info - channel informations\n");
        }
    }
    

    return   return_code;
}

_mqx_uint   FTE_1WIRE_reset(FTE_1WIRE_PTR p1Wire)
{
    assert(p1Wire != NULL);
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    FTE_LWGPIO_setValue(p1Wire->pLWGPIO, FALSE);
    FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
    fte_udelay(1000);
    FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
    _time_delay(1);
    
    return  MQX_OK;
}

_mqx_uint   FTE_1WIRE_writeByte(FTE_1WIRE_PTR p1Wire, uint_8 data)
{
    assert(p1Wire != NULL);
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    return  FTE_1WIRE_writeBits(p1Wire, &data, 8);
}

_mqx_uint   FTE_1WIRE_writeBits(FTE_1WIRE_PTR p1Wire, uint_8_ptr data, uint_32 bit_len)
{
    int             i;

    assert(p1Wire != NULL);
    if (p1Wire == NULL)
    {
        return  MQX_INVALID_DEVICE;
    }
    
    for(i = 0 ; i < bit_len / 8; i++)
    {
        for(int j = 0 ; j < 8 ; j++)
        {
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            fte_udelay(2);
            if (((data[i] >> j) & 0x01) == 0)
            {
                fte_udelay(100);
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            }
            else
            {
                FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
                fte_udelay(100);
            }
            _int_enable();
        fte_udelay(FTE_1WIRE_TIME_SLOT - 100);
        }
    }
    
    for(int j = 0 ; j < bit_len % 8 ; j++)
    {
       _int_disable();
       FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
        fte_udelay(2);
        if (((data[i] >> j) & 0x01) == 0)
        {
                fte_udelay(100);
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
        }
        else
        {
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            fte_udelay(100);
        }
        _int_disable();
        fte_udelay(FTE_1WIRE_TIME_SLOT - 100);
    }
    return MQX_OK;
}

_mqx_uint   FTE_1WIRE_readBits(FTE_1WIRE_PTR p1Wire, uint_8_ptr data, uint_32 bit_len)
{
    int     i;
    uint_8  buff;
 
    assert(p1Wire != NULL);
    
    for(i = 0 ; i < bit_len / 8; i++)
    {
        buff = 0;
        for(int j = 0 ; j < 8 ; j++)
        {
            boolean bit = 0;
            
            _int_disable();
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
            fte_udelay(2);
            FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
            fte_udelay(5);
            FTE_LWGPIO_getValue(p1Wire->pLWGPIO, &bit);
            if (bit)
            {
                buff |= 1 << j;
            }
            _int_enable();
            fte_udelay(FTE_1WIRE_TIME_SLOT - 7);
        }
        
        data[i] = buff;
    }
    
    buff = 0;
    for(int j = 0 ; j < bit_len % 8 ; j++)
    {
        boolean bit = 0;
        
        _int_disable();
        FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_OUTPUT);
        fte_udelay(2);
        FTE_LWGPIO_setDirection(p1Wire->pLWGPIO, LWGPIO_DIR_INPUT);
        fte_udelay(5);
        FTE_LWGPIO_getValue(p1Wire->pLWGPIO, &bit);
        if (bit)
        {
            buff |= 1 << j;
        }
        _int_enable();
        fte_udelay(FTE_1WIRE_TIME_SLOT - 7);
    }
    data[i] = buff;
 
    
    
    return MQX_ERROR;
}

char_ptr    FTE_1WIRE_getFailmyName(uint_32 code)
{
    static char buff[16];
    
    FTE_1WIRE_FAMILY_NAME_PTR _family = _pFamilyNames;
    
    while(_family->xFamilyCode)
    {
        if (code == _family->xFamilyCode)
        {
            return  _family->pName;
        }
        
        _family++;
    }
    
    snprintf(buff, sizeof(buff), "unknown(%02x)", code);
    return  buff;
}

static uint_32          _n1WireRecoveryTimerID = 0;

_mqx_uint   fte_1wire_auto_recovery_start(void)
{
    MQX_TICK_STRUCT     xTicks;            
    MQX_TICK_STRUCT     xDTicks;

    _time_init_ticks(&xDTicks, 0);
    _time_add_sec_to_ticks(&xDTicks, 300);
    _time_get_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    _n1WireRecoveryTimerID = _timer_start_periodic_at_ticks(_FTE_1WIRE_autoRecoveryRun, NULL, TIMER_KERNEL_TIME_MODE, &xTicks, &xDTicks);
    
    return  MQX_OK;
}

void _FTE_1WIRE_autoRecoveryRun(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    if (_n1WireRecoveryTimerID != id)
    {
        return;
    }
    
    FTE_1WIRE_discovery(0);    
}

_mqx_uint   FTE_1WIRE_lock(FTE_1WIRE_PTR p1Wire)
{
    assert(p1Wire != NULL);
    
    if (_lwsem_wait(&p1Wire->xLWSEM) != MQX_OK)
    {  
        DEBUG("\n_xLWSEM_wait failed");
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

_mqx_uint   FTE_1WIRE_unlock(FTE_1WIRE_PTR p1Wire)
{   
    assert(p1Wire != NULL);

    if (_lwsem_post(&p1Wire->xLWSEM) != MQX_OK)
    {
        DEBUG("\n_xLWSEM_post failed");
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}

#endif