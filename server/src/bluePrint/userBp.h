#ifndef USERBP_H
#define USERBP_H

#include "BpBase.h"
#include "../client/MySqlClient.h"
#include "../client/RedisClient.h"
#include "../client/HttpClient.h"
#include <memory>
#include <string>

using std::string;
using std::shared_ptr;

class UserBp : public BpBase {
    shared_ptr<HttpClient> httpClient;
    shared_ptr<MySqlClient> mysqlClient;
    shared_ptr<RedisClient> redisClient;
public:
    UserBp();
    UserBp(shared_ptr<HttpClient> httpClient, shared_ptr<MySqlClient> mysqlClient, shared_ptr<RedisClient> redisClient);
    // TODO: 移动、复制控制

    void setBP() override;
};

#endif