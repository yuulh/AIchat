#include "HttpClient_2.h"
#include "../Log.h"
#include <workflow/Workflow.h>

void HttpClient::http_callback(WFHttpTask *task)
{
    protocol::HttpRequest *req = task->get_req();
    protocol::HttpResponse *resp = task->get_resp();
    int state = task->get_state();
    int error = task->get_error();
    
    if (state != WFT_STATE_SUCCESS) { 
        sprintf(logBuf, "请求失败，错误码: %d, 错误原因: %s\n", 
            state, WFGlobal::get_error_string(state, error));
        LOG_ERROR(logBuf);
        // 向客户端发送错误信息
        // resp->String("{\"error\": \"Failed to fetch data from remote API\"}");
    } else {
        LOG_DEBUG("结束一个extract");
        sprintf(logBuf, "model_resp: %p", task->user_data);
        LOG_DEBUG_BUF;
        
        const string *model_resp = static_cast<string *>(task->user_data);
        sprintf(logBuf, "模型回复: %s", model_resp->c_str());
        LOG_DEBUG_BUF;

        
        return;
    }

}

void HttpClient::execute(const string &method, const string &url, const vector<std::pair<string, string>>& headers,
                         const Json& body, const http_callback_type & callback)
{
    sprintf(logBuf, "http req: %s %s", method.c_str(), url.c_str());
    LOG_DEBUG_BUF;

    // TODO: 3改成配置文件
    task = WFTaskFactory::create_http_task(url, 3, 3, http_callback);
    task->get_req()->set_method(method);
    // TODO: 设置超时时间
    // task->set_receive_timeout(3000);  // ms

    for(auto &header: headers){
        task->get_req()->add_header_pair(header.first, header.second);
        sprintf(logBuf, "header: %s: %s", header.first.c_str(), header.second.c_str());
        LOG_DEBUG_BUF;
    }
    if(method == "POST"){
        sprintf(logBuf, "http req body: %s", body.dump().c_str());
        LOG_DEBUG_BUF;
        task->get_req()->append_output_body(body.dump());
    }

    auto *series_task = Workflow::create_series_work(task, NULL);

    auto *get_res_task = WFTaskFactory::create_go_task("getRES", callback);
    if(callback){
        series_task->push_back(get_res_task);
    }

    series_task->start();
}

void HttpClient::GET(const string &url, const vector<std::pair<string, string>>& headers, const http_callback_type &callback)
{
    execute("GET", url, headers, {}, callback);
}

void HttpClient::POST(const string &url, const vector<std::pair<string, string>>& headers, const Json& body, const http_callback_type &callback)
{
    execute("POST", url, headers, body, callback);
}
