#include "userBp.h"
#include "../utils.h"
#include "../Log.h"
#include "../Configuration.h"
#include "../client/MySqlClient.h"
#include "../client/RedisClient.h"
#include "../client/HttpClient.h"
#include <wfrest/HttpServer.h>
#include <workflow/WFFacilities.h>

using namespace wfrest;

UserBp::UserBp()
: mysqlClient(new MySqlClient(CONFIG["MySQL_HOST"], CONFIG["MySQL_PORT"],
              CONFIG["MySQL_USER"], CONFIG["MySQL_PASSWORD"],
              CONFIG["MySQL_DATABASE"], atoi(CONFIG["Retry_MAX"].c_str())))
, redisClient(new RedisClient(CONFIG["Redis_HOST"], CONFIG["Redis_PORT"],
              CONFIG["Redis_PASSWORD"], CONFIG["Redis_DATABASE"],
              atoi(CONFIG["Retry_MAX"].c_str())))
, httpClient(new HttpClient())
{}

UserBp::UserBp(shared_ptr<HttpClient> httpClient, shared_ptr<MySqlClient> mysqlClient, shared_ptr<RedisClient> redisClient)
    : httpClient(httpClient), mysqlClient(mysqlClient), redisClient(redisClient)
{}

void UserBp::setBP()
{
    bp.POST("login", [this](const HttpReq *req, HttpResp *resp) {
        Json body = req->json();

        // 数据库查询账号密码是否正确
        const string user = body["user"];
        const string pwd = body["pwd"];

        sprintf(logBuf, "login: user: %s, pwd: %s", user.c_str(), pwd.c_str());
        LOG_DEBUG_BUF;

        // 只允许数字字母和_
        if (0) {
            LOG_ERROR("格式错误");
            resp->set_status(HttpStatusNotAcceptable);
            resp->String("格式错误");
            return;
        }

        WFFacilities::WaitGroup wait_group(1);

        mysqlClient->setDB("AIchat");
        const string sql = "select * from user where user = '" + user + "' and pwd = '" + pwd + "'";
        mysqlClient->execute(sql, [&](WFMySQLTask *task){
            LOG_DEBUG("登录任务mysql查询结束，处理开始");

            Json &res = *static_cast<Json *>(task->user_data);

            // sprintf(logBuf, "mysql_resp %ld: %s", res.size(), res.dump().c_str());
            // LOG_DEBUG_BUF;
            
            if (!res.size() || res.is_null()) {
                LOG_ERROR("用户名或密码错误");
                resp->set_status(HttpStatusNotAcceptable);
                resp->String("用户名或密码错误");
                wait_group.done();
                return;
            }

            
            string status = res[0]["status"];
    
            if(status == "OK"){
                LOG_INFO("当前用户已登录");
                resp->String("当前用户已登录");
            }else if(status == "OFF"){
                string uuid = generate_uuid();
    
                sprintf(logBuf, "登录成功: uuid: %s", uuid.c_str());
                LOG_DEBUG_BUF;
    
                // TODO: 登录成功后状态置为OK
    
                redisClient->SET(uuid, user, nullptr);
                redisClient->EXPIRE(uuid, REDIS_TTL_1_DAY, nullptr);
    
                HttpCookie cookie;
                cookie.set_key("u")
                        .set_value(uuid)
                        .set_http_only(true);
    
                sprintf(logBuf, "set cookie: u = %s", uuid.c_str());
                LOG_DEBUG_BUF;
    
                resp->add_header("Access-Control-Allow-Origin", "*"); // 允许所有源
                resp->add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                resp->add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
                resp->add_header("Access-Control-Allow-Credentials", "true");
                resp->add_cookie(std::move(cookie));
                resp->String("登录成功");
            }else {
                sprintf(logBuf, "登录失败，用户状态: %s", status.c_str());
                LOG_ERROR(logBuf);
                resp->set_status(HttpStatusNotAcceptable);
                resp->String("用户状态异常");
            }

            wait_group.done();
        });

        wait_group.wait();

        
    });
}