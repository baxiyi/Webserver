//
// Created by Administrator on 2022/3/7.
//

#ifndef WEBSERVER_TIME_H
#define WEBSERVER_TIME_H

#include<queue>
#include<deque>
#include<unordered_map>
#include<ctime>
#include<chrono>
#include<functional>
#include<memory>
#include "HTTPConnection.h"

typedef std::function<void()> TimeoutCallback;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

class TimerNode {
public:
    int id;
    TimeStamp expire;
    TimeoutCallback callback;

    bool operator < (const TimerNode& t) {
        return expire < t.expire;
    }
};

class TimerManager {
    typedef std::shared_ptr<TimerNode> SP_TimerNode;
public:
    TimerManager() {heap_.reserve(64);}
    ~TimerManager() {clear();}

    void addTimer(int id, int timeout, const TimeoutCallback& cb);
    void handle_expired_event();
    int getNextHandle();
    void update(int id, int timeout);
    void work(int id);
    void pop();
    void clear();
private:
    // 堆的操作
    void del_(size_t i);
    void shiftup_(size_t i);
    bool shiftdown_(size_t i, size_t n);
    void swapNode_(size_t i, size_t j);

    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_; // 存储一个定时器在heap_中的位置
};

#endif //WEBSERVER_TIME_H
