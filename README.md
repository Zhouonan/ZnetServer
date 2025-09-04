# 高性能服务器框架

学习sylar项目
## 阶段一：基础设施
日志系统 (Logger, Appender, Formatter)

配置系统 (Config)

线程与互斥锁 (Thread, Mutex)

## 阶段二：核心骨架
多路复用与事件循环 (Poller, Channel, EventLoop)

目标：这是现代高性能服务器的核心。你需要理解为什么“一个连接一个线程”的模式不可取，以及如何用epoll（在Linux下）实现单线程管理成千上万的网络连接。

复刻重点：EventLoop是事件驱动的核心，它不断地调用epoll_wait等待事件发生，然后根据事件类型（读、写、错误）分发给对应的Channel（通道，封装了文件描述符和它关心的事件）去处理。这是整个项目中最核心、最值得花时间的模块之一。

协程库 (Fiber, Scheduler)

目标：这是sylar项目的特色和精华。你需要理解什么是协程（用户态的轻量级线程），以及它如何解决异步编程中的“回调地狱”问题。

复刻重点：

ucontext库：理解它是如何实现用户态上下文切换的。

Fiber类：封装协程的上下文、栈空间、状态等。

Scheduler类：这是协程调度器，它将协程和我们前面做的EventLoop结合起来。当一个协程发起IO操作（比如read）时，它不是真的阻塞，而是被Scheduler切换出去（yield），并注册一个IO事件到EventLoop中。当EventLoop检测到IO完成时，再由Scheduler将这个协程切换回来（resume）继续执行。

## 阶段三：网络与协议
这个阶段是在核心骨架上添加“血肉”，让服务器能真正对外提供服务。

网络地址与Socket封装 (Address, Socket)

目标：将底层的C风格的socket API封装成面向对象的C++类，同样是利用RAII来保证资源（文件描述符）的正确释放。

HTTP协议解析与服务 (HttpRequest, HttpResponse, HttpServer)

目标：在前面所有模块的基础上，构建一个能解析HTTP请求、处理并返回HTTP响应的服务器。

复刻重点：HTTP协议本身的状态机解析。HttpServer类如何将一个新的TCP连接、HTTP协议解析、业务逻辑处理（Servlet）和协程调度串联起来。