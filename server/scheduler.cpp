#include "scheduler.h"
#include <thread>

namespace ZnetServer {
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(int threadCount, bool user_caller, const std::string& name)
        : m_name(name) {
        
    if(user_caller) {
        Fiber::GetThis(); // 激活主协程
        -- threadCount;
        m_rootFiber = std::make_shared<Fiber>([this](){ run(); }, 1024*128);
        t_scheduler_fiber = m_rootFiber.get();
        m_rootThreadId = GetThreadId();
        
    } else {
        m_rootThreadId = -1;
    }
    m_threadCount = threadCount;
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "INIT F";
}
Scheduler::~Scheduler() {

}

Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

void Scheduler::start() {
    // std::unique_lock<std::mutex> lock(m_mutex);
    m_stopping = false;
    // 创建线程 
    m_threads.resize(m_threadCount);
    for(int i = 0; i < m_threadCount; i ++) {
        m_threads[i].reset(new Thread(
            m_name + "_" + std::to_string(i), [this](){
                ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::start() thread " << GetThreadId() << " start";
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                run();
            }
        ));
    }
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::start() end";
    // lock.unlock();
    
    // if(m_rootFiber) {
    //     m_rootFiber->call();
    // }   
      
}
void Scheduler::stop() {
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::stop()";
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stopping = true;
    }
    m_condition.notify_all();
    std::vector<Thread::ptr> thrs;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        thrs.swap(m_threads); // 使用swap技巧，避免在持有锁时析构线程对象
    }
    for (auto& thr : thrs) {
        thr->join();
    }
    if(m_rootFiber) {
        m_rootFiber->call();
    }
}
void Scheduler::tickle() {
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::tickle()";
}
void Scheduler::schedule(Fiber::ptr fiber) {
    // 加入协程队列
    bool need_tickle = false;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        need_tickle = m_fibers.empty();
        m_fibers.push_back(fiber);
        ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << m_fibers.size();
    }
    if(need_tickle) m_condition.notify_one();
}
void Scheduler::idle() {
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::idle()";
    while(!m_stopping){
        sleep(1);
        Fiber::Yield();
    }
}
void Scheduler::run() {
    if(GetThreadId() != m_rootThreadId) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::run()" << m_name
    << " thread_id=" << GetThreadId() << " m_rootThreadId=" << m_rootThreadId 
    << " t_scheduler_fiber: "<< t_scheduler_fiber->getId();
    Fiber::ptr idle_fiber = std::make_shared<Fiber>([this]() {idle();}, 1024*128);
    while(1) {
        Fiber::ptr fiber;
        {
            ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::run() before lock";
            std::unique_lock<std::mutex> lock(m_mutex);
            ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Scheduler::run() after lock";
            m_condition.wait(lock, [this] {
                    return !m_fibers.empty() || m_stopping;
            });
            if(!m_fibers.empty()) {
                fiber = m_fibers.back();
                m_fibers.pop_back();
            }
        }
        
        if(fiber) {
            std::cout << "before swap" << std::endl;
            fiber->swapIn();
            std::cout << "after swap" << std::endl;
            
        } 
        else {
            if(m_stopping) break;
            if(idle_fiber->getState() == Fiber::TERM) {
                ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "idle fiber term";
                break;
            }
            // idle_fiber->swapIn();
        }
        
    }
}
}