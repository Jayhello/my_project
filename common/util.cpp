//
// Created by root on 20-8-9.
//

#include "util.h"
#include <stdarg.h>
#include <algorithm>
#include <chrono>
#include <memory>
#include <pthread.h>
#include <random>

namespace comm{
namespace util{

using namespace std;

string util::format(const char *fmt, ...) {
    char buffer[500];
    unique_ptr<char[]> release1;
    char *base;
    for (int iter = 0; iter < 2; iter++) {
        int bufsize;
        if (iter == 0) {
            bufsize = sizeof(buffer);
            base = buffer;
        } else {
            bufsize = 30000;
            base = new char[bufsize];
            release1.reset(base);
        }
        char *p = base;
        char *limit = base + bufsize;
        if (p < limit) {
            va_list ap;
            va_start(ap, fmt);
            p += vsnprintf(p, limit - p, fmt, ap);
            va_end(ap);
        }
        // Truncate to available space if necessary
        if (p >= limit) {
            if (iter == 0) {
                continue;  // Try again with larger buffer
            } else {
                p = limit - 1;
                *p = '\0';
            }
        }
        break;
    }
    return base;
}

int64_t util::timeMicro() {
    chrono::time_point<chrono::system_clock> p = chrono::system_clock::now();
    return chrono::duration_cast<chrono::microseconds>(p.time_since_epoch()).count();
}
int64_t util::steadyMicro() {
    chrono::time_point<chrono::steady_clock> p = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::microseconds>(p.time_since_epoch()).count();
}

std::string util::readableTime(time_t t) {
    struct tm tm1;
    localtime_r(&t, &tm1);
    return format("%04d-%02d-%02d %02d:%02d:%02d", tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
}

uint64_t util::gettid() {
    pthread_t tid = pthread_self();
    uint64_t uid = 0;
    memcpy(&uid, &tid, std::min(sizeof(tid), sizeof(uid)));
    return uid;
}

int util::GetRandomRange(int min, int max){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(min, max);

    return distr(gen);
}

} // util
} // comm