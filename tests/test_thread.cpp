#include "../server/thread.h"

int main() {
    ZnetServer::Logger::ptr logger = ZNS_LOG_NAME("threadTest");
    ZnetServer::Thread thread("test", [](){
        std::cout << "-------------- test ----------------" << std::endl;
        ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "thread name: " << ZnetServer::Thread::GetName();
    });
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "thread name: " << ZnetServer::Thread::GetName();
    ZNS_LOG_INFO(logger) << "current thread id: " << ZnetServer::Thread::GetThis();
    
    thread.join();
}