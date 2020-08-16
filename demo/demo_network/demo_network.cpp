//
// Created by root on 20-8-16.
//

#include <iostream>
#include "common/logging.h"
#include "common/thread_pool.h"
#include "network/network_address.h"

using namespace comm;
using namespace comm::log;
using namespace network::address;

void test_address();

int main() {
    Logger::getLogger().setLogLevel(Logger::LINFO);

    test_address();

    info("bye bye...");
    return 0;
}

void test_address(){
    Ip4Addr ip_add("198.110.11.10", 80);

    printf("ip_add: %s\n", ip_add.toString().c_str());
}
