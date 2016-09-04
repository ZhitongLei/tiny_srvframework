#ifndef _CONF_H_
#define _CONF_H_

#include "host_addr.h"
#include <vector>

namespace tiny_srv{

enum{
    BLOCK  = -1,
    NOWAIT = 0
};

const int MAX_CONN_NUM = 1024;

class srv_addr_conf
{
public:
    srv_addr_conf(const host_addr &ha, int addr_name, int pkg_head_len=0)
    {
        m_ha = ha;
        m_addr_name    = addr_name;
        m_pkg_head_len = pkg_head_len;
    }

    host_addr m_ha;
    int m_addr_name;
    int m_pkg_head_len;
};

class conf
{
public:
    conf();
    void add_tcp_ha(const host_addr &ha, int addr_name, int pkg_head_len=0);
    void add_tcp_ha(const std::string &ip, uint16_t port, int addr_name, int pkg_head_len=0);

    void add_udp_ha(const host_addr &ha, int addr_name);
    void add_udp_ha(const std::string &ip, uint16_t port, int addr_name);

public:
    uint32_t get_total_socket_num() const;
    uint32_t get_tcp_srv_num() const;
    uint32_t get_udp_srv_num() const;

    typedef std::vector<srv_addr_conf> ha_list_type;
    ha_list_type tcp_srv_ha_list;

    ha_list_type udp_srv_ha_list;

    int max_conn_num;
    int timeout;
};

} // end of namespace
#endif
