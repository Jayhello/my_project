//
// Created by wenwen on 2021/9/20.
//

#pragma once

#include <iostream>

/*
 * 此文件简单实现下智能指针, unique_ptr, shared_ptr...
 */
template<typename T>
class UniquePtr{
public:
    typedef T* Ptr;
    typedef T value_type;
    typedef T& Reference;

    explicit UniquePtr(Ptr p):ptr_(p){}

    ~UniquePtr(){
        destory();
    }

    UniquePtr& operator=(UniquePtr&& rhs){
        destory();
        reset(rhs.release());
        return *this;
    }

    Ptr get()const{
        return ptr_;
    }

    Ptr operator->()const{
        return get();
    }

    value_type operator*(){
        return *ptr_;
    }

    explicit operator bool()const{
        return get() != nullptr;
    }

    Ptr release(){
        Ptr tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    Ptr reset(Ptr p){
        Ptr res = ptr_;

        if(p != ptr_){
            ptr_ = p;
        }

        return res;
    }

    void swap(UniquePtr& rhs){
        std::swap(ptr_, rhs.ptr_);
    }

private:
    void destory(){
        if(ptr_){
            delete ptr_;
            ptr_ = nullptr;
        }
    }

private:
    Ptr operator=(const Ptr) = delete;

    UniquePtr(const UniquePtr& rhs) = delete;

private:
    Ptr ptr_;
};

template<typename T>
struct Deletor{
    void operator()(T* ptr){
        delete ptr;
    }
};

template<typename T, typename Deletor>
class UniquePtr2 {
public:
    typedef T* Ptr;

    UniquePtr2(){
        if(ptr_){

        }
    }

    Ptr ptr_;
};
