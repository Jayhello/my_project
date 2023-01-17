//
// Created by Administrator on 2022/10/9.
//

#include "raw_client_1.h"
#include "raw_comm.h"

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start client1 demo");

//    v1::echoClient();
//    v2::echoClient();
    v3::echoClient();
//    v4::echoSelectClient();

    info("exit client1 demo");
    return 0;
}

namespace v1{

void echoClient(){
    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return_if(fd < 0, "fd: %d err", fd);

    info("client fd: %d", fd);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(LOCAL_IP);
    serv_addr.sin_port = htons(PORT);

    int ret = connect(fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    return_if(ret < 0, "connect err: %d", ret);

    while(true){
        char buf[1024];
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);
//        ssize_t write_bytes = write(fd, buf, sizeof(buf));
        ssize_t write_bytes = write(fd, buf, strlen(buf));
        if(write_bytes == -1){
            info("socket already disconnected, can't write any more!");
            break;
        }
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(fd, buf, sizeof(buf));
        if(read_bytes > 0){
            info("message from server: %s", buf);
        }else if(read_bytes == 0){
            info("server socket disconnected!");
            break;
        }else if(read_bytes == -1){
            error("socket read error");
            break;
        }
    }

    info("bye....");
    close(fd);
}

} // v1

// 使用封装的函数, 示例
namespace v2{

void echoClient(){
    int fd = raw_v1::getTcpSocket();
    return_if(fd <= 0, "get_socket_fd_fail");
    info("fd: %d", fd);

    int ret = raw_v1::doConnect(fd, LOCAL_IP, PORT);
//    return_if(ret < 0, "connect_fail ret: %d, %s", ret, strerror(ret));
    return_if(ret < 0, "connect_fail ret: %d, %s", ret, strerror(errno));
    info("connect succ");

    string input;
    while(1){
        char buf[1024];
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);
        input.assign(buf, strlen(buf));

        if(input.substr(0, 1) == "q"){
            info("input q break");
            break;
        }

        int iWriteSize = raw_v1::doWrite(fd, input);
        if(iWriteSize < 0){
            warn("write fail: %d", iWriteSize);
            break;
        }

        info("send : %s, size: %d to server", input.c_str(), iWriteSize);

        string sData;
        int iReadSize = raw_v1::doRead(fd, sData, 1024);
        if(iReadSize > 0) {
            info("get msg from fd: %d, size : %d, %s", fd, iReadSize, sData.c_str());
        }else if(0 == iReadSize){
            info("server has close");
            break;
        }else{
            error("fd: %d read fail close it", fd);
            break;
        }
    }

    raw_v1::doClose(fd);
}

} // v2

namespace v3{

void echoClient(){
    std::vector<int> vec;
    const int LOOP_CNT = 2;
    for(int i = 0; i < LOOP_CNT; ++i){
        int fd = raw_v1::getTcpSocket();
        return_if(fd <= 0, "get_socket_fd_fail");
        info("fd: %d", fd);

        int ret = raw_v1::doConnect(fd, LOCAL_IP, PORT);
        return_if(ret < 0, "connect_fail ret: %d, %s", ret, strerror(errno));
        info("connect succ");

        raw_v1::setNonBlock(fd);
        vec.push_back(fd);
    }

    for(int i = 0; i < LOOP_CNT; ++i){
        for(auto fd : vec){
            string input = comm::util::util::format("%d_%d", fd, i);
            int iWriteSize = raw_v1::doWrite(fd, input);
            if(iWriteSize < 0){
                warn("write fail: %d", iWriteSize);
                break;
            }

            info("send : %s, size: %d to server", input.c_str(), iWriteSize);

            string sData;
            int iReadSize = raw_v1::doRead(fd, sData, 1024);
            if(iReadSize > 0) {
                info("get msg from fd: %d, size : %d, %s", fd, iReadSize, sData.c_str());
            }else if(0 == iReadSize){
                info("server has close");
                break;
            }else{
                int error = errno;
                if(EINTR == error){
                    info("fd: %d no data...., ret: %d error: %d, %s", fd, iReadSize, error, strerror(error));
                }else if(EAGAIN == error or EWOULDBLOCK == error){
//                    raw_v1::doClose(fd);
//                    break;
                }else{
                    error("fd: %d read fail close it, ret: %d error: %d, %s", fd, iReadSize, error, strerror(error));
                    raw_v1::doClose(fd);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(i * 10));
        }

        info("loop: %d", i);
    }

    info("exit");
}

} // v3

// 使用 select 示例
namespace v4{

void echoSelectClient(){
    int fd = raw_v1::getTcpSocket();
    return_if(fd <= 0, "get_socket_fd_fail");
    info("fd: %d", fd);

    int ret = raw_v1::doConnect(fd, LOCAL_IP, PORT);
    //    return_if(ret < 0, "connect_fail ret: %d, %s", ret, strerror(ret));
    return_if(ret < 0, "connect_fail ret: %d, %s", ret, strerror(errno));
    info("connect succ");

    fd_set   fds;
    while(1){
        FD_ZERO(&fds);

        FD_SET(0, &fds);  // 把标准输入的文件描述符加入到集合中
        FD_SET(fd, &fds);    // 把当前连接的文件描述符加入到集合中

        timeval  tv {5, 100};
        ret = select(fd + 1, &fds, NULL, NULL, &tv);
        if(ret < 0){
            int ge = errno;
            error("select fail ret: %d, errno: %d, %s", ret, ge, strerror(ge));
            break;
        }else if(0 == ret){
            info("select timeout, wait next");
        }else{
            if(FD_ISSET(fd, &fds)){
                string sData;
                int iReadSize = raw_v1::doRecv(fd, sData, 1024);
                if(iReadSize > 0) {
                    info("get msg from fd: %d, size : %d, %s", fd, iReadSize, sData.c_str());
                }else{
                    int ge = errno;
                    error("server fail ret: %d, errno: %d, %s", ret, ge, strerror(ge));
                    break;
                }
            }

            if(FD_ISSET(0, &fds)){
                string input;
                std::cin >> input;
                if(input[0] == 'q'){
                    info("input q so break");
                    break;
                }

                int iSendSize = raw_v1::doSend(fd, input);
                if(iSendSize < 0){
                    warn("send fail: %d", iSendSize);
                    break;
                }

                info("send : %s, size: %d to server", input.c_str(), iSendSize);
            }
        }
    }

    raw_v1::doClose(fd);
    info("client exit");
}

} // v4
