#ifndef Configuration_H
#define Configuration_H

#include <string>
#include <unordered_map>

#define CONFIG Configuration::getInstance()->getConfigs()

using std::unordered_map;
using std::string;

using configArgs_t = unordered_map<string, string>;

class Configuration{
    string path;  // 文件路径
    configArgs_t configs;  // 配置项
    static Configuration *p;  // 单例对象指针
    static pthread_once_t onceInstance;  // 线程安全

    void init();  // 从path初始化配置项，首次获取单例对象时完成
    std::pair<string, string> getPair(string &s, const char *delim);
    static void initInstance();
    static void destroy();
    
public:

    static Configuration * const getInstance();
    configArgs_t &getConfigs();

    Configuration (const Configuration &__rhs) = delete;
    Configuration(Configuration &&__rhs) = delete;

    Configuration &operator=(const Configuration &__rhs) = delete;
    Configuration &operator=(Configuration &&__rhs) = delete;
private:
    Configuration(){};
    ~Configuration(){};
};


#endif