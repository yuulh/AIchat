#include "../src/client/MySqlClient.h"
#include <log4cpp/PropertyConfigurator.hh>
#include <iostream>

using namespace std;

int main()
{
    log4cpp::PropertyConfigurator::configure("../config/log4cpp.conf");
    MySqlClient mysql("47.109.39.124", "3306", "yuu", "123456", "AIchat", 3);
    
    mysql.execute("select * from test");

    Json res = mysql.syncGetResp();

    cout << res.dump() << endl;
    
    cout << endl;
}