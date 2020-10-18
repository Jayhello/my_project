//
// Created by root on 20-8-30.
//

#include <iostream>
#include "common/logging.h"
#include "common/thread_pool.h"
#include "network/network_address.h"
#include "network/socket.h"
#include "common_proto/common.pb.h"

using namespace comm;
using namespace comm::log;
using namespace network::address;
using namespace network::socket;

void test_service();

class Text{
public:
    virtual std::string Show()const = 0;
};

class PureText : public Text{
public:
    virtual std::string Show()const{
        return text_;
    }

    void SetText(std::string text){
        text_ = text;
    }

protected:
    std::string text_;
};

class TextDecorator:public Text{
public:
    TextDecorator(Text* p):p_obj(p){
    }

protected:
    Text* p_obj;
};

class BoldTextDecorator:public TextDecorator{
public:
    BoldTextDecorator(Text* p):TextDecorator(p){
    }

    virtual std::string Show()const{
        return "<br>" + p_obj->Show() + "</br>";
    }
};

class SpanTextDecorator: public TextDecorator{
public:
    SpanTextDecorator(Text* p):TextDecorator(p){
    }

    virtual std::string Show()const{
        return "<span>" + p_obj->Show() + "</span>";
    }
};

class Robot{
public:
    virtual std::string move() = 0;
    virtual std::string say() = 0;
};

class Dog{
public:
    std::string wang(){
        return "dog wang wang...";
    }

    std::string run(){
        return "dog run run...";
    }
};

class RobotDogAdapter: public Robot, Dog{
public:
    virtual std::string move(){
        return "robot " + Dog::run();
    }

    virtual std::string say(){
        return "robot " + Dog::wang();
    }
};

void test_adapter(){
    Robot* robot = new RobotDogAdapter();
    std::cout<<robot->say()<<std::endl;
}

void test_decorator(){
    PureText pure;
    pure.SetText("pure");

    Text* p_bold = new BoldTextDecorator(&pure);
    std::cout<<p_bold->Show()<<std::endl;

    Text* p_bold_span = new SpanTextDecorator(p_bold);
    std::cout<<p_bold_span->Show()<<std::endl;
}

struct User{
    std::string name;
    std::string pwd;
    std::string ip;
};

class PluginBase{
public:
    virtual std::string Name() = 0;
    virtual int DoCheck(User user, bool& is_spam) = 0;
};

class AccountPlugin:public PluginBase{
public:
    virtual std::string Name(){return "check name password";}
    virtual int DoCheck(User user, bool& is_spam){
        is_spam = (user.name == "xy" and user.pwd == "xy_pwd");
        return 0;
    }
};

class IpPlugin:public PluginBase{
public:
    virtual std::string Name(){return "ip white list";}
    virtual int DoCheck(User user, bool& is_spam){
        is_spam = (user.ip == "black ip");
        return 0;
    }
};

class AntiSpam{
public:
    int DoCheck(User user, bool& is_spam){
        for(auto plugin : m_vec_plugin){
            if(0 == plugin->DoCheck(user, is_spam) and is_spam){
                printf("plugin %s hit\n", plugin->Name().c_str());
                break;
            }
        }
    }

    AntiSpam& AddPlugin(PluginBase* p_plugin){
        m_vec_plugin.push_back(p_plugin);

        return *this;
    }

private:
    std::vector<PluginBase*> m_vec_plugin;
};

void test_chain(){
    User user1{"xy", "xy_pwd", ""};
    User user2{"xy2", "xy_pwd2", "black ip"};

    AntiSpam as;
    as.AddPlugin(new AccountPlugin).AddPlugin(new IpPlugin);

    bool is_spam;
    as.DoCheck(user1, is_spam);
    as.DoCheck(user2, is_spam);
}

int main() {
    Logger::getLogger().setLogLevel(Logger::LINFO);

    test_chain();

//    test_adapter();
//    test_decorator();
//    test_service();

    return 0;
}


class WorkerHandler{

};

class ClientWorker{
public:
    typedef std::unique_ptr<Socket> UpSocket;

public:
    void processWorker();

    void addSocket(UpSocket& sock);
protected:
    void refreshRunList();

private:
    std::vector<UpSocket> m_run_list;
    std::vector<UpSocket> m_wait_list;
//    std::condition_variable cond_;
    std::mutex mtx_;
};

