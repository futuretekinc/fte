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
#define FTE_OBJ_MAX_COUNT               ((2048 - sizeof(uint_32) * 4) / sizeof(FTE_OBJECT_CONFIG))
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
#define FTE_CFG_CERT_POOL_VERSION       0x20150708

void    FTE_CFG_lock(void);
void    FTE_CFG_unlock(void);
void    _FTE_CFG_auto_save(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr);

FTE_OBJECT_CONFIG_PTR   FTE_CFG_OBJ_create(FTE_OBJECT_CONFIG_PTR pConfig);
FTE_CFG_EVENT_PTR       FTE_CFG_EVENT_create(FTE_CFG_EVENT_PTR pConfig);
_mqx_uint               FTE_CFG_CERT_load(void);
_mqx_uint               FTE_CFG_CERT_save(void);

void                    FTE_CFG_DBG_setBootTime(void);

typedef struct  _FTE_CFG_POOL_STRUCT
{
    uint_32             crc;
    uint_32             tag;
    int_32              nID;
    char                pModel[FTE_MODEL_LENGTH+1];
//    uint_8              pMAC[FTE_MAC_SIZE];
//    uint_8              pOID[FTE_OID_SIZE+1];
    char                location[FTE_LOCATION_MAX_LEN+1];
    
    FTE_SYS_CONFIG      xSystem;
    FTE_SHELL_CONFIG    xShell;
    FTE_NET_CFG         xNetwork;
}   FTE_CFG_POOL, _PTR_ FTE_CFG_POOL_PTR;

typedef struct _FTE_CFG_OBJECT_POOL_STRUCT
{
    uint_32             crc;
    uint_32             tag;
    int_32              nID;

    uint_32             uiObjectCount;
    FTE_OBJECT_CONFIG   pObjects[FTE_OBJ_MAX_COUNT];
} FTE_CFG_OBJECT_POOL, _PTR_ FTE_CFG_OBJECT_POOL_PTR;

typedef struct _FTE_CFG_EVENT_POOL_STRUCT
{
    uint_32             crc;
    uint_32             tag;
    int_32              nID;
    uint_32             uiEventCount;
    FTE_EVENT_CONFIG    pEvents[(0x800 - sizeof(uint_32) * 4) / sizeof(FTE_EVENT_CONFIG)];
}   FTE_CFG_EVENT_POOL, _PTR_ FTE_CFG_EVENT_POOL_PTR;

typedef struct _FTE_CFG_CERT_POOL_HEAD_STRUCT
{
    uint_32             crc;
    uint_32             tag;
    int_32              nID;
    uint_32             ulCertLen;
}   FTE_CFG_CERT_POOL_HEAD, _PTR_ FTE_CFG_CERT_POOL_HEAD_PTR;

typedef struct _FTE_CFG_CERT_POOL_STRUCT
{
    uint_32             crc;
    uint_32             tag;
    int_32              nID;
    uint_32             ulCertLen;
    uint_8              pCert[4096 - sizeof(uint_32)*4];
}   FTE_CFG_CERT_POOL, _PTR_ FTE_CFG_CERT_POOL_PTR;

typedef struct  _FTE_CFG_struct
{
    FTE_CFG_DESC const *    pDESC;
    boolean                 bPoolModified;
    FTE_CFG_POOL            xPool;
    boolean                 bObjectPoolModified;
    FTE_CFG_OBJECT_POOL     xObjectPool;
    boolean                 bEventPoolModified;
    FTE_CFG_EVENT_POOL      xEventPool;
    uint_32                 ulIndex;
}   FTE_CONFIG, _PTR_ FTE_CFG_PTR;

static FTE_CONFIG   _config = { .pDESC = NULL, };
static LWSEM_STRUCT _xLWSEM; 

