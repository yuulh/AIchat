#include "chatBp.h"
#include "../Log.h"
#include "../Configuration.h"
#include "../utils.h"
#include <wfrest/HttpServer.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFTaskFactory.h>


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
                    const string sql = "SELECT assistant_id, name, introduction, avatar, background FROM assistant_" + tag + " LIMIT " + std::to_string(offset) + ", " + std::to_string(page_size);
                    mysqlClient->execute(sql, [&](WFMySQLTask *task){
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
            string key = "conversation_list_" + user_id;
            redisClient->GET(key, [&](WFRedisTask *task){
                protocol::RedisValue &redis_resp = GET_REDIS_RESP;
                vector<string> redis_ret;
                redisClient->parseResp(redis_resp, redis_ret);

                if(redis_ret.empty()) {
                    LOG_ERROR("redis query is null");

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
                        redisClient->SET(key, ret.dump(), nullptr);
                        wait_group.done();
                    });
                }else{
                    // 缓存命中
                    resp->Json(redis_ret[0]);
                    wait_group.done();
                }
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
            conversation_id: string,  // 会话id，用于跟踪会话，保持记忆，若此字段为空或查询为空，开启一个新会话
            stream: bool,
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

       Json send_model = Json::Object{
        {"stream", true},
        {"model", CONFIG["MODEL_USER"]}
       };

        Json body = Json::parse(req->body());
        
        sprintf(logBuf, "post data: %s", body.dump().c_str());
        LOG_DEBUG_BUF;

        if(!body.size() || body.is_null()){
            LOG_INFO("body is null");
            resp->String("参数不能为空");
            return;
        }
        // 首先判断会话id是否为空，判断是否需要开启新会话
        // 如果是新会话，先根据角色id查询角色设定，messages为第一个聊天内容，应该push_front角色设定
        // 如果是旧会话，则messages字段应该就是模型需要的所有内容

        // redis中查询角色id，失败则查询数据库
        WFFacilities::WaitGroup wait_group(1);

        const string conversation_id = body["conversation_id"];
        string assistant_prompt;
        // 第一次聊天需要获取角色设定
        if(conversation_id.empty()){
            string key = "assistant_" + body["assistant_id"].get<string>();
            redisClient->GET(key, [&](WFRedisTask *task){
                protocol::RedisValue redis_value = GET_REDIS_RESP;
    
                vector<string> redis_ret;
                redisClient->parseResp(redis_value, redis_ret);
                
                WFFacilities::WaitGroup wait_get_prompt(1);
                if(redis_ret.empty()) {
                    // 去数据库查询
                    mysqlClient->setDB("AIchat");
                    const string sql = "SELECT prompt FROM prompt WHERE assistant_id = \"" + body["assistant_id"].get<string>() + "\"";
                    mysqlClient->execute(sql , [&](WFMySQLTask *task){
                        Json ret = GET_MYSQL_RESP;

                        if(ret.is_null()){
                            LOG_ERROR("mysql query is null");
                        }

                        assistant_prompt = ret[0]["prompt"].get<string>();

                        // TODO: 研究为什么key失效
                        key = "assistant_" + body["assistant_id"].get<string>();
                        redisClient->SET(key, ret.dump(), nullptr);
                        
                        wait_get_prompt.done();
                    });
                }else{
                    // 缓存命中
                    assistant_prompt = Json::parse(redis_ret[0])[0]["prompt"].get<string>();  // TODO: 优化此处性能
                    wait_get_prompt.done();
                }
                wait_get_prompt.wait();

                send_model["messages"].push_back(Json::Object{
                   {"role", "system"},
                   {"content", assistant_prompt} 
                });

                send_model["messages"].push_back(body["messages"][0]);
                // 至此，请求体拼接完成
                wait_group.done();

            });
        }else{  // 旧会话，直接获取body中的messages
            send_model["messages"] = body["messages"];
            wait_group.done();
        }
        
        wait_group.wait();

        WFFacilities::WaitGroup wait_tts_res(1);
        Json tts_resp;
        httpClient->POST(CONFIG["LLM_URL"],
        {
            {"Authorization", "Bearer " + this->api_key},
            {"Content-Type", "application/json"}
        },
        send_model,
        [&](){
            // TTS 请求示例
            /*
            curl --location 'https://openai.qiniu.com/v1/voice/tts' \
                --header 'Content-Type: application/json' \
                --header 'Authorization: Bearer <API_KEY>' \
                --data '{
                    "audio": {
                        "voice_type": "zh_male_M392_conversation_wvae_bigtts",
                        "encoding": "mp3",
                        "speed_ratio": 1.0
                    },
                    "request": {
                        "text": "你好，世界！"
                    }
                }'
            */

            LOG_DEBUG("geted LLM res");

            // 获取音色
            string voice_type;
            WFFacilities::WaitGroup wait_voice(1);
            redisClient->GET("tts_voice_" + body["assistant_id"].get<string>(), [&](WFRedisTask *task){
                protocol::RedisValue redis_value = GET_REDIS_RESP;
                vector<string> redis_ret;
                redisClient->parseResp(redis_value, redis_ret);


                // TODO: 查缓存封装一下

                if(redis_ret.empty()){
                    // 缓存未命中
                    // 去数据库查询
                    mysqlClient->setDB("AIchat");
                    const string sql = "SELECT voice_type FROM assistant_all WHERE assistant_id = \"" + body["assistant_id"].get<string>() + "\"";
                    mysqlClient->execute(sql , [&](WFMySQLTask *task){
                        Json ret = GET_MYSQL_RESP;
                        if(ret.is_null()){
                            LOG_ERROR("mysql query is null");
                        }

                        voice_type = ret[0]["voice_type"].get<string>();

                        redisClient->SET("tts_voice_" + body["assistant_id"].get<string>(), voice_type, nullptr);
                        wait_voice.done();
                    });
                }else{
                    // 缓存命中
                    voice_type = redis_ret[0];
                    wait_voice.done();
                }
            });
            wait_voice.wait();

            sprintf(logBuf, "voice_type: %s", voice_type.c_str());
            LOG_DEBUG_BUF;

            Json tts_req;
            tts_req["audio"] = Json::Object{
                {"voice_type", voice_type},
                {"encoding", "mp3"},
            };
            // tts_req["audio"]["speed_ratio"] = 1.0;
            // tts_req["audio"]["encoding"] = "mp3";
            // tts_req["audio"]["voice_type"] = voice_type;

            const string &resp = httpClient->getModelResp();

            sprintf(logBuf, "tts text: %s", resp.c_str());
            LOG_DEBUG_BUF;

            tts_req["request"]["text"] = resp;

            sprintf(logBuf, "tts req body: %s", tts_req.dump().c_str());
            LOG_DEBUG_BUF;

            // 时间不够，只能粗鲁一点了

            WFFacilities::WaitGroup wait_tts_temp(1);
            WFHttpTask *task = WFTaskFactory::create_http_task(CONFIG["TTS_URL"], 3, 3, [&](WFHttpTask *task){
                sprintf(logBuf, "tts req: %s", CONFIG["TTS_URL"].c_str());
                LOG_DEBUG_BUF;

                // TODO: 错误检查
                LOG_DEBUG("geted tts res");
                const void *http_resp;
                size_t len;
                task->get_resp()->get_parsed_body(&http_resp, &len);

                sprintf(logBuf, "tts resp size: %ld", len);
                LOG_DEBUG_BUF;

                string resp_buf = static_cast<const char *>(http_resp);

                sprintf(logBuf, "tts resp: %ld %s", resp_buf.size(), resp_buf.c_str());
                LOG_DEBUG_BUF;

                tts_resp = Json::parse(resp_buf);

                sprintf(logBuf, "tts resp: %s", tts_resp.dump().c_str());
                LOG_DEBUG_BUF;
                wait_tts_temp.done();
            });
            task->get_req()->set_method("POST");
            task->get_req()->add_header_pair("Content-Type", "application/json");
            task->get_req()->add_header_pair("Authorization", "Bearer " + this->api_key);


            sprintf(logBuf, "api_key: %s", this->api_key.c_str());
            LOG_DEBUG_BUF;

            task->get_resp()->append_output_body(tts_req.dump());
            task->start();
            wait_tts_temp.wait();
            wait_tts_res.done();
        });

        wait_tts_res.wait();

        // 获取当前时间戳
        string timestamp = std::to_string(time(nullptr));
        const string path = "/usr/AIchat/public/" + conversation_id + "_" + timestamp + ".mp3";
        int fd = open(path.c_str(), O_CREAT | O_RDWR, 0666);

        string audio_url;
        if(tts_resp["data"]){
            const string audio_file = tts_resp["data"].get<string>();
            write(fd, audio_file.c_str(), audio_file.size());
            audio_url = path;
        }


        Json send_resp = Json::Object{
            {"message", "success"},
            {"audio_url", audio_url},
            {"text", httpClient->getModelResp()}
        };

        resp->Json(tts_resp);

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
: httpClient(new HttpClient())
{
}

ChatBp::ChatBp(const string &key)
: api_key(key)
, mysqlClient(new MySqlClient(CONFIG["MySQL_HOST"], CONFIG["MySQL_PORT"],
              CONFIG["MySQL_USER"], CONFIG["MySQL_PASSWORD"],
              CONFIG["MySQL_DATABASE"], atoi(CONFIG["Retry_MAX"].c_str())))
, redisClient(new RedisClient(CONFIG["Redis_HOST"], CONFIG["Redis_PORT"],
              CONFIG["Redis_PASSWORD"], CONFIG["Redis_DATABASE"],
              atoi(CONFIG["Retry_MAX"].c_str())))
, httpClient(new HttpClient())
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
