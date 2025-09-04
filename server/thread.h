#ifndef __ZNS_THREAD_H__
#define __ZNS_THREAD_H__
#include <pthread.h>
#include <functional>
#include <iostream>

#include "log.h"
#include "mutex.h"
namespace ZnetServer{
class Thread {
public:
    using ptr = std::shared_ptr<Thread>;
    // 上下文感知，无需作为参数传递也可以get到
    static Thread* GetThis();
    static const std::string& GetName();
    
    Thread(const std::string& name, std::function<void()> cb);
    ~Thread();
    static void* run(void* arg);
    void join();
    void yield() { sched_yield(); }
private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
private:
    pid_t m_id = -1;
    pthread_t m_thread = 0; //用来保存 pthread_create 返回的线程ID
    std::function<void()> m_cb; // 线程要执行的回调函数
    std::string m_name; // 线程名称
    bool m_joined = false;
    Semaphore m_semaphore; 
};    

class ThreadException : public std::runtime_error {
public:
    ThreadException(const std::string& msg) : std::runtime_error(msg) {}
};

}


#endif 