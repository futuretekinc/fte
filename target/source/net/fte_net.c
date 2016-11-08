#include "fte_target.h"
#include "fte_config.h"
#include "fte_net.h"
#include "sys/fte_sys.h"
#include <tfs.h>
#include <ipcfg.h>
#include <sh_rtcs.h> 
#include "enet.h"
#include "phy_mv88e6060.h"

static  
FTE_NET_CFG_PTR _pCfgNet = NULL;

static  
MQX_TICK_STRUCT _xLastCheckTime;

static  
FTE_BOOL        _bLiveCheck = FALSE;

static  
FTE_UINT32      _ulKeepAliveTime = FTE_SYS_KEEP_ALIVE_TIME;

FTE_RET FTE_NET_init
(
    FTE_NET_CFG_PTR pConfig
)
{
    FTE_INT32       xRet;
    _enet_address   xMACAddress;
    
    /* Init RTCS */
    _RTCSPCB_init = 8;
    _RTCSPCB_grow = 4;
    _RTCSPCB_max = 32;
    _RTCS_msgpool_init = 8;
    _RTCS_msgpool_grow = 4;
    _RTCS_msgpool_max  = 32;
    _RTCS_socket_part_init = 8;
    _RTCS_socket_part_grow = 4;
    _RTCS_socket_part_max  = 32;
    
    _RTCSTASK_stacksize = 2400;
    
    xRet = RTCS_create();
    if (xRet != RTCS_OK) 
    {
        printf("RTCS failed to initialize, error = 0x%X\n", xRet);
        goto error;
    }

    _IP_forward = TRUE;

    /* Init ENET device */
    FTE_SYS_getMAC(xMACAddress);
    xRet = ipcfg_init_device (FTE_NET_PHY_ADDR, xMACAddress);
    if (xRet != RTCS_OK) 
    {
        printf("IPCFG: Device init failed. Error = 0x%X\n", xRet);
        _task_set_error(FTE_RET_OK);
        
        goto error;
    }

    _pCfgNet = pConfig;
    
    return  FTE_RET_OK;
    
error:
    return  xRet;
}

FTE_RET     FTE_NET_isActive
(
    FTE_BOOL_PTR pActive
)
{
    ASSERT(pActive != NULL);
    
    *pActive = ipcfg_get_link_active (BSP_DEFAULT_ENET_DEVICE);
    
    return  FTE_RET_OK;
}

FTE_RET FTE_NET_STATE_get
(
    FTE_NET_STATE_PTR   pState
)
{
    ASSERT(pState != NULL);
    
    *pState = (FTE_NET_STATE)ipcfg_get_state(BSP_DEFAULT_ENET_DEVICE);

    return  FTE_RET_OK;
}

FTE_CHAR const _PTR_ FTE_NET_STATE_print
(
    FTE_NET_STATE   xState
)
{
    return  ipcfg_get_state_string ((IPCFG_STATE)xState);
}

FTE_RET FTE_NET_bind
(
    void
)
{
    FTE_RET xRet = FTE_RET_OK;
    FTE_NET_STATE   xState;
    
    if (_pCfgNet->nType == FTE_NET_TYPE_DHCP)
    {
        xRet = ipcfg_bind_dhcp_wait (BSP_DEFAULT_ENET_DEVICE, FALSE, &_pCfgNet->xIPData);
        if (xRet != FTE_RET_OK)
        {
            printf ("Error during dhcp bind %08x!\n", xRet);
            goto error;
        }

        FTE_NET_STATE_get(&xState);
        if (xState == FTE_NET_STATE_DHCPAUTO_IP)
        {
            printf ("Bind via auto ip.\n");
        }
        else
        {
            ipcfg_get_ip (BSP_DEFAULT_ENET_DEVICE, &_pCfgNet->xIPData);    
            printf ("Bind via dhcp successful.\n");
        }        
    }
    else
    {        
        /* Bind IPv4 address */
        xRet = ipcfg_bind_staticip (BSP_DEFAULT_ENET_DEVICE, &_pCfgNet->xIPData);
        if (xRet != FTE_RET_OK) 
        {
            printf("\nIPCFG: Failed to bind IP address. Error = 0x%X", xRet);
            goto error;
        }
        
        printf ("Bind successful.\n");
    } 
    
    FTE_SYS_STATE_connected();
    
    return  FTE_RET_OK;
    
error:
    return  FTE_RET_ERROR;
}

