#include "fte_target.h"
#include "fte_config.h"
#include "fte_net.h"
#include "fte_time.h"
#include "fte_mbtcp.h"

/*TASK*-----------------------------------------------------------------
*
* Function Name  : FTE_TASK_net
* Returned Value : void 
* Comments       : 
*
*END------------------------------------------------------------------*/

void FTE_TASK_net
(
    FTE_UINT32  params
)
{
    FTE_RET             xRet;
    FTE_TIME_DELAY      xTimeDelay;
    FTE_BOOL            bLinkActive = FALSE;
    FTE_NET_CFG_PTR     pCfgNet;
   
    xRet = FTE_CFG_NET_get(&pCfgNet);
    if (xRet != FTE_RET_OK)
    {
        goto error;
    }

    printf("Starting netd\n");
    if (FTE_NET_init(pCfgNet) == RTCS_ERROR)
    {
        goto error;
    }

    FTE_NET_liveCheckInit(FTE_CFG_SYS_getKeepAliveTime());
    if (FTE_CFG_SYS_getSystemMonitor())
    {
        FTE_NET_liveCheckStart();
    }
    
#if FTE_HTTPD_SUPPORTED
    printf("Starting httpd ... [%s]\n",(FTE_HTTPD_init(&pCfgNet->xHTTP) == FTE_RET_OK)?"OK":"FAILED");
#endif
#if FTE_SNMPD_SUPPORTED
    printf("Starting snmpd ... [%s]\n",(FTE_SNMPD_init(&pCfgNet->xSNMP) == FTE_RET_OK)?"OK":"FAILED");
#endif
#if FTE_SMNGD_SUPPORTED
    printf("Starting smngd ... [%s]\n",(FTE_SMNGD_init(NULL) == FTE_RET_OK)?"OK":"FAILED");
#endif
#if FTE_MQTT_SUPPORTED
    printf("Starting mqtt ... [%s]\n",(FTE_MQTT_init(&pCfgNet->xMQTT) == FTE_RET_OK)?"OK":"FAILED");
#endif
#if FTE_TELNETD_SUPPORTED
    printf("Starting telnet ... [%s]\n", (FTE_TELNETD_init(NULL) == FTE_RET_OK)?"OK":"FAILED");
#endif
#if FTE_MBTCP_SUPPORTED
    printf("Starting MB/TCP ... [%s]\n", (FTE_MBTCP_SLAVE_init(&pCfgNet->xMBTCP) == FTE_RET_OK)?"OK":"FAILED");
#endif
#if FTE_MBTCP_MASTER_SUPPORTED
    printf("Starting MB/TCP Master... [%s]\n", (FTE_MBTCP_MASTER_init(pCfgNet->xMBTCPMasters, 2) == FTE_RET_OK)?"OK":"FAILED");
#endif
    // tcp/ip initialization waiting
    _time_delay(FTE_NET_INIT_WAITING_TIME);

    FTE_TIME_DELAY_init(&xTimeDelay, 100);//FTE_NET_STATE_CHECK_INTERVAL);
    FTE_NET_isActive(&bLinkActive);
    
    while(TRUE)
    {
        IPCFG_STATE xState;
        FTE_BOOL bNewLinkActive;
        
        FTE_NET_isActive(&bNewLinkActive);
        if (bLinkActive != bNewLinkActive)
        {
            printf("Link is %s!\n",(bNewLinkActive)?"up":"down");
            bLinkActive = bNewLinkActive;
        }

        xState = ipcfg_get_state(BSP_DEFAULT_ENET_DEVICE);
        if (bLinkActive)
        {
            if ((IPCFG_STATE_BUSY == xState) || (IPCFG_STATE_UNBOUND == xState))
            {
                static int nRetry = 0;
                if (FTE_NET_bind() != FTE_RET_OK)
                {
                    if (++nRetry > 2)
                    {
                        FTE_SYS_reset();
                    }
                }
                else
                {
                    nRetry = 0;
                }
            }
            
#if FTE_SNMPD_SUPPORTED
            if ((IPCFG_STATE_BUSY != xState) && (IPCFG_STATE_UNBOUND != xState))
            {
                FTE_SNMPD_TRAP_processing();                
            }
#endif
            if (FTE_NET_isStable() == FALSE)
            {
                FTE_SYS_setUnstable();
            }
        }
        else 
        {
            if (IPCFG_STATE_UNBOUND != xState)
            {
                FTE_NET_unbind();
            }
        }

        FTE_TIME_DELAY_waitingAndSetNext(&xTimeDelay);
    }
    
error:
    _task_block();
}


