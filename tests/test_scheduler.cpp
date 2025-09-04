#include "../server/scheduler.h"

int main() {
    // 1. 创建一个调度器，内含2个工作线程
    ZnetServer::Scheduler sc(2, false); 

    // 2. 启动调度器，后台线程开始 run() 循环
    sc.start(); 

    // 3. 向调度器投递一个打印任务
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "schedule";
    sc.schedule(std::make_shared<ZnetServer::Fiber>([](){
        ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "Hello, Scheduler!";
    }, 1024 * 128));

    // 4. 停止调度器（等待所有任务执行完毕）
    sleep(2);
    sc.stop(); 
    return 0;
}