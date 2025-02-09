#include "fte_target.h"
#include <message.h>
#include "sys/FTE_SYS_MSG.h"

#ifndef FTE_SYS_MESSAGE_MAX
#define FTE_SYS_MESSAGE_MAX 10
#endif

static  _queue_id   _sys_msg_qid;
static  _pool_id    _sys_msg_pool;

FTE_RET   FTE_SYS_MSGQ_init(void)
{
    /* open a message queue */
    _sys_msg_qid = _msgq_open(MSGQ_FREE_QUEUE, 0);   
    if (_sys_msg_qid == 0) 
    {
        printf("\nCould not open the server message queue\n");
        return  FTE_RET_ERROR;
    }

    /* create a message pool */   
    _sys_msg_pool = _msgpool_create(sizeof(FTE_SYS_MESSAGE), FTE_SYS_MESSAGE_MAX, 0, 0);      
    if (_sys_msg_pool== MSGPOOL_NULL_POOL_ID) 
    {
        printf("\nCount not create a message pool\n");
        return  FTE_RET_ERROR;
    }

    return  FTE_RET_OK;    
}

FTE_RET   FTE_SYS_MSGQ_final(void)
{
    if (_sys_msg_pool != MSGPOOL_NULL_POOL_ID)
    {
        _msgpool_destroy(_sys_msg_pool);
        _sys_msg_pool = MSGPOOL_NULL_POOL_ID;
    }
    
    if (_sys_msg_qid != 0)
    {
        _msgq_close(_sys_msg_qid);
        _sys_msg_qid = 0;
    }
    
    return  FTE_RET_OK;
}

FTE_VOID_PTR FTE_SYS_MSG_receive(void)
{
    if (_sys_msg_qid == 0)
    {
        return  NULL;
    }
    
    return  _msgq_receive(_sys_msg_qid, 1);
}

FTE_RET   FTE_SYS_MSG_send(void)
{
    FTE_SYS_MESSAGE_PTR pMsg = (FTE_SYS_MESSAGE_PTR)_msg_alloc(_sys_msg_pool);
    if (pMsg == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    pMsg->HEADER.SOURCE_QID = _sys_msg_qid;      
    pMsg->HEADER.TARGET_QID = _sys_msg_qid;
    pMsg->HEADER.SIZE       = sizeof(FTE_SYS_MESSAGE);
    
    if (_msgq_send(pMsg) != TRUE) 
    {
        printf("\nCould not send a message\n");
        return  FTE_RET_ERROR;
    }
        
    return  FTE_RET_OK;
}
      
FTE_VOID_PTR FTE_SYS_MSG_alloc(void)
{
    return  _msg_alloc(_sys_msg_pool);
}

void FTE_SYS_MSG_free(FTE_VOID_PTR pMsg)
{
    _msg_free(pMsg);
}
