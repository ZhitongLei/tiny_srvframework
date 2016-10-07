#include <iostream>
#include <string>
#include <utility>
#include "blocking_queue.h"
#include "log.h"

using namespace tiny_srv;
using namespace std;

int main(int argc, char **argv)
{
    BlockingQueue<Buffer> queue;
    Buffer buffer(1024);
    buffer.Append("fsdfsdfds\n");
    buffer.Append("doudoudoudoudou\n");
    buffer.Append("leileilei\n");
    buffer.Append("kkkkk  lll\n");
    cout << "content: " << buffer.toString() << endl;
    queue.Push(buffer);
    /*
    for(int i=0; i < 20; ++i) {
        queue.Push(i);
    }*/

    while(!queue.empty()) {
        cout << "==============" << endl;
        Buffer b(queue.Pop());
        cout << "element: " << b.toString() << endl;
        cout << "current size: " << queue.size() << endl;
    }

    //queue.Pop();
    return 0;
}
