#ifndef _EPOLL_WRAPPER_H_
#define _EPOLL_WRAPPER_H_

#include <sys/epoll.h>
#include <utility>

namespace tiny_srv{

#define SERVER_FLAG_READ  EPOLLIN
#define SERVER_FLAG_WRITE EPOLLET
#define SERVER_FLAG_ERROR EPOLLERR


class epoll_wrapper
{
public:
    enum{
        EPOLL_BLOCK = -1,
        EPOLL_NOWAIT = 0
    };

    epoll_wrapper();
    ~epoll_wrapper();

    bool create(int max_fd_num);
    bool ctl_fd(int fd, int option, int flag);
    bool add_fd(int fd, int flag = SERVER_FLAG_READ | SERVER_FLAG_WRITE);
    bool delete_fd(int fd);

    typedef std::pair<epoll_event*, uint32_t> result;
    result wait(int wait_ms = EPOLL_BLOCK);
    

private:
    int     m_epollfd;
    int     m_max_fd_num;
    struct  epoll_event *m_events;
};

} // end of namespace
#endif
