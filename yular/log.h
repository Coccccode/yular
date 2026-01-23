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
#include <functional>
#include <unistd.h>
#include <thread>
//日志级别、
#define LOG_LEVEL(logger, level)                                               \
  		if (logger->getLevel() <= level)                                       \
 			 LogEventWrap(logger, LogEvent::ptr(new LogEvent(                  \
                           logger->getName(), level, __FILE__, __LINE__, 0,    \
                           std::hash<std::thread::id>()(std::this_thread::get_id()), 1, time(0))))                  \
      					   .getSS()
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
    LogEvent(std::string logName, LogLevel::Level level,
             const char* file, 
             int32_t line, 
             uint32_t elapse,
             uint32_t thread_id,
             uint32_t fiber_id,
             uint64_t time);

    const std::string& getLogName() const { return m_logName;}
    const char* getFile() const { return m_file;}
    int32_t getLine() const { return m_line;}
    uint32_t getElapse() const { return m_elapse;}
    uint32_t getThreadId() const { return m_thread_id;}
    uint32_t getFiberId() const { return m_fiber_id;}
    uint64_t getTime() const { return m_time;}
    LogLevel::Level getLevel() const { return m_level;}

    std::string getContent() const { return m_ss.str(); } //【此处增加流对象转字符串！！！】
    std::stringstream& getSS() { return m_ss;}	//【此处增加流对象get方法提供流式调用！！！】
private:
    std::string m_logName; //日志名称
    const char* m_file = nullptr; //文件名
    LogLevel::Level m_level; //日志级别
    int32_t m_line = 0; //行号
    uint32_t m_elapse = 0; //程序启动到现在的毫秒数
    uint32_t m_thread_id = 0; //线程id
    uint32_t m_fiber_id = 0; //协程id
    uint64_t m_time; //时间戳 
    std::stringstream m_ss;       //字符流【此处增加流对象！！！】
};
//日志格式化器
class LogFormatter {
public:
    //%t  %thread_id 
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string pattern);
    std::string format(LogEvent::ptr event);
    void init();
    
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        FormatItem(const std::string str = "") {};
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
    virtual void log(LogEvent::ptr event) = 0;
    
    void setFormatter(LogFormatter::ptr val) { m_formatter = val;}
    LogFormatter::ptr getFormatter() const { return m_formatter;}
protected:
    LogLevel::Level m_level; //日志级别
    LogFormatter::ptr m_formatter;

};
//日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
    
    Logger(const std::string& name  = "root");
    void log(LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void fatal(LogEvent::ptr event); 
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);  
    LogLevel::Level getLevel() const { return m_level; };
    void setLevel(LogLevel::Level val) { m_level = val; };

    const std::string& getName() const { return m_name;}
private:
    std::string m_name; //日志器名称
    LogLevel::Level m_level; //日志级别
    std::list<LogAppender::ptr> m_appenders; //appender集合 
};

//输出到控制台的appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    StdoutLogAppender() {}
    void log(LogEvent::ptr event) override;
private:
};
//输出到文件的appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    void log(LogEvent::ptr event) override;
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
};

class MessageFormatItem:public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string str = "") {};
    void format(std::ostream& os,LogEvent::ptr event) {
        os << event->getContent();
    }
};

class LevelFormatItem:public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string str = "") {};
    void format(std::ostream& os,LogEvent::ptr event) {
        os << LogLevel::ToString(event->getLevel());
    }
};

class ElapseFormatItem:public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string str = "") {};
    void format(std::ostream& os,LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem:public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string str = "") {};
    void format(std::ostream& os,LogEvent::ptr event) override {
        os << event->getLogName();
    }
};


class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
            if(m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }
    void format(std::ostream& os,LogEvent::ptr event) override{
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time,&tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {};
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << "\t";
    }
private:
    std::string m_string;
};

class LogEventWrap
{
public:
    LogEventWrap(Logger::ptr logger,LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const { return m_event; }
    std::stringstream &getSS();

private:
  Logger::ptr m_logger;
  LogEvent::ptr m_event;
};

#endif