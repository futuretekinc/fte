#ifndef __FTE_SYS_MSG_H__
#define __FTE_SYS_MSG_H__

typedef struct
{
   MESSAGE_HEADER_STRUCT    HEADER;
   uint_32                  DATA;
}   FTE_SYS_MESSAGE, _PTR_ FTE_SYS_MESSAGE_PTR;

_mqx_uint   fte_sys_msg_q_init(void);
_mqx_uint   fte_sys_msg_q_final(void);

pointer     fte_sys_msg_q_receive(void);
_mqx_uint   fte_sys_msg_send(void);

pointer     fte_sys_msg_alloc(void);
void        fte_sys_msg_free(pointer pMsg);

#endif
