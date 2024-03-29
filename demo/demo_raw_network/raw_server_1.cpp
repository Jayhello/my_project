//
// Created by Administrator on 2022/10/9.
//

#include "raw_server_1.h"
#include <set>
#include <unordered_map>

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start server1 demo");

//    v1::echoServer();
//    v2::echoServer();
//    v3::echoServer();
//    v4::selectExample();
//    v4::selectServer();
//    v5::epollServer();
//    v6::epollWarpServer();

//    day05::day05_example();
//    day06::day06_example();

//    day07::day07_example();
//    day08::day08_example();

//    day10::example();
//    day11::example_1();
//    day11::example_codec_1();
    day11::example_string_slice();

    info("exit server1 demo");
    return 0;
}

namespace v1{

void echoServer(){
    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return_if(fd < 0, "fd: %d err", fd);

    info("server fd: %d", fd);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(LOCAL_IP);
    serv_addr.sin_port = htons(PORT);

    int ret = bind(fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    return_if(ret < 0, "bind err: %d", ret);

    ret = listen(fd, SOMAXCONN);
    return_if(ret < 0, "listen err: %d", ret);

    info("listen succ");
    while(1){
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_len = sizeof(clnt_addr);
        bzero(&clnt_addr, sizeof(clnt_addr));

        int cfd = accept(fd, (sockaddr*)&clnt_addr, &clnt_addr_len);
        info("accept fd %d! IP: %s Port: %d", cfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

        char buf[1024];
        bzero(&buf, sizeof(buf));

        // 如下write之后又while去accept了, 因此下面的fd不会在read了
        ssize_t read_bytes = read(cfd, buf, sizeof(buf));
        if(read_bytes > 0){
            info("message from client fd %d: %s", cfd, buf);
            write(cfd, buf, sizeof(buf));
        } else if(read_bytes == 0){
            info("client fd %d disconnected\n", cfd);
            close(cfd);
            break;
        } else if(read_bytes == -1){
            close(cfd);
            return_if(true, "socket read error");
        }
    }
}

} // v1

// 使用封装的函数, 示例
namespace v2{

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    info("catch ctrl c, exit");
    keepRunning = 0;
}

void echoServer(){
    signal(SIGINT, intHandler);

    int fd = raw_v1::getTcpSocket();
    return_if(fd <= 0, "get_socket_fd_fail");
    info("fd: %d", fd);

    int ret = raw_v1::doBind(fd, LOCAL_IP, PORT);
    return_if(ret < 0, "bind fail: %d", ret);

    ret = raw_v1::doListen(fd);
    return_if(ret < 0, "listen fail: %d", ret);

    while(keepRunning){   // 这里不见得可以捕获的到, 因为进程阻塞在了accept
        string cIp;
        int cPort = 0;
        int cfd = raw_v1::doAccept(fd, cIp, cPort);
        if(cfd <= 0){
            error("accept error: %d", cfd);
            break;
        }

        info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);

        while(1){
            string sData;
            int iReadSize = raw_v1::doRead(cfd, sData, 1024);
            if(iReadSize > 0) {
                info("get msg from fd: %d, size: %d, %s", cfd, iReadSize, sData.c_str());
                int iWriteSize = raw_v1::doWrite(cfd, sData);
                if (iWriteSize < 0) {
                    error("fd: %d write fail close it", fd);
                    raw_v1::doClose(cfd);
                    break;
                }
                info("echo back size: %d, %s", iWriteSize, sData.c_str());
            }else if(0 == iReadSize){
                info("fd: %d has close", fd);
                raw_v1::doClose(cfd);
                break;
            }else{
                error("fd: %d read fail close it", cfd);
                raw_v1::doClose(cfd);
                break;
            }
        }

        info("now wait to accept new client");
    }

    info("exit");
    raw_v1::doClose(fd);
}

} // v2

namespace v3{

class ProcessTask : public comm::thread_pool::Thread{
public:
    void addSocket(int cfd){
        raw_v1::setNonBlock(cfd);
        setFd_.insert(cfd);
    }

    virtual ~ProcessTask(){
        for(auto fd : setFd_)raw_v1::doClose(fd);
    }

    void stop(){stop_ = true;}
protected:
    virtual void run() override{
        while(not stop_){
            handlerAll();
        }
    }

    void handlerAll(){

        for(auto cfd : setFd_){
            string sData;
            int iReadSize = raw_v1::doRead(cfd, sData, 1024);
            if(iReadSize > 0) {
                info("get msg from fd: %d, size: %d, %s", cfd, iReadSize, sData.c_str());
                int iWriteSize = raw_v1::doWrite(cfd, sData);
                if (iWriteSize < 0) {
                    error("fd: %d write fail close it", cfd);
                    raw_v1::doClose(cfd);
                    break;
                }
                info("echo back size: %d, %s", iWriteSize, sData.c_str());
            }else if(0 == iReadSize){
                info("fd: %d has close", cfd);
                raw_v1::doClose(cfd);
                break;
            }else{
                int error = errno;
                if(EAGAIN == error or EWOULDBLOCK == error or EINTR == error){
                    info("fd: %d no data...., ret: %d error: %d, %s", cfd, iReadSize, error, strerror(error));
                }else{
                    error("fd: %d read fail close it, ret: %d error: %d, %s", cfd, iReadSize, error, strerror(error));
                    raw_v1::doClose(cfd);
                }
                break;
            }
        }

        info("now sleep wait...");
        sleep(1);         // sleep这么久, client发包快的话, 会导致粘包(两次请求的包, 这里一次收到)
    }

