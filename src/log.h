#ifndef __src_log_h__
#define __src_log_h__

#include "singleton.h"
#include "util.h"
#include <bits/types/time_t.h>
#include <cstdint>
#include <string>
#include <stdint.h>
#include <memory>
#include <stdarg.h>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>

#define CHPE_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        cpp_high_perf::LogEventWrap(cpp_high_perf::LogEvent::ptr(new cpp_high_perf::LogEvent(logger, level, __FILE__, __LINE__, 0, cpp_high_perf::GetThreadId(), cpp_high_perf::GetFiberId(), time(0)))).getSS()

#define CHPE_LOG_DEBUG(logger) CHPE_LOG_LEVEL(logger, cpp_high_perf::LogLevel::DEBUG)
#define CHPE_LOG_INFO(logger) CHPE_LOG_LEVEL(logger, cpp_high_perf::LogLevel::INFO)
#define CHPE_LOG_WARN(logger) CHPE_LOG_LEVEL(logger, cpp_high_perf::LogLevel::WARN)
#define CHPE_LOG_ERROR(logger) CHPE_LOG_LEVEL(logger, cpp_high_perf::LogLevel::ERROR)
#define CHPE_LOG_FATAL(logger) CHPE_LOG_LEVEL(logger, cpp_high_perf::LogLevel::FATAL)

#define CHPE_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        cpp_high_perf::LogEventWrap(cpp_high_perf::LogEvent::ptr(new cpp_high_perf::LogEvent(logger, level, __FILE__, __LINE__, 0, cpp_high_perf::GetThreadId(), cpp_high_perf::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define CHPE_LOG_FMT_DEBUG(logger, fmt, ...) CHPE_LOG_FMT_LEVEL(logger, cpp_high_perf::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define CHPE_LOG_FMT_INFO(logger, fmt, ...) CHPE_LOG_FMT_LEVEL(logger, cpp_high_perf::LogLevel::INFO, fmt, __VA_ARGS__)
#define CHPE_LOG_FMT_WARN(logger, fmt, ...) CHPE_LOG_FMT_LEVEL(logger, cpp_high_perf::LogLevel::WARN, fmt, __VA_ARGS__)
#define CHPE_LOG_FMT_ERROR(logger, fmt, ...) CHPE_LOG_FMT_LEVEL(logger, cpp_high_perf::LogLevel::ERROR, fmt, __VA_ARGS__)
#define CHPE_LOG_FMT_FATAL(logger, fmt, ...) CHPE_LOG_FMT_LEVEL(logger, cpp_high_perf::LogLevel::FATAL, fmt, __VA_ARGS__)

//读取loggermanager里面的默认logger
#define CHPE_LOG_ROOT() cpp_high_perf::LoggerMgr::GetInstance()->getRoot()

namespace cpp_high_perf {//命名空间设置为cpp_high_perf，避免命名冲突
//shared_ptr是智能指针，可以自动释放内存，不需要手动释放，就像悬崖上面的安全绳，一个对象有很多根，最后一根没了对象就没了
class Logger;

class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(LogLevel::Level level);

};

//日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;//main可以直接 LogEvent::ptr f直接创建一个智能指针
    //LogEvent();//默认构造函数
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t m_line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time);

    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_ss.str(); }//这个返回的是临时的对象
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }

    //为了 test.cc 里面可以 SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

    std::stringstream& getSS() { return m_ss; }

private:

    //为了满足m_event->getLogger()的写法，所以需要加上一个Logger的指针
    std::shared_ptr<Logger> m_logger;
    //level也是同理
    LogLevel::Level m_level;


    const char* m_file = nullptr;//文件名
    int32_t m_line = 0;//行号
    uint32_t m_elapse = 0;//程序启动开始到现在的毫秒数
    uint32_t m_threadId = 0;//线程ID
    uint32_t m_fiberId = 0;//协程ID
    uint64_t m_time = 0;//时间戳(毫秒级别)
    //std::string m_content;//日志内容,但是好像不太需要，换成string stream吧
    std::stringstream m_ss;//日志内容
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const {return m_event;}
    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;
};

//日志格式定义器,来决定最终打印日志的
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);//构造函数
    //%t    %thread_id %m%n
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);//成员函数
public:
    //解析的子模块
    class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            FormatItem(const std::string& fmt = "") {};
            virtual ~FormatItem() {} // 虚类
            //virtual std::string format(LogEvent::ptr event) = 0;
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    void init();//初始化,
private:
    std::vector<FormatItem::ptr> m_items;
    std::string m_pattern;//格式模板吧，就是id,time,level...输出顺序
};

//日志输出器(比如控制台输出，或者文件输出)，但是需要格式化输出 LogFormatter
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    LogAppender(LogLevel::Level level = LogLevel::DEBUG) : m_level(level) {}//构造函数

    virtual ~LogAppender() {}//由于这个类考虑到会被继承，所以析构函数设置为虚函数

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;//纯虚函数，子类必须实现
    //上面函数加入logger目的是为了打印日志的名称
    void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
protected://虚拟的基类要用到level
    LogLevel::Level m_level = LogLevel::DEBUG; //日志级别
    LogFormatter::ptr m_formatter;//日志格式器
};

//日志器
class Logger : public std::enable_shared_from_this<Logger>{
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name = "root");//默认构造函数

    void log(LogLevel::Level level, LogEvent::ptr event);//成员函数

    void debug(LogEvent::ptr event);//成员函数
    void info(LogEvent::ptr event);//成员函数
    void warn(LogEvent::ptr event);//成员函数
    void error(LogEvent::ptr event);//成员函数
    void fatal(LogEvent::ptr event);//成员函数

    void addAppender(LogAppender::ptr appender);//成员函数
    void delAppender(LogAppender::ptr appender);//成员函数

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

    const std::string& getName() const { return m_name; }
private:
    std::string m_name;//日志名称
    LogLevel::Level m_level;//日志级别
    std::list<LogAppender::ptr> m_appenders;//Appender集合
    LogFormatter::ptr m_formatter;
};

//继承LogAppender类，输出控制台
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;//告诉编译器我要重写这个函数
private:

};

//继承LogAppender类，输出到文件
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    FileLogAppender(const std::string& filename);

    bool reopen();//文件涉及重新打开
private:
    std::string m_filename;
    std::ofstream m_filestream;
};

//日志器管理类
class LoggerManager {
public:
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);

    void init();//可以跟配置文件结合起来，从配置读出来，很快产生一个LoggerManager
    Logger::ptr getRoot() const { return m_root; }
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;//主要的logger，就是有一个默认的logger

};

//日志器管理类单例模式
typedef cpp_high_perf::Singleton<LoggerManager> LoggerMgr;

}
#endif