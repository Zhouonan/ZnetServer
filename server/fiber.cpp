#include <stdlib.h>
#include <stdexcept>
#include <atomic>

#include "fiber.h"
#include "log.h"

namespace ZnetServer{
static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_thread_main_fiber = nullptr;
static std::atomic<uint64_t> s_fiber_id = {0};

Fiber::Fiber() {
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Fiber::Fiber main";
    m_id = s_fiber_id ++;
    if(getcontext(&m_ctx) == -1) { // 获取当前上下文
        throw std::logic_error("getcontext");
    }
    t_fiber = this; // 主协程
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
    : m_cb(std::move(cb)), m_id(s_fiber_id ++) {
    m_stack = new char[stacksize]; // 分配栈内存
    if(getcontext(&m_ctx) == -1) { // 获取当前上下文
        throw std::logic_error("getcontext");
    }
    m_stacksize = stacksize;
    m_ctx.uc_link = &t_thread_main_fiber->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack; // 设置栈指针
    m_ctx.uc_stack.ss_size = stacksize; // 设置栈大小
    
    makecontext(&m_ctx, &Fiber::MainFunc, 0); // 设置上下文函数
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Fiber::Fiber id=" << m_id << " has created";
}

Fiber::~Fiber() {
    if (m_stack) {
        delete[] static_cast<char*>(m_stack);
    }
}
void Fiber::reset(std::function<void()> cb) {
    // 重置回调函数，保留栈空间，以便重用
    if (!m_stack) {
        throw std::logic_error("Fiber::reset m_stack == nullptr");
    }
    m_cb = std::move(cb);
    if(getcontext(&m_ctx) == -1) { // 获取当前上下文
        throw std::logic_error("getcontext");
    }
    m_ctx.uc_link = &t_thread_main_fiber->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack; // 设置栈指针
    m_ctx.uc_stack.ss_size = m_stacksize; // 设置栈大小
    
    makecontext(&m_ctx, &Fiber::MainFunc, 0); // 设置上下文函数
    m_semaphore.wait();
}

void Fiber::swapIn() {
    t_fiber = this;
    if(swapcontext(&t_thread_main_fiber->m_ctx, &m_ctx) == -1) {
        throw std::runtime_error("swapcontext error");
    }
}

void Fiber::swapOut() {
    t_fiber = t_thread_main_fiber.get();
    if(swapcontext(&m_ctx, &t_thread_main_fiber->m_ctx) == -1) {
        throw std::runtime_error("swapcontext error");
    }
}

void Fiber::MainFunc() {
    ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "Fiber::MainFunc";
    Fiber::ptr cur = GetThis();
    cur->m_semaphore.notify();
    if(!cur) {
        throw std::logic_error("cannot get current fiber");
    }
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
    } catch(const std::exception& e) {
        ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "Fiber Except: " << e.what()
            << " fiber_id=" << cur->m_id;
    } catch(...) {
        ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "Fiber Except"
            << " fiber_id=" << cur->m_id;
    }
    cur->swapOut();
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) { // t_fiber是thread_local的
        // 如果当前线程的协程指针已经存在，说明主协程已创建
        return t_fiber->shared_from_this();
    }

    // --- 关键的惰性初始化 ---
    // 如果 t_fiber 为空，说明这是本线程第一次调用 GetThis()
    // 我们就在此时，此地，为它创建一个主协程
    t_thread_main_fiber.reset(new Fiber); // 创建主协程
    // t_fiber 在主协程的构造函数中被赋值为 this
    return t_thread_main_fiber;
}

}