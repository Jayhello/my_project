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

// 如果每个链接来了, 都丢到线程去异步处理, 有个问题, fd阻塞的方式read不到数据
