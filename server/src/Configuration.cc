#include "Configuration.h"

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>

using std::ifstream;
using std::cerr;
using std::cout;
using std::string;

string dirname = "/usr/AIchat";

string configPath = dirname + "/config/config.conf";

char logBuf[1024];  // 供全局使用的logBuf

// 初始化静态成员
Configuration *Configuration::p = nullptr;
pthread_once_t Configuration::onceInstance = PTHREAD_ONCE_INIT;

void Configuration::init()
{
    // 加载默认配置
    this->configs = {
        {"port", "8090"},  // 启动端口
        {"LLM_URL", "https://openai.qiniu.com/v1/chat/completions"},  // 大模型api的url
        {"MySQL_HOST", "127.0.0.1"},  // mysql数据库地址"
        {"MySQL_PORT", "3306"},  // mysql数据库端口
        {"MySQL_USER", "123"},
        {"MySQL_PASSWORD", "123"},
        {"Redis_HOST", "127.0.0.1"},
        {"Redis_PORT", "6379"},
    };

    ifstream ifs(this->path);
    if(!ifs.good()){
        cerr << "读取配置文件错误\n";
        ifs.close();
        return;
    }

    string s;
    while(std::getline(ifs, s)){
        auto conf = this->getPair(s, " ");
        if(this->configs.find(conf.first) == this->configs.end())  // 如果不存在这个配置项
            continue;
        
        this->configs[conf.first] = conf.second;
    }
    ifs.close();
    cout << "配置加载完成，配置项：" << this->configs.size() << "\n";
}

std::pair<string, string> Configuration::getPair(string &s, const char * delim)
{
    std::pair<string, string> res;
    char temp[s.size() + 1];
    strcpy(temp, s.c_str());
    res.first = strtok(temp, delim);
    res.second= strtok(nullptr, delim);
    return res;
}

Configuration * const Configuration::getInstance()
{
    if(p != nullptr) return p;

    pthread_once(&onceInstance, initInstance);
    return p;
}

void Configuration::initInstance()
{
    // 首次调用，创建单例对象并初始化
    cout << "配置类单例对象初始化\n";
    p = new Configuration();
    p->path = configPath;
    p->init();
    atexit(destroy);
}

void Configuration::destroy()
{
    if(p){
        delete p;
        p = nullptr;
        cout << "配置类单例对象销毁\n";
    }
}

configArgs_t &Configuration::getConfigs()
{
    return this->configs;
}


