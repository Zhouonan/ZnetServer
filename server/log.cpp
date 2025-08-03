#include "log.h"
#include <map>
#include <functional>
#include <sstream>
#include "config.h"
#include "util.h"

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
    
    // 小写
    LogLevel::Level LogLevel::FromString(const std::string& str) {
        if (str == "debug") return DEBUG;
        if (str == "info") return INFO;
        if (str == "warn") return WARN;
        if (str == "error") return ERROR;
        if (str == "fatal") return FATAL;
        return UNKNOW;
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
    Logger::Logger(const std::string &name, LogLevel::Level level, std::string pattern)
        : m_name(name), m_level(level)
    {
        // 默认格式
        m_formatter.reset(new LogFormatter(pattern));
    }
    Logger::Logger(const std::string &name, LogLevel::Level level, const std::string &pattern, const std::vector<std::string> &appenders, std::string outputPath)
        : m_name(name), m_level(level)
    {
        m_formatter.reset(new LogFormatter(pattern));
        for (auto &appender : appenders) {
            if (appender == "stdout") {
                addAppender(LogAppender::ptr(new StdoutLogAppender()));
            } else if (appender == "file") {
                addAppender(LogAppender::ptr(new FileLogAppender(outputPath)));
            }
        }
    }
    
    Logger::~Logger() {
        // 清理appenders列表，避免循环引用
        m_appenders.clear();
    }
    
    void Logger::addAppender(LogAppender::ptr appender)
    {
        if (!appender->getFormatter()) {
            appender->setFormatter(m_formatter);
        }
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
    
    FileLogAppender::FileLogAppender(const std::string &filepath)
        : m_filepath(filepath)
    {
    }
    
    FileLogAppender::~FileLogAppender() {
        if (m_filestream.is_open()) {
            m_filestream.close();
        }
    }
    
    void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event, std::shared_ptr<Logger> logger)
    {
        if (level >= m_level) {
            try {
                // 检查文件流状态
                if (!m_filestream.is_open()) {
                    reopen();
                }
                
                if (m_filestream.is_open()) {
                    m_filestream << m_formatter->format(logger, level, event);
                    m_filestream.flush(); // 确保数据写入磁盘
                } else {
                    // 如果文件打开失败，输出到错误日志
                    ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "Failed to open log file: " << m_filepath;
                }
            } catch (const std::exception& e) {
                ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "Error writing to log file: " << e.what();
            }
        }
    }
    bool FileLogAppender::reopen()
    {
        if (m_filestream)
            m_filestream.close();
        m_filestream.open(m_filepath);
        return !!m_filestream;
    }
    void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event, std::shared_ptr<Logger> logger)
    {
        if (level >= m_level) {
            std::cout << m_formatter->format(logger, level, event);
            // std::cout << "StdoutLogAppender::formatter: " << m_formatter->getPattern() << std::endl;
        }
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

    void LogFormatter::setPattern(const std::string &pattern)
    {
        m_pattern = pattern;
        init();
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

    LoggerManager::LoggerManager() {
        m_root.reset(new Logger("root"));
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender()));
        m_loggers[m_root->getName()] = m_root;
    }
    
    LoggerManager::~LoggerManager() {
        // 清空map，让shared_ptr自动管理内存
        m_loggers.clear();
        
        // 最后清理root logger
        if (m_root) {
            // ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "LoggerManager::~LoggerManager: cleaning up root logger";
            m_root.reset();
        }
    }

    Logger::ptr LoggerManager::getLogger(const std::string& name) {
        auto it = m_loggers.find(name);
        if(it != m_loggers.end()) {
            return it->second;
        }
        // 没有就创一个
        Logger::ptr logger(new Logger(name));
        // 设置一个兜底
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }

    bool LoggerManager::updateLogger(const std::string& name, LogLevel::Level level, const std::string &pattern, const std::vector<std::string> &appenders, std::string outputPath) {
        bool isChanged = false;
        auto it = m_loggers.find(name);
        if(it != m_loggers.end()) {
            Logger::ptr l = ZNS_LOG_NAME(it->second->getName());
            if(level != l->getLevel()) {
                l->setLevel(level);
                isChanged = true;
            }
            if(pattern != l->getFormatter()->getPattern()) {
                l->setFormatter(pattern);
                // 更新所有appender的formatter（如果appender没有自定义formatter）
                for(auto& ap : l->getAppenders()) {
                    // 这里可以添加逻辑来判断appender是否有自定义formatter
                    // 暂时简单更新所有appender
                    ap->setHasCustomFormatter(false);
                    ap->setFormatter(LogFormatter::ptr(new LogFormatter(pattern)));
                }
                isChanged = true;
            }
            // 更新appender
            for(auto& appender : appenders) {
                if(appender == "stdout") {
                    bool hasStdout = false;
                    for(auto apd : l->getAppenders()) {
                        if(apd->getAppenderType() == "stdout") hasStdout = true;
                    }
                    if(!hasStdout) {
                        l->addAppender(LogAppender::ptr(new StdoutLogAppender()));
                        isChanged = true;
                    }
                } else if(appender == "file") {
                    bool hasFile = false;
                    for(auto apd : l->getAppenders()) {
                        auto fapd = std::dynamic_pointer_cast<FileLogAppender>(apd);
                        if(fapd) {
                            hasFile = true;
                            if(fapd->getFilepath() != outputPath) {
                                fapd->setFilepath(outputPath);
                                isChanged = true;
                            }
                        }
                    }
                    if(!hasFile) {
                        l->addAppender(LogAppender::ptr(new FileLogAppender(outputPath)));
                        isChanged = true;
                    }
                }
            }            
        }
        return isChanged;
    }

    Logger::ptr LoggerManager::createLogger(const std::string& name, LogLevel::Level level, const std::string &pattern, const std::vector<std::string> &appenders, std::string outputPath) {
        auto it = m_loggers.find(name);
        if(it != m_loggers.end()) {
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "LoggerManager::createLogger: logger " << name << " already exists";
            if (updateLogger(name, level, pattern, appenders, outputPath)) {
                ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "LoggerManager::createLogger: logger " << name << " has been updated";
            }
            return it->second;
        }
        
        // 创建新的logger
        Logger::ptr logger(new Logger(name, level, pattern, appenders, outputPath));
        
        // 设置appender的level和formatter（继承logger的设置）
        for(auto& appender : logger->getAppenders()) {
            appender->setLevel(level);  // 继承logger的level
            appender->setFormatter(LogFormatter::ptr(new LogFormatter(pattern)));  // 继承logger的formatter
        }
        
        m_loggers[name] = logger;
        return logger;
    }

    // configureLogger的辅助函数：更新appender
    void updateAppender(Logger::ptr logger, LogDefine ld) {
        // 清空旧的appender
        logger->clearAppenders();
        // 注意这里不能写：logger->getAppenders().clear(); 因为getAppenders()返回的是一个拷贝的副本
        // std::cout << "logger->getAppenders().size(): " << logger->getAppenders().size() << std::endl;
        // 根据新的定义重新创建和添加 appender
        for (const auto& appender_def : ld.appender) {
            LogAppender::ptr new_appender;
            if (appender_def.type == "file") {
                new_appender.reset(new FileLogAppender(appender_def.path));
            } else if (appender_def.type == "stdout") {
                new_appender.reset(new StdoutLogAppender());
            }

            if (new_appender) {
                // 如果 appender 有自定义的 level，则设置
                if (appender_def.hasCustomLevel()) {
                    new_appender->setLevel(appender_def.level);
                } else {
                    new_appender->setLevel(ld.level); // 否则继承 logger 的 level
                }

                // 如果 appender 有自定义的 formatter，则设置
                if (appender_def.hasCustomFormatter()) {
                    new_appender->setFormatter(LogFormatter::ptr(new LogFormatter(appender_def.formatter)));
                    new_appender->setHasCustomFormatter(true);
                } else {
                    // 否则继承 logger 的 formatter
                    new_appender->setFormatter(logger->getFormatter());
                    new_appender->setHasCustomFormatter(false);
                }
                logger->addAppender(new_appender);
            }
        }
    }

    void LoggerManager::configureLogger(LogDefine ld) {
        auto it = m_loggers.find(ld.name);
        if(it != m_loggers.end()){
            Logger::ptr logger = it->second;
            logger->setLevel(ld.level);
            logger->setFormatter(ld.formatter);
            updateAppender(logger, ld);
            return;
        } 

        Logger::ptr logger(new Logger(ld.name, ld.level, ld.formatter));
        m_loggers[ld.name] = logger;
        updateAppender(logger, ld);
        
    }

    void LoggerManager::removeLogger(const std::string &name) {
        auto it = m_loggers.find(name);
        if(it != m_loggers.end()) {
            // 记录删除操作
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "LoggerManager::removeLogger: removing logger " << name;
            
            // 获取logger的引用计数信息（用于调试）
            auto logger = it->second;
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "LoggerManager::removeLogger: logger " << name << " use_count: " << logger.use_count();
            
            // 从map中移除
            m_loggers.erase(it);
            
            // 注意：如果其他地方还有对该logger的引用，它不会立即被销毁
            // 这是正常的shared_ptr行为，但需要确保没有循环引用
        } else {
            ZNS_LOG_WARN(ZNS_LOG_ROOT()) << "LoggerManager::removeLogger: logger " << name << " not found";
        }
    }
    
    // string -> LogDefine
    template<>
    class LexicalCast<std::string, LogDefine> {
    public:
        LogDefine operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            LogDefine ld;
            ld.name = node["name"].as<std::string>();
            ld.level = LogLevel::FromString(to_lower(node["level"].as<std::string>()));
            ld.formatter = node["formatter"].as<std::string>();
            if (node["appender"].IsDefined()) {
                for (size_t i = 0; i < node["appender"].size(); ++i) {
                    LogAppenderDefine lad;
                    lad.type = node["appender"][i]["type"].as<std::string>();
                    if (lad.type == "file") {
                        lad.path = node["appender"][i]["file"].as<std::string>();
                    } else if (lad.type == "stdout") {
                        lad.path = "";
                    }
                    if (node["appender"][i]["level"].IsDefined()) {
                        lad.level = LogLevel::FromString(to_lower(node["appender"][i]["level"].as<std::string>()));
                    }
                    if (node["appender"][i]["formatter"].IsDefined()) {
                        lad.formatter = node["appender"][i]["formatter"].as<std::string>();
                    }
                    ld.appender.push_back(lad);
                }
            }
            return ld;
        }
    };

    template<>
    class LexicalCast<LogDefine, std::string> {
    public:
        std::string operator()(const LogDefine &v) {
            YAML::Node node;
            node["name"] = v.name;
            node["level"] = to_lower(LogLevel::ToString(v.level));
            node["formatter"] = v.formatter;
            for (auto &i : v.appender) {
                YAML::Node appender_node;
                appender_node["type"] = i.type;
                if (i.type == "file") {
                    appender_node["file"] = i.path;
                }
                // 倘若level和fomatter是继承logger而非指定的则不输出
                if (i.hasCustomLevel()) {
                    appender_node["level"] = to_lower(LogLevel::ToString(i.level));
                }
                if (i.hasCustomFormatter()) {
                    appender_node["formatter"] = i.formatter;
                }
                node["appender"].push_back(appender_node);
            }
            return YAML::Dump(node);
        }
    };

    ZnetServer::ConfigVar<std::set<LogDefine>>::ptr g_loggers_config = ZnetServer::Config::Create<std::set<LogDefine>>("loggers", std::set<LogDefine>{}, "loggers");

    std::string LoggerManager::toYamlString()
    {
        YAML::Node node;
        for (auto &i : m_loggers) {
            std::string logger_name = i.first;
            Logger::ptr logger = i.second;
            struct LogDefine ld;
            ld.name = logger_name;
            ld.level = logger->getLevel();
            ld.formatter = logger->getFormatter()->getPattern();

            for (auto &i : logger->getAppenders()) {
                LogAppenderDefine lad;
                lad.type = i->getAppenderType();
                if (i->getAppenderType() == "file") {
                    lad.path = std::dynamic_pointer_cast<FileLogAppender>(i)->getFilepath();
                }
                if (i->getHasCustomFormatter()) {
                    lad.formatter = i->getFormatter()->getPattern();
                } 
                lad.level = i->getLevel();
                ld.appender.push_back(lad);
            }
            node["loggers"].push_back(YAML::Load(LexicalCast<LogDefine, std::string>()(ld)));
        }
        // // test LexicalCast LogDefine->string 
        // std::stringstream ss;
        // for(auto& i : g_loggers_config->getValue()) {
        //     ss << YAML::Load(LexicalCast<LogDefine, std::string>()(i)) << std::endl;
        // }
        // ZNS_LOG_DEBUG(ZNS_LOG_ROOT()) << "lexical cast test: \n" << ss.str();
        return YAML::Dump(node);
    }

    //添加对应的listener
    struct LogInit {
        LogInit() {
            g_loggers_config->addListener(1, [](const std::set<LogDefine>& new_value, const std::set<LogDefine>& old_value) {
                // 使用unordered_map预处理new_value，实现O(1)查找
                std::unordered_map<std::string, LogDefine> new_value_map;
                for (const auto& ld : new_value) {
                    new_value_map[ld.name] = ld;
                }
                
                // 检查删除的logger - 使用unordered_map查找，O(1)复杂度
                for (auto &i : LoggerMgr::GetInstance()->getLoggers()) {
                    if (new_value_map.find(i.first) == new_value_map.end()) {
                        ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "LogInit::LogInit: remove logger " << i.first;
                        LoggerMgr::GetInstance()->removeLogger(i.first);
                    }
                }
                
                // 检查新增和更新的logger
                for (const auto& ld : new_value) {
                    LoggerMgr::GetInstance()->configureLogger(ld);
                }
            });
        }
    };

    
    static LogInit s_log_init;
}
