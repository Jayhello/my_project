//
// Created by Administrator on 2022/10/9.
//

#include "raw_server_1.h"
#include <fcntl.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include "common/logging.h"
#include "common/thread_pool.h"

using namespace comm;
using namespace comm::log;

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start server1 demo");

    info("exit server1 demo");
    return 0;
}

namespace v1{

void echoServer(){
    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

}

} // v1