#include "log.h"

    LogEvent::LogEvent(LogLevel::Level level,
             const char* file, 
             int32_t line, 
             uint32_t elapse,
             uint32_t thread_id,
             uint32_t fiber_id,
             uint64_t time)
        :m_level(level)
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
        : m_name(name) {

    }
    void Logger::log(LogLevel::Level level,LogEvent::ptr event) {
        if(level >= m_level) {
            for(auto& appender : m_appenders) {
                appender->log(level, event);
            }
        }
    }
    void Logger::debug(LogEvent::ptr event) {
        log(LogLevel::Level::DEBUG,event);
    }
    void Logger::info(LogEvent::ptr event) {
        log(LogLevel::Level::INFO,event);
    }
    void Logger::warn(LogEvent::ptr event) {
        log(LogLevel::Level::WARN,event);
    }
    void Logger::fatal(LogEvent::ptr event) {
        log(LogLevel::Level::FATAL,event);
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


    void FileLogAppender::log(LogLevel::Level level,LogEvent::ptr event)
    {
        if(level >= m_level)
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

    void StdoutLogAppender::log(LogLevel::Level level,LogEvent::ptr event)
    {
        if(level >= m_level)
        {
            std::cout << m_formatter->format(event);
        }
    }

    LogFormatter::LogFormatter(const std::string pattern):m_pattern(pattern)
    {


    }


    void LogFormatter::init()
    {
        size_t last_pos = 0;
        std::stringstream ss;
        std::vector<std::tuple<std::string,std::string,int>> vec;
        std::string nstr;
        for(size_t i = 0;i < m_pattern.size();i++)
        {
            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin;
            std::string str;
            std::string fmt;
            if(m_pattern[i] != '%') 
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            if(i + 1 < m_pattern.size())
            {
                if(m_pattern[i + 1] == '%')
                {
                    nstr.append(1,'%');
                    continue;
                }
            }
            while(n < m_pattern.size())
            {
                if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) 
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if(fmt_status == 0)
                {
                    if(m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i+1,n-i-1);
                        fmt_status = 1;
                        fmt_begin = n;
                        n++;
                        continue;
                    }
                }
                else if(fmt_status == 1)
                {
                    if(m_pattern[n] == '}')
                    {
                        fmt = m_pattern.substr(fmt_begin + 1,n - fmt_begin);
                        fmt_status = 0;
                        n++;
                        break;
                    }
                }
                n++;
                if(n == m_pattern.size()) 
                {
                    if(str.empty()) 
                    {
                        str = m_pattern.substr(i + 1);
                    }
                }

            }
            if(fmt_status == 0)
            {
                if(!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if(fmt_status == 1)
            {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }

        }
        if(!nstr.empty()) 
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }
	
	//输出看下
        for(auto& it : vec) 
        {
            std::cout 
                << std::get<0>(it) 
                << " : " << std::get<1>(it) 
                << " : " << std::get<2>(it)
                << std::endl;
        }
    }
    std::string LogFormatter::format(LogEvent::ptr event)
    {
        std::stringstream ss;
        for(auto &item : m_items)
        {
            item->format(ss,event);
        }
        return ss.str();
    }

int main(int argc,char** argv)
{
    LogFormatter::ptr formatter(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    formatter->init();  
    return 0;
}

