#ifndef _SRV_FRAMEWORK_H_
#define _SRV_FRAMEWORK_H_

#include "epoll_wrapper.h"
#include "host_addr.h"
#include "udp_socket.h"
#include "tcp_socket.h"
#include "conf.h"
#include <map>

namespace tiny_srv{

const int MAX_BUFFER_LEN = 4096;

struct sock_context
{
    uint32_t id;
    host_addr addr;
    size_t recv_buf_size;
    size_t send_buf_size;
};

class udp_context
{
public:
    udp_context(): m_recv_buf(NULL) {}

    void create_buf(int buf_len = MAX_BUFFER_LEN)
    {
		m_recv_buf = new char[buf_len];
    }

    void release_buf()
    {
		if(m_recv_buf != NULL)
		{
		   delete m_recv_buf;
		   m_recv_buf = NULL;
		}
    }

public:
    sock_context m_conf;
	udp_socket m_sock;
    char *m_recv_buf;
};

typedef enum
{
	SOCKET_UNUSED,
	SOCKET_TCP_ACCEPT,
	SOCKET_TCP_LISTEN,
	SOCKET_TCP_CONNECTING,
	SOCKET_TCP_CONNECTED,
	SOCKET_TCP_RECONNECT_WAIT
} tcp_context_status;


class tcp_context
{
public:
	tcp_context(int sock_fd = -1): 
        m_sock(sock_fd), 
        m_recv_buf(NULL), m_send_buf(NULL), 
        m_bytes_recv(0),  m_bytes_tosend(0),
        m_pkg_head_len(0), m_pkg_len(0),
        m_status(SOCKET_UNUSED)
   	{}

	void create_buf(int recv_buf_len = MAX_BUFFER_LEN, int send_buf_len = MAX_BUFFER_LEN)
	{
		m_recv_buf = new char[recv_buf_len];
		m_send_buf = new char[send_buf_len];
	}

	void release_buf()
	{
		if(m_recv_buf != NULL)
		{
			delete m_recv_buf;
			m_recv_buf = NULL;
		}

		if(m_send_buf != NULL)
		{
			delete m_send_buf;
			m_send_buf = NULL;
		}
	}

public:
	sock_context m_conf;
	tcp_socket   m_sock;

	char *m_recv_buf;
	char *m_send_buf;
	int  m_bytes_recv;
	int  m_bytes_tosend;
	int  m_pkg_head_len;
    int  m_pkg_len;

	tcp_context_status m_status;
};
    

class srv_framework
{
public:
    virtual ~srv_framework();
	bool run(const conf &srv_conf);

protected:
    virtual bool initialize(const conf &srv_conf);

	virtual bool handle_loop() = 0;

    // for udp
	bool init_udp_context(udp_context &udp_if, const host_addr &addr, int udp_name);

    virtual int32_t handle_udp_packet(uint32_t sock_name, int sock_fd, 
                                      const host_addr &ha, const char* buf, size_t buf_len);

	bool send_udp_packet(uint32_t sock_name, int sock_fd, 
                         const char* buf, size_t buf_len, 
                         const host_addr &ha);

    // for tcp
	bool init_tcp_listen_socket(const host_addr &addr, int tcp_name, 
                                int pkg_head_len, tcp_context &tcp_if);
	bool init_tcp_socket_addr(uint32_t sock_name, const host_addr &addr, 
                              int pkg_head_len, tcp_context &tcp_if);

    virtual int32_t handle_tcp_accept(tcp_context &tcp_if, uint32_t sock_name);
    int32_t handle_tcp_read(tcp_context &tcp_if);
    virtual int32_t handle_tcp_head(const tcp_context &tcp_if, const char *buffer, 
                                    int sock_fd, const host_addr &ha, int &pkg_len);

    virtual int32_t handle_tcp_package(uint32_t sock_name, tcp_context &tcp_if,
                                       const host_addr &ha,
                                       const char* buf, size_t buf_len);
	bool send_tcp_packet(tcp_context &tcp_if, const char* buf, size_t buf_len);
    virtual int32_t close_connection(tcp_context &tcp_if, uint32_t sock_name, const char *err_msg);                                      

private:
    bool _initialize_(const conf &srv_conf);
	bool create_listen_socket();
    void _handle_epoll_(int timeout);

    bool _handle_udp_packet_(udp_context &udp_if);

	bool _handle_tcp_accept_(tcp_context &listen_tcp_if);
    bool _close_connection_(tcp_context &tcp_if, const char *err_msg);
   
protected:
    epoll_wrapper m_epoll;
	typedef std::map<int, udp_context> udp_context_type;
    udp_context_type m_udp_context;

	typedef std::map<int, tcp_context> tcp_context_type;
	tcp_context_type m_tcp_context;

private:
    conf m_conf;
};

} // end of namespace
#endif