_mqx_uint   FTE_CFG_init(FTE_CFG_DESC const *desc)
{
    _mqx_int                i;
    boolean                 bIgnore = TRUE;
    MQX_TICK_STRUCT         xTicks, xDTicks;
    FTE_CFG_POOL_PTR        pPool = NULL;
    FTE_CFG_OBJECT_POOL_PTR pObjectPool = NULL;
    FTE_CFG_EVENT_POOL_PTR  pEventPool = NULL;
    FTE_CFG_CERT_POOL_PTR   pCertPool = NULL;
    void _PTR_              pBuff = NULL;
    
    assert(_config.pDESC == NULL && desc != NULL);
    
    if (_lwsem_create(&_xLWSEM, 1) != MQX_OK)
    {
        return  MQX_ERROR;
    }
    
    pBuff = (void _PTR_)FTE_MEM_allocZero(4096);
    if (pBuff == NULL)
    {
        return  MQX_ERROR;
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
        read(fp, (char_ptr)pPool, sizeof(FTE_CFG_POOL));
        fclose(fp);
        
        if ((pPool->crc != fte_crc32(0, (char_ptr)&pPool->tag, sizeof(FTE_CFG_POOL) - sizeof(uint_32))) ||
            (pPool->tag != FTE_CFG_POOL_VERSION) || 
            (pPool->nID > (uint_32)MAX_INT_32))
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
            int_32          nLen;
            MQX_FILE_PTR    fp;    
            
            /* Open the flash device */
            fp = fopen(desc->pMTDs[nMTD], NULL);
            if (fp == NULL) 
            {
                continue;
            }

            ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
            nLen = read(fp, (char_ptr)pObjectPool, sizeof(FTE_CFG_OBJECT_POOL));
            fclose(fp);
            
            if ((nLen != sizeof(FTE_CFG_OBJECT_POOL)) ||
                (pObjectPool->crc != fte_crc32(0, (char_ptr)&pObjectPool->tag, sizeof(FTE_CFG_OBJECT_POOL) - sizeof(uint_32))) ||
                (pObjectPool->tag != FTE_CFG_OBJECT_POOL_VERSION) || 
                (pObjectPool->nID > (uint_32)MAX_INT_32))
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
            read(fp, (char_ptr)pEventPool, sizeof(FTE_CFG_EVENT_POOL));
            fclose(fp);

            if ((pEventPool->crc != fte_crc32(0, (char_ptr)&pEventPool->tag, sizeof(FTE_CFG_EVENT_POOL) - sizeof(uint_32))) ||
                (pEventPool->tag != FTE_CFG_EVENT_POOL_VERSION) || 
                (pEventPool->nID > (uint_32)MAX_INT_32))
            {
                continue;
            }
                
            if (pEventPool->nID > _config.xEventPool.nID)
            {
                memcpy(&_config.xEventPool, pEventPool, sizeof(FTE_CFG_EVENT_POOL));
            }
        }     
    }
    
    if (_config.xPool.nID == 0)
    {
        _config.xPool.tag    = FTE_CFG_POOL_VERSION;
        strcpy(_config.xPool.pModel, FTE_MODEL);
        
        FTE_CFG_SYS_set(desc->pSystem);
        FTE_CFG_SHELL_set(desc->pShell);
        FTE_CFG_NET_set(desc->pNetwork);        
    }
    
    if (_config.xObjectPool.nID == 0)
    {
        _config.xObjectPool.tag    = FTE_CFG_OBJECT_POOL_VERSION;
        
        for( i = 0 ; i < desc->nObjects ; i++)
        {
            FTE_CFG_OBJ_create(desc->pObjects[i]);
        }
    } 
    
    if (_config.xEventPool.nID == 0)
    {
        _config.xEventPool.tag    = FTE_CFG_EVENT_POOL_VERSION;
        
        for( i = 0 ; i < desc->nEvents ; i++)
        {
            FTE_CFG_EVENT_create(desc->pEvents[i]);
        }
    } 

    FTE_LOG_init();
    FTE_CFG_DBG_setBootTime();
    
    FTE_CFG_save(FALSE);
    
    FTE_MEM_free(pPool);  
    FTE_MEM_free(pObjectPool);
    FTE_MEM_free(pEventPool);    
    FTE_MEM_free(pBuff);    

    _time_init_ticks(&xDTicks, 0);
    _time_add_msec_to_ticks(&xDTicks, _config.xPool.xSystem.ulAutoSaveInterval);
    _time_get_elapsed_ticks(&xTicks);
    _time_add_sec_to_ticks(&xTicks, 1);
    
    _timer_start_periodic_at_ticks(_FTE_CFG_auto_save, NULL, TIMER_ELAPSED_TIME_MODE, &xTicks, &xDTicks);

    return  MQX_OK;
    
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
        
    if (pCertPool != NULL)
    {
        FTE_MEM_free(pCertPool);
    }
        
    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
    }
    
    return  MQX_ERROR;
}


