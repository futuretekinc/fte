#include "fte_target.h"
#include "fte_net.h"
#include "fte_config.h"
#include "fte_object.h"
#include "fte_crc.h"
#include "fte_time.h"
#include "fte_sys.h"
#include "fte_log.h"
#include <ipcfg.h>
#include <sh_rtcs.h> 


#ifndef FTE_OBJ_MAX_COUNT
#define FTE_OBJ_MAX_COUNT               ((2048 - sizeof(FTE_UINT32) * 4) / sizeof(FTE_OBJECT_CONFIG))
#endif

#ifndef FTE_LOG_BOOT_TIME_MAX_COUNT
#define FTE_LOG_BOOT_TIME_MAX_COUNT     5
#endif

#ifndef FTE_EVENT_MAX_COUNT
#define FTE_EVENT_MAX_COUNT             32
#endif

#define FTE_CFG_POOL_VERSION            0x20150708
#define FTE_CFG_OBJECT_POOL_VERSION     0x20150708
#define FTE_CFG_EVENT_POOL_VERSION      0x20150708
#define FTE_CFG_EXT_POOL_VERSION        0x20150708
#define FTE_CFG_CERT_POOL_VERSION       0x20150708

static
void    FTE_CFG_lock(void);

static
void    FTE_CFG_unlock(void);

static
void    FTE_CFG_autoSave(FTE_TIMER_ID xTimerID, FTE_VOID_PTR pData, MQX_TICK_STRUCT_PTR pTick);

FTE_CFG_EVENT_PTR       FTE_CFG_EVENT_create(FTE_CFG_EVENT_PTR pConfig);
FTE_RET               FTE_CFG_CERT_load(void);
FTE_RET               FTE_CFG_CERT_save(void);

void                    FTE_CFG_DBG_setBootTime(void);

typedef struct  _FTE_CFG_POOL_STRUCT
{
    FTE_UINT32          ulCRC;
    FTE_UINT32          ulTag;
    FTE_INT32           nID;
    FTE_CHAR            pModel[FTE_MODEL_LENGTH+1];
    FTE_CHAR            pLocation[FTE_LOCATION_MAX_LEN+1];
    
    FTE_SYS_CONFIG      xSystem;
    FTE_SHELL_CONFIG    xShell;
    FTE_NET_CFG         xNetwork;
}   FTE_CFG_POOL, _PTR_ FTE_CFG_POOL_PTR;

typedef struct _FTE_CFG_OBJECT_POOL_STRUCT
{
    FTE_UINT32             ulCRC;
    FTE_UINT32             ulTag;
    FTE_INT32              nID;

    FTE_UINT32             uiObjectCount;
    FTE_OBJECT_CONFIG   pObjects[FTE_OBJ_MAX_COUNT];
} FTE_CFG_OBJECT_POOL, _PTR_ FTE_CFG_OBJECT_POOL_PTR;

typedef struct _FTE_CFG_EVENT_POOL_STRUCT
{
    FTE_UINT32             ulCRC;
    FTE_UINT32             ulTag;
    FTE_INT32              nID;
    FTE_UINT32             uiEventCount;
    FTE_EVENT_CONFIG    pEvents[(0x800 - sizeof(FTE_UINT32) * 4) / sizeof(FTE_EVENT_CONFIG)];
}   FTE_CFG_EVENT_POOL, _PTR_ FTE_CFG_EVENT_POOL_PTR;

typedef struct _FTE_CFG_EXT_POOL_STRUCT
{
    FTE_UINT32             ulCRC;
    FTE_UINT32             ulTag;
    FTE_INT32              nID;
#if FTE_CIAS_SIOUX_CU_SUPPORTED
    FTE_CIAS_SIOUX_CU_EXT_CONFIG    xCU;
#endif
#if FTE_IOEX_SUPPORTED
    FTE_IOEX_EXT_CONFIG xIOEX;
#endif
#if FTE_DOTECH_SUPPORTED
    FTE_DOTECH_EXT_CONFIG xDOTECH;
#endif
#if FTE_MULTI_DIO_NODE_SUPPORTED
    FTE_DIO_NODE_EXT_CONFIG xDIO_NODE;
#endif
}   FTE_CFG_EXT_POOL, _PTR_ FTE_CFG_EXT_POOL_PTR;

typedef struct _FTE_CFG_CERT_POOL_HEAD_STRUCT
{
    FTE_UINT32             ulCRC;
    FTE_UINT32             ulTag;
    FTE_INT32              nID;
    FTE_UINT32             ulCertLen;
}   FTE_CFG_CERT_POOL_HEAD, _PTR_ FTE_CFG_CERT_POOL_HEAD_PTR;

typedef struct _FTE_CFG_CERT_POOL_STRUCT
{
    FTE_UINT32             ulCRC;
    FTE_UINT32             ulTag;
    FTE_INT32              nID;
    FTE_UINT32             ulCertLen;
    FTE_UINT8              pCert[4096 - sizeof(FTE_UINT32)*4];
}   FTE_CFG_CERT_POOL, _PTR_ FTE_CFG_CERT_POOL_PTR;

typedef struct  _FTE_CFG_struct
{
    FTE_CFG_DESC const *    pDESC;
    FTE_BOOL                 bPoolModified;
    FTE_CFG_POOL            xPool;
    FTE_BOOL                 bObjectPoolModified;
    FTE_CFG_OBJECT_POOL     xObjectPool;
    FTE_BOOL                 bEventPoolModified;
    FTE_CFG_EVENT_POOL      xEventPool;
    FTE_BOOL                 bExtPoolModified;
    FTE_CFG_EXT_POOL        xExtPool;
    FTE_UINT32                 ulIndex;
}   FTE_CONFIG, _PTR_ FTE_CFG_PTR;

static 
FTE_CONFIG   _config = { .pDESC = NULL, };

static  
FTE_SYS_LOCK    _xLock;

