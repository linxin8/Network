# 一个简单的Linux C++ TCP 网络库

一个简单Linux C++ TCP网络库，实现了RPC注册、RPC同步/异步调用、序列化/反序列化、网络通信、异步日志。

## 样例

### RPC服务

rpc register server

```c++
#include "../src/rpc_register_server.h"
#include <thread>

int main()
{
    RPCRegisterServer server{2000};
    while (true)
    {
        CurrentThread::sleeps(10);
    }
}
```

rpc register client

```c++
#include "../src/rpc_register_client.h"
#include "../src/rpc_server.h"
#include <thread>
int main()
{
    RPCRegisterClient client{"localhost", 2000};
    client.registerFun("add", "localhost", 3000);

    RPCServer server{3000};
    server.registerFuction("add", [](Message message) {
        auto a = std::stoi(message["arg0"]);
        auto b = std::stoi(message["arg1"]);
        auto c = a + b;
        return Message::createResultMessage(std::to_string(c));
    });

    server.registerFuction("ping", [](Message message) {
        return Message::createResultMessage("pong");
    });

    server.registerFuction("echo", [](Message message) {
        return Message::createResultMessage(message["arg0"]);
    });

    server.registerFuction("get_time", [](Message message) {
        auto now = TimePoint::now();
        char str[60];
        sprintf(str, "%d:%d:%d", now.hours(), now.minute(), now.second());
        return Message::createResultMessage(str);
    });

    while (true)
    {
        CurrentThread::sleeps(1000);
    }
}
```

rpc server

```c++
#include "../src/rpc_server.h"
#include "../../../src/tcp_server.h"
#include <cstring>

int add(int a, int b)
{
    return a + b;
}

std::string getTime()
{
    auto now = TimePoint::now();
    char str[60];
    sprintf(str, "%d:%d:%d", now.hours(), now.minute(), now.second());
    return str;
}

Message rpcAdd(Message arg)
{
    LOG_ASSERT(arg["fun"] == "add");
    Message reply;
    try
    {
        int  a          = std::stoi(arg["arg0"]);
        int  b          = std::stoi(arg["arg1"]);
        auto result     = add(a, b);
        reply["status"] = "ok";
        reply["result"] = std::to_string(result);
    }
    catch (std::out_of_range& e)
    {
        reply["status"] = "error";
        reply["error"]  = "argument out of range";
    }
    catch (std::invalid_argument& e)
    {
        reply["status"] = "error";
        reply["error"]  = "invalid argument";
    }
    catch (std::exception& e)
    {
        reply["status"] = "error";
        reply["error"]  = e.what();
    }
    return std::move(reply);
}

Message rpcGetTime(Message)
{
    return Message::createResultMessage(getTime());
}

int main()
{
    RPCServer server{2019};
    server.registerFuction("add", rpcAdd);
    server.registerFuction("add", rpcGetTime);
    while (true)
    {
        CurrentThread::sleep(1000);
    }
}
```

rpc client

```c++
#include "src/rpc_client.h"

int main()
{
    RPCClient client{"localhost", 3000};
    int       a         = 0;
    int       b         = 1;
    auto      t1        = TimePoint::now();
    const int frequency = 100;
    int       n         = frequency;
    while (n--)
    {
        auto result = client.call(
            "add", {{"arg0", std::to_string(a)}, {"arg1", std::to_string(b)}});
        LOG_DEBUG() << result.toReadableString();
        int c = a + b;
        a     = c;
        b     = 1;
    }
    auto t2 = TimePoint::now();
    n       = frequency;
    std::vector<std::shared_ptr<RPCAsynResult>> result;
    while (n--)
    {
        auto r = client.callAsyn(
            "add", {{"arg0", std::to_string(a)}, {"arg1", std::to_string(b)}});
        result.push_back(r);
        int c = a + b;
        a     = c;
        b     = 1;
    }
    for (auto& r : result)
    {
        r->wait();
        LOG_DEBUG() << r->extract().toReadableString();
    }
    auto t3 = TimePoint::now();
    LOG_DEBUG() << "test case" << frequency << "call time"
                << t2.sub(t1).second() << "call asyn time"
                << t3.sub(t1).second();

    LOG_DEBUG() << "test get time fun"
                << client.call("get_time", {}).toReadableString();
    LOG_DEBUG() << "test ping fun"
                << client.call("ping", {}).toReadableString();
    LOG_DEBUG()
        << "test echo fun"
        << client.call("echo", {{"arg0", "2333ababa23333"}}).toReadableString();
    return 0;
}
```