FTE_RET FTE_NET_unbind
(
    void
)
{
    ipcfg_unbind(BSP_DEFAULT_ENET_DEVICE);
    printf ("Unbind successful.\n");

    FTE_SYS_STATE_disconnected();
    
    return  FTE_RET_OK;
}

FTE_RET  FTE_NET_getMACAddress
(
    _enet_address address
)
{
    if (_pCfgNet == NULL)
    {
        return  FTE_RET_ERROR;
    }
    
    ENET_get_mac_address(FTE_NET_PHY_ADDR, _pCfgNet->xIPData.ip, address);
    
    return  FTE_RET_OK;
}

#if FTE_SNMPD_SUPPORTED
FTE_INT32  FTE_NET_SERVER_count
(
    void
)
{
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();
    
    return  pCfgNet->xSNMP.xTrap.ulCount;
}


FTE_UINT32  FTE_NET_SERVER_getAt
(
    FTE_UINT32 ulIndex
)
{
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();
     
    if (pCfgNet->xSNMP.xTrap.ulCount > ulIndex)
    {
        return  pCfgNet->xSNMP.xTrap.pList[ulIndex];
    }
    else
    {
        return  0;
    }
}

FTE_BOOL FTE_NET_SERVER_isExist
(
    _ip_address ip
)
{
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();
   
   if (pCfgNet == NULL)
    {
        return  FALSE;
    }
    
    for(int i = 0 ; i < pCfgNet->xSNMP.xTrap.ulCount ; i++)
    {
        if (pCfgNet->xSNMP.xTrap.pList[i] == ip)
        {
            return  TRUE;
        }
    }
    
    return  FALSE;
}
#endif

FTE_RET FTE_NET_liveCheckInit
(
    FTE_UINT32  ulKeepAliveTime
)
{
    if ((ulKeepAliveTime >= FTE_SYS_KEEP_ALIVE_TIME_MIN) && (ulKeepAliveTime <= FTE_SYS_KEEP_ALIVE_TIME_MAX))
    {
        _time_get_elapsed_ticks(&_xLastCheckTime);
        _ulKeepAliveTime = ulKeepAliveTime;
        return  FTE_RET_OK;
    }
    
    return  FTE_RET_ERROR;
} 

FTE_RET   FTE_NET_liveCheckStart
(
    void
)
{
    _time_get_elapsed_ticks(&_xLastCheckTime);
    _bLiveCheck = TRUE;

    return  FTE_RET_OK;
}

FTE_RET   FTE_NET_liveCheckStop
(
    void
)
{
    _bLiveCheck = FALSE;
    
    return  FTE_RET_OK;
}

FTE_BOOL     FTE_NET_isLiveChecking
(
    void
)
{
    return  _bLiveCheck;
}

FTE_RET   FTE_NET_lastLiveCheckTime
(
    MQX_TICK_STRUCT_PTR pTime
)
{
    memcpy(pTime, &_xLastCheckTime, sizeof(MQX_TICK_STRUCT));
    
    return  FTE_RET_OK;
}

FTE_RET   FTE_NET_liveTouch
(
    void
)
{
    _time_get_elapsed_ticks(&_xLastCheckTime);

    return  FTE_RET_OK;
}


FTE_BOOL     FTE_NET_isStable
(
    void
)
{
    if (_bLiveCheck)
    {
        MQX_TICK_STRUCT xCurrentTime;
        FTE_BOOL         bOverflow = FALSE;
        FTE_UINT32         ulDiffTime;
        
        _time_get_elapsed_ticks(&xCurrentTime);
        
        ulDiffTime = _time_diff_seconds(&xCurrentTime, &_xLastCheckTime, &bOverflow);
        if ( (ulDiffTime> _ulKeepAliveTime) || bOverflow)
        {
            return  FALSE;
        }
    }
    
    return  TRUE;
}

