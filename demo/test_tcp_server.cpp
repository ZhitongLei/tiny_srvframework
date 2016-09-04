// File Name: ../demo/test_tcp_server.cpp
// Author: lei
// Created Time: 2016-03-27 22:32:50

#include "srv_framework.h"
#include "host_addr.h"
#include "protocol.h"
#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

using namespace tiny_srv;

const int TIMEOUT = 2000;

class test_srv : public srv_framework
{
protected:
	bool handle_loop()
	{
		cout << "calling handle_loop" << endl;
		return true;
	}

    int32_t handle_tcp_accept(tcp_context &tcp_if, uint32_t sock_name)
    {
        cout << "connect new socket" << endl;
        return 0;
    }

    int32_t handle_udp_packet(uint32_t sock_name, int sock_fd, const host_addr &ha,
                              const char* buf, size_t buf_len)
    {
        cout << "enter handle_udp_packet" << endl;
        return 0;
    }

    int32_t handle_tcp_package(uint32_t sock_name, tcp_context &tcp_if, const host_addr &ha,
                               const char* buf, size_t buf_len)
    {
        cout << "enter handle_tcp_package" << endl;
        BaseProtocol p;
        string package(buf, buf_len);
        if(p.Unpackage(package)){
            cout << "receive: " << p.body() << endl; 
        }

        string response("I had receive your message: " + p.body());
        p.set_body(response);
        string buffer;
        if(p.Package(buffer))
        {
            send_tcp_packet(tcp_if, buffer.c_str(), buffer.size());
        }
        
        return 0;
    }

};

int main()
{
	test_srv srv;
	conf srv_conf;
	srv_conf.add_tcp_ha("127.0.0.1", 51755, 1, 4);
	srv.run(srv_conf);
    return 0;
}


