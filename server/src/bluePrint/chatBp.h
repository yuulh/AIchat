#ifndef CHATBP_H
#define CHATBP_H

#include "BpBase.h"
#include <wfrest/BluePrint.h>
#include <string>
#include <vector>

using std::string;
using std::vector;


/*
    用于和模型进行交互
*/
class ChatBp : public BpBase{
    string key;  // api key
public:
    ChatBp();
    ChatBp(const string &key);

    void setContext(void *context);

    int sendMessage(const string &message);

    void setBP() override;

    vector<string> system;  // 用于设置模型的基础行为、身份或规则，是上下文的最高级指令。
    vector<string> user;  // 用户发送的内容
    vector<string> assistant;  // 模型的历史回复
    
    void *context;  // 对象间通信用
};

#endif