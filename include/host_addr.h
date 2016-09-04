#ifndef _HOST_ADDR_H_
#define _HOST_ADDR_H_

#include <string>
#include <netinet/in.h>

namespace tiny_srv{

class host_addr
{
public:
    host_addr();
    host_addr(const std::string &ip, uint16_t port);
    void set(const std::string &ip, uint16_t port);
    void set(const struct sockaddr &sa);
    
    const sockaddr_in* get_addr_in() const;
    const sockaddr* get_addr() const;
    std::string get_ip() const;
    uint16_t get_port() const;

	host_addr& operator=(const host_addr& ha);

private:
    sockaddr_in m_addr;     
};

} // end of namespace
#endif
