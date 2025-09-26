#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <workflow/WFTaskFactory.h>

#define REDIS_TTL_1_MINUTE 60
#define REDIS_TTL_1_HOUR 3600
#define REDIS_TTL_1_DAY 86400
#define REDIS_TTL_1_WEEK 604800
#define REDIS_TTL_1_MONTH 2592000
#define REDIS_TTL_LONG -1

using std::string;
using std::vector;

// TODO: 更优雅的实现，参数为RedisResp &resp，外部调用直接使用该参数就是结果
// 通过WFRedisTask获取序列，从context中获取protocol::RedisValue
using redis_query_callback = std::function<void(WFRedisTask *)>;

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
    
    int execute(const string& command, const vector<string>& args, const redis_query_callback &callback);

    void wait();
    string getUrl();

    int SET(const string &key, const string &value, const redis_query_callback &callback);
    int GET(const string &key, const redis_query_callback &callback);
    int DEL(const string &key, const redis_query_callback &callback);
    int EXISTS(const string &key, const redis_query_callback &callback);

    int HGET(const string &key, const string &field, const redis_query_callback &callback);
    int HSET(const string &key, const string &field, const string &value, const redis_query_callback &callback);

    int EXPIRE(const string &key, int seconds, const redis_query_callback &callback);

    // 递归解析查询结果
    // 结果为纯string的数组
    void parseResp(const protocol::RedisValue &resp, vector<string> &ret);
};

#endif