//
// Created by root on 20-10-24.
//
#pragma once

#include "http_utils.h"
#include <unordered_map>
#include <map>

namespace httplib{

struct CmpIgnoreCase{
    bool operator()(const std::string& s1, const std::string& s2)const{
        return iequal(s1, s2);
    }
};

typedef std::unordered_map<std::string, std::string, std::hash<std::string>, CmpIgnoreCase> Headers;

class HttpHeader{
public:
    void AddHeader(const std::string& key, const std::string& value);
    void RemoveHeader(const std::string& key);
    std::string Get(const std::string& key, const std::string& default_value="")const;

    std::string ToString();
private:
    Headers headers_;
};

} // httplib
