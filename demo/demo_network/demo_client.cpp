//
// Created by root on 20-8-29.
//

#include <iostream>
#include "common/logging.h"
#include "common/thread_pool.h"
#include "network/network_address.h"
#include "network/socket.h"
#include "common_proto/common.pb.h"

using namespace comm;
using namespace comm::log;
using namespace network::address;
using namespace network::socket;

void test_client();

int main() {
    Logger::getLogger().setLogLevel(Logger::LINFO);

    test_client();

    return 0;
}

void encode_msg(std::string& msg){
    common::Info info;
    info.set_id(1);
    info.set_name("xy");

    info.SerializeToString(&msg);
}

void decode_msg(const std::string& msg, common::Info& info){
    info.ParseFromString(msg);
}

void test_client(){
    int ret = 0;
    Socket sock_cli = Socket::CreateTcpSocket();

    std::string ip = "127.0.0.1";
    uint16_t port = 8888;
    Ip4Addr addr(ip, port);

    sock_cli.Connect(addr);

    std::string buf;
    encode_msg(buf);
    int send_len = sock_cli.Send(buf.c_str(), buf.size());
    info("send: %s, len: %d, fd: %d", buf.c_str(), send_len, sock_cli.GetFd());

    std::string recv_buf(send_len, '\0');
    int recv_len = sock_cli.Recv(&recv_buf[0], recv_buf.size());
    common::Info info;
    decode_msg(recv_buf, info);

    info("recv: %s, %s, len: %d, fd: %d", info.ShortDebugString().c_str(), recv_buf.c_str(),
         recv_len, sock_cli.GetFd());

    sock_cli.Close();

    info("cli close");
}
