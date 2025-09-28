#include "MySqlClient.h"
#include "../Log.h"
#include <workflow/WFTaskFactory.h>

string MySqlClient::getUrl() const
{
    return "mysql://" + user + ":" + password + "@" + host + ":" + port + "/" + database;
}

void MySqlClient::mysql_callback(WFMySQLTask* task)
{
    LOG_DEBUG("MySQL任务开始");
    
    if(!task->user_data){
        LOG_ERROR("this指针为空!");
        return;
    }
    MySqlClient *client = static_cast<MySqlClient *>(task->user_data);  // this指针

    client->mysql_resp.clear();

    // 判断任务状态
    if (task->get_state() != WFT_STATE_SUCCESS) {
        sprintf(logBuf, "task error: %d  %s", task->get_error(), task->get_resp()->get_error_msg().c_str());
        LOG_ERROR(logBuf);
        return;
    }

    MySQLResponse* resp = task->get_resp();

    // 判断MySQL响应是否错误
    if (resp->get_packet_type() == MYSQL_PACKET_ERROR) {
        sprintf(logBuf, "ERROR. error_code=%d %s\n",
            task->get_resp()->get_error_code(),
            task->get_resp()->get_error_msg().c_str());
        LOG_ERROR(logBuf);

        return;
    }

    MySQLResultCursor cursor(resp);
    const MySQLField *const *fields;  // 存放field结果
	std::vector<MySQLCell> arr;  // 行结果

    do {
        if (cursor.get_cursor_status() != MYSQL_STATUS_GET_RESULT &&
            cursor.get_cursor_status() != MYSQL_STATUS_OK) {
            LOG_ERROR("MySQL结果集错误");
            break;
        }

        // 读操作结果集
        if (cursor.get_cursor_status() == MYSQL_STATUS_GET_RESULT) {
            int field_size = cursor.get_field_count();
            int rows_size = cursor.get_rows_count();
            sprintf(logBuf, "读操作成功: field_count=%u rows_count=%u",
                field_size, rows_size);
            LOG_INFO(logBuf);

            // 获取field结果
            fields = cursor.fetch_fields();
            
            
            // setResp(client->mysql_resp, "db_name", fields[0]->get_db());
            // setResp(client->mysql_resp, "table_name", fields[0]->get_table());
            
            sprintf(logBuf, "数据库:[%s] 表:[%s]",
                fields[0]->get_db().c_str(), fields[0]->get_table().c_str());
                LOG_INFO(logBuf);
                
            // 解析field结果
            string key, val;
            while (cursor.fetch_row(arr)) {
                Json row;
                for (size_t i = 0; i < arr.size(); i++) {
                    key = fields[i]->get_name();
                    
                    // 单独处理浮点型
                    if(arr[i].is_double() || arr[i].is_float()){
                        const void* ptr;
                        size_t len;
                        int data_type;
                        arr[i].get_cell_nocopy(&ptr, &len, &data_type);
                        size_t pos;
                        for (pos = 0; pos < len; pos++)
                            if (*((const char*)ptr + pos) == '.')
                                break;
                        if (pos != len)
                            pos = len - pos - 1;
                        else
                            pos = 0;

                        if(arr[i].is_double())
                            sprintf(&val[0], "%.*lf", (int)pos, arr[i].as_double());
                        else
                            sprintf(&val[0], "%.*lf", (int)pos, arr[i].as_float());
                    } else if (arr[i].is_int()) {
                        val = std::to_string(arr[i].as_int());
                    } else if (arr[i].is_ulonglong()) {
                        val = std::to_string(arr[i].as_ulonglong());
                    } else if (arr[i].is_date() || arr[i].is_time() || arr[i].is_datetime()) {
                        val = arr[i].as_string();
                    } else if (arr[i].is_null()) {
                        val = "NULL";
                    } else {
                        val = arr[i].as_binary_string();
                    }
                    
                    setResp(row, key, val);
                }
                client->mysql_resp.push_back(row);
            }
        
        // 写操作结果集
        } else if (cursor.get_cursor_status() == MYSQL_STATUS_OK) {
            string key = "writed";
            string val;
            sprintf(&val[0], "成功写入%llu行", cursor.get_affected_rows());
            sprintf(logBuf, "  %d警告. insert_id=%llu. %s\n",
                cursor.get_warnings(), cursor.get_insert_id(),
                cursor.get_info().c_str());
            val += logBuf;

            setResp(client->mysql_resp, key, val);
        }

    } while (cursor.next_result_set());

    // series_of(task)->set_context(&client->mysql_resp);
    task->user_data = &client->mysql_resp;

    sprintf(logBuf, "%s", client->mysql_resp.dump().c_str());
    LOG_DEBUG_BUF;
}

void MySqlClient::execute(const string& sql, const mysql_query_callback &callback)
{
    sprintf(logBuf, "MySQL url: %s send command: %s", this->getUrl().c_str(), sql.c_str());
    LOG_DEBUG_BUF;

    task = WFTaskFactory::create_mysql_task(this->getUrl(), retry_max, mysql_callback);
    task->user_data = this;
    task->get_req()->set_query(sql);

    auto *series_task = Workflow::create_series_work(this->task, nullptr);
    auto *get_res_task = WFTaskFactory::create_go_task("getRES", callback, this->task);
    if(callback){
        auto *get_res_task = WFTaskFactory::create_go_task("getRES", callback, this->task);   
        series_task->push_back(get_res_task);
    }

    series_task->start();
}

void MySqlClient::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    // 最多阻塞1秒
    _cv.wait_for(lock, std::chrono::seconds(1), [this] { return task_finished_; });
    // _cv.wait_for(lock, [this] { return task_finished_; });
}

Json &MySqlClient::getResp()
{
    return mysql_resp;
}

/* Json &MySqlClient::syncGetResp()
{
    /* WFTaskFactory::create_go_task()
    return mysql_resp; 
}*/

MySqlClient::MySqlClient(const string& host, const string& port, const string& user,
const string& password, const string& database, const int& retry_max)
    : host(host)
    , port(port)
    , user(user)
    , password(password)
    , database(database)
    , retry_max(retry_max)
{
}
