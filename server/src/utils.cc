#include "utils.h"
#include <cstring>

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




