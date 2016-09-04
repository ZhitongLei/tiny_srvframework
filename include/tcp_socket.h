#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include "socket.h"
#include <sys/types.h>

namespace tiny_srv{

class tcp_socket : public socket
{
public:
    tcp_socket();
    tcp_socket(int fd);
    bool create();
	bool listen(int backlog = 5);
	int accept(host_addr &ha);
    ssize_t recv(char *buf, size_t buf_len, int flags = 0);
	ssize_t send(const char *buf, size_t buf_len, int flags = 0);
};

} // end of namespace
#endif
