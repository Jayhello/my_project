//
// Created by root on 20-10-24.
//
#pragma once

#include "http_header.h"

namespace httplib{

class Request{
public:

private:
    HttpHeader header_;
    std::string url_;
    std::string body_;
};

} // httplib
