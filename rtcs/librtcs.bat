@echo off

IF NOT EXIST ..\include\rtcs    mkdir ..\include\rtcs
IF NOT EXIST ..\include\httpd   mkdir ..\include\httpd

copy source\httpd\httpd.h ..\include\httpd\httpd.h /Y
copy source\httpd\httpd_cnfg.h ..\include\httpd\httpd_cnfg.h /Y
copy source\httpd\httpd_mqx.h ..\include\httpd\httpd_mqx.h /Y
copy source\httpd\httpd_supp.h ..\include\httpd\httpd_supp.h /Y
copy source\httpd\httpd_types.h ..\include\httpd\httpd_types.h /Y
copy source\httpd\httpd_wrapper.h ..\include\httpd\httpd_wrapper.h /Y
copy source\include\addrinfo.h ..\include\rtcs\addrinfo.h /Y
copy source\include\arp.h ..\include\rtcs\arp.h /Y
copy source\include\arpif.h ..\include\rtcs\arpif.h /Y
copy source\include\asn1.h ..\include\rtcs\asn1.h /Y
copy source\include\bootp.h ..\include\rtcs\bootp.h /Y
copy source\include\ccp.h ..\include\rtcs\ccp.h /Y
copy source\include\chap.h ..\include\rtcs\chap.h /Y
copy source\include\checksum.h ..\include\rtcs\checksum.h /Y
copy source\include\dhcp.h ..\include\rtcs\dhcp.h /Y
copy source\include\dhcpsrv.h ..\include\rtcs\dhcpsrv.h /Y
copy source\include\dns.h ..\include\rtcs\dns.h /Y
copy source\include\fcs16.h ..\include\rtcs\fcs16.h /Y
copy source\include\ftp.h ..\include\rtcs\ftp.h /Y
copy source\include\ftpc.h ..\include\rtcs\ftpc.h /Y
copy source\include\ftpd.h ..\include\rtcs\ftpd.h /Y
copy source\include\ftpd_msg.h ..\include\rtcs\ftpd_msg.h /Y
copy source\include\httpsrv.h ..\include\rtcs\httpsrv.h /Y
copy source\include\httpsrv_prv.h ..\include\rtcs\httpsrv_prv.h /Y
copy source\include\httpsrv_supp.h ..\include\rtcs\httpsrv_supp.h /Y
copy source\include\icmp.h ..\include\rtcs\icmp.h /Y
copy source\include\icmp6.h ..\include\rtcs\icmp6.h /Y
copy source\include\icmp6_prv.h ..\include\rtcs\icmp6_prv.h /Y
copy source\include\igmp.h ..\include\rtcs\igmp.h /Y
copy source\include\ip-e.h ..\include\rtcs\ip-e.h /Y
copy source\include\ip.h ..\include\rtcs\ip.h /Y
copy source\include\ip6.h ..\include\rtcs\ip6.h /Y
copy source\include\ip6_if.h ..\include\rtcs\ip6_if.h /Y
copy source\include\ip6_prv.h ..\include\rtcs\ip6_prv.h /Y
copy source\include\ip6_trace.h ..\include\rtcs\ip6_trace.h /Y
copy source\include\ipc_udp.h ..\include\rtcs\ipc_udp.h /Y
copy source\include\ipcfg.h ..\include\rtcs\ipcfg.h /Y
copy source\include\ipcp.h ..\include\rtcs\ipcp.h /Y
copy source\include\ipradix.h ..\include\rtcs\ipradix.h /Y
copy source\include\iptunnel.h ..\include\rtcs\iptunnel.h /Y
copy source\include\iwcfg.h ..\include\rtcs\iwcfg.h /Y
copy source\include\lcp.h ..\include\rtcs\lcp.h /Y
copy source\include\md5.h ..\include\rtcs\md5.h /Y
copy source\include\nat.h ..\include\rtcs\nat.h /Y
copy source\include\nat_session.h ..\include\rtcs\nat_session.h /Y
copy source\include\natvers.h ..\include\rtcs\natvers.h /Y
copy source\include\nd6.h ..\include\rtcs\nd6.h /Y
copy source\include\pap.h ..\include\rtcs\pap.h /Y
copy source\include\ppp.h ..\include\rtcs\ppp.h /Y
copy source\include\pppfsm.h ..\include\rtcs\pppfsm.h /Y
copy source\include\ppphdlc.h ..\include\rtcs\ppphdlc.h /Y
copy source\include\rip.h ..\include\rtcs\rip.h /Y
copy source\include\route.h ..\include\rtcs\route.h /Y
copy source\include\rtcs.h ..\include\rtcs\rtcs.h /Y
copy source\include\rtcs25x.h ..\include\rtcs\rtcs25x.h /Y
copy source\include\rtcs_base64.h ..\include\rtcs\rtcs_base64.h /Y
copy source\include\rtcs_in.h ..\include\rtcs\rtcs_in.h /Y
copy source\include\rtcs_smtp.h ..\include\rtcs\rtcs_smtp.h /Y
copy source\include\rtcscfg.h ..\include\rtcs\rtcscfg.h /Y
copy source\include\rtcshdlc.h ..\include\rtcs\rtcshdlc.h /Y
copy source\include\rtcslog.h ..\include\rtcs\rtcslog.h /Y
copy source\include\rtcspcb.h ..\include\rtcs\rtcspcb.h /Y
copy source\include\rtcsrtos.h ..\include\rtcs\rtcsrtos.h /Y
copy source\include\rtcstime.h ..\include\rtcs\rtcstime.h /Y
copy source\include\rtcsvers.h ..\include\rtcs\rtcsvers.h /Y
copy source\include\snmp.h ..\include\rtcs\snmp.h /Y
copy source\include\snmpcfg.h ..\include\rtcs\snmpcfg.h /Y
copy source\include\socket.h ..\include\rtcs\socket.h /Y
copy source\include\tcp.h ..\include\rtcs\tcp.h /Y
copy source\include\tcpip.h ..\include\rtcs\tcpip.h /Y
copy source\include\telnet.h ..\include\rtcs\telnet.h /Y
copy source\include\tftp.h ..\include\rtcs\tftp.h /Y
copy source\include\ticker.h ..\include\rtcs\ticker.h /Y
copy source\include\udp.h ..\include\rtcs\udp.h /Y
