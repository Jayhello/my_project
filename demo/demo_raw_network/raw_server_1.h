//
// Created by Administrator on 2022/10/9.
//
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <sys/epoll.h>//epoll
#include <functional>
#include "raw_comm.h"

struct EndPoint{
    int fd;
    std::string sip;
    int port;
    inline std::string toString()const{
        return comm::util::util::format("ip:%s,port:%d,fd:%d", sip.c_str(), port, fd);
    }
};

// 原始的 socket 实现的echo server
namespace v1{

void echoServer();

} // v1

// 使用封装的函数, 示例
namespace v2{

void echoServer();

}

/*
 1. 如果每个链接来了, 都丢到线程去异步处理, 有个问题, fd阻塞的方式read不到数据
 如果client链接之后必须, 立马发送数据, 如果不发那么线程就会一直阻塞在这个fd.

 2. 将socket设置为非阻塞, 一个线程可以遍历所有的socket检查是否有数据读, 有则去处理
    a. 得一直轮询不能sleep, 这样消息处理不及时
    b. 大多fd数链接没有数据轮询就有点浪费了
*/

namespace v3{

void echoServer();

} // v3

// select example
namespace v4{

void selectExample();

void selectServer();

} // v4

// epoll example
namespace v5{

void epollServer();

} // v5

// epoll 简单的封装为比较通用的 server
namespace v6{

void epollWarpServer();

}// v6

namespace day05{

class Channel;

//using ChannelPtr     = std::shared_ptr<Channel>;  // 这里不适合用shared_ptr
using ChannelPtr     = Channel*;
using ChannelPtrList = std::vector<ChannelPtr>;

class Epoll{
public:
    Epoll();

    ~Epoll();

    int init();

    int poll(ChannelPtrList& vList, int timeout = -1);

    const static int MAX_EVENTS = 100;

    void updateEvent(Channel* ptr);
private:
    int    epfd_;
    struct epoll_event* events_;
};

class Channel{
public:
    Channel(Epoll* ep, int fd):ep_(ep), fd_(fd){}

    void enableRead();

    bool hasAdd()const{
        return bAdd_;
    }
    int getFd()const{
        return fd_;
    }
    int getEvent()const{
        return events_;
    }

    void setEpEvent(int events){
        ep_events_ = events;
    }

    int getEpEvents()const{
        return ep_events_;
    }

private:
    Epoll* ep_;
    int fd_;
    int events_ = EPOLLET;
    bool bAdd_ = false;
    int  ep_events_;
};

void day05_example();

void handleRead(int fd);

} // day05

namespace day06{

void day06_example();

// 下面的指针没有释放的问题, 先放下
class Channel;
using ChannelPtr     = Channel*;
using ChannelPtrList = std::vector<ChannelPtr>;

class Epoll{
public:
    Epoll():epfd_(-1), events_(nullptr){}
    ~Epoll();

    int init();

    void updateChannel(ChannelPtr pc);

    int poll(ChannelPtrList& vList, int timeout = -1);

    const static int MAX_EVENTS = 100;
private:
    int  epfd_          = -1;
    struct epoll_event* events_;
};

using EpollPtr = Epoll*;

using ReadCallbackFunc = std::function<void(ChannelPtr)>;

class Channel{
public:
    Channel(EpollPtr pep, int fd):p_ep_(pep), fd_(fd){}   // 原版的代码是和EventLoop绑定的, 这里先还是用eptr

    void setReadCallback(ReadCallbackFunc cb){read_cb_ = cb;}

    void enableRead();

    // 这里也有问题, 不能只有一个read(先简单的用read)
    void handleRead(){
        read_cb_(this);
    }

    int getFd()const{return fd_;}
    int getEvent()const{return event_;}
    int getEpEvent()const{return r_event_;}
    void setEpEvent(int event){r_event_ = event;}

    bool inEpoll()const{return b_in_ep_;}

    void setInEpoll(bool flag){b_in_ep_ = flag;}
private:
    EpollPtr p_ep_;
    int      fd_;
    int      event_ = EPOLLET;
    int      r_event_ = 0;
    bool     b_in_ep_ = false;
    ReadCallbackFunc read_cb_;
};

class EventLoop{
public:
//    EventLoop(EpollPtr p_ep_);   // 这里可以支持传不同的Epoll类型, 例如select实现的poll
    EventLoop():p_ep_(nullptr){}

