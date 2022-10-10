//
// Created by Administrator on 2022/10/9.
//

#include "raw_comm.h"


namespace raw_v1{

int getTcpSocket(){
    return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int doConnect(int fd, const string& sIp, int iPort){
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(sIp.c_str());
    serv_addr.sin_port = htons(iPort);

    return ::connect(fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
}

int doBind(int fd, const string& sIp, int iPort){
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(sIp.c_str());
    serv_addr.sin_port = htons(iPort);

    return bind(fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
}

int doListen(int fd, int n){
    return ::listen(fd, n);
}

int doAccept(int fd, string& sIp, int& iPort){
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));

    int cfd = accept(fd, (sockaddr*)&clnt_addr, &clnt_addr_len);
    if(cfd > 0){
        sIp = inet_ntoa(clnt_addr.sin_addr);
        iPort = clnt_addr.sin_port;
    }

    return cfd;
}

int doWrite(int fd, const string& sData){
    return ::write(fd, sData.c_str(), sData.size());
}

int doRead(int fd, string& sData, size_t iReadSize){
    sData.reserve(iReadSize);
    ssize_t read_bytes = read(fd, &sData[0], iReadSize);
    sData.resize(read_bytes >= 0 ? read_bytes : 0);
    return read_bytes;
}

int doClose(int fd){
    return ::close(fd);
}

} // raw_v1
