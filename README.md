# Tiny server framework
### 简介
Tiny server framework是C++实现的异步网络框架，底层基于epoll，目前只支持Linux，IPV4，功能比较简单

使用Tiny server framework只需继承基类，实现自己handle_udp_packet/handle_tcp_accept/handle_tcp_package/handle_loop等几个函数，即可完成TCP/UDP网络操作，无需调用方关心网络通信细节。

目前还在开发中，功能不完备

作者：Zhitong Lei (leizhitong@foxmail.com)

### 特性
- 支持TCP/UDP
- 基于epoll的事件机制（ET模式）
- 单进程，单线程
- 异步，基于事件回调

## 性能
### 环境
    CPU: 24 x Intel(R) Xeon(R) CPU E5-2420 0 @ 1.90GHz
    Memory: 64 GB
    Network: Gigabit Ethernet

### UDP性能
使用/demo/benchmark进行benchmark，启动50个client，每个client发送1w个包进行测试


    ./benchmark -h <your ip> -p 51755 -u -c 50 -n 10000
    Concurrency: 50
    Requests: 500000
    Request_finished: 500000
    Total latency: 10.19 seconds (QPS: 49048)
    Percentage of the requests served within a certain time:
    428332 (85.67%) <= 1 milliseconds
    499899 (99.98%) <= 2 milliseconds
    499926 (99.99%) <= 3 milliseconds
    499943 (99.99%) <= 4 milliseconds
    499979 (100.00%) <= 5 milliseconds
    499981 (100.00%) <= 6 milliseconds
    500000 (100.00%) <= 7 milliseconds
    
    
### TCP性能
等待测试

## 代码结构
**include:** 项目所有头文件都在此目录（后续会调整，此目录最终只会包含public接口相关头文件，private接口用户将不可见）

**src:** Tiny server framework的具体实现

**demo:** 包含了**TCP、UDP server**的简单示例，以及**benchmark**程序代码

## 如何编译
### 编译环境
- Linux
- GCC-4.8 or above

### 编译 libtinysrv.a
进入根目录

    make
    
即可在根目录得到 libtinysrv.a

## 如何使用
可参见 /demo/test_tcp_server.cpp 

只需继承基类srv_framework，然后自定义业务相关收包处理函数、建立连接处理函数即可，以tcp server为例（udp server类似）：
```
#include "srv_framework.h"
#include "host_addr.h"
#include "protocol.h"

using namespace tiny_srv;

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

test_srv srv;
conf srv_conf;
srv_conf.add_tcp_ha("127.0.0.1", 51755, 1, 4);
srv.run(srv_conf);
```

## TODO
- 限制最大连接数
- TCP模式发送数据改为异步（目前是直接调用send system call，有可能会阻塞，若socket不可写，应缓存数据，等到epoll返回可写时再写）
- TCP增加超时功能，支持自动关闭超时连接