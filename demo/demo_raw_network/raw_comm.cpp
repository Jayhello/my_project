//
// Created by Administrator on 2022/10/9.
//

#include "raw_comm.h"


namespace raw_v1{

int getTcpSocket(){
    return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int doConnect(int fd, const string& sIp, int iPort){
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(sIp.c_str());
    serv_addr.sin_port = htons(iPort);

    return ::connect(fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
}

int doBind(int fd, const string& sIp, int iPort){
    int op = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(sIp.c_str());
    serv_addr.sin_port = htons(iPort);

    return bind(fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
}

int doListen(int fd, int n){
    return ::listen(fd, n);
}

int createTcpServerSocket(const string& sIp, int iPort){
    int socketFd = getTcpSocket();
    if(socketFd < 0) return -1;

    setReuseAddr(socketFd);
    setReusePort(socketFd);

    int ret = raw_v1::doBind(socketFd, sIp, iPort);
    if(ret < 0)return -2;

    ret = doListen(socketFd);
    if(ret < 0) return -3;
    return socketFd;
}

int doAccept(int fd, string& sIp, int& iPort){
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));

    int cfd = accept(fd, (sockaddr*)&clnt_addr, &clnt_addr_len);
    if(cfd > 0){
        sIp = inet_ntoa(clnt_addr.sin_addr);
        iPort = clnt_addr.sin_port;
    }

    return cfd;
}

int doWrite(int fd, const string& sData){
    return ::write(fd, sData.c_str(), sData.size());
}

int doRead(int fd, string& sData, size_t iReadSize){
    // 如下不能直接read(fd, string) 有问题
//    sData.reserve(iReadSize + 1);
//    ssize_t read_bytes = read(fd, const_cast<char*>(sData.data()), iReadSize);
//    sData.resize(read_bytes > 0 ? read_bytes + 1 : 0);
    char arr[1024]= {0};
    ssize_t read_bytes = read(fd, arr, iReadSize);
    if(read_bytes > 0){
        sData.assign(arr, read_bytes);
    }

    return int(read_bytes);
}

// < 0 错误 , > 0 写的数据size  (这个write没有返回0的)
int doSend(int fd, const string& sData){
    return ::send(fd, sData.c_str(), sData.size(), 0);
}

// < 0 错误,  = 0 对方关闭了链接, > 0 写的数据size
int doRecv(int fd, string& sData, size_t iReadSize){
    char arr[1024]= {0};
    ssize_t read_bytes = recv(fd, arr, iReadSize, 0);
    if(read_bytes > 0){
        sData.assign(arr, read_bytes);
    }

    return int(read_bytes);
}

int doClose(int fd){
    return ::close(fd);
}

int setReuseAddr(int fd){
    int flag = 1;
    return setSocketOpt(fd, SO_REUSEADDR, (void*)(&flag), (socklen_t)(sizeof(flag)), SOL_SOCKET);
}

int setReusePort(int fd){
    int flag = 1;
    return setSocketOpt(fd, SO_REUSEPORT, (void*)(&flag), (socklen_t)(sizeof(flag)), SOL_SOCKET);
}

int setSocketOpt(int fd, int opt, const void* val, socklen_t opt_len, int level){
    return setsockopt(fd, level, opt, &val, opt_len);;
}

int setNonBlock(int fd, bool value) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return errno;
    }
    if (value) {
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

void setNoBlock(int fd){
    int flag = fcntl(fd, F_GETFL, 0);

    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

} // raw_v1

namespace raw_comm{

ChannelBase::ChannelBase(const EndPoint& ePoint, EventLoopPtr ptrEl): ePoint_(ePoint), ptrEl_(ptrEl){
    ptrEl_->getPollBase()->addChannel(this);   // 构造的时候加入到epoll
}

void ChannelBase::enableRead(bool enable){
    if(enable){
        events_ |= kReadEvent;
    }else{
        events_ &= (~kReadEvent);
    }
    ptrEl_->getPollBase()->updateChannel(this);
}

void ChannelBase::enableWrite(bool enable){
    if(enable){
        events_ |= kWriteEvent;
    }else{
        events_ &= (~kWriteEvent);
    }
    ptrEl_->getPollBase()->updateChannel(this);
}

bool ChannelBase::readEnabled()const{
    return events_ | kReadEvent;
}

bool ChannelBase::writEnabled()const{
    return events_ | kWriteEvent;
}

int EpollBase::init(){
    epfd_ = epoll_create(1);
    return_ret_if(epfd_ < 0, -1, "epoll_create_fail");

    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);

    return 0;
}

void EpollBase::addChannel(ChannelPtr pc){
    struct epoll_event epev;
    memset(&epev, 0, sizeof(epev));

    epev.events = pc->getEvent();
    epev.data.fd = pc->getFd();
    epev.data.ptr = pc;
    epoll_ctl(epfd_, EPOLL_CTL_ADD, pc->getFd(), &epev);
}

void EpollBase::updateChannel(ChannelPtr pc){
    struct epoll_event epev;
    memset(&epev, 0, sizeof(epev));

    epev.events = pc->getEvent();
    epev.data.fd = pc->getFd();
    epev.data.ptr = pc;
    epoll_ctl(epfd_, EPOLL_CTL_MOD, pc->getFd(), &epev);
}

ChannelPtrList EpollBase::pollOnce(int timeoutMs){
    ChannelPtrList vRes;
    int num = epoll_wait(epfd_, events_, MAX_EVENTS, timeoutMs);
    for(int i = 0; i < num; ++i){
        auto ptr = ChannelPtr(events_[i].data.ptr);
        ptr->setActiveEvent(events_[i].events);
        vRes.push_back(ptr);
    }

    return vRes;
}

int EventLoopBase::init(){
    ptrPoll_ = new EpollBase();
    int ret = ptrPoll_->init();
    return_ret_if(ret < 0, ret, "poll_init_fail");

    init_ = true;

    return 0;
}

EventLoopBase::~EventLoopBase(){
    if(ptrPoll_){
        delete ptrPoll_;
        ptrPoll_ = nullptr;
    }
}

void EventLoopBase::loop(){
    while(not stop_){
        ChannelPtrList vActive = ptrPoll_->pollOnce(1000);

        for(auto pc : vActive){
            if(pc->canRead()){
                pc->handleRead();
            }
            if(pc->canWrite()){
                pc->handleWrite();
            }
        }
    }
}

ConnectionBase::ConnectionBase(EventLoopPtr ptrEl, const EndPoint& ePoint):ptrEl_(ptrEl), ePoint_(ePoint){
    ptrChannel_ = new ChannelBase(ePoint, ptrEl);   // 构造的时候add到epoll中

    {
        auto rcb = std::bind(&ConnectionBase::onRead, this);
        ptrChannel_->setReadHandle(rcb);

        auto wcb = std::bind(&ConnectionBase::onWrite, this);
        ptrChannel_->setWriteHandle(wcb);
    }
}

void ConnectionBase::onRead(){
    int fd = ptrChannel_->getFd();
    while(true) {  //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        string sData;
        int iReadSize = raw_v1::doRead(fd, sData, 1024);
        if (iReadSize > 0) {
            rBuf_.append(sData);
            Msg msg;
            int tmp = codec_.tryDecode(rBuf_, msg);
            info("get msg from fd: %d, size: %d, %s, decode_ret: %d", fd, iReadSize, sData.c_str(), tmp);
            if(tmp > 0){
                onMsg(msg);
                rBuf_.remove(0, tmp);
            }
        } else if (0 == iReadSize) {  //EOF，客户端断开连接
            info("fd: %d has close", fd);
            raw_v1::doClose(fd);
            //            epoll_ctl(eFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
            break;
        } else {  // -1
            int error = errno;
            if (EAGAIN == error or EWOULDBLOCK == error) {  //非阻塞IO，这个条件表示数据全部读取完毕
                info("fd: %d no data...., ret: %d error: %d, %s", fd, iReadSize, error, strerror(error));
                break;
            } else if (EINTR == error) {  //客户端正常中断、继续读取
                info("fd: %d continue reading...., ret: %d error: %d, %s", fd, iReadSize, error,
                     strerror(error));
                continue;
            } else {
                error("fd: %d read fail close it, ret: %d error: %d, %s", fd, iReadSize, error,
                      strerror(error));
                raw_v1::doClose(fd);
                break;
            }
        }
    }
}

void ConnectionBase::onMsg(const Msg& msg){
    info("ep: %s, on_msg: %s", ePoint_.toString().c_str(), msg.c_str());

    int iWriteSize = raw_v1::doWrite(ePoint_.fd, msg.data());  // < 0 简洁点去掉
    info("echo back size: %d", iWriteSize);
}

void ConnectionBase::onWrite(){

}

ConnectionBase* ConnectionBase::createConnection(EventLoopPtr ptrEl, string ip, int port){
    int fd = raw_v1::getTcpSocket();
    if(fd < 0){
        error("create socket fail");
        return nullptr;
    }

    info("fd: %d", fd);

    raw_v1::setNonBlock(fd);
    int ret = raw_v1::doConnect(fd, ip, port);

    EndPoint endPoint;
    endPoint.fd = fd;
    ConnectionBase* pc = new ConnectionBase(ptrEl, endPoint);
//    iState_ = eHandshaking;

}

AcceptorBase::AcceptorBase(EventLoopPtr ptrEl):ptrEl_(ptrEl){
}

int AcceptorBase::init(){
    ePoint_.fd = raw_v1::createTcpServerSocket(LOCAL_IP, PORT);
    return_ret_if(ePoint_.fd <= 0, -1, "get_server_fd_fail");
    info("acceptor fd: %d", ePoint_.fd);

    {
        ptrChannel_ = new ChannelBase(ePoint_, ptrEl_);
        auto readCb = std::bind(&AcceptorBase::onAccept, this);
        ptrChannel_->setReadHandle(readCb);
        ptrChannel_->enableRead(true);
    }

    return 0;
}

void AcceptorBase::onAccept(){
    string cIp;
    int cPort = 0;
    int cfd = raw_v1::doAccept(ePoint_.fd, cIp, cPort);
    return_if(cfd < 0, "accept_fail");

    raw_v1::setNonBlock(cfd);
    EndPoint ep{cfd, cIp, cPort};
    acceptCallback_(ep);
}

void AcceptorBase::setAcceptCallback(AcceptCallback cb){
    acceptCallback_ = cb;
}

int Server::init(){
    int ret = 0;
    ptrAcceptor_ = new AcceptorBase(ptrEl_);
    ret = ptrAcceptor_->init();
    return_ret_if(ret < 0, ret, "acceptor_init_fail");

    {
        auto cb = std::bind(&Server::acceptCallback, this, std::placeholders::_1);
        ptrAcceptor_->setAcceptCallback(cb);
    }

    return 0;
}

void Server::acceptCallback(const EndPoint& ep){
    ConnectionPtr pc = new ConnectionBase(ptrEl_, ep);
    info("accept new client %s, sub", ep.toString().c_str());
    mFdConnection_[ep.fd] = pc;
}

void LengthCodec::encode(const Msg& msg, Buffer& buf){
//    buf.resize(sizeof(MAGIC) + msg.size());
//    memcpy((void*)(&MAGIC), buf.c_str(), sizeof(MAGIC));

//    buf.append((char*)(&CodecBase::MAGIC), sizeof(CodecBase::MAGIC));  // head magic
    buf.append((char*)(&MAGIC), sizeof(MAGIC));  // head magic

    int len = msg.size();
    buf.append((char*)(&len), sizeof(len));      // length
    buf.append(msg.data());                      // data
}

// < 0 buf数据异常, = 0 数据不完整, > 0 解析出了一个多大的msg包
int LengthCodec::tryDecode(const Buffer& buf, Msg& msg){
    if(buf.size() < 8){
        return 0;
    }

//    if(0 != memcmp(buf.c_str(), (char*)(&CodecBase::MAGIC), sizeof(CodecBase::MAGIC))){
    if(0 != memcmp(buf.c_str(), (char*)(&MAGIC), sizeof(MAGIC))){
        return -1;
    }

    int len = *((int*)(buf.c_str() + 4));
    if(len > 10 * 1024 * 124)return -2;

    if(buf.size() >= len + 8){
        msg.append(buf.c_str() + 8, len);
        return len + 8;
    }

    return 0;
}


} // raw_comm

