// wfrest接收jsonPOST请求


#include "wfrest/HttpServer.h"
#include "wfrest/Json.h"
#include <workflow/Workflow.h>
#include <iostream>

using namespace std;
using namespace wfrest;

int main()
{
	WFRedisTask *task0 = WFTaskFactory::create_redis_task("redis://47.109.39.124:6379/1", 3, [](WFRedisTask *task){
		protocol::RedisRequest* req = task->get_req();
		protocol::RedisResponse* resp = task->get_resp();
		int state = task->get_state();
		int error = task->get_error();

		char logBuf[1024]{0};
		cerr << "start\n";

		switch (state) {
			case WFT_STATE_SYS_ERROR:
				cerr << "sys error\n";
				break;
			case WFT_STATE_DNS_ERROR:
				cerr << "dns error\n";
				break;
			case WFT_STATE_SSL_ERROR:
				cerr << "ssl error\n";
				break;
			case WFT_STATE_TASK_ERROR:
				cerr << "task error\n";
				break;
			case WFT_STATE_SUCCESS:{
				protocol::RedisValue &redisResp = *static_cast<protocol::RedisValue *>(task->user_data);
				resp->get_result(redisResp);
				if (redisResp.is_error()) {
					cerr << "redis error\n";
					state = WFT_STATE_TASK_ERROR;
				} else {
					std::string cmd;
					req->get_command(cmd);
					vector<string> params;
					req->get_params(params);
					strcpy(logBuf, "执行成功: ");
					strncat(logBuf, cmd.c_str(), cmd.size());
					strcat(logBuf, " ");
					for (auto& param : params) {
						strncat(logBuf, param.c_str(), param.size());
						strcat(logBuf, " ");
					}

					printf("%s\n", logBuf);
				}
				break;
			}
			case WFT_STATE_ABORTED:
				cerr << "aborted\n";
				break;
		}

		if (state != WFT_STATE_SUCCESS) {
			cerr << "失败\n";
			return;
		}

	});

	protocol::RedisValue redisResp;
	task0->user_data = &redisResp;

	task0->get_req()->set_request("GET", {"haha"});

	auto *gotask = WFTaskFactory::create_go_task("aaa", [task0](){
		protocol::RedisValue *redisResp = (protocol::RedisValue *)task0->user_data;
		cerr << "go task\n";
		cerr << redisResp->string_value() << "\n";
	});

	SeriesWork *task = Workflow::create_series_work(task0, NULL);

	task->push_back(gotask);

	task->start();


	getchar();
}