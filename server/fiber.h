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
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();
    static Fiber::ptr GetThis();
    uint32_t getId() const { return m_id;}
    void reset(std::function<void()> cb);
    void swapIn();
    void swapOut();
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
};
}
