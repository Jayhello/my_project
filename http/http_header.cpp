//
// Created by root on 20-10-24.
//

#include "http_header.h"
#include "common/util.h"

namespace httplib{

void HttpHeader::AddHeader(const std::string& key, const std::string& value){
    headers_[key] = value;
}

void HttpHeader::RemoveHeader(const std::string& key){
    if(headers_.count(key)){
        headers_.erase(key);
    }
}

std::string HttpHeader::Get(const std::string& key, const std::string& default_value)const{
    if(headers_.count(key)){
        return headers_.at(key);
    }

    return default_value;
}

std::string HttpHeader::ToString(){
    std::string res;

    for(const auto& item : headers_){
        std::string tmp = comm::util::util::format("%s:%s\n", item.first.c_str(), item.second.c_str());
        res += tmp;
    }

    return res;
}

} // httplib
