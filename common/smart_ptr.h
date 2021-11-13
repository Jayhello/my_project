//
// Created by wenwen on 2021/9/20.
//

#pragma once

#include <iostream>
#include <atomic>

/*
 * 此文件简单实现下智能指针, unique_ptr, shared_ptr...
 */
template<typename T>
class UniquePtr{
public:
    typedef T* Ptr;
    typedef T value_type;
    typedef UniquePtr<T> this_type;

    explicit UniquePtr(Ptr p = nullptr):ptr_(p){}

//    UniquePtr(UniquePtr&& rhs){
//        reset(rhs.release());
//    }

    UniquePtr(UniquePtr&& rhs):ptr_(rhs.release()){
    }

    this_type& operator=(UniquePtr&& rhs){
        reset(rhs.release());
        return *this;
    }

    template<typename U>
    UniquePtr& operator=(UniquePtr<U>&& rhs){
        reset(rhs.release());
        return *this;
    }

    ~UniquePtr(){
        reset();
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

    void reset(Ptr p = nullptr){
        if(p != ptr_){
            delete ptr_;
            ptr_ = p;
        }
    }

    void swap(UniquePtr& rhs){
        std::swap(ptr_, rhs.ptr_);
    }

protected:
    UniquePtr(const UniquePtr& rhs);
    UniquePtr& operator=(const Ptr);

private:
    Ptr ptr_;
};

template<typename T>
class sharedPtr{
public:
    typedef T*  pointer;

    struct SharedCounter{
        pointer ptr;
        std::atomic<int> cnt;
    };

    explicit sharedPtr(pointer ptr = nullptr){
        obj_->ptr = ptr;
        obj_->cnt = 1;
    }

//    sharedPtr(const sharedPtr& rhs){
    sharedPtr(sharedPtr& rhs){
        ++rhs.obj_->cnt;
        obj_ = rhs.obj_;
    }

    SharedCounter& operator=(sharedPtr& rhs){

    }

    ~sharedPtr(){
        --obj_->cnt;
        if(0 == obj_->cnt and !obj_->ptr){
            delete obj_->ptr;
        }
    }

private:
    SharedCounter* obj_;
};
