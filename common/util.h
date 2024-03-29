//
// Created by root on 20-8-9.
//

#pragma once

#include "noncopyable.h"
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace comm{
namespace util{

struct util {
    static std::string format(const char *fmt, ...);
    static int64_t timeMicro();
    static int64_t timeMilli() { return timeMicro() / 1000; }
    static int64_t steadyMicro();
    static int64_t steadyMilli() { return steadyMicro() / 1000; }
    static std::string readableTime(time_t t);
    static int64_t atoi(const char *b, const char *e) { return strtol(b, (char **) &e, 10); }
    static int64_t atoi(const char *b) { return atoi(b, b + strlen(b)); }
    static uint64_t gettid();
    static int GetRandomRange(int min, int max);
};

struct ExitCaller : private noncopyable {
    ~ExitCaller() { functor_(); }
    ExitCaller(std::function<void()> &&functor) : functor_(std::move(functor)) {}

private:
    std::function<void()> functor_;
};

} // util
} // comm