FTE_INT32  FTE_PHY_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL    bPrintUsage, bShortHelp = FALSE;
    FTE_INT32   nRet = SHELL_EXIT_SUCCESS;

    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(nArgc)
        {
        case    1:
            {
                int nPhyID, nRegID ;
                for(nRegID = 0 ; nRegID < 18 ; nRegID++)
                {
                    printf("%2d : ", nRegID);
                    for(nPhyID = 0 ; nPhyID < 16  ; nPhyID++)
                    {
                        FTE_UINT32 ulValue;
                        phy_mv88e6060_get_reg(nPhyID, nRegID, &ulValue);
                        printf("%04x ", ulValue);
                    }
                    printf("\n");
                }
            }
            break;
            
        case    3:
            {
                FTE_UINT32 nPhyID, nRegID, ulValue;

                if (!Shell_parse_hexnum(pArgv[1], &nPhyID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (!Shell_parse_hexnum(pArgv[2], &nRegID))
                {
                    bPrintUsage = TRUE;
                    break;
                }
                if (phy_mv88e6060_get_reg(nPhyID, nRegID, &ulValue))
                {
                    printf("MV88E6060 REG[ %02x:%02x ] : %04x\n", nPhyID, nRegID, ulValue);
                }
                else
                {
                    printf("MV88E6060 REG[ %02x:%02x ] : Error\n", nPhyID, nRegID);
                }
                
            }
            break;

        case    4:
            {
                FTE_UINT32 nPhyID, nRegID, ulValue ;

                if (!Shell_parse_hexnum(pArgv[1], &nPhyID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (!Shell_parse_hexnum(pArgv[2], &nRegID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (!Shell_parse_hexnum(pArgv[3], &ulValue))
                {
                    bPrintUsage = TRUE;
                    break;
                }
                if (phy_mv88e6060_set_reg(nPhyID, nRegID, ulValue))
                {
                    printf("MV88E6060 REG[ %02x:%02x ] set success\n", nPhyID, nRegID);
                }
                else
                {
                    printf("MV88E6060 REG[ %02x:%02x ] set failed\n", nPhyID, nRegID);
                }

            }
            break;
        }
    }
    
    return  nRet;
}

FTE_INT32 FTE_NET_SHELL_cmd
(
    FTE_INT32       nArgc, 
    FTE_CHAR_PTR    pArgv[] 
)
{ 
    FTE_BOOL                 bPrintUsage, bShortHelp = FALSE;
    FTE_INT32                  nRet = SHELL_EXIT_SUCCESS;
    FTE_NET_CFG             xNetCfg;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            FTE_NET_STATE            xState;
            FTE_BOOL                 bLink;
            FTE_BOOL                 bTask;
            _enet_address           xMAC= {0};
            IPCFG_IP_ADDRESS_DATA   xIPData;
#if RTCSCFG_IPCFG_ENABLE_DNS
            _ip_address             xDNS;
#endif

            FTE_NET_STATE_get(&xState);
            if (xState != FTE_NET_STATE_INIT)
            {
                FTE_NET_isActive(&bLink);
                bTask = ipcfg_task_status ();
                ipcfg_get_mac (0, xMAC);
                ipcfg_get_ip (0, &xIPData);
        /* It take amount of addresses for device and store it to n */
#if RTCSCFG_IPCFG_ENABLE_DNS
                xDNS = ipcfg_get_dns_ip (0, 0);
#endif
                printf ("Link: %s\n", bLink ? "on" : "off");
                printf ("MAC : %02x:%02x:%02x:%02x:%02x:%02x\n", xMAC[0], xMAC[1], xMAC[2], xMAC[3], xMAC[4], xMAC[5]);
                printf ("IP4 : %d.%d.%d.%d type: %s\n", IPBYTES(xIPData.ip), FTE_NET_STATE_print(xState) );
        /************************************************/
                printf ("MASK: %d.%d.%d.%d\n", IPBYTES(xIPData.mask));
#if RTCSCFG_ENABLE_GATEWAYS
                printf ("GATE: %d.%d.%d.%d\n", IPBYTES(xIPData.gateway));
#endif

#if RTCSCFG_IPCFG_ENABLE_DNS
                printf ("DNS1: %d.%d.%d.%d\n", IPBYTES(xDNS));
#endif
                printf ("Link status task %s\n", bTask ? "running" : "stopped");
            }
            else
            {
                printf ("Ethernet device %d not yet initialized.\n", 0);
            }
            return SHELL_EXIT_SUCCESS;
        }
        break;
        
    case    2:
        {
            
            if (strcmp(pArgv[1], "dhcp") == 0)
            {
                FTE_CFG_NET_copy(&xNetCfg);
                xNetCfg.nType = FTE_NET_TYPE_DHCP;

                FTE_CFG_NET_set(&xNetCfg);
            }
            else if (strcmp(pArgv[1], "stat") == 0)
            {
                FTE_NET_STATISTICS_print();
            }
#if RTCSCFG_IPCFG_ENABLE_DNS
            else if (strcmp(pArgv[1], "dns") == 0)
            {
                FTE_UINT32     i;
                _ip_address xDNS;

                for (i = 0, xDNS = ipcfg_get_dns_ip (0, i); xDNS != 0 ; i++, xDNS = ipcfg_get_dns_ip (0, i))
                {
                    printf ("%d: %d.%d.%d.%d\n", i + 1, IPBYTES(xDNS));
                }
            }
#endif
            else
            {
                nRet = SHELL_EXIT_ERROR;
                goto error;
            }
        }
        break;
        
    case    4:
        {
#if RTCSCFG_IPCFG_ENABLE_DNS
            if (strcmp(pArgv[1], "dns") == 0)
            {
                _ip_address xDNS;

                if (strcmp (pArgv[2], "add") == 0)
                {
                    if (! Shell_parse_ip_address (pArgv[3], &xDNS))
                    {
                        printf ("Error in dns command, invalid ip address!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    if (ipcfg_add_dns_ip (0, xDNS))
                    {
                        printf ("Add dns ip successful.\n");
                    }
                    else
                    {
                        printf ("Add dns ip failed!\n");
                        return SHELL_EXIT_ERROR;
                    }
                }
                else if (strcmp (pArgv[2], "del") == 0)
                {
                    if (! Shell_parse_ip_address (pArgv[3], &xDNS))
                    {
                        printf ("Error in dns command, invalid ip address!\n");
                        return SHELL_EXIT_ERROR;
                    }
                    if (ipcfg_del_dns_ip (0, xDNS))
                    {
                        printf ("Del dns ip successful.\n");
                    }
                    else
                    {
                        printf ("Del dns ip failed!\n");
                        return SHELL_EXIT_ERROR;
                    }
                }
                else
                {
                    printf ("Error in dns command, unknown parameter!\n");
                    return SHELL_EXIT_ERROR;
                }

            }
#endif
        }
        break;
        
    case    5:
        {
            if (strcmp(pArgv[1], "static") == 0)
            {
                _ip_address xHostIP = 0;
                _ip_address xNetmask = 0;
                _ip_address xGatewayIP= 0;

                if (! Shell_parse_ip_address (pArgv[2], &xHostIP))
                {
                    printf ("Error in parameter, invalid ip address!\n");
                    nRet = SHELL_EXIT_ERROR;
                    goto error;
                }

                if (! Shell_parse_ip_address (pArgv[3], &xNetmask))
                {
                    printf ("Error in parameter, invalid netmask!\n");
                    nRet = SHELL_EXIT_ERROR;
                    goto error;
                }

                if (! Shell_parse_ip_address (pArgv[4], &xGatewayIP))
                {
                    printf ("Error in parameter, invalid gateway ip address!\n");
                    nRet = SHELL_EXIT_ERROR;
                    goto error;
                }
                
                FTE_CFG_NET_copy(&xNetCfg);
                
                xNetCfg.nType = FTE_NET_TYPE_STATIC;
                xNetCfg.xIPData.ip = xHostIP;
                xNetCfg.xIPData.mask = xNetmask;
                xNetCfg.xIPData.gateway = xGatewayIP;
                
                FTE_CFG_NET_set(&xNetCfg);
            }
            else
            {
                nRet = SHELL_EXIT_ERROR;
                goto error;
            }
        }
        break;
        
    default:
        {
            bPrintUsage = TRUE;
        }
    }

error:    
    if (bPrintUsage)  
    {
        if (bShortHelp)  
        {
            printf("%s [<command>]\n", pArgv[0]);
        } 
        else  
        {
            printf("Usage: %s [<command>]\n",pArgv[0]);
            printf("  Commands:\n");
            printf("    static <IP> <Netmask> <Gateway>\n");
            printf("        Static IP configuration.\n");
            printf("    dhcp\n");
            printf("        Dynamic IP configuration.\n");
            printf("    stat\n");
            printf("        Statistics informations.\n");
        }
    }
    
    return  nRet;
} 

void    FTE_NET_STATISTICS_print
(
    void
)
{
    IP_STATS_PTR    pStats = IP_stats();
    if (pStats != NULL)
    {
        printf("\n<Network Statistics>\n");
        printf("\n< RX STAT >\n");
        printf("%16s : %d\n", "Total",      pStats->COMMON.ST_RX_TOTAL);
        printf("%16s : %d\n", "Missed",     pStats->COMMON.ST_RX_MISSED);
        printf("%16s : %d\n", "Discarded",  pStats->COMMON.ST_RX_DISCARDED);
        printf("%16s : %d\n", "Errors",     pStats->COMMON.ST_RX_ERRORS);
        
        printf("\n< TX STAT >\n");
        printf("%16s : %d\n", "Total",      pStats->COMMON.ST_TX_TOTAL);
        printf("%16s : %d\n", "Missed",     pStats->COMMON.ST_TX_MISSED);
        printf("%16s : %d\n", "Discarded",  pStats->COMMON.ST_TX_DISCARDED);
        printf("%16s : %d\n", "Errors",     pStats->COMMON.ST_TX_ERRORS);

        printf("\n< RX ERROR >\n");
        printf("%16s : %d\n", "Header Errs",pStats->ST_RX_HDR_ERRORS);          /* Discarded -- error in IP header    */
        printf("%16s : %d\n", "Addr Errs",  pStats->ST_RX_ADDR_ERRORS);         /* Discarded -- illegal destination   */
        printf("%16s : %d\n", "No Proto",   pStats->ST_RX_NO_PROTO);            /* Discarded -- unrecognized protocol */
        printf("%16s : %d\n", "Delivered",  pStats->ST_RX_DELIVERED);           /* Datagrams delivered to upper layer */
        printf("%16s : %d\n", "Forwarded",  pStats->ST_RX_FORWARDED);           /* Datagrams forwarded                */

        printf("%16s : %d\n", "Bad Version",    pStats->ST_RX_BAD_VERSION);     /* Datagrams with version != 4        */
        printf("%16s : %d\n", "Bad Checksum",   pStats->ST_RX_BAD_CHECKSUM);    /* Datagrams with invalid checksum    */
        printf("%16s : %d\n", "Bad Source",     pStats->ST_RX_BAD_SOURCE);      /* Datagrams with invalid src address */
        printf("%16s : %d\n", "Small HDR",      pStats->ST_RX_SMALL_HDR);       /* Datagrams with header too small    */
        printf("%16s : %d\n", "Small DGRAM",    pStats->ST_RX_SMALL_DGRAM);     /* Datagrams smaller than header      */
        printf("%16s : %d\n", "Small Pkt",      pStats->ST_RX_SMALL_PKT);       /* Datagrams larger than frame        */
        printf("%16s : %d\n", "TTL Exceeded",   pStats->ST_RX_TTL_EXCEEDED);    /* Datagrams to route with TTL = 0    */

        printf("%16s : %d\n", "Frag Recvd",     pStats->ST_RX_FRAG_RECVD);      /* Number of received IP fragments    */
        printf("%16s : %d\n", "Frag Reasmd",    pStats->ST_RX_FRAG_REASMD);      /* Number of reassembled datagrams    */
        printf("%16s : %d\n", "Frag Discarded", pStats->ST_RX_FRAG_DISCARDED);   /* Number of discarded fragments      */

        printf("%16s : %d\n", "Frag Sent",      pStats->ST_TX_FRAG_SENT);        /* Number of sent fragments           */
        printf("%16s : %d\n", "Frag Fragd",     pStats->ST_TX_FRAG_FRAGD);       /* Number of fragmented datagrams     */
        printf("%16s : %d\n", "Frag Discarded", pStats->ST_TX_FRAG_DISCARDED);   /* Number of fragmentation failures   */

    }
}