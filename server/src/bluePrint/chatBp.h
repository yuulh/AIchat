#ifndef CHATBP_H
#define CHATBP_H

#include "BpBase.h"
#include "../client/HttpClient.h"
#include "../client/MySqlClient.h"
#include "../client/RedisClient.h"
#include <wfrest/BluePrint.h>
#include <wfrest/Json.h>
#include <string>
#include <vector>
#include <memory>

using std::string;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using namespace wfrest;


/*
    用于和模型进行交互
*/
class ChatBp : public BpBase{
    string key;  // api key
    shared_ptr<HttpClient> httpClient;  // 用于和模型交互的http客户端
    shared_ptr<RedisClient> redisClient;
    shared_ptr<MySqlClient> mysqlClient;
public:
    ChatBp();
    ChatBp(const string &key);

    // TODO: 移动、复制控制

    void setContext(void *context);

    int sendMessage(const string &message);

    void setBP() override;

    Json system;  // 用于设置模型的基础行为、身份或规则
    Json user;  // 用户发送的内容
    Json assistant;  // 模型的历史回复
    
    void *context;  // 对象间通信用
};

#endif