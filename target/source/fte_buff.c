#include "fte_target.h"
#include "fte_buff.h"

/* Local variables */
static LWSEM_STRUCT _lwsem;
static uint_32      _buff_len = 0;
static uint_8 _PTR_ _buff = NULL;

_mqx_uint fte_buff_init(uint_32 size)
{
    _mqx_uint ret;
    
    /* Create the lightweight semaphore */
    ret = _lwsem_create(&_lwsem, 1);
    if (ret != MQX_OK)  
    {
        DEBUG("\nCreating sem failed: 0x%X", ret);
        return  ret;
    }
    
    _buff = (uint_8_ptr)FTE_MEM_allocZero(size);
    if (_buff == NULL)
    {
        DEBUG("\nNot enough memory: Size = %d", size);
        return  MQX_ERROR;
    }
    _buff_len = size;
    
    return  MQX_OK;
}

uint_32 fte_buff_size(void)
{
    return  _buff_len;
}

_mqx_uint fte_buff_lock(pointer _PTR_ buffer)
{
#if 1
    _mqx_uint   ret;

    assert(buffer != NULL);
    
    printf("%s : %08x\n", __func__, buffer);
    ret = _lwsem_wait(&_lwsem);
    if (ret != MQX_OK)
    {  
        DEBUG("\n_lwsem_wait failed");
        return  ret;
    }
    
    _PTR_ buffer = _buff;
#else
    _PTR_ buffer = FTE_MEM_alloc(1024);
#endif
    return  MQX_OK;
}

_mqx_uint fte_buff_unlock(pointer buffer)
{
    if (buffer != NULL)
    {
#if 1
        _mqx_uint   ret;
    
        printf("%s : %08x\n", __func__, buffer);
    
        ret = _lwsem_post(&_lwsem);
        if (ret != MQX_OK)
        {
            DEBUG("\n_lwsem_post failed");
            return  ret;
        }
#else
        FTE_MEM_free(buffer);    
#endif    
    }
    
    return  MQX_OK;
}
