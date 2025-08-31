#include "../server/fiber.h"
#include "../server/log.h"

int main() {
    ZnetServer::Fiber::ptr main_fiber = ZnetServer::Fiber::GetThis();
    ZnetServer::Fiber::ptr f(new ZnetServer::Fiber([](){
        ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "hello fiber";
    }, 1024 * 128));
    f->swapIn();
    
    return 0;
}