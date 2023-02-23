//
// Created by Administrator on 2022/10/9.
//
#pragma once

#include <fcntl.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>//epoll
#include "common/logging.h"
#include "common/thread_pool.h"
#include <string>
#include <iostream>
#include <signal.h>
#include <sys/poll.h>
#include "coroutine.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;
using namespace comm;
using namespace comm::log;
using log_v1::ScopeLog;

#define LOCAL_IP    "127.0.0.1"
#define PORT        8880

struct EndPoint{
    int fd;
    std::string sip;
    int port;
    inline std::string toString()const{
        return comm::util::util::format("ip:%s,port:%d,fd:%d", sip.c_str(), port, fd);
    }
};

// 用原始的socket编程不少参数不好理解, 而且繁琐, 因此这里封装更好懂的c++接口, 简化编程
namespace raw_v1{

// < 0 错误
int getTcpSocket();

// < 0 错误
int doConnect(int fd, const string& sIp, int iPort);

int doBind(int fd, const string& sIp, int iPort);

int doListen(int fd, int n = SOMAXCONN);

int doAccept(int fd, string& sIp, int& iPort);

// create, bind, listen, ret > 0 succ
int createTcpServerSocket(const string& sIp, int iPort);

// < 0 错误 , > 0 写的数据size  (这个write没有返回0的)
// todo read 这里的参数 sData 是否可以优化下, const char* str, len
int doWrite(int fd, const string& sData);

// < 0 错误,  = 0 对方关闭了链接, > 0 写的数据size
int doRead(int fd, string& sData, size_t iReadSize);

// < 0 错误 , > 0 写的数据size  (这个write没有返回0的)
int doSend(int fd, const string& sData);

// < 0 错误,  = 0 对方关闭了链接, > 0 写的数据size
int doRecv(int fd, string& sData, size_t iReadSize);

int doClose(int fd);

int setNonBlock(int fd, bool value = true);

int setReuseAddr(int fd);

int setReusePort(int fd);

int setSocketOpt(int fd, int opt, const void* val, socklen_t opt_len, int level);

} // raw_v1

/*
    将channel, poll, event_loop, connection写个个基类, 后面扩展代码的时候简洁点
    函数构造传指针即可
*/
namespace raw_comm{

class EventLoop;
using EventLoopPtr = EventLoop*;

enum kEvents{
    kReadEvent  = POLLIN,        // 0x001
    kWriteEvent = POLLOUT,       // 0x004
};

class ChannelBase{
public:
    ChannelBase(const EndPoint& ePoint, EventLoopPtr ptrEl): ePoint_(ePoint), ptrEl_(ptrEl){}

    const EndPoint& getEndPoint()const{
        return ePoint_;
    }

    int getEvent()const{
        return events_;
    }

    using ReadHandle  = std::function<void(void)>;
    using WriteHandle = std::function<void(void)>;

    void setReadHandle(const ReadHandle& rh){readHandle_ = rh;}
    void setWriteHandle(const WriteHandle& wh){writeHandle_ = wh;}

    void handleRead(){readHandle_();}
    void handleWrite(){writeHandle_();}

    void enableRead(bool flag);
    void enableWrite(bool flag);

    bool readEnabled()const;
    bool writEnabled()const;
private:
    EndPoint      ePoint_;
    EventLoopPtr  ptrEl_;

    int           events_ = 0;
    ReadHandle    readHandle_;
    WriteHandle   writeHandle_;
};

using ChannelPtr = ChannelBase*;

using ChannelPtrList = std::vector<ChannelPtr>;

class EpollBase{
public:
    int init();

    void addChannel(ChannelPtr);

    void updateChannel(ChannelPtr);

    ChannelPtrList pollOnce(int timeoutMs);

    const static int MAX_EVENTS = 100;
private:
    int epfd_ = -1;
    struct epoll_event* events_;
};

using EpollBasePtr = EpollBase*;

class EventLoopBase{
public:
    int init();

    ~EventLoopBase();

//    void waitForShutDown();
    void loop();

private:
    EpollBasePtr ptrPoll_ = nullptr;
    bool         init_    = false;
};

class ConnectionBase{
public:
    ConnectionBase(ChannelPtr pc);

    void onRead();

    void onWrite();

private:
    ChannelPtr     ptrChannel_;
};

using AcceptCallback = std::function<void(const EndPoint&)>;

class AcceptorBase{
public:
    int init();

    void onAccept();

private:
    ChannelPtr     ptrChannel_;
    AcceptCallback acceptCallback_;
};

class Server{
public:

private:

};


}
