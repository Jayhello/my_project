//
// Created by wenwen on 2021/9/20.
//

#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>
#include <thread>
#include "common/singleton.h"
#include "common/logging.h"
#include <sstream>

void test_thread_singleton();

using comm::log::Logger;

int main(){
//    Logger::getLogger().setLogLevel(Logger::LINFO);

    test_thread_singleton();

    return 0;
}


//class DbHandle : public comm::ThreadSingleton<DbHandle>{
class DbHandle : public comm::Singleton<DbHandle>{
public:
    void print_addr(){
        std::cout << "addr: " << &(*this) << std::endl;
    }
};

void test_thread_singleton(){
    int num = 3;
    std::vector<std::thread> vTh;
    for(int i = 0; i < num; ++i){
        vTh.emplace_back(std::thread([](){
            DbHandle::getInstance()->print_addr();
        }));
    }

    for(int i = 0; i < num; ++i){
        vTh[i].join();
    }
}
