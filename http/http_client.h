//
// Created by root on 20-10-24.
//
#pragma once

#include "http_request.h"

namespace httplib{

class HttpClient{
public:
    int Get(const std::string& url, std::string& resp);

    int Post(const std::string& url, const Request& req);

private:

};

} // httplib
