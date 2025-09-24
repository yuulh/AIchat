#include "RedisClient.h"
#include "../Log.h"

RedisClient::RedisClient()
    : connected(false)
    , task(nullptr)
{
}

RedisClient::RedisClient(const string &host, const string &port,
                const string &password, const string &database,
                int retry_max)
    : connected(false)
    , host(host)
    , password(password)
    , database(database)
    , port(port)
    , retry_max(retry_max)
    , task(nullptr)
{
}

RedisClient::~RedisClient()
{
    if(connected){
        disconnect();
    }
}

void RedisClient::redis_callback(WFRedisTask *task)
{
	protocol::RedisRequest *req = task->get_req();
	protocol::RedisResponse *resp = task->get_resp();
	int state = task->get_state();
	int error = task->get_error();

	switch (state)
	{
	case WFT_STATE_SYS_ERROR:
		sprintf(logBuf, "system error: %s\n", strerror(error));
        LOG_ERROR(logBuf);
		break;
    case WFT_STATE_DNS_ERROR:
		sprintf(logBuf, "DNS error: %s\n", gai_strerror(error));
        LOG_ERROR(logBuf);
		break;
	case WFT_STATE_SSL_ERROR:
		sprintf(logBuf, "SSL error: %d\n", error);
        LOG_ERROR(logBuf);
		break;
	case WFT_STATE_TASK_ERROR:
		sprintf(logBuf, "Task error: %d\n", error);
        LOG_ERROR(logBuf);
		break;
	case WFT_STATE_SUCCESS:
        auto &redis_resp = *((protocol::RedisValue *)task->user_data);
		resp->get_result(redis_resp);
		if (redis_resp.is_error())
		{
			sprintf(logBuf, "%*s\n", (int)redis_resp.string_view()->size(),
									redis_resp.string_view()->c_str());
            LOG_ERROR(logBuf);
			state = WFT_STATE_TASK_ERROR;
		}else{
            std::string cmd;
	        req->get_command(cmd);
            vector<string> params;
            req->get_params(params);
            strncpy(logBuf, cmd.c_str(), cmd.size());
            strncat(logBuf, " ", 1);
            for(auto &param : params){
                strncat(logBuf, param.c_str(), param.size());
                strncat(logBuf, " ", 1);
            }
            strncat(logBuf, "\n执行成功", 2);
            LOG_DEBUG(logBuf);
        }
		break;
	}

	if (state != WFT_STATE_SUCCESS)
	{
		sprintf(logBuf, "Redis command execution failed. ");
        LOG_ERROR(logBuf);
		return;
	}

	/* std::string cmd;
	req->get_command(cmd);
	if (cmd == "SET")
	{
        // SET结束之后再GET查询结果
		tutorial_task_data *data = (tutorial_task_data *)task->user_data;
		WFRedisTask *next = WFTaskFactory::create_redis_task(data->url,
															 RETRY_MAX,
															 redis_callback);

		next->get_req()->set_request("GET", { data->key });
		// Push next task(GET task) to current series.
		series_of(task)->push_back(next);
		fprintf(stderr, "Redis SET request success. Trying to GET...\n");
	}
	else // if (cmd == "GET")
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
	} */
}

string RedisClient::getUrl()
{
    return "redis://" + host + ":" + port + "/" + database;
}

int RedisClient::execute(const string& command, const vector<string> &args)
{
    this->task = WFTaskFactory::create_redis_task(this->getUrl(), retry_max, this->redis_callback);
    this->task->user_data = &this->redis_resp;
    this->task->get_req()->set_request(command, args);
    this->task->start();
    return 0;
}

int RedisClient::set(const string& key, const string& value)
{
    return execute("SET", {key, value});
}

int RedisClient::get(const string& key)
{
    return execute("GET", {key});
}

int RedisClient::del(const string& key)
{
    return execute("DEL", {key});
}

int RedisClient::exists(const string& key)
{
    return execute("EXISTS", {key});
}

