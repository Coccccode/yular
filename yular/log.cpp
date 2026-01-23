#include "log.h"


    LogEvent::LogEvent(std::string logName, LogLevel::Level level,
             const char* file, 
             int32_t line, 
             uint32_t elapse,
             uint32_t thread_id,
             uint32_t fiber_id,
             uint64_t time)
        :m_logName(logName)
        ,m_level(level)
        ,m_file(file)
        ,m_line(line)
        ,m_elapse(elapse)
        ,m_thread_id(thread_id)
        ,m_fiber_id(fiber_id)
        ,m_time(time)
    {

    }
    LogEvent::LogEvent()
    {

    }
    Logger::Logger(const std::string& name)
        : m_name(name)
        , m_level(LogLevel::DEBUG) {
            m_formatter.rest
    }
    void Logger::log(LogEvent::ptr event) {
        if(event->getLevel() >= m_level) {
            for(auto& appender : m_appenders) {
                appender->log(event);
            }
        }
    }
    void Logger::addAppender(LogAppender::ptr appender) {
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender) {
        for(auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
            if (*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    } 


    void FileLogAppender::log(LogEvent::ptr event)
    {
        if(event->getLevel() >= m_level)
        {
            m_filestream << m_formatter->format(event);
        }
    }
    bool FileLogAppender::reopen()
    {
        if(m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }

    void StdoutLogAppender::log(LogEvent::ptr event)
    {
        if(event->getLevel() >= m_level)
        {
            std::cout << m_formatter->format(event);
        }
    }

    LogFormatter::LogFormatter(const std::string pattern):m_pattern(pattern)
    {
        init();
    }


    void LogFormatter::init()
    {
        size_t last_pos = 0;
        std::stringstream ss;
        std::vector<std::tuple<std::string,std::string,int>> vec;
        std::string nstr;
        for(size_t i = 0;i < m_pattern.size();i++) {
            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin;
            std::string str;
            std::string fmt;
            if(m_pattern[i] != '%')  {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            if(i + 1 < m_pattern.size()) {
                if(m_pattern[i + 1] == '%') {
                    nstr.append(1,'%');
                    continue;
                }
            }
            while(n < m_pattern.size()) {
                if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if(fmt_status == 0) {
                    if(m_pattern[n] == '{') {
                        str = m_pattern.substr(i+1,n-i-1);
                        fmt_status = 1;
                        fmt_begin = n;
                        n++;
                        continue;
                    }
                }
                else if(fmt_status == 1) {
                    if(m_pattern[n] == '}') {
                        fmt = m_pattern.substr(fmt_begin + 1,n - fmt_begin - 1);
                        fmt_status = 0;
                        n++;
                        break;
                    }
                }
                n++;
                if(n == m_pattern.size()) {
                    if(str.empty()) {
                        str = m_pattern.substr(i + 1);
                    }
                }

            }
            if(fmt_status == 0) {
                if(!nstr.empty()) {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if(fmt_status == 1) {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }

        }
        if(!nstr.empty()) {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }
        
	
	//输出看下
        // for(auto& it : vec) {
        //     std::cout 
        //         << std::get<0>(it) 
        //         << " : " << std::get<1>(it) 
        //         << " : " << std::get<2>(it)
        //         << std::endl;
        // }
        static std::map<std::string,std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str,C) \
            {#str,[](const std::string& fmt){ return FormatItem::ptr(new C(fmt)); }}
        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem),
#undef XX
        };
        for(auto& i : vec) {
            if(std::get<2>(i) == 0) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else {
                auto it = s_format_items.find(std::get<0>(i));
                if(it == s_format_items.end()) {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                }
                else {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
        }
    }
    std::string LogFormatter::format(LogEvent::ptr event) 
    {
        std::stringstream ss;
        for(auto &item : m_items) {
            item->format(ss,event);
        }
        return ss.str();
    }
    LogEventWrap::LogEventWrap(Logger::ptr logger, LogEvent::ptr e)
        : m_logger(logger), m_event(e) {
    }

    LogEventWrap::~LogEventWrap() { 
        m_logger->log(m_event); 
    }

    std::stringstream &LogEventWrap::getSS() { return m_event->getSS(); }

int main(int argc,char** argv) 
{
  std::cout << "======START======" << std::endl;
  Logger::ptr lg(new Logger("XYZ"));
  LogFormatter::ptr formatter(new LogFormatter(
      "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
  //添加控制台输出适配器
  StdoutLogAppender::ptr stdApd(new StdoutLogAppender());
  stdApd->setFormatter(formatter);
  lg->addAppender(stdApd);
  LOG_LEVEL(lg,LogLevel::INFO) << "Hello XYZ !";
  std::cout << "=======END=======" << std::endl;
    
    return 0;

}

