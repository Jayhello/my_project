#include <iostream>
#include "common/logging.h"

using namespace comm::log;

int main() {
    Logger::getLogger().setLogLevel(Logger::LINFO);

    info("hello world");

    return 0;
}