FTE_RET   FTE_CFG_init(FTE_CFG_DESC const *desc)
{
    FTE_INT32               i;
    FTE_BOOL                bIgnore = TRUE;
    MQX_TICK_STRUCT         xTicks, xDTicks;
    FTE_CFG_POOL_PTR        pPool = NULL;
    FTE_CFG_OBJECT_POOL_PTR pObjectPool = NULL;
    FTE_CFG_EVENT_POOL_PTR  pEventPool = NULL;
    FTE_CFG_EXT_POOL_PTR    pExtPool = NULL;
    FTE_CFG_CERT_POOL_PTR   pCertPool = NULL;
    FTE_VOID_PTR              pBuff = NULL;
    
    assert(_config.pDESC == NULL && desc != NULL);
    
    if (FTE_SYS_LOCK_init(&_xLock, 1) != FTE_RET_OK)
    {
        return  FTE_RET_ERROR;
    }
    
    pBuff = (FTE_VOID_PTR)FTE_MEM_allocZero(4096);
    if (pBuff == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pPool = (FTE_CFG_POOL_PTR)FTE_MEM_allocZero(sizeof(FTE_CFG_POOL));
    if (pPool == NULL)
    {
        goto error;
    }

    pObjectPool = (FTE_CFG_OBJECT_POOL_PTR)FTE_MEM_allocZero(sizeof(FTE_CFG_OBJECT_POOL));
    if (pObjectPool == NULL)
    {
        goto error;
    }

    pEventPool = (FTE_CFG_EVENT_POOL_PTR)FTE_MEM_allocZero(sizeof(FTE_CFG_EVENT_POOL));
    if (pEventPool == NULL)
    {
        goto error;
    }

    pExtPool = (FTE_CFG_EXT_POOL_PTR)FTE_MEM_allocZero(sizeof(FTE_CFG_EXT_POOL));
    if (pExtPool == NULL)
    {
        goto error;
    }

    memset(&_config, 0, sizeof(_config));
    _config.pDESC = desc;
    
    for(int nMTD = 0 ; nMTD < 2 ; nMTD++)
    {
        MQX_FILE_PTR    fp;    
        
        /* Open the flash device */
        fp = fopen(desc->pMTDs[nMTD], NULL);
        if (fp == NULL) 
        {
            continue;
        }

        ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
        read(fp, (FTE_CHAR_PTR)pPool, sizeof(FTE_CFG_POOL));
        fclose(fp);
        
        if ((pPool->ulCRC != FTE_CRC32(0, &pPool->ulTag, sizeof(FTE_CFG_POOL) - sizeof(FTE_UINT32))) ||
            (pPool->ulTag != FTE_CFG_POOL_VERSION) || 
            (pPool->nID > (FTE_UINT32)MAX_INT_32))
        {
            continue;
        }
            
        if (pPool->nID > _config.xPool.nID)
        {
            bIgnore = FALSE;
            memcpy(&_config.xPool, pPool, sizeof(FTE_CFG_POOL));
        }
    }            
        
    if (!bIgnore)
    {
        for(int nMTD = 2 ; nMTD < 4 ; nMTD++)
        {
            FTE_INT32          nLen;
            MQX_FILE_PTR    fp;    
            
            /* Open the flash device */
            fp = fopen(desc->pMTDs[nMTD], NULL);
            if (fp == NULL) 
            {
                continue;
            }

            ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
            nLen = read(fp, (FTE_CHAR_PTR)pObjectPool, sizeof(FTE_CFG_OBJECT_POOL));
            fclose(fp);
            
            if ((nLen != sizeof(FTE_CFG_OBJECT_POOL)) ||
                (pObjectPool->ulCRC != FTE_CRC32(0, &pObjectPool->ulTag, sizeof(FTE_CFG_OBJECT_POOL) - sizeof(FTE_UINT32))) ||
                (pObjectPool->ulTag != FTE_CFG_OBJECT_POOL_VERSION) || 
                (pObjectPool->nID > (FTE_UINT32)MAX_INT_32))
            {
                continue;
            }
                
            if (pObjectPool->nID > _config.xObjectPool.nID)
            {
                memcpy(&_config.xObjectPool, pObjectPool, sizeof(FTE_CFG_OBJECT_POOL));
            }
        }            
            
        for(int nMTD = 4 ; nMTD < 6 ; nMTD++)
        {
            MQX_FILE_PTR    fp;    
            
            /* Open the flash device */
            fp = fopen(desc->pMTDs[nMTD], NULL);
            if (fp == NULL) 
            {
                continue;
            }

            ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
            read(fp, (FTE_CHAR_PTR)pEventPool, sizeof(FTE_CFG_EVENT_POOL));
            fclose(fp);

            if ((pEventPool->ulCRC != FTE_CRC32(0, &pEventPool->ulTag, sizeof(FTE_CFG_EVENT_POOL) - sizeof(FTE_UINT32))) ||
                (pEventPool->ulTag != FTE_CFG_EVENT_POOL_VERSION) || 
                (pEventPool->nID > (FTE_UINT32)MAX_INT_32))
            {
                continue;
            }
                
            if (pEventPool->nID > _config.xEventPool.nID)
            {
                memcpy(&_config.xEventPool, pEventPool, sizeof(FTE_CFG_EVENT_POOL));
            }
        }    
        
        for(int nMTD = 6 ; nMTD < 8 ; nMTD++)
        {
            MQX_FILE_PTR    fp;    
            
            /* Open the flash device */
            fp = fopen(desc->pMTDs[nMTD], NULL);
            if (fp == NULL) 
            {
                continue;
            }

            ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
            read(fp, (FTE_CHAR_PTR)pExtPool, sizeof(FTE_CFG_EXT_POOL));
            fclose(fp);

            if ((pExtPool->ulCRC != FTE_CRC32(0, &pExtPool->ulTag, sizeof(FTE_CFG_EXT_POOL) - sizeof(FTE_UINT32))) ||
                (pExtPool->ulTag != FTE_CFG_EXT_POOL_VERSION) || 
                (pExtPool->nID > (FTE_UINT32)MAX_INT_32))
            {
                continue;
            }
                
            if (pExtPool->nID > _config.xExtPool.nID)
            {
                memcpy(&_config.xExtPool, pExtPool, sizeof(FTE_CFG_EXT_POOL));
            }
        }     
    }
    
    if (_config.xPool.nID == 0)
    {
        _config.xPool.ulTag    = FTE_CFG_POOL_VERSION;
        strcpy(_config.xPool.pModel, FTE_MODEL);
        
        FTE_CFG_SYS_set(desc->pSystem);
        FTE_CFG_SHELL_set(desc->pShell);
        FTE_CFG_NET_set(desc->pNetwork);        
    }
    
    if (_config.xObjectPool.nID == 0)
    {
        _config.xObjectPool.ulTag    = FTE_CFG_OBJECT_POOL_VERSION;
        
        for( i = 0 ; i < desc->nObjects ; i++)
        {
            FTE_OBJECT_CONFIG_PTR   pNewConfig;
            
            FTE_CFG_OBJ_create(desc->pObjects[i], &pNewConfig, NULL, 0, NULL);
        }
    } 
    
    if (_config.xEventPool.nID == 0)
    {
        _config.xEventPool.ulTag    = FTE_CFG_EVENT_POOL_VERSION;
        
        for( i = 0 ; i < desc->nEvents ; i++)
        {
            FTE_CFG_EVENT_create(desc->pEvents[i]);
        }
    } 

    if (_config.xExtPool.nID == 0)
    {
        FTE_CFG_EXT_init();
    } 

    FTE_LOG_init();
    FTE_CFG_DBG_setBootTime();
    
    FTE_CFG_save(FALSE);
    
    FTE_MEM_free(pPool);  
    FTE_MEM_free(pObjectPool);
    FTE_MEM_free(pEventPool);    
    FTE_MEM_free(pExtPool);    
    FTE_MEM_free(pBuff);    

    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, _config.xPool.xSystem.ulAutoSaveInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    _timer_start_periodic_at_ticks(FTE_CFG_autoSave, NULL, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);

    return  FTE_RET_OK;
    
error:

    if (pPool != NULL)
    {
        FTE_MEM_free(pPool);
    }
    
    if (pObjectPool != NULL)
    {
        FTE_MEM_free(pObjectPool);
    }
    
    if (pEventPool != NULL)
    {
        FTE_MEM_free(pEventPool);
    }
        
    if (pExtPool != NULL)
    {
        FTE_MEM_free(pExtPool);
    }
        
    if (pCertPool != NULL)
    {
        FTE_MEM_free(pCertPool);
    }
        
    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
    }
    
    return  FTE_RET_ERROR;
}


