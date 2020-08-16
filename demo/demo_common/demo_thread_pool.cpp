//
// Created by root on 20-8-9.
//

#include <iostream>
#include "common/logging.h"
#include "common/thread_pool.h"

using namespace comm;
using namespace comm::log;

void fake_get_db(uint32_t id, std::string& name){
    info("now get from db id: %u", id);

    std::chrono::seconds sec(3);
    std::this_thread::sleep_for(sec);
    name = std::to_string(id);
}

int fake_download(const std::string& url){
    info("now download url: %s", url.c_str());

    std::chrono::seconds sec(3);
    std::this_thread::sleep_for(sec);
    info("done");
}

void test_ThreadPool1(){
    thread_pool::ThreadPool1 tp(3);

    uint32_t id = 1101;
    std::string name;
    tp.emplace(fake_get_db, id, name);

    std::string url = "www.qq.com";
    tp.emplace(fake_download, url);
}

void test_ThreadPool(){
    thread_pool::ThreadPool tp(3);

    uint32_t id = 1101;
    std::string name;
    tp.addTask(std::bind(fake_get_db, id, name));

    std::string url = "www.qq.com";
    tp.addTask(std::bind(fake_download, url));

//    std::chrono::seconds sec(4);
//    std::this_thread::sleep_for(sec);
}

int main() {
    Logger::getLogger().setLogLevel(Logger::LINFO);

    test_ThreadPool();

    info("add done");
    return 0;
}