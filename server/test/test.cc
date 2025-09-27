// wfrest接收jsonPOST请求


#include "wfrest/HttpServer.h"
#include "wfrest/Json.h"
#include <workflow/Workflow.h>
#include <workflow/WFTaskFactory.h>
#include <iostream>
#include <functional>

using namespace std;
using namespace wfrest;
using f = function<void(int)>;




void callback(WFHttpTask *task)
{
    protocol::HttpResponse *resp = task->get_resp();
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
		const void *res;
		size_t size;
		resp->get_parsed_body(&res, &size);

		cerr << (const char *)res << endl;
    }
}

int main()
{
	cout << std::to_string(time(NULL)) << endl;
		
	getchar();
}