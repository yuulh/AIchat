#ifndef MYSQLCLIENT_H
#define MYSQLCLIENT_H

#include <string>
#include <vector>
#include <workflow/MySQLResult.h>
#include <wfrest/HttpServer.h>
#include <wfrest/Json.h>
#include <mutex>
#include <condition_variable>


using namespace wfrest;
using namespace protocol;
using std::string;
using std::vector;

class MySqlClient {
    string host;
    string port;
    string user;
    string password;
    string database;
    int retry_max;

    // 实现wait
    std::mutex _mutex;
    std::condition_variable _cv;
    bool task_finished_;
    void wait();
    
    WFMySQLTask *task;
    // 结果json
    // 可能的字段：
    // "db_name"  "table_name"  "字段名"  "writed"(仅在写操作时)
    Json mysql_resp;
    static void mysql_callback(WFMySQLTask *task);

    static inline void setResp(Json &resp, const string &key, const string &value)
    {
        resp[key] = value;
    }

public:

    MySqlClient(const string &host, const string &port, const string &user,
                const string &password, const string &database, const int &retry_max);

    Json &getResp();  // 获取结果json，当sql操作未完成时会发生错误
    Json &syncGetResp();  // 同步等待sql结束后获取结果

    void execute(const string &sql);
    vector<vector<string>> query(const string &sql);
    string getUrl() const;

    inline void setDB(const string &database)
    {
        this->database = database;
    }

private:
};

#endif