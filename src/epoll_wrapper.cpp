#include "epoll_wrapper.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

namespace tiny_srv{

epoll_wrapper::epoll_wrapper()
: m_epollfd(-1),
  m_max_fd_num(0),
  m_events(0)
{}


epoll_wrapper::~epoll_wrapper()
{
    if(m_events != NULL)
    {
        delete[] m_events;
        m_events = NULL;
    }

    ::close(m_epollfd);
}


bool epoll_wrapper::create(int max_fd_num)
{
    if(max_fd_num <= 0)
        return false;

    m_max_fd_num = max_fd_num;
    m_epollfd = ::epoll_create(max_fd_num);

    if(m_epollfd < 0)
        return false;

    m_events = new epoll_event[max_fd_num];
    return true;
}


bool epoll_wrapper::ctl_fd(int fd, int option, int flag)
{
    if(m_epollfd == -1)
        return false;

    epoll_event ev;
    ev.data.fd = fd;
    ev.events = flag;

    int ret = ::epoll_ctl(m_epollfd, option, fd, &ev);

    if(ret < 0)
    {
        int e = errno;
        if((e == EEXIST && option == EPOLL_CTL_ADD)
            || (e == ENOENT && option == EPOLL_CTL_DEL))
            return true;
        else
            return false;
    }

    return true;
}


bool epoll_wrapper::add_fd(int fd, int flag)
{
    return ctl_fd(fd, EPOLL_CTL_ADD, flag);
}


bool epoll_wrapper::delete_fd(int fd)
{
    return ctl_fd(fd, EPOLL_CTL_DEL, 0);
}

epoll_wrapper::result epoll_wrapper::wait(int wait_ms)
{
    int n = 0;
    do{
        n = ::epoll_wait(m_epollfd, m_events, m_max_fd_num, wait_ms);
        if(n < 0  && errno != EINTR)
        {
            return result(NULL, 0);
        }
    } while(n < 0);

    return result(m_events, n);
}

} // end of namespace
