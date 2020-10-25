//
// Created by root on 20-10-24.
//

#include "http_utils.h"

namespace httplib{

bool iequal(const std::string& s1, const std::string& s2){
    int len = static_cast<int>(s1.size());
    if(len != s2.size()){
        return false;
    }

    for(int i = 0; i < len;++i){
        if(std::tolower(s1[i]) != std::tolower(s2[i])){
            return false;
        }
    }

    return true;

}

} // httplib