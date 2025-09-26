#include "utils.h"
#include <uuid/uuid.h>


string Cookie2User_id(RedisClient &redis, const string &cookie)
{
    vector<string> redis_ret;
    redis.execute("GET", {cookie});
    redis.wait();
    redis.getResp(redis.redis_resp, redis_ret);

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




