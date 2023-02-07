//
// Created by wenwen on 2023/2/7.
//

#include "hd_example.h"
#include "network/hd_ev.h"

int main(){
    hd_ex::simple_timer();

    return 0;
}

namespace hd_ex{

void simple_timer(){
    info("%s start task", __FUNCTION__);

    hd::EpollTimer timer;
    int ret = timer.init();
    info("timer init ret: %d", ret);

    timer.runAfter(100, [](){
        info("task 1");
    });

    timer.runAt(NOW_MS + 1200, []{
        info("task 2");
    });

    timer.runEvery(2200, []{
        info("repeat task 3");
    });

    timer.loop();
}

} // hd_ex