//
// Created by Administrator on 2022/10/9.
//

#include "raw_server_1.h"
#include "raw_comm.h"

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start server1 demo");

    v1::echoServer();

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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

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