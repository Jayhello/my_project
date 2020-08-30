//
// Created by root on 20-8-16.
//

#include <iostream>
#include "common/logging.h"
#include "common/thread_pool.h"
#include "network/network_address.h"
#include "network/socket.h"

using namespace comm;
using namespace comm::log;
using namespace network::address;
using namespace network::socket;

void test_address();

void test_Socket_Svr();

int main() {
    Logger::getLogger().setLogLevel(Logger::LINFO);

//    test_address();
    test_Socket_Svr();

    info("bye bye...");
    return 0;
}

void test_address(){
    Ip4Addr ip_add("198.110.11.10", 80);

    printf("ip_add: %s\n", ip_add.toString().c_str());
}

void test_Socket_Svr(){
    int ret = 0;
    Socket sock_svr(Socket::CreateTcpSocket());
//    sock_svr = Socket::CreateTcpSocket();
//    sock_svr = Socket::CreateTcpFd();

    std::string ip = "127.0.0.1";
    uint16_t port = 8888;
    Ip4Addr addr(ip, port);

    sock_svr.Bind(addr);

    sock_svr.Listen();

    Ip4Addr cli_addr;
    int cli_fd = sock_svr.Accept(&cli_addr);

    info("new cli addr: %s, fd: %d", cli_addr.toString().c_str(), cli_fd);

    Socket sock_cli = Socket(cli_fd);

    const int LEN = 1024;
    std::string buf(LEN, '\0');
    int recv_len = sock_cli.Recv(&buf[0], LEN);

    info("recv: %s, len: %d", buf.c_str(), recv_len);

    int send_len = sock_cli.Send(buf.c_str(), recv_len);
    info("send len: %d", send_len);

    info("svr exit.............");
}