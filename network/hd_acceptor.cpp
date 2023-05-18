//
// Created by wenwen on 2023/5/5.
//

#include "hd_acceptor.h"
#include "hd_net_util.h"

namespace hd{

Acceptor::Acceptor(int port){
    int fd = createTcpServerSocket("", port);

}

void Acceptor::handleAccept(){

}

} // hd
