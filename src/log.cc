#include "log.h"
#include <cstddef>
#include <memory>
#include <string>
#include <iostream>
#include <functional>
#include <thread>
#include <vector>
#include <map>

namespace cpp_high_perf {

const char* LogLevel::ToString(LogLevel::Level level) {
    switch(level) {
        //下面可以理解成为定义的一个宏函数吧,下面的name就是传递的参数
        #define XX(name) \
            case LogLevel::name: \
                return #name; \
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

LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e) {

}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t m_line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time)
            :m_logger(logger), m_level(level), m_file(file), m_line(m_line), m_elapse(elapse), m_threadId(thread_id), 
            m_fiberId(fiber_id), m_time(time) {}

void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);//去调用下面的 void LogEvent::format(const char* fmt, va_list al) 函数
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

//在头文件指定默认值，在cpp文件就不应该再指定了
Logger::Logger(const std::string& name):m_name(name), m_level(LogLevel::DEBUG) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);//来保证每个都有格式器
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    //输出大于等于日志级别的日志
    if (level >= m_level) {
        auto self = shared_from_this();//获取指向当前对象的shared_ptr
        for (auto& i : m_appenders) {
            i->log(self, level, event);//这个是appenders的输出函数
        }
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

FileLogAppender::FileLogAppender(const std::string& filename):m_filename(filename) {
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        m_filestream << m_formatter->format(logger, level, event);
    }
}

bool FileLogAppender::reopen() {
    if (m_filestream) {//是来写入文件的
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;//相当于is_open(), !!转换数据类型
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::string str = m_formatter->format(logger, level, event);
        std::cout << str << std::endl;
    }
}

LogFormatter::LogFormatter(const std::string& pattern):m_pattern(pattern) {
    //初始化一下打印的格式吧,也就是标准的日志格式
    init();//来进行解析日志格式的
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

class MessageFormatItem : public LogFormatter::FormatItem {
    public:
    MessageFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
            os << event->getContent();
        }
};

class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
            os << LogLevel::ToString(level);
        }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
            os << event->getElapse();
        }
};

class NameFormatItem : public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
            os << logger->getName();
        }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
            os << event->getThreadId();
        }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
    public:
        FiberIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
            os << event->getFiberId();
        }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
    public:
        DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S"):m_format(format) {
            if (m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
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

class FilenameFormatItem : public LogFormatter::FormatItem {
    public:
        FilenameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event) override {
            os << event->getFile();
        }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
private:
    std::string m_string;
};

//主要是来区分用户给定的pattern这一个日志格式！是否是合法的！
//比如 %xxx %xxx{xxx} %% 转义
void LogFormatter::init() {
    //解析pattern这一个日志格式的// m_pattern "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
    //str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;//来表示遍历到了哪个位置了吧
    for(size_t i = 0; i < m_pattern.size(); i++) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);//这个1表示插入的数量
            continue;
        }

        //也就是说现在就是遇到了%
        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') { //解析 “%%”
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;//状态1解析时间{%Y-%m-%d %H:%M:%S}；状态2解析之后的
        size_t fmt_begin = 0;//格式的开始位置

        std::string str;//d T t N等格式
        std::string fmt;//保存时间格式 %Y-%m-%d %H:%M:%S	

        while(n < m_pattern.size()){
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                //遇到了非字母的字符，比如遇到下一个%，就可以直接截断子串了
                str = m_pattern.substr(i + 1, n - i - 1);//直接就是读取了T/t/N，就是非d的
                break;
            }
            if(fmt_status == 0){	//开始解析时间格式
				if(m_pattern[n] == '{'){
					str = m_pattern.substr(i + 1, n - i - 1);	//str = "d"
					fmt_status = 1;	
					fmt_begin = n;
					++n;
					continue;
				}
            } else if (fmt_status == 1) {	//解析时间格式
                if(m_pattern[n] == '}') {
                    // fmt = %Y-%m-%d %H:%M:%S
					fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
					fmt_status = 0;
					++n;
					break;		//解析时间结束break
				}
            }

            ++n;
            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);//最后一个子字符一定要截取出来
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));//(e.g.) ("d", %Y-%m-%d %H:%M:%S, 1) type为1
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));//(e.g.) 最后一个字符为[ ] :(没搞懂nstr到底是啥东西呢)
    }

    //创建一个静态的映射表 s_format_items，将字符串映射到创建 FormatItem 对象的函数上（返回的是智能指针）
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
//传递进来了两个参数，str和C，其中的C是class参数
#define XX(str, C) {#str, [](const std::string& fmt) {return FormatItem::ptr(new C(fmt));}}//就是初始化一个map不是
        //{"m", [](const std::string& fmt) {return FormatItem::ptr(new MessageFormatItem(fmt));}},
        
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
        if(std::get<2>(i) == 0) {//这是得到type吧
            //将解析出的FormatItem放到m_items中 [ ] :
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));//这个地方是来查找的
            if (it == s_format_items.end()) {
                //报错
                std::cout << "!!!error_format!!!" << std::endl;
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));//这个地方是来创建的
            }
        }
    }
};

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
};

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;
};



} 