    std::set<int>  setFd_;
    volatile bool stop_ = false;
};

void echoServer(){
    int fd = raw_v1::getTcpSocket();
    return_if(fd <= 0, "get_socket_fd_fail");
    info("fd: %d", fd);

    int ret = raw_v1::doBind(fd, LOCAL_IP, PORT);
    return_if(ret < 0, "bind fail: %d", ret);

    ret = raw_v1::doListen(fd);
    return_if(ret < 0, "listen fail: %d", ret);

    ProcessTask task;
    task.start();

    while(1){
        string cIp;
        int cPort = 0;
        int cfd = raw_v1::doAccept(fd, cIp, cPort);
        if(cfd <= 0){
            error("accept error: %d", cfd);
            break;
        }

        info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);

        task.addSocket(cfd);
    }

    task.stop();
}

} // v3

namespace v4{

void selectExample(){
    fd_set fds;
    FD_ZERO(&fds);
    // 把标准输入的文件描述符 0 加入到集合中 (cin 0, cout 1, cerr 2)
    FD_SET(0, &fds);

    timeval tv;   // timeout 5.5 second
    tv.tv_sec = 5;
    tv.tv_usec = 500;

    int ret = select(1, &fds, nullptr, nullptr, &tv); // block and wait 5.5s
    if(ret < 0)
        error("select error ret: %d", ret);
    else if(ret == 0)
        info("select timeout");
    else{
        string buf;
        std::cin >> buf;
        info("input bus: %s", buf.c_str());
    }

    info("ending.....");
}

void selectServer(){
    int fd = raw_v1::getTcpSocket();
    return_if(fd <= 0, "get_socket_fd_fail");
    info("fd: %d", fd);

    int ret = raw_v1::doBind(fd, LOCAL_IP, PORT);
    return_if(ret < 0, "bind fail: %d", ret);

    ret = raw_v1::doListen(fd);
    return_if(ret < 0, "listen fail: %d", ret);

    fd_set   fds;
    std::set<int> setClientFd;
    int max_fd = fd;
    while(1){
        FD_ZERO(&fds);

        FD_SET(fd, &fds);    // 把当前连接的文件描述符加入到集合中
        for(auto cfd : setClientFd){
            FD_SET(cfd, &fds);
            max_fd = std::max(max_fd, cfd);
        }

        info("select fds size: %d", int(setClientFd.size()));
        timeval  tv {5, 100};
        ret = select(max_fd + 1, &fds, NULL, NULL, &tv);
        if(ret < 0){
            int ge = errno;
            error("select fail ret: %d, errno: %d, %s", ret, ge, strerror(ge));
            break;
        }else if(0 == ret){
            info("select timeout, wait next");
        }else{
            if(FD_ISSET(fd, &fds)){
                string cIp;
                int cPort = 0;
                int cfd = raw_v1::doAccept(fd, cIp, cPort);
                if(cfd <= 0){
                    error("accept error: %d", cfd);
                    break;
                }

                info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);
                setClientFd.insert(cfd);
            }

            for(auto cfd : setClientFd){
                if(FD_ISSET(cfd, &fds)){
                    string sData;
                    int iReadSize = raw_v1::doRead(cfd, sData, 1024);
                    if(iReadSize > 0) {
                        info("get msg from fd: %d, size: %d, %s", cfd, iReadSize, sData.c_str());
                        int iWriteSize = raw_v1::doWrite(cfd, sData);
                        if (iWriteSize < 0) {
                            error("fd: %d write fail close it", cfd);
                            raw_v1::doClose(cfd);
                            setClientFd.erase(cfd);
                            break;
                        }
                        info("echo back size: %d, %s", iWriteSize, sData.c_str());
                    }else{
                        info("fd: %d has close", cfd);
                        raw_v1::doClose(cfd);
                        setClientFd.erase(cfd);
                        break;
                    }
                }
            }
        }
    }

    info("exit");
    raw_v1::doClose(fd);
}


} // v4

