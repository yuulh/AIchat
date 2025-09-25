#include "chatBp.h"
#include "../Log.h"
#include "../Configuration.h"
#include <wfrest/HttpServer.h>

void ChatBp::setBP()
{
    // 获取角色列表
    // /assistant/list?tag=all&page=1&page_size=10
    bp.GET("/assistant/list", [this](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        // const std::map<std::string, std::string>& query_list = req->query_list();
        const string &type = req->query("tag");
        const string &page = req->query("page");
        const string &page_size = req->query("page_size");
        if(type.empty() || page.empty() || page_size.empty())
        {
            resp->set_status(HttpStatusBadRequest);
            return;
        }

        if(redisClient){
            redisClient->execute(resp, "GET", "assistant:" + type);
        }
    });

    // 与模型聊天
    bp.POST("/chat", [this](const wfrest::HttpReq *req, wfrest::HttpResp *resp)
    {
        /*
        用户发送一个消息，内容应该为：
        {
            assistant_id: string,   // 角色id，用于查询角色设定
            conversation_id: string,  // 会话id，用于跟踪会话，保持记忆
            messages: [{
                role: string,
                content: [
                    type: string,
                    text: string,
                ],
            }],
            data: {},  // 额外的数据
        }
        */
        json body = json::parse(req->body());
        // redis中查询角色id，失败则查询数据库
        // 通过角色id获取角色设定等，不存在则不带任何设定
        // 获取会话id
        // 会话不存在则开启新会话
        // 存在则获取上下文
        // 拼接上下文
        // 发送到大模型


        // 获取结果
        // 返回给用户

    });
}

ChatBp::ChatBp()
: httpClient(HttpClient(CONFIG["LLM_URL"]))
{
}

ChatBp::ChatBp(const string &key)
: key(key)
, httpClient(HttpClient(CONFIG["LLM_URL"]))
{
}

void ChatBp::setContext(void *context)
{
    this->context = context;
}

int ChatBp::sendMessage(const string &message)
{
    
}
/* 
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
}; */
