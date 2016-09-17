#include "host_addr.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

namespace tiny_srv{

host_addr::host_addr()
{
   ::memset(&m_addr, 0, sizeof(sockaddr_in));
}

host_addr::host_addr(const std::string &ip , uint16_t port)
{
    set(ip, port);
}

void host_addr::set(const std::string &ip, uint16_t port)
{
    ::memset(&m_addr, 0, sizeof(sockaddr_in));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    m_addr.sin_port = htons(port);
}

void host_addr::set(const struct sockaddr &sa)
{
    ::memcpy(&m_addr, &sa, sizeof(struct sockaddr));
}

const sockaddr_in* host_addr::get_addr_in() const
{
    return &m_addr;
}

const sockaddr* host_addr::get_addr() const
{
    return (sockaddr*) &m_addr;
}

std::string host_addr::get_ip() const
{
    return std::string(inet_ntoa(m_addr.sin_addr));
}

uint16_t host_addr::get_port() const
{
    return ntohs(m_addr.sin_port);
}

host_addr& host_addr::operator=(const host_addr& ha)
{
    if(this != &ha)
    {
        ::memcpy(&m_addr, &ha.m_addr, sizeof(struct sockaddr));
    }
    return *this;
}

} // end of namespace
