//
// Created by wenwen on 2021/9/20.
//

#include <iostream>
#include <memory>
#include <algorithm>

void uniq_ptr_demo();


class Base{
public:
    virtual void hi(){
        std::cout << "base hi" << std::endl;
    }
};

class Sub : public Base{
public:
    virtual void hi() override{
        std::cout << "sub hi" << std::endl;
    }
};

std::unique_ptr<Base> getSub(){
    std::unique_ptr<Sub> p_sub(new Sub);
    return p_sub;
}

void fn(const std::unique_ptr<Base>& p){
    p->hi();
}

int main(){
//    UniquePtr<int> up(new int(3));

//    UniquePtr<int> up2 = std::move(up);
//    std::cout << *up2 << std::endl;


    uniq_ptr_demo();

    return 0;
}

#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <string.h>



class BitSet{
public:
//    typedef uint32_t value_ype;
    typedef uint16_t value_ype;
    static const int PART_BIT_LEN = sizeof(value_ype) * 8;

    explicit BitSet(size_t len){
        len_ = len;
        cnt_ = len / PART_BIT_LEN + 1;
        arr_ = new value_ype[cnt_];

        reset();
    }

    ~BitSet(){
        if(arr_){
            delete[] arr_;
            arr_ = nullptr;
        }
    }

    std::string toStr(){
        std::string res(len_, '0');
        for(int i = 0; i < len_; ++i){
            if(getBit(i))res[i] = '1';
        }

        return res;
    }

    void reset(){
        memset((void*)arr_, 0, cnt_ * sizeof(value_ype));
    }

    bool getBit(int idx){
        int i1 = idx / PART_BIT_LEN;
        int i2 = idx % PART_BIT_LEN;
        return arr_[i1] & (1 << i2);
    }

    bool test(int idx){
        return getBit(idx);
    }

    void setBit(int idx){
        int i1 = idx / PART_BIT_LEN;
        int i2 = idx % PART_BIT_LEN;
        arr_[i1] |= ((1 << i2));
    }

    size_t  len_;
    int cnt_;
    value_ype*   arr_;
};


void uniq_ptr_demo(){
    std::string res(35, '0');

BitSet bs(35);
bs.setBit(1);
bs.setBit(4);
bs.setBit(6);
bs.setBit(34);

    std::cout << bs.toStr() << std::endl;
//    bs.reset();
//    std::cout << bs.toStr() << std::endl;

    std::cout << bs.getBit(1) << std::endl;
    std::cout << bs.getBit(10) << std::endl;
    std::cout << bs.getBit(6) << std::endl;

    /*
    std::string path = "/tmp/r_file.txt";
std::ifstream fi;
fi.open(path, std::ios_base::in);

char buff[200] = {'\0'};
fi >> buff;
printf("%s\n", buff); // line
    std::string str = buff;
    std::cout << str.size() << ", " << str << std::endl;


std::vector<int> vec{1, 2, 3};
std::ofstream fout;
fout.open("/tmp/vector.txt");

for (int i = 0; i < vec.size(); i++)
    fout << vec[i] << std::endl;

std::istringstream iss;
*/

}
