#ifndef __YULAR_LOG_H__
#define __YULAR_LOG_H__

#include <iostream>
#include <string>   //日志名称用到了字符串
#include <memory>   //智能指针所需的头文件
#include <stdint.h>
#include <list> 
#include <vector>
#include <time.h>
#include <ctime>
#include <tuple>
#include <map>
#include <sstream>
#include <fstream>
//日志级别
class LogLevel
{
public:
    enum Level{
        UNKNOW = 0, //起手先来个未知级别兜底
        DEBUG = 1,  //调试级别
        INFO = 2,   //普通信息级别
        WARN = 3,   //警告信息
        ERROR = 4,  //错误信息
        FATAL = 5   //灾难级信息
    };

	static const char* ToString(LogLevel::Level level);
};
//日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent();
    LogEvent(LogLevel::Level level,
             const char* file, 
             int32_t line, 
             uint32_t elapse,
             uint32_t thread_id,
             uint32_t fiber_id,
             uint64_t time);
private:
    const char* m_file = nullptr; //文件名
    LogLevel::Level m_level; //日志级别
    int32_t m_line = 0; //行号
    uint32_t m_elapse = 0; //程序启动到现在的毫秒数
    uint32_t m_thread_id = 0; //线程id
    uint32_t m_fiber_id = 0; //协程id
    uint64_t m_time; //时间戳 
};
//日志格式化器
class LogFormatter {
public:
    //%t  %thread_id 
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string pattern);
    std::string format(LogEvent::ptr event);
    void init();
private:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {};
        virtual void format(std::ostream& os,LogEvent::ptr event) = 0;

    };
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
};

inline const char* LogLevel::ToString(LogLevel::Level level)
{
    switch(level)
    {
#define XX(name) \
        case LogLevel::name: \
            return #name;
            break;
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
    }
    return "UNKNOW";
}
//日志输出器
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender() {}
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
protected:
    LogLevel::Level m_level; //日志级别
    LogFormatter::ptr m_formatter;

};
//日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
    
    Logger(const std::string& name  = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void fatal(LogEvent::ptr event); 
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);  
    LogLevel::Level getLevel() const { return m_level; };
    void setLevel(LogLevel::Level val) { m_level = val; };
private:
    std::string m_name; //日志器名称
    LogLevel::Level m_level; //日志级别
    std::list<LogAppender::ptr> m_appenders; //appender集合 
};

//输出到控制台的appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(LogLevel::Level level, LogEvent::ptr event) override;
private:
};
//输出到文件的appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    void log(LogLevel::Level level,LogEvent::ptr event) override;
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
};
#endif