### echo服务

```c++
#include "../../src/acceptor.h"
#include "../../src/event_loop.h"
#include "../../src/event_loop_thread.h"
#include "../../src/log.h"
#include "../../src/tcp_server.h"
#include "../../src/thread.h"
#include <iostream>

uint16_t port = 9001;

void serviceV2()
{
    TcpServer server{port};
    server.setSubReactorThreadNumber(2);
    std::vector<std::shared_ptr<TcpConnection>> cons;
    server.setOnReadyToRead([&](std::shared_ptr<TcpConnection> connection) {
        std::string data = "echo back info: ";
        char        buffer[100];
        size_t      size = connection->recv(buffer, 100);
        data.append(buffer, size);
        std::weak_ptr<TcpConnection> weakConnection = connection; 
        LOG_INFO() << "echo back: " << data;
        connection->sendAsyn(data.c_str(), data.size());
        cons.push_back(connection); 
    });
    server.listen();
    while (true)
    {
        CurrentThread::sleep(1000);
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        std::string number = argv[1];
        port               = std::stoi(number);
    }
    serviceV2();
    return 0;
} 
```

### TCP通信

TCPServer

```c++
#include "../src/tcp_server.h"
#include "../src/event_loop_thread.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(TcpServer, resovle)
{
    TcpServer server{9000};
    server.setSubReactorThreadNumber(2);
    server.setOnReadyToRead([](std::shared_ptr<TcpConnection> connection) {
        char   buffer[100] = "echo back: ";
        size_t size        = connection->recv(buffer + strlen(buffer), 100);
        std::weak_ptr<TcpConnection> weakConnection = connection;
        connection->setOnSent([weakConnection](size_t) {
            if (auto con = weakConnection.lock())
            {
                con->close();
            }
        });
        connection->sendAsyn(buffer, strlen(buffer) + size);
    });
    server.listen();
    while (true)
    {
        CurrentThread::sleep(1000);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
 
```

TCP client

```c++
#include "../src/tcp_client.h"
#include "../src/event_loop_thread.h"
#include "../src/log.h"
#include "../src/tcp_server.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(TcpClient, resovle)
{
    TcpClient client;
    auto [ok, address] = InetAddress::resolve("127.0.0.1", 2019);
    GTEST_ASSERT_EQ(ok, true);
    LOG_INFO() << address.getIp4String() << " " << address.getPort();
    client.setOnReadyToRead([&]() { LOG_DEBUG() << client.read(); });
    ok = client.connectTo(address);
    GTEST_ASSERT_EQ(ok, true);
    auto begin = rand();
    for (int i = 0; i < 100; i++)
    {
        client.sendAsyn(std::to_string(i) + " " + std::to_string(begin) + "\n");
        begin++;
    }
    CurrentThread::sleeps(3);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


```

## 网络通信

### 反应堆（事件循环）模式

事件处理线程不停地监听事件,当事件发送时,调用对应的处理事件

```c++
void eventloop(){
    while(true){
        auto events=readEventList();
        for(auto& e:events){
            e.handle();
        }
    } 
}
```

此时，每个Socket都属于一个网络IO线程，其产生的所有事件(如read write)都在同一网络IO线程上运行，避免了多线程操作的数据竞争问题

### 事件处理

#### 事件源的抽象--通道

对于每个产生事件的事件源，Linux系统使用fd（文件描述符）表示，如socket fd、文件fd。因此，只需要对fd进行封装,将事件源封装为通道。

```c++
Channel createChannle(int fd){
    Channel c;
    c.setFd(fd);
    return c;
} 

```

### 事件的监听

事件源有可读、可写、错误、关闭事件，使用epoll可以同时监听多个通道以及通道的多个事件。监听前需要向epoll注册事件源以及关心的事件。

epooll监听的事件主要有以下几种:

|宏|信息|
|---|---|
|EPOLLHUP|关闭|
|EPOLLERR|错误|
|EPOLLIN|可读|
|EPOLLRDHUP|可读|
|EPOLLPRI|可读|
|EPOLLOUT|可写|
  
---
向epool注册事件源与事件

```c++
void epollAdd(Channel* ch){
    epoll_event event;
    event.data.ptr = channel;
    event.events   = channel->getListenEvents();//获取需要监听的事件
    epoll_ctl(epollFd, EPOLL_CTL_ADD, channel->getFd(), &event);
}
```

向epool修改事件源与事件  

