//
// Created by Administrator on 2022/3/11.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#endif //WEBSERVER_THREADPOOL_H

#include<thread>
#include<condition_variable>
#include<mutex>
#include<vector>
#include<queue>
#include<future>

class ThreadPool {
private:
    bool m_stop;
    std::vector<std::thread> m_thread;
    std::queue<std::function<void()>> tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;

public:
    // 每个线程都是一个循环，做的工作就是等待条件变量m_cv，如果submit了一个新的task，则m_cv释放，
    // 并从等待队列中取出一个线程，执行该task，执行完成后线程再次回到等待m_cv的状态
    explicit ThreadPool(size_t threadNumber) : m_stop(false) {
        for (size_t i = 0; i < threadNumber; i++) {
            m_thread.emplace_back([this](){
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lk(m_mutex);
                        m_cv.wait(lk, [this](){return m_stop || !tasks.empty();});
                        if (m_stop && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    // 禁止拷贝构造和移动构造
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;

    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();
        for (auto& thread : m_thread) {
            thread.join();
        }
    }

    template<typename F, typename ...Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        auto taskPtr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (m_stop) {
                throw std::runtime_error("submit on stopped ThreadPool");
            }
            tasks.emplace([taskPtr](){(*taskPtr)();});
        }
        m_cv.notify_one();
        return taskPtr->get_future();
    }


};