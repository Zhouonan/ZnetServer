#include "thread.h"

namespace ZnetServer {
static thread_local Thread* t_thread = nullptr; // thread_local的实现原理是什么
static thread_local std::string t_thread_name = "UNKNOWN";

static Logger::ptr t_logger = ZNS_LOG_NAME("thread"); // 静态不静态区别大不大
// 避免了“全局变量污染”
Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}

Thread::Thread(const std::string& name, std::function<void()> cb) 
    : m_cb(cb), m_name(name) {
    if (name.empty()) {
        m_name = "UNKNOW";
    }
    
    // 调用 pthread_create，把 this 指针作为参数传给 run 函数
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this); 
    if (rt) {
        ZNS_LOG_ERROR(t_logger) << "pthread_create fail";
        throw std::logic_error("pthread_create fail"); // 为什么是逻辑错误
    }
    m_semaphore.wait(); // 等待线程初始化完成
}

Thread::~Thread() {
    if(m_thread && !m_joined) {
        // 保证资源一定会被回收，且不会卡死析构函数
        pthread_detach(m_thread);
    }
}

void* Thread::run(void* arg) {
    Thread* thread = (Thread*) arg;
    // 设置自己的核心变量
    t_thread = thread;
    t_thread_name = thread->m_name;
    // 获取内核线程id
    thread->m_id = syscall(SYS_gettid);
    // 设置线程在内核中的名字，方便调试
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
    thread->m_semaphore.notify(); // 通知线程创建完成
    std::function<void()> cb;
    std::swap(cb, thread->m_cb); // 交换函数对象，避免在析构时调用
    cb(); // 执行线程回调函数
    
    return nullptr; // 这个为什么返回空指针
}

void Thread::join() {
    if (m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if (rt) {
            ZNS_LOG_ERROR(t_logger) << "pthread_join error";
            throw std::logic_error("pthread_create error");
        }
        m_joined = true;
    }
    
}    
}
