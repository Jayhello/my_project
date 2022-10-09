//
// Created by Administrator on 2022/10/9.
//

#include "raw_client_1.h"
#include "common/logging.h"
#include "common/thread_pool.h"

using namespace comm;
using namespace comm::log;

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);
    info("start client1 demo");

    info("exit client1 demo");
    return 0;
}

namespace v1{


} // v1