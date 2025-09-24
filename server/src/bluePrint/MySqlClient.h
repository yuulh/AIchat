#ifndef MYSQLCLIENT_H
#define MYSQLCLIENT_H

#include <string>
#include <vector>
#include <workflow/MySQLResult.h>
#include <wfrest/HttpServer.h>
#include <wfrest/Json.h>


using namespace wfrest;
using namespace protocol;
using std::string;
using std::vector;

class MySqlClient {
    string host;
    string user;
    string password;
    string database;

    bool connected;
public:
    MySqlClient(const string &host, const string &user, const string &password, const string &database);
    ~MySqlClient();
    int execute(const string &sql);
    vector<vector<string>> query(const string &sql);
    int getLastInsertId();
    int getAffectedRows();
    int getErrorCode();
    string getErrorMessage();

private:
    void *mysql;
}

#endif