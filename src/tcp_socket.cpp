#include "tcp_socket.h"
#include "host_addr.h"
#include <sys/socket.h>

namespace tiny_srv{

tcp_socket::tcp_socket()
{}

tcp_socket::tcp_socket(int fd)
    :socket(fd)
{}

bool tcp_socket::create()
{
    m_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);   
    return is_valid();
}

bool tcp_socket::listen(int backlog)
{
    ssize_t ret = ::listen(m_sockfd, backlog);
    return ret == 0;    
}

int tcp_socket::accept(host_addr &ha)
{
    sockaddr sa;
    socklen_t sock_len = sizeof(sockaddr);
    ssize_t ret = ::accept(m_sockfd, &sa, &sock_len);

    if(ret >= 0){ ha.set(sa); }
    return ret;
}

ssize_t tcp_socket::recv(char *buf, size_t buf_len, int flags)
{
    ssize_t ret = ::recv(m_sockfd, buf, buf_len, flags);
    return ret;
}

ssize_t tcp_socket::send(const char *buf, size_t buf_len, int flags)
{
    return ::send(m_sockfd, buf, buf_len, flags);
}

} // end of namespace
