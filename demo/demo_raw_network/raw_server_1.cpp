//
// Created by Administrator on 2022/10/9.
//

#include "raw_server_1.h"
#include "raw_comm.h"

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start server1 demo");

//    v1::echoServer();
//    v2::echoServer();
    v3::echoServer();

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
        vFd_.push_back(cfd);
    }

    virtual ~ProcessTask(){
        for(auto fd : vFd_)raw_v1::doClose(fd);
    }

    void stop(){stop_ = true;}
protected:
    virtual void run() override{
        while(not stop_){
            handlerAll();
        }
    }

    void handlerAll(){
        for(auto cfd : vFd_){
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
        sleep(1);
    }

    std::vector<int>  vFd_;
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
