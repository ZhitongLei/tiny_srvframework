#include "udp_socket.h"
#include "host_addr.h"
#include <sys/socket.h>

namespace tiny_srv{

bool udp_socket::create()
{
    m_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);   
    return is_valid();
}

ssize_t udp_socket::recvfrom(char *buf, size_t buf_len, host_addr &ha, int flags)
{
    sockaddr sa;
    socklen_t sock_len = sizeof(sockaddr);
    ssize_t ret = ::recvfrom(m_sockfd, buf, buf_len, flags, &sa, &sock_len);
    if(ret >= 0)
    {
		ha.set(sa);	
    }
    return ret;
}


ssize_t udp_socket::sendto(const char *buf, size_t buf_len, const host_addr &ha, int flags)
{
    return ::sendto(m_sockfd, buf, buf_len, flags, ha.get_addr(), sizeof(sockaddr));
}

} // end of namespace
