#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>
#include <workflow/WFTaskFactory.h>
#include <wfrest/Json.h>
#include <vector>

#define GET_HTTP_RESP (*static_cast<Json *>(task->user_data))

using namespace wfrest;
using std::string;
using std::vector;

using http_callback_type = std::function<void(void)>;

class HttpClient{
    int redirect_max;
    int retry_max;

    Json http_resp;
    string model_resp_buf;  // 模型回复的文字内容
    WFHttpTask *task;

    static void http_callback(WFHttpTask *task);
    void execute(const string &method, const string &url, const vector<std::pair<string, string>>& headers, const Json& body, const http_callback_type & callback);

public:
    HttpClient(): redirect_max(3), retry_max(3) {}  // TODO: 配置获取


    // url，请求头，回调函数
    void GET(const string &url, const vector<std::pair<string, string>>& headers, const http_callback_type & callback);
    void POST(const string &url, const vector<std::pair<string, string>>& headers, const Json& body, const http_callback_type & callback);

    inline string &getModelResp(){ return this->model_resp_buf; }
};

#endif