#include "MySqlClient.h"

string MySqlClient::getUrl() const
{
    return "mysql://" + user + ":" + password + "@" + host + "/" + database;
}

string MySqlClient::execute(HttpResp *resp, const string &sql)
{
    resp->MySQL(getUrl(), sql, [resp](MySQLResultCursor *cursor){
            std::string res;
            std::vector<MySQLCell> arr;
            while (cursor->fetch_row(arr))
            {
                res.append(arr[0].as_string());
                res.append("\n");
            }
            return res;  // 存在多余拷贝
    }); 
}

MySqlClient::MySqlClient(const string &host, const string &user, const string &password, const string &database)
: host(host), user(user), password(password), database(database), connected(false)
{
}

