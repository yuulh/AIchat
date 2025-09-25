#include "RedisClient.h"
#include "../Log.h"

RedisClient::RedisClient(const string& host, const string& port,
    const string& password, const string& database,
    int retry_max)
    : host(host)
    , password(password)
    , database(database)
    , port(port)
    , retry_max(retry_max)
    , task(nullptr)
{
}

/* RedisClient::~RedisClient()
{
    if (connected) {
    }
} */

void RedisClient::redis_callback(WFRedisTask* task)
{
    protocol::RedisRequest* req = task->get_req();
    protocol::RedisResponse* resp = task->get_resp();
    int state = task->get_state();
    int error = task->get_error();

	sprintf(logBuf, "Redis state: %d, error: %d", state, error);
	LOG_DEBUG_BUF;

    switch (state) {
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
		case WFT_STATE_SUCCESS:{
			protocol::RedisValue &redisResp = static_cast<RedisClient *>(task->user_data)->redis_resp;
			resp->get_result(redisResp);
			if (redisResp.is_error()) {
				sprintf(logBuf, "%*s\n", (int)redisResp.string_view()->size(),
					redisResp.string_view()->c_str());
				LOG_ERROR(logBuf);
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
				LOG_DEBUG(logBuf);
			}
			break;
		}
		case WFT_STATE_ABORTED:
			LOG_ERROR("Task aborted.");
			break;
    }

    if (state != WFT_STATE_SUCCESS) {
        sprintf(logBuf, "Redis command execution failed. ");
        LOG_ERROR(logBuf);
        return;
    }

	RedisClient *client = static_cast<RedisClient *>(task->user_data);
	std::lock_guard<std::mutex> lock(client->_mutex);
	client->task_finished_ = true;
	client->_cv.notify_one();


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

void RedisClient::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this] { return task_finished_; });
}

void RedisClient::getResp(const protocol::RedisValue& resp, vector<string>& ret)
{
    int val_type = resp.get_type();
    switch (val_type) {
    case REDIS_REPLY_TYPE_NIL:
        break;
    case REDIS_REPLY_TYPE_STRING:
        ret.push_back(resp.string_value());
        break;
    case REDIS_REPLY_TYPE_INTEGER:
        ret.push_back(std::to_string(resp.int_value()));
        break;
    case REDIS_REPLY_TYPE_ARRAY:
        int size = resp.arr_size();
        for (int i = 0; i < size; i++) {
            auto& next = resp.arr_at(i);
            getResp(next, ret);
        }

        break;
    }
}

string RedisClient::getUrl()
{
    return "redis://" + host + ":" + port + "/" + database;
}

int RedisClient::execute(const string& command, const vector<string>& args)
{
	sprintf(logBuf, "Redis url: %s send command: %s %s", this->getUrl().c_str(), command.c_str(), args[0].c_str());
	LOG_DEBUG_BUF;
    this->task = WFTaskFactory::create_redis_task(this->getUrl(), retry_max, this->redis_callback);
    this->task->get_req()->set_request(command, args);
    this->task->user_data = this;
    this->task->start();
    return 0;
}

int RedisClient::SET(const string& key, const string& value)
{
    return execute("SET", { key, value });
}

int RedisClient::GET(const string& key)
{
    return execute("GET", { key });
}

int RedisClient::DEL(const string& key)
{
    return execute("DEL", { key });
}

int RedisClient::EXISTS(const string& key)
{
    return execute("EXISTS", { key });
}
