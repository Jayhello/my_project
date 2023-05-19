//
// Created by wenwen on 2023/5/5.
//

#include "hd_acceptor.h"
#include "hd_net_util.h"

namespace hd{

Acceptor::Acceptor(EventLoop* ep, int port):loop_(ep){
    int fd = createTcpServerSocket("", port);
    acceptChannel_ = new Channel(fd, loop_);

}

void Acceptor::handleAccept(){

}

} // hd
