#include <iostream>
#include <string>
#include <unistd.h>

#include "udp_socket.h"
#include "host_addr.h"

using namespace tiny_srv;
using namespace std;

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		cout << "usage: ./" << argv[0] << " message" << endl;
		return 1;
	}

	udp_socket us;
	us.create();

	host_addr ha("127.0.0.1", 51755);
	string str(argv[1]);

	us.sendto(str.c_str(), str.length(), ha);

	char buf[512];
	int n = us.recvfrom(buf, sizeof(buf), ha);
	string resp(buf, n);

	cout << "receive: " << resp << endl;
	return 0;
}
