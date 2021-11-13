//
// Created by wenwen on 2021/9/20.
//

#include <iostream>
#include <memory>
#include <algorithm>
#include "common/smart_ptr.h"

void uniq_ptr_demo();

template<typename T, T v>
struct integral_constant{
    static const T value = v;
    typedef T value_type;
    typedef integral_constant<T, v> type;
};

struct false_type: public integral_constant<bool, false>{
};

struct true_type: public integral_constant<bool, true>{
};

template<typename T>
struct remove_const{
    typedef T type;
};

template<typename T>
struct remove_const<const T>{
    typedef T type;
};


template<typename T>
struct remove_volatile{
    typedef T type;
};

template<typename T>
struct remove_volatile<volatile T>{
    typedef T type;
};

template <typename T>
struct remove_cv{
    typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

template<typename T>
struct is_integral_helper : public false_type{
};

template<>
struct is_integral_helper<int> : public true_type{
};

template<>
struct is_integral_helper<long> : public true_type{
};

template<typename T>
struct is_integral : public is_integral_helper<typename remove_cv<T>::type>{
};

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



    UniquePtr<int> up(new int(3));

    UniquePtr<int> up2 = std::move(up);
//    std::cout << *up2 << std::endl;


//    uniq_ptr_demo();

    return 0;
}

void uniq_ptr_demo(){
    std::unique_ptr<int> pi(new int(3));
    std::cout << *pi << std::endl;

    // std::unique_ptr<int> pi2(pi); // error

    pi.release();
    pi.reset();


}