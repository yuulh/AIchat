#include "chatBp.h"
#include "../Log.h"
#include "../Configuration.h"
#include "../utils.h"
#include <wfrest/HttpServer.h>
#include <workflow/WFFacilities.h>



void ChatBp::setBP()
{
    // 获取角色列表
    // /assistant/list?tag=all&page=1&page_size=10
    bp.GET("/assistant/list", [this](const HttpReq *req, HttpResp *resp){
        sprintf(logBuf, "get req %s", req->get_request_uri());
        LOG_DEBUG_BUF;

        const string &tag = req->query("tag");
        const int &page = req->query("page").empty() ? 1 : atoi(req->query("page").c_str());
        const int &page_size = req->query("page_size").empty() ? 10 : atoi(req->query("page_size").c_str());
        int offset = (page - 1) * page_size;

        if(tag.empty()) {
            resp->set_status(HttpStatusBadRequest);
            return;
        }

        auto getKeyName = [](const string &tag, const int &page, const int &page_size) {
            return "assistant_" + tag + "_page_" + std::to_string(page) + "_" + std::to_string(page_size);
        };

        WFFacilities::WaitGroup wait_group(1);
        if(redisClient){

            string key = getKeyName(tag, page, page_size);
            
            redisClient->GET(key, [&](WFRedisTask *task){
                protocol::RedisValue &redis_resp = GET_REDIS_RESP;
                
                // 未知原因，到这里key会被修改，需要重新赋值
                key = getKeyName(tag, page, page_size);
                vector<string> redis_ret;
                redisClient->parseResp(redis_resp, redis_ret);

                

                sprintf(logBuf, "redis_resp: %ld    redis_ret: %ld", redis_resp.arr_size(), redis_ret.size());
                LOG_DEBUG_BUF;

                if(redis_ret.empty()) {
                    // 去数据库查询
                    mysqlClient->setDB("AIchat");
                    mysqlClient->execute("SELECT * FROM assistant_" + tag + " LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size), [&](WFMySQLTask *task){
                        Json ret = GET_MYSQL_RESP;

                        if(ret.is_null()){
                            LOG_ERROR("mysql query is null");
                        }

                        redisClient->SET(key, ret.dump(), nullptr);
                        resp->Json(ret);
                        wait_group.done();
                    });
                    
                }else{
                    // 缓存命中
                    resp->Json(redis_ret[0]);
                    wait_group.done();
                }
            });
        }else{
            LOG_ERROR("redisClient is null");
            resp->String("服务器查询失败");
            wait_group.done();
        }
        wait_group.wait();
        
    });
        
    // 获取会话列表
    // 请求头需要包含Cookie
    // /conversation/list?page=1&page_size=10
    /*
    响应:
    [{
        "conversation_id": "string",
        "title": "string",
        "update_time": "string"
    }]
    */
    bp.GET("/conversation/list", [this](const HttpReq *req, HttpResp *resp){
        sprintf(logBuf, "get req %s", req->get_request_uri());
        LOG_DEBUG_BUF;

        // 获取cookie
        const std::string &cookie_val = req->cookie("u");  // 用户id
        
        sprintf(logBuf, "cookie_val: %s", cookie_val.c_str());
        LOG_DEBUG(logBuf);

        if(cookie_val.empty()) {
            resp->set_status(HttpStatusBadRequest);
            return;
        }

        const int &page = req->query("page").empty() ? 1 : atoi(req->query("page").c_str());
        const int &page_size = req->query("page_size").empty() ? 10 : atoi(req->query("page_size").c_str());
        int offset = (page - 1) * page_size;

        // 去redis查找用户id
        const string user_id = Cookie2User_id(*redisClient, cookie_val);

        // 不存在该cookie，登录失效
        if(user_id.empty()) {
            resp->set_status(HttpStatusBadRequest);
            resp->String("无效的Cookie");
            return;
        }else{
            WFFacilities::WaitGroup wait_group(1);
            // 数据库查询会话列表
            mysqlClient->setDB("AIchat");
            const string sql = "SELECT conversation_id, title, update_time FROM conversation_list WHERE user_id = '" + user_id +
                                 "' LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);
            mysqlClient->execute(sql, [&](WFMySQLTask *task){
                Json ret = GET_MYSQL_RESP;
                if(ret.is_null()){
                    LOG_ERROR("mysql query is null");
                }
                resp->Json(ret);
                wait_group.done();
            });

            wait_group.wait();
        }
        
    });

    // 与模型聊天
    bp.POST("/assistant/stream", [this](const HttpReq *req, HttpResp *resp)
    {
        /* 用户发送一个消息，内容应该为：
        {
            assistant_id: string,   // 角色id，用于查询角色设定
            conversation_id: string,  // 会话id，用于跟踪会话，保持记忆
            stream: bool,
            first: bool,  // 是否第一次聊天, 或者说是否为一个新的会话
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

       /* 模型需要的
        
            curl https://openai.qiniu.com/v1/chat/completions \
            --request POST \
            --header 'Authorization: Bearer <API_KEY>' \
            --header 'Content-Type: application/json' \
            --data '{
            "stream": true,
            "model": "qwen-turbo",
            "messages": [
                {
                "role": "system",
                "content": "You are a helpful assistant."  // 角色设定
                },
                {
                "role": "user",
                "content": "你好"
                },
                {
                    "role": "assistant",
                    "content": "你好啊，我是通义千问。"
                },
            ]
            }'
       */
        json body = json::parse(req->body());
        
        sprintf(logBuf, "post data: %s", body.dump().c_str());
        LOG_DEBUG_BUF;

        if(!body.size() || body.is_null()){
            LOG_INFO("body is null");
            resp->String("参数不能为空");
            return;
        }

        // redis中查询角色id，失败则查询数据库
        WFFacilities::WaitGroup wait_group(2);

        // 第一次聊天需要获取角色设定
        if(body["first"]){
            redisClient->GET("assistant_" + body["assistant_id"], [&](WFRedisTask *task){
                protocol::RedisValue redis_value = GET_REDIS_RESP;
    
                vector<string> redis_ret;
                redisClient->parseResp(redis_value, redis_ret);
    
                if(redis_ret.empty()) {
                    // 去数据库查询
                    mysqlClient->setDB("AIchat");
                    const string sql = "SELECT prompt FROM prompt WHERE assistant_id = " + body["assistant_id"];
                    mysqlClient->execute(sql , [&](WFMySQLTask *task){
                        Json ret = GET_MYSQL_RESP;
    
                        if(ret.is_null()){
                            LOG_ERROR("mysql query is null");
                        }
    
                        redisClient->SET(key, ret.dump(), nullptr);
                        resp->Json(ret);
                        wait_group.done();
                    });
                }
    
            });
        }
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
: httpClient(new HttpClient(CONFIG["LLM_URL"]))
{
}

ChatBp::ChatBp(const string &key)
: key(key)
, mysqlClient(new MySqlClient(CONFIG["MySQL_HOST"], CONFIG["MySQL_PORT"],
              CONFIG["MySQL_USER"], CONFIG["MySQL_PASSWORD"],
              CONFIG["MySQL_DATABASE"], atoi(CONFIG["Retry_MAX"].c_str())))
, redisClient(new RedisClient(CONFIG["Redis_HOST"], CONFIG["Redis_PORT"],
              CONFIG["Redis_PASSWORD"], CONFIG["Redis_DATABASE"],
              atoi(CONFIG["Retry_MAX"].c_str())))
, httpClient(new HttpClient(CONFIG["LLM_URL"]))
{
}

void ChatBp::setContext(void *context)
{
    this->context = context;
}

int ChatBp::sendMessage(const string &message)
{
    return 0;
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
