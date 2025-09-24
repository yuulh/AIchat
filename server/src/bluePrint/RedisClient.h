#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include <string>
#include <vector>

using std::string;
using std::vector;

class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    int connect(const string &host, int port);
    int disconnect();
    int set(const string &key, const string &value);
    int get(const string &key, string &value);
    int del(const string &key);
    int exists(const string &key);
    int flushdb();
    int keys(const string &pattern, vector<string> &keys);
}

#endif