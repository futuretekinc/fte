#include "fte_target.h"
#include "fte_buff.h"
#include "fte_net.h"
#include <tfs.h> 

FTE_UINT32 FTE_HTTPD_init(FTE_HTTP_CFG_PTR  pConfig)
{ 
    FTE_UINT32                              server;
    extern const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[];
    extern const TFS_DIR_ENTRY           tfs_data[];
    HTTPSRV_PARAM_STRUCT                 srv_param;

    //fte_buff_init(FTE_NET_HTTP_CGI_BUFF_SIZE);

    /* Install trivial file system. HTTP server pages are stored there. */
    _io_tfs_install("tfs:", tfs_data);

    _mem_zero(&srv_param, sizeof(HTTPSRV_PARAM_STRUCT));
    srv_param.af = FTE_NET_HTTP_INET_AF;  //IPv4, IPv6 or from config.h

    srv_param.root_dir = "tfs:";
    srv_param.index_page = "\\index.html";
    srv_param.auth_table = pConfig->pAuthRealms;
    /*
    ** If there is only one server listen on any IP address 
    ** so address can change in runtime (DHCP etc.).
    ** Otherwise we will use static IP for server.
    */
    if (FTE_NET_HTTP_INET_AF & AF_INET)
    {
        #if RTCSCFG_ENABLE_IP4
        srv_param.ipv4_address.s_addr = INADDR_ANY;
        #endif
    }
    
    /*
    ** Maximum default number of available sockets in RTCS is six.
    ** Maximum of two are used for HTTP server as listening sockets (one for IPv4 and one for IPv6).
    ** So we take two of sockets and use them for sessions.
    */
    srv_param.max_ses = FTE_NET_HTTP_MAX_SESSION;
    srv_param.cgi_lnk_tbl = (HTTPSRV_CGI_LINK_STRUCT*) cgi_lnk_tbl;

    /* There are a lot of static data in CGIs so we have to set large enough stack for script handler */
    srv_param.script_stack = FTE_NET_HTTP_STACK_SIZE;
    server = HTTPSRV_init(&srv_param);

    if (server)
    {   
        return  FTE_RET_OK;
    }
    else
    {
        return  FTE_RET_ERROR;
    }
}