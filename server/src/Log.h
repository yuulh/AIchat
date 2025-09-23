#ifndef LOG_H
#define LOG_H

// 依赖于main中提前读取配置文件
/*
#include <log4cpp/PropertyConfigurator.hh>
int main()
{
     log4cpp::PropertyConfigurator::configure("logConfigPath");
     // 其中logConfigPath可以是：CONFIG["logConfigPath"]
}
*/

#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>

#define LOG_DEBUG(msg) (log4cpp::Category::getRoot() << log4cpp::Priority::DEBUG << __FILE__ << ":" \
     << __LINE__ << " " << msg)
#define LOG_DEBUG_BUF LOG_DEBUG(logBuf)

#define LOG_WARN(msg) (log4cpp::Category::getRoot() << log4cpp::Priority::WARN << __FILE__ << ":" \
     << __LINE__ << " " << msg)

#define LOG_ERROR(msg) (log4cpp::Category::getRoot() << log4cpp::Priority::ERROR << __FILE__ << ":" \
     << __LINE__ << " " << msg)

#define LOG_INFO(msg) (log4cpp::Category::getRoot() << log4cpp::Priority::INFO<< __FILE__ << ":" \
     << __LINE__ << " " << msg)


extern char logBuf[1024];  // 用于格式化日志，实体定义在Configuration.cc中

#endif