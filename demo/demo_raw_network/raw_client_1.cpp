//
// Created by Administrator on 2022/10/9.
//

#include "raw_client_1.h"
#include "raw_comm.h"

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start client1 demo");

    v1::echoClient();

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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    int ret = connect(fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    return_if(ret < 0, "connect err: %d", ret);

    while(true){
        char buf[1024];
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);
        ssize_t write_bytes = write(fd, buf, sizeof(buf));
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

    int ret = raw_v1::doConnect(fd, "127.0.0.1", 8888);
    return_if(ret < 0, "connect_fail");
    info("connect succ");

    string input;
    while(cin >> input){
        if(input.substr(0, 1) == "q"){
            info("input q break");
            break;
        }

        int iWriteSize = raw_v1::doWrite(fd, input);
        if(iWriteSize < 0){
            warn("write fail: %d", iWriteSize);
            break;
        }
    }

    raw_v1::doClose(fd);
}

} // v2
