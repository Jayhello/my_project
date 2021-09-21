//
// Created by wenwen on 2021/9/20.
//

#include <iostream>
#include <memory>

void uniq_ptr_demo();

int main(){
    uniq_ptr_demo();

    return 0;
}

void uniq_ptr_demo(){
    std::unique_ptr<int> pi(new int(3));
    std::cout << *pi << std::endl;

    // std::unique_ptr<int> pi2(pi); // error

    pi.release();
    pi.reset();
    if(pi){

    }
}