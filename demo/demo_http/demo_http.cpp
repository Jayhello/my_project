//
// Created by root on 20-10-25.
//

#include <iostream>
#include "common/logging.h"
#include "http/http_header.h"

void test_header();

int main(){
    test_header();

    return 0;
}

using namespace comm;
using namespace comm::log;
using namespace httplib;
using std::cout;
using std::endl;

void test_header(){
    HttpHeader header;

    header.AddHeader("api_version", "1.0.0");
    header.AddHeader("api_scene", "10");
    header.AddHeader("wxg", "10");
    cout<<header.ToString()<<endl;
}
