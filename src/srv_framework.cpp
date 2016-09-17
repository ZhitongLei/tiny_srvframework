#include "srv_framework.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>

using namespace std;

namespace tiny_srv{

srv_framework::~srv_framework()
{
    udp_context_type::iterator udp_it;
    for(udp_it = m_udp_context.begin(); udp_it != m_udp_context.end(); udp_it++)
    {
        udp_it->second.release_buf();
        udp_it->second.m_sock.close();
        m_epoll.delete_fd(udp_it->second.m_sock.get_fd());
    }

    tcp_context_type::iterator tcp_it;
    for(tcp_it = m_tcp_context.begin(); tcp_it != m_tcp_context.end(); tcp_it++)
    {
        tcp_it->second.release_buf();
        tcp_it->second.m_sock.close();
        m_epoll.delete_fd(tcp_it->second.m_sock.get_fd());
    }
}


bool srv_framework::init_udp_context(udp_context &udp_if, const host_addr &addr, int udp_name)
{
    udp_if.m_conf.id = udp_name;
    udp_if.m_conf.addr = addr;
    udp_if.m_conf.recv_buf_size = MAX_BUFFER_LEN;
    udp_if.m_conf.send_buf_size = 0;
    udp_if.create_buf(udp_if.m_conf.recv_buf_size);

    if(!udp_if.m_sock.create()){
        // need log
        cout << "srv_framework::add_udp_bind_addr: create udp socket failed" << endl;
        return false;
    }

    if(!udp_if.m_sock.set_nonblock()){
        cout << "srv_framework::add_udp_bind_addr: set socket nonblock failed" << endl;
        return false;
    }
            
    if(!udp_if.m_sock.bind(udp_if.m_conf.addr)){
        // need log
        cout << "srv_framework::add_udp_bind_addr: create udp socket bind address failed" << endl;
        return false;
    }

    return true;
}


int32_t srv_framework::handle_udp_packet(uint32_t sock_name, int sock_fd, const host_addr &ha,
                                         const char* buf, size_t buf_len)
{
    return 0;
}


bool srv_framework::init_tcp_listen_socket(const host_addr &addr, int tcp_name, 
                                           int pkg_head_len, tcp_context &tcp_if)

{
    tcp_if.m_conf.id = tcp_name;
    tcp_if.m_conf.addr = addr;
    tcp_if.m_conf.recv_buf_size = MAX_BUFFER_LEN;
    tcp_if.m_conf.send_buf_size = MAX_BUFFER_LEN;
    tcp_if.create_buf(tcp_if.m_conf.recv_buf_size);
    tcp_if.create_buf(tcp_if.m_conf.send_buf_size);
    tcp_if.m_pkg_head_len = pkg_head_len;

    if(!tcp_if.m_sock.create()){
        // need log
        cout << "srv_framework::init_tcp_listen_socket: create tcp socket failed" << endl;
        return false;
    }

    if(!tcp_if.m_sock.set_nonblock()){
        cout << "srv_framework::init_tcp_listen_socket: set socket nonblock failed" << endl;
        return false;
    }
            
    if(!tcp_if.m_sock.bind(tcp_if.m_conf.addr)){
        // need log
        cout << "srv_framework::init_tcp_listen_socket: socket bind address failed" << endl;
        return false;
    }

    if(!tcp_if.m_sock.listen()){
        cout << "srv_framework::init_tcp_listen_socket: tcp socket listen failed" << endl;
        return false;
    }

    /* change socket status to listening */
    tcp_if.m_status = SOCKET_TCP_LISTEN;
    return true;
}


bool srv_framework::init_tcp_socket_addr(uint32_t sock_name, const host_addr &addr, 
                                    int pkg_head_len, tcp_context &tcp_if)
{
    tcp_if.m_conf.id = sock_name;
    tcp_if.m_conf.addr = addr;
    tcp_if.m_conf.recv_buf_size = MAX_BUFFER_LEN;
    tcp_if.m_conf.send_buf_size = MAX_BUFFER_LEN;
    tcp_if.create_buf(tcp_if.m_conf.recv_buf_size);
    tcp_if.create_buf(tcp_if.m_conf.send_buf_size);
    tcp_if.m_pkg_head_len = pkg_head_len;

    /*
    if(!tcp_if.m_sock.create()){
        // TODO: log
        cout << "srv_framework::init_tcp_socket create tcp socket failed" << endl;
        return false;
    }*/

    /*
    if(!tcp_if.m_sock.set_nonblock()){
        cout << "srv_framework::init_tcp_socket set socket nonblock failed" << endl;
        return false;
    }*/

    return true;
}


bool srv_framework::create_listen_socket()
{
    conf::ha_list_type::size_type sz  = m_conf.udp_srv_ha_list.size();
    const conf::ha_list_type &udp_srv_ha_list = m_conf.udp_srv_ha_list;
    for(uint32_t i = 0; i < sz; i++)
    {
        udp_context udp_if;
        if(!init_udp_context(udp_if, udp_srv_ha_list[i].m_ha, 
                             udp_srv_ha_list[i].m_addr_name)){ return false; }
        m_udp_context.insert(std::pair<int,udp_context>(udp_if.m_sock.get_fd(), udp_if));

        if(!m_epoll.add_fd(udp_if.m_sock.get_fd()))
        {
            cout << "srv_framework::create_listen_socket: m_epoll udp_socket failed" << endl;
            return false;
        }
    }

    sz = m_conf.tcp_srv_ha_list.size();
    const conf::ha_list_type &tcp_srv_ha_list = m_conf.tcp_srv_ha_list;
    for(uint32_t i = 0; i < sz; i++)
    {
        tcp_context tcp_if;
        if(!init_tcp_listen_socket(tcp_srv_ha_list[i].m_ha, 
                                   tcp_srv_ha_list[i].m_addr_name, 
                                   tcp_srv_ha_list[i].m_pkg_head_len, tcp_if)){ 
            return false; 
        }
        m_tcp_context.insert(std::pair<int,tcp_context>(tcp_if.m_sock.get_fd(), tcp_if));

        if(!m_epoll.add_fd(tcp_if.m_sock.get_fd()))
        {
            cout << "srv_framework::create_listen_socket: m_epoll tcp_socket failed" << endl;
            return false;
        }
    }
    
    return true;
}


bool srv_framework::_initialize_(const conf &srv_conf)
{
    if(!initialize(srv_conf))
    {
        cout << "srv_framework::_initialize_ error" << endl;
        return false;
    }
       

    if(m_conf.get_total_socket_num() == 0)
    {
        cout << "srv_framework::_initialize_ error:zero server number" << endl;
        return false;
    }

    return true;
}


bool srv_framework::initialize(const conf &srv_conf)
{
    m_conf = srv_conf;
    return true;
}

int32_t srv_framework::handle_tcp_accept(tcp_context &tcp_if, uint32_t sock_name)
{
    return 0;
}


bool srv_framework::_handle_tcp_accept_(tcp_context &listen_tcp_if)
{
    while(true){
        host_addr client_ha;
        int accepted_sock_fd = 0;
        if((accepted_sock_fd = listen_tcp_if.m_sock.accept(client_ha)) < 0){
            if(errno != EAGAIN && errno != EWOULDBLOCK){
                fprintf(stderr, "%s socket accept failed with errno: %d tcp_id: %d\n", 
                        __func__, errno, listen_tcp_if.m_conf.id);
            }
            break;
        }

        tcp_context client_tcp_if(accepted_sock_fd);
        if(!client_tcp_if.m_sock.set_nonblock()){
            cout << "srv_framework::init_tcp_socket set socket nonblock failed" << endl;
            return false;
        }

        bool ret = init_tcp_socket_addr(listen_tcp_if.m_conf.id, 
                                        client_ha, 
                                        listen_tcp_if.m_pkg_head_len, 
                                        client_tcp_if);
        if(!ret){ return false; }

        client_tcp_if.m_status = SOCKET_TCP_ACCEPT;
        m_tcp_context.insert(std::pair<int, tcp_context>(client_tcp_if.m_sock.get_fd(), 
                                                         client_tcp_if));
        if(!m_epoll.add_fd(client_tcp_if.m_sock.get_fd())){
            fprintf(stderr, "%s m_epoll.add_fd failed\n", __func__);
            _close_connection_(client_tcp_if, "epoll add fd failed");
            return false;
        }

        if(0 != handle_tcp_accept(listen_tcp_if, listen_tcp_if.m_conf.id)){
            _close_connection_(client_tcp_if, "handle_tcp_accept failed");
            return false;
        }
    } // end of while(...)

    fprintf(stdout, "here: %s\n", __func__);
    return true;
}

int32_t srv_framework::handle_tcp_package(uint32_t sock_name, tcp_context &tcp_if, const host_addr &ha,
                                          const char* buf, size_t buf_len)
{
    return 0;
}


int32_t srv_framework::close_connection(tcp_context &tcp_if, uint32_t sock_name, const char *err_msg)
{
    return 0;
}

bool srv_framework::_close_connection_(tcp_context &tcp_if, const char *err_msg)
{
    close_connection(tcp_if, tcp_if.m_conf.id, err_msg); 
    tcp_if.m_status = SOCKET_UNUSED;
    tcp_if.m_pkg_head_len = 0;
    tcp_if.m_pkg_len = 0;
    tcp_if.m_bytes_recv = 0;
    tcp_if.m_bytes_tosend = 0;
    tcp_if.release_buf();

    int fd = tcp_if.m_sock.get_fd();
    if(!m_epoll.delete_fd(fd)){
        fprintf(stderr, "%s epoll delete fd error\n", __func__);
    }

    int ret = tcp_if.m_sock.close();
    if(ret){
        fprintf(stderr, "close socket failed, errno: %d\n", errno);
    }

    // do this at last
    m_tcp_context.erase(fd);

    if(err_msg){
        fprintf(stderr, "close socket, error message: %s\n", err_msg);
    }
    return true;
}

int32_t srv_framework::handle_tcp_head(const tcp_context &tcp_if, const char *buffer, 
                                       int sock_fd, const host_addr &ha, int &pkg_len)
{
    if(NULL == buffer){
        fprintf(stderr, "%s: empty buffer\n", __func__);
        return -1;
    }

    if(*buffer != SRV_STX){
        fprintf(stderr, "%s: receive buffer not start with SRV_STX\n", __func__);
        return -1;
    }

    uint32_t stx_len = sizeof(SRV_STX);
    ProtocolHead protocol_head = *reinterpret_cast<ProtocolHead*>(const_cast<char*>(buffer + stx_len));
    Net2Host(protocol_head);
    pkg_len = protocol_head.package_length;
    return 0;
}


int32_t srv_framework::handle_tcp_read(tcp_context &tcp_if)
{
    int buffer_len = tcp_if.m_conf.recv_buf_size;
    int head_len = tcp_if.m_pkg_head_len;
    int sock_name = tcp_if.m_conf.id;
    int sock_fd = tcp_if.m_sock.get_fd();
    host_addr &ha = tcp_if.m_conf.addr;

    fprintf(stdout, "enter %s\n", __func__);
    char *buffer = tcp_if.m_recv_buf;
    int &bytes_recv = tcp_if.m_bytes_recv;
    int &package_len = tcp_if.m_pkg_len;

    while(true){
        int ret = tcp_if.m_sock.recv(buffer + bytes_recv, buffer_len, 0); 
        if(ret < 0){
            if(errno != EAGAIN && errno != EWOULDBLOCK){
                fprintf(stderr, "recv failed with code %d errno %d tcp_id %d\n", 
                        ret, errno, tcp_if.m_conf.id);
                _close_connection_(tcp_if, NULL);
                return -1;
            }
            break;
        }
        else if(0 == ret){
            // the peer has performed an orderly shutdown
            _close_connection_(tcp_if, "orderly shutdown");
            break;
        }

        bytes_recv += ret;
        if(0 == package_len){
           if(bytes_recv < head_len){ continue; }
           handle_tcp_head(tcp_if, buffer, sock_fd, ha, package_len);
        }
        if(package_len > buffer_len){
            fprintf(stderr, "%s: too long package, package_len: %d recv_buffer_len: %d\n",
                    __func__, package_len, buffer_len);
            _close_connection_(tcp_if, "too long package");
            return -1;
        }

        // recvive complete package
        if(bytes_recv >= package_len){
            ret = handle_tcp_package(sock_name, tcp_if, ha, buffer, package_len);
            if(ret){
                fprintf(stderr, "handle_tcp_package failed with code %d tcp_id %d buffer_len %d",
                        ret, sock_name, buffer_len);
                _close_connection_(tcp_if, "handle_tcp_package error");
                return -1;                        
            }

            bytes_recv -= package_len;
            memmove(tcp_if.m_recv_buf, tcp_if.m_recv_buf + package_len, bytes_recv);
            package_len = 0;
            //if(0 == bytes_recv){ break; }
        }
    } // end of while(...)
    
    return 0;
}

bool srv_framework::send_tcp_packet(tcp_context &tcp_if, const char* buf, size_t buf_len)
{
    return tcp_if.m_sock.send(buf, buf_len, 0) != -1;
}

/*
int32_t srv_framework::handle_tcp_read(tcp_context &tcp_if)
{
    int buffer_len = tcp_if.m_conf.recv_buf_size;
    int stx_len = sizeof(SRV_STX), etx_len = sizeof(SRV_ETX);
    int head_len = tcp_if.m_pkg_head_len;
    int sock_name = tcp_if.m_conf.id;
    int sock_fd = tcp_if.m_sock.get_fd();
    host_addr &ha = tcp_if.m_conf.addr;

    fprintf(stdout, "enter %s\n", __func__);

    while(true){
        char *buffer = tcp_if.m_recv_buf;
        int &bytes_recv = tcp_if.m_bytes_recv;
        int &message_body_len = tcp_if.m_pkg_len;

        int ret = tcp_if.m_sock.recv(buffer + bytes_recv, buffer_len, 0); 
        if(ret < 0){
            if(errno != EAGAIN && errno != EWOULDBLOCK){
                fprintf(stderr, "recv failed with code %d errno %d tcp_id %d\n", 
                        ret, errno, tcp_if.m_conf.id);
                _close_connection_(tcp_if, NULL);
                return -1;
            }
            break;
        }
        else if(0 == ret){
            // the peer has performed an orderly shutdown
            _close_connection_(tcp_if, "orderly shutdown");
            break;
        }

        bytes_recv += ret;
        // check package begin flag
        if(*buffer != SRV_STX){
            _close_connection_(tcp_if, "bad package format, can't find STX");
            return -1;
        }
  
        if(0 == message_body_len){
           if(bytes_recv < (head_len + stx_len)){ continue; }
           handle_tcp_head(tcp_if, buffer + stx_len, sock_fd, ha, message_body_len);
        }

        int package_total_len = stx_len + head_len + message_body_len + etx_len;
        if(package_total_len > buffer_len){
            fprintf(stderr, "too long package, package_len: %d recv_buffer_len: %d\n",
                    package_total_len, buffer_len);
            _close_connection_(tcp_if, "too long package");
            return -1;
        }

        // recvive complete package
        if(bytes_recv >= package_total_len){
            // check package end flag
            if(*(buffer + package_total_len - 1) != SRV_ETX){
                _close_connection_(tcp_if, "bad package format, can't find ETX");
                return -1;
            }

            ret = handle_tcp_package(sock_name, sock_fd, ha, 
                                     buffer + head_len + stx_len, 
                                     message_body_len);
            if(ret){
                fprintf(stderr, "handle_tcp_package failed with code %d tcp_id %d buffer_len %d",
                        ret, sock_name, buffer_len);
                _close_connection_(tcp_if, "handle_tcp_package error");
                return -1;                        
            }

            message_body_len = 0;
            bytes_recv -= package_total_len;
            if(0 == bytes_recv){ break; }

            memmove(tcp_if.m_recv_buf, tcp_if.m_recv_buf + package_total_len, bytes_recv);
        }

    } // end of while(...)
    
    return 0;
}
*/

void srv_framework::_handle_epoll_(int timeout)
{
    epoll_wrapper::result epoll_ret;

    epoll_ret = m_epoll.wait(timeout);
    
    if(epoll_ret.first){
        for(uint32_t i = 0; i < epoll_ret.second; i++){
            epoll_event &ev = epoll_ret.first[i];

            /*
            if(ev.events & EPOLLOUT){
                tcp_context_type::iterator tcp_it = m_tcp_context.find(ev.data.fd);
                if(tcp_it != m_tcp_context.end()){
                    _close_connection_(tcp_it->second, "peer performed orderly shutdown");
                    continue;
                }
            }*/

#ifdef _DEBUG_
            fprintf(stdout, "epoll_wait socket i= %d, fd=%d, events=0x%x\n", 
                    i, ev.data.fd, ev.events);
#endif

            /*
            if(ev.events & EPOLLHUP){
                tcp_context_type::iterator tcp_it = m_tcp_context.find(ev.data.fd);
                if(tcp_it != m_tcp_context.end()){
                    fprintf(stdout, "encounter EPOLLHUP\n");
                    //::close(tcp_it->second.m_sock.get_fd());
                    _close_connection_(tcp_it->second, "perr close");
                }
                continue;
            }*/

            if(ev.events & SERVER_FLAG_READ){ //  || ev.events & EPOLLHUP){
                udp_context_type::iterator udp_it = m_udp_context.find(ev.data.fd);
                if(udp_it != m_udp_context.end()){
                    _handle_udp_packet_(udp_it->second);
                }

                tcp_context_type::iterator tcp_it = m_tcp_context.find(ev.data.fd);
                if(tcp_it != m_tcp_context.end()){
                    switch(tcp_it->second.m_status){
                        case SOCKET_TCP_LISTEN:
                            _handle_tcp_accept_(tcp_it->second);
                            break;
                        case SOCKET_TCP_ACCEPT:
                            handle_tcp_read(tcp_it->second);
                            break;
                        default:
                            break;
                    } // end of switch(...)
                }

            }
        } // end of for(uint32_t i, ...)
    }
}


bool srv_framework::_handle_udp_packet_(udp_context &udp_if)
{
    char       *buf    = udp_if.m_recv_buf;
    size_t     buf_len = udp_if.m_conf.recv_buf_size;
    host_addr  &ha     = udp_if.m_conf.addr;

    // use edge-triggered behavior of epoll
    // so we need to read until EAGAIN or EWOULDBLOCK
    while(true){
        ssize_t n = udp_if.m_sock.recvfrom(buf, buf_len, ha);
        if(n <= 0){
            if(errno != EAGAIN && errno != EWOULDBLOCK){
                fprintf(stderr, "%s recvfrom failed with code %d udp_id %d\n", 
                        __func__, errno, udp_if.m_conf.id);
                return false;
            }
            break;
        }

        int sock_name = udp_if.m_conf.id;
        int sock_fd   = udp_if.m_sock.get_fd();
        handle_udp_packet(sock_name, sock_fd, ha, buf, n);
    } // end of while(...)

    return true;
}


bool srv_framework::send_udp_packet(uint32_t sock_name, int sock_fd, 
                                    const char* buf, size_t buf_len, const host_addr &ha)
{
    udp_context_type::iterator it = m_udp_context.find(sock_fd);

    if(it != m_udp_context.end())
    {
        return it->second.m_sock.sendto(buf, buf_len, ha) != -1;
    }    

    // need log
    cout << "srv_framework::send_udp_packet: can't find fd " << sock_fd << endl;
    return false;
}

bool srv_framework::run(const conf &srv_conf)
{
    if(! _initialize_(srv_conf)){ return false; }

    if(!m_epoll.create(m_conf.get_total_socket_num())){
        cout << "srv_framework: m_epoll.create() failed" << endl;
        return false;
    }

    if(!create_listen_socket()){ return false; }

    while(true){
        _handle_epoll_(srv_conf.timeout);
        handle_loop();
    }

    return true;
}

} // end of namespace
