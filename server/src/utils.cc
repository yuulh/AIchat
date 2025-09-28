#include "utils.h"
#include <uuid/uuid.h>
#include <workflow/WFFacilities.h>


string Cookie2User_id(RedisClient &redis, const string &cookie)
{
    vector<string> redis_ret;
    WFFacilities::WaitGroup wait_group(1);

    redis.execute("GET", {cookie}, [&](WFRedisTask *task){
        protocol::RedisValue &redis_resp = GET_REDIS_RESP;
        redis.parseResp(redis_resp, redis_ret);
        wait_group.done();
    });

    wait_group.wait();
    if(redis_ret.empty()) return "";
    return redis_ret[0];
}

std::string generate_uuid() {
    uuid_t uuid;
    uuid_generate(uuid);

    char uuid_str[37];
    uuid_unparse(uuid, uuid_str);

    return uuid_str;
}

// 获取当前时间，格式：yyyy-mm-dd hh:mm:ss
string getCurrDateTime()
{
    time_t now = time(nullptr);
    struct tm *tm_info = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return string(buf);
}


// base64解码
static const std::string base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::string base64_decode(const std::string &encoded) {
    std::string ret;
    std::vector<int> vec(256, -1);
    
    for (int i = 0; i < 64; i++)
        vec[base64_chars[i]] = i;
    
    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (vec[c] == -1) break;
        val = (val << 6) + vec[c];
        valb += 6;
        if (valb >= 0) {
            ret.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return ret;
}


/* files_t utils::readDir(const string &dir)
{
    DIR *dirf = opendir(dir.c_str());
    dirent *dirIt;
    files_t res;
    while((dirIt = readdir(dirf))){
        if(dirIt->d_name[0]=='.' || dirIt->d_type==DT_DIR) continue;
        string realPath(dir);
        if(dir.back() != '/') realPath.push_back('/');
        realPath += dirIt->d_name;
        realPath.resize(255);
        realpath(realPath.c_str(), &realPath.front());
        res.push_back(realPath);
    }
    closedir(dirf);
    return res;
}
 */




