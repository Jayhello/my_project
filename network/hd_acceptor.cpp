//
// Created by wenwen on 2023/5/5.
//

#include "hd_acceptor.h"
#include "hd_net_util.h"
#include "common/logging.h"

namespace hd{

using namespace::comm::log;

Acceptor::Acceptor(EventLoop* ep, int port):loop_(ep){
    int fd = createTcpServerSocket("", port);
    fatalif(fd < 0, "accept create fd fail %d, %s", errno, strerror(errno));
    acceptChannel_ = new Channel(fd, loop_);
    acceptChannel_->enableRead();
    auto callbackRead = std::bind(&Acceptor::handleAccept, this);
    acceptChannel_->setReadCallback(callbackRead);
}

void Acceptor::handleAccept(){

}

} // hd
