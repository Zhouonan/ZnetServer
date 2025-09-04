#pragma once 
#include <ucontext.h>
#include <memory>
#include <functional>
#include "noncopyable.h"
#include "mutex.h"

namespace ZnetServer{
class Fiber : public std::enable_shared_from_this<Fiber>{
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State {
        INIT, // 初始态
        HOLD, // 暂停态
        EXEC, // 执行态
        TERM, // 结束态
        EXCEPT, // 异常态
    };
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();
    static uint64_t GetFiberId();
    static Fiber::ptr GetThis();
    static void Yield();
    uint32_t getId() const { return m_id;}
    void reset(std::function<void()> cb);
    void call();
    void back();    
    void swapIn();
    void swapOut();
    State getState() const { return m_state; }
    void setState(State s) { m_state = s; }
private:
    Fiber(); // 主协程
    static void MainFunc();
private:
    uint32_t m_id; // 协程id
    std::function<void()> m_cb;
    ucontext_t m_ctx;
    void* m_stack = nullptr;
    uint32_t m_stacksize = 0;
    Semaphore m_semaphore;
    State m_state = INIT;
};
}
