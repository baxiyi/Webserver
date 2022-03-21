//
// Created by Administrator on 2022/3/13.
//

#include "webserver.h"

WebServer::WebServer(int port, int trigMode, int timeoutMs, bool optLinger, int threadNum):
port_(port), openLinger_(optLinger), timeoutMS_(timeoutMs), isClose_(false),
timer_(new TimerManager()), threadPool_(new ThreadPool(threadNum)), epoller_(new Epoller())
{
    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16);
    HTTPConnection::userCount = 0;
    HTTPConnection::srcDir = srcDir_;
    initEventMode_(trigMode);
    if (!initSocket_()) {
        isClose_ = true;
    }
}

WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
}

void WebServer::initEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connectionEvent_ = EPOLLONESHOT;
    switch (trigMode) {
        case 0:
            break;
        case 1:
            connectionEvent_ |= EPOLLET;
            break;
        case 2:
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            listenEvent_ |= EPOLLET;
            connectionEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            connectionEvent_ |= EPOLLET;
            break;
    }
    HTTPConnection::isET = (connectionEvent_ & EPOLLET);
}

void WebServer::Start() {
    int timeMs = -1;
    if(!isClose_)
    {
        std::cout<<"============================";
        std::cout<<"Server Start!";
        std::cout<<"============================";
        std::cout<<std::endl;
    }
    while (!isClose_) {
        if (timeoutMS_ > 0) {
            timeMs = timer_->getNextHandle();
        }
        int eventCnt = epoller_->wait(timeMs);
        for (int i = 0; i < eventCnt; i++) {
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            if (fd == listenFd_) {
                handleListen_();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                closeConn_(&users_[fd]);
            } else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                handleRead_(&users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                handleWrite_(&users_[fd]);
            } else {
                std::cout << "unexpected event" << std::endl;
            }
        }
    }
}

void WebServer::sendError_(int fd, const char *info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        std::cout << "send error to client" << fd << "error" << std::endl;
    }
    close(fd);
}

void WebServer::closeConn_(HTTPConnection *client) {
    assert(client);
    epoller_->delFd(client->getFd());
    client->closeHTTPConn();
}

void WebServer::addClientConnection(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].initHTTPConn(fd, addr);
    if (timeoutMS_ > 0) {
        timer_->addTimer(fd, timeoutMS_, std::bind(&WebServer::closeConn_, this, &users_[fd]));
    }
    epoller_->addFd(fd, EPOLLIN | connectionEvent_);
    setFdNonblock(fd);
}

void WebServer::handleListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&addr, &len);
        if (fd <= 0) {
            return;
        }
        if (HTTPConnection::userCount >= MAX_FD) {
            sendError_(fd, "Server busy!");
            return;
        }
        addClientConnection(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

void WebServer::handleRead_(HTTPConnection *client) {
    assert(client);
    extentTime_(client);
    threadPool_->submit(std::bind(&WebServer::onRead_, this, client));
}

void WebServer::handleWrite_(HTTPConnection *client) {
    assert(client);
    extentTime_(client);
    threadPool_->submit(std::bind(&WebServer::onWrite_, this, client));
}

void WebServer::extentTime_(HTTPConnection *client) {
    assert(client);
    if (timeoutMS_ > 0) {
        timer_->update(client->getFd(), timeoutMS_);
    }
}

void WebServer::onRead_(HTTPConnection *client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client -> readBuffer(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN) {
        closeConn_(client);
        return;
    }
    onProcess_(client);
}

void WebServer::onProcess_(HTTPConnection *client) {
    if (client->handleHTTPConn()) {
        // 通知主线程可写事件就绪
        epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLOUT);
    } else {
        epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLIN);
    }
}

void WebServer::onWrite_(HTTPConnection *client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->writeBuffer(&writeErrno);
    if (client->writeBytes() == 0) {
        if (client->isKeepAlive()) {
            onProcess_(client);
            return;
        }
    } else if (ret < 0) {
        if (writeErrno == EAGAIN) {
            epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLOUT);
            return;
        }
    }
    closeConn_(client);
}

bool WebServer::initSocket_() {
    int ret;
    struct sockaddr_in addr;
    if (port_ > 65535 || port_ < 1024) {
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    struct linger optLinger = {0};
    if (openLinger_) {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    // protocol为0表示默认协议
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        return false;
    }
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if (ret < 0) {
        close(listenFd_);
        return false;
    }
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if (ret < 0) {
        close(listenFd_);
        return false;
    }
    ret = bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        close(listenFd_);
        return false;
    }
    ret = listen(listenFd_, 5);
    if (ret < 0) {
        close(listenFd_);
        return false;
    }
    ret = epoller_->addFd(listenFd_, listenEvent_ | EPOLLIN);
    if (ret == 0) {
        close(listenFd_);
        return false;
    }
    setFdNonblock(listenFd_);
    return true;
}

int WebServer::setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}