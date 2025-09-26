#include "userBp.h"

UserBp::UserBp(shared_ptr<HttpClient> httpClient, shared_ptr<MySqlClient> mysqlClient, shared_ptr<RedisClient> redisClient)
    : httpClient(httpClient), mysqlClient(mysqlClient), redisClient(redisClient)
{
}

void UserBp::setBP()
{
    bp.POST("login", )
}