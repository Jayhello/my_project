//
// Created by Administrator on 2022/10/9.
//

#include "raw_server_1.h"
#include "raw_comm.h"
#include <set>

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start server1 demo");

//    v1::echoServer();
//    v2::echoServer();
//    v3::echoServer();
//    v4::selectExample();
//    v4::selectServer();
    v5::epollServer();

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
                        raw_v1::setNonBlock(cfd);
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
                    }else if(0 == iReadSize){
                        info("fd: %d has close", events[i].data.fd);
                        raw_v1::doClose(events[i].data.fd);
                        epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                        break;
                    }else{
                        int error = errno;
                        if(EAGAIN == error or EWOULDBLOCK == error or EINTR == error){
                            info("fd: %d no data...., ret: %d error: %d, %s", events[i].data.fd, iReadSize, error, strerror(error));
                        }else{
                            error("fd: %d read fail close it, ret: %d error: %d, %s", events[i].data.fd, iReadSize, error, strerror(error));
                            raw_v1::doClose(events[i].data.fd);
                            epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
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
