INC_DIR = -I../include
LIB_INSTALL_PATH = ../lib

# -pg for gprof, -g for gdb
#C_ARGS = -pg -g -Wall #-D _DEBUG_
C_ARGS = -Wall #-D _DEBUG_
CXX = g++
CXX11 = g++ -std=c++11 -static-libgcc -static-libstdc++
CC  = gcc

# $^ 代表所有的是的通过目录搜索得到的依赖文件的完整路径名（目录+一般文件名）列表
# $@ 代表规则的目标

.cpp.o:
	$(CXX11) $(C_ARGS) $(INC_DIR) -c $^

.c.o:
	$(CC) $(C_ARGS) $(INC_DIR) -c $^


BINARY = test_srv
LIB = libtinysrv.a

OBJECT:= protocol.o socket.o host_addr.o conf.o udp_socket.o tcp_socket.o epoll_wrapper.o srv_framework.o blocking_queue.o log.o

all:$(LIB)

blocking_queue.o:
	$(CXX11) $(C_ARGS) ../include/blocking_queue.h -c -o blocking_queue.o

libtinysrv.a: $(OBJECT)
	ar -rv $@ $^

install:
	cp -R -f $(LIB) $(LIB_INSTALL_PATH)

clean:
	rm -f *.o $(LIB)
