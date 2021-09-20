//
// Created by root on 20-10-25.
//

#include <iostream>
#include "common/logging.h"
#include "http/http_header.h"
#include "common_proto/common.pb.h"

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
    common::Info info;
    info.set_id(1);
    info.set_name("xy");
    info.PrintDebugString();

    HttpHeader header;

    header.AddHeader("api_version", "1.0.0");
    header.AddHeader("api_scene", "10");
    header.AddHeader("wxg", "10");
    cout<<header.ToString()<<endl;
}
