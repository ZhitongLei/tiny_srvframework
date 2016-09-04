#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#include "socket.h"
#include <sys/types.h>

namespace tiny_srv{

class udp_socket : public socket
{
public:
    bool create();
    ssize_t recvfrom(char *buf, size_t buf_len, host_addr &ha, int flags = 0);
    ssize_t sendto(const char *buf, size_t buf_len, const host_addr &ha, int flags = 0);
};

} // end of namespace
#endif
