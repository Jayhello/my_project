//
// Created by Administrator on 2022/3/21.
//
#pragma once
#include "noncopyable.h"

namespace comm{

template <typename T>
class Singleton : public noncopyable{
public:
    typedef   T*     pointer;

    static pointer getInstance(){
        static T  t;
        return &t;
    }
};

template <typename T>
class ThreadSingleton : public noncopyable{
public:
    typedef   T*     pointer;

    static pointer getInstance(){
        static thread_local T  t;
        return &t;
    }
};

} // comm