FTE_RET FTE_CFG_save(FTE_BOOL force)
{
    ASSERT(_config.pDESC != NULL);
    
    FTE_CFG_lock();
    
    if ((force) || (_config.bPoolModified == TRUE))
    {
        int             i;
        MQX_FILE_PTR    fp;
        
        for(i = 0 ; i < 2 ; i++)
        {
            _config.xPool.nID++;
            _config.xPool.ulCRC = FTE_CRC32(0, &_config.xPool.ulTag, sizeof(FTE_CFG_POOL) - sizeof(FTE_UINT32));   
        
            int nMTD = _config.xPool.nID & 0x01;
            
            /* Open the flash device */
            fp = fopen(_config.pDESC->pMTDs[nMTD], NULL);
            if (fp != NULL) 
            {
                ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
                
                if (sizeof(FTE_CFG_POOL) != write(fp, (FTE_VOID_PTR)&_config.xPool, sizeof(FTE_CFG_POOL)))
                {
                    fprintf(stderr, "\nError writing to the file. Error code: %d", _io_ferror(fp));
                    fclose(fp);
                    
                    goto error;
                }

                fflush(fp);
                fclose(fp);
            }
        }
        
        _config.bPoolModified = FALSE;
    }

    if ((force) || (_config.bObjectPoolModified == TRUE))
    {
        int             i;
        MQX_FILE_PTR    fp;
        
        for(i = 0 ; i < 2 ; i++)
        {
            _config.xObjectPool.nID++;
            _config.xObjectPool.ulCRC = FTE_CRC32(0, &_config.xObjectPool.ulTag, sizeof(FTE_CFG_OBJECT_POOL) - sizeof(FTE_UINT32));   
        
            int nMTD = _config.xObjectPool.nID & 0x01;
            
            /* Open the flash device */
            fp = fopen(_config.pDESC->pMTDs[nMTD + 2], NULL);
            if (fp != NULL) 
            {
                ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
                
                if (sizeof(FTE_CFG_OBJECT_POOL) != write(fp, (FTE_VOID_PTR)&_config.xObjectPool, sizeof(FTE_CFG_OBJECT_POOL)))
                {
                    fprintf(stderr, "\nError writing to the file. Error code: %d", _io_ferror(fp));
                    fclose(fp);
                    
                    goto error;
                }

                fflush(fp);
                fclose(fp);
            }
        }
        
        _config.bObjectPoolModified = FALSE;
    }

    if ((force) || (_config.bEventPoolModified == TRUE))
    {
        int             i;
        MQX_FILE_PTR    fp;
        
        for(i = 0 ; i < 2 ; i++)
        {
            _config.xEventPool.nID++;
            _config.xEventPool.ulCRC = FTE_CRC32(0, &_config.xEventPool.ulTag, sizeof(FTE_CFG_EVENT_POOL) - sizeof(FTE_UINT32));   
        
            int nMTD = _config.xEventPool.nID & 0x01;

            fp = fopen(_config.pDESC->pMTDs[nMTD + 4], NULL);
            if (fp != NULL) 
            {
                ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
                
                if (sizeof(FTE_CFG_EVENT_POOL) != write(fp, (FTE_VOID_PTR)&_config.xEventPool, sizeof(FTE_CFG_EVENT_POOL)))
                {
                    fprintf(stderr, "\nError writing to the file. Error code: %d", _io_ferror(fp));
                    fclose(fp);
                    
                    goto error;
                }

                fflush(fp);
                fclose(fp);
            }
        }
        
        _config.bEventPoolModified = FALSE;
    }

    if ((force) || (_config.bExtPoolModified == TRUE))
    {
        int             i;
        MQX_FILE_PTR    fp;
        
        for(i = 0 ; i < 2 ; i++)
        {
            _config.xExtPool.nID++;
            _config.xExtPool.ulCRC = FTE_CRC32(0, &_config.xExtPool.ulTag, sizeof(FTE_CFG_EXT_POOL) - sizeof(FTE_UINT32));   
        
            int nMTD = _config.xExtPool.nID & 0x01;

            fp = fopen(_config.pDESC->pMTDs[nMTD + 6], NULL);
            if (fp != NULL) 
            {
                ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
                
                if (sizeof(FTE_CFG_EXT_POOL) != write(fp, (FTE_VOID_PTR)&_config.xExtPool, sizeof(FTE_CFG_EXT_POOL)))
                {
                    fprintf(stderr, "\nError writing to the file. Error code: %d", _io_ferror(fp));
                    fclose(fp);
                    
                    goto error;
                }

                fflush(fp);
                fclose(fp);
            }
        }
        
        _config.bExtPoolModified = FALSE;
    }

    goto success;
    
error:
    FTE_CFG_unlock();
    
    return  FTE_RET_ERROR;

success:
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

FTE_RET FTE_CFG_clear(void) 
{
    FTE_INT32   i;

    assert(_config.pDESC != NULL);
    
    FTE_CFG_lock();
    memset(&_config.xPool, 0, sizeof(FTE_CFG_POOL));
    memset(&_config.xObjectPool, 0, sizeof(FTE_CFG_OBJECT_POOL));
    memset(&_config.xEventPool, 0, sizeof(FTE_CFG_EVENT_POOL));
    memset(&_config.xExtPool, 0, sizeof(FTE_CFG_EXT_POOL));

    _config.xPool.ulTag    = FTE_CFG_POOL_VERSION;    
    FTE_CFG_unlock();
    
    FTE_CFG_SYS_set(_config.pDESC->pSystem);
    FTE_CFG_SHELL_set(_config.pDESC->pShell);
    FTE_CFG_NET_set(_config.pDESC->pNetwork);        
   
    _config.xObjectPool.ulTag    = FTE_CFG_OBJECT_POOL_VERSION;    
    for( i = 0 ; i < _config.pDESC->nObjects ; i++)
    {
        FTE_OBJECT_CONFIG_PTR   pNewConfig;
            
        FTE_CFG_OBJ_create(_config.pDESC->pObjects[i], &pNewConfig, NULL, 0, NULL);
    }
    
    FTE_CFG_lock();
    _config.xEventPool.ulTag    = FTE_CFG_EVENT_POOL_VERSION;
    FTE_CFG_unlock();
      
    for( i = 0 ; i < _config.pDESC->nEvents ; i++)
    {
        FTE_CFG_EVENT_create(_config.pDESC->pEvents[i]);
    }

    FTE_CFG_lock();
    FTE_CFG_EXT_init();
    FTE_CFG_unlock();
    
    FTE_CFG_save(TRUE);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_CFG_clearObject(void) 
{
    FTE_INT32   i;

    assert(_config.pDESC != NULL);
    
    FTE_CFG_lock();
    memset(&_config.xObjectPool, 0, sizeof(FTE_CFG_OBJECT_POOL));
    memset(&_config.xEventPool, 0, sizeof(FTE_CFG_EVENT_POOL));
    memset(&_config.xExtPool, 0, sizeof(FTE_CFG_EXT_POOL));

    _config.xPool.ulTag    = FTE_CFG_POOL_VERSION;    
    FTE_CFG_unlock();
    
    FTE_CFG_SYS_set(_config.pDESC->pSystem);
    FTE_CFG_SHELL_set(_config.pDESC->pShell);
    FTE_CFG_NET_set(_config.pDESC->pNetwork);        
   
    _config.xObjectPool.ulTag    = FTE_CFG_OBJECT_POOL_VERSION;    
    for( i = 0 ; i < _config.pDESC->nObjects ; i++)
    {
        FTE_OBJECT_CONFIG_PTR   pNewConfig;
            
        FTE_CFG_OBJ_create(_config.pDESC->pObjects[i], &pNewConfig, NULL, 0, NULL);
    }
    
    FTE_CFG_lock();
    _config.xEventPool.ulTag    = FTE_CFG_EVENT_POOL_VERSION;
    FTE_CFG_unlock();
      
    for( i = 0 ; i < _config.pDESC->nEvents ; i++)
    {
        FTE_CFG_EVENT_create(_config.pDESC->pEvents[i]);
    }

    FTE_CFG_lock();
    FTE_CFG_EXT_init();
    FTE_CFG_unlock();
    
    FTE_CFG_save(TRUE);
    
    return  FTE_RET_OK;
}

void    FTE_CFG_lock(void)
{
    _int_disable();
    FTE_SYS_LOCK_enable(&_xLock);
}

void    FTE_CFG_unlock(void)
{
    FTE_SYS_LOCK_disable(&_xLock);
    _int_enable();
}

FTE_RET FTE_CFG_isChanged
(
    FTE_BOOL_PTR    pChanged
)
{
    ASSERT(pChanged != NULL);
    
    *pChanged = _config.bPoolModified;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_OBJ_save(FTE_OBJECT_PTR pObj)
{
    FTE_OBJECT_CONFIG_PTR pConfig = FTE_CFG_OBJ_get(pObj->pConfig->xCommon.nID);
    if (pConfig == NULL)
    {
        return  FTE_RET_ERROR;
    }

    FTE_CFG_lock();
    memcpy(pConfig, pObj->pConfig, sizeof(FTE_OBJECT_CONFIG));
    _config.bObjectPoolModified  = TRUE;
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_getLocation(FTE_CHAR_PTR pLocation, FTE_UINT32 nLen)
{
    if (nLen < strlen(_config.xPool.pLocation))
    {
        return  FTE_RET_INVALID_SIZE;
    }
    
    strncpy(pLocation, _config.xPool.pLocation, nLen);
    
    return  FTE_RET_OK;
}
 
FTE_RET   FTE_CFG_setLocation(FTE_CHAR_PTR pLocation, FTE_UINT32 nLen)
{
    if (nLen > FTE_LOCATION_MAX_LEN)
    {
        return  FTE_RET_INVALID_SIZE;
    }
    
    FTE_CFG_lock();
    strncpy(_config.xPool.pLocation, pLocation, nLen);
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

FTE_UINT32 FTE_CFG_objects_count(void)
{
    return  _config.xObjectPool.uiObjectCount;
}

FTE_VOID_PTR FTE_CFG_OBJ_get(FTE_UINT32 oid)
{
    if (_config.pDESC != NULL && _config.xObjectPool.uiObjectCount != 0)
    {
        FTE_RET   i;
    
        for(i = 0 ; i < FTE_OBJ_MAX_COUNT ; i++)
        {
            if ((_config.xObjectPool.pObjects[i].xCommon.nID != 0) && (_config.xObjectPool.pObjects[i].xCommon.nID == oid))
            {
                return  &_config.xObjectPool.pObjects[i];
            }
        }
    }
    
    return  NULL;
}

FTE_VOID_PTR             FTE_CFG_OBJ_getAt(FTE_UINT32 oid, FTE_UINT32 ulMask, FTE_UINT32 ulIdx)
{
    if ((_config.pDESC != NULL) && (ulIdx < _config.xObjectPool.uiObjectCount))
    {
        if (ulMask == 0)
        {
            return  &_config.xObjectPool.pObjects[ulIdx];
        }
        else
        {
            FTE_RET   i;
        
            for(i = 0 ; i < FTE_OBJ_MAX_COUNT ; i++)
            {
                if ((_config.xObjectPool.pObjects[i].xCommon.nID != 0) && ((_config.xObjectPool.pObjects[i].xCommon.nID & ulMask) == (oid & ulMask)))
                {
                    if (ulIdx == 0)
                    {
                        return  &_config.xObjectPool.pObjects[i];
                    }
                    
                    ulIdx--;
                }
            }
        }
    }
    
    return  NULL;
}

FTE_OBJECT_CONFIG_PTR FTE_CFG_OBJ_alloc(FTE_UINT32 nID)
{    
    if (_config.pDESC != NULL && _config.xObjectPool.uiObjectCount != FTE_OBJ_MAX_COUNT)
    {
        FTE_RET   i;
        
        for(i = 0 ; i < FTE_OBJ_MAX_COUNT ; i++)
        {
            if (_config.xObjectPool.pObjects[i].xCommon.nID == 0)
            {
//                _config.xPool.pObjects[i].nID = nID;
                _config.xObjectPool.uiObjectCount++;
                
                return  &_config.xObjectPool.pObjects[i];
            }
        }
    }    

    return  NULL;
}

FTE_RET   FTE_CFG_OBJ_free(FTE_UINT32 oid)
{
    if (_config.pDESC != NULL && _config.xObjectPool.uiObjectCount != 0)
    {
        FTE_RET   i;
         
        for(i = 0 ; i < FTE_OBJ_MAX_COUNT ; i++)
        {
            if (_config.xObjectPool.pObjects[i].xCommon.nID == oid)
            {
                memset(&_config.xObjectPool.pObjects[i], 0, sizeof(FTE_OBJECT_CONFIG));
                _config.xObjectPool.uiObjectCount--;

                return  FTE_RET_OK;
            }
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_RET FTE_CFG_OBJ_create
(
    FTE_OBJECT_CONFIG_PTR   pBase,
    FTE_OBJECT_CONFIG_PTR _PTR_ ppNewConfig,
    FTE_OBJECT_CONFIG_PTR _PTR_ pChildObjects,
    FTE_UINT32                 ulMaxCount,
    FTE_UINT32_PTR             pulCount
)
{
    FTE_UINT32 ulChildCount = 0;
    
    FTE_OBJECT_CONFIG_PTR pConfig = FTE_CFG_OBJ_alloc(pBase->xCommon.nID);
    if (pConfig == NULL)
    {
        return  FTE_RET_INSUFFICIENT_MEMORY;
    }
            
    int     i;
    FTE_UINT32 ulIndexID = 0;
    FTE_UINT32 ulGroupID = 0;
    FTE_UINT32 ulCount = FTE_CFG_OBJ_count(pBase->xCommon.nID & FTE_OBJ_CLASS_MASK, FTE_OBJ_CLASS_MASK);
    
    ulIndexID = (ulCount + 1);
    
    memcpy(pConfig, pBase, sizeof(FTE_OBJECT_CONFIG));
    if (pConfig->xCommon.ulChild != 0)
    {
        ulGroupID = ulIndexID;
    }

    pConfig->xCommon.nID = (pBase->xCommon.nID & FTE_OBJ_TYPE_MASK) | (ulGroupID << 8) | ulIndexID;
    
    snprintf(pConfig->xCommon.pName, MAX_OBJECT_NAME_LEN, "%s-%04x",  pBase->xCommon.pName, ulIndexID);
    
    for(i = 0 ; i < pConfig->xCommon.ulChild ; i++)
    {
        FTE_OBJECT_CONFIG_PTR pChild = FTE_CFG_OBJ_alloc(pConfig->xCommon.pChild[i]->xCommon.nID);
        if (pChild != NULL)
        {
            ulCount = FTE_CFG_OBJ_count(pConfig->xCommon.pChild[i]->xCommon.nID & FTE_OBJ_CLASS_MASK, FTE_OBJ_CLASS_MASK);
            
            memcpy(pChild, pConfig->xCommon.pChild[i], sizeof(FTE_OBJECT_CONFIG));
            memset(pChild->xCommon.pName, 0, sizeof(pChild->xCommon.pName));
            snprintf(pChild->xCommon.pName, MAX_OBJECT_NAME_LEN, "%s-%04x",  pConfig->xCommon.pChild[i]->xCommon.pName, (FTE_UINT16)pConfig->xCommon.nID);                
            ((FTE_IFCE_CONFIG_PTR)pChild)->nDevID = pConfig->xCommon.nID;
            pChild->xCommon.nID = (pConfig->xCommon.pChild[i]->xCommon.nID & FTE_OBJ_TYPE_MASK) | (ulGroupID << 8) | (ulCount + 1);
            
            if ((pChildObjects != NULL) && (ulChildCount < ulMaxCount))
            {
                pChildObjects[ulChildCount++] = pChild;
            }
            else
            {
                ulCount++;
            }
        }
    }

    if ((pulCount != NULL) && (ulChildCount <= ulMaxCount))
    {
        *pulCount = ulChildCount;
    }    
    
    *ppNewConfig = pConfig;
    
    _config.bObjectPoolModified = TRUE;
    
    return  FTE_RET_OK;
}

FTE_UINT32 FTE_CFG_OBJ_count(FTE_UINT32 ulType, FTE_UINT32 ulMask)
{
    if (_config.pDESC != NULL)
    {
        if (ulType == 0)
        {
            return  _config.xObjectPool.uiObjectCount;
        }
        else
        {
            int     i;
            FTE_UINT32 ulCount = 0;
            for(i = 0 ; i < FTE_OBJ_MAX_COUNT ; i++)
            {
                if ((_config.xObjectPool.pObjects[i].xCommon.nID & ulMask) == ulType)
                {
                    ulCount++;
                }
            }
            
            return  ulCount;            
        }
    }
    
    return  0;
}

FTE_VOID_PTR FTE_CFG_OBJ_getFirst(void)
{
    if ((_config.pDESC != NULL) && (_config.xObjectPool.uiObjectCount != 0))
    {
        _config.ulIndex = 0;
        return  &_config.xObjectPool.pObjects[_config.ulIndex];
    }
    
    return  NULL;
}

FTE_VOID_PTR FTE_CFG_OBJ_getNext(void)
{
    if ((_config.pDESC != NULL) && (++_config.ulIndex < _config.xObjectPool.uiObjectCount))
    {
        {
            return  &_config.xObjectPool.pObjects[_config.ulIndex];
        }
    }
    
    return  NULL;
}


FTE_CFG_EVENT_PTR FTE_CFG_EVENT_alloc(FTE_UINT32  ulOID)
{    
    if (_config.pDESC != NULL && _config.xEventPool.uiEventCount != FTE_EVENT_MAX_COUNT)
    {
        FTE_RET   i;
        
        for(i = 0 ; i < FTE_EVENT_MAX_COUNT ; i++)
        {
            if (_config.xEventPool.pEvents[i].ulEPID == 0)
            {
                _config.xEventPool.pEvents[i].ulEPID = ulOID;
                _config.xEventPool.uiEventCount++;
                
                return  &_config.xEventPool.pEvents[i];
            }
        }
    }    

    return  NULL;
}

FTE_RET   FTE_CFG_EVENT_free(FTE_CFG_EVENT_PTR pConfig)
{
    if (_config.pDESC != NULL && _config.xEventPool.uiEventCount != 0)
    {
        FTE_RET   i;
         
        for(i = 0 ; i < FTE_EVENT_MAX_COUNT ; i++)
        {
            if (&_config.xEventPool.pEvents[i] == pConfig)
            {
                memset(&_config.xEventPool.pEvents[i], 0, sizeof(FTE_EVENT_CONFIG));
                _config.xEventPool.uiEventCount--;

                return  FTE_RET_OK;
            }
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_CFG_EVENT_PTR FTE_CFG_EVENT_create(FTE_CFG_EVENT_PTR pConfig)
{
    FTE_CFG_EVENT_PTR pEvent = FTE_CFG_EVENT_alloc(pConfig->ulEPID);
    
    if (pEvent != NULL)
    {
        memcpy(pEvent, pConfig, sizeof(FTE_EVENT_CONFIG));
        _config.bEventPoolModified = TRUE;
    }
    
    return  pEvent;
}

FTE_UINT32 FTE_CFG_EVENT_count(void)
{
    if (_config.pDESC != NULL)
    {
        return  _config.xEventPool.uiEventCount;
    }
    
    return  0;
}

FTE_UINT32 FTE_CFG_EVENT_getAt(FTE_UINT32 ulIndex, FTE_CFG_EVENT_PTR _PTR_ ppConfig)
{
    if ((_config.pDESC != NULL) && (ulIndex < _config.xEventPool.uiEventCount))
    {
        for(int i = 0 ; i < FTE_EVENT_MAX_COUNT ; i++)
        {
            if (_config.xEventPool.pEvents[i].ulEPID != 0)
            {
                if (ulIndex == 0)
                {
                    *ppConfig = &_config.xEventPool.pEvents[i];
                    return  FTE_RET_OK;
                }
                
                ulIndex --;
            }
        }
    }
    
    return  FTE_RET_ERROR;
}

FTE_VOID_PTR FTE_CFG_EVENT_getFirst(void)
{
    if ((_config.pDESC != NULL) && (_config.xEventPool.uiEventCount != 0))
    {
        _config.ulIndex = 0;
        return  &_config.xEventPool.pEvents[_config.ulIndex];
    }
    
    return  NULL;
}

FTE_VOID_PTR FTE_CFG_EVENT_getNext(void)
{
    if ((_config.pDESC != NULL) && (++_config.ulIndex < _config.xEventPool.uiEventCount))
    {
        {
            return  &_config.xEventPool.pEvents[_config.ulIndex];
        }
    }
    
    return  NULL;
}

FTE_RET   FTE_CFG_EXT_init(void)
{
    _config.xExtPool.ulTag    = FTE_CFG_EXT_POOL_VERSION;
#if FTE_CIAS_SIOUX_CU_SUPPORTED
    FTE_CIAS_SIOUX_CU_initDefaultExtConfig(&_config.xExtPool.xCU);
#endif
#if FTE_IOEX_SUPPORTED
    FTE_IOEX_initDefaultExtConfig(&_config.xExtPool.xIOEX);
#endif
#if FTE_DOTECH_SUPPORTED
    FTE_DOTECH_initDefaultExtConfig(&_config.xExtPool.xDOTECH);
#endif
#if FTE_MULTI_DIO_NODE_SUPPORTED
    FTE_DIO_NODE_initDefaultExtConfig(&_config.xExtPool.xDIO_NODE);
#endif
    _config.bExtPoolModified = TRUE;
     
    return  FTE_RET_OK;
} 

#if FTE_CIAS_SIOUX_CU_SUPPORTED
FTE_RET   FTE_CFG_CIAS_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen)
{
    if ((_config.pDESC == NULL) || (ulBuffLen != sizeof(_config.xExtPool.xCU)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(pBuff, &_config.xExtPool.xCU, sizeof(_config.xExtPool.xCU));
    
    return  FTE_RET_OK;    
}

FTE_RET   FTE_CFG_CIAS_setExtConfig(FTE_VOID_PTR pCIAS, FTE_UINT32 ulCIASLen)
{
    if ((_config.pDESC == NULL) || (ulCIASLen != sizeof(_config.xExtPool.xCU)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(&_config.xExtPool.xCU, pCIAS, sizeof(_config.xExtPool.xCU));
    _config.bExtPoolModified = TRUE;
    
    return  FTE_RET_OK;    
}
#endif

#if FTE_IOEX_SUPPORTED
FTE_RET   FTE_CFG_IOEX_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen)
{
    if ((_config.pDESC == NULL) || (ulBuffLen != sizeof(_config.xExtPool.xIOEX)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(pBuff, &_config.xExtPool.xIOEX, sizeof(_config.xExtPool.xIOEX));
    
    return  FTE_RET_OK;    
}

FTE_RET   FTE_CFG_IOEX_setExtConfig(FTE_VOID_PTR pIOEX, FTE_UINT32 ulIOEXLen)
{
    if ((_config.pDESC == NULL) || (ulIOEXLen != sizeof(_config.xExtPool.xIOEX)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(&_config.xExtPool.xIOEX, pIOEX, sizeof(_config.xExtPool.xIOEX));
    _config.bExtPoolModified = TRUE;
    
    return  FTE_RET_OK;    
}
#endif

#if FTE_MULTI_DIO_NODE_SUPPORTED
FTE_RET   FTE_CFG_DIO_NODE_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen)
{
    if ((_config.pDESC == NULL) || (ulBuffLen != sizeof(_config.xExtPool.xDIO_NODE)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(pBuff, &_config.xExtPool.xDIO_NODE, sizeof(_config.xExtPool.xDIO_NODE));
    
    return  FTE_RET_OK;    
}

FTE_RET   FTE_CFG_DIO_NODE_setExtConfig(FTE_VOID_PTR pIOEX, FTE_UINT32 ulIOEXLen)
{
    if ((_config.pDESC == NULL) || (ulIOEXLen != sizeof(_config.xExtPool.xDIO_NODE)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(&_config.xExtPool.xDIO_NODE, pIOEX, sizeof(_config.xExtPool.xDIO_NODE));
    _config.bExtPoolModified = TRUE;
    
    return  FTE_RET_OK;    
}
#endif

#if FTE_DOTECH_SUPPORTED
FTE_RET   FTE_CFG_DOTECH_getExtConfig(FTE_VOID_PTR pBuff, FTE_UINT32 ulBuffLen)
{
    if ((_config.pDESC == NULL) || (ulBuffLen != sizeof(_config.xExtPool.xDOTECH)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(pBuff, &_config.xExtPool.xDOTECH, sizeof(_config.xExtPool.xDOTECH));
    
    return  FTE_RET_OK;    
}

FTE_RET   FTE_CFG_DOTECH_setExtConfig(FTE_VOID_PTR pDOTECH, FTE_UINT32 ulLen)
{
    if ((_config.pDESC == NULL) || (ulLen != sizeof(_config.xExtPool.xDOTECH)))
    {
        return  FTE_RET_ERROR;
    }

    memcpy(&_config.xExtPool.xDOTECH, pDOTECH, sizeof(_config.xExtPool.xDOTECH));
    _config.bExtPoolModified = TRUE;
    
    return  FTE_RET_OK;    
}
#endif


/******************************************************************************
 * Support for Certificate
 ******************************************************************************/
static FTE_CFG_CERT_POOL_PTR pCertPool = NULL;

FTE_BOOL FTE_CFG_CERT_valid(void)
{
    if (pCertPool == NULL)
    {
        if (FTE_CFG_CERT_load() != FTE_RET_OK)
        {
            return  FALSE;
        }
    }
    
    return  TRUE;
}

FTE_UINT32 FTE_CFG_CERT_size(void)
{
    if (pCertPool == NULL)
    {
        if (FTE_CFG_CERT_load() != FTE_RET_OK)
        {
            return  0;
        }
    }

    return  pCertPool->ulCertLen;
}

FTE_RET FTE_CFG_CERT_get
(
    FTE_VOID_PTR    pBuff, 
    FTE_UINT32      ulBuffLen,
    FTE_UINT32_PTR  pLen
)
{
    ASSERT(pBuff != NULL);
    
    FTE_RET xRet;
    
    if (pCertPool == NULL)
    {
        xRet = FTE_CFG_CERT_load();
        if (xRet != FTE_RET_OK)
        {            
            return  xRet;
        }
    }
    
    if (ulBuffLen < pCertPool->ulCertLen)
    {
        return  FTE_RET_BUFFER_TOO_SMALL;
    }
    
    memcpy(pBuff, pCertPool->pCert, pCertPool->ulCertLen);
    
    if (pLen != NULL)
    {
        *pLen = pCertPool->ulCertLen;
    }
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_CERT_load(void)
{
    FTE_CFG_CERT_POOL_PTR pTempCertPools = (FTE_CFG_CERT_POOL_PTR)0x7E000;
    
    if (_config.pDESC == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    if (pCertPool != NULL)
    {
        return  FTE_RET_OK;
    }
    
    for(int i = 0 ; i < 1 ; i++)
    {
        if ((pTempCertPools[i].ulCertLen > sizeof(pTempCertPools[i].pCert)) ||
            (pTempCertPools[i].ulTag != FTE_CFG_CERT_POOL_VERSION) ||
            (pTempCertPools[i].ulCRC != FTE_CRC32(0, &pTempCertPools[i].ulTag, sizeof(FTE_CFG_CERT_POOL_HEAD) - sizeof(FTE_UINT32) + pTempCertPools[i].ulCertLen)) ||
            (pTempCertPools[i].nID > (FTE_UINT32)MAX_INT_32))
        {
            continue;
        }

        if ((pCertPool == NULL) || (pTempCertPools[i].nID > pCertPool->nID))
        {
            pCertPool = &pTempCertPools[i];
        }
    }            
    
    if (pCertPool == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    return  FTE_RET_OK;
}


FTE_RET   FTE_CFG_CERT_set
(
    FTE_VOID_PTR    pCert, 
    FTE_UINT32      ulCertLen
)
{
    ASSERT(pCert != NULL);

    FTE_CFG_CERT_POOL_HEAD  xCertHead;
    
    if (pCertPool == NULL)
    {
        xCertHead.nID = 0;
    }
    else
    {
        xCertHead.nID = pCertPool->nID;
    }
    xCertHead.ulTag = FTE_CFG_CERT_POOL_VERSION;
        
    for(int i = 0 ; i < 2 ; i++)
    {
        MQX_FILE_PTR    fp;
        
        xCertHead.nID++;
        xCertHead.ulCRC = FTE_CRC32(0, &xCertHead.ulTag, sizeof(FTE_CFG_CERT_POOL_HEAD) - sizeof(FTE_UINT32));   
        xCertHead.ulCRC = FTE_CRC32(xCertHead.ulCRC, pCert, ulCertLen);
        
        FTE_CHAR    pFileName[32];
            
        sprintf(pFileName, "flashx:cert%d", (xCertHead.nID & 0x01));
            
        fp = fopen(pFileName, NULL);
        if (fp == NULL) 
        {
            break;
        }
        
        ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
        if ((sizeof(FTE_CFG_CERT_POOL_HEAD) != write(fp, &xCertHead, sizeof(xCertHead))) ||
            (ulCertLen != write(fp, pCert, ulCertLen)))
        {
            fprintf(stderr, "\nError writing to the file. Error code: %d", _io_ferror(fp));
        }

        fflush(fp);
        fclose(fp);
    }       

    return  FTE_RET_OK;
}

/******************************************************************************
 * CERT command
 ******************************************************************************/
FTE_INT32  FTE_CFG_CERT_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[]
)
{
    FTE_BOOL        bPrintUsage, bShortHelp = FALSE;
    FTE_INT32       xRet = SHELL_EXIT_SUCCESS;
    FTE_UINT32      ulBuffLen = 4096;
    FTE_UINT8_PTR   pBuff = NULL;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    pBuff = (FTE_UINT8_PTR)FTE_MEM_alloc(ulBuffLen);
    if (pBuff == NULL)
    {
        printf("Not enough memory!\n");
        return  SHELL_EXIT_ERROR;
    }
    
    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    2:
            {
                FTE_UINT32 ulLen;
               
                if (strcmp(pArgv[1], "show") == 0)
                {
                   xRet = FTE_CFG_CERT_get(pBuff, ulBuffLen, &ulLen);
                   if (xRet != FTE_RET_OK)
                   {
                        printf("Can't find CERT\n");
                   }
                   else
                   {
                       printf("%s", pBuff);                   
                   }
                }
            }
            break;
            
        case    4:
            {
                FTE_UINT8_PTR   pData;
                FTE_UINT32      ulDataLen = 0;
                FTE_UINT32      ulLen;
                _ip_address     xServerIP = 0;
                TFTP_DATA_STRUCT    xTFTPData;

                if (strcmp(pArgv[1], "load") == 0)
                {
                    if (FTE_strToIP(pArgv[2], &xServerIP) != FTE_RET_OK)
                    {
                        printf ("Error in parameter, invalid ip address!\n");
                        xRet = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    xTFTPData.SERVER   = xServerIP;
                    xTFTPData.FILENAME = pArgv[3];
                    xTFTPData.FILEMODE = "octet";

                    if ((*FT_TFTP->OPEN)(&xTFTPData) != 0) 
                    {
                        printf("can't open tftp\n");
                        break;
                    }
                    
                    /*** Repeat until end of file */
                    while (!(*FT_TFTP->EOFT)()) 
                    {
                        pData = (*FT_TFTP->READ)(&ulLen);
                        if (!pData) 
                        {
                            (*FT_TFTP->CLOSE)();
                        } /* Endif */
                        
                        if (ulDataLen + ulLen < ulBuffLen)
                        {
                            memcpy(&pBuff[ulDataLen], pData, ulLen);
                            ulDataLen += ulLen;                        
                        }
                        else
                        {
                            printf("Buffer too small1\n");
                            (*FT_TFTP->CLOSE)();
                        }

                    } /* Endwhile */

                    /*
                    ** End the file transfer session
                    */
                    if ((*FT_TFTP->CLOSE)() != 0) 
                    {
                        printf("TFTP close error!\n");
                        break;
                    } /* Endif */
                    
                    
                    FTE_CFG_CERT_set(pBuff, ulDataLen);
                }
                else
                {
                    bPrintUsage = TRUE;
                }
                
            }
            break;
            
        default:
            bPrintUsage = TRUE;
        }
        
    }
    
   
    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
        pBuff = NULL;
    }
    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<command>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<command>]\n", pArgv[0]);
            printf("  Commands:\n");
            printf("    show\n");
            printf("        show certificate.\n");
            printf("    load <ip> <file_name>\n");
            printf("        download and save.\n");
            printf("  Parameters:\n");
            printf("      <ip>        = TFTP server IP address.\n");
            printf("      <file_name> = Certificate file name.\n");
        }
    }
    return   xRet;
}

/******************************************************************************
 * Support for Network
 ******************************************************************************/

FTE_RET FTE_CFG_NET_get
(   
    FTE_NET_CFG_PTR _PTR_ ppConfig
)
{
    ASSERT(ppConfig != NULL);
    
    *ppConfig = &_config.xPool.xNetwork;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_NET_copy
(
    FTE_NET_CFG_PTR     pCfgNet
)
{
    FTE_PRODUCT_DESC const *desc = FTE_getProductDescription();

    memcpy(pCfgNet, &_config.xPool.xNetwork, sizeof(FTE_NET_CFG));
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_NET_set(FTE_NET_CFG const *pCfgNet)
{
    FTE_PRODUCT_DESC const *desc = FTE_getProductDescription();

    FTE_CFG_lock();
    memcpy(&_config.xPool.xNetwork, pCfgNet, sizeof(FTE_NET_CFG));
    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_NET_setIP(_ip_address xIP)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xIPData.ip = xIP;
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_NET_setNetmask(_ip_address xNetmask)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xIPData.mask = xNetmask;
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_NET_setGatewayIP(_ip_address xIP)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xIPData.gateway = xIP;
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

#if FTE_SNMPD_SUPPORTED
FTE_UINT32     FTE_CFG_NET_TRAP_count(void)
{
    return  _config.xPool.xNetwork.xSNMP.xTrap.ulCount;
}

FTE_RET   FTE_CFG_NET_TRAP_clear(void)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xSNMP.xTrap.ulCount = 0;
    memset(_config.xPool.xNetwork.xSNMP.xTrap.pList, 0, sizeof(_config.xPool.xNetwork.xSNMP.xTrap.pList));
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

_ip_address FTE_CFG_NET_TRAP_getAt(FTE_UINT32 ulIndex)
{
    if (ulIndex < _config.xPool.xNetwork.xSNMP.xTrap.ulCount)
    {        
        int i;

        for(i = 0 ; i < FTE_NET_SNMP_TRAP_COUNT ; i++)
        {
            if ((_config.xPool.xNetwork.xSNMP.xTrap.pList[i] != 0) && (ulIndex == 0))
            {
                return  _config.xPool.xNetwork.xSNMP.xTrap.pList[i];
            }
            ulIndex--;
        }
    }
    
    return  0;
}

FTE_RET   FTE_CFG_NET_TRAP_addIP(_ip_address xIP)
{
    if (FTE_CFG_NET_TRAP_isExist(xIP))
    {
        return  FTE_RET_OK;
    }
    
    if (_config.xPool.xNetwork.xSNMP.xTrap.ulCount < FTE_NET_SNMP_TRAP_COUNT)
    {        
        FTE_CFG_lock();
        _config.xPool.xNetwork.xSNMP.xTrap.pList[_config.xPool.xNetwork.xSNMP.xTrap.ulCount++] = xIP;
        _config.bPoolModified = TRUE;
        FTE_CFG_unlock();
        
        return  FTE_RET_OK;
    }
    
    return  FTE_RET_ERROR;
}
    
FTE_RET   FTE_CFG_NET_TRAP_delIP(_ip_address xIP)
{
    int i, nIndex = -1;

    for(i = 0 ; i < _config.xPool.xNetwork.xSNMP.xTrap.ulCount ; i++)
    {
        if (_config.xPool.xNetwork.xSNMP.xTrap.pList[i] == xIP)
        {
            nIndex = i;
        }
    }
    
    if (nIndex >= 0)
    {
        FTE_CFG_lock();
        
        for(i = nIndex ; i < _config.xPool.xNetwork.xSNMP.xTrap.ulCount - 1; i++)
        {        
            _config.xPool.xNetwork.xSNMP.xTrap.pList[i] = _config.xPool.xNetwork.xSNMP.xTrap.pList[i+1];
        }
        
        _config.xPool.xNetwork.xSNMP.xTrap.ulCount--;
        _config.xPool.xNetwork.xSNMP.xTrap.pList[_config.xPool.xNetwork.xSNMP.xTrap.ulCount] = 0;
        
        _config.bPoolModified = TRUE;
        FTE_CFG_unlock();
    }
    
    return  FTE_RET_OK;
}
    
FTE_BOOL FTE_CFG_NET_TRAP_isExist(_ip_address nTrapIP)
{
    if ((nTrapIP != 0) && (_config.xPool.xNetwork.xSNMP.xTrap.ulCount != 0))
    {        
        int i;

        for(i = 0 ; i < FTE_NET_SNMP_TRAP_COUNT ; i++)
        {
            if (_config.xPool.xNetwork.xSNMP.xTrap.pList[i] == nTrapIP)
            {
                return  TRUE;
            }
        }
    }
    
    return  FALSE;
}
#endif

/******************************************************************************
 * System Configuration
 ******************************************************************************/

FTE_RET FTE_CFG_SYS_get(FTE_SYS_CONFIG_PTR _PTR_ ppConfig)
{
    ASSERT(ppConfig != NULL);
    
    *ppConfig = &_config.xPool.xSystem;
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_CFG_SYS_set
(
    FTE_SYS_CONFIG const *pConfig
)
{
    FTE_CFG_lock();
    memcpy(&_config.xPool.xSystem, pConfig, sizeof(FTE_SYS_CONFIG));
    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  FTE_RET_OK;
}


FTE_BOOL FTE_CFG_SYS_getSystemMonitor(void)
{
    return  _config.xPool.xSystem.xFlags.bSystemMonitor;
}

FTE_RET   FTE_CFG_SYS_setSystemMonitor
(
    FTE_BOOL    bStart
)
{
    FTE_CFG_lock();
    _config.xPool.xSystem.xFlags.bSystemMonitor = bStart;    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  FTE_RET_OK;
}

FTE_UINT32     FTE_CFG_SYS_getAllowedFailureCount(void)
{
    return  _config.xPool.xSystem.ulAllowFailureCount;
}

FTE_UINT32     FTE_CFG_SYS_getKeepAliveTime(void)
{
    return  _config.xPool.xSystem.ulKeepAliveTime;
}

FTE_RET   FTE_CFG_SYS_setKeepAliveTime(FTE_UINT32 ulTime)
{
    if (ulTime > FTE_SYS_KEEP_ALIVE_TIME_MAX)
    {
        return  FTE_RET_ERROR;
    }
  
    FTE_CFG_lock();
    _config.xPool.xSystem.ulKeepAliveTime = ulTime;    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  FTE_RET_OK;
}

/******************************************************************************
 * Shell Configuration
 ******************************************************************************/

FTE_SHELL_CONFIG_PTR    FTE_CFG_SHELL_get(void)
{
    return  &_config.xPool.xShell;
}

FTE_RET   FTE_CFG_SHELL_set(FTE_SHELL_CONFIG const *pConfig)
{
    FTE_CFG_lock();
    memcpy(&_config.xPool.xShell, pConfig, sizeof(FTE_SHELL_CONFIG));
    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  FTE_RET_OK;
}

void    FTE_CFG_autoSave
(
    FTE_TIMER_ID    xTimerID, 
    FTE_VOID_PTR    pData, 
    MQX_TICK_STRUCT_PTR pTick
)
{
    FTE_BOOL    bChanged = FALSE;
    
    FTE_CFG_isChanged(&bChanged);
    if (bChanged)
    {
        FTE_CFG_save(FALSE);
    }
}

/******************************************************************************
 * Debugging Utils
 ******************************************************************************/

void    FTE_CFG_DBG_setBootTime(void)
{
    FTE_LOG_addSystem(FTE_LOG_SYSTEM_MESSAGE_BOOT);
}

/******************************************************************************
 * Shell command
 ******************************************************************************/
FTE_INT32  FTE_CFG_SHELL_cmd(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[])
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
                FTE_UINT8  pMAC[FTE_MAC_SIZE];
                
                FTE_SYS_getMAC(pMAC);
                printf("<Device Informations>\n");
                printf("%16s : %s\n", "OID", FTE_SYS_getOIDString());
                printf("%16s : %02x:%02x:%02x:%02x:%02x:%02x\n", 
                       "MAC Address", pMAC[0], pMAC[1], pMAC[2],
                       pMAC[3], pMAC[4], pMAC[5]);
            }
            break;
            
        case    2:
            { 
                if (strcmp(pArgv[1], "oid") == 0)
                {
                    printf("%s\n", FTE_SYS_getOIDString());
                }
                else if (strcmp(pArgv[1], "mac") == 0)
                {
                    FTE_UINT8  pMAC[FTE_MAC_SIZE];
                    
                    FTE_SYS_getMAC(pMAC);
                    
                    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", 
                           pMAC[0], pMAC[1], pMAC[2],
                           pMAC[3], pMAC[4], pMAC[5]);
                }
                else if (strcmp(pArgv[1], "save") == 0)
                {
                    IPCFG_IP_ADDRESS_DATA   ip_data ;
                    FTE_NET_CFG_PTR         pConfig;

                    ipcfg_get_ip (0, &ip_data);
                    xRet = FTE_CFG_NET_get(&pConfig);
                    if (xRet != FTE_RET_OK)
                    {
                        break;
                    }
                    
                    if ((pConfig->xIPData.ip != ip_data.ip) || (pConfig->xIPData.mask != ip_data.mask) || (pConfig->xIPData.gateway != ip_data.gateway))
                    {
                        pConfig->xIPData.ip     = ip_data.ip;
                        pConfig->xIPData.mask   = ip_data.mask;
                        pConfig->xIPData.gateway= ip_data.gateway;
                        _config.bPoolModified = TRUE;
                    }
                    
                    if (_config.bPoolModified == FALSE)
                    {
                        if (_config.xPool.ulCRC != FTE_CRC32(0, &_config.xPool.ulTag, sizeof(FTE_CFG_POOL) - sizeof(FTE_UINT32)))
                        {
                            _config.bPoolModified = TRUE;
                        }                    
                    }
                    FTE_CFG_save(FALSE);
                }
                else if (strcmp(pArgv[1], "reset") == 0)
                {
                    FTE_CFG_clear();
                }
            }
            break;
            
        default:
            {
                bPrintUsage = TRUE;
            }
        }
            
    }
    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s [<commands>]\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s [<commands>]\n", pArgv[0]);
            printf("  Commands : \n");
            printf("    auto_reset [ enable | disable ]\n");
            printf("        The processes is restarted when unstable state.\n"); 
        }
    }
    return   xRet;
}

/******************************************************************************
 * Shell command
 ******************************************************************************/
FTE_INT32  FTE_CFG_SHELL_cmdSave(FTE_INT32 nArgc, FTE_CHAR_PTR pArgv[])
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
                IPCFG_IP_ADDRESS_DATA   ip_data ;
                FTE_NET_CFG_PTR         pConfig;

                ipcfg_get_ip (0, &ip_data);
                xRet = FTE_CFG_NET_get(&pConfig);
                if (xRet != FTE_RET_OK)
                {
                    break;
                }
                
                if ((pConfig->xIPData.ip != ip_data.ip) || (pConfig->xIPData.mask != ip_data.mask) || (pConfig->xIPData.gateway != ip_data.gateway))
                {
                    pConfig->xIPData.ip     = ip_data.ip;
                    pConfig->xIPData.mask   = ip_data.mask;
                    pConfig->xIPData.gateway= ip_data.gateway;
                    _config.bPoolModified = TRUE;
                }
                
                if (_config.bPoolModified == FALSE)
                {
                    if (_config.xPool.ulCRC != FTE_CRC32(0, &_config.xPool.ulTag, sizeof(FTE_CFG_POOL) - sizeof(FTE_UINT32)))
                    {
                        _config.bPoolModified = TRUE;
                    }                    
                }
                
                FTE_CFG_save(TRUE);
            }
            break;
        }
    }
    
    if (bPrintUsage || (xRet !=SHELL_EXIT_SUCCESS))
    {
        if (bShortHelp)
        {
            printf ("%s\n", pArgv[0]);
        }
        else
        {
            printf("Usage : %s\n", pArgv[0]);
        }
    }
    return   xRet;
}
