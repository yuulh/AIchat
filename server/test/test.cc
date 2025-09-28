// wfrest接收jsonPOST请求

#include "wfrest/HttpServer.h"
#include "wfrest/Json.h"
#include <functional>
#include <iostream>
#include <workflow/WFTaskFactory.h>
#include <workflow/WFHttpChunkedClient.h>
#include <workflow/Workflow.h>

using namespace std;
using namespace wfrest;
using f = function<void(int)>;

void callback(WFHttpTask* task)
{
    protocol::HttpResponse* resp = task->get_resp();
    int state = task->get_state();
    int error = task->get_error();
    HttpResp* client_resp = (HttpResp*)task->user_data;

    if (state != WFT_STATE_SUCCESS) {
        fprintf(stderr, "请求失败，错误码: %d, 错误原因: %s\n",
            state, WFGlobal::get_error_string(state, error));
        // 向客户端发送错误信息
        client_resp->String("{\"error\": \"Failed to fetch data from remote API\"}");
    } else {
        // 正常完成，可以做一些清理工作
        const void* res;
        size_t size;
        resp->get_parsed_body(&res, &size);

        cerr << (const char*)res << endl;
    }
}

string getCurrDateTime()
{
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return string(buf);
}

int main()
{

    WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
	settings.endpoint_params.use_tls_sni = true;
	WORKFLOW_library_init(&settings);

    const void *http_resp;
    size_t http_resp_len;

    Json tts_req;
    tts_req["audio"] = Json::Object{
                {"voice_type", "qiniu_zh_male_ybxknjs"},
                {"encoding", "mp3"},
            };

    tts_req["request"]["text"] = "你好你好，我是你爹";

    Json tts_resp;
/*     WFHttpTask* task = WFTaskFactory::create_http_task("https://openai.qiniu.com/v1/voice/tts", 0, 3, [&](WFHttpTask* task) {
        // 错误检查

        if (task->get_state() != WFT_STATE_SUCCESS) {
            cerr << "error: " << gai_strerror(task->get_error()) << endl;
            return;
        }
        
        // const void *temp_http_resp;
        cout << task->get_resp()->get_parser()->msgsize << endl;
        // cerr << (char *)task->get_resp()->get_parser()->msgbuf << endl;
        task->get_resp()->get_parsed_body(&http_resp, &http_resp_len);

        cerr << "size = " << http_resp_len << endl;

        write(fd, task->get_resp()->get_parser()->msgbuf, task->get_resp()->get_parser()->msgsize);
        cerr << "ok\n";
        close(fd);
    }); */
    int fd = open("file", O_CREAT | O_WRONLY | O_TRUNC, 0666);
/* 
    task->get_req()->set_method("POST");
    task->get_req()->add_header_pair("Content-Type", "application/json");
    task->get_req()->add_header_pair("Authorization", "Bearer sk-02cb2e739a0ec134960a42c7409cc62d5d01d4e9ad6f411179a27ab1652fe808");

    task->get_req()->append_output_body(tts_req.dump());
    task->start();
    getchar(); */

    string res;
        int totol = 0;
    WFHttpChunkedTask *task = WFHttpChunkedClient::create_chunked_task("https://openai.qiniu.com/v1/voice/tts", 0, [&](WFHttpChunkedTask* task) { 
        protocol::HttpMessageChunk *chunk = task->get_chunk();
        const void *chunk_data;
        size_t chunk_size;

        chunk->get_chunk_data(&chunk_data, &chunk_size);    // extract函数里，get_chunk_data必然成功

        res += (char*)chunk_data;

        res.erase(res.size() - 2);

        totol += chunk_size;

        write(fd, chunk_data, chunk_size);
        // cerr << "write size = " << chunk_size << endl;
    }, [&](WFHttpChunkedTask* task) {
        close(fd);
        cerr << "totol = " << totol << endl;

        cerr << "res size = " << res.size() << endl;

        int fd2 = open("file2", O_CREAT | O_WRONLY | O_TRUNC, 0666);
        write(fd2, res.c_str(), res.size());

        Json json = Json::parse(res);
        cerr << "dump size = " << json.dump().size() << endl;


        cerr << "ok\n";
    });

    task->get_req()->set_method("POST");
    task->get_req()->add_header_pair("Content-Type", "application/json");
    task->get_req()->add_header_pair("Authorization", "Bearer sk-02cb2e739a0ec134960a42c7409cc62d5d01d4e9ad6f411179a27ab1652fe808");

    task->get_req()->append_output_body(tts_req.dump());
    task->start();
    getchar();


    /* int fd = open("../src/build/file", O_RDONLY);
    char* a = new char[557383];
    read(fd, a, 557383);
    close(fd);

    int http_resp_len = 557373;

    int i, j;
    for (i = 0; i < http_resp_len; i++) {
        if (a[i] == '{') {
            break;
        }
    }

    for (j = http_resp_len - 1; j >= 0; j--) {
        if (a[j] == '}') {
            break;
        }
    }

    string json_str(a+i, a+j+1);
    cerr << json_str.size() << endl;


    Json json = Json::parse(json_str);

    cerr << json.dump().size() << endl;
    // cerr << json["reqid"] << endl; */
}