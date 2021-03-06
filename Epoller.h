//
// Created by mzw on 2022/3/20.
//

#ifndef WEBSERVER_EPOLLER_H
#define WEBSERVER_EPOLLER_H

#include<vector>
#include<sys/epoll.h>
#include<fcntl.h>
#include<unistd.h>
#include<assert.h>
#include<errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);
    int wait(int timeWait = -1);

    int getEventFd(size_t i) const;
    uint32_t getEvents(size_t i) const;
private:
    int epollerFd_;
    std::vector<struct epoll_event> events_;
};


#endif //WEBSERVER_EPOLLER_H
