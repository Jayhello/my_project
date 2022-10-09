//
// Created by Administrator on 2022/10/9.
//

#include "raw_server_1.h"

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


} // v1