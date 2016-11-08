#ifndef __FTE_SYS_MSG_H__
#define __FTE_SYS_MSG_H__

typedef struct
{
   MESSAGE_HEADER_STRUCT    HEADER;
   FTE_UINT32               DATA;
}   FTE_SYS_MESSAGE, _PTR_ FTE_SYS_MESSAGE_PTR;

FTE_RET FTE_SYS_MSGQ_init(void);
FTE_RET FTE_SYS_MSGQ_final(void);

FTE_VOID_PTR    FTE_SYS_MSG_receive(void);
FTE_RET FTE_SYS_MSG_send(void);

FTE_VOID_PTR    FTE_SYS_MSG_alloc(void);
void    FTE_SYS_MSG_free(FTE_VOID_PTR pMsg);

#endif
