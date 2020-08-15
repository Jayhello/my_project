#include <yaml-cpp/yaml.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <boost/algorithm/string.hpp>

std::string yaml_path = "/home/xy/c_code/my_project/demo/demo_common/test_yaml.yaml";
using std::cout;
using std::endl;

int GetNode_(const YAML::Node& root, YAML::Node& sub){
    sub = root;
    return 0;
}

template<typename... Args>
int GetNode_(const YAML::Node& root, YAML::Node& sub, const std::string& key, const Args&...args){
    return GetNode_(root[key], sub, args...);
}

template<typename T, typename... Args>
int GetValue(T& value, const Args& ... args){
    YAML::Node root = YAML::LoadFile(yaml_path);
    YAML::Node sub;

    GetNode_(root, sub, args...);
    value = sub.as<T>();

    return 0;
}

void test_yaml(){
    YAML::Node config = YAML::LoadFile(yaml_path);

    const std::string username = config["username"].as<std::string>();
    const std::string password = config["password"].as<std::string>();

    // name: c++, pwd: xxx
    printf("\n name: %s, pwd: %s \n", username.c_str(), password.c_str());

    std::vector<std::string> vec_str = config["members"].as<std::vector<std::string>>();
    cout<<boost::algorithm::join(vec_str, ", ")<<endl; // xy1, xy2

    cout<<config["msg_info"]["id"].as<uint32_t>()<<endl; // 3

    std::string name;
    GetValue(name, "username");
    printf("\n name: %s \n", name.c_str()); // name: c++

    std::vector<std::string> members;
    GetValue(members, "members");
    printf("members: %s\n", boost::algorithm::join(members, "->").c_str());
    // members: xy1->xy2

    uint32_t id;
    GetValue(id, "msg_info", "id");
    printf("\n id: %u \n", id); // members: xy1->xy2
}

int main(int argc, char* argv[]){
    test_yaml();

    return 0;
}