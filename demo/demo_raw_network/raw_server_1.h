//
// Created by Administrator on 2022/10/9.
//
#pragma once


// 原始的 socket 实现的echo server
namespace v1{

void echoServer();

} // v1

// 使用封装的函数, 示例
namespace v2{

void echoServer();

}

/*
 1. 如果每个链接来了, 都丢到线程去异步处理, 有个问题, fd阻塞的方式read不到数据
 如果client链接之后必须, 立马发送数据, 如果不发那么线程就会一直阻塞在这个fd.

 2. 将socket设置为非阻塞, 一个线程可以遍历所有的socket检查是否有数据读, 有则去处理
    a. 得一直轮询不能sleep, 这样消息处理不及时
    b. 大多fd数链接没有数据轮询就有点浪费了
*/

namespace v3{

void echoServer();

} // v3

// select example
namespace v4{

void selectExample();

void selectServer();

} // v4
