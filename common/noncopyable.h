//
// Created by Administrator on 2022/3/21.
//
#pragma once

namespace comm{

class noncopyable {
protected:
    // 子类可以构造和析构
    noncopyable() = default;
    ~noncopyable() = default;

private:
    noncopyable(const noncopyable &) = delete;
    noncopyable& operator=(const noncopyable &) = delete;
};

/*
struct noncopyable {
protected:
    noncopyable() = default;
    virtual ~noncopyable() = default;   // 没有必要为 virtual

private:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};
 */

/*  boost
class noncopyable{
protected:
    noncopyable() {}
    ~noncopyable() {}
private:  // emphasize the following members are private
    noncopyable( const noncopyable& );
    const noncopyable& operator=( const noncopyable& );
};
*/

} // comm
