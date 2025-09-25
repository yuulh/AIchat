#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <workflow/WFTaskFactory.h>

using std::string;
using std::vector;

class RedisClient {
    string host;
    string port;
    string password;
    string database;
    int retry_max;

    // 实现wait
    std::mutex _mutex;
    std::condition_variable _cv;
    bool task_finished_;

    WFRedisTask *task;
    
    static void redis_callback(WFRedisTask *task);
    
public:
    protocol::RedisValue redis_resp;  // 保存redis返回的结果
    RedisClient(const string &host, const string &port,
                const string &password, const string &database,
                int retry_max);
    // ~RedisClient();
    
    int execute(const string &command, const vector<string> &args);

    void wait();
    string getUrl();

    int SET(const string &key, const string &value);
    int GET(const string &key);
    int DEL(const string &key);
    int EXISTS(const string &key);

    int HGET(const string &key, const string &field);
    int HSET(const string &key, const string &field, const string &value);

    // 获取查询结果，resp传入this->redis_resp
    // 结果为纯string的数组
    void getResp(const protocol::RedisValue &resp, vector<string> &ret);
};

#endif