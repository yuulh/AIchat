#include "../src/client/RedisClient.h"
#include <log4cpp/PropertyConfigurator.hh>
#include <iostream>

using namespace std;

int main()
{
    log4cpp::PropertyConfigurator::configure("../config/log4cpp.conf");
    RedisClient redis("47.109.39.124", "6379", "", "1", 3);
    redis.GET("assistant_all_page_1_10", [&](WFRedisTask *task){
        auto resp = GET_REDIS_RESP;

        vector<string> res;
        redis.parseResp(resp, res);
        cout << "OK res size " << res.size() << endl;
        if(res.size())
            cout << "res 0 " << res[0] << endl;
        for(auto &i: res) {
            std::cout << i << " ";
        }
    });
    
    cout << endl;

    getchar();
}