```c++
void epollMod(Channel* ch){
    epoll_event event;
    event.data.ptr = channel;
    event.events   = channel->getListenEvents();//获取需要监听的事件
    epoll_ctl(epollFd, EPOLL_CTL_MOD, channel->getFd(), &event);
}
```

向epool删除事件源与事件

```c++
void epollDel(Channel* ch){
    epoll_event event;
    event.data.ptr = channel;
    event.events   = channel->getListenEvents();//获取需要监听的事件
    epoll_ctl(epollFd, EPOLL_CTL_DEL, channel->getFd(), &event);
}
```

### 事件回调函数

通过回调函数来处理事件，为每种事件设置一个回调函数

```c++
void Channel::setOnCloseCallBack(OnCloseCallBack callback);//关闭事件的回调
void Channel::setOnReadCallBack(OnReadCallBack callback);//可读事件的回调
void Channel::setOnWriteCallBack(OnWriteCallBack callback);//可写时间的回调
void Channel::setOnErrorCallBack(OnErrorCallBack callback);//错误事件的回调

```

### 事件的处理

注册完毕后即可通过epoll监听事件,把监听的事件更新到channel中

```c++
vector<Channel*> epoll(int msecond){
    struct epoll_event buffer[N];
    int number = epoll_wait(_fd,buffer,N,msecond);
    vector<Channel*> channels;
    for (int i = 0; i < number; i++)
    {
        auto& event = buffer[i];
        auto  ch    = event.data.ptr;
        ch->setEventGenerated(event.events);
        channels.push_back(ch);
    }
    return channels;
}
```

在channel中使用事前设置好的回调函数处理事件

```c++

void handleEvent()
{ 
    if (isClosed()){
        if (onCloseCallBack){
            onCloseCallBack();
        }
    } 
    if (isError()){
        if (onErrorCallBack){
            int       option;
            socklen_t length = sizeof(option);
            bool      error = -1 == getsockopt(_fd, SOL_SOCKET, SO_ERROR, &option, &length);
            onErrorCallBack(error ? errno : option);
        }
    } 
    if (isReadyToRead()){
        if (onReadCallBack){
            onReadCallBack();
        }
    }
    if (isReadyToWrite()){
        if (onWriteCallBack)
        {
            onWriteCallBack();
        }
    }
    resetEvent();
}
```

### 多反应堆模式

使用一个反应堆来接受连接，使用其他的反应堆来处理连接。

接受连接的反应堆

```c++
void listen(){
    channel.setOnRead([]{
        int fd = socket.accept();
        eventLoopPool.insertConnection(fd);
    });
    channel.enableRead();
}
```

## 异步日志

### 日志buffer

日志每一个buffer是一个定长的string,buffer记录了每一条日志的所有信息。每一个buffer都从buffer池中获取，然后归还给buffer池。当新插入的数据长度大于日志的剩余容量时，自动替换为buffer池中的一个更大容量buffer。

### 日志buffer池

事先分配好size为N,2N,4N,8N,16N,32N......的buffer池，每次获取buffer时，按照从size小到大的匹配顺序找到空闲的buffer。如果找不到可用的buffer，则从内存中new一个新的buffer。

### 日志输出

使用RAII来构造日志，通过日志宏来输出日志。

```c++
class Logger{
    Logger(){
        //写入日志头信息，如时间，线程id,线程名，代码文件名，代码行号
    }
    ~Logger(){
        //写入日志
    }
    template <typename T>
    Logger& operator<<(const T& data){
        //写入日志内容
    }
    Buffer buffer;
};

#define LOG_DEBUG()  Logger()
```

### 异步写入日志

线程写入日志时，直接写入到日志队列。当日志线程空闲时，获取日志队列，再将日志后台写入到文件中。

```c++
void append(LogBuffer buffer){  
    std::lock_guard<std::mutex> guard{_mutex};
    _writeVector.push_back(std::move(buffer));
    if (_isWaiting){
        _condition.notify_one();
        _isWaiting = false;
    } 
}

void writing(){
    WriteOnlyFile  file;
    std::vector<LogBuffer> writeVector;  
    while(true){
        std::unique_lock<std::mutex> lock{_mutex};
        if (!_writeVector.empty()){
            writeVector.swap(_writeVector);
        }
        else{
            _isWaiting = true;
            _condition.wait_for(lock,std::chrono::milliseconds{_flushInterval});
        }
    }
    for (auto& buffer : writeVector){
        file.append(buffer.rawData(),buffer.size());
    }
    writeVector.clear(); 
}

```
