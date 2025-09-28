#include "HttpClient.h"
#include "../Log.h"
#include <workflow/Workflow.h>

void HttpClient::http_extract(WFHttpChunkedTask *task)
{
    protocol::HttpMessageChunk *chunk = task->get_chunk();
    const void *chunk_data;
    size_t chunk_size;

    chunk->get_chunk_data(&chunk_data, &chunk_size);    // extract函数里，get_chunk_data必然成功

    // 因为返回消息格式是：data: {xxx}
    /*  json示例
    {
        "id":"chatcmpl-9ef8d824a79a43f182fce220eaad31d9","object":"chat.completion.chunk","created":1758976270,"model":"qwen-turbo",
        "choices":[{"index":0,"delta":{"content":"啊"},"finish_reason":null}]
    }
    */

    // 不优雅的获取结果

    
    string chunk_buf = string((const char *)chunk_data);

    // sprintf(logBuf, "chunk_data: %p  %s", chunk_data, chunk_buf.c_str());
    // LOG_DEBUG_BUF;

    if(chunk_buf.empty() || chunk_buf.size() < 20) return;
    Json data = Json::parse(chunk_buf.substr(strlen("data: ")));


    sprintf(logBuf, "a extract json dump: /[%s]/\n", data.dump().c_str());
    LOG_DEBUG_BUF;

    const string &content = data["choices"][0]["delta"]["content"].get<string>();
    if(!content.empty())
        static_cast<string *>(task->user_data)->append(content);

    sprintf(logBuf, "user_data_size: %ld", static_cast<string *>(task->user_data)->size());
    LOG_DEBUG_BUF;

    /* auto *resp = task->get_resp();

    resp->add_header_pair("Content-Type", "text/event-stream");
	resp->add_header_pair("Connection", "keep-alive");
	resp->add_header_pair("Cache-Control", "no-cache");

    resp->append_output_body((const char *)chunk_data); */

}

void HttpClient::http_callback(WFHttpChunkedTask *task)
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
    task = WFHttpChunkedClient::create_chunked_task(url, 3, http_extract, http_callback);
    task->get_req()->set_method(method);
    // TODO: 设置超时时间
    // task->set_receive_timeout(3000);  // ms

    task->user_data = &model_resp_buf;

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
