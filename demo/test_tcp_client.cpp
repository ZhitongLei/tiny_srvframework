// File Name: ../demo/test_tcp_client.cpp
// Author: lei
// Created Time: 2016-05-15 22:40:21


#include <iostream>
#include <string>
#include <unistd.h>
#include <errno.h>

#include "tcp_socket.h"
#include "host_addr.h"
#include "protocol.h"

using namespace tiny_srv;
using namespace std;

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		cout << "usage: ./" << argv[0] << " message" << endl;
		return 1;
	}

	tcp_socket ts;
	if(!ts.create()){ cout << "create error, errno: " << errno << endl; }

    //ts.set_nonblock();

	host_addr ha("127.0.0.1", 51755);
    if(!ts.connect(ha)){ cout << "connect error, errno: " << errno << endl; }
	string str(argv[1]);
    
    BaseProtocol p; p.set_body(str);
    string request;
    p.Package(request);

    ts.send(request.c_str(), request.length());
	char buf[512];
    int n = ts.recv(buf, sizeof(buf));
    string resp(buf, n);
    if(p.Unpackage(resp)){
        cout << "receive: " << p.body() << endl;
        if(ts.close()){ cout << "close socket error, errno: " << errno << endl; }
    }
    //sleep(200);

    /*
	us.sendto(str.c_str(), str.length(), ha);

	char buf[512];
	int n = us.recvfrom(buf, sizeof(buf), ha);
	string resp(buf, n);

	cout << "receive: " << resp << endl;
    */
	return 0;
}
