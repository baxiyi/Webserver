//
// Created by mzw on 2022/3/20.
//

#include "Epoller.h"

Epoller::Epoller(int maxEvent):epollerFd_(epoll_create(512)), events_(maxEvent) {
    assert(epollerFd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epollerFd_);
}

bool Epoller::addFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epollerFd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::modFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epollerFd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool Epoller::delFd(int fd) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev;
    return epoll_ctl(epollerFd_, EPOLL_CTL_DEL, fd, &ev) == 0;
}

int Epoller::wait(int timeout) {
    return epoll_wait(epollerFd_, &events_[0], events_.size(), timeout);
}

int Epoller::getEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}