_mqx_uint FTE_CFG_save(boolean force)
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
            _config.xPool.crc = fte_crc32(0, (pointer)&_config.xPool.tag, sizeof(FTE_CFG_POOL) - sizeof(uint_32));   
        
            int nMTD = _config.xPool.nID & 0x01;
            
            /* Open the flash device */
            fp = fopen(_config.pDESC->pMTDs[nMTD], NULL);
            if (fp != NULL) 
            {
                ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
                
                if (sizeof(FTE_CFG_POOL) != write(fp, (pointer)&_config.xPool, sizeof(FTE_CFG_POOL)))
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
            _config.xObjectPool.crc = fte_crc32(0, (pointer)&_config.xObjectPool.tag, sizeof(FTE_CFG_OBJECT_POOL) - sizeof(uint_32));   
        
            int nMTD = _config.xObjectPool.nID & 0x01;
            
            /* Open the flash device */
            fp = fopen(_config.pDESC->pMTDs[nMTD + 2], NULL);
            if (fp != NULL) 
            {
                ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
                
                if (sizeof(FTE_CFG_OBJECT_POOL) != write(fp, (pointer)&_config.xObjectPool, sizeof(FTE_CFG_OBJECT_POOL)))
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
            _config.xEventPool.crc = fte_crc32(0, (pointer)&_config.xEventPool.tag, sizeof(FTE_CFG_EVENT_POOL) - sizeof(uint_32));   
        
            int nMTD = _config.xEventPool.nID & 0x01;

            fp = fopen(_config.pDESC->pMTDs[nMTD + 4], NULL);
            if (fp != NULL) 
            {
                ioctl(fp, FLASH_IOCTL_ENABLE_SECTOR_CACHE, NULL);
                
                if (sizeof(FTE_CFG_EVENT_POOL) != write(fp, (pointer)&_config.xEventPool, sizeof(FTE_CFG_EVENT_POOL)))
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


    goto success;
    
error:
    FTE_CFG_unlock();
    
    return  MQX_ERROR;

success:
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

_mqx_uint FTE_CFG_clear(void) 
{
    _mqx_int        i;

    assert(_config.pDESC != NULL);
    
    FTE_CFG_lock();
    memset(&_config.xPool, 0, sizeof(FTE_CFG_POOL));
    memset(&_config.xObjectPool, 0, sizeof(FTE_CFG_OBJECT_POOL));
    memset(&_config.xEventPool, 0, sizeof(FTE_CFG_EVENT_POOL));

    _config.xPool.tag    = FTE_CFG_POOL_VERSION;    
    FTE_CFG_unlock();
    
    FTE_CFG_SYS_set(_config.pDESC->pSystem);
    FTE_CFG_SHELL_set(_config.pDESC->pShell);
    FTE_CFG_NET_set(_config.pDESC->pNetwork);        
   
    _config.xObjectPool.tag    = FTE_CFG_OBJECT_POOL_VERSION;    
    for( i = 0 ; i < _config.pDESC->nObjects ; i++)
    {
        FTE_CFG_OBJ_create(_config.pDESC->pObjects[i]);
    }
    
    FTE_CFG_lock();
    _config.xEventPool.tag    = FTE_CFG_EVENT_POOL_VERSION;
    FTE_CFG_unlock();
      
    for( i = 0 ; i < _config.pDESC->nEvents ; i++)
    {
        FTE_CFG_EVENT_create(_config.pDESC->pEvents[i]);
    }

    FTE_CFG_save(TRUE);
    
    return  MQX_OK;
}

void    FTE_CFG_lock(void)
{
    _int_disable();
    _lwsem_wait(&_xLWSEM);
}

void    FTE_CFG_unlock(void)
{
    _lwsem_post(&_xLWSEM);
    _int_enable();
}

boolean FTE_CFG_isChanged(void)
{
    return  _config.bPoolModified;
}

_mqx_uint   FTE_CFG_OBJ_save(FTE_OBJECT_PTR pObj)
{
    FTE_OBJECT_CONFIG_PTR pConfig = FTE_CFG_OBJ_get(pObj->pConfig->xCommon.nID);
    if (pConfig == NULL)
    {
        return  MQX_ERROR;
    }

    FTE_CFG_lock();
    memcpy(pConfig, pObj->pConfig, sizeof(FTE_OBJECT_CONFIG));
    _config.bObjectPoolModified  = TRUE;
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

_mqx_uint   FTE_CFG_getLocation(char_ptr pLocation, uint_32 nLen)
{
    if (nLen < strlen(_config.xPool.location))
    {
        return  MQX_INVALID_SIZE;
    }
    
    strncpy(pLocation, _config.xPool.location, nLen);
    
    return  MQX_OK;
}
 
_mqx_uint   FTE_CFG_setLocation(char_ptr pLocation, uint_32 nLen)
{
    if (nLen > FTE_LOCATION_MAX_LEN)
    {
        return  MQX_INVALID_SIZE;
    }
    
    FTE_CFG_lock();
    strncpy(_config.xPool.location, pLocation, nLen);
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

uint_32 FTE_CFG_objects_count(void)
{
    return  _config.xObjectPool.uiObjectCount;
}

pointer FTE_CFG_OBJ_get(uint_32 oid)
{
    if (_config.pDESC != NULL && _config.xObjectPool.uiObjectCount != 0)
    {
        _mqx_uint   i;
    
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

pointer             FTE_CFG_OBJ_getAt(uint_32 oid, uint_32 ulMask, uint_32 ulIdx)
{
    if ((_config.pDESC != NULL) && (ulIdx < _config.xObjectPool.uiObjectCount))
    {
        if (ulMask == 0)
        {
            return  &_config.xObjectPool.pObjects[ulIdx];
        }
        else
        {
            _mqx_uint   i;
        
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

FTE_OBJECT_CONFIG_PTR FTE_CFG_OBJ_alloc(uint_32 nID)
{    
    if (_config.pDESC != NULL && _config.xObjectPool.uiObjectCount != FTE_OBJ_MAX_COUNT)
    {
        _mqx_uint   i;
        
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

_mqx_uint   FTE_CFG_OBJ_free(uint_32 oid)
{
    if (_config.pDESC != NULL && _config.xObjectPool.uiObjectCount != 0)
    {
        _mqx_uint   i;
         
        for(i = 0 ; i < FTE_OBJ_MAX_COUNT ; i++)
        {
            if (_config.xObjectPool.pObjects[i].xCommon.nID == oid)
            {
                memset(&_config.xObjectPool.pObjects[i], 0, sizeof(FTE_OBJECT_CONFIG));
                _config.xObjectPool.uiObjectCount--;

                return  MQX_OK;
            }
        }
    }
    
    return  MQX_ERROR;
}

FTE_OBJECT_CONFIG_PTR FTE_CFG_OBJ_create(FTE_OBJECT_CONFIG_PTR pConfig)
{
    FTE_OBJECT_CONFIG_PTR pObject = FTE_CFG_OBJ_alloc(pConfig->xCommon.nID);
    if (pObject != NULL)
    {
        int     i;
        uint_32 ulGroupID = 0;
        uint_32 ulCount = FTE_CFG_OBJ_count(pConfig->xCommon.nID & FTE_OBJ_CLASS_MASK, FTE_OBJ_CLASS_MASK);
        
        memcpy(pObject, pConfig, sizeof(FTE_OBJECT_CONFIG));
        pObject->xCommon.nID = (pConfig->xCommon.nID & FTE_OBJ_TYPE_MASK) | (ulCount + 1);
    
        ulGroupID = (pObject->xCommon.nID >> 16) & 0xFF;
        
        for(i = 0 ; i < pObject->xCommon.ulChild ; i++)
        {
            FTE_OBJECT_CONFIG_PTR pChild = FTE_CFG_OBJ_alloc(pObject->xCommon.pChild[i]->xCommon.nID);
            if (pChild != NULL)
            {
                ulCount = FTE_CFG_OBJ_count(pObject->xCommon.pChild[i]->xCommon.nID & FTE_OBJ_CLASS_MASK, FTE_OBJ_CLASS_MASK);
                
                memcpy(pChild, pObject->xCommon.pChild[i], sizeof(FTE_OBJECT_CONFIG));
                ((FTE_IFCE_CONFIG_PTR)pChild)->nDevID = pObject->xCommon.nID;
                pChild->xCommon.nID = (pObject->xCommon.pChild[i]->xCommon.nID & FTE_OBJ_TYPE_MASK) | (ulGroupID << 8) | (ulCount + 1);
            }
        }

        _config.bObjectPoolModified = TRUE;
    }
    
    return  pObject;
}

uint_32 FTE_CFG_OBJ_count(uint_32 ulType, uint_32 ulMask)
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
            uint_32 ulCount = 0;
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

pointer FTE_CFG_OBJ_getFirst(void)
{
    if ((_config.pDESC != NULL) && (_config.xObjectPool.uiObjectCount != 0))
    {
        _config.ulIndex = 0;
        return  &_config.xObjectPool.pObjects[_config.ulIndex];
    }
    
    return  NULL;
}

pointer FTE_CFG_OBJ_getNext(void)
{
    if ((_config.pDESC != NULL) && (++_config.ulIndex < _config.xObjectPool.uiObjectCount))
    {
        {
            return  &_config.xObjectPool.pObjects[_config.ulIndex];
        }
    }
    
    return  NULL;
}


FTE_CFG_EVENT_PTR FTE_CFG_EVENT_alloc(uint_32  ulOID)
{    
    if (_config.pDESC != NULL && _config.xEventPool.uiEventCount != FTE_EVENT_MAX_COUNT)
    {
        _mqx_uint   i;
        
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

_mqx_uint   FTE_CFG_EVENT_free(FTE_CFG_EVENT_PTR pConfig)
{
    if (_config.pDESC != NULL && _config.xEventPool.uiEventCount != 0)
    {
        _mqx_uint   i;
         
        for(i = 0 ; i < FTE_EVENT_MAX_COUNT ; i++)
        {
            if (&_config.xEventPool.pEvents[i] == pConfig)
            {
                memset(&_config.xEventPool.pEvents[i], 0, sizeof(FTE_EVENT_CONFIG));
                _config.xEventPool.uiEventCount--;

                return  MQX_OK;
            }
        }
    }
    
    return  MQX_ERROR;
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

uint_32 FTE_CFG_EVENT_count(void)
{
    if (_config.pDESC != NULL)
    {
        return  _config.xEventPool.uiEventCount;
    }
    
    return  0;
}

uint_32 FTE_CFG_EVENT_getAt(uint_32 ulIndex, FTE_CFG_EVENT_PTR _PTR_ ppConfig)
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
                    return  MQX_OK;
                }
                
                ulIndex --;
            }
        }
    }
    
    return  MQX_ERROR;
}

pointer FTE_CFG_EVENT_getFirst(void)
{
    if ((_config.pDESC != NULL) && (_config.xEventPool.uiEventCount != 0))
    {
        _config.ulIndex = 0;
        return  &_config.xEventPool.pEvents[_config.ulIndex];
    }
    
    return  NULL;
}

pointer FTE_CFG_EVENT_getNext(void)
{
    if ((_config.pDESC != NULL) && (++_config.ulIndex < _config.xEventPool.uiEventCount))
    {
        {
            return  &_config.xEventPool.pEvents[_config.ulIndex];
        }
    }
    
    return  NULL;
}


/******************************************************************************
 * Support for Certificate
 ******************************************************************************/
static FTE_CFG_CERT_POOL_PTR pCertPool = NULL;

boolean FTE_CFG_CERT_valid(void)
{
    if (pCertPool == NULL)
    {
        if (FTE_CFG_CERT_load() != MQX_OK)
        {
            return  FALSE;
        }
    }
    
    return  TRUE;
}

uint_32 FTE_CFG_CERT_size(void)
{
    if (pCertPool == NULL)
    {
        if (FTE_CFG_CERT_load() != MQX_OK)
        {
            return  0;
        }
    }

    return  pCertPool->ulCertLen;
}

uint_32 FTE_CFG_CERT_get(void _PTR_ pBuff, uint_32 ulBuffLen)
{
    if (pCertPool == NULL)
    {
        if (FTE_CFG_CERT_load() != MQX_OK)
        {
            return  0;
        }
    }
    
    if (ulBuffLen < pCertPool->ulCertLen)
    {
        return  0;
    }
    
    memcpy(pBuff, pCertPool->pCert, pCertPool->ulCertLen);
    
    return  pCertPool->ulCertLen;
}

_mqx_uint   FTE_CFG_CERT_load(void)
{
    FTE_CFG_CERT_POOL_PTR pTempCertPools = (FTE_CFG_CERT_POOL_PTR)0x7E000;
    
    if (_config.pDESC == NULL)
    {
        return  MQX_ERROR;
    }
    
    if (pCertPool != NULL)
    {
        return  MQX_OK;
    }
    
    for(int i = 0 ; i < 1 ; i++)
    {
        if ((pTempCertPools[i].ulCertLen > sizeof(pTempCertPools[i].pCert)) ||
            (pTempCertPools[i].tag != FTE_CFG_CERT_POOL_VERSION) ||
            (pTempCertPools[i].crc != fte_crc32(0, (char_ptr)&pTempCertPools[i].tag, sizeof(FTE_CFG_CERT_POOL_HEAD) - sizeof(uint_32) + pTempCertPools[i].ulCertLen)) ||
            (pTempCertPools[i].nID > (uint_32)MAX_INT_32))
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
        return  MQX_ERROR;
    }
    
    return  MQX_OK;
}


_mqx_uint   FTE_CFG_CERT_set(void _PTR_ pCert, uint_32 ulCertLen)
{
    FTE_CFG_CERT_POOL_HEAD  xCertHead;
    
    if (pCertPool == NULL)
    {
        xCertHead.nID = 0;
    }
    else
    {
        xCertHead.nID = pCertPool->nID;
    }
    xCertHead.tag = FTE_CFG_CERT_POOL_VERSION;
        
    for(int i = 0 ; i < 2 ; i++)
    {
        MQX_FILE_PTR    fp;
        
        xCertHead.nID++;
        xCertHead.crc = fte_crc32(0, (pointer)&xCertHead.tag, sizeof(FTE_CFG_CERT_POOL_HEAD) - sizeof(uint_32));   
        xCertHead.crc = fte_crc32(xCertHead.crc, pCert, ulCertLen);
        
        char    pFileName[32];
            
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

    return  MQX_OK;
}

/******************************************************************************
 * CERT command
 ******************************************************************************/
int_32  FTE_CFG_CERT_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean     print_usage, shorthelp = FALSE;
    int_32      return_code = SHELL_EXIT_SUCCESS;
    uint_32     ulBuffLen = 4096;
    uint_8_ptr  pBuff = NULL;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    pBuff = (uint_8_ptr)FTE_MEM_alloc(ulBuffLen);
    if (pBuff == NULL)
    {
        printf("Not enough memory!\n");
        return  SHELL_EXIT_ERROR;
    }
    
    if (!print_usage)
    {
        switch(argc)
        {
        case    2:
            {
                uint_32 ulLen;
               
                if (strcmp(argv[1], "show") == 0)
                {
                   ulLen = FTE_CFG_CERT_get(pBuff, ulBuffLen);
                   if (ulLen == 0)
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
                uchar_ptr           pData;
                uint_32             ulDataLen = 0;
                uint_32             ulLen;
                _ip_address         xServerIP = 0;
                TFTP_DATA_STRUCT    xTFTPData;

                if (strcmp(argv[1], "load") == 0)
                {
                    if (! Shell_parse_ip_address (argv[2], &xServerIP))
                    {
                        printf ("Error in parameter, invalid ip address!\n");
                        return_code = SHELL_EXIT_ERROR;
                        break;
                    }
                    
                    xTFTPData.SERVER   = xServerIP;
                    xTFTPData.FILENAME = argv[3];
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
                    print_usage = TRUE;
                }
                
            }
            break;
            
        default:
            print_usage = TRUE;
        }
        
    }
    
   
    if (pBuff != NULL)
    {
        FTE_MEM_free(pBuff);
        pBuff = NULL;
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<command>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<command>]\n", argv[0]);
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
    return   return_code;
}

/******************************************************************************
 * Support for Network
 ******************************************************************************/

FTE_NET_CFG_PTR FTE_CFG_NET_get(void)
{
    return  &_config.xPool.xNetwork;
}

_mqx_uint   FTE_CFG_NET_copy(FTE_NET_CFG_PTR pCfgNet)
{
    FTE_PRODUCT_DESC const *desc = fte_get_product_desc();

    memcpy(pCfgNet, &_config.xPool.xNetwork, sizeof(FTE_NET_CFG));
    return  MQX_OK;
}

_mqx_uint   FTE_CFG_NET_set(FTE_NET_CFG const *pCfgNet)
{
    FTE_PRODUCT_DESC const *desc = fte_get_product_desc();

    FTE_CFG_lock();
    memcpy(&_config.xPool.xNetwork, pCfgNet, sizeof(FTE_NET_CFG));
    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  MQX_OK;
}

_mqx_uint   FTE_CFG_NET_setIP(_ip_address xIP)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xIPData.ip = xIP;
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

_mqx_uint   FTE_CFG_NET_setNetmask(_ip_address xNetmask)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xIPData.mask = xNetmask;
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

_mqx_uint   FTE_CFG_NET_setGatewayIP(_ip_address xIP)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xIPData.gateway = xIP;
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

#if FTE_SNMPD_SUPPORTED
uint_32     FTE_CFG_NET_TRAP_count(void)
{
    return  _config.xPool.xNetwork.xSNMP.xTrap.ulCount;
}

_mqx_uint   FTE_CFG_NET_TRAP_clear(void)
{
    FTE_CFG_lock();
    _config.xPool.xNetwork.xSNMP.xTrap.ulCount = 0;
    memset(_config.xPool.xNetwork.xSNMP.xTrap.pList, 0, sizeof(_config.xPool.xNetwork.xSNMP.xTrap.pList));
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

_ip_address FTE_CFG_NET_TRAP_getAt(uint_32 ulIndex)
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

_mqx_uint   FTE_CFG_NET_TRAP_addIP(_ip_address xIP)
{
    if (FTE_CFG_NET_TRAP_isExist(xIP))
    {
        return  MQX_OK;
    }
    
    if (_config.xPool.xNetwork.xSNMP.xTrap.ulCount < FTE_NET_SNMP_TRAP_COUNT)
    {        
        FTE_CFG_lock();
        _config.xPool.xNetwork.xSNMP.xTrap.pList[_config.xPool.xNetwork.xSNMP.xTrap.ulCount++] = xIP;
        _config.bPoolModified = TRUE;
        FTE_CFG_unlock();
        
        return  MQX_OK;
    }
    
    return  MQX_ERROR;
}
    
_mqx_uint   FTE_CFG_NET_TRAP_delIP(_ip_address xIP)
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
    
    return  MQX_OK;
}
    
boolean FTE_CFG_NET_TRAP_isExist(_ip_address nTrapIP)
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

FTE_SYS_CONFIG_PTR  FTE_CFG_SYS_get(void)
{
    return  &_config.xPool.xSystem;
}

_mqx_uint   FTE_CFG_SYS_set(FTE_SYS_CONFIG const *pConfig)
{
    FTE_CFG_lock();
    memcpy(&_config.xPool.xSystem, pConfig, sizeof(FTE_SYS_CONFIG));
    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  MQX_OK;
}


boolean FTE_CFG_SYS_getSystemMonitor(void)
{
    return  _config.xPool.xSystem.xFlags.bSystemMonitor;
}

_mqx_uint   FTE_CFG_SYS_setSystemMonitor(boolean bStart)
{
    FTE_CFG_lock();
    _config.xPool.xSystem.xFlags.bSystemMonitor = bStart;    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  MQX_OK;
}

uint_32     FTE_CFG_SYS_getKeepAliveTime(void)
{
    return  _config.xPool.xSystem.ulKeepAliveTime;
}

_mqx_uint   FTE_CFG_SYS_setKeepAliveTime(uint_32 ulTime)
{
    if (ulTime > FTE_SYS_KEEP_ALIVE_TIME_MAX)
    {
        return  MQX_ERROR;
    }
  
    FTE_CFG_lock();
    _config.xPool.xSystem.ulKeepAliveTime = ulTime;    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();
    
    return  MQX_OK;
}

/******************************************************************************
 * Shell Configuration
 ******************************************************************************/

FTE_SHELL_CONFIG_PTR    FTE_CFG_SHELL_get(void)
{
    return  &_config.xPool.xShell;
}

_mqx_uint   FTE_CFG_SHELL_set(FTE_SHELL_CONFIG const *pConfig)
{
    FTE_CFG_lock();
    memcpy(&_config.xPool.xShell, pConfig, sizeof(FTE_SHELL_CONFIG));
    
    _config.bPoolModified = TRUE;
    FTE_CFG_unlock();

    return  MQX_OK;
}

static void _FTE_CFG_auto_save(_timer_id id, pointer data_ptr, MQX_TICK_STRUCT_PTR tick_ptr)
{
    if (FTE_CFG_isChanged())
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
int_32  FTE_CFG_SHELL_cmd(int_32 argc, char_ptr argv[])
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
                uint_8  pMAC[FTE_MAC_SIZE];
                
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
                if (strcmp(argv[1], "oid") == 0)
                {
                    printf("%s\n", FTE_SYS_getOIDString());
                }
                else if (strcmp(argv[1], "mac") == 0)
                {
                    uint_8  pMAC[FTE_MAC_SIZE];
                    
                    FTE_SYS_getMAC(pMAC);
                    
                    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", 
                           pMAC[0], pMAC[1], pMAC[2],
                           pMAC[3], pMAC[4], pMAC[5]);
                }
                else if (strcmp(argv[1], "save") == 0)
                {
                    IPCFG_IP_ADDRESS_DATA   ip_data ;
                    FTE_NET_CFG_PTR         pConfig;

                    ipcfg_get_ip (0, &ip_data);
                    pConfig = FTE_CFG_NET_get();
                    
                    if ((pConfig->xIPData.ip != ip_data.ip) || (pConfig->xIPData.mask != ip_data.mask) || (pConfig->xIPData.gateway != ip_data.gateway))
                    {
                        pConfig->xIPData.ip     = ip_data.ip;
                        pConfig->xIPData.mask   = ip_data.mask;
                        pConfig->xIPData.gateway= ip_data.gateway;
                        _config.bPoolModified = TRUE;
                    }
                    
                    if (_config.bPoolModified == FALSE)
                    {
                        if (_config.xPool.crc != fte_crc32(0, (pointer)&_config.xPool.tag, sizeof(FTE_CFG_POOL) - sizeof(uint_32)))
                        {
                            _config.bPoolModified = TRUE;
                        }                    
                    }
                    FTE_CFG_save(FALSE);
                }
                else if (strcmp(argv[1], "reset") == 0)
                {
                    FTE_CFG_clear();
                }
            }
            break;
            
        default:
            {
                print_usage = TRUE;
            }
        }
            
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s [<commands>]\n", argv[0]);
        }
        else
        {
            printf("Usage : %s [<commands>]\n", argv[0]);
            printf("  Commands : \n");
            printf("    auto_reset [ enable | disable ]\n");
            printf("        The processes is restarted when unstable state.\n"); 
        }
    }
    return   return_code;
}

/******************************************************************************
 * Shell command
 ******************************************************************************/
int_32  FTE_CFG_SHELL_cmdSave(int_32 argc, char_ptr argv[])
{
    boolean              print_usage, shorthelp = FALSE;
    int_32               return_code = SHELL_EXIT_SUCCESS;
    
    print_usage = Shell_check_help_request (argc, argv, &shorthelp);

    if (!print_usage)
    {
        switch(argc)
        {
        case    1:
            {
                IPCFG_IP_ADDRESS_DATA   ip_data ;
                FTE_NET_CFG_PTR         pConfig;

                ipcfg_get_ip (0, &ip_data);
                pConfig = FTE_CFG_NET_get();
                
                if ((pConfig->xIPData.ip != ip_data.ip) || (pConfig->xIPData.mask != ip_data.mask) || (pConfig->xIPData.gateway != ip_data.gateway))
                {
                    pConfig->xIPData.ip     = ip_data.ip;
                    pConfig->xIPData.mask   = ip_data.mask;
                    pConfig->xIPData.gateway= ip_data.gateway;
                    _config.bPoolModified = TRUE;
                }
                
                if (_config.bPoolModified == FALSE)
                {
                    if (_config.xPool.crc != fte_crc32(0, (pointer)&_config.xPool.tag, sizeof(FTE_CFG_POOL) - sizeof(uint_32)))
                    {
                        _config.bPoolModified = TRUE;
                    }                    
                }
                
                FTE_CFG_save(TRUE);
            }
            break;
        }
    }
    
    if (print_usage || (return_code !=SHELL_EXIT_SUCCESS))
    {
        if (shorthelp)
        {
            printf ("%s\n", argv[0]);
        }
        else
        {
            printf("Usage : %s\n", argv[0]);
        }
    }
    return   return_code;
}