// epoll example
namespace v5{

// https://juejin.cn/post/6936836371352911902

void epollServer(){
    const int EVENTS_SIZE = 20;

    int socketFd = raw_v1::getTcpSocket();
    return_if(socketFd <= 0, "get_socket_fd_fail");
    info("fd: %d", socketFd);

    int ret = raw_v1::doBind(socketFd, LOCAL_IP, PORT);
    return_if(ret < 0, "bind fail: %d", ret);

    ret = raw_v1::doListen(socketFd);
    return_if(ret < 0, "listen fail: %d", ret);

    //创建一个epoll,size已经不起作用了,一般填1就好了
    int eFd = epoll_create(1);

    //把socket包装成一个epoll_event对象
    //并添加到epoll中
    epoll_event epev{};
    epev.events = EPOLLIN;//可以响应的事件,这里只响应可读就可以了
    epev.data.fd = socketFd;//socket的文件描述符
    epoll_ctl(eFd, EPOLL_CTL_ADD, socketFd, &epev);//添加到epoll中

    //回调事件的数组,当epoll中有响应事件时,通过这个数组返回
    epoll_event events[EVENTS_SIZE];

    //整个epoll_wait 处理都要在一个死循环中处理
    while (true) {
        //这个函数会阻塞,直到超时或者有响应事件发生
        int eNum = epoll_wait(eFd, events, EVENTS_SIZE, 5);
        return_if(ret < 0, "epoll_wait fail: %d", ret);

        //遍历所有的事件
        for (int i = 0; i < eNum; i++) {
            //判断这次是不是socket可读(是不是有新的连接)
            if (events[i].data.fd == socketFd) {
                if (events[i].events & EPOLLIN) {
                    string cIp;
                    int cPort = 0;
                    int cfd = raw_v1::doAccept(socketFd, cIp, cPort);

                    if (cfd > 0) {
                        //设置响应事件,设置可读和边缘(ET)模式
                        //很多人会把可写事件(EPOLLOUT)也注册了,后面会解释
                        epev.events = EPOLLIN | EPOLLET;
                        epev.data.fd = cfd;
                        //设置连接为非阻塞模式
                        raw_v1::setNonBlock(cfd);  // 依然要设置为非阻塞(不然后面的while read 循环那里会被阻塞)
                        //将新的连接添加到epoll中
                        epoll_ctl(eFd, EPOLL_CTL_ADD, cfd, &epev);
                        info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);
                    }
                }
            } else {//不是socket的响应事件
                //判断是不是断开和连接出错
                //因为连接断开和出错时,也会响应`EPOLLIN`事件
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
                    //出错时,从epoll中删除对应的连接
                    epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                    info("fd: %d has close", events[i].data.fd);
                    raw_v1::doClose(events[i].data.fd);
                } else if (events[i].events & EPOLLIN) {//如果是可读事件
                    while(true){  //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
                        string sData;
                        int iReadSize = raw_v1::doRead(events[i].data.fd, sData, 1024);
                        if(iReadSize > 0) {
                            info("get msg from fd: %d, size: %d, %s", events[i].data.fd, iReadSize, sData.c_str());
                            int iWriteSize = raw_v1::doWrite(events[i].data.fd, sData);
                            if (iWriteSize < 0) {
                                error("fd: %d write fail close it", events[i].data.fd);
                                raw_v1::doClose(events[i].data.fd);
                                break;
                            }
                            info("echo back size: %d, %s", iWriteSize, sData.c_str());
                        }else if(0 == iReadSize){  //EOF，客户端断开连接
                            info("fd: %d has close", events[i].data.fd);
                            raw_v1::doClose(events[i].data.fd);
                            epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                            break;
                        }else{  // -1
                            int error = errno;
                            if(EAGAIN == error or EWOULDBLOCK == error){  //非阻塞IO，这个条件表示数据全部读取完毕
                                info("fd: %d no data...., ret: %d error: %d, %s", events[i].data.fd, iReadSize, error, strerror(error));
                                break;
                            }else if(EINTR == error){  //客户端正常中断、继续读取
                                info("fd: %d continue reading...., ret: %d error: %d, %s", events[i].data.fd, iReadSize, error, strerror(error));
                                continue;
                            }else{
                                error("fd: %d read fail close it, ret: %d error: %d, %s", events[i].data.fd, iReadSize, error, strerror(error));
                                raw_v1::doClose(events[i].data.fd);
                                epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    raw_v1::doClose(socketFd);
    info("exit");
}

} // v5


// epoll 简单的封装为比较通用的 server
namespace v6{

class EventLoopServer : noncopyable{
public:
    explicit EventLoopServer(const string& ip, int port):sIp_(ip), port_(port){}

    ~EventLoopServer();

    int init();

    int loop();

protected:
    virtual int onClose(const EndPoint& ep);

    virtual int onRead(const EndPoint& ep, const string& sData);

    virtual int onConnect(int cfd, const string& ip, int port);
protected:
    string sIp_;
    int port_;
    int sfd_ = -1;
    int efd_ = -1;
    std::unordered_map<int, EndPoint> mFdEndPoint_;  // 先简单点不加锁
};

void epollWarpServer(){
    EventLoopServer server(LOCAL_IP, PORT);
    int ret = server.init();
    return_if(ret < 0, "init_fail_ret: %d", ret);

    info("start loop");
    server.loop();
    info("exit");
}

EventLoopServer::~EventLoopServer(){
    if(sfd_ > 0){
        int ret = raw_v1::doClose(sfd_);
        info("server exit ip: %s, port: %d, fd: %d, close_ret: %d", sIp_.c_str(), port_, sfd_, ret);
    }
}

int EventLoopServer::init(){
    ScopeLog Log;
    Log << "init ip:" << sIp_ << "port:" << port_;

    sfd_ = raw_v1::getTcpSocket();
    RETURN_IF(sfd_ <= 0, "get_socket_fd_fail", -1);
    Log << "sfd:" << sfd_;

    int ret = raw_v1::doBind(sfd_, sIp_, port_);
    RETURN_IF(ret < 0, "bind_fail", ret);

    ret = raw_v1::setNonBlock(sfd_);
    Log << "set_no_block_ret" << ret;

    ret = raw_v1::setReuseAddr(sfd_);
    Log << "set_readdr_ret" << ret;

    ret = raw_v1::setReusePort(sfd_);
    Log << "set_report_ret" << ret;

    ret = raw_v1::doListen(sfd_);
    RETURN_IF(ret < 0, "listen fail", ret);

    Log << "init_succ";

    return 0;
}

int EventLoopServer::loop(){
    efd_ = epoll_create(1);
    info("epoll_fd: %d", efd_);

    epoll_event epev{};
    epev.events = EPOLLIN;//可以响应的事件,这里只响应可读就可以了
    epev.data.fd = sfd_;//socket的文件描述符
    epoll_ctl(efd_, EPOLL_CTL_ADD, sfd_, &epev);//添加到epoll中

    //回调事件的数组,当epoll中有响应事件时,通过这个数组返回
    const static int SIZE = 100;
    epoll_event events[SIZE];
    while(1){
        int num = epoll_wait(efd_, events, SIZE, 5000);
        return_ret_if(num < 0, num, "epoll_wait_fail_ret: %d", num);

        if(0 == num){
            info("epoll_wait_next_loop");
            continue;
        }else{
            for(int i = 0; i < num; ++i){
                if(events[i].data.fd == sfd_){
                    string ip;
                    int port = 0;
                    int cfd = raw_v1::doAccept(sfd_, ip, port);
                     if(cfd > 0){
                         onConnect(cfd, ip, port);
                     }
                }else{
                    if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP){
                        onClose(mFdEndPoint_[events[i].data.fd]);
                    }else if (events[i].events & EPOLLIN) {//如果是可读事件
                        string sData;
                        int iReadSize = raw_v1::doRead(events[i].data.fd, sData, 1024);
                        if(iReadSize > 0) {
                            onRead(mFdEndPoint_[events[i].data.fd], sData);
                        } else if(iReadSize == 0){
                            onClose(mFdEndPoint_[events[i].data.fd]);
                        }else{
                            int error = errno;
                            if(EAGAIN == error or EWOULDBLOCK == error or EINTR == error){
                                info("fd: %d no data...., ret: %d error: %d, %s", events[i].data.fd, iReadSize, error, strerror(error));
                            }else{
                                error("fd: %d read fail close it, ret: %d error: %d, %s", events[i].data.fd, iReadSize, error, strerror(error));
                                raw_v1::doClose(events[i].data.fd);
                                onClose(mFdEndPoint_[events[i].data.fd]);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int EventLoopServer::onConnect(int cfd, const string& ip, int port){
    raw_v1::setNonBlock(cfd);  // 依然要设置为非阻塞(不然后面的while read 循环那里会被阻塞)

    epoll_event epev{};
    epev.events = EPOLLIN;//可以响应的事件,这里只响应可读就可以了
    epev.data.fd = cfd;//socket的文件描述符
    epoll_ctl(efd_, EPOLL_CTL_ADD, cfd, &epev);//添加到epoll中
    info("accept new client fd: %d, ip: %s, port: %d", cfd, ip.c_str(), port);

    EndPoint ep{cfd, ip, port};
    mFdEndPoint_[cfd] = ep;

    return 0;
}

int EventLoopServer::onClose(const EndPoint& ep){
    ScopeLog Log;
    Log << "close" << ep.toString();

    int ret = epoll_ctl(efd_, EPOLL_CTL_DEL, ep.fd, nullptr);
    Log << "epoll_ctl_del_ret" << ret;

    ret = raw_v1::doClose(ep.fd);
    Log << "close_ret" << ret;

    mFdEndPoint_.erase(ep.fd);

    return 0;
}

int EventLoopServer::onRead(const EndPoint& ep, const string& sData){
    ScopeLog Log;
    Log << "on_read_ep" << ep.toString() << "msg" << sData;

    int iWriteSize = raw_v1::doWrite(ep.fd, sData);
    if (iWriteSize < 0) {
        Log << "write_fail_ret" << iWriteSize;
        onClose(ep);
        return -1;
    }

    Log << "write_size" << iWriteSize;
    return 0;
}

}// v6

namespace day05{

Epoll::Epoll():epfd_(-1), events_(nullptr){
}

Epoll::~Epoll(){
    if(epfd_ > 0){
        close(epfd_);
        epfd_ = -1;
    }

    delete[] events_;
}

int Epoll::init(){
    epfd_ = epoll_create(1);
    if(epfd_ < 0) return -1;

    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);
}

int Epoll::poll(ChannelPtrList& vList, int timeout){
    int num = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    return_ret_if(num < 0, num, "epoll_wait_fail");

    for(int i = 0; i < num; ++i){
        auto ptr = ChannelPtr(events_[i].data.ptr);
        ptr->setEpEvent(events_[i].events);
        vList.push_back(ptr);
    }

    return 0;
}

void Epoll::updateEvent(Channel* ptr){
    epoll_event epev{};
    epev.events = ptr->getEvent();//可以响应的事件,这里只响应可读就可以了
    epev.data.fd = ptr->getFd();//socket的文件描述符
    epev.data.ptr = ptr;

    if(ptr->hasAdd()){
        epoll_ctl(epfd_, EPOLL_CTL_MOD, ptr->getFd(), &epev);
    }else{
        epoll_ctl(epfd_, EPOLL_CTL_ADD, ptr->getFd(), &epev);
    }
}

void Channel::enableRead(){
    events_ |= EPOLLIN;
    ep_->updateEvent(this);  // 这里不好传shared_ptr
    bAdd_ = true;
}

void day05_example(){
    int sfd = raw_v1::createTcpServerSocket(LOCAL_IP, PORT);
    return_if(sfd <= 0, "get_server_fd_fail");
    info("fd: %d", sfd);

    Epoll tEp;
    int ret = tEp.init();
    return_if(ret < 0, "epoll_init_fail");

//    ChannelPtr sSc = std::make_shared<Channel>(&tEp, sfd);
    ChannelPtr pSc = new Channel(&tEp, sfd);
    pSc->enableRead();

    while(true){
        ChannelPtrList vList;
        tEp.poll(vList, 1);
        for(auto ptr : vList){
            if(ptr->getFd() == sfd){
                if(ptr->getEpEvents() & EPOLLIN){
                    string cIp;
                    int cPort = 0;
                    int cfd = raw_v1::doAccept(sfd, cIp, cPort);
                    if (cfd < 0)continue;
                    raw_v1::setNonBlock(cfd);
                    ChannelPtr pc = new Channel(&tEp, cfd);
                    pc->enableRead();
                    info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);
                }
            }else{
                handleRead(ptr->getFd());
            }
        }
    }
}

void handleRead(int fd){
    while(true) {  //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        string sData;
        int iReadSize = raw_v1::doRead(fd, sData, 1024);
        if (iReadSize > 0) {
            info("get msg from fd: %d, size: %d, %s", fd, iReadSize, sData.c_str());
            int iWriteSize = raw_v1::doWrite(fd, sData);  // < 0 简洁点去掉
            info("echo back size: %d, %s", iWriteSize, sData.c_str());
        } else if (0 == iReadSize) {  //EOF，客户端断开连接
            info("fd: %d has close", fd);
            raw_v1::doClose(fd);
//            epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
            break;
        } else {  // -1
            int error = errno;
            if (EAGAIN == error or EWOULDBLOCK == error) {  //非阻塞IO，这个条件表示数据全部读取完毕
                info("fd: %d no data...., ret: %d error: %d, %s", fd, iReadSize, error, strerror(error));
                break;
            } else if (EINTR == error) {  //客户端正常中断、继续读取
                info("fd: %d continue reading...., ret: %d error: %d, %s", fd, iReadSize, error,
                     strerror(error));
                continue;
            } else {
                error("fd: %d read fail close it, ret: %d error: %d, %s", fd, iReadSize, error,
                      strerror(error));
                raw_v1::doClose(fd);
//                epoll_ctl(eFd, EPOLL_CTL_DEL, fd, nullptr);
                break;
            }
        }
    }
}

} // day05

namespace day06{

void day06_example(){
    EventLoop ep;
    int ret = ep.init();
    return_if(ret < 0, "event_loop_init_fail");

    Server svr(&ep);
    ret = svr.init();
    return_if(ret < 0, "svr_init_fail");

    ep.loop();
}

Epoll::~Epoll(){
    if(epfd_ > 0){
        close(epfd_);
        epfd_ = -1;
    }

    if(events_){
        delete[] events_;
        events_ = nullptr;
    }
}

int Epoll::init(){
    epfd_ = epoll_create(1);
    if(epfd_ < 0) return -1;

    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);
}

void Epoll::updateChannel(ChannelPtr pc){
    struct epoll_event epev;
    epev.events = pc->getEvent();
    epev.data.fd = pc->getFd();
    epev.data.ptr = pc;
    if(pc->inEpoll()){
        epoll_ctl(epfd_, EPOLL_CTL_MOD, pc->getFd(), &epev);
    }else{
        epoll_ctl(epfd_, EPOLL_CTL_ADD, pc->getFd(), &epev);
    }
}

int Epoll::poll(ChannelPtrList& vList, int timeout){
    int num = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    return_ret_if(num < 0, num, "epoll_wait_fail");

    for(int i = 0; i < num; ++i){
        auto ptr = ChannelPtr(events_[i].data.ptr);
        ptr->setEpEvent(events_[i].events);
        vList.push_back(ptr);
    }
    return 0;
}

void Channel::enableRead(){
    event_ |= EPOLLIN;
    p_ep_->updateChannel(this);
    b_in_ep_ = true;
}

EventLoop::~EventLoop(){
    if(nullptr != p_ep_){
        delete p_ep_;
        p_ep_ = nullptr;
    }
}

int EventLoop::init(){
    p_ep_ = new Epoll();
    int ret = p_ep_->init();
    return_ret_if(ret < 0, ret, "epoll_init_fail");
    return ret;
}

void EventLoop::loop(){
    while(true){
        ChannelPtrList vList;
        p_ep_->poll(vList, 2);
        for(auto ptr : vList){
            ptr->handleRead();
        }
    }
}

int Server::init(){
    sfd_ = raw_v1::createTcpServerSocket(LOCAL_IP, PORT);
    return_ret_if(sfd_ <= 0, -1, "get_server_fd_fail");
    info("server fd: %d", sfd_);

    ChannelPtr pSc = new Channel(p_el_->getEpollPtr(), sfd_);
    ReadCallbackFunc cb = std::bind(&Server::onNewConnection, this, pSc);

    pSc->setReadCallback(cb);
    pSc->enableRead();
}

void Server::onNewConnection(ChannelPtr ptr){
    if(ptr->getEpEvent() & EPOLLIN){
        string cIp;
        int cPort = 0;
        int cfd = raw_v1::doAccept(sfd_, cIp, cPort);

        raw_v1::setNonBlock(cfd);

        ChannelPtr pc = new Channel(p_el_->getEpollPtr(), cfd);
        ReadCallbackFunc cb = std::bind(&Server::onRead, this, pc);

        pc->setReadCallback(cb);
        pc->enableRead();
        info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);
    }
}

void Server::onRead(ChannelPtr ptr){
    int fd = ptr->getFd();
    day05::handleRead(fd);
}

} // day06

namespace day07{

int Acceptor::init(){
    sfd_ = raw_v1::createTcpServerSocket(LOCAL_IP, PORT);
    return_ret_if(sfd_ <= 0, -1, "get_server_fd_fail");
    info("acceptor fd: %d", sfd_);

    ChannelPtr pSc = new Channel(p_el_->getEpollPtr(), sfd_);
    ReadCallbackFunc cb = std::bind(&Acceptor::defaultAcceptCallback, this, pSc);

    pSc->setReadCallback(cb);
    pSc->enableRead();

    {
        auto default_cb = std::bind(&Acceptor::defaultAcceptCallback, this, pSc);
        setAcceptCallback(default_cb);
    }
}

void Acceptor::setAcceptCallback(AcceptCallbackFunc cb){
    cb_ = cb;
}

void onRead(ChannelPtr ptr){
    int fd = ptr->getFd();
    day05::handleRead(fd);
}

void Acceptor::defaultAcceptCallback(ChannelPtr ptr){
    if(not (ptr->getEpEvent() & EPOLLIN)){
        return ;
    }

    string cIp;
    int cPort = 0;
    int cfd = raw_v1::doAccept(sfd_, cIp, cPort);

    raw_v1::setNonBlock(cfd);

    ChannelPtr pc = new Channel(p_el_->getEpollPtr(), cfd);
    ReadCallbackFunc cb = std::bind(onRead, pc);

    pc->setReadCallback(cb);
    pc->enableRead();
    info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);
}

int Server::init(){
    return acceptor_.init();
}

void day07_example(){
    EventLoop ep;
    int ret = ep.init();
    return_if(ret < 0, "event_loop_init_fail");

    day07::Server svr(&ep);
    ret = svr.init();
    return_if(ret < 0, "svr_init_fail");

    info("server init succ, now loop...");
    ep.loop();
}

} // day07

namespace day08{

void day08_example(){
    EventLoop ep;
    int ret = ep.init();
    return_if(ret < 0, "event_loop_init_fail");

    day08::Server svr(&ep);
    ret = svr.init();
    return_if(ret < 0, "svr_init_fail");

    info("%s, server init succ, now loop...", __FUNCTION__);
    ep.loop();
}

int Acceptor::init(){
    sfd_ = raw_v1::createTcpServerSocket(LOCAL_IP, PORT);
    return_ret_if(sfd_ <= 0, -1, "get_server_fd_fail");
    info("acceptor fd: %d", sfd_);

    ChannelPtr pSc = new Channel(p_el_->getEpollPtr(), sfd_);
    ReadCallbackFunc cb = std::bind(&Acceptor::onAcceptEvent, this, pSc);

    pSc->setReadCallback(cb);
    pSc->enableRead();
}

void Acceptor::onAcceptEvent(ChannelPtr ptr){
    if(not (ptr->getEpEvent() & EPOLLIN)){
        return ;
    }

    string cIp;
    int cPort = 0;
    int cfd = raw_v1::doAccept(sfd_, cIp, cPort);

    raw_v1::setNonBlock(cfd);
    EndPoint ep{cfd, cIp, cPort};
    cb_afterAccept_(ep);

//    ChannelPtr pc = new Channel(p_el_->getEpollPtr(), cfd);
//    ReadCallbackFunc cb = std::bind(onRead, pc);
//
//    pc->setReadCallback(cb);
//    pc->enableRead();
//    info("accept new client fd: %d, ip: %s, port: %d", cfd, cIp.c_str(), cPort);
}

void Acceptor::setAfterAcceptCallback(AfterAcceptCallbackFunc cb){
    cb_afterAccept_ = cb;
}

Connection::Connection(EndPoint ep, EventLoopPtr p_el):ep_(ep), p_el_(p_el){
    pc_ = new Channel(p_el_->getEpollPtr(), ep_.fd);

    ReadCallbackFunc cb = std::bind(&Connection::handleEvent, this, pc_);
    pc_->setReadCallback(cb);
    pc_->enableRead();
}

void Connection::handleEvent(ChannelPtr ptr){
    int fd = ptr->getFd();
    day05::handleRead(fd);
}

int Server::init(){
    auto cb = std::bind(&Server::afterAcceptCallback, this, std::placeholders::_1);
    acceptor_.setAfterAcceptCallback(cb);
    return acceptor_.init();
}

void Server::afterAcceptCallback(EndPoint ep){
    info("accept new client %s", ep.toString().c_str());
    ConnectionPtr pc = new Connection(ep, p_el_);
    m_fd_con_[ep.fd] = pc;
}

} // day08

namespace day10{

void example(){
    Server svr;
    svr.waitForShutdown();
}

Channel::Channel(EventLoopPtr pev, int fd):p_ev_(pev), fd_(fd){
}

void Channel::enableRead(){
    events_ |= (EPOLLIN | EPOLLPRI);
    p_ev_->updateChannel(this);
    inEpoll_ = true;
}

void Channel::updateEvent(int ev){
    revents_ = ev;
}

void Channel::setHandle(EventHandle eh){
    handle_ = eh;
}

void Channel::handleEvent(){
    handle_();
}

Epoll::~Epoll(){
    if(nullptr != events_){
        delete []events_;
    }
}

int Epoll::init(){
    epfd_ = epoll_create(1);
    if(epfd_ < 0) return -1;

    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);
}

void Epoll::updateChannel(ChannelPtr pc){
    struct epoll_event epev;
    epev.events = pc->getEvent();
    epev.data.fd = pc->getFd();
    epev.data.ptr = pc;
    if(pc->inEpoll()){
        epoll_ctl(epfd_, EPOLL_CTL_MOD, pc->getFd(), &epev);
    }else{
        epoll_ctl(epfd_, EPOLL_CTL_ADD, pc->getFd(), &epev);
    }
}

int Epoll::poll(ChannelPtrList& vList, int timeout){
    int num = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    return_ret_if(num < 0, num, "epoll_wait_fail");

    for(int i = 0; i < num; ++i){
        auto ptr = ChannelPtr(events_[i].data.ptr);
        ptr->updateEvent(events_[i].events);
        vList.push_back(ptr);
    }
    return 0;
}

EventLoop::EventLoop(ThreadPoolPtr ptp):tp_(ptp){
}

EventLoop::~EventLoop(){
    if(nullptr != ep_){
        delete ep_;
    }
}

int EventLoop::init(){
    ep_ = new Epoll();
    int ret = ep_->init();
    return_ret_if(ret < 0, ret, "epoll_init_fail");
    return ret;
}

void EventLoop::loop(){
    while(true){
        ChannelPtrList vList;
        ep_->poll(vList, 2);
        for(auto ptr : vList){
            ptr->handleEvent();
        }
    }
}

void EventLoop::updateChannel(ChannelPtr pc){
    ep_->updateChannel(pc);
}

Connection::Connection(EventLoopPtr ep, EndPoint e):ep_(ep), e_(e){
    ch_ = new Channel(ep_, e.fd);

    auto cb = std::bind(&Connection::handleEvent, this);
    ch_->setHandle(cb);
    ch_->enableRead();
}

void Connection::handleEvent(){
    int fd = ch_->getFd();
    while(true) {  //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        string sData;
        int iReadSize = raw_v1::doRead(fd, sData, 1024);
        if (iReadSize > 0) {
            info("get msg from fd: %d, size: %d, %s", fd, iReadSize, sData.c_str());
            int iWriteSize = raw_v1::doWrite(fd, sData);  // < 0 简洁点去掉
            info("echo back size: %d, %s", iWriteSize, sData.c_str());
        } else if (0 == iReadSize) {  //EOF，客户端断开连接
            info("fd: %d has close", fd);
            raw_v1::doClose(fd);
            //            epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
            break;
        } else {  // -1
            int error = errno;
            if (EAGAIN == error or EWOULDBLOCK == error) {  //非阻塞IO，这个条件表示数据全部读取完毕
                info("fd: %d no data...., ret: %d error: %d, %s", fd, iReadSize, error, strerror(error));
                break;
            } else if (EINTR == error) {  //客户端正常中断、继续读取
                info("fd: %d continue reading...., ret: %d error: %d, %s", fd, iReadSize, error,
                     strerror(error));
                continue;
            } else {
                error("fd: %d read fail close it, ret: %d error: %d, %s", fd, iReadSize, error,
                      strerror(error));
                raw_v1::doClose(fd);
                close_cb_(fd);
                //                epoll_ctl(eFd, EPOLL_CTL_DEL, fd, nullptr);
                break;
            }
        }
    }
}

void Connection::setCloseCallback(CloseCallback cb){
    close_cb_ = cb;
}

Acceptor::Acceptor(EventLoopPtr ep):ep_(ep){
}

int Acceptor::init(){
    sfd_ = raw_v1::createTcpServerSocket(LOCAL_IP, PORT);
    return_ret_if(sfd_ <= 0, -1, "get_server_fd_fail");
    info("acceptor fd: %d", sfd_);

    ChannelPtr pSc = new Channel(ep_, sfd_);
    auto cb = std::bind(&Acceptor::handleEvent, this);

    pSc->setHandle(cb);
    pSc->enableRead();
    return 0;
}

void Acceptor::handleEvent(){
    string cIp;
    int cPort = 0;
    int cfd = raw_v1::doAccept(sfd_, cIp, cPort);
    return_if(cfd < 0, "accept_fail");

    raw_v1::setNonBlock(cfd);
    EndPoint ep{cfd, cIp, cPort};
    connect_cb_(ep);
}

void Acceptor::setConnectCallback(ConnectCallback cb){
    connect_cb_ = cb;
}

Server::Server(){
}

int Server::waitForShutdown(){
    mainTp_ = new ThreadPool(3);
    mainEp_ = new EventLoop(mainTp_);
    int ret = mainEp_->init();
    return_ret_if(ret < 0, ret, "main_loop_init_fail");

    subTp_ = new ThreadPool(3);

    for(int i = 0; i < subSize_; ++i){
        auto p = new EventLoop(subTp_);
        ret = p->init();
        return_ret_if(ret < 0, ret, "sub_loop_init_fail");
        subEps_.push_back(p);

        subTp_->add([p](){
            p->loop();
        });
    }

    p_acceptor_ = new Acceptor(mainEp_);
    ret = p_acceptor_->init();
    return_ret_if(ret < 0, ret, "accept_init_fail");

    auto cb = std::bind(&Server::connectCallback, this, std::placeholders::_1);
    p_acceptor_->setConnectCallback(cb);

    mainTp_->add([this](){
        mainEp_->loop();
    });

    info("init succ");
    sleep(10000);

    return 0;
}

void Server::connectCallback(EndPoint ep){
    int idx = ep.fd % subSize_;
    info("accept new client %s, sub idx: %d", ep.toString().c_str(), idx);

    auto pc = new Connection(subEps_[idx], ep);
    auto cb = std::bind(&Server::closeCallback, this, ep);
    pc->setCloseCallback(cb);

    m_fdConn[ep.fd] = pc;
}

void Server::closeCallback(EndPoint ep){
    info("delete connection %s", ep.toString().c_str());
    m_fdConn.erase(ep.fd);
}

} // day10

namespace day11{

void example_1(){
    raw_comm::EventLoopBase ep;
    int ret = ep.init();
    return_if(ret < 0, "event_loop_init_fail");

    raw_comm::Server svr(&ep);
    ret = svr.init();
    return_if(ret < 0, "svr_init_fail");

    info("%s, server init succ, now loop...", __FUNCTION__);
    ep.loop();
}


void example_codec_1(){
    log_v1::ScopeLog Log;

    raw_comm::Msg    msg1("123456");
    raw_comm::Buffer bf1;

    raw_comm::LengthCodec cc;
    cc.encode(msg1, bf1);
    Log << "bf1_size: " << bf1.size();

    raw_comm::Msg msg2;
    int ret = cc.tryDecode(bf1, msg2);
    Log << ", decode_ret: " << ret << ", decode_msg: " << msg2.data();

}

void example_string_slice(){
    {
        raw_comm::StringSlice ss("abc");
        std::cout << ss << endl;
    }

    {
        string str = "def";
        raw_comm::StringSlice ss(str);
        std::cout << ss << endl;

        auto ss2 = ss;
        std::cout << "copy_ss: " <<  ss2 << endl;
    }

}

} // day11
