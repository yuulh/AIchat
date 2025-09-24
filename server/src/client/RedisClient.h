#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include <string>
#include <vector>

#include <workflow/WFTaskFactory.h>

using std::string;
using std::vector;

class RedisClient {
    string host;
    string port;
    string password;
    string database;
    int retry_max;
    bool connected;

    WFRedisTask *task;
    protocol::RedisValue redis_resp;  // 保存redis返回的结果

    static void redis_callback(WFRedisTask *task);

public:
    RedisClient();
    RedisClient(const string &host, const string &port,
                const string &password, const string &database,
                int retry_max);
    ~RedisClient();
    
    int execute(const string &command, const vector<string> &args);

    string getUrl();

    int set(const string &key, const string &value);
    int get(const string &key);
    int del(const string &key);
    int exists(const string &key);
};

#endif