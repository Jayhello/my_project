//
// Created by wenwen on 2021/9/20.
//

#include <iostream>
#include <memory>

void uniq_ptr_demo();
void uniq_ptr_c();

int main(){
//    uniq_ptr_demo();

    uniq_ptr_c();

    return 0;
}

class Base{
public:
    virtual void print(){printf("this is base: %d\n", a_);}

    int a_ = 0;
};

class Sub: public Base{
public:
    virtual void print(){printf("this is sub: %d\n", a_);}
};

template<typename T>
class TestDerive{
public:
    void invoke(){
        t.print();
    }

    T t;
};

typedef TestDerive<Base> TestDeriveBase;
typedef TestDerive<Sub> TestDeriveTestDerive;

void test_tem(TestDeriveBase& tb){
    tb.invoke();
}

void uniq_ptr_demo(){
//    TestDeriveTestDerive td;
//    test_tem(td);  // compile error



    std::unique_ptr<int> pi(new int(3));
    std::cout << *pi << std::endl;

    // std::unique_ptr<int> pi2(pi); // error

    pi.release();
    pi.reset();
    if(pi){

    }
}

typedef std::unique_ptr<Base> BasePtr;
typedef std::unique_ptr<Sub>  SubPtr;

void uniq_ptr_c(){
    BasePtr bp = SubPtr(new Sub);
    bp->print();
}