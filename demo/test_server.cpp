#include "srv_framework.h"
#include "host_addr.h"
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
    int32_t handle_udp_packet(uint32_t sock_name, int sock_fd, const char* buf, size_t buf_len, const host_addr &ha)
	{
		string str(buf, buf_len);
		cout << str << endl;

		if(!send_udp_packet(sock_name, sock_fd, str.c_str(), str.length(), ha))
		{
			cout << "send respon error" << endl;
			return -1;
		}

		return 0;
	}

	bool handle_loop()
	{
		cout << "calling handle_loop" << endl;
		return true;
	}

};

int main()
{
	test_srv srv;
	conf srv_conf;
	srv_conf.add_udp_ha("127.0.0.1", 51755, 0);
	srv.run(srv_conf);
    return 0;
}


