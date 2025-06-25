#ifndef __ZNS_LOG_H__
#define __ZNS_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

#define ZNS_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        ZnetServer::LogEventWrap(ZnetServer::Logger::ptr(logger), ZnetServer::LogEvent::ptr(new ZnetServer::LogEvent(\
                        __FILE__, __LINE__, 0, (uint32_t)gettid(), \
                     ZnetServer::GetFiberId(), time(0), "", level))).getSS()

#define ZNS_LOG_DEBUG(logger) ZNS_LOG_LEVEL(logger, ZnetServer::LogLevel::DEBUG)
#define ZNS_LOG_INFO(logger) ZNS_LOG_LEVEL(logger, ZnetServer::LogLevel::INFO)
#define ZNS_LOG_WARN(logger) ZNS_LOG_LEVEL(logger, ZnetServer::LogLevel::WARN)
#define ZNS_LOG_ERROR(logger) ZNS_LOG_LEVEL(logger, ZnetServer::LogLevel::ERROR)
#define ZNS_LOG_FATAL(logger) ZNS_LOG_LEVEL(logger, ZnetServer::LogLevel::FATAL)

namespace ZnetServer
{
    class Logger;
    // 日志级别
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static const char* ToString(LogLevel::Level level);
    };
    
    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(const char *file, int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time, const std::string &content, LogLevel::Level level);

    private:
        const char *m_file = nullptr;
        int32_t m_line = 0;
        uint32_t m_elapse = 0;
        uint32_t m_threadId = 0;
        uint32_t m_fiberId = 0;
        uint64_t m_time = 0;
        std::stringstream m_ss;
        LogLevel::Level m_level;
    public:
        const char *getFile() const { return m_file; }
        void setFile(const char *file) { m_file = file; }

        int32_t getLine() const { return m_line; }
        void setLine(int32_t line) { m_line = line; }

        uint32_t getElapse() const { return m_elapse; }
        void setElapse(uint32_t elapse) { m_elapse = elapse; }

        uint32_t getThreadId() const { return m_threadId; }
        void setThreadId(uint32_t threadId) { m_threadId = threadId; }

        uint32_t getFiberId() const { return m_fiberId; }
        void setFiberId(uint32_t fiberId) { m_fiberId = fiberId; }

        uint64_t getTime() const { return m_time; }
        void setTime(uint64_t time) { m_time = time; }

        std::string getContent() const { return m_ss.str(); }
        std::stringstream& getSS() { return m_ss; }

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level level) { m_level = level; }
    };

    // 日志事件包装器RAII
    class LogEventWrap
    {
    public:
        LogEventWrap(std::shared_ptr<Logger> logger, LogEvent::ptr event)
            :m_logger(logger), m_event(event)
        {
        }
        ~LogEventWrap();

        std::stringstream& getSS() { return m_event->getSS(); }
    private:
        std::shared_ptr<Logger> m_logger;
        LogEvent::ptr m_event;
    };

    // 日志格式器
    // %m -- 消息体
    // %p -- 进程号
    // %r -- 启动时间
    // %c -- 日志名称
    // %t -- 线程id
    // %n -- 换行
    // %d -- 时间
    // %f -- 文件名
    // %l -- 行号
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string &pattern);
        //%t   %thread_id %m%n
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem // 子模块？
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {};
            // virtual std::string format(LogEvent::ptr event) = 0;
            // 直接整合到流里性能会更好一点
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) = 0;
        };

        void init(); // pattern解析
    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        LogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
        virtual ~LogAppender() {};

        virtual void log(LogLevel::Level level, LogEvent::ptr event, std::shared_ptr<Logger> logger) = 0;
        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }

    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root", LogLevel::Level level = LogLevel::Level::DEBUG);
        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);

        std::string getName(){return m_name;}
        LogLevel::Level getLevel(){return m_level;}
        void setLevel(LogLevel::Level level){m_level = level;}
    private:
        std::string m_name;
        LogLevel::Level m_level;
        std::list<LogAppender::ptr> m_appenders;
        LogFormatter::ptr m_formatter;
        Logger::ptr m_root;
    };

    // 输出到控制台的Appender
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(LogLevel::Level level, LogEvent::ptr event, std::shared_ptr<Logger> logger) override;
    };

    // 输出到文件
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(LogLevel::Level level, LogEvent::ptr event, std::shared_ptr<Logger> logger) override;

        bool reopen();

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };

}

#endif