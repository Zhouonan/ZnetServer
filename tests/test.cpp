#include "log.h"

int main()
{
    ZnetServer::Logger::ptr logger(new ZnetServer::Logger);
    logger->addAppender(ZnetServer::LogAppender::ptr(new ZnetServer::StdoutLogAppender));

    ZnetServer::LogEvent::ptr event(new ZnetServer::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0), "hello", ZnetServer::LogLevel::DEBUG));
    logger->log(ZnetServer::LogLevel::DEBUG, event);

    return 0;
}
