//
// Created by Administrator on 2022/3/13.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Epoller.h"
#include "Time.h"
#include "ThreadPool.h"
#include "HTTPConnection.h"

class WebServer {
public:
    WebServer(int port, int trigMode, int timeoutMs, bool optLinger, int threadNum);
    ~WebServer();
    void Start();

private:
    bool initSocket_();
    void initEventMode_(int trigMode);
    void addClientConnection(int fd, sockaddr_in addr);
    void closeConn_(HTTPConnection *client);

    void handleListen_();
    void handleWrite_(HTTPConnection *client);
    void handleRead_(HTTPConnection *client);

    void onRead_(HTTPConnection *client);
    void onWrite_(HTTPConnection *client);
    void onProcess_(HTTPConnection *client);
    void sendError_(int fd, const char* info);
    void extentTime_(HTTPConnection* client);

    static const int MAX_FD = 65536;
    static int setFdNonblock(int fd);

    int port_;
    int timeoutMS_;
    bool isClose_;
    int listenFd_;
    bool openLinger_;
    char* srcDir_;

    uint32_t listenEvent_;
    uint32_t connectionEvent_;

    std::unique_ptr<TimerManager> timer_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HTTPConnection> users_;
};


#endif //WEBSERVER_WEBSERVER_H
