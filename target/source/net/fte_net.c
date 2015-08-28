#include "fte_target.h"
#include "fte_config.h"
#include "fte_net.h"
#include "sys/fte_sys.h"
#include <tfs.h>
#include <ipcfg.h>
#include <sh_rtcs.h> 
#include "enet.h"
#include "phy_mv88e6060.h"

static  FTE_NET_CFG_PTR _pCfgNet = NULL;

int_32 FTE_NET_init(FTE_NET_CFG_PTR pConfig)
{
    int_32          ret;
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
    
    ret = RTCS_create();
    if (ret != RTCS_OK) 
    {
        printf("RTCS failed to initialize, error = 0x%X\n", ret);
        goto error;
    }

    _IP_forward = TRUE;

    /* Init ENET device */
    FTE_SYS_getMAC(xMACAddress);
    ret = ipcfg_init_device (FTE_NET_PHY_ADDR, xMACAddress);
    if (ret != RTCS_OK) 
    {
        printf("IPCFG: Device init failed. Error = 0x%X\n", ret);
        _task_set_error(MQX_OK);
        
        goto error;
    }

    _pCfgNet = pConfig;
    
    return  RTCS_OK;
    
error:
    return  RTCS_ERROR;
}

uint_32 FTE_NET_bind(void)
{
    int_32  ret = IPCFG_OK;
    
    if (_pCfgNet->nType == FTE_NET_TYPE_DHCP)
    {
        ret = ipcfg_bind_dhcp_wait (BSP_DEFAULT_ENET_DEVICE, FALSE, &_pCfgNet->xIPData);
        if (ret != IPCFG_OK)
        {
            printf ("Error during dhcp bind %08x!\n", ret);
            goto error;
        }

        if (ipcfg_get_state(BSP_DEFAULT_ENET_DEVICE) == IPCFG_STATE_DHCPAUTO_IP)
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
        ret = ipcfg_bind_staticip (BSP_DEFAULT_ENET_DEVICE, &_pCfgNet->xIPData);
        if (ret != RTCS_OK) 
        {
            printf("\nIPCFG: Failed to bind IP address. Error = 0x%X", ret);
            goto error;
        }
        
        printf ("Bind successful.\n");
    } 
    
    FTE_SYS_STATE_connected();
    
    return  MQX_OK;
    
error:
    return  MQX_ERROR;
}

uint_32 FTE_NET_unbind(void)
{
    ipcfg_unbind(BSP_DEFAULT_ENET_DEVICE);
    printf ("Unbind successful.\n");

    FTE_SYS_STATE_disconnected();
    
    return  MQX_OK;
}

int_32  FTE_NET_getMACAddress(_enet_address address)
{
    if (_pCfgNet == NULL)
    {
        return  RTCS_ERROR;
    }
    
    ENET_get_mac_address(FTE_NET_PHY_ADDR, _pCfgNet->xIPData.ip, address);
    
    return  RTCS_OK;
}

#if FTE_SNMPD_SUPPORTED
int_32  FTE_NET_SERVER_count(void)
{
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();
    
    return  pCfgNet->xSNMP.xTrap.ulCount;
}


uint_32  FTE_NET_SERVER_getAt(uint_32 idx)
{
    FTE_NET_CFG_PTR pCfgNet = FTE_CFG_NET_get();
     
    if (pCfgNet->xSNMP.xTrap.ulCount > idx)
    {
        return  pCfgNet->xSNMP.xTrap.pList[idx];
    }
    else
    {
        return  0;
    }
}

boolean FTE_NET_SERVER_isExist(_ip_address ip)
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

int_32  FTE_PHY_SHELL_cmd(int_32 argc, char_ptr argv[] )
{ 
    boolean              bPrintUsage, bShortHelp = FALSE;
    int_32               nRet = SHELL_EXIT_SUCCESS;

    bPrintUsage = Shell_check_help_request (argc, argv, &bShortHelp);

    if (!bPrintUsage)
    {
        switch(argc)
        {
        case    1:
            {
                int nPhyID, nRegID ;
                for(nRegID = 0 ; nRegID < 18 ; nRegID++)
                {
                    printf("%2d : ", nRegID);
                    for(nPhyID = 0 ; nPhyID < 16  ; nPhyID++)
                    {
                        uint_32 ulValue;
                        phy_mv88e6060_get_reg(nPhyID, nRegID, &ulValue);
                        printf("%04x ", ulValue);
                    }
                    printf("\n");
                }
            }
            break;
            
        case    3:
            {
                uint_32 nPhyID, nRegID, ulValue;

                if (!Shell_parse_hexnum(argv[1], &nPhyID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (!Shell_parse_hexnum(argv[2], &nRegID))
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
                uint_32 nPhyID, nRegID, ulValue ;

                if (!Shell_parse_hexnum(argv[1], &nPhyID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (!Shell_parse_hexnum(argv[2], &nRegID))
                {
                    bPrintUsage = TRUE;
                    break;
                }

                if (!Shell_parse_hexnum(argv[3], &ulValue))
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

int_32 FTE_NET_SHELL_cmd(int_32 nArgc, char_ptr pArgv[] )
{ 
    boolean                 bPrintUsage, bShortHelp = FALSE;
    int_32                  nRet = SHELL_EXIT_SUCCESS;
    uint_32                 ulEnetDevice = BSP_DEFAULT_ENET_DEVICE;
    FTE_NET_CFG             xNetCfg;
    
    bPrintUsage = Shell_check_help_request (nArgc, pArgv, &bShortHelp);
    
    switch(nArgc)
    {
    case    1:
        {
            IPCFG_STATE             xState;
            boolean                 bLink;
            boolean                 bTask;
            _enet_address           xMAC= {0};
            IPCFG_IP_ADDRESS_DATA   xIPData;
        #if RTCSCFG_IPCFG_ENABLE_DNS
            _ip_address             xDNS;
        #endif

            xState = ipcfg_get_state (ulEnetDevice);
            if (xState != IPCFG_STATE_INIT)
            {
                bLink = ipcfg_get_link_active (ulEnetDevice);
                bTask = ipcfg_task_status ();
                ipcfg_get_mac (ulEnetDevice, xMAC);
                ipcfg_get_ip (ulEnetDevice, &xIPData);
        /* It take amount of addresses for device and store it to n */
#if RTCSCFG_IPCFG_ENABLE_DNS
                xDNS = ipcfg_get_dns_ip (enet_device, 0);
#endif
                printf ("Link: %s\n", bLink ? "on" : "off");
                printf ("MAC : %02x:%02x:%02x:%02x:%02x:%02x\n", xMAC[0], xMAC[1], xMAC[2], xMAC[3], xMAC[4], xMAC[5]);
                printf ("IP4 : %d.%d.%d.%d type: %s\n", IPBYTES(xIPData.ip), ipcfg_get_state_string (xState) );
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
                printf ("Ethernet device %d not yet initialized.\n", ulEnetDevice);
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
                FTE_NET_printStats();
            }
            else
            {
                nRet = SHELL_EXIT_ERROR;
                goto error;
            }
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

void    FTE_NET_printStats(void)
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