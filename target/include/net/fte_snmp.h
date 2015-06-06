#ifndef __FTE_SNMP_H__
#define __FTE_SNMP_H__

/**HEADER********************************************************************
* 
* Copyright (c) 2014 FutureTek Inc.;
* All Rights Reserved
*
*************************************************************************** 
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* $FileName: fte_snmp.h$
* $Version : 1.0.0.0$
* $Date    : Jun-18-2014$
*
* Comments:
*
*
*END************************************************************************/
typedef enum    _FTE_NET_SNMP_TRAP_TYPE
{
    FTE_NET_SNMP_TRAP_TYPE_ALERT,
    FTE_NET_SNMP_TRAP_TYPE_DISCOVERY
}   FTE_NET_SNMP_TRAP_TYPE, _PTR_ FTE_NET_SNMP_TRAP_TYPE_PTR;

typedef enum  _fte_snmp_trap_alert_level
{
    FTE_ALERT_INFO      = 0,
    FTE_ALERT_NORMAL    = 1,
    FTE_ALERT_WARNING   = 2,
    FTE_ALERT_CRITICAL  = 3    
}   FTE_ALERT_LEVEL;

typedef struct  FTE_SNMP_CFG_STRUCT
{
    boolean         bEnable;
    struct
    {
        uint_32     ulCount;
        _ip_address pList[FTE_NET_SNMP_TRAP_COUNT];    
        char        pCommunity[FTE_NET_SNMP_COMMUNITY_LENGTH + 1];
    }   xTrap;
}   FTE_SNMP_CFG, _PTR_ FTE_SNMP_CFG_PTR;

_mqx_uint   FTE_SNMPD_init(FTE_SNMP_CFG_PTR pConfig);

_mqx_uint   FTE_SNMPD_TRAP_add(_ip_address target, boolean bStatic);
_mqx_uint   FTE_SNMPD_TRAP_del(_ip_address target);
void        FTE_SNMPD_TRAP_processing(void);
_mqx_uint   FTE_SNMPD_TRAP_sendAlert(uint_32 nOID, boolean bOccurred);
_mqx_uint   FTE_SNMPD_TRAP_discovery(_ip_address xHostIP);

int_32      FTE_SNMPD_SHELL_cmd(int_32 argc, char_ptr argv[] );

#endif
