#include <iostream>
#include "common/logging.h"
#include "common/thread_pool.h"

using namespace comm;
using namespace comm::log;


int main() {
    Logger::getLogger().setLogLevel(Logger::LINFO);

    info("hello world");
    return 0;
}