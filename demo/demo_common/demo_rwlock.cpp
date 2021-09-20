//
// Created by root on 20-8-9.
//

#include "common/rw_lock.h"
#include "common/thread_pool.h"
#include "common/util.h"
#include "common/logging.h"

#include <boost/thread.hpp>
#include <boost/scope_exit.hpp>

using namespace comm;
using namespace comm::log;

class BankAccount{
public:
    BankAccount():money_(0){}

    int GetMoney();

    int AddMoney(int val);

    int DelMoney(int val);

protected:
    int DeltMoney(const std::string& delt_op, int val);
private:
    int money_;
    rw_lock::RWLock rw_lock_;
};

class Counter{
public:
    Counter():count_(0){}

    int Inc(int val);
    int Del(int val);
    int Get();
    int Delt(const std::string& delt_op, int val);

private:
//    typedef boost::shared_mutex RWMutex;
//    typedef boost::unique_lock<RWMutex> WLock;
//    typedef boost::shared_lock<RWMutex> RLock;

    typedef rw_lock::ShareMutex RWMutex;
    typedef rw_lock::RW_W_Lock WLock;
    typedef rw_lock::RW_R_Lock RLock;

    RWMutex mtx_;
    int count_;
};

class SpinCount{
public:
    void add(int i);

    int get();
private:
    int count_ = 0;
    rw_lock::SpinMutex mtx_;
};

void TestRWLock();

void TestRWLockBoost();

void TestSpinLock();

int main(int argc, char** argv){
    Logger::getLogger().setLogLevel(Logger::LINFO);

    auto start = comm::util::util::timeMicro();

//    TestRWLock();
//    TestRWLockBoost();

    TestSpinLock();

    auto end = comm::util::util::timeMicro();

    std::cout << "consume: " << (end - start) << std::endl;

    return 0;
}

void TestRWLock(){
    BankAccount ba;
    auto get_fun = std::bind(&BankAccount::GetMoney, std::ref(ba));

    thread_pool::ThreadPool1 tp(4);

    for(int i = 0; i < 50; ++i){
        tp.emplace(get_fun);

        auto add_fun = std::bind(&BankAccount::AddMoney, std::ref(ba), i);
        tp.emplace(add_fun);

        auto del_fun = std::bind(&BankAccount::DelMoney, std::ref(ba), 49 - i);
        tp.emplace(del_fun);
    }
}

void TestRWLockBoost(){
    Counter counter;

    BOOST_SCOPE_EXIT_ALL(&){
        info("exit count: %d", counter.Get());
    };

    auto get_fun = std::bind(&Counter::Get, std::ref(counter));

    thread_pool::ThreadPool1 tp(4);

    for(int i = 1; i <= 100; ++i){
        tp.emplace(get_fun);

        auto add_fun = std::bind(&Counter::Inc, std::ref(counter), i);
        tp.emplace(add_fun);

        auto del_fun = std::bind(&Counter::Del, std::ref(counter), 100 - i);
        tp.emplace(del_fun);
    }
}

void TestSpinLock(){
    SpinCount counter;

    BOOST_SCOPE_EXIT_ALL(&){
        info("exit count: %d", counter.get());
    };

    auto get_fun = [&](){
        int c = 100;
        while(c--){
            int val = counter.get();
            if(val % 100 == 0){
                std::cout << "get: " << val << std::endl;
            }
        }
    };

    auto add_fun = [&](){
        for(int i = 0; i < 1000; ++i){
            counter.add(1);
        }
    };

    thread_pool::ThreadPool1 tp(8);

    tp.emplace(add_fun);
    tp.emplace(get_fun);
    tp.emplace(add_fun);
    tp.emplace(add_fun);
    tp.emplace(add_fun);
}

void SleepSec(int s){
//    std::chrono::seconds sec(s);
    std::chrono::milliseconds sec(s);
    std::this_thread::sleep_for(sec);
}

int Counter::Inc(int val){
    return Delt("Inc", val);
}

int Counter::Del(int val){
    return Delt("Del", -val);
}

int Counter::Get(){
    RLock r_lock(mtx_);

    return count_;
}

int Counter::Delt(const std::string& delt_op, int val){
    WLock w_lock(mtx_);
    count_ += val;

    return count_;
}

int BankAccount::GetMoney(){
    int val = util::util::GetRandomRange(1, 8);
    rw_lock_.lockR();

    int money = money_;
    info("now i sleep %d, money: %d", val, money);
    SleepSec(val);

    rw_lock_.unlockR();
}

int BankAccount::AddMoney(int val){
    return DeltMoney("AddMoney", val);
}

int BankAccount::DelMoney(int val){
    return DeltMoney("DelMoney", -val);
}

int BankAccount::DeltMoney(const std::string& delt_op, int delt){
    int val = util::util::GetRandomRange(1, 8);
    rw_lock_.lockW();

    money_ += delt;
    info("%s now i sleep %d, delt: %d, money: %d", delt_op.c_str(), val, delt, money_);
    SleepSec(val);

    rw_lock_.unlockW();
    return money_;
}

void SpinCount::add(int i){
    rw_lock::Lock_T<rw_lock::SpinMutex> lk(mtx_);
    count_ += i;
}

int SpinCount::get(){
    rw_lock::Lock_T<rw_lock::SpinMutex> lk(mtx_);
    return count_;
}
