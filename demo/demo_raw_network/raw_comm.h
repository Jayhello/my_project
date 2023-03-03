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
#include <map>
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

class EventLoopBase;
using EventLoopPtr = EventLoopBase*;

enum kEvents{
    kReadEvent  = POLLIN,        // 0x001
    kWriteEvent = POLLOUT,       // 0x004
};

class ChannelBase{
public:
    ChannelBase(const EndPoint& ePoint, EventLoopPtr ptrEl);

    const EndPoint& getEndPoint()const{
        return ePoint_;
    }

    int getFd()const{return ePoint_.fd;}

    int getEvent()const{
        return events_;
    }

    void enableRead(bool enable);
    void enableWrite(bool enable);

    bool readEnabled()const;
    bool writEnabled()const;

    void setActiveEvent(int events){activeEvents_ = events;}
    bool canRead()const{return activeEvents_ & kReadEvent;}
    bool canWrite()const{return activeEvents_ & kWriteEvent;}

    using ReadHandle  = std::function<void(void)>;
    using WriteHandle = std::function<void(void)>;

    void setReadHandle(const ReadHandle& rh){readHandle_ = rh;}
    void setWriteHandle(const WriteHandle& wh){writeHandle_ = wh;}

    void handleRead(){readHandle_();}
    void handleWrite(){writeHandle_();}
private:
    EndPoint      ePoint_;
    EventLoopPtr  ptrEl_;

    int           events_ = kReadEvent;
    int           activeEvents_ = 0;
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

    EpollBasePtr getPollBase(){return ptrPoll_;}

private:
    EpollBasePtr ptrPoll_ = nullptr;
    bool         init_    = false;
    bool         stop_ = false;
};

class Msg{
public:
    Msg() = default;

    Msg(const string& str):data_(str){}

    const string& data()const{return data_;}

    string& data(){return data_;}

    void append(const string& str){data_.append(str);}

    const char* c_str()const{return data.c_str();}

    string data_;
};

class Buffer{
    Buffer() = default;
    Buffer(const string& str):data_(str){}

    const string& data()const{return data_;}

    string& data(){return data_;}

    void append(const string& str){data_.append(str);}

    const char* c_str()const{return data_.c_str();}

    void remove(int pos, int len){data_.erase(pos, len);}

    string data_;
};

class CodecBase{
public:
    virtual  ~CodecBase() = 0;

    virtual void encode(const Msg& msg, Buffer& buf) = 0;

    // < 0 buf数据异常, = 0 数据不完整, > 0 解析出了一个多大的msg包
    virtual int tryDecode(const Buffer& buf, Msg& msg) = 0;
};

class ConnectionBase{
public:
    ConnectionBase(EventLoopPtr ptrEl, const EndPoint& ePoint);

    void onRead();

    void onWrite();

    void handleHandshake();

    enum eConnectState{
        eInvalid        = 0,
        eHandshaking    = 1,
        eConnected      = 2,
        eClosed         = 3,
        eFailed         = 4,
    };

    // 支持下创建链接
    static ConnectionBase* createConnection(EventLoopPtr ptrEl, string ip, int port);

    void setState(int iState){iState_ = iState;}

    void onState();

private:
    EventLoopPtr   ptrEl_;
    EndPoint       ePoint_;
    ChannelPtr     ptrChannel_;
    int            iState_;
};

using ConnectionPtr  = ConnectionBase*;

using AcceptCallback = std::function<void(const EndPoint&)>;

class AcceptorBase{
public:
    AcceptorBase(EventLoopPtr ptrEl);

    int init();

    void onAccept();

    void setAcceptCallback(AcceptCallback cb);

private:
    EventLoopPtr   ptrEl_;
    EndPoint       ePoint_;
    ChannelPtr     ptrChannel_;
    AcceptCallback acceptCallback_;
};

using AcceptorPtr = AcceptorBase*;

class Server{
public:
    Server(EventLoopPtr ptrEl):ptrEl_(ptrEl){}

    int init();

//    void waitForShutDown();

    void acceptCallback(const EndPoint& ep);

private:
    EventLoopPtr  ptrEl_;
    AcceptorPtr   ptrAcceptor_;
    std::map<int, ConnectionPtr> mFdConnection_;
};


} // raw_comm
