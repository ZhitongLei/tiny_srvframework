#include "conf.h"

namespace tiny_srv{

conf::conf()
: max_conn_num(MAX_CONN_NUM), timeout(BLOCK)
{}


void conf::add_tcp_ha(const host_addr &ha, int addr_name, int pkg_head_len)
{
    srv_addr_conf addr_conf(ha, addr_name, pkg_head_len);
	tcp_srv_ha_list.push_back(addr_conf);
}

void conf::add_tcp_ha(const std::string &ip, uint16_t port, int addr_name, int pkg_head_len)
{
    add_tcp_ha(host_addr(ip, port), addr_name, pkg_head_len);
}

void conf::add_udp_ha(const host_addr &ha, int addr_name)
{
    srv_addr_conf addr_conf(ha, addr_name);
	udp_srv_ha_list.push_back(addr_conf);
}

void conf::add_udp_ha(const std::string &ip, uint16_t port, int addr_name)
{
    add_udp_ha(host_addr(ip, port), addr_name);
}

uint32_t conf::get_total_socket_num() const
{
    return get_tcp_srv_num() + get_udp_srv_num();
}

uint32_t conf::get_tcp_srv_num() const
{
    return tcp_srv_ha_list.size();
}

uint32_t conf::get_udp_srv_num() const
{
    return udp_srv_ha_list.size();
}

} // end of namespace