void ClientWorker::processWorker(){
    for(auto& sock: m_run_list){
        const int buf_len = 1024;
        std::string recv_buf(buf_len, '\0');
        int recv_len = sock->Recv(&recv_buf[0], buf_len);
        if(recv_len <= 0){
            error("fd: %d recv_len: %d", sock->GetFd(), recv_len);
            sock->Close();
            continue;
        }

        info("fd: %d recv: %s, len: %d", sock->GetFd(), recv_buf.c_str(), recv_len);
        int send_len = sock->Send(&recv_buf[0], recv_len);
        if(send_len <= 0){
            error("fd: %d send_len: %d", sock->GetFd(), send_len);
            sock->Close();
            continue;
        }

        info("fd: %d, send_len: %d", sock->GetFd(), send_len);
    }

    int before_size = int(m_run_list.size());
    refreshRunList();
    int after_size = int(m_run_list.size());

    if(before_size != after_size){
        info("refreshRunList before_size: %d, after_size: %d", before_size, after_size);
    }

    if(m_run_list.empty()){
        info("now i sleep 10 s");
        std::chrono::seconds sec(10);
        std::this_thread::sleep_for(sec);
    }

}

void ClientWorker::addSocket(UpSocket& sock) {
    std::lock_guard<std::mutex> lock(mtx_);

    m_wait_list.push_back(std::move(sock));
}

void ClientWorker::refreshRunList(){
    std::lock_guard<std::mutex> lock(mtx_);

    auto it = m_run_list.begin();
    while(it != m_run_list.end()){
        if((*it)->IsClose()){
            info("fd: %d close erase it", (*it)->GetFd());
            it = m_run_list.erase(it);
        }else{
            ++it;
        }
    }

    while(not m_wait_list.empty()){
        m_run_list.push_back(std::move(m_wait_list.back()));
        m_wait_list.pop_back();
    }
}

class MultiThreadSvr{
public:
    MultiThreadSvr():sock_accept_(Socket::CreateTcpSocket()), count_(0){}
    void start(int thread_num);

    void stop(){stop_ = true;}

public:
    typedef std::unique_ptr<ClientWorker> UpClientWorker;
protected:
    void initAccept();

    void startAccept();

    void startWorker(int idx);

private:
    Socket sock_accept_;
    std::atomic_uint32_t count_;
    int thread_num_;
    std::vector<UpClientWorker> m_workers;
    bool stop_ = false;
};

void MultiThreadSvr::start(int thread_num){
    //m_workers.reserve(thread_num_);
    thread_num_ = thread_num;
    initAccept();

    std::thread th_accept([this]{
        this->startAccept();
    });

    thread_pool::ThreadPool tp(thread_num_);

    for(int i = 0 ;i < thread_num_; ++i){
        UpClientWorker p_worker(new ClientWorker());

        m_workers.push_back(std::move(p_worker));

        tp.addTask([this, i]{
            this->startWorker(i);
        });
    }

    th_accept.join();
}

void MultiThreadSvr::initAccept(){
    std::string ip = "";
    uint16_t port = 8888;
    Ip4Addr addr(ip, port);

//    sock_accept_ = Socket::CreateTcpSocket();
    sock_accept_.SetReuseAddr();
    sock_accept_.SetReusePort();

    sock_accept_.Bind(addr);
    sock_accept_.Listen();
}

void MultiThreadSvr::startWorker(int idx){
    while(not stop_){
        m_workers[idx]->processWorker();
    }
}

void MultiThreadSvr::startAccept(){
    while(not stop_){
        Ip4Addr cli_addr;
        int cli_fd = sock_accept_.Accept(&cli_addr);
        info("accept new client addr: %s, fd: %d", cli_addr.toString().c_str(), cli_fd);
        if(cli_fd < 0){
            continue;
        }

        ClientWorker::UpSocket sock_cli(new Socket(cli_fd));
        int idx = count_ % thread_num_;
        m_workers[idx]->addSocket(sock_cli);

        info("add sock to: %d, count: %u", idx, count_.load());

        count_++;
    }
}

void test_service(){
    info("now start svr...");
    MultiThreadSvr svr;
    svr.start(5);

    info("bye bye...");
}
