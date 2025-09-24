#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "workflow/RedisMessage.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/WFFacilities.h"

#define RETRY_MAX       2

struct tutorial_task_data
{
	std::string url;
	std::string key;
};

void redis_callback(WFRedisTask *task)
{
	protocol::RedisRequest *req = task->get_req();
	protocol::RedisResponse *resp = task->get_resp();
	int state = task->get_state();
	int error = task->get_error();
	protocol::RedisValue val;

	switch (state)
	{
	case WFT_STATE_SYS_ERROR:
		fprintf(stderr, "system error: %s\n", strerror(error));
		break;
	case WFT_STATE_DNS_ERROR:
		fprintf(stderr, "DNS error: %s\n", gai_strerror(error));
		break;
	case WFT_STATE_SSL_ERROR:
		fprintf(stderr, "SSL error: %d\n", error);
		break;
	case WFT_STATE_TASK_ERROR:
		fprintf(stderr, "Task error: %d\n", error);
		break;
	case WFT_STATE_SUCCESS:
		resp->get_result(val);
		if (val.is_error())
		{
			fprintf(stderr, "%*s\n", (int)val.string_view()->size(),
									val.string_view()->c_str());
			state = WFT_STATE_TASK_ERROR;
		}
		break;
	}

	if (state != WFT_STATE_SUCCESS)
	{
		fprintf(stderr, "Failed. Press Ctrl-C to exit.\n");
		return;
	}
    

	std::string cmd;
	req->get_command(cmd);
	if (cmd == "SET")
	{
        // SET完成之后再GET查询结果
		tutorial_task_data *data = (tutorial_task_data *)task->user_data;
		WFRedisTask *next = WFTaskFactory::create_redis_task(data->url,
															 RETRY_MAX,
															 redis_callback);

		next->get_req()->set_request("GET", { data->key });
		/* Push next task(GET task) to current series. */
		series_of(task)->push_back(next);
		fprintf(stderr, "Redis SET request success. Trying to GET...\n");
	}
	else /* if (cmd == "GET") */
	{
		if (val.is_string())
		{
			fprintf(stderr, "Redis GET success. value = %s\n",
					val.string_value().c_str());
		}
		else
		{
			fprintf(stderr, "Error: Not a string value. \n");
		}

		fprintf(stderr, "Finished. Press Ctrl-C to exit.\n");
	}
}

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

int main(int argc, char *argv[])
{
	WFRedisTask *task;


	signal(SIGINT, sig_handler);

	/* This struct only used in this tutorial. */
	struct tutorial_task_data data;

	/* Redis URL format: redis://:password@host:port/dbnum
	   examples:
	   redis://127.0.0.1
	   redis://:12345@redis.sogou:6379/3
	*/
	data.url = "redis://47.109.39.124/1";

	data.key = "nihao";

	task = WFTaskFactory::create_redis_task(data.url, RETRY_MAX,
											redis_callback);
	protocol::RedisRequest *req = task->get_req();
	req->set_request("SET", { data.key, "ppp" });
    
	/* task->user_data is a public (void *), can store anything. */
	task->user_data = &data;
    
	/* task->start() equel to:
    * Workflow::start_series_work(task, nullptr) or
    * Workflow::create_series_work(task, nullptr)->start() */
   task->start();
   
	wait_group.wait();
	return 0;
}

