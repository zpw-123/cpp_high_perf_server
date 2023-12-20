#include <iostream>
#include "../src/log.h"
#include "../src/util.h"

int main()
{
    cpp_high_perf::Logger::ptr logger(new cpp_high_perf::Logger);
    logger->addAppender(cpp_high_perf::LogAppender::ptr(new cpp_high_perf::StdoutLogAppender));

    cpp_high_perf::FileLogAppender::ptr file_appender(new cpp_high_perf::FileLogAppender("/home/zpw/cpp_high_perf_server/log.txt"));

    cpp_high_perf::LogFormatter::ptr fmt(new cpp_high_perf::LogFormatter("%d%T%m%n"));//只是在初始化智能指针罢了
    file_appender->setFormatter(fmt);
    file_appender->setLevel(cpp_high_perf::LogLevel::ERROR);
    logger->addAppender(file_appender);
    
    //cpp_high_perf::LogEvent::ptr event(new cpp_high_perf::LogEvent(__FILE__, __LINE__, 0, cpp_high_perf::GetThreadId(), cpp_high_perf::GetFiberId(), time(0)));
    //cpp_high_perf::LogEvent::ptr event(new cpp_high_perf::LogEvent(logger, cpp_high_perf::LogLevel::DEBUG , __FILE__, __LINE__, 0, cpp_high_perf::GetThreadId(), cpp_high_perf::GetFiberId(), time(0)));
    
    //logger->log(cpp_high_perf::LogLevel::DEBUG, event);
    std::cout << "Hello world" << std::endl;

    CHPE_LOG_INFO(logger) << "test macro";
    CHPE_LOG_ERROR(logger) << "test macro error";
    CHPE_LOG_FMT_DEBUG(logger, "test macro fmt error %s", "aa_zpw");

    auto it = cpp_high_perf::LoggerMgr::GetInstance()->getLogger("xx");
    CHPE_LOG_INFO(it) << "xxx";
    return 0;
}