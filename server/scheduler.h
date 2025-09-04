#pragma once
#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <condition_variable>

#include "fiber.h"
#include "thread.h"

namespace ZnetServer {
class Scheduler {
public:
    Scheduler(int n_threads = 1, bool user_caller = true, const std::string& name = "");
    virtual ~Scheduler(); 
    static Fiber* GetMainFiber();
    void start();
    void stop();
    void tickle();
    void schedule(Fiber::ptr fiber);
private:
    void run();
    void idle();
private:
    std::string m_name;
    std::mutex m_mutex;
    int m_threadCount;
    std::vector<Thread::ptr> m_threads;
    std::list<Fiber::ptr> m_fibers;
    bool m_stopping = true;
    Fiber::ptr m_rootFiber;
    pid_t m_rootThreadId;
    Semaphore m_semaphore;
    std::condition_variable m_condition;
};
}