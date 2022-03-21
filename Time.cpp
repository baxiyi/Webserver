//
// Created by Administrator on 2022/3/7.
//

#include "Time.h"
void TimerManager::shiftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while (j >= 0) {
        if (heap_[j] < heap_[i]) {
            break;
        }
        swapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

void TimerManager::swapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

bool TimerManager::shiftdown_(size_t i, size_t n) {
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t start = i;
    size_t j = i * 2 + 1;
    while (j < n) {
        if (j + 1 < n && heap_[j + 1] < heap_[j]) {
            j++;
        }
        if (heap_[i] < heap_[j]) {
            break;
        }
        swapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > start;
}

void TimerManager::addTimer(int id, int timeout, const TimeoutCallback &cb) {
    assert(id >= 0);
    size_t i;
    if (ref_.count(id) == 0) {
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        shiftup_(i);
    } else {
        i = ref_[id];
        heap_[i].expire = Clock::now() + MS(timeout);
        heap_[i].callback = cb;
        if (!shiftdown_(i, heap_.size())) {
            shiftup_(i);
        }
    }
}

void TimerManager::work(int id) {
    if (heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.callback();
    del_(i);
}

void TimerManager::del_(size_t i) {
    assert(!heap_.empty() && i >= 0 && i < heap_.size());
    size_t index = i;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if (i < n) {
        swapNode_(i, n);
        if (!shiftdown_(i, n)) {
            shiftup_(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void TimerManager::update(int id, int timeout) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expire = Clock::now() + MS(timeout);
    // 更新当前定时器只会增加expire，所以只需要下调，不需要上调
    shiftdown_(ref_[id], heap_.size());
}

void TimerManager::handle_expired_event() {
    if (heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expire - Clock::now()).count() > 0) {
            break;
        }
        node.callback();
        pop();
    }
}

void TimerManager::pop() {
    assert(!heap_.empty());
    del_(0);
}

void TimerManager::clear() {
    ref_.clear();
    heap_.clear();
}

int TimerManager::getNextHandle() {
    handle_expired_event();
    size_t res = -1;
    if (!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expire - Clock::now()).count();
        if (res < 0) {
            res = 0;
        }
    }
    return res;
}