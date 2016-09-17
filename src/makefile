INC_DIR = -I../include

C_ARGS = -g -Wall #-D _DEBUG_
CXX = g++
CC  = gcc

# $^ 代表所有的是的通过目录搜索得到的依赖文件的完整路径名（目录+一般文件名）列表
# $@ 代表规则的目标

.cpp.o:
	$(CXX) $(C_ARGS) $(INC_DIR) -c $^

.c.o:
	$(CC) $(C_ARGS) $(INC_DIR) -c $^

BINARY = test_srv 
LIB = libtinysrv.a

OBJECT:= protocol.o socket.o host_addr.o conf.o udp_socket.o tcp_socket.o epoll_wrapper.o srv_framework.o

all:$(LIB) 

libtinysrv.a: $(OBJECT)
	ar -rv $@ $^


clean:
	rm -f *.o $(LIB)