    ~EventLoop();

    int init();

    void loop();

    EpollPtr getEpollPtr(){
        return p_ep_;
    }
private:
    EpollPtr p_ep_;
};

using EventLoopPtr = EventLoop*;

/*
    1. 创建服务的fd
    2. 设置accept, client fd的处理函数
*/
class Server{
public:
    Server(EventLoopPtr pel):sfd_(-1), p_el_(pel){}

    int init();

    void onNewConnection(ChannelPtr);

    void onRead(ChannelPtr);
private:
    int            sfd_;
    EventLoopPtr   p_el_;
};

} // day06

/*
 !!!!!  能复用的代码就复用吧, 这里就不在重新写了(一大堆代码看着也不好)
 1. 新增Acceptor类
 2. 修改day06的channel -> event loop关系
*/
namespace day07{

void day07_example();

using namespace day06;

using AcceptCallbackFunc = std::function<void(ChannelPtr)>;

class Acceptor{
public:
    Acceptor(EventLoopPtr pEl):p_el_(pEl){}

    int init();

    void setAcceptCallback(AcceptCallbackFunc);

    void defaultAcceptCallback(ChannelPtr);

private:
    EventLoopPtr   p_el_;
    int sfd_ = -1;
    ChannelPtr pac_;
    AcceptCallbackFunc cb_;
};

class Server{
public:
    Server(EventLoopPtr p_el):p_el_(p_el), acceptor_(p_el){}
    int init();

    EventLoopPtr   p_el_;
    Acceptor       acceptor_;
};  // Server

} // day07

namespace day08{

void day08_example();

using namespace day06;

// 接受链接之后的 callback
using AfterAcceptCallbackFunc = std::function<void(EndPoint)>;

class Acceptor{
public:
    Acceptor(EventLoopPtr p_el):p_el_(p_el){}

    int init();

    void onAcceptEvent(ChannelPtr);

    void setAfterAcceptCallback(AfterAcceptCallbackFunc cb);

    EventLoopPtr            p_el_;
    int                     sfd_ = -1;
    ChannelPtr              pac_;
    AfterAcceptCallbackFunc cb_afterAccept_;
};

class Connection{
public:
    Connection(EndPoint ep, EventLoopPtr p_el);

    void handleEvent(ChannelPtr ptr);

    EndPoint        ep_;
    EventLoopPtr    p_el_;
    ChannelPtr      pc_;
};

using ConnectionPtr = Connection*;

class Server{
public:
    Server(EventLoopPtr p_el):p_el_(p_el), acceptor_(p_el){}

    int init();

    void afterAcceptCallback(EndPoint);

    EventLoopPtr     p_el_;
    day08::Acceptor  acceptor_;
    std::map<int, ConnectionPtr> m_fd_con_;
};  // Server

} // day08


/*
    在 day08 的基础上加buffer
    这里的buffer太简单了, 没有parse这些功能(后面在加吧)
*/
namespace day09{

void example_09();

using namespace day06;

class Buffer{
public:
    void append(const string& str){
        buf_.append(str);
    }

    int size()const{return buf_.size();}

    const string& data()const{return buf_;}

    std::string buf_;
};

using CloseConnectionCallback = std::function<void(const EndPoint&)>;

class Connection{
public:
    Connection(EndPoint ep, EventLoopPtr p_el);

    void handleEvent(ChannelPtr ptr);

    void setCloseConnectionCallback(CloseConnectionCallback cb);

    EndPoint        ep_;
    EventLoopPtr    p_el_;
    ChannelPtr      pc_;
    Buffer          r_buf_;
    CloseConnectionCallback cb_;
};

using ConnectionPtr = Connection*;

class Server{
public:
    Server(EventLoopPtr p_el):p_el_(p_el), acceptor_(p_el){}

    int init();

    void afterAcceptCallback(EndPoint);

    void onClose(const EndPoint& ep);

    EventLoopPtr     p_el_;
    day08::Acceptor  acceptor_;
    std::map<int, ConnectionPtr> m_fd_con_;
};  // Server

} // day09

/*
 这里将day10, day12合在一起, 支持线程池, main-reactor, sub-reactor
*/

namespace day10{

void example();


class Channel{

};


} // day10

