#include "fte_target.h"
#include "fte_config.h"
#include "fte_net.h"

/*TASK*-----------------------------------------------------------------
*
* Function Name  : FTE_TASK_net
* Returned Value : void 
* Comments       : 
*
*END------------------------------------------------------------------*/

void FTE_TASK_net(uint_32 params)
{
 
    int_32              ret;
    uint_32             nLinkCheck = 5;
    boolean             bLinkActive = FALSE;
    FTE_NET_CFG_PTR     pCfgNet = FTE_CFG_NET_get();

    ASSERT(pCfgNet != NULL);
    
    printf("Starting netd\n");
    ret = FTE_NET_init(pCfgNet); 
    if (ret == RTCS_ERROR)
    {
        goto error;
    }

    FTE_TASK_append(FTE_TASK_TYPE_MQX, _task_get_id());
    
#if FTE_HTTPD_SUPPORTED
    printf("Starting httpd ... [%s]\n",(FTE_HTTPD_init(&pCfgNet->xHTTP) == MQX_OK)?"OK":"FAILED");
#endif
#if FTE_SNMPD_SUPPORTED
    printf("Starting snmpd ... [%s]\n",(FTE_SNMPD_init(&pCfgNet->xSNMP) == MQX_OK)?"OK":"FAILED");
#endif
#if FTE_SMNGD_SUPPORTED
    printf("Starting smngd ... [%s]\n",(FTE_SMNGD_init(NULL) == MQX_OK)?"OK":"FAILED");
#endif
#if FTE_MQTT_SUPPORTED
    printf("Starting mqtt ... [%s]\n",(FTE_MQTT_init(&pCfgNet->xMQTT) == MQX_OK)?"OK":"FAILED");
#endif
#if FTE_TELNETD_SUPPORTED
    printf("Starting telnet ... [%s]\n", (FTE_TELNETD_init(NULL) == MQX_OK)?"OK":"FAILED");
#endif
    
    // tcp/ip initialization waiting
    _time_delay(10000);

    while(1)
    {
        if (-- nLinkCheck <= 0)
        {
            IPCFG_STATE xState;
            
            boolean bNewLinkActive =ipcfg_get_link_active (BSP_DEFAULT_ENET_DEVICE);
            if (bLinkActive != bNewLinkActive)
            {
                if (bNewLinkActive)
                {
                    printf("Link is up!\n");
                }
                else
                {
                    printf("Link Down : %d\n", ipcfg_get_link_active (BSP_DEFAULT_ENET_DEVICE));
                    printf("Link is down!\n");
                }
                
                bLinkActive = bNewLinkActive;
            }

            xState = ipcfg_get_state(BSP_DEFAULT_ENET_DEVICE);
            if (bLinkActive)
            {
                if ((IPCFG_STATE_BUSY == xState) || (IPCFG_STATE_UNBOUND == xState))
                {
                    static int nRetry = 0;
                    if (FTE_NET_bind() != MQX_OK)
                    {
                        if (++nRetry > 2)
                        {
                            FTE_SYS_reset();
                        }
                    }
                    else
                    {
                        nRetry = 0;;
                    }
                }
                
#if FTE_SNMPD_SUPPORTED
                if ((IPCFG_STATE_BUSY != xState) && (IPCFG_STATE_UNBOUND != xState))
                {
                    FTE_SNMPD_TRAP_processing();                
                }
#endif
            }
            else 
            {
                if (IPCFG_STATE_UNBOUND != xState)
                {
                    FTE_NET_unbind();
                }
            }
            
            nLinkCheck = 2;
        }


        _time_delay(200);
    }
    
error:
    _task_block();
}

