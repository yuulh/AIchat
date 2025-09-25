#include "../src/client/RedisClient.h"
#include <log4cpp/PropertyConfigurator.hh>
#include <iostream>

using namespace std;

int main()
{
    log4cpp::PropertyConfigurator::configure("../config/log4cpp.conf");
    RedisClient redis("47.109.39.124", "6379", "", "1", 3);
    redis.GET("haha");

    redis.wait();
    vector<string> res;
    redis.getResp(redis.redis_resp, res);

    cout << "OK res size " << res.size() << endl;
    for(auto &i: res) {
        std::cout << i << " ";
    }
    cout << endl;
}