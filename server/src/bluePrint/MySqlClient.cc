#include "MySqlClient.h"


MySqlClient::MySqlClient(const string &host, const string &user, const string &password, const string &database)
: host(host), user(user), password(password), database(database), connected(false)
{
}

