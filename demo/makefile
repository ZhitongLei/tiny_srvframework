all: test_server test_client test_tcp_server test_tcp_client protocol_use

test_server: test_server.cpp libtinysrv.a
	g++ -g test_server.cpp libtinysrv.a -I../include -o test_server

test_tcp_server: test_tcp_server.cpp libtinysrv.a
	g++ -g test_tcp_server.cpp libtinysrv.a -I../include -o test_tcp_server

test_tcp_client: test_tcp_client.cpp libtinysrv.a
	g++ -g test_tcp_client.cpp libtinysrv.a -I../include -o test_tcp_client 

test_client: test_client.cpp libtinysrv.a
	g++ test_client.cpp libtinysrv.a -I../include -o test_client

protocol_use: protocol_use.cpp libtinysrv.a
	g++ protocol_use.cpp libtinysrv.a -I../include -o protocol_use

clean:
	rm -f *.o test_server test_client test_tcp_server test_tcp_client protocol_use
