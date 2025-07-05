#include "../server/log.h"
#include "../server/util.h"
#include <unistd.h>

int main()
{
    ZnetServer::Logger::ptr logger(new ZnetServer::Logger("ZNNN"));
    logger->addAppender(ZnetServer::LogAppender::ptr(new ZnetServer::StdoutLogAppender));

    ZnetServer::LogEvent::ptr event(new ZnetServer::LogEvent(
        __FILE__, __LINE__, 0, (uint32_t)gettid(), ZnetServer::GetFiberId(),
        time(0), "hello", ZnetServer::LogLevel::DEBUG));
    event->getSS() << "hello ZnetServer" << std::endl;
    logger->log(ZnetServer::LogLevel::DEBUG, event);
    logger->setLevel(ZnetServer::LogLevel::INFO);
    // TEST01被过滤
    ZNS_LOG_DEBUG(logger) << "TEST01";
    ZNS_LOG_INFO(logger) << "TEST02";
    ZNS_LOG_WARN(logger) << "TEST03";
    ZNS_LOG_ERROR(logger) << "TEST04";
    ZNS_LOG_FATAL(logger) << "TEST05";
    // std::cout << "main thread id: " << gettid() << std::endl;

    auto l = ZnetServer::LoggerMgr::GetInstance()->getLogger("root");
    ZNS_LOG_DEBUG(l) << "TEST06";
    return 0;
}
