all: test_udp_server test_client test_tcp_server test_tcp_client protocol_use benchmark blocking_queue_use log_test

test_udp_server: test_udp_server.cpp libtinysrv.a
	g++ -g test_udp_server.cpp libtinysrv.a -I../include -o test_udp_server

test_tcp_server: test_tcp_server.cpp libtinysrv.a
	g++ -g test_tcp_server.cpp libtinysrv.a -I../include -o test_tcp_server

test_tcp_client: test_tcp_client.cpp libtinysrv.a
	g++ -g test_tcp_client.cpp libtinysrv.a -I../include -o test_tcp_client 

test_client: test_client.cpp libtinysrv.a
	g++ test_client.cpp libtinysrv.a -I../include -o test_client

protocol_use: protocol_use.cpp libtinysrv.a
	g++ protocol_use.cpp libtinysrv.a -I../include -o protocol_use

benchmark: benchmark.cpp libtinysrv.a
	g++ -g -std=c++11 -lpthread -static-libgcc -static-libstdc++ benchmark.cpp libtinysrv.a -I../include -o benchmark

blocking_queue_use: blocking_queue_use.cpp 
	g++ -g -std=c++11 -lpthread -static-libgcc -static-libstdc++ blocking_queue_use.cpp -I../include -o blocking_queue_use 

log_test: log_test.cpp libtinysrv.a
	g++ -std=c++11 -lpthread -static-libgcc -static-libstdc++ log_test.cpp libtinysrv.a -I../include -o log_test
#g++ -pg -g -std=c++11 -lpthread -static-libgcc -static-libstdc++ log_test.cpp libtinysrv.a -I../include -o log_test

clean:
	rm -f *.o test_udp_server test_client test_tcp_server test_tcp_client protocol_use benchmark blocking_queue_use log_test
