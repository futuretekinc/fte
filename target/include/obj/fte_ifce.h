#ifndef __FTE_IFCE_H__
#define __FTE_IFCE_H__


/*****************************************************************************
 * AD7785-based IFCE Object Structure Description
 *****************************************************************************/
typedef struct _fte_ifce_config_struct
{
    FTE_COMMON_CONFIG   xCommon;
    uint_32             nDevID;
    uint_32             nRegID;
    uint_32             nInterval;
}   FTE_IFCE_CONFIG, _PTR_ FTE_IFCE_CONFIG_PTR;

typedef struct  _fte_ifce_status_struct
{
    FTE_OBJECT_STATUS   xCommon;
    uint_32             hRepeatTimer;
    uint_32             hConvertTimer;
    FTE_OBJECT_PTR      pParent;
}   FTE_IFCE_STATUS, _PTR_ FTE_IFCE_STATUS_PTR;

_mqx_uint fte_ifce_preinit(uint_32 nMaxObjects);

_mqx_uint   fte_ifce_attach(FTE_OBJECT_PTR pObj);
_mqx_uint   fte_ifce_detach(FTE_OBJECT_PTR pObj);

#endif
