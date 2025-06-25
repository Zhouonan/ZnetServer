#include "log.h"
#include <map>
#include <functional>
#include <sstream>

namespace ZnetServer
{
    LogEvent::LogEvent(const char *file, int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time, const std::string &content, LogLevel::Level level)
        : m_file(file), m_line(line), m_elapse(elapse), m_threadId(threadId), m_fiberId(fiberId), m_time(time), m_level(level)
    {
    }
    
    const char *LogLevel::ToString(LogLevel::Level level)
    {

        switch (level)
        {
#define LOG_LEVEL_TO_STRING(level) \
    case level:                    \
        return #level;             \
        break;

            LOG_LEVEL_TO_STRING(DEBUG)
            LOG_LEVEL_TO_STRING(INFO)
            LOG_LEVEL_TO_STRING(WARN)
            LOG_LEVEL_TO_STRING(ERROR)
            LOG_LEVEL_TO_STRING(FATAL)

#undef LOG_LEVEL_TO_STRING
        default:
            return "UNKNOWN";
        }
        return "UNKNOWN";
    }
    LogEventWrap::~LogEventWrap()
    {
        m_event->getSS() << std::endl;
        m_logger->log(m_event->getLevel(), m_event);
    }
    
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << event->getContent();
        }
    };
    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << LogLevel::ToString(level);
        }
    };
    class ElapsedFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapsedFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << event->getElapse();
        }
    };
    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << logger->getName();
        }
    };
    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << event->getThreadId();
        }
    };
    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << event->getFiberId();
        }
    };
    //实际上就只有这个用到了format字段
    class TimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        TimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
            : m_format(format)
        {// 初始化时传入空字符串，则设置为默认格式
            if (m_format.empty()) 
                m_format = "%Y-%m-%d %H:%M:%S";
        }
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    private:
        std::string m_format;
    };
    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << event->getFile();
        }
    };
    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << event->getLine();
        }
    };
    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str)
            : m_string(str)
        {
        }
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };
    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << std::endl;
        }
    };
    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &fmt = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
        {
            os << "\t";
        }
    };
    // class ThreadNameFormatItem : public LogFormatter::FormatItem
    // {
    // public:
    //     ThreadNameFormatItem(const std::string &fmt = "") {}
    //     void format(std::ostream &os, std::shared_ptr<Logger> logger, LogEvent::ptr event, LogLevel::Level level) override
    //     {
    //         os << event->getThreadName();
    //     }
    // };
    Logger::Logger(const std::string &name, LogLevel::Level level)
        : m_name(name), m_level(level)
    {
        // 默认格式
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }
    void Logger::addAppender(LogAppender::ptr appender)
    {
        if (!appender->getFormatter())
            appender->setFormatter(m_formatter);
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender)
    {
        for (auto it = m_appenders.begin();
             it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }
    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            auto self = shared_from_this();
            for (auto &i : m_appenders)
            {
                i->log(level, event, self);
            }
        }
    }
    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }
    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }

    LogAppender::LogAppender(LogLevel::Level level)
        : m_level(level)
    {
    }
    
    FileLogAppender::FileLogAppender(const std::string &filename)
        : m_filename(filename)
    {
    }
    void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event, std::shared_ptr<Logger> logger)
    {
        if (level >= m_level)
            m_filestream << m_formatter->format(logger, level, event);
    }
    bool FileLogAppender::reopen()
    {
        if (m_filestream)
            m_filestream.close();
        m_filestream.open(m_filename);
        return !!m_filestream;
    }
    void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event, std::shared_ptr<Logger> logger)
    {
        if (level >= m_level)
            std::cout << m_formatter->format(logger, level, event);
    }
    LogFormatter::LogFormatter(const std::string &pattern)
        : m_pattern(pattern)
    {
        init();
    }
    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &item : m_items)
        {
            item->format(ss, logger, event, level);
        }
        return ss.str();
    }

    void LogFormatter::init()
    {
        // str, format, type
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string literal_str;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {   
            // xxx%... 将%前的普通文本加入literal_str
            if (m_pattern[i] != '%')
            {
                literal_str.append(1, m_pattern[i]);
                continue;
            }
            // 出现 % 处理
            // %% +%
            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    literal_str.append(1, '%');
                    continue;
                }
            }

            size_t j = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0; // 记录fmt字段的起始位置

            std::string field_str;
            std::string fmt;
            while (j < m_pattern.size())
            {
                if (!fmt_status && (!isalpha(m_pattern[j]) && m_pattern[j] != '{' && m_pattern[j] != '}'))
                {
                    field_str = m_pattern.substr(i + 1, j - i - 1);
                    j --;
                    break;
                }
                if (fmt_status == 0 && m_pattern[j] == '{')
                {
                    // 遇到 { 开始解析fmt字段
                    field_str = m_pattern.substr(i + 1, j - i - 1);
                    fmt_status = 1;
                    fmt_begin = j + 1;
                    continue;
                }
                if (fmt_status == 1) // 解析fmt字段
                {
                    if (m_pattern[j] == '}')
                    {
                        fmt_status = 2;
                        fmt = m_pattern.substr(fmt_begin, j - fmt_begin);
                        break;
                    }
                }
                j ++;
            } 
            // 处理最后一个
            if (j == m_pattern.size() && j - i - 1 > 0)
                field_str = m_pattern.substr(i + 1, j - i - 1);
            
            // 处理普通文本
            if (!literal_str.empty())
            {
                vec.push_back(std::make_tuple(literal_str, "", 0));
                literal_str.clear();
            }
            // 处理字段
            if (fmt_status == 1) // 格式错误
            {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
            else 
                vec.push_back(std::make_tuple(field_str, fmt, 1));

            i = j;
        }
        
        static std::map<std::string, std::function<FormatItem::ptr(const std::string &)>> s_format_items = {
#define XX(str, C) \
    {#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); }}

            XX(m, MessageFormatItem),
            XX(p, LevelFormatItem),
            XX(r, ElapsedFormatItem),
            XX(c, NameFormatItem),
            XX(t, ThreadIdFormatItem),
            XX(n, NewLineFormatItem),
            XX(d, TimeFormatItem),
            XX(f, FilenameFormatItem),
            XX(l, LineFormatItem),
            XX(T, TabFormatItem),               //T:Tab
            XX(F, FiberIdFormatItem),           //F:协程id
            // XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
        };
        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0) // 普通文本
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else // 字段
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end()) // fmt字段不存在
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                }
                else
                {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
            // std::cout << std::get<0>(i) << '-' << std::get<1>(i) << '-' << std::get<2>(i) << std::endl;
        }
    }
}
