#ifndef CHATBP_H
#define CHATBP_H

#include "BpBase.h"
#include "../client/HttpClient.h"
#include <wfrest/BluePrint.h>
#include <wfrest/Json.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using namespace wfrest;


/*
    用于和模型进行交互
*/
class ChatBp : public BpBase{
    string key;  // api key
    HttpClient httpClient;  // 用于和模型交互的http客户端
public:
    ChatBp();
    ChatBp(const string &key);

    void setContext(void *context);

    int sendMessage(const string &message);

    void setBP() override;

    Json system;  // 用于设置模型的基础行为、身份或规则
    Json user;  // 用户发送的内容
    Json assistant;  // 模型的历史回复
    
    void *context;  // 对象间通信用
};

#endif